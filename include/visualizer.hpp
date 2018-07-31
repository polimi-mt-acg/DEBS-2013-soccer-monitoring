#ifndef SOCCER_MONITORING_VISUALIZER_H
#define SOCCER_MONITORING_VISUALIZER_H

#include "metadata.hpp"
#include <iostream>
#include <map>
#include <ostream>
#include <string>

namespace game {
namespace details {
struct PartialsCmp {
  explicit PartialsCmp(TeamMap const &teams) : teams{std::addressof(teams)} {}

  bool operator()(std::string const &p1, std::string const &p2) const;

private:
  TeamMap const *teams;
};
} // namespace details

class Visualizer {
public:
  Visualizer(PlayerMap const &players, TeamMap const &teams);

  Visualizer(PlayerMap const &players, TeamMap const &teams,
             std::string const &output_path);

  virtual ~Visualizer();

  void draw();
  void update_stats(std::unordered_map<std::string, double> const &partial);

private:
  std::ostream *os;
  PlayerMap const &players;
  TeamMap const &teams;
  std::map<std::string, double, details::PartialsCmp> partials;
  double team_a_partial = 0.0;
  double team_b_partial = 0.0;

  void init_partials(TeamMap const &teams);

  void draw_separator();
  void draw_header();
  void draw_entry(std::string const &name, double percentage);
  void draw_team_entry(Team team);
  void draw_team_separator();
};
} // namespace game
#endif // SOCCER_MONITORING_VISUALIZER_H
