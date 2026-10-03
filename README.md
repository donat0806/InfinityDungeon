# InfinityDungeon

Infinite dungeon crawler scaffold with:

- C++ game core using CMake with modular folders (`src/sim`, `src/render`, `src/input`, `src/net`)
- ASP.NET Core API (`server/`) for accounts and leaderboard
- PostgreSQL + Docker compose setup for backend services
- CI workflow to build and test C++ and .NET components

## Local quickstart

### Game build/test

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

### Server run

```bash
docker compose up --build
```

API endpoints:

- `POST /api/accounts/register`
- `POST /api/leaderboard/submit`
- `GET /api/leaderboard?limit=10`
