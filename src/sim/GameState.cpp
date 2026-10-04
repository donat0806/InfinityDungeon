#include "GameState.hpp"

#include <algorithm>
#include <cmath>

namespace infinity_dungeon::sim {

namespace {

constexpr int kSpawnAttempts = 20;
// Shooters hold position while within this distance of their preferred range.
constexpr float kShooterBand = 20.0f;

// Scales (x, y) down to unit length if it is longer, so diagonal input is no
// faster than straight input. Shorter vectors (analog sticks) are kept as is.
void ClampLength(float& x, float& y) {
    const float length = std::sqrt(x * x + y * y);
    if (length > 1.0f) {
        x /= length;
        y /= length;
    }
}

void ClampToArena(const Arena& arena, float radius, float& x, float& y) {
    x = std::clamp(x, arena.min_x + radius, arena.max_x - radius);
    y = std::clamp(y, arena.min_y + radius, arena.max_y - radius);
}

bool Overlaps(float ax, float ay, float ar, float bx, float by, float br) {
    const float dx = ax - bx;
    const float dy = ay - by;
    const float reach = ar + br;
    return dx * dx + dy * dy <= reach * reach;
}

// The living player closest to (x, y); ties go to the earliest in `players`.
const Player* NearestLivingPlayer(const std::vector<Player>& players, float x, float y) {
    const Player* best = nullptr;
    float best_dist2 = 0.0f;
    for (const auto& player : players) {
        if (!player.Alive()) {
            continue;
        }
        const float dx = player.x - x;
        const float dy = player.y - y;
        const float dist2 = dx * dx + dy * dy;
        if (best == nullptr || dist2 < best_dist2) {
            best = &player;
            best_dist2 = dist2;
        }
    }
    return best;
}

void DamagePlayer(Player& player, int damage) {
    player.health = std::max(0, player.health - damage);
    player.invulnerable_time = player.stats.invulnerability_duration;
}

} // namespace

GameState::GameState(std::uint32_t seed, EnemyConfig config)
    : seed_(seed),
      simulator_(seed),
      rooms_(simulator_.GenerateLevel(depth_)),
      rng_(seed ^ 0x9E3779B9u, 77u),
      config_(config) {}

PlayerId GameState::AddPlayer(float x, float y) {
    const PlayerId id = next_player_id_++;
    Player player{.id = id, .x = x, .y = y};
    player.health = player.stats.max_health;
    players_.push_back(player);
    return id;
}

EnemyId GameState::SpawnEnemy(EnemyKind kind, float x, float y) {
    const EnemyStats& stats = config_.For(kind);
    Enemy enemy{.id = next_enemy_id_++, .kind = kind, .x = x, .y = y};
    enemy.health = stats.max_health;
    enemy.fire_cooldown = stats.fire_interval;
    enemy.stats = stats;
    if (kind == EnemyKind::Patroller) {
        constexpr float kDiagonal = 0.70710678f;
        const std::uint32_t bits = rng_.NextU32();
        enemy.dir_x = (bits & 1u) != 0 ? kDiagonal : -kDiagonal;
        enemy.dir_y = (bits & 2u) != 0 ? kDiagonal : -kDiagonal;
    }
    enemies_.push_back(enemy);
    return enemy.id;
}

void GameState::SpawnInitialWave() {
    for (int i = 0; i < config_.wave.count; ++i) {
        const auto kind = static_cast<EnemyKind>(rng_.NextIndex(kEnemyKindCount));
        const float radius = config_.For(kind).radius;

        float x = 0.0f;
        float y = 0.0f;
        for (int attempt = 0; attempt < kSpawnAttempts; ++attempt) {
            x = rng_.NextFloat(arena_.min_x + radius, arena_.max_x - radius);
            y = rng_.NextFloat(arena_.min_y + radius, arena_.max_y - radius);
            const bool too_close = std::any_of(players_.begin(), players_.end(), [&](const Player& player) {
                return Overlaps(x, y, config_.wave.min_spawn_distance, player.x, player.y, 0.0f);
            });
            if (!too_close) {
                break;
            }
        }
        SpawnEnemy(kind, x, y);
    }
}

bool GameState::AllPlayersDead() const {
    return !players_.empty() && std::none_of(players_.begin(), players_.end(), [](const Player& p) { return p.Alive(); });
}

void GameState::Tick(float fixed_dt, const std::unordered_map<PlayerId, input::PlayerInput>& inputs) {
    TickPlayers(fixed_dt, inputs);
    TickEnemies(fixed_dt);
    TickProjectiles(fixed_dt);
    ResolveCollisions();
}

void GameState::TickPlayers(float fixed_dt, const std::unordered_map<PlayerId, input::PlayerInput>& inputs) {
    // Iterate players_ (not the input map) so the order, and therefore the
    // order projectiles are spawned in, is deterministic.
    for (auto& player : players_) {
        player.fire_cooldown = std::max(0.0f, player.fire_cooldown - fixed_dt);
        player.invulnerable_time = std::max(0.0f, player.invulnerable_time - fixed_dt);
        if (!player.Alive()) {
            continue;
        }

        const auto it = inputs.find(player.id);
        if (it == inputs.end()) {
            continue;
        }
        const auto& input = it->second;
        const auto& stats = player.stats;

        float move_x = input.move_x;
        float move_y = input.move_y;
        ClampLength(move_x, move_y);
        player.x += move_x * stats.move_speed * fixed_dt;
        player.y += move_y * stats.move_speed * fixed_dt;
        ClampToArena(arena_, stats.radius, player.x, player.y);

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
            .faction = Faction::Player,
            .damage = stats.projectile_damage,
        });
        player.fire_cooldown = stats.fire_interval;
    }
}

