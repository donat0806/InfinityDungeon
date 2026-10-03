#include "sim/GameState.hpp"

#include <doctest/doctest.h>

#include <algorithm>
#include <cmath>
#include <set>
#include <unordered_map>

using infinity_dungeon::input::PlayerInput;
using infinity_dungeon::sim::GameState;
using infinity_dungeon::sim::PlayerId;

namespace {

constexpr float kDt = 1.0f / 60.0f;

void TickN(GameState& state, PlayerId id, const PlayerInput& input, int ticks) {
    const std::unordered_map<PlayerId, PlayerInput> inputs{{id, input}};
    for (int i = 0; i < ticks; ++i) {
        state.Tick(kDt, inputs);
    }
}

} // namespace

TEST_CASE("diagonal movement is no faster than straight movement") {
    GameState straight(1);
    GameState diagonal(1);
    const PlayerId a = straight.AddPlayer(0.0f, 0.0f);
    const PlayerId b = diagonal.AddPlayer(0.0f, 0.0f);

    TickN(straight, a, PlayerInput{.move_x = 1.0f}, 30);
    TickN(diagonal, b, PlayerInput{.move_x = 1.0f, .move_y = 1.0f}, 30);

    const auto& s = straight.Players()[0];
    const auto& d = diagonal.Players()[0];
    CHECK(std::sqrt(d.x * d.x + d.y * d.y) == doctest::Approx(s.x).epsilon(0.001));
}

TEST_CASE("player is clamped inside the arena") {
    GameState state(1);
    const PlayerId id = state.AddPlayer(0.0f, 0.0f);

    TickN(state, id, PlayerInput{.move_x = 1.0f, .move_y = -1.0f}, 600);

    const auto& arena = state.ArenaBounds();
    const auto& player = state.Players()[0];
    CHECK(player.x == doctest::Approx(arena.max_x - player.stats.radius));
    CHECK(player.y == doctest::Approx(arena.min_y + player.stats.radius));
}

TEST_CASE("aiming spawns a projectile in the aim direction; no aim spawns none") {
    GameState state(1);
    const PlayerId id = state.AddPlayer(0.0f, 0.0f);

    TickN(state, id, PlayerInput{.move_x = 1.0f}, 1);
    CHECK(state.Projectiles().empty());

    TickN(state, id, PlayerInput{.aim_x = 1.0f}, 1);
    REQUIRE(state.Projectiles().size() == 1);
    const auto& p = state.Projectiles()[0];
    CHECK(p.owner == id);
    CHECK(p.vx > 0.0f);
    CHECK(p.vy == doctest::Approx(0.0f));
}

TEST_CASE("holding aim fires at the fire interval") {
    GameState state(1);
    const PlayerId id = state.AddPlayer(0.0f, 0.0f);
    const auto& stats = state.Players()[0].stats;

    // Projectiles despawn within the second, so count spawns by the highest
    // ID seen (IDs are assigned sequentially from 0).
    std::size_t shots = 0;
    for (int i = 0; i < 60; ++i) {
        TickN(state, id, PlayerInput{.aim_y = 1.0f}, 1);
        for (const auto& p : state.Projectiles()) {
            shots = std::max<std::size_t>(shots, p.id + 1);
        }
    }

    const auto expected = static_cast<std::size_t>(std::ceil(1.0f / stats.fire_interval));
    CHECK(shots >= expected - 1);
    CHECK(shots <= expected + 1);
}

TEST_CASE("projectiles despawn after their lifetime or on leaving the arena") {
    GameState state(1);
    const PlayerId id = state.AddPlayer(0.0f, 0.0f);

    // Fire once, then stop aiming and let it fly.
    TickN(state, id, PlayerInput{.aim_x = 1.0f}, 1);
    REQUIRE(state.Projectiles().size() == 1);
    TickN(state, id, PlayerInput{}, 120);
    CHECK(state.Projectiles().empty());
}

TEST_CASE("projectile ids are unique") {
    GameState state(1);
    const PlayerId id = state.AddPlayer(0.0f, 0.0f);

    TickN(state, id, PlayerInput{.aim_y = 1.0f}, 60);

    std::set<infinity_dungeon::sim::ProjectileId> ids;
    for (const auto& p : state.Projectiles()) {
        ids.insert(p.id);
    }
    CHECK(ids.size() == state.Projectiles().size());
}
