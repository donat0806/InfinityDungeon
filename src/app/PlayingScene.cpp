#include "PlayingScene.hpp"

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

} // namespace

PlayingScene::PlayingScene(std::uint32_t seed) : state_(seed) {}

void PlayingScene::OnEnter() {
    local_player_id_ = state_.AddPlayer(0.0f, 0.0f);
}

std::unique_ptr<Scene> PlayingScene::Tick(float fixed_dt, const input::PlayerInput& input) {
    const std::unordered_map<sim::PlayerId, input::PlayerInput> inputs{{local_player_id_, input}};
    state_.Tick(fixed_dt, inputs);
    return nullptr;
}

void PlayingScene::Draw(const render::Renderer& /*renderer*/) const {
    DrawText(TextFormat("Depth %u", state_.Depth()), 20, 20, 32, RAYWHITE);
    DrawText(TextFormat("Rooms: %d", static_cast<int>(state_.Rooms().size())), 20, 60, 20, GRAY);
    DrawText("WASD move - Arrows shoot", 1000, 20, 20, GRAY);

    const auto& arena = state_.ArenaBounds();
    const Vector2 arena_min = WorldToScreen(arena.min_x, arena.min_y);
    DrawRectangleLinesEx(Rectangle{arena_min.x, arena_min.y, arena.max_x - arena.min_x, arena.max_y - arena.min_y}, 2.0f,
                         DARKGRAY);

    for (const auto& projectile : state_.Projectiles()) {
        DrawCircleV(WorldToScreen(projectile.x, projectile.y), projectile.radius, SKYBLUE);
    }

    for (const auto& player : state_.Players()) {
        DrawCircleV(WorldToScreen(player.x, player.y), player.stats.radius, GREEN);
    }
}

} // namespace infinity_dungeon::app
