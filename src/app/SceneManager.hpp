#pragma once

#include "app/Scene.hpp"
#include "input/PlayerInput.hpp"

#include <memory>

namespace infinity_dungeon::render {
class Renderer;
}

namespace infinity_dungeon::app {

// Owns the current scene and swaps it out when the scene itself requests a
// transition, calling OnExit()/OnEnter() around the switch.
class SceneManager {
public:
    explicit SceneManager(std::unique_ptr<Scene> initial);

    void Tick(float fixed_dt, const input::PlayerInput& input);
    void Draw(const render::Renderer& renderer) const;

private:
    std::unique_ptr<Scene> current_;
};

} // namespace infinity_dungeon::app
