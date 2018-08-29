#include "context.hpp"

namespace game {

Snapshot Context::take_snapshot() const {
  auto snapshot = std::unordered_map<std::string, Positions>{};
  auto players = get_player_names();
  for (auto const &player : players) {
    auto sid = get_player_sids(player).front();
    snapshot[player] = get_position(sid);
  }

  snapshot["Ball"] = get_ball_position();
  return snapshot;
}

void Context::add_position(Positions const &position,
                           std::vector<int> const &sids) {
  positions.push_back(position);
  auto idx = positions.size() - 1;
  for (int sid : sids) {
    position_sids.insert({sid, idx});
  }

  if (std::holds_alternative<BallPosition>(position)) {
    ball_position_idx = idx;
  }
}
} // namespace game
