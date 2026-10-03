#include "PlayerInput.hpp"

#include <raylib.h>

namespace infinity_dungeon::input {

PlayerInput ReadKeyboard() {
    PlayerInput input;
    if (IsKeyDown(KEY_A)) input.move_x -= 1.0f;
    if (IsKeyDown(KEY_D)) input.move_x += 1.0f;
    if (IsKeyDown(KEY_W)) input.move_y -= 1.0f;
    if (IsKeyDown(KEY_S)) input.move_y += 1.0f;
    if (IsKeyDown(KEY_LEFT)) input.aim_x -= 1.0f;
    if (IsKeyDown(KEY_RIGHT)) input.aim_x += 1.0f;
    if (IsKeyDown(KEY_UP)) input.aim_y -= 1.0f;
    if (IsKeyDown(KEY_DOWN)) input.aim_y += 1.0f;
    input.confirm = IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_ENTER);
    return input;
}

} // namespace infinity_dungeon::input
