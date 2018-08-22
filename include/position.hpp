#ifndef GAME_POSITION_H
#define GAME_POSITION_H

#include <array>
#include <numeric>
#include <tuple>
#include <variant>
#include <vector>

namespace game {
/**
 * The position of an entity on the field. Each entity has some sensors at some
 * coordinates on the field. This class abstracts away the number of sensors,
 * providing a single position for the entity.
 *
 * This class also is the base class for the Curiously-Recurring Template
 * Pattern (CRTP) for static polymorphism. @see
 * https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
 *
 * @tparam Derived The derived class.
 */
template <typename Derived> class Position {
public:
  /**
   * @return the position in a (x, y, z) space of this entity.
   */
  std::tuple<double, double, double> vector() const {
    return static_cast<Derived &>(*this).vector();
  }
  /**
   * Add a sensor to this entity to track.
   * @param sid The sensor id.
   */
  void add_sensor(int sid) { static_cast<Derived &>(*this).add_sensor(sid); }
  /**
   * Updates the position of a sensor
   * @param sid The sensor id which position to update.
   * @param vector The new position in a (x, y, z) space of the sensor.
   */
  void update_sensor(int sid, std::tuple<int, int, int> vector) {
    static_cast<Derived &>(*this).update_sensor(sid, vector);
  }
  /**
   * @return the list of sensor ids attached to this entity.
   */
  std::vector<int> const &get_sids() const {
    return static_cast<Derived &>(*this).get_sids();
  }
};

/**
 * The position of a ball on the field. This implements game::Position for a
 * ball.
 */
class BallPosition : public Position<BallPosition> {
public:
  /**
   * Implementation of game::Position::vector().
   *
   * @return the position of the sensor of the ball on the field, if any.
   *         A (inf, inf, inf) vector in case no ball is on the field.
   */
  std::tuple<double, double, double> vector() const;
  /**
   * Implementation of game::Position::update_sensor().
   */
  void update_sensor(int sid, std::tuple<int, int, int> vector);
  /**
   * Implementation of game::Position::add_sensor().
   *
   * The new sensor is set as the ball on the field.
   */
  void add_sensor(int sid) {
    sids.push_back(sid);
    xs.push_back(0);
    ys.push_back(0);
    zs.push_back(0);

    game_ball = sids.size() - 1; // Set last add ball as the game one
  }
  /**
   * Implementation of game::Position::get_sids().
   */
  std::vector<int> const &get_sids() const { return sids; }

private:
  std::size_t game_ball = out_range_idx;
  std::vector<int> sids = {};
  std::vector<int> xs = {};
  std::vector<int> ys = {};
  std::vector<int> zs = {};

  std::size_t sid_index(int sid) const;
  static constexpr auto out_range_idx = std::numeric_limits<std::size_t>::max();
  static constexpr auto inf = std::numeric_limits<double>::max();
};

/**
 * The position of a Player on the field. This implements game::Position for
 * player.
 */
class PlayerPosition : public Position<PlayerPosition> {
public:
  /**
   * Implementation of game::Position::vector().
   *
   * @return a vector (x_hat, y_hat, z_hat) where each entry is the average for
   * that dimension of the values of each sensor.
   */
  std::tuple<double, double, double> vector() const;
  /**
   * Implementation of game::Position::update_sensor().
   */
  void update_sensor(int sid, std::tuple<int, int, int> vector);
  /**
   * Implementation of game::Position::add_sensor().
   */
  void add_sensor(int sid) {
    sids.push_back(sid);
    xs.push_back(0);
    ys.push_back(0);
    zs.push_back(0);
  }
  /**
   * Implementation of game::Position::get_sids().
   */
  std::vector<int> const &get_sids() const { return sids; };

private:
  std::vector<int> sids = {};
  std::vector<int> xs = {};
  std::vector<int> ys = {};
  std::vector<int> zs = {};

  std::size_t sid_index(int sid) const;
};

/**
 * A std::variant of the known derived classes from game::Position to implement
 * static-polymorphism.
 */
using Positions = std::variant<BallPosition, PlayerPosition>;

/**
 * Computes the arithmetic average of values in the range [@p first, @p end).
 *
 * @tparam Iter The iterator type.
 * @param first The iterator begin.
 * @param end The iterator end.
 * @return The arithmetic mean.
 */
template <typename Iter> double mean(Iter first, Iter end) {
  auto sum = std::accumulate(first, end, 0.0);
  auto n = std::distance(first, end);
  return sum / n;
}

} // namespace game

#endif