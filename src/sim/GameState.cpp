#include "GameState.hpp"

namespace infinity_dungeon::sim {

GameState::GameState(std::uint32_t seed) : seed_(seed), simulator_(seed), rooms_(simulator_.GenerateLevel(depth_)) {}

PlayerId GameState::AddPlayer(float x, float y) {
    const PlayerId id = next_player_id_++;
    players_.push_back(Player{id, x, y});
    return id;
}

void GameState::Tick(float fixed_dt, const std::unordered_map<PlayerId, input::PlayerInput>& inputs) {
    for (auto& player : players_) {
        const auto it = inputs.find(player.id);
        if (it == inputs.end()) {
            continue;
        }

        const auto& input = it->second;
        player.x += input.move_x * kPlayerSpeed * fixed_dt;
        player.y += input.move_y * kPlayerSpeed * fixed_dt;
    }
}

} // namespace infinity_dungeon::sim
