#ifndef SOCCER_MONITORING_VISUALIZER_H
#define SOCCER_MONITORING_VISUALIZER_H

#include "metadata.hpp"
#include <chrono>
#include <iostream>
#include <map>
#include <ostream>
#include <string>

#include "details/partials_cmp.hpp"

namespace game {
class Visualizer {
public:
  Visualizer(PlayerMap const &players, TeamMap const &teams, int time_units);

  Visualizer(PlayerMap const &players, TeamMap const &teams, int time_units,
             std::string const &output_path);

  virtual ~Visualizer();

  void draw();
  void update_stats(std::unordered_map<std::string, double> const &partial,
                    bool is_game_end = false);

private:
  std::ostream *os;
  PlayerMap const &players;
  TeamMap const &teams;
  std::map<std::string, double, details::PartialsCmp> partials;
  int time_units = 0;
  std::chrono::seconds game_time = std::chrono::seconds{0};
  double team_a_partial = 0.0;
  double team_b_partial = 0.0;

  void init_partials(TeamMap const &teams);

  void draw_separator();
  void draw_players_header();
  void draw_entry(std::string const &name, double percentage);
  void draw_teams_header();
  void draw_teams_entry();
};
} // namespace game
#endif // SOCCER_MONITORING_VISUALIZER_H
