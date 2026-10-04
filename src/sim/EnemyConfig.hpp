#pragma once

#include "sim/Enemy.hpp"

#include <filesystem>
#include <string_view>

namespace infinity_dungeon::sim {

struct WaveConfig {
    int count = 6;
    float min_spawn_distance = 200.0f; // enemies spawn at least this far from any player
};

// Enemy balance data. The member defaults are the built-in fallback, so the
// sim and its tests work without any file; config/enemies.json overrides them.
struct EnemyConfig {
    EnemyStats chaser{.max_health = 3, .move_speed = 90.0f, .radius = 14.0f, .contact_damage = 1};
    EnemyStats shooter{.max_health = 2,
                       .move_speed = 60.0f,
                       .radius = 12.0f,
                       .contact_damage = 1,
                       .preferred_distance = 250.0f,
                       .fire_interval = 1.5f,
                       .projectile_speed = 220.0f,
                       .projectile_damage = 1,
                       .projectile_lifetime = 3.0f,
                       .projectile_radius = 6.0f};
    EnemyStats patroller{.max_health = 4, .move_speed = 130.0f, .radius = 16.0f, .contact_damage = 1};
    WaveConfig wave{};

    [[nodiscard]] const EnemyStats& For(EnemyKind kind) const;
};

// Parses enemy config JSON. Missing fields keep their defaults; malformed JSON
// or fields of the wrong type throw std::runtime_error.
EnemyConfig ParseEnemyConfig(std::string_view json);

// Reads and parses a config file; throws std::runtime_error if it can't be read.
EnemyConfig LoadEnemyConfig(const std::filesystem::path& path);

} // namespace infinity_dungeon::sim
