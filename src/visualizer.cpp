#include "visualizer.hpp"

#include <fstream>
#include <iomanip>
#include <visualizer.hpp>

namespace game {

Visualizer::Visualizer(PlayerMap const &players, TeamMap const &teams,
                       int time_units)
    : players{players}, teams{teams}, os{&std::cout}, time_units{time_units},
      partials{std::map<std::string, double, details::PartialsCmp>(
          details::PartialsCmp{teams})} {
  init_partials(teams);
}

Visualizer::Visualizer(PlayerMap const &players, TeamMap const &teams,
                       int time_units, std::string const &output_path)
    : players{players}, teams{teams}, time_units{time_units},
      partials{std::map<std::string, double, details::PartialsCmp>(
          details::PartialsCmp{teams})} {
  os = new std::ofstream(output_path);
  init_partials(teams);
}

void Visualizer::update_stats(
    std::unordered_map<std::string, double> const &partial, bool is_game_end,
    std::chrono::picoseconds last_ts) {
  team_a_partial = 0.0;
  team_b_partial = 0.0;

  for (auto &[name, percentage] : partials) {
    if (auto search = partial.find(name); search != partial.end()) {
      percentage = search->second;
      if (teams[name] == Team::A) {
        team_a_partial += percentage;
      } else {
        team_b_partial += percentage;
      }
    } else {
      percentage = 0.0;
    }
  }

  auto total = team_a_partial + team_b_partial;

  if (total == 0.0) {
    team_a_partial = 0.0;
    team_b_partial = 0.0;
  } else {
    team_a_partial /= total;
    team_b_partial /= total;
  }

  if (!is_game_end) {
    update_game_time(last_ts);
  }
}

void Visualizer::init_partials(TeamMap const &teams) {
  auto names = players.get_player_names();
  for (auto const &name : names) {
    partials.insert({name, 0.0});
  }
}

void Visualizer::update_game_time(std::chrono::picoseconds last_ts) {
  namespace chrono = std::chrono;
  // Increment game time
  if (last_ts >= game_start) {
    // Add epsilon to round the numbers off
    last_ts += chrono::milliseconds(1);
    if (last_ts <= break_start) {
      game_time = chrono::duration_cast<chrono::seconds>(last_ts - game_start);
    } else if (break_start < last_ts && last_ts < break_end) {
      game_time = chrono::duration_cast<chrono::seconds>(official_break_start -
                                                         game_start);
    } else {
      auto first_half_duration = chrono::duration_cast<chrono::seconds>(
          official_break_start - game_start);
      game_time = chrono::duration_cast<chrono::seconds>(
          expected_half_duration + last_ts - break_end);
    }
  } else {
    game_time = game_time + chrono::seconds(time_units);
  }
}

void Visualizer::draw() {
  draw_separator();
  draw_teams_header();
  draw_teams_entry();

  draw_separator();
  draw_players_header();
  draw_separator();
  for (auto const &[name, percentage] : partials) {
    draw_entry(name, percentage);
  }
  draw_separator();
  *os << std::endl;
}

void Visualizer::draw_final_stats(
    std::unordered_map<std::string, double> const &game_stats) {
  update_stats(game_stats, true, {});
  *os << "--------- Game End. Final Statistics ----------\n\n";
  draw();
}

void Visualizer::draw_stats(
    std::unordered_map<std::string, double> const &partial, bool is_game_end,
    std::chrono::picoseconds last_ts) {
  update_stats(partial, is_game_end, last_ts);
  draw();
}

void Visualizer::draw_separator() {
  *os << std::left << std::setw(2) << std::setfill('-') << '-';
  *os << std::left << std::setw(20) << std::setfill('-') << '-';
  *os << std::left << std::setw(8) << std::setfill('-') << '-';
  *os << std::right << std::setw(15) << std::setfill('-') << '-';
  *os << std::left << std::setw(2) << std::setfill('-') << '-';
  *os << '\n';
}

void Visualizer::draw_players_header() {
  *os << std::left << std::setw(2) << std::setfill(' ') << '|';
  *os << std::left << std::setw(20) << std::setfill(' ') << "Player";
  *os << std::left << std::setw(8) << std::setfill(' ') << "Team";
  *os << std::right << std::setw(15) << std::setfill(' ') << "Ball Possession";
  *os << std::right << std::setw(2) << std::setfill(' ') << '|';
  *os << "\n";
}

void Visualizer::draw_entry(std::string const &name, double percentage) {
  auto team = teams[name];
  auto percentage_s = fmt::format("{:02.02f}%", percentage * 100);
  *os << std::left << std::setw(2) << std::setfill(' ') << '|';
  *os << std::left << std::setw(20) << std::setfill(' ') << name;
  *os << std::left << std::setw(8) << std::setfill(' ') << team;
  *os << std::right << std::setw(15) << std::setfill(' ') << percentage_s;
  *os << std::right << std::setw(2) << std::setfill(' ') << '|';
  *os << "\n";
}

void Visualizer::draw_teams_entry() {
  *os << std::left << std::setw(2) << std::setfill(' ') << '|';
  *os << std::right << std::setw(17) << std::setfill(' ')
      << std::setprecision(4) << team_a_partial * 100;
  *os << "    %    ";
  *os << std::left << std::setw(17) << std::setfill(' ') << std::setprecision(4)
      << team_b_partial * 100;
  *os << std::right << std::setw(2) << std::setfill(' ') << '|';
  *os << "\n";
}

void Visualizer::draw_teams_header() {
  namespace chrono = std::chrono;
  auto minutes = chrono::duration_cast<chrono::minutes>(game_time);
  auto seconds = game_time - minutes;

  *os << std::left << std::setw(2) << std::setfill(' ') << '|';
  *os << std::right << std::setw(17) << std::setfill(' ') << "Team A";
  *os << fmt::format("  {:02d}:{:02d}  ", minutes.count(), seconds.count());
  *os << std::left << std::setw(17) << std::setfill(' ') << "Team B";
  *os << std::right << std::setw(2) << std::setfill(' ') << '|';
  *os << "\n";
}

Visualizer::~Visualizer() {
  if (os != &std::cout) {
    delete os;
  }
}
} // namespace game