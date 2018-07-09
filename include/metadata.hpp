#ifndef GAME_MAPS_H
#define GAME_MAPS_H

#include <string>
#include <unordered_map>
#include <vector>

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

private:
  std::unordered_map<int, std::string> player = {};
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
} // namespace game

#endif