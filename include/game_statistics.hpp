#include <utility>

//
// Created by leonardoarcari on 27/07/18.
//

#ifndef SOCCER_MONITORING_GAME_STATISTICS_H
#define SOCCER_MONITORING_GAME_STATISTICS_H

#include "context.hpp"
#include "event.hpp"

#include <limits>
#include <unordered_map>
#include <vector>

namespace game {
class GameStatistics {
public:
  explicit GameStatistics(double maximum_distance, Context &context)
      : maximum_distance{maximum_distance}, context{context} {
    player_names = context.get_player_names();
  }

  void batch_stats(std::vector<PositionEvent> const &batch,
                   bool period_last_batch = false);

  std::unordered_map<std::string, double> partial_stats() const;

  static constexpr auto infinite_distance = std::numeric_limits<double>::max();

private:
  Context &context;
  double maximum_distance = 0.0;
  std::vector<std::string> player_names = {};
  std::unordered_map<std::string, int> accumulator = {};
};

class DistanceResults {
public:
  explicit DistanceResults(std::string player)
      : player_name{std::move(player)} {}

  void push_back(double distance) { distances.push_back(distance); }
  std::string const &get_player_name() const { return player_name; }

  using size_type = typename std::vector<double>::size_type;
  using iterator = typename std::vector<double>::iterator;
  using const_iterator = typename std::vector<double>::const_iterator;

  double const &operator[](size_type index) const { return distances[index]; }

  iterator begin() { return distances.begin(); }
  const_iterator cbegin() const { return distances.cbegin(); }

  iterator end() { return distances.end(); }
  const_iterator cend() const { return distances.cend(); }

  size_type size() const { return distances.size(); }

private:
  std::string player_name;
  std::vector<double> distances = {};
};

// ==-----------------------------------------------------------------------==
//                              Ball Possession
// ==-----------------------------------------------------------------------==
namespace details {
template <typename BallPossession, bool IsConst>
class ball_possession_iterator {
public:
  using difference_type = std::ptrdiff_t;
  using value_type = std::pair<double, std::string>;
  using reference = std::conditional_t<IsConst, const value_type, value_type> &;
  using pointer = std::add_pointer_t<reference>;
  using iterator_category = std::input_iterator_tag;

  using iterator = ball_possession_iterator<BallPossession, IsConst>;

  explicit ball_possession_iterator(BallPossession const &ballPossession,
                                    std::size_t index = 0)
      : bp{ballPossession}, index{index} {
    if (index < bp.min_distances.size()) {
      current_value =
          std::make_pair(bp.min_distances[index], bp.closest_players[index]);
    } else {
      current_value = std::make_pair(-1, "iterator_end");
    }
  }

  ball_possession_iterator(iterator const &other)
      : bp{other.bp}, index{other.index}, current_value{other.current_value} {}

  iterator &operator++() {
    ++index;
    if (index < bp.min_distances.size()) {
      current_value =
          std::make_pair(bp.min_distances[index], bp.closest_players[index]);
    } else {
      current_value = std::make_pair(-1, "iterator_end");
    }
    return *this;
  }

  reference operator*() { return current_value; }
  value_type operator*() const { return current_value; }

  iterator operator++(int) {
    auto it = *this;
    this->operator++();
    return it;
  }

  pointer operator->() const { return this; }

  iterator &operator=(const iterator &other) {
    bp = other.bp;
    index = other.index;
    current_value = other.current_value;
    return *this;
  }

  friend bool operator==(iterator const &lhs, iterator const &rhs) {
    return lhs.index == rhs.index;
  }

  friend bool operator!=(iterator const &lhs, iterator const &rhs) {
    return !(lhs == rhs);
  }

private:
  BallPossession const &bp;
  std::size_t index;
  std::pair<double, std::string> current_value;
};
} // namespace details

class BallPossession {
  friend class details::ball_possession_iterator<BallPossession, true>;
  friend class details::ball_possession_iterator<BallPossession, false>;

  using const_iterator =
      details::ball_possession_iterator<BallPossession, true>;
  using iterator = details::ball_possession_iterator<BallPossession, false>;

public:
  static constexpr auto infinite_distance = std::numeric_limits<double>::max();
  static const std::string none_player;

  void reduce(DistanceResults const &distance);

  iterator begin() { return iterator{*this}; }
  const_iterator cbegin() const { return const_iterator{*this}; }

  iterator end() { return iterator{*this, min_distances.size()}; }
  const_iterator cend() const {
    return const_iterator{*this, min_distances.size()};
  }

private:
  std::vector<std::string> closest_players = {};
  std::vector<double> min_distances = {};
};

} // namespace game
#endif // SOCCER_MONITORING_GAME_STATISTICS_H
