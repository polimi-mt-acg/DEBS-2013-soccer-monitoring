#include "catch.hpp"

#include "event_fetcher.hpp"
#include "game_statistics.hpp"
#include "metadata.hpp"
#include "test_dataset.hpp"

#include <chrono>
#include <iomanip>
#include <map>
#include <vector>

#include "fmt/format.h"

game::DistanceResults build_distance_results(std::string const &name,
                                             std::vector<double> &&distances) {
  auto results = game::DistanceResults{name};
  for (auto d : distances) {
    results.push_back(d);
  }

  return results;
}

void print_possession(game::BallPossession &possession) {
  for (const auto &[distance, name] : possession) {
    fmt::print("{} is the closest player (distance: {})\n", name, distance);
  }
}

void print_partial_stats(game::GameStatistics const &stats,
                         game::Context const &context) {
  auto partials = stats.accumulated_stats();
  auto sorted =
      std::map<std::string, double>(partials.cbegin(), partials.cend());
  std::cout << std::left << std::setw(20) << std::setfill(' ') << "Player";
  std::cout << std::left << std::setw(8) << std::setfill(' ') << "Team";
  std::cout << std::left << std::setw(8) << std::setfill(' ') << "Percentage";
  std::cout << "\n";
  for (auto const &[name, percentage] : sorted) {
    auto team = (context.get_teams()[name] == game::Team::A) ? "A" : "B";
    std::cout << std::left << std::setw(20) << std::setfill(' ') << name;
    std::cout << std::left << std::setw(8) << std::setfill(' ') << team;
    std::cout << std::left << std::setw(8) << std::setfill(' ')
              << fmt::format("{}%", percentage * 100);
    std::cout << "\n";
  }
}

TEST_CASE("Test ball possession iterator") {
  auto distance_1 = build_distance_results(
      "Player 1", {1, 1, 2, 3, game::GameStatistics::infinite_distance});
  auto distance_2 = build_distance_results(
      "Player 2", {3, 2, 1, 1, game::GameStatistics::infinite_distance});

  auto possession = game::BallPossession{};
  possession.reduce(distance_1);
  print_possession(possession);
  possession.reduce(distance_2);
  print_possession(possession);
}

TEST_CASE("Test batch_stats computation") {
  auto meta = game::parse_metadata_string(metadata);
  auto &players = meta.players;
  auto &teams = meta.teams;
  auto &balls = meta.balls;

  auto context = game::Context{};
  context.set_player_map(players);
  context.set_team_map(teams);
  context.set_ball_map(balls);

  for (auto &position : meta.positions) {
    auto sids = std::visit([](auto &&pos) { return pos.get_sids(); }, position);
    context.add_position(position, sids);
  }

  SECTION("10_50 String Stream") {
    std::size_t batch_size = 10;
    int time_units = 90 * 60;

    auto fetcher =
        game::EventFetcher{game_data_start_10_50, game::string_stream{},
                           time_units, batch_size, context};
    auto stats =
        game::GameStatistics{game::GameStatistics::infinite_distance, context};

    for (auto const &[batch, is_period_last_batch] : fetcher) {
      stats.batch_stats(batch, false);
      print_partial_stats(stats, context);
      fmt::print("\n");
    }
  }
}