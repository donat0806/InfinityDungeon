using Npgsql;

namespace InfinityDungeon.Server.Services;

internal static class DatabaseInitializer
{
    public static async Task EnsureSchemaAsync(string connectionString)
    {
        await using var connection = new NpgsqlConnection(connectionString);
        await connection.OpenAsync();

        const string sql = """
            CREATE TABLE IF NOT EXISTS accounts (
                id SERIAL PRIMARY KEY,
                name TEXT NOT NULL UNIQUE,
                password_hash TEXT NOT NULL,
                created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
            );

            CREATE TABLE IF NOT EXISTS leaderboard_entries (
                id SERIAL PRIMARY KEY,
                account_name TEXT NOT NULL REFERENCES accounts(name),
                deepest_level INTEGER NOT NULL CHECK (deepest_level >= 0),
                created_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
            );
            """;

        await using var command = new NpgsqlCommand(sql, connection);
        await command.ExecuteNonQueryAsync();
    }
}
