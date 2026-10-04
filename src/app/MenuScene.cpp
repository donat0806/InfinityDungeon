#include "MenuScene.hpp"

#include "app/PlayingScene.hpp"

#include <raylib.h>

namespace infinity_dungeon::app {

MenuScene::MenuScene(std::uint32_t run_seed, sim::EnemyConfig enemy_config, sim::FloorConfig floor_config)
    : run_seed_(run_seed), enemy_config_(enemy_config), floor_config_(floor_config) {}

std::unique_ptr<Scene> MenuScene::Tick(float /*fixed_dt*/, const input::PlayerInput& input) {
    if (input.confirm) {
        return std::make_unique<PlayingScene>(run_seed_, enemy_config_, floor_config_);
    }
    return nullptr;
}

void MenuScene::Draw(const render::Renderer& /*renderer*/) const {
    DrawText("InfinityDungeon", 20, 20, 40, RAYWHITE);
    DrawText("Press SPACE to descend", 20, 80, 20, GRAY);
}

} // namespace infinity_dungeon::app
