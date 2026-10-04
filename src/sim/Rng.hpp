#pragma once

#include <cstdint>

namespace infinity_dungeon::sim {

// Small PCG32 generator. Used instead of <random> distributions because those
// are allowed to differ between standard libraries, which would make a seed
// produce different runs on different platforms.
class Rng {
public:
    explicit Rng(std::uint64_t seed, std::uint64_t stream = 54u) : inc_((stream << 1u) | 1u) {
        NextU32();
        state_ += seed;
        NextU32();
    }

    std::uint32_t NextU32() {
        const std::uint64_t old = state_;
        state_ = old * 6364136223846793005ULL + inc_;
        const auto xorshifted = static_cast<std::uint32_t>(((old >> 18u) ^ old) >> 27u);
        const auto rot = static_cast<std::uint32_t>(old >> 59u);
        return (xorshifted >> rot) | (xorshifted << ((~rot + 1u) & 31u));
    }

    // Uniform in [min, max).
    float NextFloat(float min, float max) {
        const float unit = static_cast<float>(NextU32() >> 8u) * (1.0f / 16777216.0f);
        return min + (max - min) * unit;
    }

    // Uniform-ish in [0, n); n must be > 0.
    std::uint32_t NextIndex(std::uint32_t n) { return NextU32() % n; }

private:
    std::uint64_t state_ = 0;
    std::uint64_t inc_;
};

} // namespace infinity_dungeon::sim
