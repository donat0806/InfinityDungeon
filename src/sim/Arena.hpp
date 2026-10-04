#pragma once

namespace infinity_dungeon::sim {

// The walkable rectangle of every room, in world units centered on the origin.
// All rooms share one size; doors sit at the middle of each wall.
struct Arena {
    float min_x = -480.0f;
    float min_y = -260.0f;
    float max_x = 480.0f;
    float max_y = 260.0f;
    float door_half_width = 40.0f; // doors span [-half, +half] along their wall
    float hatch_radius = 24.0f;    // the exit hatch sits at the room center
};

} // namespace infinity_dungeon::sim
