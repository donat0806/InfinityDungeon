#include "sim/GameState.hpp"

#include <doctest/doctest.h>

#include <algorithm>
#include <cmath>
#include <deque>
#include <unordered_map>
#include <vector>

using infinity_dungeon::input::PlayerInput;
using infinity_dungeon::sim::Direction;
using infinity_dungeon::sim::DirectionDx;
using infinity_dungeon::sim::DirectionDy;
using infinity_dungeon::sim::EnemyConfig;
using infinity_dungeon::sim::FloorConfig;
using infinity_dungeon::sim::GameState;
using infinity_dungeon::sim::kDirectionCount;
using infinity_dungeon::sim::Opposite;
using infinity_dungeon::sim::PlayerId;
using infinity_dungeon::sim::RoomId;

namespace {

constexpr float kDt = 1.0f / 60.0f;

void Step(GameState& state, PlayerId id, float move_x, float move_y) {
    state.Tick(kDt, {{id, PlayerInput{.move_x = move_x, .move_y = move_y}}});
}

float Steer(float error) {
    return std::clamp(error / 10.0f, -1.0f, 1.0f);
}

// Walks the player to the middle of the wall on side `dir`, then out through
// its door. Returns true once the current room has changed.
bool Cross(GameState& state, PlayerId id, Direction dir) {
    const RoomId from = state.CurrentRoomId();
    const bool horizontal = DirectionDx(dir) != 0;
    for (int i = 0; i < 2000 && state.CurrentRoomId() == from; ++i) {
        const auto& player = state.Players()[0];
        const float across = horizontal ? player.y : player.x;
        if (std::abs(across) > 2.0f) {
            // Line up with the door first.
            Step(state, id, horizontal ? 0.0f : Steer(-player.x), horizontal ? Steer(-player.y) : 0.0f);
        } else {
            Step(state, id, static_cast<float>(DirectionDx(dir)), static_cast<float>(DirectionDy(dir)));
        }
    }
    return state.CurrentRoomId() != from;
}

// Walks the player to the room center.
void WalkToCenter(GameState& state, PlayerId id) {
    for (int i = 0; i < 2000; ++i) {
        const auto& player = state.Players()[0];
        if (std::abs(player.x) < 1.0f && std::abs(player.y) < 1.0f) {
            return;
        }
        Step(state, id, Steer(-player.x), Steer(-player.y));
    }
}

// Directions to take from the start room to reach the exit, by breadth-first search.
std::vector<Direction> PathToExit(const GameState& state) {
    const auto& floor = state.CurrentFloor();
    std::vector<int> came_from(floor.rooms.size(), -1);
    std::vector<Direction> came_by(floor.rooms.size(), Direction::North);
    std::deque<RoomId> queue{floor.start};
    came_from[floor.start] = static_cast<int>(floor.start);
    while (!queue.empty()) {
        const RoomId id = queue.front();
        queue.pop_front();
        for (std::size_t d = 0; d < kDirectionCount; ++d) {
            const auto next = floor.Neighbor(id, static_cast<Direction>(d));
            if (next && came_from[*next] < 0) {
                came_from[*next] = static_cast<int>(id);
                came_by[*next] = static_cast<Direction>(d);
                queue.push_back(*next);
            }
        }
    }
    std::vector<Direction> path;
    for (RoomId id = floor.exit; id != floor.start; id = static_cast<RoomId>(came_from[id])) {
        path.push_back(came_by[id]);
    }
    std::reverse(path.begin(), path.end());
    return path;
}

EnemyConfig NoEnemies() {
    EnemyConfig config;
    config.wave.count = 0;
    return config;
}

// Stationary, harmless, one-shot enemies, so a test can clear rooms reliably.
EnemyConfig HarmlessEnemies(int count = 2) {
    EnemyConfig config;
    config.wave.count = count;
    for (auto* stats : {&config.chaser, &config.shooter, &config.patroller}) {
        stats->max_health = 1;
        stats->move_speed = 0.0f;
        stats->contact_damage = 0;
        stats->projectile_damage = 0;
    }
    return config;
}

// Shoots the room's enemies one by one, closing in when they are out of range.
void ClearRoom(GameState& state, PlayerId id) {
    for (int i = 0; i < 6000 && !state.Enemies().empty(); ++i) {
        const auto& player = state.Players()[0];
        const auto& enemy = state.Enemies()[0];
        const float dx = enemy.x - player.x;
        const float dy = enemy.y - player.y;
        const bool far = std::sqrt(dx * dx + dy * dy) > 300.0f;
        state.Tick(kDt, {{id, PlayerInput{.move_x = far ? dx : 0.0f, .move_y = far ? dy : 0.0f, .aim_x = dx, .aim_y = dy}}});
    }
}

} // namespace

