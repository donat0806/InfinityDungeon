using InfinityDungeon.Server.Models;
using InfinityDungeon.Server.Services;
using Npgsql;

var builder = WebApplication.CreateBuilder(args);

var connectionString = builder.Configuration.GetConnectionString("Postgres")
                       ?? Environment.GetEnvironmentVariable("POSTGRES_CONNECTION")
                       ?? throw new InvalidOperationException("Missing PostgreSQL connection string.");

await DatabaseInitializer.EnsureSchemaAsync(connectionString);

var app = builder.Build();

app.MapGet("/health", () => Results.Ok(new { status = "ok" }));

app.MapPost("/api/accounts/register", async (AccountRegisterRequest request) =>
{
    if (string.IsNullOrWhiteSpace(request.Name) || string.IsNullOrWhiteSpace(request.Password))
    {
        return Results.BadRequest("Name and password are required.");
    }

    await using var connection = new NpgsqlConnection(connectionString);
    await connection.OpenAsync();

    const string sql = "INSERT INTO accounts(name, password_hash) VALUES (@name, @hash);";
    await using var command = new NpgsqlCommand(sql, connection);
    command.Parameters.AddWithValue("name", request.Name);
    command.Parameters.AddWithValue("hash", PasswordHasher.Hash(request.Password));

    try
    {
        await command.ExecuteNonQueryAsync();
        return Results.Created($"/api/accounts/{request.Name}", new { request.Name });
    }
    catch (PostgresException ex) when (ex.SqlState == "23505")
    {
        return Results.Conflict("Account already exists.");
    }
});

app.MapPost("/api/leaderboard/submit", async (LeaderboardSubmitRequest request) =>
{
    if (string.IsNullOrWhiteSpace(request.AccountName) || request.DeepestLevel < 0)
    {
        return Results.BadRequest("AccountName and non-negative DeepestLevel are required.");
    }

    await using var connection = new NpgsqlConnection(connectionString);
    await connection.OpenAsync();

    const string sql = "INSERT INTO leaderboard_entries(account_name, deepest_level) VALUES (@accountName, @deepestLevel);";
    await using var command = new NpgsqlCommand(sql, connection);
    command.Parameters.AddWithValue("accountName", request.AccountName);
    command.Parameters.AddWithValue("deepestLevel", request.DeepestLevel);

    try
    {
        await command.ExecuteNonQueryAsync();
        return Results.Ok();
    }
    catch (PostgresException ex) when (ex.SqlState == "23503")
    {
        return Results.BadRequest("Unknown account. Register account before submitting leaderboard scores.");
    }
});

app.MapGet("/api/leaderboard", async (int? limit) =>
{
    var take = Math.Clamp(limit ?? 10, 1, 100);

    await using var connection = new NpgsqlConnection(connectionString);
    await connection.OpenAsync();

    const string sql = """
        SELECT account_name, MAX(deepest_level) AS deepest_level
        FROM leaderboard_entries
        GROUP BY account_name
        ORDER BY deepest_level DESC, account_name ASC
        LIMIT @limit;
        """;

    await using var command = new NpgsqlCommand(sql, connection);
    command.Parameters.AddWithValue("limit", take);

    await using var reader = await command.ExecuteReaderAsync();
    var entries = new List<LeaderboardEntry>();

    while (await reader.ReadAsync())
    {
        entries.Add(new LeaderboardEntry(reader.GetString(0), reader.GetInt32(1)));
    }

    return Results.Ok(entries);
});

app.Run();
