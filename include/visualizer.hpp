#ifndef SOCCER_MONITORING_VISUALIZER_H
#define SOCCER_MONITORING_VISUALIZER_H

#include "metadata.hpp"
#include <chrono>
#include <iostream>
#include <map>
#include <ostream>
#include <string>

#include "details/visualizer_impl.hpp"

namespace game {
/**
 * A Visualizer displays game statistics in a human-friendly way to the desired
 * output stream. Data is formatted into a table resembling the classic TV
 * info-graphics for soccer game, e.g.:
 *  -----------------------------------------------
 *  |            Team A  03:27  Team B            |
 *  |                 0    %    0                 |
 *  -----------------------------------------------
 *  | Player              Team    Ball Possession |
 *  -----------------------------------------------
 *  | Dennis Dotterweich  A                 0.00% |
 *  | Erik Engelhardt     A                 0.00% |
 *  | Nick Gertje         A                 0.00% |
 *  | Niklas Waelzlein    A                 0.00% |
 *  | Philipp Harlass     A                 0.00% |
 *  | Roman Hartleb       A                 0.00% |
 *  | Sandro Schneider    A                 0.00% |
 *  | Wili Sommer         A                 0.00% |
 *  | Ben Mueller         B                 0.00% |
 *  | Christopher Lee     B                 0.00% |
 *  | Kevin Baer          B                 0.00% |
 *  | Leo Langhans        B                 0.00% |
 *  | Leon Heinze         B                 0.00% |
 *  | Leon Krapf          B                 0.00% |
 *  | Luca Ziegler        B                 0.00% |
 *  | Vale Reitstetter    B                 0.00% |
 *  -----------------------------------------------
 */
class Visualizer {
public:
  /**
   * Construct a new Visualizer object with std::cout as output stream
   *
   * @param players The players map
   * @param teams The teams map
   * @param time_units the number of seconds after which to output partial
   *        statistics
   */
  Visualizer(PlayerMap const &players, TeamMap const &teams, int time_units);
  /**
   * Construct a new Visualizer object with the desired file as output stream
   *
   * @param players The players map
   * @param teams The teams map
   * @param time_units the number of seconds after which to output partial
   *        statistics
   * @param output_path The file to display statistics on
   */
  Visualizer(PlayerMap const &players, TeamMap const &teams, int time_units,
             std::string const &output_path);
  /**
   * Destruct this object, closing open resources.
   */
  ~Visualizer();
  /**
   * Draw a table displaying the stored statistics on the output stream
   */
  void draw();
  /**
   * Updates the stored statistics and draw them on the output stream
   *
   * @param partial The statistics to store and draw. It maps player names to
   *        their ball possesion percentage.
   * @param is_game_end True if input statistics are the last of the game.
   *        False otherwise.
   * @param last_ts The timestamp of the last event from which statistics were
   *        computed.
   */
  void draw_stats(std::unordered_map<std::string, double> const &partial,
                  bool is_game_end, std::chrono::picoseconds last_ts);
  /**
   * Updates the stored statistics and draw them on the output stream, along
   * with a message stating that game is over and the showed stats are the whole
   * game ones.
   *
   * @param game_stats The whole-game statistics
   */
  void
  draw_final_stats(std::unordered_map<std::string, double> const &game_stats);
  /**
   * Updates the stored statistics
   *
   * @param partial The statistics to store and draw. It maps player names to
   *        their ball possesion percentage.
   * @param is_game_end True if input statistics are the last of the game.
   *        False otherwise.
   * @param last_ts The timestamp of the last event from which statistics were
   *        computed.
   */
  void update_stats(std::unordered_map<std::string, double> const &partial,
                    bool is_game_end, std::chrono::picoseconds last_ts);

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
  void update_game_time(std::chrono::picoseconds last_ts);

  void draw_separator();
  void draw_players_header();
  void draw_entry(std::string const &name, double percentage);
  void draw_teams_header();
  void draw_teams_entry();
};
} // namespace game
#endif // SOCCER_MONITORING_VISUALIZER_H
