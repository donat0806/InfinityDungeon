#include "FloorGenerator.hpp"

#include "sim/Rng.hpp"

#include <algorithm>
#include <deque>

namespace infinity_dungeon::sim {

namespace {

constexpr int kMaxAttempts = 100;
constexpr std::uint32_t kRngStreamBase = 1000;

struct Cell {
    int x;
    int y;
};

class Grid {
public:
    Grid(int width, int height) : width_(width), height_(height), ids_(static_cast<std::size_t>(width * height), -1) {}

    [[nodiscard]] bool InBounds(int x, int y) const { return x >= 0 && y >= 0 && x < width_ && y < height_; }
    [[nodiscard]] int& At(int x, int y) { return ids_[static_cast<std::size_t>(y * width_ + x)]; }
    [[nodiscard]] int At(int x, int y) const { return ids_[static_cast<std::size_t>(y * width_ + x)]; }

    [[nodiscard]] int OccupiedNeighbors(int x, int y) const {
        int count = 0;
        for (std::size_t d = 0; d < kDirectionCount; ++d) {
            const auto dir = static_cast<Direction>(d);
            const int nx = x + DirectionDx(dir);
            const int ny = y + DirectionDy(dir);
            if (InBounds(nx, ny) && At(nx, ny) >= 0) {
                ++count;
            }
        }
        return count;
    }

private:
    int width_;
    int height_;
    std::vector<int> ids_;
};

// One growth attempt: returns the cells in the order they were added (index =
// RoomId). Every new cell touches exactly one existing cell, so the layout is a
// tree with no loops. May return fewer than `target` cells if growth gets stuck.
std::vector<Cell> Grow(Rng& rng, const FloorConfig& config, std::size_t target) {
    Grid grid(config.grid_width, config.grid_height);
    std::vector<Cell> cells;
    cells.push_back({config.grid_width / 2, config.grid_height / 2});
    grid.At(cells[0].x, cells[0].y) = 0;

    std::deque<std::size_t> frontier{0};
    bool added_this_pass = false;
    while (cells.size() < target) {
        if (frontier.empty()) {
            // Everything was visited; start another pass over all rooms, unless
            // the last pass added nothing (the layout is stuck).
            if (!added_this_pass) {
                break;
            }
            added_this_pass = false;
            for (std::size_t i = 0; i < cells.size(); ++i) {
                frontier.push_back(i);
            }
        }
        const std::size_t current = frontier.front();
        frontier.pop_front();

        std::array<Direction, kDirectionCount> order{Direction::North, Direction::East, Direction::South,
                                                     Direction::West};
        for (std::size_t i = order.size() - 1; i > 0; --i) {
            std::swap(order[i], order[rng.NextIndex(static_cast<std::uint32_t>(i + 1))]);
        }

        for (const Direction dir : order) {
            if (cells.size() >= target) {
                break;
            }
            const int nx = cells[current].x + DirectionDx(dir);
            const int ny = cells[current].y + DirectionDy(dir);
            if (!grid.InBounds(nx, ny) || grid.At(nx, ny) >= 0 || grid.OccupiedNeighbors(nx, ny) != 1) {
                continue;
            }
            if (rng.NextIndex(2) != 0) {
                continue;
            }
            grid.At(nx, ny) = static_cast<int>(cells.size());
            frontier.push_back(cells.size());
            cells.push_back({nx, ny});
            added_this_pass = true;
        }
    }
    return cells;
}

} // namespace

Floor GenerateFloor(std::uint32_t seed, std::uint32_t depth, const FloorConfig& config_in) {
    FloorConfig config = config_in;
    config.grid_width = std::max(1, config.grid_width);
    config.grid_height = std::max(1, config.grid_height);
    config.room_count = std::clamp(config.room_count, 1, config.grid_width * config.grid_height);
    const auto target = static_cast<std::size_t>(config.room_count);

    Rng rng(seed, kRngStreamBase + depth);
    std::vector<Cell> best;
    for (int attempt = 0; attempt < kMaxAttempts && best.size() < target; ++attempt) {
        auto cells = Grow(rng, config, target);
        if (cells.size() > best.size()) {
            best = std::move(cells);
        }
    }

    Floor floor;
    floor.depth = depth;
    floor.rooms.reserve(best.size());
    for (std::size_t i = 0; i < best.size(); ++i) {
        Room room;
        room.id = static_cast<RoomId>(i);
        room.grid_x = best[i].x;
        room.grid_y = best[i].y;
        room.type = i == 0 ? RoomType::Start : RoomType::Normal;
        floor.rooms.push_back(room);
    }
    for (auto& room : floor.rooms) {
        for (std::size_t d = 0; d < kDirectionCount; ++d) {
            const auto dir = static_cast<Direction>(d);
            room.doors[d] = floor.RoomAt(room.grid_x + DirectionDx(dir), room.grid_y + DirectionDy(dir)).has_value();
        }
    }

    // Breadth-first distance from the start over doors.
    std::vector<int> dist(floor.rooms.size(), -1);
    std::deque<RoomId> queue{floor.start};
    dist[floor.start] = 0;
    while (!queue.empty()) {
        const RoomId id = queue.front();
        queue.pop_front();
        for (std::size_t d = 0; d < kDirectionCount; ++d) {
            const auto next = floor.Neighbor(id, static_cast<Direction>(d));
            if (next && dist[*next] < 0) {
                dist[*next] = dist[id] + 1;
                queue.push_back(*next);
            }
        }
    }

    // Exit: the farthest dead end; ties go to the lowest id. With no dead end,
    // take the farthest room instead.
    auto door_count = [&](const Room& room) {
        return std::count(room.doors.begin(), room.doors.end(), true);
    };
    RoomId exit = floor.start;
    for (const bool dead_ends_only : {true, false}) {
        int exit_dist = -1;
        for (const auto& room : floor.rooms) {
            if (room.id == floor.start || (dead_ends_only && door_count(room) != 1)) {
                continue;
            }
            if (dist[room.id] > exit_dist) {
                exit = room.id;
                exit_dist = dist[room.id];
            }
        }
        if (exit_dist >= 0) {
            break;
        }
    }
    floor.exit = exit;
    if (exit != floor.start) {
        floor.rooms[exit].type = RoomType::Exit;
    }
    return floor;
}

} // namespace infinity_dungeon::sim
