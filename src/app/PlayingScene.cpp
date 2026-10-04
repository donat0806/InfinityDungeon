#include "PlayingScene.hpp"

#include "app/MenuScene.hpp"

#include <raylib.h>

#include <algorithm>
#include <cmath>
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

PlayingScene::PlayingScene(std::uint32_t seed, sim::EnemyConfig enemy_config, sim::FloorConfig floor_config)
    : seed_(seed), enemy_config_(enemy_config), floor_config_(floor_config), state_(seed, enemy_config, floor_config) {}

void PlayingScene::OnEnter() {
    local_player_id_ = state_.AddPlayer(0.0f, 0.0f);
}

std::unique_ptr<Scene> PlayingScene::Tick(float fixed_dt, const input::PlayerInput& input) {
    const std::unordered_map<sim::PlayerId, input::PlayerInput> inputs{{local_player_id_, input}};
    state_.Tick(fixed_dt, inputs);
    if (state_.AllPlayersDead()) {
        return std::make_unique<MenuScene>(seed_, enemy_config_, floor_config_);
    }
    return nullptr;
}

namespace {

// Draws one wall as two segments with a gap for the door, or as a single line.
void DrawWall(Vector2 from, Vector2 to, bool has_door, float door_half, Color color) {
    if (!has_door) {
        DrawLineEx(from, to, 3.0f, color);
        return;
    }
    const Vector2 mid{(from.x + to.x) * 0.5f, (from.y + to.y) * 0.5f};
    const float length = std::sqrt((to.x - from.x) * (to.x - from.x) + (to.y - from.y) * (to.y - from.y));
    const float t = door_half / length; // fraction of the wall each side of the middle
    const Vector2 gap_a{mid.x + (from.x - to.x) * t, mid.y + (from.y - to.y) * t};
    const Vector2 gap_b{mid.x + (to.x - from.x) * t, mid.y + (to.y - from.y) * t};
    DrawLineEx(from, gap_a, 3.0f, color);
    DrawLineEx(gap_b, to, 3.0f, color);
}

} // namespace

void PlayingScene::DrawRoom() const {
    const auto& arena = state_.ArenaBounds();
    const auto& room = state_.CurrentRoom();
    const Vector2 top_left = WorldToScreen(arena.min_x, arena.min_y);
    const Vector2 top_right = WorldToScreen(arena.max_x, arena.min_y);
    const Vector2 bottom_left = WorldToScreen(arena.min_x, arena.max_y);
    const Vector2 bottom_right = WorldToScreen(arena.max_x, arena.max_y);
    const float door = arena.door_half_width;
    DrawWall(top_left, top_right, room.HasDoor(sim::Direction::North), door, GRAY);
    DrawWall(top_right, bottom_right, room.HasDoor(sim::Direction::East), door, GRAY);
    DrawWall(bottom_left, bottom_right, room.HasDoor(sim::Direction::South), door, GRAY);
    DrawWall(top_left, bottom_left, room.HasDoor(sim::Direction::West), door, GRAY);

    if (state_.CurrentRoomId() == state_.CurrentFloor().exit) {
        const Vector2 center = WorldToScreen(0.0f, 0.0f);
        const Color hatch = state_.HatchOpen() ? GOLD : DARKGRAY;
        DrawCircleV(center, arena.hatch_radius, BLACK);
        DrawCircleLinesV(center, arena.hatch_radius, hatch);
    }
}

// Shows rooms the player has visited plus the unvisited rooms behind their doors.
void PlayingScene::DrawMinimap() const {
    constexpr float kCell = 14.0f;
    constexpr float kGap = 3.0f;
    constexpr float kMargin = 20.0f;
    constexpr float kScreenWidth = 1280.0f;

    const auto& floor = state_.CurrentFloor();
    // Anchor the map's bounding box to the top-right corner so it never leaves the screen.
    int min_gx = floor.rooms.front().grid_x;
    int max_gx = min_gx;
    int min_gy = floor.rooms.front().grid_y;
    for (const auto& room : floor.rooms) {
        min_gx = std::min(min_gx, room.grid_x);
        max_gx = std::max(max_gx, room.grid_x);
        min_gy = std::min(min_gy, room.grid_y);
    }
    const float map_width = static_cast<float>(max_gx - min_gx + 1) * (kCell + kGap) - kGap;
    const float origin_x = kScreenWidth - kMargin - map_width;
    const float origin_y = kMargin;

    for (const auto& room : floor.rooms) {
        bool known = state_.Visited(room.id);
        for (std::size_t d = 0; d < sim::kDirectionCount && !known; ++d) {
            const auto next = floor.Neighbor(room.id, static_cast<sim::Direction>(d));
            known = next && state_.Visited(*next);
        }
        if (!known) {
            continue;
        }
        const Rectangle rect{origin_x + static_cast<float>(room.grid_x - min_gx) * (kCell + kGap),
                             origin_y + static_cast<float>(room.grid_y - min_gy) * (kCell + kGap), kCell, kCell};
        const bool visited = state_.Visited(room.id);
        Color color = visited ? GRAY : DARKGRAY;
        if (room.id == floor.exit && visited) {
            color = GOLD;
        }
        if (room.id == state_.CurrentRoomId()) {
            color = RAYWHITE;
        }
        if (visited) {
            DrawRectangleRec(rect, color);
        } else {
            DrawRectangleLinesEx(rect, 1.0f, color);
        }
    }
}

void PlayingScene::Draw(const render::Renderer& /*renderer*/) const {
    DrawText(TextFormat("Depth %u", state_.Depth()), 20, 20, 32, RAYWHITE);
    DrawText("WASD move - Arrows shoot", 20, 690, 20, GRAY);

    for (const auto& player : state_.Players()) {
        if (player.id == local_player_id_) {
            DrawText(TextFormat("HP %d/%d", player.health, player.stats.max_health), 20, 60, 24, GREEN);
        }
    }
    if (state_.HatchOpen()) {
        DrawText("Hatch open - step in to descend", 20, 90, 24, GOLD);
    } else if (state_.Enemies().empty()) {
        DrawText("Room cleared", 20, 90, 24, GOLD);
    }

    DrawRoom();
    DrawMinimap();

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
