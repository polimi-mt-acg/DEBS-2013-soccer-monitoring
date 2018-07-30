#ifndef GAME_POSITION_H
#define GAME_POSITION_H

#include <array>
#include <numeric>
#include <tuple>
#include <variant>
#include <vector>

namespace game {
template <typename Derived> class Position {
public:
  std::tuple<double, double, double> vector() const {
    return static_cast<Derived &>(*this).vector();
  }

  void add_sensor(int sid) { static_cast<Derived &>(*this).add_sensor(sid); }

  void update_sensor(int sid, std::tuple<int, int, int> vector) {
    static_cast<Derived &>(*this).update_sensor(sid, vector);
  }

  std::vector<int> const&get_sids() const {
    return static_cast<Derived &>(*this).get_sids();
  }
};

class BallPosition : public Position<BallPosition> {
public:
  std::tuple<double, double, double> vector() const {
    return {xs[game_ball], ys[game_ball], zs[game_ball]};
  }

  void update_sensor(int sid, std::tuple<int, int, int> vector);

  void add_sensor(int sid) {
    sids.push_back(sid);
    xs.push_back(0);
    ys.push_back(0);
    zs.push_back(0);

    game_ball = sids.size() - 1; // Set last add ball as the game one
  }

  std::vector<int> const &get_sids() const { return sids; }

private:
  std::size_t game_ball = -1;
  std::vector<int> sids = {};
  std::vector<int> xs = {};
  std::vector<int> ys = {};
  std::vector<int> zs = {};

  std::size_t sid_index(int sid) const;
};

class PlayerPosition : public Position<PlayerPosition> {
public:
  std::tuple<double, double, double> vector() const;
  void update_sensor(int sid, std::tuple<int, int, int> vector);
  void add_sensor(int sid) {
    sids.push_back(sid);
    xs.push_back(0);
    ys.push_back(0);
    zs.push_back(0);
  }
  std::vector<int> const &get_sids() const { return sids; };

private:
  std::vector<int> sids = {};
  std::vector<int> xs = {};
  std::vector<int> ys = {};
  std::vector<int> zs = {};

  std::size_t sid_index(int sid) const;
};

using Positions = std::variant<BallPosition, PlayerPosition>;

template <typename Iter> double mean(Iter first, Iter end) {
  auto sum = std::accumulate(first, end, 0.0f);
  auto n = std::distance(first, end);
  return static_cast<double>(sum) / n;
}

} // namespace game

#endif