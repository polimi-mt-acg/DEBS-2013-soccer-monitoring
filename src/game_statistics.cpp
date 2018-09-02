//
// Created by leonardoarcari on 27/07/18.
//

#include "game_statistics.hpp"
#include "distance.hpp"

#include <batch.hpp>
#include <game_statistics.hpp>
#include <string>

namespace game {
using namespace std::literals;

void GameStatistics::accumulate_stats(const game::Batch &batch) {
  auto ball_possession = details::BallPossession{};

  // For each player, scan the batch only for events of sensors worn by that
  // player
#pragma omp parallel for shared(context, ball_possession)
  for (std::size_t i = 0; i < player_names.size(); ++i) {
    auto const &name = player_names[i];
    auto const &sids = context.get_player_sids(name);
    auto ball_position = batch.snapshot.at("Ball");
    auto position = batch.snapshot.at(name);

    auto mine = [&sids](int sid) {
      return std::find(sids.cbegin(), sids.cend(), sid) != sids.cend();
    };

    auto as_vector = [](auto &&pos) -> std::tuple<double, double, double> {
      return std::visit([](auto &&p) { return p.vector(); }, pos);
    };

    auto distances = details::DistanceResults{name};
    for (auto const &event : *batch.data) {
      auto event_sid = event.get_sid();

      // If player sensor, check if mine
      if (context.get_players().is_player(event_sid)) {
        // If mine, update my position
        if (mine(event_sid)) {
          update_sensor_position(position, event);
        }
      }

      // If ball sensor:
      if (context.get_balls().is_ball(event_sid)) {
        // Update local ball position
        update_sensor_position(ball_position, event);

        // Compute ball possession
        auto distance =
            distance::euclidean(as_vector(ball_position), as_vector(position));

        // If distance is within maximum distance, add it. Otherwise set
        // distance to infinity
        if (as_meters(distance) <= maximum_distance) {
          distances.push_back(distance);
        } else {
          distances.push_back(infinite_distance);
        }
      }
    }

#pragma omp critical(possession_update)
    {
      ball_possession.reduce(distances); // Reduce step
    };
  }

  // Update partial statistics
  accumulate_partial_statistics(ball_possession);

  // If last batch for this period, output partial statistics
  if (batch.is_period_last_batch) {
    compute_partial_statistics();
  }
}

std::unordered_map<std::string, double>
GameStatistics::accumulated_stats() const {
  auto total =
      std::accumulate(accumulator.cbegin(), accumulator.cend(), 0,
                      [](int acc, auto &&pair) { return acc + pair.second; });

  auto partials = std::unordered_map<std::string, double>();

  for (auto const &[name, nb_possessions] : accumulator) {
    partials.insert({name, static_cast<double>(nb_possessions) / total});
  }

  return partials;
}

std::unordered_map<std::string, double> const &
GameStatistics::last_partial() const {
  return partials.back();
}

std::unordered_map<std::string, double> GameStatistics::game_stats() const {
  auto stats = std::unordered_map<std::string, double>();
  auto total =
      std::accumulate(game_accumulator.cbegin(), game_accumulator.cend(), 0,
                      [](int acc, auto &&pair) { return acc + pair.second; });

  for (auto const &[name, nb_possessions] : game_accumulator) {
    stats.insert({name, static_cast<double>(nb_possessions) / total});
  }

  return stats;
}

void GameStatistics::accumulate_partial_statistics(
    const game::details::BallPossession &ball_possession) {
  for (auto const &[d, player_name] : ball_possession) {
    if (player_name != details::BallPossession::none_player) {
      accumulator[player_name] += 1;
    }
  }
}

void GameStatistics::compute_partial_statistics() {
  partials.push_back(accumulated_stats());
  for (auto const &[player_name, hits] : accumulator) {
    game_accumulator[player_name] += hits;
  }
  accumulator.clear();
}
} // namespace game
