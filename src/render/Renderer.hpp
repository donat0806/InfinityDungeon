#pragma once

#include <cstdint>

namespace infinity_dungeon::render {

class Renderer {
public:
    void DrawFrame(std::uint32_t depth) const;
};

} // namespace infinity_dungeon::render
