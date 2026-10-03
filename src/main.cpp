#include "input/PlayerInput.hpp"
#include "net/LeaderboardClient.hpp"
#include "render/Renderer.hpp"
#include "sim/DungeonSimulator.hpp"

#include <cstdint>
#include <iostream>

int main() {
    constexpr std::uint32_t seed = 1337;
    constexpr std::uint32_t depth = 1;

    infinity_dungeon::sim::DungeonSimulator simulator(seed);
    const auto rooms = simulator.GenerateLevel(depth);

    infinity_dungeon::render::Renderer renderer;
    renderer.DrawFrame(depth);

    infinity_dungeon::net::LeaderboardClient leaderboard("http://localhost:8080");

    std::cout << "Generated rooms: " << rooms.size() << '\n';
    std::cout << "Sample payload: " << leaderboard.BuildSubmitPayload("player", depth) << '\n';

    return 0;
}
