#ifndef GAME_CONTEXT_H
#define GAME_CONTEXT_H

#include "metadata.hpp"
#include "position.hpp"

#include <functional>
#include <unordered_map>
#include <vector>

namespace game {
class Context {
public:
  void set_player_map(PlayerMap const &players) { this->players = players; }
  void set_player_map(PlayerMap &&players) {
    this->players = std::move(players);
  }

  PlayerMap &get_players() { return players; }
  PlayerMap const &get_players() const { return players; }

  void set_team_map(TeamMap const &teams) { this->teams = teams; }
  void set_team_map(TeamMap &&teams) { this->teams = std::move(teams); }

  TeamMap &get_teams() { return teams; }
  TeamMap const &get_teams() const { return teams; }

  void set_ball_map(BallMap const &balls) { this->balls = balls; }
  void set_ball_map(BallMap const &&balls) { this->balls = std::move(balls); }

  BallMap &get_balls() { return balls; }
  BallMap const &get_balls() const { return balls; }

  void add_position(Positions const &position, std::vector<int> const &sids) {
    positions.push_back(position);
    auto idx = positions.size() - 1;
    for (int sid : sids) {
      position_sids.insert({sid, idx});
    }
  }

  Positions &get_position(int sid) { return positions[position_sids.at(sid)]; }

private:
  PlayerMap players = {};
  TeamMap teams = {};
  BallMap balls = {};

  std::vector<Positions> positions = {};
  std::unordered_map<int, std::vector<Positions>::size_type> position_sids = {};
};
} // namespace game

#endif