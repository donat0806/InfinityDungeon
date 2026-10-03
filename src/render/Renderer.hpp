#pragma once

namespace infinity_dungeon::render {

// Owns the raylib window for its lifetime and brackets each frame.
// Scenes draw their own content between BeginFrame() and EndFrame().
class Renderer {
public:
    Renderer(int width, int height, const char* title);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    [[nodiscard]] bool ShouldClose() const;

    void BeginFrame() const;
    void EndFrame() const;
};

} // namespace infinity_dungeon::render
