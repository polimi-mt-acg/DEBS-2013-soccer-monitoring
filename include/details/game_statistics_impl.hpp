#ifndef SOCCER_MONITORING_GAME_STATISTICS_IMPL_HPP
#define SOCCER_MONITORING_GAME_STATISTICS_IMPL_HPP

#include <string>
#include <vector>

namespace game {
namespace details {

/**
 * The distances vector for a player. Creates a mapping between a player and its
 * distances from the ball.
 */
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
template <bool IsConst> class ball_possession_iterator; // Forward declared

class BallPossession {
  friend class details::ball_possession_iterator<true>;
  friend class details::ball_possession_iterator<false>;

  using const_iterator = details::ball_possession_iterator<true>;
  using iterator = details::ball_possession_iterator<false>;

public:
  static constexpr auto infinite_distance = std::numeric_limits<double>::max();
  static const std::string none_player;

  void reduce(details::DistanceResults const &distance);

  iterator begin();
  const_iterator cbegin() const;
  iterator end();
  const_iterator cend() const;

private:
  std::vector<std::string> closest_players = {};
  std::vector<double> min_distances = {};
};

// ==-----------------------------------------------------------------------==
//                        Ball Possession Iterator
// ==-----------------------------------------------------------------------==
template <bool IsConst> class ball_possession_iterator {
public:
  using difference_type = std::ptrdiff_t;
  using value_type = std::pair<double, std::string>;
  using reference = std::conditional_t<IsConst, const value_type, value_type> &;
  using pointer = std::add_pointer_t<reference>;
  using iterator_category = std::input_iterator_tag;

  using iterator = ball_possession_iterator<IsConst>;

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

  pointer operator->() const { return &current_value; }

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
} // namespace game

#endif // SOCCER_MONITORING_GAME_STATISTICS_IMPL_HPP
