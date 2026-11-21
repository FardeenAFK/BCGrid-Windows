// Host-side test for the portable render core (no Adobe SDK needed).
// Build: c++ -std=c++17 -O2 -I../src render_test.cpp -o render_test
// Writes PPM frames for every pattern at several progress values, plus self-checks.
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "BCGridCore.h"

using namespace bcgrid;

static int failures = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #c); ++failures; } } while (0)

struct Image8 {
    int w, h;
    std::vector<Pixel8> px;
    Image8(int W, int H) : w(W), h(H), px((size_t)W * H) {}
    World world() { return World{px.data(), (int)(w * sizeof(Pixel8)), w, h}; }
};

static void FillGradient(Image8& im, bool second) {
    for (int y = 0; y < im.h; ++y)
        for (int x = 0; x < im.w; ++x) {
            Pixel8& p = im.px[(size_t)y * im.w + x];
            p.alpha = 255;
            if (second) { p.red = 30; p.green = (uint8_t)(60 + 120 * x / im.w); p.blue = 200; }
            else        { p.red = 220; p.green = (uint8_t)(80 + 100 * y / im.h); p.blue = 40; }
        }
}

static void WritePPM(const Image8& im, const std::string& path) {
    FILE* f = std::fopen(path.c_str(), "wb");
    std::fprintf(f, "P6\n%d %d\n255\n", im.w, im.h);
    for (const Pixel8& p : im.px) { unsigned char c[3] = {p.red, p.green, p.blue}; std::fwrite(c, 1, 3, f); }
    std::fclose(f);
}

int main(int argc, char** argv) {
    const std::string outDir = argc > 1 ? argv[1] : ".";

    // --- ordering checks ---------------------------------------------------
    {
        auto r = ComputeRanks(3, 3, kRows, 1);
        for (int i = 0; i < 9; ++i) CHECK(r[i] == i);
        r = ComputeRanks(3, 2, kColumns, 1);  // column by column, top to bottom
        int expect[6] = {0, 2, 4, 1, 3, 5};
        for (int i = 0; i < 6; ++i) CHECK(r[i] == expect[i]);
        r = ComputeRanks(3, 3, kFromCenter, 1);
        CHECK(r[4] == 0);                      // centre goes first
        r = ComputeRanks(3, 3, kToCenter, 1);
        CHECK(r[4] == 8);                      // centre goes last
        r = ComputeRanks(4, 4, kRandom, 7);
        std::vector<int> seen(16, 0);
        for (int v : r) seen[v]++;
        for (int v : seen) CHECK(v == 1);      // a permutation
        CHECK(ComputeRanks(4, 4, kRandom, 7) == ComputeRanks(4, 4, kRandom, 7));
        CHECK(ComputeRanks(4, 4, kRandom, 7) != ComputeRanks(4, 4, kRandom, 8));
        CHECK(To16(255) == 32768 && To16(0) == 0 && To16(128) == 16448);
    }

    // --- render checks -----------------------------------------------------
    const int W = 480, H = 270;
    Image8 A(W, H), B(W, H), out(W, H);
    FillGradient(A, false);
    FillGradient(B, true);
    const Pixel8 line{255, 0, 0, 0};

    Settings s{};
    s.cols = 3; s.rows = 3; s.thickness = 4; s.flashOn = true; s.intensity = 0.8;
    s.flashWidth = 4.0 / 24.0;

    s.progress = 0.0;
    auto rank = ComputeRanks(3, 3, kRandom, 1);
    Render<Pixel8>(A.world(), B.world(), out.world(), s, rank, line, 255, 255, 255);
    CHECK(std::memcmp(out.px.data(), A.px.data(), out.px.size() * sizeof(Pixel8)) == 0);
    s.progress = 1.0;
    Render<Pixel8>(A.world(), B.world(), out.world(), s, rank, line, 255, 255, 255);
    CHECK(std::memcmp(out.px.data(), B.px.data(), out.px.size() * sizeof(Pixel8)) == 0);

    // Mid transition, no flash: interior pixels are exactly A or B; borders are line colour.
    s.progress = 0.5; s.flashOn = false;
    Render<Pixel8>(A.world(), B.world(), out.world(), s, rank, line, 255, 255, 255);
    {
        const Pixel8 p = out.px[0];
        CHECK(p.alpha == 255 && p.red == 0 && p.green == 0 && p.blue == 0);  // frame edge is a line
        int nA = 0, nB = 0;
        for (int cy = 0; cy < 3; ++cy)
            for (int cx = 0; cx < 3; ++cx) {
                const int x = cx * W / 3 + W / 6, y = cy * H / 3 + H / 6;
                const Pixel8 q = out.px[(size_t)y * W + x];
                if (q.red == A.px[(size_t)y * W + x].red) ++nA; else ++nB;
            }
        CHECK(nA + nB == 9 && nB == 5);  // ranks 0..4 have t <= 0.5 -> 5 cells switched
    }

    // --- frames for eyeballing ---------------------------------------------
    const char* names[] = {"", "aleatorio", "filas", "columnas", "diagonal", "desde_centro", "hacia_centro"};
    s.cols = 6; s.rows = 4; s.flashOn = true; s.thickness = 4;
    for (int pat = 1; pat <= 6; ++pat) {
        auto rk = ComputeRanks(s.cols, s.rows, pat, 1);
        for (int step = 0; step <= 4; ++step) {
            s.progress = step / 4.0;
            Render<Pixel8>(A.world(), B.world(), out.world(), s, rk, line, 255, 255, 255);
            char buf[256];
            std::snprintf(buf, sizeof buf, "%s/%d_%s_%03d.ppm", outDir.c_str(), pat, names[pat], step * 25);
            WritePPM(out, buf);
        }
    }

    std::printf(failures ? "%d check(s) FAILED\n" : "all checks passed\n", failures);
    return failures ? 1 : 0;
}
