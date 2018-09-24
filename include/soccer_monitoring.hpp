#ifndef SOCCER_MONITORING_SOCCER_MONITORING_HPP
#define SOCCER_MONITORING_SOCCER_MONITORING_HPP

#include "context.hpp"
#include "visualizer.hpp"
#include <filesystem>
#include <string>

namespace game {
/**
 * Application entry-point, the top-level function to run the game monitoring.
 * Game statistics are displayed on the standard output stream.
 *
 * @param time_units The number of seconds after which to output partial
 *        statistics.
 * @param maximum_distance The maximum distance at which any player is still
 *      eligible for ball possession.
 * @param game_data The game events file path.
 * @param metadata The metadata file path
 * @param nb_threads The number of threads to use in computation.
 * @param batch_size The maximum size of a batch of computation.
 */
void run_game_monitoring(int time_units, double maximum_distance,
                         std::filesystem::path const &game_data,
                         std::filesystem::path const &metadata, int nb_threads,
                         std::size_t batch_size);
/**
 * Application entry-point, the top-level function to run the game monitoring.
 * Game statistics are displayed on the desired file.
 *
 * @param time_units The number of seconds after which to output partial
 *        statistics.
 * @param maximum_distance The maximum distance at which any player is still
 *      eligible for ball possession.
 * @param game_data The game events file path.
 * @param metadata The metadata file path
 * @param nb_threads The number of threads to use in computation.
 * @param batch_size The maximum size of a batch of computation.
 * @param output_path The output file path
 */
void run_game_monitoring(int time_units, double maximum_distance,
                         std::filesystem::path const &game_data,
                         std::filesystem::path const &metadata, int nb_threads,
                         std::size_t batch_size,
                         std::string const &output_path);

namespace details {
void run_game_monitoring(int time_units, double maximum_distance,
                         std::filesystem::path const &game_data, int nb_threads,
                         std::size_t batch_size, Context &context,
                         Visualizer &visualizer);
}
} // namespace game

#endif // SOCCER_MONITORING_SOCCER_MONITORING_HPP
