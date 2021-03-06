#include "soccer_monitoring.hpp"
#include "context.hpp"
#include "event_fetcher.hpp"
#include "game_statistics.hpp"
#include "visualizer.hpp"

#include <omp.h>
#include <soccer_monitoring.hpp>

namespace game {
void run_game_monitoring(int time_units, double maximum_distance,
                         std::filesystem::path const &game_data,
                         std::filesystem::path const &metadata, int nb_threads,
                         std::size_t batch_size) {
  auto context = game::Context::build_from(metadata);
  auto visualizer =
      game::Visualizer{context.get_players(), context.get_teams(), time_units};

  details::run_game_monitoring(time_units, maximum_distance, game_data,
                               nb_threads, batch_size, context, visualizer);
}

void run_game_monitoring(int time_units, double maximum_distance,
                         std::filesystem::path const &game_data,
                         std::filesystem::path const &metadata, int nb_threads,
                         std::size_t batch_size,
                         std::string const &output_path) {
  auto context = game::Context::build_from(metadata);
  auto visualizer = game::Visualizer{context.get_players(), context.get_teams(),
                                     time_units, output_path};

  details::run_game_monitoring(time_units, maximum_distance, game_data,
                               nb_threads, batch_size, context, visualizer);
}

namespace details {
void run_game_monitoring(int time_units, double maximum_distance,
                         std::filesystem::path const &game_data, int nb_threads,
                         std::size_t batch_size, Context &context,
                         Visualizer &visualizer) {
  auto fetcher = game::EventFetcher{game_data.string(), game::file_stream{},
                                    time_units, batch_size, context};
  auto stats = game::GameStatistics{maximum_distance, context};
  omp_set_num_threads(nb_threads);

  visualizer.draw();
  auto t1 = std::chrono::steady_clock::now();
  for (auto const &batch : fetcher) {
    // Check if batch returned because time_units seconds are elapsed
    stats.accumulate_stats(batch);

    if (batch.is_period_last_batch) {
      auto const &partials = stats.last_partial();

      auto t2 = std::chrono::steady_clock::now();
      std::chrono::duration<double> diff = t2 - t1;
      fmt::print("Processed {} seconds of the stream (~ {} events) in {:.3f} "
                 "seconds\n",
                 time_units, time_units * 15000, diff.count());
      t1 = t2;
      visualizer.draw_stats(partials, false, batch.final_ts);
    }
  }

  auto game_stats = stats.game_stats();
  visualizer.draw_final_stats(game_stats);
}
} // namespace details
} // namespace game