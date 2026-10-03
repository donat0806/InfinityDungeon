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
