#ifndef GAME_CONTEXT_H
#define GAME_CONTEXT_H

#include "metadata.hpp"
#include "position.hpp"

#include <filesystem>
#include <functional>
#include <unordered_map>
#include <vector>

namespace game {
/**
 * A snapshot of the position of players and ball on the field. It maps a player
 * name to its position. Additionally, it maps the key "Ball" to the ball
 * position.
 */
using Snapshot = std::unordered_map<std::string, Positions>;

/**
 * The game monitoring context. This class carries the state of the game
 * monitoring while the dataset is streamed and statistics are computed.
 *
 * Specifically, the following structures are available through the Context
 * interface: a) game::PlayerMap for accessing players and sensors b)
 * game::TeamMap for accessing teams c) game::BallMap for accessing balls
 * sensors d) game::Positions of the players and the ball on the field.
 */
class Context {
protected:
  Context() = default;

public:
  static Context build_from(std::filesystem::path const &metadata);
  static Context build_from(std::string const &metadata);
  /**
   * Store a copy of input game::PlayerMap
   * @param players the players map
   */
  void set_player_map(PlayerMap const &players) { this->players = players; }
  /**
   * Store a move-constructed copy of input game::PlayerMap
   * @param players the players map
   */
  void set_player_map(PlayerMap &&players) {
    this->players = std::move(players);
  }
  /**
   * @return the game::PlayerMap
   */
  PlayerMap &get_players() { return players; }
  /**
   * @return the game::PlayerMap
   */
  PlayerMap const &get_players() const { return players; }
  /**
   * @return the list of player names
   */
  std::vector<std::string> get_player_names() const {
    return players.get_player_names();
  }
  /**
   * @param name The player name
   * @return the list of sensor ids attached to @p name player
   */
  std::vector<int> const &get_player_sids(std::string const &name) const {
    return players.get_player_sids(name);
  }
  /**
   * Store a copy of input game::TeamMap
   * @param teams the teams map
   */
  void set_team_map(TeamMap const &teams) { this->teams = teams; }
  /**
   * Store a move-constructed copy of input game::TeamMap
   * @param teams the teams map
   */
  void set_team_map(TeamMap &&teams) { this->teams = std::move(teams); }
  /**
   * @return the game::TeamMap
   */
  TeamMap &get_teams() { return teams; }
  /**
   * @return the game::TeamMap
   */
  TeamMap const &get_teams() const { return teams; }
  /**
   * Store a copy of input game::BallMap
   * @param balls the balls map
   */
  void set_ball_map(BallMap const &balls) { this->balls = balls; }
  /**
   * Store a move-constructed copy of input game::BallMap
   * @param balls the balls map
   */
  void set_ball_map(BallMap &&balls) { this->balls = std::move(balls); }
  /**
   * @return the game::BallMap
   */
  BallMap &get_balls() { return balls; }
  /**
   * @return the game::BallMap
   */
  BallMap const &get_balls() const { return balls; }
  /**
   * Store a the position for an entity wearing a set of sensors
   * @param position the entity game::Position
   * @param sids the list of sensor ids attached to the entity
   */
  void add_position(Positions const &position, std::vector<int> const &sids);
  /**
   * @param sid the sensor id
   * @return the game::Position of the entity wearing the input sensor @p sid.
   */
  Positions &get_position(int sid) { return positions[position_sids.at(sid)]; }
  /**
   * @param sid the sensor id
   * @return the game::Position of the entity wearing the input sensor @p sid.
   */
  Positions const &get_position(int sid) const {
    return positions[position_sids.at(sid)];
  }
  /**
   * @return the game::Position of the ball
   */
  Positions &get_ball_position() { return positions[ball_position_idx]; }
  /**
   * @return the game::Position of the ball
   */
  Positions const &get_ball_position() const {
    return positions[ball_position_idx];
  }
  /**
   * Takes a Snapshot of the Position of players and ball on the field. @see
   * game::Snapshot for details.
   * @return the Snapshot of the field
   */
  Snapshot take_snapshot() const;

private:
  PlayerMap players = {};
  TeamMap teams = {};
  BallMap balls = {};

  std::vector<Positions> positions = {};
  std::vector<Positions>::size_type ball_position_idx = {};
  std::unordered_map<int, std::vector<Positions>::size_type> position_sids = {};
};
} // namespace game

#endif