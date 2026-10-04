#pragma once

#include <filesystem>
#include <string_view>

namespace infinity_dungeon::sim {

// Floor layout tunables. The member defaults are the built-in fallback, so the
// sim and its tests work without any file; config/floor.json overrides them.
struct FloorConfig {
    int grid_width = 9;
    int grid_height = 8;
    int room_count = 10;
};

// Parses floor config JSON. Missing fields keep their defaults; malformed JSON
// or fields of the wrong type throw std::runtime_error. Values are clamped to
// something generatable (grid >= 1x1, 1 <= room_count <= grid cells).
FloorConfig ParseFloorConfig(std::string_view json);

// Reads and parses a config file; throws std::runtime_error if it can't be read.
FloorConfig LoadFloorConfig(const std::filesystem::path& path);

} // namespace infinity_dungeon::sim
