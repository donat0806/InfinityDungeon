#pragma once

namespace infinity_dungeon::sim {

// The walkable rectangle players are confined to, in world units centered on
// the origin. A stand-in for real rooms until floor generation lands.
struct Arena {
    float min_x = -480.0f;
    float min_y = -260.0f;
    float max_x = 480.0f;
    float max_y = 260.0f;
};

} // namespace infinity_dungeon::sim
