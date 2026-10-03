#include "DungeonSimulator.hpp"

namespace infinity_dungeon::sim {

DungeonSimulator::DungeonSimulator(std::uint32_t seed) : seed_(seed) {}

std::vector<Room> DungeonSimulator::GenerateLevel(std::uint32_t depth) const {
    std::vector<Room> rooms;
    rooms.reserve(4);

    std::uint32_t state = seed_ ^ (depth * 2654435761u);
    for (std::uint32_t i = 0; i < 4; ++i) {
        state = state * 1664525u + 1013904223u;
        rooms.push_back(Room{(depth * 10u) + i, 1u + (state % (depth + 2u))});
    }

    return rooms;
}

} // namespace infinity_dungeon::sim
