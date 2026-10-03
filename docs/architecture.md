# InfinityDungeon Architecture

This repository is split into:

- `src/sim`: deterministic game simulation logic (no rendering dependencies)
- `src/render`: rendering integration point (raylib wiring can be added here)
- `src/input`: device/input abstraction
- `src/net`: leaderboard API client interface for future online features
- `server`: ASP.NET Core API for accounts and leaderboard backed by PostgreSQL