void GameState::TickEnemies(float fixed_dt) {
    for (auto& enemy : enemies_) {
        const auto& stats = enemy.stats;
        const float step = stats.move_speed * fixed_dt;

        if (enemy.kind == EnemyKind::Patroller) {
            enemy.x += enemy.dir_x * step;
            enemy.y += enemy.dir_y * step;
            if (enemy.x - stats.radius < arena_.min_x) {
                enemy.dir_x = std::abs(enemy.dir_x);
            } else if (enemy.x + stats.radius > arena_.max_x) {
                enemy.dir_x = -std::abs(enemy.dir_x);
            }
            if (enemy.y - stats.radius < arena_.min_y) {
                enemy.dir_y = std::abs(enemy.dir_y);
            } else if (enemy.y + stats.radius > arena_.max_y) {
                enemy.dir_y = -std::abs(enemy.dir_y);
            }
            ClampToArena(arena_, stats.radius, enemy.x, enemy.y);
            continue;
        }

        enemy.fire_cooldown = std::max(0.0f, enemy.fire_cooldown - fixed_dt);

        const Player* target = NearestLivingPlayer(players_, enemy.x, enemy.y);
        if (target == nullptr) {
            continue;
        }
        const float dx = target->x - enemy.x;
        const float dy = target->y - enemy.y;
        const float dist = std::sqrt(dx * dx + dy * dy);
        if (dist <= 0.0f) {
            continue;
        }
        const float dir_x = dx / dist;
        const float dir_y = dy / dist;

        if (enemy.kind == EnemyKind::Chaser) {
            enemy.x += dir_x * step;
            enemy.y += dir_y * step;
        } else { // Shooter
            if (dist > stats.preferred_distance + kShooterBand) {
                enemy.x += dir_x * step;
                enemy.y += dir_y * step;
            } else if (dist < stats.preferred_distance - kShooterBand) {
                enemy.x -= dir_x * step;
                enemy.y -= dir_y * step;
            }
            if (enemy.fire_cooldown <= 0.0f) {
                projectiles_.push_back(Projectile{
                    .id = next_projectile_id_++,
                    .owner = enemy.id,
                    .x = enemy.x + dir_x * stats.radius,
                    .y = enemy.y + dir_y * stats.radius,
                    .vx = dir_x * stats.projectile_speed,
                    .vy = dir_y * stats.projectile_speed,
                    .radius = stats.projectile_radius,
                    .remaining_life = stats.projectile_lifetime,
                    .faction = Faction::Enemy,
                    .damage = stats.projectile_damage,
                });
                enemy.fire_cooldown = stats.fire_interval;
            }
        }
        ClampToArena(arena_, stats.radius, enemy.x, enemy.y);
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

void GameState::ResolveCollisions() {
    // A projectile that hits something is spent by zeroing its life; spent
    // projectiles and dead enemies are erased once all hits are resolved.
    for (auto& projectile : projectiles_) {
        if (projectile.faction == Faction::Player) {
            for (auto& enemy : enemies_) {
                if (enemy.health > 0 &&
                    Overlaps(projectile.x, projectile.y, projectile.radius, enemy.x, enemy.y, enemy.stats.radius)) {
                    enemy.health -= projectile.damage;
                    projectile.remaining_life = 0.0f;
                    break;
                }
            }
        } else {
            for (auto& player : players_) {
                if (player.Alive() && player.invulnerable_time <= 0.0f &&
                    Overlaps(projectile.x, projectile.y, projectile.radius, player.x, player.y, player.stats.radius)) {
                    DamagePlayer(player, projectile.damage);
                    projectile.remaining_life = 0.0f;
                    break;
                }
            }
        }
    }

    for (const auto& enemy : enemies_) {
        if (enemy.health <= 0) {
            continue;
        }
        for (auto& player : players_) {
            if (player.Alive() && player.invulnerable_time <= 0.0f &&
                Overlaps(enemy.x, enemy.y, enemy.stats.radius, player.x, player.y, player.stats.radius)) {
                DamagePlayer(player, enemy.stats.contact_damage);
            }
        }
    }

    std::erase_if(projectiles_, [](const Projectile& p) { return p.remaining_life <= 0.0f; });
    std::erase_if(enemies_, [](const Enemy& e) { return e.health <= 0; });
}

} // namespace infinity_dungeon::sim
