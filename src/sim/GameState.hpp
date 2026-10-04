#pragma once

#include "input/PlayerInput.hpp"
#include "sim/Arena.hpp"
#include "sim/Enemy.hpp"
#include "sim/EnemyConfig.hpp"
#include "sim/Floor.hpp"
#include "sim/FloorConfig.hpp"
#include "sim/PlayerStats.hpp"
#include "sim/Rng.hpp"

#include <cstdint>
#include <optional>
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
// floor and room, and the players, addressed by ID rather than by pointer or index
// so entities can be added, removed, or sent over the network later
// without invalidating references held elsewhere.
//
// No raylib here; this is advanced only by Tick(), at a fixed step, so the
// same seed and the same sequence of inputs always produce the same run.
class GameState {
public:
    explicit GameState(std::uint32_t seed, EnemyConfig config = EnemyConfig{},
                       FloorConfig floor_config = FloorConfig{});

    PlayerId AddPlayer(float x, float y);

    // Adds one enemy of the given kind. Patrollers get a seeded diagonal heading.
    EnemyId SpawnEnemy(EnemyKind kind, float x, float y);

    // Spawns config.wave.count enemies in the current room at seeded positions
    // away from the players. Happens automatically on first entry to a room
    // other than the start room; also callable directly (e.g. by tests).
    void SpawnWave();

    void Tick(float fixed_dt, const std::unordered_map<PlayerId, input::PlayerInput>& inputs);

    [[nodiscard]] std::uint32_t Depth() const { return depth_; }
    [[nodiscard]] const Floor& CurrentFloor() const { return floor_; }
    [[nodiscard]] RoomId CurrentRoomId() const { return current_room_; }
    [[nodiscard]] const Room& CurrentRoom() const { return floor_.rooms[current_room_]; }
    [[nodiscard]] bool Visited(RoomId id) const { return visited_[id]; }
    // The exit hatch is usable once the exit room has been cleared.
    [[nodiscard]] bool HatchOpen() const { return current_room_ == floor_.exit && enemies_.empty(); }
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
    // Keeps a player inside the room except through doors; returns the side
    // whose door the player's center has crossed, if any.
    std::optional<Direction> ConstrainToRoom(Player& player) const;
    void EnterRoom(Direction through);
    void StartFloor();
    void CheckHatch();

    std::uint32_t seed_;
    std::uint32_t depth_ = 1;
    FloorConfig floor_config_;
    Floor floor_;
    RoomId current_room_ = 0;
    std::vector<bool> visited_;
    std::vector<std::vector<Enemy>> stashed_enemies_; // per room, for rooms left mid-fight
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
