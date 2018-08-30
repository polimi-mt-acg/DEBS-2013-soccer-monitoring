#include "details/game_statistics_impl.hpp"

namespace game {
namespace details {
using namespace std::string_literals;

const std::string BallPossession::none_player = "None"s;

void BallPossession::reduce(DistanceResults const &distance) {
  if (min_distances.empty()) {
    // Initialize internal containers
    std::copy(distance.cbegin(), distance.cend(),
              std::back_inserter(min_distances));

    for (auto const &d : min_distances) {
      if (d == infinite_distance) {
        closest_players.push_back(none_player);
      } else {
        closest_players.push_back(distance.get_player_name());
      }
    }
  } else {
    for (std::size_t i = 0; i < min_distances.size(); ++i) {
      if (distance[i] < min_distances[i]) {
        min_distances[i] = distance[i];
        closest_players[i] = distance.get_player_name();
      }
    }
  }
}

BallPossession::iterator BallPossession::begin() { return iterator{*this}; }
BallPossession::const_iterator BallPossession::cbegin() const {
  return const_iterator{*this};
}
BallPossession::iterator BallPossession::end() {
  return iterator{*this, min_distances.size()};
}
BallPossession::const_iterator BallPossession::cend() const {
  return const_iterator{*this, min_distances.size()};
}
} // namespace details
} // namespace game
