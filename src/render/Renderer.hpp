#pragma once

#include <cstddef>
#include <cstdint>

namespace infinity_dungeon::render {

// Owns the raylib window for its lifetime.
class Renderer {
public:
    Renderer(int width, int height, const char* title);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    [[nodiscard]] bool ShouldClose() const;
    void DrawFrame(std::uint32_t depth, std::size_t room_count) const;
};

} // namespace infinity_dungeon::render
