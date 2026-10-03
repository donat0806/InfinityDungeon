#pragma once

#include <cstdint>
#include <string>

namespace infinity_dungeon::net {

class LeaderboardClient {
public:
    explicit LeaderboardClient(std::string base_url);
    [[nodiscard]] std::string BuildSubmitPayload(std::string account_name, std::uint32_t deepest_level) const;

private:
    std::string base_url_;
};

} // namespace infinity_dungeon::net
