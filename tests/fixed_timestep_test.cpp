#include "sim/FixedTimestep.hpp"

#include <doctest/doctest.h>

using infinity_dungeon::sim::FixedTimestepClock;

TEST_CASE("FixedTimestepClock runs one step per exact tick interval") {
    FixedTimestepClock clock(60.0f); // fixed dt = 1/60 s

    CHECK(clock.Advance(1.0f / 60.0f) == 1);
    CHECK(clock.Advance(1.0f / 60.0f) == 1);
}

TEST_CASE("FixedTimestepClock accumulates fractional time across frames") {
    FixedTimestepClock clock(60.0f);

    CHECK(clock.Advance(1.0f / 120.0f) == 0); // half a step: not enough yet
    CHECK(clock.Advance(1.0f / 120.0f) == 1); // the other half completes it
}

TEST_CASE("FixedTimestepClock caps steps and drops the backlog on a long stall") {
    FixedTimestepClock clock(60.0f);

    const int steps = clock.Advance(10.0f); // a huge stall (e.g. a breakpoint)
    CHECK(steps <= 5);
    CHECK(clock.Alpha() == doctest::Approx(0.0f));
}

TEST_CASE("FixedTimestepClock alpha reports progress toward the next step") {
    FixedTimestepClock clock(60.0f);

    clock.Advance(1.0f / 120.0f);
    CHECK(clock.Alpha() == doctest::Approx(0.5f));
}
