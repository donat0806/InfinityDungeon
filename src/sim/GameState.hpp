#pragma once

#include "input/PlayerInput.hpp"
#include "sim/Arena.hpp"
#include "sim/DungeonSimulator.hpp"
#include "sim/PlayerStats.hpp"

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
};

struct Projectile {
    ProjectileId id;
    PlayerId owner;
    float x = 0.0f;
    float y = 0.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float radius = 0.0f;
    float remaining_life = 0.0f; // seconds
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
    explicit GameState(std::uint32_t seed);

    PlayerId AddPlayer(float x, float y);

    void Tick(float fixed_dt, const std::unordered_map<PlayerId, input::PlayerInput>& inputs);

    [[nodiscard]] std::uint32_t Depth() const { return depth_; }
    [[nodiscard]] const std::vector<Room>& Rooms() const { return rooms_; }
    [[nodiscard]] const std::vector<Player>& Players() const { return players_; }
    [[nodiscard]] const std::vector<Projectile>& Projectiles() const { return projectiles_; }
    [[nodiscard]] const Arena& ArenaBounds() const { return arena_; }

private:
    void TickPlayers(float fixed_dt, const std::unordered_map<PlayerId, input::PlayerInput>& inputs);
    void TickProjectiles(float fixed_dt);

    std::uint32_t seed_;
    std::uint32_t depth_ = 1;
    DungeonSimulator simulator_;
    std::vector<Room> rooms_;
    Arena arena_;
    std::vector<Player> players_;
    std::vector<Projectile> projectiles_;
    PlayerId next_player_id_ = 0;
    ProjectileId next_projectile_id_ = 0;
};

} // namespace infinity_dungeon::sim
