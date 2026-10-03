#include "../src/sim/DungeonSimulator.hpp"

int main() {
    const infinity_dungeon::sim::DungeonSimulator a(42);
    const infinity_dungeon::sim::DungeonSimulator b(42);

    const auto level_a = a.GenerateLevel(7);
    const auto level_b = b.GenerateLevel(7);

    if (level_a.size() != level_b.size()) {
        return 1;
    }

    for (std::size_t i = 0; i < level_a.size(); ++i) {
        if (level_a[i].id != level_b[i].id || level_a[i].difficulty != level_b[i].difficulty) {
            return 1;
        }
    }

    return 0;
}
