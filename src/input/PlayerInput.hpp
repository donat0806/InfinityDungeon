#pragma once

namespace infinity_dungeon::input {

struct PlayerInput {
    float move_x = 0.0f;
    float move_y = 0.0f;
    bool attack = false;
};

// Samples the keyboard (WASD/arrow keys to move, Space to attack).
PlayerInput ReadKeyboard();

} // namespace infinity_dungeon::input
