#pragma once

#include "input/PlayerInput.hpp"
#include "sim/Arena.hpp"
#include "sim/DungeonSimulator.hpp"
#include "sim/Enemy.hpp"
#include "sim/EnemyConfig.hpp"
#include "sim/PlayerStats.hpp"
#include "sim/Rng.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace infinity_dungeon::sim {

using PlayerId = std::uint32_t;
using ProjectileId = std::uint32_t;

struct Player {
    PlayerId id;
    float x = 0.0f;
    float y = 0.0f;
    PlayerStats stats{};
    float fire_cooldown = 0.0f; // seconds until the player may shoot again
    int health = PlayerStats{}.max_health;
    float invulnerable_time = 0.0f; // seconds of immunity left after a hit

    [[nodiscard]] bool Alive() const { return health > 0; }
};

enum class Faction : std::uint8_t { Player, Enemy };

struct Projectile {
    ProjectileId id;
    PlayerId owner; // PlayerId or EnemyId, depending on faction
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float radius = 0.0f;
    float remaining_life = 0.0f; // seconds
    Faction faction = Faction::Player;
    int damage = 1;
};

// Holds everything a run needs to be deterministic: the seed, the current
// level, and the players, addressed by ID rather than by pointer or index
// so entities can be added, removed, or sent over the network later
// without invalidating references held elsewhere.
//
// No raylib here; this is advanced only by Tick(), at a fixed step, so the
// same seed and the same sequence of inputs always produce the same run.
class GameState {
public:
    explicit GameState(std::uint32_t seed, EnemyConfig config = EnemyConfig{});

    PlayerId AddPlayer(float x, float y);

    // Adds one enemy of the given kind. Patrollers get a seeded diagonal heading.
    EnemyId SpawnEnemy(EnemyKind kind, float x, float y);

    // Spawns config.wave.count enemies at seeded positions away from the players.
    // Call after adding players.
    void SpawnInitialWave();

    void Tick(float fixed_dt, const std::unordered_map<PlayerId, input::PlayerInput>& inputs);

    [[nodiscard]] std::uint32_t Depth() const { return depth_; }
    [[nodiscard]] const std::vector<Room>& Rooms() const { return rooms_; }
    [[nodiscard]] const std::vector<Player>& Players() const { return players_; }
    [[nodiscard]] const std::vector<Projectile>& Projectiles() const { return projectiles_; }
    [[nodiscard]] const std::vector<Enemy>& Enemies() const { return enemies_; }
    [[nodiscard]] const Arena& ArenaBounds() const { return arena_; }

    // True when there is at least one player and none of them is alive.
    [[nodiscard]] bool AllPlayersDead() const;

private:
    void TickPlayers(float fixed_dt, const std::unordered_map<PlayerId, input::PlayerInput>& inputs);
    void TickEnemies(float fixed_dt);
    void TickProjectiles(float fixed_dt);
    void ResolveCollisions();

    std::uint32_t seed_;
    std::uint32_t depth_ = 1;
    DungeonSimulator simulator_;
    std::vector<Room> rooms_;
    Arena arena_;
    Rng rng_;
    EnemyConfig config_;
    std::vector<Player> players_;
    std::vector<Projectile> projectiles_;
    std::vector<Enemy> enemies_;
    PlayerId next_player_id_ = 0;
    ProjectileId next_projectile_id_ = 0;
    EnemyId next_enemy_id_ = 0;
};

} // namespace infinity_dungeon::sim