TEST_CASE("the player starts in an empty start room with only it visited") {
    GameState state(42);
    state.AddPlayer(0.0f, 0.0f);
    state.Tick(kDt, {});

    const auto& floor = state.CurrentFloor();
    CHECK(state.CurrentRoomId() == floor.start);
    CHECK(state.Enemies().empty());
    for (const auto& room : floor.rooms) {
        CHECK(state.Visited(room.id) == (room.id == floor.start));
    }
}

TEST_CASE("a wall without a door stops the player") {
    // Find a seed whose start room has at least one closed wall.
    std::uint32_t seed = 0;
    for (;; ++seed) {
        const GameState probe(seed);
        const auto& doors = probe.CurrentRoom().doors;
        if (std::count(doors.begin(), doors.end(), true) < 4) {
            break;
        }
    }
    GameState state(seed);
    const PlayerId id = state.AddPlayer(0.0f, 0.0f);
    const auto& room = state.CurrentRoom();
    const auto missing = std::find(room.doors.begin(), room.doors.end(), false);
    REQUIRE(missing != room.doors.end());
    const auto dir = static_cast<Direction>(missing - room.doors.begin());

    for (int i = 0; i < 600; ++i) {
        Step(state, id, static_cast<float>(DirectionDx(dir)), static_cast<float>(DirectionDy(dir)));
    }

    CHECK(state.CurrentRoomId() == state.CurrentFloor().start);
    const auto& arena = state.ArenaBounds();
    const auto& player = state.Players()[0];
    const float r = player.stats.radius;
    if (DirectionDx(dir) != 0) {
        CHECK(std::abs(player.x) == doctest::Approx(arena.max_x - r));
    } else {
        CHECK(std::abs(player.y) == doctest::Approx(arena.max_y - r));
    }
}

TEST_CASE("walking through a door enters the neighboring room at the opposite side") {
    GameState state(42, NoEnemies());
    const PlayerId id = state.AddPlayer(0.0f, 0.0f);
    const auto& room = state.CurrentRoom();
    const auto open = std::find(room.doors.begin(), room.doors.end(), true);
    REQUIRE(open != room.doors.end());
    const auto dir = static_cast<Direction>(open - room.doors.begin());
    const RoomId expected = *state.CurrentFloor().Neighbor(state.CurrentFloor().start, dir);

    REQUIRE(Cross(state, id, dir));

    CHECK(state.CurrentRoomId() == expected);
    CHECK(state.Visited(expected));
    const auto& arena = state.ArenaBounds();
    const auto& player = state.Players()[0];
    CHECK(state.CurrentRoom().HasDoor(Opposite(dir)));
    if (DirectionDx(dir) != 0) {
        CHECK((dir == Direction::East ? player.x < arena.min_x + 40.0f : player.x > arena.max_x - 40.0f));
    } else {
        CHECK((dir == Direction::South ? player.y < arena.min_y + 40.0f : player.y > arena.max_y - 40.0f));
    }
}

TEST_CASE("doors lock while a room has enemies") {
    GameState state(42, HarmlessEnemies(3));
    const PlayerId id = state.AddPlayer(0.0f, 0.0f);
    CHECK_FALSE(state.DoorsLocked()); // the start room is empty
    const auto& start_room = state.CurrentRoom();
    const auto open = std::find(start_room.doors.begin(), start_room.doors.end(), true);
    REQUIRE(open != start_room.doors.end());
    const auto dir = static_cast<Direction>(open - start_room.doors.begin());

    REQUIRE(Cross(state, id, dir));
    const RoomId entered = state.CurrentRoomId();
    REQUIRE(state.Enemies().size() == 3);
    CHECK(state.DoorsLocked());

    // The way back is shut too.
    CHECK_FALSE(Cross(state, id, Opposite(dir)));
    CHECK(state.CurrentRoomId() == entered);
}

