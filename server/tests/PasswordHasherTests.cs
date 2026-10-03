using InfinityDungeon.Server.Services;
using Xunit;

namespace InfinityDungeon.Server.Tests;

public class PasswordHasherTests
{
    [Fact]
    public void HashAndVerifyRoundtrip()
    {
        const string password = "hunter2";
        var hash = PasswordHasher.Hash(password);

        Assert.True(PasswordHasher.Verify(password, hash));
        Assert.False(PasswordHasher.Verify("wrong", hash));
    }
}
