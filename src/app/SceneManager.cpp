#include "SceneManager.hpp"

#include <utility>

namespace infinity_dungeon::app {

SceneManager::SceneManager(std::unique_ptr<Scene> initial) : current_(std::move(initial)) {
    current_->OnEnter();
}

void SceneManager::Tick(float fixed_dt, const input::PlayerInput& input) {
    auto next = current_->Tick(fixed_dt, input);
    if (next != nullptr) {
        current_->OnExit();
        current_ = std::move(next);
        current_->OnEnter();
    }
}

void SceneManager::Draw(const render::Renderer& renderer) const {
    current_->Draw(renderer);
}

} // namespace infinity_dungeon::app
