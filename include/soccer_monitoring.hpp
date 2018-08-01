#ifndef SOCCER_MONITORING_SOCCER_MONITORING_HPP
#define SOCCER_MONITORING_SOCCER_MONITORING_HPP

#include <filesystem>
#include <string>

namespace game {
void run_game_monitoring(int time_units, double maximum_distance,
                         std::filesystem::path const &game_data,
                         std::filesystem::path const &metadata, int nb_threads,
                         std::size_t batch_size);

void run_game_monitoring(int time_units, double maximum_distance,
                         std::filesystem::path const &game_data,
                         std::filesystem::path const &metadata, int nb_threads,
                         std::size_t batch_size,
                         std::string const &output_path);

} // namespace game

#endif // SOCCER_MONITORING_SOCCER_MONITORING_HPP
