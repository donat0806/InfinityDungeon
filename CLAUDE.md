# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

InfinityDungeon is a top-down, room-based endless dungeon crawler (inspired by *The Binding of Isaac*). It has two independent halves:

- **C++20 game client** using raylib (root `CMakeLists.txt`, `src/`, `tests/`)
- **ASP.NET Core (.NET 10) leaderboard API** (`server/`), backed by PostgreSQL

The two halves share no build system. They only meet over HTTP: `src/net/LeaderboardClient` builds payloads for `/api/leaderboard/submit`.

The README roadmap tracks progress. Design decisions go in `docs/decisions.md` as dated entries; add an entry when making a notable architectural or tooling choice.

## Commands

### C++ game

```bash
cmake -B build                                     # or: cmake --preset default
cmake --build build --config Release               # or: cmake --build --preset release
ctest --test-dir build --output-on-failure -C Release
ctest --test-dir build -C Release -R "same seed"   # single test, by regex on the TEST_CASE name
build/Release/infinity_tests.exe -tc="same seed*"  # or run doctest directly
```

- On Windows the default generator is Visual Studio (multi-config), so binaries land in `build/Release/` or `build/Debug/`. The game executable is `InfinityDungeon`.
- raylib (`5.5`) and doctest (`v2.4.12`) are fetched by `FetchContent` at configure time; the first configure needs network access.
- Tests use doctest. All test sources are compiled into one `infinity_tests` executable (`tests/test_main.cpp` provides `main`). `doctest_discover_tests` registers each `TEST_CASE` with CTest. To add a test file, list it in the `add_executable(infinity_tests ...)` call in `CMakeLists.txt`.

### Server

```bash
dotnet build server/InfinityDungeon.Server.csproj
dotnet test server/tests/InfinityDungeon.Server.Tests.csproj
dotnet test server/tests/InfinityDungeon.Server.Tests.csproj --filter "FullyQualifiedName~PasswordHasherTests"   # single test
docker compose up --build   # Postgres (5432) + API (8080)
```

CI (`.github/workflows/ci.yml`) builds and tests the game on Ubuntu and Windows, and builds and tests the server on Ubuntu.

## Architecture rules (game)

These rules come from the project plan and exist so that co-op and online play are possible later without a rewrite. New code must follow them:

- **Simulation is separate from presentation.** `src/sim` is plain C++ with no raylib. This is enforced by CMake: only `infinity_render`, `infinity_input` and the executable link raylib, so a raylib include in sim fails to compile. Don't work around this by linking raylib to `infinity_sim`.
- **Input is data.** Each player produces a `PlayerInput` struct per tick (`src/input/PlayerInput.hpp`). Device code (keyboard now; gamepad and network later) only fills that struct, and the sim consumes it.
- **Fixed timestep.** The sim advances at a constant tick rate, independent of frame rate. The current `main.cpp` loop is a placeholder that does not have this yet (roadmap item 2).
- **Players are a collection.** Never assume a single player. Entities reference each other by ID, not raw pointers.
- **Deterministic, seeded randomness.** Gameplay RNG lives in the game state; a seed plus an input sequence must reproduce a run exactly. Don't use `std::random_device`, wall-clock time, or global RNG in sim code. `tests/sim_determinism_test.cpp` guards this.
- **Data-driven balance.** Enemy stats and difficulty scaling are meant to be loaded from config files, not hard-coded.

Each `src/` folder is its own static library (`infinity_sim`, `infinity_input`, `infinity_render`, `infinity_net`) with `src/` as a public include directory, so includes are written as `"sim/DungeonSimulator.hpp"`. Code lives in namespaces `infinity_dungeon::<module>`. `render::Renderer` owns the raylib window (RAII).

## Server

- All endpoints are minimal APIs in `server/Program.cs` using raw `Npgsql` commands (no ORM, no DI-registered services).
- The connection string comes from `ConnectionStrings:Postgres` config, falling back to the `POSTGRES_CONNECTION` env var. Startup fails without one.
- `Services/DatabaseInitializer.cs` creates the schema at startup with `CREATE TABLE IF NOT EXISTS`. There are no migrations, so changes to existing tables must be handled manually.
- `leaderboard_entries.account_name` is a foreign key to `accounts.name`. Endpoints map Postgres error codes to HTTP responses: `23505` (unique violation) → 409 and `23503` (foreign-key violation) → 400.
- The test project lives in `server/tests/` and references the server project. The server `.csproj` excludes `tests/**/*.cs` from compilation; keep that exclusion if restructuring.
