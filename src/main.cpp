#include "input/PlayerInput.hpp"
#include "render/Renderer.hpp"
#include "sim/DungeonSimulator.hpp"

#include <cstdint>

int main() {
    constexpr std::uint32_t seed = 1337;
    constexpr std::uint32_t depth = 1;

    const infinity_dungeon::sim::DungeonSimulator simulator(seed);
    const auto rooms = simulator.GenerateLevel(depth);

    const infinity_dungeon::render::Renderer renderer(1280, 720, "InfinityDungeon");
    while (!renderer.ShouldClose()) {
        [[maybe_unused]] const auto input = infinity_dungeon::input::ReadKeyboard();
        renderer.DrawFrame(depth, rooms.size());
    }

    return 0;
}
