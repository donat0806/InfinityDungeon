#pragma once

#include "sim/Floor.hpp"
#include "sim/FloorConfig.hpp"

#include <cstdint>

namespace infinity_dungeon::sim {

// Builds the room layout for one floor. Pure: the same (seed, depth, config)
// always gives the same floor, and each depth draws from its own RNG stream.
// Rooms form a tree on a grid (start at the grid center); the exit is the
// dead end farthest from the start.
Floor GenerateFloor(std::uint32_t seed, std::uint32_t depth, const FloorConfig& config);

} // namespace infinity_dungeon::sim
