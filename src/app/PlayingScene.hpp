#pragma once

#include "app/Scene.hpp"
#include "sim/EnemyConfig.hpp"
#include "sim/GameState.hpp"

#include <cstdint>

namespace infinity_dungeon::app {

// Drives one run: ticks the deterministic GameState at the fixed step and
// draws its current contents (arena, players, enemies, projectiles). Returns
// to the menu once every player is dead. Depth progression is filled in by
// later roadmap items.
class PlayingScene final : public Scene {
public:
    PlayingScene(std::uint32_t seed, sim::EnemyConfig enemy_config);

    void OnEnter() override;
    std::unique_ptr<Scene> Tick(float fixed_dt, const input::PlayerInput& input) override;
    void Draw(const render::Renderer& renderer) const override;

private:
    std::uint32_t seed_;
    sim::EnemyConfig enemy_config_;
    sim::GameState state_;
    sim::PlayerId local_player_id_ = 0;
};

} // namespace infinity_dungeon::app
