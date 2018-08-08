#ifndef GAME_MAPS_H
#define GAME_MAPS_H

#include <regex>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "fmt/format.h"

#include "position.hpp"
#include "stream_types.hpp"

namespace game {

/**
 * @brief A mapping between sensor-ids and players owning those sensors.
 */
class PlayerMap {
public:
  /**
   * @brief Stores a mapping between a sensor and a player. If the sensor is
   * already registered, the player is not added.
   *
   * @param sensor_id The sensor id of a sensor of @p player.
   * @param player The player wearing the sensor identified by @p sensor_id.
   */
  void add_sensor(int sensor_id, std::string const &player);

  /**
   * @brief Checks if a sensor is registered as worn by a player.
   *
   * @param sensor_id The sensor id.
   * @return true If a sensor with @p sensor_id is registered.
   * @return false otherwise.
   */
  bool is_player(int sensor_id) const;

  /**
   * @brief Get the player wearing the passed sensor (if any).
   *
   * @param sensor_id The sensor id.
   * @return std::string const& The player wearing the sensor.
   * @throws std::out_of_range if no player is wearing a sensor with @p
   * sensor_id.
   */
  std::string const &operator[](int const &sensor_id) const;

  /**
   * @return the list of all player names.
   */
  std::vector<std::string> get_player_names() const;

  /**
   * @param name The player name.
   * @return the list of sensor ids attached to player @p name.
   */
  std::vector<int> const &get_player_sids(std::string const &name) const;

private:
  std::unordered_map<int, std::string> player = {};
  std::unordered_map<std::string, std::vector<int>> sids = {};
};

/**
 * Enumerates the team names.
 */
enum class Team { A = 1, B };

/**
 * Operator<< overload for game::Team.
 */
std::ostream &operator<<(std::ostream &os, Team team);

/**
 * A mapping between players and the team they play in.
 */
class TeamMap {
public:
  /**
   * Registers a player in a team.
   * @param player The player name.
   * @param team The team the player plays in.
   */
  void add_player(std::string const &player, Team team);

  /**
   * Get the team of a player (if registered).
   *
   * @param player The player name.
   * @return The team the player plays in.
   * @throws std::out_of_range if no player with name @p player is registered.
   */
  Team operator[](std::string const &player) const;

private:
  std::vector<std::string> team_A = {};
  std::vector<std::string> team_B = {};
};

/**
 * @brief A mapping between sensor-ids and balls wrapping those sensors.
 */
class BallMap {
public:
  /**
   * @brief Checks if a sensor is registered as wrapped by a ball.
   *
   * @param sensor_id The sensor id.
   * @return true If a sensor with @p sensor_id is registered.
   * @return false otherwise.
   */
  bool is_ball(int sensor_id) const;

  /**
   * @brief Register that a sensor is wrapped by a ball.
   *
   * @param sensor_id The sensor id.
   */
  void add_ball(int sensor_id);

private:
  std::vector<int> balls = {};
};

/**
 * The game metadata.
 */
struct Metadata {
  PlayerMap players; ///< The game players names and sensors.
  TeamMap teams;     ///< The game teams.
  BallMap balls;     ///< The game balls and sensors.

  ///< The positions of players and balls on the field.
  std::vector<Positions> positions;
};

/**
 * Constants for parsing metadata from dataset.
 */
struct ParseMetadata {
  static const std::regex ball_re;
  static constexpr auto ball_re_sid_idx = 2;

  static const std::regex player_re;
  static constexpr auto player_re_team_idx = 1;
  static constexpr auto player_re_name_idx = 2;
  static constexpr auto player_re_sid_start_idx = 3;
  static constexpr auto player_re_sid_end_idx = 6;
};

/**
 * An error reporting that a given file could not be found on the file system.
 */
struct file_not_found_error : public std::runtime_error {
  explicit file_not_found_error(std::string path)
      : runtime_error(""), path{std::move(path)} {}

  const char *what() const throw() {
    auto str = fmt::format("File {} not found.", path);
    return strdup(str.c_str());
  }

private:
  std::string path;
};

/**
 * Parses metadata from a file.
 * @param path The metadata file path.
 * @return The game metadata.
 */
Metadata parse_metadata_file(std::string const &path);

/**
 * Parses metadata from a string.
 * @param path The metadata string.
 * @return The game metadata.
 */
Metadata parse_metadata_string(std::string const &metadata);
} // namespace game

#endif