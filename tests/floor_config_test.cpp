#include "sim/FloorConfig.hpp"

#include <doctest/doctest.h>

#include <stdexcept>

using infinity_dungeon::sim::FloorConfig;
using infinity_dungeon::sim::ParseFloorConfig;

TEST_CASE("a full floor config parses to the given values") {
    const auto config = ParseFloorConfig(R"({"grid_width": 7, "grid_height": 6, "room_count": 12})");

    CHECK(config.grid_width == 7);
    CHECK(config.grid_height == 6);
    CHECK(config.room_count == 12);
}

TEST_CASE("a partial floor config keeps defaults for missing fields") {
    const FloorConfig defaults;
    const auto config = ParseFloorConfig(R"({"room_count": 5})");

    CHECK(config.room_count == 5);
    CHECK(config.grid_width == defaults.grid_width);
    CHECK(config.grid_height == defaults.grid_height);
    CHECK(ParseFloorConfig("{}").room_count == defaults.room_count);
}

TEST_CASE("floor config values are clamped to something generatable") {
    const auto config = ParseFloorConfig(R"({"grid_width": 3, "grid_height": 2, "room_count": 99})");
    CHECK(config.room_count == 6);

    const auto tiny = ParseFloorConfig(R"({"grid_width": 0, "grid_height": -4, "room_count": 0})");
    CHECK(tiny.grid_width == 1);
    CHECK(tiny.grid_height == 1);
    CHECK(tiny.room_count == 1);
}

TEST_CASE("malformed or wrongly typed floor config throws") {
    CHECK_THROWS_AS(ParseFloorConfig("{not json"), std::runtime_error);
    CHECK_THROWS_AS(ParseFloorConfig("[1, 2]"), std::runtime_error);
    CHECK_THROWS_AS(ParseFloorConfig(R"({"room_count": "many"})"), std::runtime_error);
}
