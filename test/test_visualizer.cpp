#include "catch.hpp"

#include "context.hpp"
#include "event_fetcher.hpp"
#include "game_statistics.hpp"
#include "test_dataset.hpp"
#include "visualizer.hpp"

TEST_CASE("Test Visualizer") {
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

    auto visualizer = game::Visualizer{players, teams, time_units};
    visualizer.draw();

    for (auto const &[batch, is_period_last_batch] : fetcher) {
      stats.batch_stats(batch, false);
      auto partials = stats.accumulated_stats();
      visualizer.update_stats(partials);
      visualizer.draw();
    }
  }
}
