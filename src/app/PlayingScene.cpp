#include "PlayingScene.hpp"

#include <raylib.h>

#include <unordered_map>

namespace infinity_dungeon::app {

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

    for (const auto& player : state_.Players()) {
        const int x = 640 + static_cast<int>(player.x);
        const int y = 360 + static_cast<int>(player.y);
        DrawCircle(x, y, 12, GREEN);
    }
}

} // namespace infinity_dungeon::app
