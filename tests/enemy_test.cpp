#include "sim/GameState.hpp"

#include <doctest/doctest.h>

#include <cmath>
#include <unordered_map>

using infinity_dungeon::input::PlayerInput;
using infinity_dungeon::sim::EnemyConfig;
using infinity_dungeon::sim::EnemyKind;
using infinity_dungeon::sim::Faction;
using infinity_dungeon::sim::GameState;
using infinity_dungeon::sim::PlayerId;
using infinity_dungeon::sim::Projectile;

namespace {

constexpr float kDt = 1.0f / 60.0f;

void TickN(GameState& state, PlayerId id, const PlayerInput& input, int ticks) {
    const std::unordered_map<PlayerId, PlayerInput> inputs{{id, input}};
    for (int i = 0; i < ticks; ++i) {
        state.Tick(kDt, inputs);
    }
}

float Distance(float ax, float ay, float bx, float by) {
    return std::sqrt((ax - bx) * (ax - bx) + (ay - by) * (ay - by));
}

} // namespace

TEST_CASE("a chaser closes in on the player") {
    GameState state(1);
    const PlayerId id = state.AddPlayer(0.0f, 0.0f);
    state.SpawnEnemy(EnemyKind::Chaser, 300.0f, 0.0f);

    const float before = Distance(300.0f, 0.0f, 0.0f, 0.0f);
    TickN(state, id, PlayerInput{}, 30);

    REQUIRE(state.Enemies().size() == 1);
    const auto& enemy = state.Enemies()[0];
    CHECK(Distance(enemy.x, enemy.y, 0.0f, 0.0f) < before);
}

TEST_CASE("a shooter fires enemy projectiles at the player on its interval") {
    GameState state(1);
    const PlayerId id = state.AddPlayer(0.0f, 0.0f);
    state.SpawnEnemy(EnemyKind::Shooter, 250.0f, 0.0f);
    const float interval = state.Enemies()[0].stats.fire_interval;

    // Just short of the first shot: nothing yet.
    TickN(state, id, PlayerInput{}, static_cast<int>(interval * 60.0f) - 5);
    CHECK(state.Projectiles().empty());

    TickN(state, id, PlayerInput{}, 10);
    REQUIRE(state.Projectiles().size() == 1);
    const Projectile& shot = state.Projectiles()[0];
    CHECK(shot.faction == Faction::Enemy);
    CHECK(shot.vx < 0.0f); // toward the player at the origin
}

TEST_CASE("a shooter backs away when the player is too close") {
    GameState state(1);
    const PlayerId id = state.AddPlayer(0.0f, 0.0f);
    state.SpawnEnemy(EnemyKind::Shooter, 100.0f, 0.0f);

    TickN(state, id, PlayerInput{}, 10);

    CHECK(state.Enemies()[0].x > 100.0f);
}

TEST_CASE("a patroller stays inside the arena and bounces off walls") {
    GameState state(1);
    const PlayerId id = state.AddPlayer(-400.0f, 200.0f);
    state.SpawnEnemy(EnemyKind::Patroller, 0.0f, 0.0f);
    const auto& arena = state.ArenaBounds();

    float min_x = 0.0f;
    float max_x = 0.0f;
    for (int i = 0; i < 1800; ++i) {
        TickN(state, id, PlayerInput{}, 1);
        const auto& enemy = state.Enemies()[0];
        CHECK(enemy.x - enemy.stats.radius >= arena.min_x);
        CHECK(enemy.x + enemy.stats.radius <= arena.max_x);
        CHECK(enemy.y - enemy.stats.radius >= arena.min_y);
        CHECK(enemy.y + enemy.stats.radius <= arena.max_y);
        min_x = std::min(min_x, enemy.x);
        max_x = std::max(max_x, enemy.x);
    }
    // It travelled in both directions along x, so it must have bounced.
    CHECK(min_x < 0.0f);
    CHECK(max_x > 0.0f);
}

TEST_CASE("a player projectile damages an enemy and is consumed; enemy dies at zero health") {
    GameState state(1);
    const PlayerId id = state.AddPlayer(0.0f, 0.0f);
    // A patroller moves diagonally, so use a shooter parked far away instead: it only
    // matters that it is hit, not how it moves.
    state.SpawnEnemy(EnemyKind::Shooter, 150.0f, 0.0f);
    const int max_health = state.Enemies()[0].health;
    REQUIRE(max_health >= 2);

    // Hold fire along +x until the first hit lands.
    for (int i = 0; i < 60 && state.Enemies()[0].health == max_health; ++i) {
        TickN(state, id, PlayerInput{.aim_x = 1.0f}, 1);
    }
    REQUIRE(!state.Enemies().empty());
    CHECK(state.Enemies()[0].health == max_health - 1);

    for (int i = 0; i < 240 && !state.Enemies().empty(); ++i) {
        TickN(state, id, PlayerInput{.aim_x = 1.0f}, 1);
    }
    CHECK(state.Enemies().empty());
}

