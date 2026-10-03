#include "FixedTimestep.hpp"

namespace infinity_dungeon::sim {

FixedTimestepClock::FixedTimestepClock(float tick_rate_hz) : fixed_delta_seconds_(1.0f / tick_rate_hz) {}

int FixedTimestepClock::Advance(float real_delta_seconds) {
    accumulator_ += real_delta_seconds;

    int steps = 0;
    while (accumulator_ >= fixed_delta_seconds_ && steps < kMaxStepsPerAdvance) {
        accumulator_ -= fixed_delta_seconds_;
        ++steps;
    }

    if (steps == kMaxStepsPerAdvance) {
        accumulator_ = 0.0f;
    }

    return steps;
}

} // namespace infinity_dungeon::sim
