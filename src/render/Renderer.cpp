#include "Renderer.hpp"

#include <iostream>

namespace infinity_dungeon::render {

void Renderer::DrawFrame(std::uint32_t depth) const {
    std::cout << "[render] depth=" << depth << '\n';
}

} // namespace infinity_dungeon::render
