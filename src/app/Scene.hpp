#pragma once

#include "input/PlayerInput.hpp"

#include <memory>

namespace infinity_dungeon::render {
class Renderer;
}

namespace infinity_dungeon::app {

// One state of the app (menu, playing, game over, ...). Owned and driven by
// SceneManager. Tick() runs at the fixed simulation step; Draw() runs once
// per render frame, between Renderer::BeginFrame()/EndFrame().
class Scene {
public:
    virtual ~Scene() = default;

    virtual void OnEnter() {}
    virtual void OnExit() {}

    // Returns the scene to switch to, or nullptr to stay on this one.
    virtual std::unique_ptr<Scene> Tick(float fixed_dt, const input::PlayerInput& input) = 0;

    virtual void Draw(const render::Renderer& renderer) const = 0;
};

} // namespace infinity_dungeon::app