TEST_CASE("contact damage hurts the player once, then invulnerability blocks repeats") {
    GameState state(1);
    const PlayerId id = state.AddPlayer(0.0f, 0.0f);
    state.SpawnEnemy(EnemyKind::Chaser, 0.0f, 0.0f);
    const int start = state.Players()[0].health;

    TickN(state, id, PlayerInput{}, 1);
    CHECK(state.Players()[0].health == start - 1);

    TickN(state, id, PlayerInput{}, 30); // still inside the 1s invulnerability window
    CHECK(state.Players()[0].health == start - 1);

    TickN(state, id, PlayerInput{}, 60); // window over; the chaser is still on top of the player
    CHECK(state.Players()[0].health < start - 1);
}

TEST_CASE("enemy projectiles hurt the player but not other enemies") {
    GameState state(1);
    const PlayerId id = state.AddPlayer(0.0f, 0.0f);
    state.SpawnEnemy(EnemyKind::Shooter, 250.0f, 0.0f);
    // A bystander in the line of fire, between the shooter and the player.
    state.SpawnEnemy(EnemyKind::Patroller, 125.0f, 0.0f);
    const int bystander_health = state.Enemies()[1].health;
    const int player_health = state.Players()[0].health;

    TickN(state, id, PlayerInput{}, 150);

    CHECK(state.Players()[0].health < player_health);
    for (const auto& enemy : state.Enemies()) {
        if (enemy.kind == EnemyKind::Patroller) {
            CHECK(enemy.health == bystander_health);
        }
    }
}

TEST_CASE("a dead player cannot move or shoot, and enemies stop targeting") {
    GameState state(1);
    const PlayerId id = state.AddPlayer(0.0f, 0.0f);
    state.SpawnEnemy(EnemyKind::Chaser, 0.0f, 0.0f);

    // Let the chaser hit through repeated invulnerability windows until the player is dead.
    for (int i = 0; i < 60 * 10 && !state.AllPlayersDead(); ++i) {
        TickN(state, id, PlayerInput{}, 1);
    }
    REQUIRE(state.AllPlayersDead());

    const float player_x = state.Players()[0].x;
    const float enemy_x = state.Enemies()[0].x;
    const auto projectiles_before = state.Projectiles().size();
    TickN(state, id, PlayerInput{.move_x = 1.0f, .aim_x = 1.0f}, 30);

    CHECK(state.Players()[0].x == doctest::Approx(player_x));
    CHECK(state.Projectiles().size() == projectiles_before);
    CHECK(state.Enemies()[0].x == doctest::Approx(enemy_x));
}

TEST_CASE("AllPlayersDead is false with no players") {
    const GameState state(1);
    CHECK_FALSE(state.AllPlayersDead());
}

TEST_CASE("the initial wave spawns the configured count away from the player") {
    EnemyConfig config;
    config.wave.count = 12;
    config.wave.min_spawn_distance = 200.0f;
    GameState state(7, config);
    state.AddPlayer(0.0f, 0.0f);
    state.SpawnWave();

    REQUIRE(state.Enemies().size() == 12);
    for (const auto& enemy : state.Enemies()) {
        CHECK(Distance(enemy.x, enemy.y, 0.0f, 0.0f) >= 200.0f);
    }
}

TEST_CASE("the initial wave is deterministic per seed and differs between seeds") {
    auto wave = [](std::uint32_t seed) {
        GameState state(seed);
        state.AddPlayer(0.0f, 0.0f);
        state.SpawnWave();
        return std::vector(state.Enemies());
    };

    const auto a = wave(42);
    const auto b = wave(42);
    const auto c = wave(43);

    REQUIRE(a.size() == b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        CHECK(a[i].kind == b[i].kind);
        CHECK(a[i].x == doctest::Approx(b[i].x));
        CHECK(a[i].y == doctest::Approx(b[i].y));
    }

    bool differs = a.size() != c.size();
    for (std::size_t i = 0; !differs && i < a.size(); ++i) {
        differs = a[i].kind != c[i].kind || a[i].x != c[i].x || a[i].y != c[i].y;
    }
    CHECK(differs);
}

TEST_CASE("enemies do not overlap each other, even when spawned on the same spot") {
    GameState state(1);
    const PlayerId id = state.AddPlayer(0.0f, 0.0f);
    for (int i = 0; i < 5; ++i) {
        state.SpawnEnemy(EnemyKind::Chaser, 200.0f, 100.0f);
    }

    TickN(state, id, PlayerInput{}, 120);

    const auto& enemies = state.Enemies();
    for (std::size_t i = 0; i < enemies.size(); ++i) {
        for (std::size_t j = i + 1; j < enemies.size(); ++j) {
            const float reach = enemies[i].stats.radius + enemies[j].stats.radius;
            CHECK(Distance(enemies[i].x, enemies[i].y, enemies[j].x, enemies[j].y) >= reach - 2.0f);
        }
    }
}
