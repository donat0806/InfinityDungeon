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

    const std::unordered_map<PlayerId, PlayerInput> inputs{{id, PlayerInput{.move_x = 1.0f}}};
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

    const std::unordered_map<PlayerId, PlayerInput> inputs_a{
        {id_a, PlayerInput{.move_x = 0.5f, .move_y = -1.0f, .aim_x = 1.0f, .aim_y = 0.5f}}};
    const std::unordered_map<PlayerId, PlayerInput> inputs_b{
        {id_b, PlayerInput{.move_x = 0.5f, .move_y = -1.0f, .aim_x = 1.0f, .aim_y = 0.5f}}};

    for (int i = 0; i < 60; ++i) {
        a.Tick(1.0f / 60.0f, inputs_a);
        b.Tick(1.0f / 60.0f, inputs_b);
    }

    CHECK(a.Players()[0].x == doctest::Approx(b.Players()[0].x));
    CHECK(a.Players()[0].y == doctest::Approx(b.Players()[0].y));

    REQUIRE(a.Projectiles().size() == b.Projectiles().size());
    for (std::size_t i = 0; i < a.Projectiles().size(); ++i) {
        CHECK(a.Projectiles()[i].id == b.Projectiles()[i].id);
        CHECK(a.Projectiles()[i].x == doctest::Approx(b.Projectiles()[i].x));
        CHECK(a.Projectiles()[i].y == doctest::Approx(b.Projectiles()[i].y));
    }
}

TEST_CASE("GameState with an enemy wave is deterministic for the same seed and inputs") {
    GameState a(42);
    GameState b(42);
    const PlayerId id_a = a.AddPlayer(0.0f, 0.0f);
    const PlayerId id_b = b.AddPlayer(0.0f, 0.0f);
    a.SpawnInitialWave();
    b.SpawnInitialWave();

    const std::unordered_map<PlayerId, PlayerInput> inputs_a{{id_a, PlayerInput{.move_x = 1.0f, .aim_x = 1.0f}}};
    const std::unordered_map<PlayerId, PlayerInput> inputs_b{{id_b, PlayerInput{.move_x = 1.0f, .aim_x = 1.0f}}};

    for (int i = 0; i < 300; ++i) {
        a.Tick(1.0f / 60.0f, inputs_a);
        b.Tick(1.0f / 60.0f, inputs_b);
    }

    REQUIRE(a.Enemies().size() == b.Enemies().size());
    for (std::size_t i = 0; i < a.Enemies().size(); ++i) {
        CHECK(a.Enemies()[i].id == b.Enemies()[i].id);
        CHECK(a.Enemies()[i].health == b.Enemies()[i].health);
        CHECK(a.Enemies()[i].x == doctest::Approx(b.Enemies()[i].x));
        CHECK(a.Enemies()[i].y == doctest::Approx(b.Enemies()[i].y));
    }
    CHECK(a.Players()[0].health == b.Players()[0].health);
}
