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

  auto visualizer = game::Visualizer{players, teams};

  SECTION("10_50 String Stream") {
    std::size_t batch_size = 10;
    auto fetcher = game::EventFetcher{
        game_data_start_10_50, game::string_stream{}, batch_size, context};
    auto stats =
        game::GameStatistics{game::GameStatistics::infinite_distance, context};
    visualizer.draw();

    for (auto const &batch : fetcher) {
      stats.batch_stats(batch, false);
      auto partials = stats.partial_stats();
      visualizer.update_stats(partials);
      visualizer.draw();
    }
  }

#ifdef GAME_DATA_START_10_1e7
  SECTION("GAME_DATA_START_10_1e7 File Stream") {
    std::size_t batch_size = 1500;
    auto path = std::string{GAME_DATA_START_10_1e7};

    fmt::print("Opening file {}...\n", path);
    auto fetcher =
        game::EventFetcher{path, game::file_stream{}, batch_size, context};
    auto stats =
        game::GameStatistics{game::GameStatistics::infinite_distance, context};

    for (auto const &batch : fetcher) {
      stats.batch_stats(batch, false);
      auto partials = stats.partial_stats();
      visualizer.update_stats(partials);
      visualizer.draw();
    }
  }
#endif
}
