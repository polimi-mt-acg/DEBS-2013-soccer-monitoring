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
  //= -------------------------- Setup OpenMP ------------------------------- =
  omp_set_num_threads(nb_threads);

  //= --------------------- Setup game::Context ----------------------------- =
  auto meta = game::parse_metadata_file(metadata.string());
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

  //= ----------------------- Business logic -------------------------------- =
  auto visualizer = game::Visualizer{players, teams};
  auto fetcher = game::EventFetcher{game_data.string(), game::file_stream{},
                                    batch_size, context};
  auto stats = game::GameStatistics{maximum_distance, context};

  visualizer.draw();
  for (auto const &batch : fetcher) {
    // TODO: Check if time_units are expired
    stats.batch_stats(batch, false);
    auto partials = stats.partial_stats();
    visualizer.update_stats(partials);
    visualizer.draw();
  }
}

void run_game_monitoring(int time_units, double maximum_distance,
                         std::filesystem::path const &game_data,
                         std::filesystem::path const &metadata, int nb_threads,
                         std::size_t batch_size,
                         std::string const &output_path) {}
} // namespace game