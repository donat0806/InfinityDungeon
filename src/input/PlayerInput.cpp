#include "PlayerInput.hpp"

#include <raylib.h>

namespace infinity_dungeon::input {

PlayerInput ReadKeyboard() {
    PlayerInput input;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) input.move_x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) input.move_x += 1.0f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) input.move_y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) input.move_y += 1.0f;
    input.attack = IsKeyDown(KEY_SPACE);
    return input;
}

} // namespace infinity_dungeon::input
