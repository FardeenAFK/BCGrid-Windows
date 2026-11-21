// Renders preview frames of the transition with the real render core.
// Build: c++ -std=c++17 -O2 -Isrc test/make_preview.cpp -o make_preview
// Usage: make_preview <outdir> <pattern 1-6> <cols> <rows> <width> <height> <frames>
// Writes <outdir>/f0000.ppm ... (frames of the transition plus a short hold on each end).
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "BCGridCore.h"

using namespace bcgrid;

static uint8_t clamp8(double v) { return (uint8_t)(v < 0 ? 0 : v > 255 ? 255 : v); }

// Clip A: warm sunset with a sun and hills. Clip B: cool night with a moon and stars.
static void PaintScene(std::vector<Pixel8>& px, int W, int H, bool night) {
    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x) {
            const double u = (double)x / W, v = (double)y / H;
            double r, g, b;
            if (!night) {
                r = 255 - 40 * v; g = 140 + 60 * (1 - v) - 40 * v; b = 90 + 60 * v;
                const double d = std::hypot(u - 0.68, (v - 0.45) * H / W);
                if (d < 0.11) { r = 255; g = 236; b = 170; }
                else if (d < 0.2) { const double k = (0.2 - d) / 0.09; r += (255 - r) * k * 0.5; g += (220 - g) * k * 0.5; b += (160 - b) * k * 0.4; }
                const double hill = 0.72 + 0.06 * std::sin(u * 7.0) + 0.03 * std::sin(u * 17.0 + 1.3);
                if (v > hill) { r = 120 - 40 * v; g = 50; b = 70 + 20 * u; }
            } else {
                r = 12 + 25 * v; g = 24 + 50 * v; b = 70 + 90 * v;
                const double d = std::hypot(u - 0.3, (v - 0.32) * H / W);
                if (d < 0.08) { r = 235; g = 240; b = 255; }
                else if (d < 0.16) { const double k = (0.16 - d) / 0.08; r += 90 * k * 0.4; g += 110 * k * 0.4; b += 140 * k * 0.4; }
                const unsigned h = (unsigned)(x * 73856093u ^ y * 19349663u);
                if (h % 1400 == 0 && v < 0.65) { r = g = b = 255; }
                const double hill = 0.78 + 0.05 * std::sin(u * 5.0 + 2.0) + 0.02 * std::sin(u * 21.0);
                if (v > hill) { r = 8; g = 30 + 20 * u; b = 40; }
            }
            Pixel8& p = px[(size_t)y * W + x];
            p.alpha = 255; p.red = clamp8(r); p.green = clamp8(g); p.blue = clamp8(b);
        }
}

int main(int argc, char** argv) {
    if (argc < 8) { std::fprintf(stderr, "usage: make_preview outdir pattern cols rows w h frames\n"); return 1; }
    const std::string dir = argv[1];
    const int pattern = std::atoi(argv[2]), cols = std::atoi(argv[3]), rows = std::atoi(argv[4]);
    const int W = std::atoi(argv[5]), H = std::atoi(argv[6]), frames = std::atoi(argv[7]);
    const int hold = frames / 4;

    std::vector<Pixel8> a((size_t)W * H), b((size_t)W * H), out((size_t)W * H);
    PaintScene(a, W, H, false);
    PaintScene(b, W, H, true);
    const World A{a.data(), W * 4, W, H}, B{b.data(), W * 4, W, H}, O{out.data(), W * 4, W, H};

    Settings s{};
    s.cols = cols; s.rows = rows;
    s.thickness = 4.0 * W / 1920.0 * 2.0;  // default 4 px at 1080p, scaled to preview size
    s.flashOn = true; s.intensity = 0.8;
    s.flashWidth = 4.0 / frames;
    const std::vector<int> rank = ComputeRanks(cols, rows, pattern, 1);
    const Pixel8 line{255, 0, 0, 0};

    int idx = 0;
    for (int f = -hold; f <= frames + hold; ++f) {
        s.progress = f <= 0 ? 0.0 : f >= frames ? 1.0 : (double)f / frames;
        Render<Pixel8>(A, B, O, s, rank, line, 255, 255, 255);
        char path[512];
        std::snprintf(path, sizeof path, "%s/f%04d.ppm", dir.c_str(), idx++);
        FILE* fp = std::fopen(path, "wb");
        std::fprintf(fp, "P6\n%d %d\n255\n", W, H);
        for (const Pixel8& p : out) { unsigned char c[3] = {p.red, p.green, p.blue}; std::fwrite(c, 1, 3, fp); }
        std::fclose(fp);
    }
    return 0;
}