TEST_CASE("clearing a room unlocks its doors and it stays cleared") {
    GameState state(42, HarmlessEnemies(3));
    const PlayerId id = state.AddPlayer(0.0f, 0.0f);
    const auto& start_room = state.CurrentRoom();
    const auto open = std::find(start_room.doors.begin(), start_room.doors.end(), true);
    REQUIRE(open != start_room.doors.end());
    const auto dir = static_cast<Direction>(open - start_room.doors.begin());

    REQUIRE(Cross(state, id, dir));
    const RoomId entered = state.CurrentRoomId();
    ClearRoom(state, id);
    REQUIRE(state.Enemies().empty());
    CHECK_FALSE(state.DoorsLocked());

    REQUIRE(Cross(state, id, Opposite(dir)));
    CHECK(state.CurrentRoomId() == state.CurrentFloor().start);
    REQUIRE(Cross(state, id, dir));
    CHECK(state.CurrentRoomId() == entered);
    CHECK(state.Enemies().empty());
    CHECK_FALSE(state.DoorsLocked());
}

TEST_CASE("the exit hatch stays shut until the exit room is cleared") {
    GameState state(42, HarmlessEnemies(1));
    const PlayerId id = state.AddPlayer(0.0f, 0.0f);
    for (const Direction dir : PathToExit(state)) {
        ClearRoom(state, id);
        REQUIRE(Cross(state, id, dir));
    }

    CHECK(state.CurrentRoomId() == state.CurrentFloor().exit);
    CHECK_FALSE(state.Enemies().empty());
    CHECK(state.DoorsLocked());
    CHECK_FALSE(state.HatchOpen());

    ClearRoom(state, id);
    CHECK(state.HatchOpen());
}

TEST_CASE("stepping on the open hatch descends to a new floor") {
    GameState state(42, NoEnemies());
    const PlayerId id = state.AddPlayer(0.0f, 0.0f);
    const auto first_floor = state.CurrentFloor();

    for (const Direction dir : PathToExit(state)) {
        REQUIRE(Cross(state, id, dir));
    }
    REQUIRE(state.CurrentRoomId() == first_floor.exit);
    CHECK(state.HatchOpen());
    CHECK(state.Depth() == 1);

    WalkToCenter(state, id);

    CHECK(state.Depth() == 2);
    CHECK(state.CurrentFloor().depth == 2);
    CHECK(state.CurrentRoomId() == state.CurrentFloor().start);
    CHECK(state.Players()[0].x == doctest::Approx(0.0f));
    CHECK(state.Players()[0].y == doctest::Approx(0.0f));
    CHECK(state.Enemies().empty());
    for (const auto& room : state.CurrentFloor().rooms) {
        CHECK(state.Visited(room.id) == (room.id == state.CurrentFloor().start));
    }
}

TEST_CASE("a whole walk through the floors is deterministic for the same seed") {
    auto run = [](std::uint32_t seed) {
        GameState state(seed, HarmlessEnemies());
        const PlayerId id = state.AddPlayer(0.0f, 0.0f);
        for (const Direction dir : PathToExit(state)) {
            ClearRoom(state, id);
            Cross(state, id, dir);
        }
        for (int i = 0; i < 120; ++i) {
            state.Tick(kDt, {{id, PlayerInput{.move_x = 0.3f, .aim_x = 1.0f}}});
        }
        return state;
    };

    const GameState a = run(7);
    const GameState b = run(7);

    CHECK(a.CurrentRoomId() == b.CurrentRoomId());
    CHECK(a.Players()[0].x == doctest::Approx(b.Players()[0].x));
    CHECK(a.Players()[0].health == b.Players()[0].health);
    REQUIRE(a.Enemies().size() == b.Enemies().size());
    for (std::size_t i = 0; i < a.Enemies().size(); ++i) {
        CHECK(a.Enemies()[i].x == doctest::Approx(b.Enemies()[i].x));
        CHECK(a.Enemies()[i].y == doctest::Approx(b.Enemies()[i].y));
    }
}
