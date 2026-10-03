#include "Renderer.hpp"

#include <raylib.h>

namespace infinity_dungeon::render {

Renderer::Renderer(int width, int height, const char* title) {
    InitWindow(width, height, title);
    SetTargetFPS(60);
}

Renderer::~Renderer() {
    CloseWindow();
}

bool Renderer::ShouldClose() const {
    return WindowShouldClose();
}

void Renderer::DrawFrame(std::uint32_t depth, std::size_t room_count) const {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawText(TextFormat("Depth %u", depth), 20, 20, 32, RAYWHITE);
    DrawText(TextFormat("Rooms: %d", static_cast<int>(room_count)), 20, 60, 20, GRAY);
    EndDrawing();
}

} // namespace infinity_dungeon::render
