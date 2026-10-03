#include "app/MenuScene.hpp"
#include "app/SceneManager.hpp"
#include "input/PlayerInput.hpp"
#include "render/Renderer.hpp"
#include "sim/FixedTimestep.hpp"

#include <raylib.h>

#include <cstdint>

int main() {
    constexpr std::uint32_t seed = 1337;

    infinity_dungeon::render::Renderer renderer(1280, 720, "InfinityDungeon");
    infinity_dungeon::app::SceneManager scenes(std::make_unique<infinity_dungeon::app::MenuScene>(seed));
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
