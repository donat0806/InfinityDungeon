#pragma once

#include "app/Scene.hpp"
#include "sim/GameState.hpp"

#include <cstdint>

namespace infinity_dungeon::app {

// Drives one run: ticks the deterministic GameState at the fixed step and
// draws its current contents (arena, players, projectiles). Enemies and
// depth progression are filled in by later roadmap items.
class PlayingScene final : public Scene {
public:
    explicit PlayingScene(std::uint32_t seed);

    void OnEnter() override;
    std::unique_ptr<Scene> Tick(float fixed_dt, const input::PlayerInput& input) override;
    void Draw(const render::Renderer& renderer) const override;

private:
    sim::GameState state_;
    sim::PlayerId local_player_id_ = 0;
};

} // namespace infinity_dungeon::app
