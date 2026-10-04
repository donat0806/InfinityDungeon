#include "PlayingScene.hpp"

#include "app/MenuScene.hpp"

#include <raylib.h>

#include <unordered_map>

namespace infinity_dungeon::app {

namespace {

// World origin sits at the center of the 1280x720 window.
constexpr float kScreenCenterX = 640.0f;
constexpr float kScreenCenterY = 360.0f;

Vector2 WorldToScreen(float x, float y) {
    return Vector2{kScreenCenterX + x, kScreenCenterY + y};
}

Color EnemyColor(sim::EnemyKind kind) {
    switch (kind) {
    case sim::EnemyKind::Shooter:
        return ORANGE;
    case sim::EnemyKind::Patroller:
        return PURPLE;
    case sim::EnemyKind::Chaser:
    default:
        return RED;
    }
}

} // namespace

PlayingScene::PlayingScene(std::uint32_t seed, sim::EnemyConfig enemy_config)
    : seed_(seed), enemy_config_(enemy_config), state_(seed, enemy_config) {}

void PlayingScene::OnEnter() {
    local_player_id_ = state_.AddPlayer(0.0f, 0.0f);
    state_.SpawnInitialWave();
}

std::unique_ptr<Scene> PlayingScene::Tick(float fixed_dt, const input::PlayerInput& input) {
    const std::unordered_map<sim::PlayerId, input::PlayerInput> inputs{{local_player_id_, input}};
    state_.Tick(fixed_dt, inputs);
    if (state_.AllPlayersDead()) {
        return std::make_unique<MenuScene>(seed_, enemy_config_);
    }
    return nullptr;
}

void PlayingScene::Draw(const render::Renderer& /*renderer*/) const {
    DrawText(TextFormat("Depth %u", state_.Depth()), 20, 20, 32, RAYWHITE);
    DrawText(TextFormat("Rooms: %d", static_cast<int>(state_.Rooms().size())), 20, 60, 20, GRAY);
    DrawText("WASD move - Arrows shoot", 1000, 20, 20, GRAY);

    for (const auto& player : state_.Players()) {
        if (player.id == local_player_id_) {
            DrawText(TextFormat("HP %d/%d", player.health, player.stats.max_health), 20, 90, 24, GREEN);
        }
    }
    if (state_.Enemies().empty()) {
        DrawText("Arena cleared", 20, 120, 24, GOLD);
    }

    const auto& arena = state_.ArenaBounds();
    const Vector2 arena_min = WorldToScreen(arena.min_x, arena.min_y);
    DrawRectangleLinesEx(Rectangle{arena_min.x, arena_min.y, arena.max_x - arena.min_x, arena.max_y - arena.min_y}, 2.0f,
                         DARKGRAY);

    for (const auto& enemy : state_.Enemies()) {
        DrawCircleV(WorldToScreen(enemy.x, enemy.y), enemy.stats.radius, EnemyColor(enemy.kind));
    }

    for (const auto& projectile : state_.Projectiles()) {
        const Color color = projectile.faction == sim::Faction::Player ? SKYBLUE : MAROON;
        DrawCircleV(WorldToScreen(projectile.x, projectile.y), projectile.radius, color);
    }

    for (const auto& player : state_.Players()) {
        if (!player.Alive()) {
            continue;
        }
        // Blink while invulnerable after a hit.
        const bool blink_off = player.invulnerable_time > 0.0f && static_cast<int>(player.invulnerable_time * 10.0f) % 2 == 0;
        if (!blink_off) {
            DrawCircleV(WorldToScreen(player.x, player.y), player.stats.radius, GREEN);
        }
    }
}

} // namespace infinity_dungeon::app
