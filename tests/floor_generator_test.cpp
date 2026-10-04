#include "sim/FloorGenerator.hpp"

#include <doctest/doctest.h>

#include <algorithm>
#include <cstdlib>
#include <deque>
#include <set>
#include <utility>

using infinity_dungeon::sim::Direction;
using infinity_dungeon::sim::DirectionDx;
using infinity_dungeon::sim::DirectionDy;
using infinity_dungeon::sim::Floor;
using infinity_dungeon::sim::FloorConfig;
using infinity_dungeon::sim::GenerateFloor;
using infinity_dungeon::sim::kDirectionCount;
using infinity_dungeon::sim::Opposite;
using infinity_dungeon::sim::RoomId;
using infinity_dungeon::sim::RoomType;

namespace {

bool SameFloor(const Floor& a, const Floor& b) {
    if (a.rooms.size() != b.rooms.size() || a.start != b.start || a.exit != b.exit) {
        return false;
    }
    for (std::size_t i = 0; i < a.rooms.size(); ++i) {
        if (a.rooms[i].grid_x != b.rooms[i].grid_x || a.rooms[i].grid_y != b.rooms[i].grid_y ||
            a.rooms[i].type != b.rooms[i].type || a.rooms[i].doors != b.rooms[i].doors) {
            return false;
        }
    }
    return true;
}

} // namespace

TEST_CASE("same seed and depth produce identical floors") {
    const FloorConfig config;
    const auto a = GenerateFloor(42, 7, config);
    const auto b = GenerateFloor(42, 7, config);

    REQUIRE_FALSE(a.rooms.empty());
    CHECK(SameFloor(a, b));
}

TEST_CASE("different seeds produce different floors") {
    const FloorConfig config;
    int differing = 0;
    for (std::uint32_t seed = 0; seed < 20; ++seed) {
        differing += SameFloor(GenerateFloor(seed, 1, config), GenerateFloor(seed + 1000, 1, config)) ? 0 : 1;
    }
    CHECK(differing >= 18);
}

TEST_CASE("different depths of the same seed produce different floors") {
    const FloorConfig config;
    int differing = 0;
    for (std::uint32_t seed = 0; seed < 20; ++seed) {
        differing += SameFloor(GenerateFloor(seed, 1, config), GenerateFloor(seed, 2, config)) ? 0 : 1;
    }
    CHECK(differing >= 18);
}

TEST_CASE("generated floors are valid connected trees with a sensible exit") {
    for (const FloorConfig config : {FloorConfig{}, FloorConfig{.grid_width = 5, .grid_height = 5, .room_count = 15},
                                     FloorConfig{.grid_width = 3, .grid_height = 3, .room_count = 2}}) {
        for (std::uint32_t seed = 0; seed < 200; ++seed) {
            const std::uint32_t depth = 1 + seed % 5;
            const Floor floor = GenerateFloor(seed, depth, config);
            CAPTURE(seed);

            REQUIRE(floor.rooms.size() == static_cast<std::size_t>(config.room_count));
            CHECK(floor.depth == depth);

            // Unique, in-bounds cells; start at the grid center.
            std::set<std::pair<int, int>> cells;
            for (const auto& room : floor.rooms) {
                CHECK(room.grid_x >= 0);
                CHECK(room.grid_y >= 0);
                CHECK(room.grid_x < config.grid_width);
                CHECK(room.grid_y < config.grid_height);
                cells.insert({room.grid_x, room.grid_y});
            }
            CHECK(cells.size() == floor.rooms.size());
            CHECK(floor.rooms[floor.start].grid_x == config.grid_width / 2);
            CHECK(floor.rooms[floor.start].grid_y == config.grid_height / 2);
            CHECK(floor.rooms[floor.start].type == RoomType::Start);

            // Doors are symmetric and only between grid neighbors; tree => rooms - 1 edges.
            int door_ends = 0;
            for (const auto& room : floor.rooms) {
                for (std::size_t d = 0; d < kDirectionCount; ++d) {
                    const auto dir = static_cast<Direction>(d);
                    const auto neighbor = floor.RoomAt(room.grid_x + DirectionDx(dir), room.grid_y + DirectionDy(dir));
                    // A door exists exactly when the adjacent cell holds a room.
                    CHECK(room.doors[d] == neighbor.has_value());
                    if (room.doors[d] && neighbor) {
                        door_ends += 1;
                        CHECK(floor.rooms[*neighbor].HasDoor(Opposite(dir)));
                    }
                }
            }
            CHECK(door_ends == 2 * static_cast<int>(floor.rooms.size() - 1));

            // Everything is reachable from the start.
            std::set<RoomId> seen{floor.start};
            std::deque<RoomId> queue{floor.start};
            while (!queue.empty()) {
                const RoomId id = queue.front();
                queue.pop_front();
                for (std::size_t d = 0; d < kDirectionCount; ++d) {
                    const auto next = floor.Neighbor(id, static_cast<Direction>(d));
                    if (next && seen.insert(*next).second) {
                        queue.push_back(*next);
                    }
                }
            }
            CHECK(seen.size() == floor.rooms.size());

            // The exit is another room, marked as such, and a dead end.
            CHECK(floor.exit != floor.start);
            CHECK(floor.rooms[floor.exit].type == RoomType::Exit);
            const auto& exit_doors = floor.rooms[floor.exit].doors;
            CHECK(std::count(exit_doors.begin(), exit_doors.end(), true) == 1);
            CHECK(std::count_if(floor.rooms.begin(), floor.rooms.end(),
                                [](const auto& room) { return room.type == RoomType::Exit; }) == 1);
        }
    }
}

TEST_CASE("unusable config values are clamped instead of failing") {
    const Floor floor = GenerateFloor(1, 1, FloorConfig{.grid_width = 0, .grid_height = -3, .room_count = 50});
    CHECK(floor.rooms.size() == 1);
    CHECK(floor.exit == floor.start);
}
