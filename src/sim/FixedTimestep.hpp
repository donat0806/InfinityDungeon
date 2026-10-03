#pragma once

namespace infinity_dungeon::sim {

// Converts variable real frame time into a whole number of fixed-size
// simulation steps, so gameplay ticks at a constant rate independent of
// the render frame rate. Caller runs the sim `Advance()` times per frame,
// each for `FixedDeltaSeconds()`, then uses `Alpha()` to interpolate
// rendering between the last two simulation states if desired.
class FixedTimestepClock {
public:
    explicit FixedTimestepClock(float tick_rate_hz = 60.0f);

    // Feeds elapsed real time and returns how many fixed steps to run now.
    // Caps the number of steps per call so a long stall (e.g. a breakpoint
    // or a slow frame) cannot force an unbounded burst of simulation steps
    // ("spiral of death"); leftover time is discarded when the cap is hit.
    int Advance(float real_delta_seconds);

    [[nodiscard]] float FixedDeltaSeconds() const { return fixed_delta_seconds_; }

    // How far between the last completed step and the next one, as a
    // fraction of one step (0 = just completed a step, <1 = partway to the
    // next). Intended for render interpolation.
    [[nodiscard]] float Alpha() const { return accumulator_ / fixed_delta_seconds_; }

private:
    static constexpr int kMaxStepsPerAdvance = 5;

    float fixed_delta_seconds_;
    float accumulator_ = 0.0f;
};

} // namespace infinity_dungeon::sim
