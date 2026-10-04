#include "app/MenuScene.hpp"
#include "app/SceneManager.hpp"
#include "input/PlayerInput.hpp"
#include "render/Renderer.hpp"
#include "sim/EnemyConfig.hpp"
#include "sim/FixedTimestep.hpp"

#include <raylib.h>

#include <cstdint>
#include <cstdio>
#include <exception>
#include <filesystem>

namespace {

// Enemy balance lives in config/enemies.json next to the executable. A missing
// or broken file falls back to the built-in defaults so the game still starts.
infinity_dungeon::sim::EnemyConfig LoadEnemyConfigOrDefault() {
    const auto path = std::filesystem::path(GetApplicationDirectory()) / "config" / "enemies.json";
    try {
        return infinity_dungeon::sim::LoadEnemyConfig(path);
    } catch (const std::exception& e) {
        std::fprintf(stderr, "%s; using built-in enemy defaults\n", e.what());
        return infinity_dungeon::sim::EnemyConfig{};
    }
}

} // namespace

int main() {
    constexpr std::uint32_t seed = 1337;

    infinity_dungeon::render::Renderer renderer(1280, 720, "InfinityDungeon");
    infinity_dungeon::app::SceneManager scenes(
        std::make_unique<infinity_dungeon::app::MenuScene>(seed, LoadEnemyConfigOrDefault()));
    infinity_dungeon::sim::FixedTimestepClock clock(60.0f);

    while (!renderer.ShouldClose()) {
        const auto input = infinity_dungeon::input::ReadKeyboard();
        const int steps = clock.Advance(GetFrameTime());
        for (int i = 0; i < steps; ++i) {
            scenes.Tick(clock.FixedDeltaSeconds(), input);
        }

        renderer.BeginFrame();
        scenes.Draw(renderer);
        renderer.EndFrame();
    }

    return 0;
}
