#include "sim/DungeonSimulator.hpp"

#include <doctest/doctest.h>

using infinity_dungeon::sim::DungeonSimulator;
using infinity_dungeon::sim::Room;

namespace {

bool SameRooms(const std::vector<Room>& a, const std::vector<Room>& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (a[i].id != b[i].id || a[i].difficulty != b[i].difficulty) {
            return false;
        }
    }
    return true;
}

} // namespace

TEST_CASE("same seed and depth produce identical levels") {
    const DungeonSimulator a(42);
    const DungeonSimulator b(42);

    const auto level_a = a.GenerateLevel(7);
    const auto level_b = b.GenerateLevel(7);

    REQUIRE_FALSE(level_a.empty());
    CHECK(SameRooms(level_a, level_b));
}

TEST_CASE("different seeds produce different levels") {
    const DungeonSimulator a(42);
    const DungeonSimulator b(1337);

    CHECK_FALSE(SameRooms(a.GenerateLevel(7), b.GenerateLevel(7)));
}
