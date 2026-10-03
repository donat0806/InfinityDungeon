#pragma once

#include <cstdint>
#include <vector>

namespace infinity_dungeon::sim {

struct Room {
    std::uint32_t id;
    std::uint32_t difficulty;
};

class DungeonSimulator {
public:
    explicit DungeonSimulator(std::uint32_t seed);
    std::vector<Room> GenerateLevel(std::uint32_t depth) const;

private:
    std::uint32_t seed_;
};

} // namespace infinity_dungeon::sim
