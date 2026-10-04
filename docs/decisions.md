# Decision log

Newest entries at the bottom. Each entry records what was decided and why.

## 2026-10-03: Fetch raylib with CMake FetchContent

raylib (pinned to tag `5.5`) is downloaded and built by CMake at configure time instead of being installed separately or vendored.

- Contributors and CI need only CMake, a compiler and Git.
- Pinning a tag keeps builds reproducible.
- Trade-off: the first configure needs network access and takes longer.

## 2026-10-03: doctest as the C++ test framework

Tests use doctest (pinned to `v2.4.12`), also via FetchContent. `doctest_discover_tests` registers each `TEST_CASE` as its own CTest test.

- doctest is a single header and compiles quickly, which suits a small test suite.
- Catch2 v3 has more features (generators, benchmarks, matchers), but nothing here needs them yet.

## 2026-10-03: Simulation stays free of raylib, enforced by the build

Each `src/` folder is its own CMake library. Only `infinity_render`, `infinity_input` and the executable link raylib; `infinity_sim` does not, so `#include <raylib.h>` in sim code fails to compile.

- This keeps game logic testable without a window and deterministic, which matters for replays and online co-op later.
- The rule is enforced by the build, not by convention.

## 2026-10-03: CMakePresets for standard configurations

`CMakePresets.json` defines one configure preset (`default`, which builds into `build/`) plus `debug` and `release` build and test presets.

- IDEs (VS Code, Visual Studio, CLion) pick these up automatically.
- The plain `cmake -B build` workflow still works.
- Personal overrides go in the git-ignored `CMakeUserPresets.json`.

## 2026-10-03: New `infinity_app` module for the scene/state system

`src/app` owns `Scene`, `SceneManager`, and the concrete scenes (`MenuScene`, `PlayingScene`). It links `infinity_sim`, `infinity_render`, `infinity_input`, and raylib directly, so scenes can draw with raylib calls.

- `Scene`/`SceneManager` orchestrate sim and presentation together; they are neither pure simulation nor a rendering primitive, so they didn't fit in an existing module.
- Giving scenes direct raylib access (rather than growing `Renderer` into a generic drawing API) keeps `Renderer` a thin window/frame owner and lets scene-specific drawing (HUD, menu text, later: camera, sprites) live next to the scene it belongs to.
- `infinity_sim` still never links raylib; only this orchestration layer and `infinity_render`/`infinity_input` do.

## 2026-10-03: Fixed-timestep loop via a dedicated clock

`sim::FixedTimestepClock` turns raylib's variable `GetFrameTime()` into a whole number of fixed-size steps per frame (default 60 Hz), capped at 5 steps per call to avoid a stall snowballing into unbounded catch-up simulation ("spiral of death").

- Keeps `GameState::Tick` deterministic and frame-rate independent, as the architecture rules require.
- The cap trades a visible slowdown during a long stall for a bounded worst case, rather than the game freezing while it tries to catch up.
- Exposes `Alpha()` for render interpolation later; unused for now since there's nothing yet to interpolate between ticks.

## 2026-10-03: Twin-stick controls, `PlayerInput` split into move, aim and confirm

Movement is WASD and shooting is the arrow keys (Isaac-style). `PlayerInput` now carries `move_x/y`, `aim_x/y` and `confirm`; the old `attack` flag is gone.

- Aim is a vector, so a gamepad's right stick or a network packet fills the same fields as the keyboard. `(0, 0)` means not shooting.
- `confirm` (Space/Enter) is for menus only, so UI input is not mixed up with gameplay input.
- The sim normalizes move vectors longer than 1 (no faster diagonals) and clamps players to a fixed `Arena` rectangle, a stand-in until floor generation.
- Per-player tunables live in `sim::PlayerStats` on each `Player`. Loading them from config files was deferred past this step; enemy stats are the first to be data-driven (see the 2026-10-04 entry).

## 2026-10-03: Strict `-std=c++20` (no compiler extensions)

`CMakeLists.txt` sets `CMAKE_CXX_EXTENSIONS OFF`, so GCC and Clang build with `-std=c++20` rather than `gnu++20`.

- Code that only compiles with a compiler-specific extension fails locally instead of in CI on another platform.
- Sources were also checked with `g++ -Wall -Wextra -pedantic` alongside the MSVC build; both are warning-free.
- The sim uses C++20 designated initializers (e.g. `PlayerInput{.move_x = 1.0f}`), which keeps call sites readable as structs grow.

## 2026-10-04: Enemy balance loaded from JSON with nlohmann/json

Enemy stats and wave settings live in `config/enemies.json`, parsed by `sim::ParseEnemyConfig` (`src/sim/EnemyConfig.*`) with nlohmann/json (v3.11.3, fetched by CMake, linked privately to `infinity_sim`, no raylib).

- Satisfies the data-driven balance rule: tuning needs no recompile. CMake copies `config/` next to the executable after each build.
- The built-in member defaults of `EnemyConfig` are the fallback: missing JSON fields keep their defaults, so the sim and tests need no file. A missing or broken file makes `main.cpp` log to stderr and use the defaults.
- Parsing is a pure string function (the file reader is a thin wrapper), so it is unit-tested without touching the disk.
- Player stats stay hard-coded in `PlayerStats` for now.
- The sim has its own PCG32 `sim::Rng`, seeded from the run seed, instead of `<random>` distributions, whose output may differ between standard libraries and break cross-platform determinism.

## 2026-10-04: Projectile factions; dead players stay in the collection

Projectiles carry a `Faction` (Player or Enemy) and a `damage` value, so one projectile type and one collision pass serve both sides. Factions never hurt their own side.

- A player at 0 health stays in `players_` (so `PlayerId`s stay stable and a later co-op revive is possible) but ignores input and is not targeted. `GameState::AllPlayersDead()` drives `PlayingScene`'s return to the menu until a proper death screen exists.
- Players get brief invulnerability after each hit, so contact damage doesn't drain health every tick.
- Enemies are spawned as one seeded wave at run start; wave respawning and room-based spawning wait for floor generation.

## 2026-10-04: Grid-based floor generation; rooms stashed per room; party moves together

`sim::GenerateFloor(seed, depth, FloorConfig)` (`src/sim/FloorGenerator.*`) builds each floor as a tree of rooms on a grid, replacing the `DungeonSimulator` stub. Layout size comes from `config/floor.json` (`FloorConfig`, same defaults-as-fallback pattern as enemies).

- Pure function with its own RNG stream per depth (`Rng(seed, 1000 + depth)`): same seed and depth give the same floor, and descending never depends on what happened in the previous floor.
- Frontier growth from the grid center, adding only cells that touch exactly one existing room, so floors are loop-free with branching dead ends. Growth that gets stuck is retried (bounded), so the default config always reaches `room_count`.
- The exit is the dead end farthest from the start (by door distance), so reaching it means exploring.
- All rooms share one size (`Arena`), with a door in the middle of each wall. Doors are always open for now; locking them until a room is cleared is the next roadmap item.
- Enemies spawn on first entry to a room (not the start room). Leaving a room mid-fight stashes its enemies, and they are restored on return rather than respawned. Projectiles are dropped on a room change.
- Every player moves to the new room together and arrives just inside the opposite door, which keeps local co-op simple (no split-screen or per-player rooms).
- Descending is a hatch in the middle of the exit room, usable once its enemies are dead. Depth feeds only the generator for now; difficulty scaling is a later item.
