// BC(c)Grid - grid transition by BillyCreative.
// Portable render core: no Adobe SDK types, so it can be unit-tested on any OS.
// Behaviour mirrors the shipped macOS build (BCGrid.plugin v1.0) exactly.
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace bcgrid {

enum Pattern {
    kRandom = 1,      // Aleatorio
    kRows = 2,        // Filas
    kColumns = 3,     // Columnas
    kDiagonal = 4,    // Diagonal
    kFromCenter = 5,  // Desde el centro
    kToCenter = 6     // Hacia el centro
};

// Same memory layout as PF_Pixel / PF_Pixel16 (alpha first).
struct Pixel8 { uint8_t alpha, red, green, blue; };
struct Pixel16 { uint16_t alpha, red, green, blue; };

struct World {
    void* data;
    int rowbytes;
    int width;
    int height;
};

struct Settings {
    double progress;    // 0..1
    int cols, rows;     // >= 1
    double thickness;   // line width in output pixels; 0 = no lines
    bool flashOn;       // flash checkbox on AND intensity > 0
    double intensity;   // 0..1
    double flashWidth;  // flash half-width in progress units (>= 0.001)
};

// 8-bit -> 16-bit (0..32768) channel conversion used by the macOS build.
inline uint16_t To16(uint8_t c) {
    return (uint16_t)((((uint64_t)((uint32_t)c << 15)) * 0x01010102ull) >> 32);
}

// rank[cell] = position (0..n-1) in which that cell switches to the second clip.
inline std::vector<int> ComputeRanks(int cols, int rows, int pattern, int seed) {
    const int n = cols * rows;
    std::vector<double> key((size_t)n, 0.0);
    std::vector<int> order((size_t)n);
    const double cx = (double)(unsigned)(cols - 1) / 2.0;
    const double cy = (double)(unsigned)(rows - 1) / 2.0;

    for (int y = 0; y < rows; ++y) {
        const double dy = (double)y - cy;
        for (int x = 0; x < cols; ++x) {
            const int idx = y * cols + x;
            order[idx] = idx;
            double k = 0.0;
            switch (pattern) {
                case kRows:       k = (double)idx; break;
                case kColumns:    k = (double)(x * rows + y); break;
                case kDiagonal:   k = (double)(x + y); break;
                case kFromCenter: { const double dx = (double)x - cx; k = std::sqrt(dx * dx + dy * dy); } break;
                case kToCenter:   { const double dx = (double)x - cx; k = -std::sqrt(dx * dx + dy * dy); } break;
                default: break;
            }
            key[idx] = k;
        }
    }

    if (pattern == kRandom) {
        if (n >= 2) {
            // Seeded xorshift32 Fisher-Yates shuffle (identical sequence to macOS).
            uint32_t s = ((uint32_t)seed * 0x9E3779B1u) ^ 0x92D68CA2u;
            for (uint32_t i = (uint32_t)n; i >= 2; --i) {
                s ^= s << 13;
                s ^= s >> 17;
                s ^= s << 5;
                const uint32_t j = s % i;
                std::swap(order[i - 1], order[j]);
            }
        }
    } else {
        std::stable_sort(order.begin(), order.end(),
                         [&key](int a, int b) { return key[a] < key[b]; });
    }

    std::vector<int> rank((size_t)n, 0);
    for (int k = 0; k < n; ++k) rank[order[k]] = k;
    return rank;
}

template <typename Px>
inline Px* RowPtr(const World& w, int y) {
    return reinterpret_cast<Px*>(static_cast<char*>(w.data) + (intptr_t)(w.rowbytes * y));
}

// Px = Pixel8 or Pixel16. linePx is the opaque line pixel, fr/fg/fb the flash colour
// already in the target bit depth.
template <typename Px>
void Render(const World& A, const World& B, const World& out, const Settings& s,
            const std::vector<int>& rank, Px linePx, int fr, int fg, int fb) {
    const int W = out.width, H = out.height;
    if (W < 1 || H < 1) return;

    const int cols = s.cols, rows = s.rows;
    const double n = (double)(unsigned)(cols * rows);
    const bool full = (s.progress <= 0.0) || (s.progress >= 1.0);
    const World& fullSrc = (s.progress <= 0.0) ? A : B;
    const double half = s.thickness * 0.5;

    for (int y = 0; y < H; ++y) {
        Px* dst = RowPtr<Px>(out, y);
        int r = (int)((double)rows * (double)y / (double)H);
        if (r > rows - 1) r = rows - 1;
        const double top = (double)y - (double)H * (double)r / (double)rows;
        const double bottom = ((double)H * (double)(r + 1) / (double)rows - 1.0) - (double)y;

        for (int x = 0; x < W; ++x) {
            const World* src = &fullSrc;
            double f = 0.0;

            if (!full) {
                int c = (int)((double)cols * (double)x / (double)W);
                if (c > cols - 1) c = cols - 1;

                if (s.thickness > 0.0) {
                    const double left = (double)x - (double)W * (double)c / (double)cols;
                    const double right = ((double)W * (double)(c + 1) / (double)cols - 1.0) - (double)x;
                    if (left < half || right < half || top < half || bottom < half) {
                        dst[x] = linePx;
                        continue;
                    }
                }

                const double t = ((double)rank[c + r * cols] + 0.5) / n;
                if (s.flashOn) {
                    const double d = std::fabs(s.progress - t) / s.flashWidth;
                    if (d < 1.0) {
                        const double g = 1.0 - d;
                        f = g * (s.intensity * g);
                    }
                }
                src = (s.progress >= t) ? &B : &A;
            }

            if (!src->data || src->width - 1 < 0 || src->height <= 0) {
                dst[x] = Px{};
                continue;
            }
            int xs = std::min(x, src->width - 1);
            int ys = std::min(y, src->height - 1);
            if (xs < 0) xs = 0;
            if (ys < 0) ys = 0;
            const Px p = RowPtr<Px>(*src, ys)[xs];
            dst[x] = p;

            if (f > 0.0) {
                if (f > 1.0) f = 1.0;
                dst[x].red = (decltype(p.red))(int)((double)(fr - (int)p.red) * f + (double)p.red);
                dst[x].green = (decltype(p.green))(int)((double)(fg - (int)p.green) * f + (double)p.green);
                dst[x].blue = (decltype(p.blue))(int)((double)(fb - (int)p.blue) * f + (double)p.blue);
            }
        }
    }
}

}  // namespace bcgrid
