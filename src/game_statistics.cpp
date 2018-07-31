//
// Created by leonardoarcari on 27/07/18.
//

#include "game_statistics.hpp"
#include "distance.hpp"

#include <string>

namespace game {
using namespace std::literals;

void GameStatistics::batch_stats(const std::vector<PositionEvent> &batch,
                                 bool period_last_batch) {
  auto ball_possession = BallPossession{};

  // For each player, scan the batch only for events of sensors worn by that
  // player
#pragma omp parallel for
  for (std::size_t i = 0; i < player_names.size(); ++i) {
    auto const &name = player_names[i];
    auto sids = context.get_player_sids(name);
    auto &position = context.get_position(sids.front());
    auto &ball_position = context.get_ball_position();

    auto mine = [&sids](int sid) {
      return std::find(sids.cbegin(), sids.cend(), sid) != sids.cend();
    };

    auto as_vector = [](auto &&pos) -> std::tuple<float, float, float> {
      return std::visit([](auto &&p) { return p.vector(); }, pos);
    };

    auto distances = DistanceResults{name};
    for (auto const &event : batch) {
      auto event_sid = event.get_sid();

      // If player sensor, check if mine
      if (context.get_players().is_player(event_sid)) {
        // If mine, update my position
        if (mine(event_sid)) {
          std::visit(
              [&event, event_sid](auto &&pos) {
                pos.update_sensor(event_sid, event.get_vector());
              },
              position);
        }
      }

      // If ball sensor:
      if (context.get_balls().is_ball(event_sid)) {
        // Update local ball position
        std::visit(
            [&event, event_sid](auto &&pos) {
              pos.update_sensor(event_sid, event.get_vector());
            },
            ball_position);

        // Compute ball possession
        auto distance =
            distance::euclidean(as_vector(ball_position), as_vector(position));

        // If distance is within maximum distance, add it. Otherwise set
        // distance to infinity
        // TODO: Make conversion more visible
        if (distance / 1000 <= maximum_distance) {
          distances.push_back(distance);
        } else {
          distances.push_back(infinite_distance);
        }
      }
    }

#pragma omp critical
    { ball_possession.reduce(distances); };
  }

  // Update partial statistics
  for (auto const &[d, player_name] : ball_possession) {
    if (player_name != BallPossession::none_player) {
      ++accumulator[player_name];
    }
  }

  // If last batch for this period, output partial statistics
  if (period_last_batch) {
    partials.push_back(partial_stats());
    accumulator.clear();
  }
}

std::unordered_map<std::string, double> GameStatistics::partial_stats() const {
  auto total =
      std::accumulate(accumulator.cbegin(), accumulator.cend(), 0,
                      [](int acc, auto &&pair) { return acc + pair.second; });

  auto partials = std::unordered_map<std::string, double>();

  for (auto const &[name, nb_possessions] : accumulator) {
    partials.insert({name, static_cast<double>(nb_possessions) / total});
  }
  return partials;
}

const std::string BallPossession::none_player = "None"s;

void BallPossession::reduce(DistanceResults const &distance) {
  if (min_distances.empty()) {
    // Initialize internal containers
    min_distances.insert(min_distances.begin(), distance.cbegin(),
                         distance.cend());

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
} // namespace game
