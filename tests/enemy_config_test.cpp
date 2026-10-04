#include "sim/EnemyConfig.hpp"
#include "sim/Rng.hpp"

#include <doctest/doctest.h>

#include <stdexcept>

using infinity_dungeon::sim::EnemyConfig;
using infinity_dungeon::sim::ParseEnemyConfig;
using infinity_dungeon::sim::Rng;

TEST_CASE("a full enemy config parses to the given values") {
    const auto config = ParseEnemyConfig(R"({
        "enemies": {
            "chaser": {"max_health": 9, "move_speed": 11.5, "radius": 3, "contact_damage": 2},
            "shooter": {"fire_interval": 0.25, "projectile_damage": 3},
            "patroller": {"move_speed": 77}
        },
        "wave": {"count": 4, "min_spawn_distance": 123}
    })");

    CHECK(config.chaser.max_health == 9);
    CHECK(config.chaser.move_speed == doctest::Approx(11.5f));
    CHECK(config.chaser.contact_damage == 2);
    CHECK(config.shooter.fire_interval == doctest::Approx(0.25f));
    CHECK(config.shooter.projectile_damage == 3);
    CHECK(config.patroller.move_speed == doctest::Approx(77.0f));
    CHECK(config.wave.count == 4);
    CHECK(config.wave.min_spawn_distance == doctest::Approx(123.0f));
}

TEST_CASE("a partial enemy config keeps defaults for missing fields") {
    const EnemyConfig defaults;
    const auto config = ParseEnemyConfig(R"({"enemies": {"chaser": {"move_speed": 5}}})");

    CHECK(config.chaser.move_speed == doctest::Approx(5.0f));
    CHECK(config.chaser.max_health == defaults.chaser.max_health);
    CHECK(config.shooter.fire_interval == doctest::Approx(defaults.shooter.fire_interval));
    CHECK(config.wave.count == defaults.wave.count);

    const auto empty = ParseEnemyConfig("{}");
    CHECK(empty.patroller.move_speed == doctest::Approx(defaults.patroller.move_speed));
}

TEST_CASE("malformed or wrongly typed enemy config throws") {
    CHECK_THROWS_AS(ParseEnemyConfig("{not json"), std::runtime_error);
    CHECK_THROWS_AS(ParseEnemyConfig("[1, 2]"), std::runtime_error);
    CHECK_THROWS_AS(ParseEnemyConfig(R"({"enemies": {"chaser": {"max_health": "lots"}}})"), std::runtime_error);
    CHECK_THROWS_AS(ParseEnemyConfig(R"({"enemies": 3})"), std::runtime_error);
}

TEST_CASE("Rng gives the same sequence for the same seed and stays in range") {
    Rng a(99);
    Rng b(99);
    Rng c(100);

    bool differs = false;
    for (int i = 0; i < 100; ++i) {
        const auto va = a.NextU32();
        CHECK(va == b.NextU32());
        differs = differs || va != c.NextU32();
    }
    CHECK(differs);

    Rng r(5);
    for (int i = 0; i < 1000; ++i) {
        const float f = r.NextFloat(-10.0f, 10.0f);
        CHECK(f >= -10.0f);
        CHECK(f < 10.0f);
        CHECK(r.NextIndex(3) < 3u);
    }
}
