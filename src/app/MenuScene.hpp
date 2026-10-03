#pragma once

#include "app/Scene.hpp"

#include <cstdint>

namespace infinity_dungeon::app {

// Idle title screen. Starts a run with a fixed seed when the player presses
// attack (space); seed selection can grow (random, seed entry, ...) later.
class MenuScene final : public Scene {
public:
    explicit MenuScene(std::uint32_t run_seed);

    std::unique_ptr<Scene> Tick(float fixed_dt, const input::PlayerInput& input) override;
    void Draw(const render::Renderer& renderer) const override;

private:
    std::uint32_t run_seed_;
};

} // namespace infinity_dungeon::app
