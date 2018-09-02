
#include <context.hpp>

#include "context.hpp"

namespace game {

Context Context::build_from(std::filesystem::path const &metadata) {
  auto meta = game::parse_metadata_file(metadata.string());
  auto &players = meta.players;
  auto &teams = meta.teams;
  auto &balls = meta.balls;

  auto context = game::Context{};
  context.set_player_map(players);
  context.set_team_map(teams);
  context.set_ball_map(balls);

  for (auto &position : meta.positions) {
    auto sids = std::visit([](auto &&pos) { return pos.get_sids(); }, position);
    context.add_position(position, sids);
  }
  return context;
}

Context Context::build_from(std::string const &metadata) {
  auto meta = game::parse_metadata_string(metadata);
  auto &players = meta.players;
  auto &teams = meta.teams;
  auto &balls = meta.balls;

  auto context = game::Context{};
  context.set_player_map(players);
  context.set_team_map(teams);
  context.set_ball_map(balls);

  for (auto &position : meta.positions) {
    auto sids = std::visit([](auto &&pos) { return pos.get_sids(); }, position);
    context.add_position(position, sids);
  }
  return context;
}

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
