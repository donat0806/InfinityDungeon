#include "GameState.hpp"

#include <algorithm>
#include <cmath>

namespace infinity_dungeon::sim {

namespace {

// Scales (x, y) down to unit length if it is longer, so diagonal input is no
// faster than straight input. Shorter vectors (analog sticks) are kept as is.
void ClampLength(float& x, float& y) {
    const float length = std::sqrt(x * x + y * y);
    if (length > 1.0f) {
        x /= length;
        y /= length;
    }
}

} // namespace

GameState::GameState(std::uint32_t seed) : seed_(seed), simulator_(seed), rooms_(simulator_.GenerateLevel(depth_)) {}

PlayerId GameState::AddPlayer(float x, float y) {
    const PlayerId id = next_player_id_++;
    players_.push_back(Player{.id = id, .x = x, .y = y});
    return id;
}

void GameState::Tick(float fixed_dt, const std::unordered_map<PlayerId, input::PlayerInput>& inputs) {
    TickPlayers(fixed_dt, inputs);
    TickProjectiles(fixed_dt);
}

void GameState::TickPlayers(float fixed_dt, const std::unordered_map<PlayerId, input::PlayerInput>& inputs) {
    // Iterate players_ (not the input map) so the order, and therefore the
    // order projectiles are spawned in, is deterministic.
    for (auto& player : players_) {
        player.fire_cooldown = std::max(0.0f, player.fire_cooldown - fixed_dt);

        const auto it = inputs.find(player.id);
        if (it == inputs.end()) {
            continue;
        }
        const auto& input = it->second;
        const auto& stats = player.stats;

        float move_x = input.move_x;
        float move_y = input.move_y;
        ClampLength(move_x, move_y);
        player.x = std::clamp(player.x + move_x * stats.move_speed * fixed_dt, arena_.min_x + stats.radius,
                              arena_.max_x - stats.radius);
        player.y = std::clamp(player.y + move_y * stats.move_speed * fixed_dt, arena_.min_y + stats.radius,
                              arena_.max_y - stats.radius);

        if ((input.aim_x == 0.0f && input.aim_y == 0.0f) || player.fire_cooldown > 0.0f) {
            continue;
        }
        float aim_x = input.aim_x;
        float aim_y = input.aim_y;
        const float aim_length = std::sqrt(aim_x * aim_x + aim_y * aim_y);
        aim_x /= aim_length;
        aim_y /= aim_length;

        projectiles_.push_back(Projectile{
            .id = next_projectile_id_++,
            .owner = player.id,
            .x = player.x + aim_x * stats.radius,
            .y = player.y + aim_y * stats.radius,
            .vx = aim_x * stats.projectile_speed,
            .vy = aim_y * stats.projectile_speed,
            .radius = stats.projectile_radius,
            .remaining_life = stats.projectile_lifetime,
        });
        player.fire_cooldown = stats.fire_interval;
    }
}

void GameState::TickProjectiles(float fixed_dt) {
    for (auto& projectile : projectiles_) {
        projectile.x += projectile.vx * fixed_dt;
        projectile.y += projectile.vy * fixed_dt;
        projectile.remaining_life -= fixed_dt;
    }

    std::erase_if(projectiles_, [this](const Projectile& p) {
        return p.remaining_life <= 0.0f || p.x - p.radius < arena_.min_x || p.x + p.radius > arena_.max_x ||
               p.y - p.radius < arena_.min_y || p.y + p.radius > arena_.max_y;
    });
}

} // namespace infinity_dungeon::sim
