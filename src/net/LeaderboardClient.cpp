#include "LeaderboardClient.hpp"

namespace infinity_dungeon::net {

LeaderboardClient::LeaderboardClient(std::string base_url) : base_url_(std::move(base_url)) {}

std::string LeaderboardClient::BuildSubmitPayload(std::string account_name, std::uint32_t deepest_level) const {
    return "{\"baseUrl\":\"" + base_url_ + "\",\"accountName\":\"" + account_name + "\",\"deepestLevel\":" +
           std::to_string(deepest_level) + "}";
}

} // namespace infinity_dungeon::net
