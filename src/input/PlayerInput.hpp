#pragma once

namespace infinity_dungeon::input {

// One player's intent for one tick. Move and aim are separate vectors
// (twin-stick style), so a gamepad's two sticks or a network packet can fill
// this as easily as the keyboard.
struct PlayerInput {
    float move_x = 0.0f; // each axis in [-1, 1]
    float move_y = 0.0f;
    float aim_x = 0.0f;  // each axis in [-1, 1]; (0, 0) means not shooting
    float aim_y = 0.0f;
    bool confirm = false; // menu actions
};

// Samples the keyboard: WASD to move, arrow keys to shoot, Space/Enter to
// confirm.
PlayerInput ReadKeyboard();

} // namespace infinity_dungeon::input
