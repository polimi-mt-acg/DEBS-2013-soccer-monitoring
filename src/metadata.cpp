#include "metadata.hpp"
#include <algorithm>

namespace game {
void PlayerMap::add_sensor(int sensor_id, std::string const &player) {
  this->player.insert({sensor_id, player});
}

bool PlayerMap::is_player(int sensor_id) const {
  auto search = player.find(sensor_id);
  return search != player.end();
}

std::string const &PlayerMap::operator[](int const &sensor_id) const {
  return player.at(sensor_id);
}

void BallMap::add_ball(int sensor_id) { balls.push_back(sensor_id); }

bool BallMap::is_ball(int sensor_id) const {
  auto search = std::find(balls.cbegin(), balls.cend(), sensor_id);
  return search != balls.cend();
}

} // namespace game