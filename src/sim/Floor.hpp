#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace infinity_dungeon::sim {

using RoomId = std::uint32_t; // index into Floor::rooms

// Grid y grows downward, matching world coordinates: North is y - 1.
enum class Direction : std::uint8_t { North, East, South, West };
inline constexpr std::size_t kDirectionCount = 4;

constexpr int DirectionDx(Direction d) {
    return d == Direction::East ? 1 : (d == Direction::West ? -1 : 0);
}
constexpr int DirectionDy(Direction d) {
    return d == Direction::South ? 1 : (d == Direction::North ? -1 : 0);
}
constexpr Direction Opposite(Direction d) {
    return static_cast<Direction>((static_cast<int>(d) + 2) % 4);
}

enum class RoomType : std::uint8_t { Start, Normal, Exit };

struct Room {
    RoomId id;
    int grid_x = 0;
    int grid_y = 0;
    RoomType type = RoomType::Normal;
    std::array<bool, kDirectionCount> doors{}; // indexed by Direction

    [[nodiscard]] bool HasDoor(Direction d) const { return doors[static_cast<std::size_t>(d)]; }
};

struct Floor {
    std::uint32_t depth = 1;
    std::vector<Room> rooms;
    RoomId start = 0;
    RoomId exit = 0;

    [[nodiscard]] std::optional<RoomId> RoomAt(int grid_x, int grid_y) const {
        for (const auto& room : rooms) {
            if (room.grid_x == grid_x && room.grid_y == grid_y) {
                return room.id;
            }
        }
        return std::nullopt;
    }

    // The room behind the door on side `d`; nullopt if there is no such door.
    [[nodiscard]] std::optional<RoomId> Neighbor(RoomId id, Direction d) const {
        const Room& room = rooms[id];
        if (!room.HasDoor(d)) {
            return std::nullopt;
        }
        return RoomAt(room.grid_x + DirectionDx(d), room.grid_y + DirectionDy(d));
    }
};

} // namespace infinity_dungeon::sim
