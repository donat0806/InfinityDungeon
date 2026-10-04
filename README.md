# InfinityDungeon

A top-down, room-based dungeon crawler inspired by *The Binding of Isaac*, written in C++ with raylib. There is no final floor: every level you descend is harder than the last, and your score is how deep you get. Compete on global leaderboards, solo or as a duo.

> **Status:** early development. The feature list below describes the goal; the roadmap shows what is actually done.

<!-- Add a GIF or screenshot here once something is playable -->
<!-- ![InfinityDungeon gameplay](docs/media/gameplay.gif) -->

[![CI](https://github.com/donat0806/InfinityDungeon/actions/workflows/ci.yml/badge.svg)](https://github.com/donat0806/InfinityDungeon/actions/workflows/ci.yml)

## Features

- Twin-stick style movement and shooting in room-based floors
- Procedurally generated floors, built from a seed
- Doors that stay locked until the room is cleared
- Endless descent with difficulty that scales with depth (enemy health, count, and speed)
- Enemies with distinct behaviors (chasers, shooters, patrollers)
- Global leaderboards ranked by deepest floor reached, with separate boards for **Solo** and **Duo**
- Local co-op for two players, with online co-op planned

## Tech stack

| Area | Technology |
|---|---|
| Game | C++ (C++20), [raylib](https://www.raylib.com/) |
| Build | CMake, CMakePresets |
| Config | JSON, parsed with [nlohmann/json](https://github.com/nlohmann/json) |
| Tests | CTest with a C++ test framework (Catch2 / doctest) |
| Leaderboard API | ASP.NET Core (C#), PostgreSQL |
| CI | GitHub Actions (Windows and Linux) |

## Architecture

The project is built around a few rules, chosen so that co-op and online play are possible without rewriting the game:

- **Simulation is separate from presentation.** Game logic is plain C++ with no raylib calls. raylib is used only for rendering, audio, and reading devices.
- **Input is data.** Each player produces a `PlayerInput` struct per tick. Keyboard, gamepad, and (later) network players all feed the same interface.
- **Fixed timestep.** The simulation runs at a constant tick rate, independent of frame rate.
- **Players are a collection.** Nothing assumes a single player; entities reference each other by ID, not raw pointers.
- **Deterministic, seeded randomness.** Gameplay RNG lives in the game state, so a seed and a list of inputs always reproduce the same run. A test checks this.
- **Data-driven balance.** Enemy stats and difficulty scaling are loaded from config files, so they can be tuned without recompiling.

```
InfinityDungeon/
├── src/
│   ├── sim/          # game logic (no raylib)
│   ├── render/       # window and frame handling (raylib)
│   ├── input/        # device -> PlayerInput
│   ├── app/          # scenes (menu, playing) tying sim, render and input together
│   └── net/          # leaderboard client (and later, multiplayer)
├── config/           # balance and layout data (enemies.json, floor.json), copied next to the executable
├── server/           # leaderboard API (ASP.NET Core)
├── tests/            # unit and determinism tests
├── assets/
├── docs/             # design notes and decision log
└── CMakeLists.txt
```

*(Folder layout may change as the project evolves.)*

## Building

### Requirements

- CMake 3.24 or newer
- A C++20 compiler: MSVC (Visual Studio 2022+ or Build Tools), GCC, or Clang
- Git

raylib and nlohmann/json are fetched automatically by CMake, so you don't need to install them yourself.

### Build and run

```bash
git clone https://github.com/donat0806/InfinityDungeon.git
cd InfinityDungeon
cmake -B build
cmake --build build --config Release
```

Then run the executable from the `build` folder (on Windows with MSVC: `build/Release/InfinityDungeon.exe`).

### Controls

| Action | Keys |
|---|---|
| Start a run | Space or Enter |
| Move | W A S D |
| Shoot | Arrow keys (hold two for diagonals) |

### Run the tests

```bash
ctest --test-dir build --output-on-failure -C Release
```

## Roadmap

- [x] Project skeleton: CMake, CI, first test
- [x] Fixed-timestep game loop and scene/state system
- [x] Player movement and shooting
- [x] First enemy types
- [x] Procedural floor generation from a seed
- [ ] Room clearing and locked doors
- [ ] Depth-based difficulty scaling
- [ ] Death screen and score
- [ ] Determinism test (same seed + inputs = same result)
- [ ] Local co-op (two players on one machine)
- [ ] Leaderboard API and in-game submission (Solo and Duo boards)
- [ ] Polish: audio, effects, menus
- [ ] Online co-op (host-authoritative)

## Development notes

I'm building this project with AI assistance (Claude Code) as part of my workflow. I own the architecture, review every change, and require the build and tests to pass in CI before anything is merged. Design decisions and their reasoning are recorded in [`docs/decisions.md`](docs/decisions.md).

## License

MIT

## Author

Made by donat0806. Feedback and ideas are welcome via issues.
