#include "FloorConfig.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace infinity_dungeon::sim {

namespace {

using Json = nlohmann::json;

} // namespace

FloorConfig ParseFloorConfig(std::string_view json) {
    FloorConfig config;
    try {
        const Json root = Json::parse(json);
        if (!root.is_object()) {
            throw std::runtime_error("floor config: top level must be an object");
        }
        config.grid_width = root.value("grid_width", config.grid_width);
        config.grid_height = root.value("grid_height", config.grid_height);
        config.room_count = root.value("room_count", config.room_count);
    } catch (const Json::exception& e) {
        throw std::runtime_error(std::string("floor config: ") + e.what());
    }
    config.grid_width = std::max(1, config.grid_width);
    config.grid_height = std::max(1, config.grid_height);
    config.room_count = std::clamp(config.room_count, 1, config.grid_width * config.grid_height);
    return config;
}

FloorConfig LoadFloorConfig(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("floor config: cannot open " + path.string());
    }
    std::ostringstream contents;
    contents << file.rdbuf();
    return ParseFloorConfig(contents.str());
}

} // namespace infinity_dungeon::sim
