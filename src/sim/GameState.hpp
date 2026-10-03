#pragma once

#include "input/PlayerInput.hpp"
#include "sim/DungeonSimulator.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace infinity_dungeon::sim {

using PlayerId = std::uint32_t;

struct Player {
    PlayerId id;
    float x = 0.0f;
    float y = 0.0f;
};

// Holds everything a run needs to be deterministic: the seed, the current
// level, and the players, addressed by ID rather than by pointer or index
// so entities can be added, removed, or sent over the network later
// without invalidating references held elsewhere.
//
// No raylib here; this is advanced only by Tick(), at a fixed step, so the
// same seed and the same sequence of inputs always produce the same run.
class GameState {
public:
    explicit GameState(std::uint32_t seed);

    PlayerId AddPlayer(float x, float y);

    void Tick(float fixed_dt, const std::unordered_map<PlayerId, input::PlayerInput>& inputs);

    [[nodiscard]] std::uint32_t Depth() const { return depth_; }
    [[nodiscard]] const std::vector<Room>& Rooms() const { return rooms_; }
    [[nodiscard]] const std::vector<Player>& Players() const { return players_; }

private:
    static constexpr float kPlayerSpeed = 200.0f; // units per second

    std::uint32_t seed_;
    std::uint32_t depth_ = 1;
    DungeonSimulator simulator_;
    std::vector<Room> rooms_;
    std::vector<Player> players_;
    PlayerId next_player_id_ = 0;
};

} // namespace infinity_dungeon::sim
