#pragma once

namespace infinity_dungeon::sim {

// Per-player tunables. Kept on each Player so items or co-op characters can
// change them individually; meant to be loaded from config files once balance
// becomes data-driven.
struct PlayerStats {
    float move_speed = 200.0f;          // units per second
    float radius = 12.0f;
    float fire_interval = 0.35f;        // seconds between shots
    float projectile_speed = 450.0f;    // units per second
    float projectile_lifetime = 0.9f;   // seconds
    float projectile_radius = 5.0f;
};

} // namespace infinity_dungeon::sim
