#include "catch.hpp"

#include "context.hpp"
#include "event_fetcher.hpp"
#include "game_statistics.hpp"
#include "test_dataset.hpp"
#include "visualizer.hpp"

TEST_CASE("Test Visualizer") {
  auto context = game::Context::build_from(metadata);

  SECTION("10_50 String Stream") {
    std::size_t batch_size = 10;
    int time_units = 90 * 60;

    auto fetcher =
        game::EventFetcher{game_data_start_10_50, game::string_stream{},
                           time_units, batch_size, context};
    auto stats =
        game::GameStatistics{game::GameStatistics::infinite_distance, context};

    auto visualizer = game::Visualizer{context.get_players(),
                                       context.get_teams(), time_units};
    visualizer.draw();

    for (auto const &batch : fetcher) {
      stats.accumulate_stats(batch);
      auto partials = stats.accumulated_stats();
      visualizer.update_stats(partials);
      visualizer.draw();
    }
  }
}
