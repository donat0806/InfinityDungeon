#include "EnemyConfig.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace infinity_dungeon::sim {

namespace {

using Json = nlohmann::json;

EnemyStats ParseStats(const Json& node, EnemyStats stats) {
    stats.max_health = node.value("max_health", stats.max_health);
    stats.move_speed = node.value("move_speed", stats.move_speed);
    stats.radius = node.value("radius", stats.radius);
    stats.contact_damage = node.value("contact_damage", stats.contact_damage);
    stats.preferred_distance = node.value("preferred_distance", stats.preferred_distance);
    stats.fire_interval = node.value("fire_interval", stats.fire_interval);
    stats.projectile_speed = node.value("projectile_speed", stats.projectile_speed);
    stats.projectile_damage = node.value("projectile_damage", stats.projectile_damage);
    stats.projectile_lifetime = node.value("projectile_lifetime", stats.projectile_lifetime);
    stats.projectile_radius = node.value("projectile_radius", stats.projectile_radius);
    return stats;
}

void ParseKind(const Json& enemies, const char* key, EnemyStats& stats) {
    if (enemies.contains(key)) {
        stats = ParseStats(enemies.at(key), stats);
    }
}

} // namespace

const EnemyStats& EnemyConfig::For(EnemyKind kind) const {
    switch (kind) {
    case EnemyKind::Shooter:
        return shooter;
    case EnemyKind::Patroller:
        return patroller;
    case EnemyKind::Chaser:
    default:
        return chaser;
    }
}

EnemyConfig ParseEnemyConfig(std::string_view json) {
    EnemyConfig config;
    try {
        const Json root = Json::parse(json);
        if (!root.is_object()) {
            throw std::runtime_error("enemy config: top level must be an object");
        }
        if (root.contains("enemies")) {
            const Json& enemies = root.at("enemies");
            if (!enemies.is_object()) {
                throw std::runtime_error("enemy config: \"enemies\" must be an object");
            }
            ParseKind(enemies, "chaser", config.chaser);
            ParseKind(enemies, "shooter", config.shooter);
            ParseKind(enemies, "patroller", config.patroller);
        }
        if (root.contains("wave")) {
            const Json& wave = root.at("wave");
            config.wave.count = wave.value("count", config.wave.count);
            config.wave.min_spawn_distance = wave.value("min_spawn_distance", config.wave.min_spawn_distance);
        }
    } catch (const Json::exception& e) {
        throw std::runtime_error(std::string("enemy config: ") + e.what());
    }
    return config;
}

EnemyConfig LoadEnemyConfig(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("enemy config: cannot open " + path.string());
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    return ParseEnemyConfig(contents.str());
}

} // namespace infinity_dungeon::sim
