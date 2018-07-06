#ifndef GAME_MAPS_H
#define GAME_MAPS_H

#include <string>
#include <unordered_map>
#include <vector>

namespace game {

class PlayerMap {
public:
  void add_sensor(int sensor_id, std::string const &player);
  bool is_player(int sensor_id) const;
  std::string const &operator[](int const &sensor_id) const;

private:
  std::unordered_map<int, std::string> player = {};
};

class BallMap {
  bool is_ball(int sensor_id) const;
  void add_ball(int sensor_id);

private:
  std::vector<int> balls = {};
};
} // namespace game

#endif