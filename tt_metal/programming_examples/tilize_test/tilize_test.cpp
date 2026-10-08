// Host-only experiment: shows how tilize_nfaces() reorders a row-major matrix. No device is opened.

#include <cstdint>
#include <numeric>
#include <vector>

#include <fmt/core.h>
#include <tt-metalium/constants.hpp>
#include <tt-metalium/tilize_utils.hpp>

using namespace tt::constants;

int main() {
    // 64x64 matrix = 2x2 tiles. Each element holds its own row-major index, so after tilizing, the
    // value tells us where the element came from: row = value / N, col = value % N.
    constexpr uint32_t M = 64;
    constexpr uint32_t N = 64;
    std::vector<uint32_t> row_major(M * N);
    std::iota(row_major.begin(), row_major.end(), 0);

    const std::vector<uint32_t> tiled = tilize_nfaces(row_major, M, N);

    auto row = [&](uint32_t out_idx) { return tiled[out_idx] / N; };
    auto col = [&](uint32_t out_idx) { return tiled[out_idx] % N; };

    fmt::print("Value at the same index in both vectors, with the (row, col) that value came from:\n");
    fmt::print("  idx | row_major    | tiled\n");
    for (uint32_t i = 0; i < 20; i++) {
        fmt::print(
            "  {:3} | {:4} ({}, {:2}) | {:4} ({}, {:2})\n",
            i,
            row_major[i],
            row_major[i] / N,
            row_major[i] % N,
            tiled[i],
            row(i),
            col(i));
    }

    fmt::print("\nWhere each face comes from:\n");
    fmt::print("\ntest: {}\n", tiled.size());
    constexpr uint32_t faces_per_tile = TILE_HW / FACE_HW;
    for (uint32_t first = 0; first < tiled.size(); first += FACE_HW) {
        const uint32_t last = first + FACE_HW - 1;
        const uint32_t face = first / FACE_HW;
        fmt::print(
            "  tile {} face {}: out[{:4}..{:4}] <- rows {:2}..{:2}, cols {:2}..{:2}\n",
            face / faces_per_tile,
            face % faces_per_tile,
            first,
            last,
            row(first),
            row(last),
            col(first),
            col(last));
    }

    const bool round_trip_ok = untilize_nfaces(tiled, M, N) == row_major;
    fmt::print("\nuntilize_nfaces(tilize_nfaces(x)) == x: {}\n", round_trip_ok);
    return round_trip_ok ? 0 : 1;
}
