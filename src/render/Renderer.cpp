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

void Renderer::BeginFrame() const {
    BeginDrawing();
    ClearBackground(BLACK);
}

void Renderer::EndFrame() const {
    EndDrawing();
}

} // namespace infinity_dungeon::render
