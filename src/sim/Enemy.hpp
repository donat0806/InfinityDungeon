#pragma once

#include <cstdint>

namespace infinity_dungeon::sim {

using EnemyId = std::uint32_t;

enum class EnemyKind : std::uint8_t { Chaser, Shooter, Patroller };
inline constexpr std::uint32_t kEnemyKindCount = 3;

// Per-enemy tunables, copied onto each Enemy so depth scaling can adjust them
// per enemy later. Loaded from config/enemies.json (see EnemyConfig).
struct EnemyStats {
    int max_health = 3;
    float move_speed = 90.0f;           // units per second
    float radius = 14.0f;
    int contact_damage = 1;
    float preferred_distance = 250.0f;  // shooters keep roughly this far from their target
    float fire_interval = 1.5f;         // seconds between shots (shooters)
    float projectile_speed = 220.0f;    // units per second
    int projectile_damage = 1;
    float projectile_lifetime = 3.0f;   // seconds
    float projectile_radius = 6.0f;
};

struct Enemy {
    EnemyId id;
    EnemyKind kind;
    float x = 0.0f;
    float y = 0.0f;
    float dir_x = 0.0f; // patroller heading (unit vector); unused by other kinds
    float dir_y = 0.0f;
    int health = 0;
    float fire_cooldown = 0.0f; // seconds until a shooter may fire again
    EnemyStats stats{};
};

} // namespace infinity_dungeon::sim
