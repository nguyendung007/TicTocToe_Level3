#pragma once
#include <cstdint>
#include <random>

using Rng = std::mt19937;

inline constexpr std::uint32_t DEFAULT_SEED = 8702;
inline Rng makeRng(std::uint32_t seed = DEFAULT_SEED) {
    return Rng(seed);
}
