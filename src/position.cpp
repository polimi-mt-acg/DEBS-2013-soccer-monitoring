#include "position.hpp"
#include "event.hpp"

#include <algorithm>
#include <exception>
#include <position.hpp>

#include "fmt/format.h"

namespace game {

// ==-----------------------------------------------------------------------==
//                        PlayerPosition implementation
// ==-----------------------------------------------------------------------==

void PlayerPosition::update_sensor(int sid, std::tuple<int, int, int> vector) {
  auto [x, y, z] = vector;
  auto idx = sid_index(sid);
  xs[idx] = x;
  ys[idx] = y;
  zs[idx] = z;
}

std::size_t PlayerPosition::sid_index(int sid) const {
  if (auto it = std::find(sids.cbegin(), sids.cend(), sid); it != sids.cend()) {
    return std::distance(sids.cbegin(), it);
  } else {
    throw std::out_of_range{fmt::format("Unknown sid {}", sid)};
  }
}

std::tuple<double, double, double> PlayerPosition::vector() const {
  auto x_bar = mean(xs.cbegin(), xs.cend());
  auto y_bar = mean(ys.cbegin(), ys.cend());
  auto z_bar = mean(zs.cbegin(), zs.cend());

  return {x_bar, y_bar, z_bar};
}

// ==-----------------------------------------------------------------------==
//                        BallPosition implementation
// ==-----------------------------------------------------------------------==

std::size_t BallPosition::sid_index(int sid) const {
  if (auto it = std::find(sids.cbegin(), sids.cend(), sid); it != sids.cend()) {
    return std::distance(sids.cbegin(), it);
  } else {
    throw std::out_of_range{fmt::format("Unknown sid {}", sid)};
  }
}

void BallPosition::update_sensor(int sid, std::tuple<int, int, int> vector) {
  // Update sensor position
  auto [x, y, z] = vector;
  auto idx = sid_index(sid);
  xs[idx] = x;
  ys[idx] = y;
  zs[idx] = z;

  // If sensor inside the field, set as game ball
  if (field_lower_x <= x && x <= field_upper_x && field_lower_y <= y &&
      y <= field_upper_y) {
    game_ball = idx;
  }

  // If the game ball goes outside, it's no longer the game ball
  auto is_outside = x < field_lower_x || x > field_upper_x ||
                    y < field_lower_y || y > field_upper_y;
  if (idx == game_ball && is_outside) {
    game_ball = out_range_idx;
  }
}

std::tuple<double, double, double> BallPosition::vector() const {
  if (game_ball != out_range_idx) {
    return {xs[game_ball], ys[game_ball], zs[game_ball]};
  } else {
    return {inf, inf, inf};
  }
}

void update_sensor_position(game::Positions &position,
                            game::PositionEvent const &position_event) {
  std::visit(
      [&position_event](auto &&pos) {
        pos.update_sensor(position_event.get_sid(),
                          position_event.get_vector());
      },
      position);
}
} // namespace game