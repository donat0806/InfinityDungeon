#include "sim/GameState.hpp"

#include <doctest/doctest.h>

#include <unordered_map>

using infinity_dungeon::input::PlayerInput;
using infinity_dungeon::sim::GameState;
using infinity_dungeon::sim::PlayerId;

TEST_CASE("GameState generates the level for its starting depth") {
    const GameState state(42);
    CHECK(state.Depth() == 1);
    CHECK_FALSE(state.Rooms().empty());
}

TEST_CASE("GameState moves a player by its input, scaled by the fixed step") {
    GameState state(42);
    const PlayerId id = state.AddPlayer(0.0f, 0.0f);

    const std::unordered_map<PlayerId, PlayerInput> inputs{{id, PlayerInput{1.0f, 0.0f, false}}};
    state.Tick(1.0f / 60.0f, inputs);

    REQUIRE(state.Players().size() == 1);
    CHECK(state.Players()[0].x > 0.0f);
    CHECK(state.Players()[0].y == doctest::Approx(0.0f));
}

TEST_CASE("GameState leaves a player in place when it has no input this tick") {
    GameState state(42);
    const PlayerId id = state.AddPlayer(5.0f, 5.0f);

    state.Tick(1.0f / 60.0f, {});

    REQUIRE(state.Players().size() == 1);
    CHECK(state.Players()[0].id == id);
    CHECK(state.Players()[0].x == doctest::Approx(5.0f));
    CHECK(state.Players()[0].y == doctest::Approx(5.0f));
}

TEST_CASE("GameState ticking is deterministic for the same seed and inputs") {
    GameState a(42);
    GameState b(42);
    const PlayerId id_a = a.AddPlayer(0.0f, 0.0f);
    const PlayerId id_b = b.AddPlayer(0.0f, 0.0f);

    const std::unordered_map<PlayerId, PlayerInput> inputs_a{{id_a, PlayerInput{0.5f, -1.0f, true}}};
    const std::unordered_map<PlayerId, PlayerInput> inputs_b{{id_b, PlayerInput{0.5f, -1.0f, true}}};

    for (int i = 0; i < 10; ++i) {
        a.Tick(1.0f / 60.0f, inputs_a);
        b.Tick(1.0f / 60.0f, inputs_b);
    }

    CHECK(a.Players()[0].x == doctest::Approx(b.Players()[0].x));
    CHECK(a.Players()[0].y == doctest::Approx(b.Players()[0].y));
}
