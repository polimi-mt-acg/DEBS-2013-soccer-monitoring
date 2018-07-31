#include "visualizer.hpp"

#include <fstream>
#include <iomanip>
#include <visualizer.hpp>

namespace game {

Visualizer::Visualizer(PlayerMap const &players, TeamMap const &teams)
    : players{players}, teams{teams}, os{&std::cout},
      partials{std::map<std::string, double, details::PartialsCmp>(
          details::PartialsCmp{teams})} {
  init_partials(teams);
}

Visualizer::Visualizer(PlayerMap const &players, TeamMap const &teams,
                       std::string const &output_path)
    : players{players}, teams{teams},
      partials{std::map<std::string, double, details::PartialsCmp>(
          details::PartialsCmp{teams})} {
  os = new std::ofstream(output_path);
  init_partials(teams);
}

void Visualizer::init_partials(TeamMap const &teams) {
  auto names = players.get_player_names();
  for (auto const &name : names) {
    partials.insert({name, 0.0});
  }
}

void Visualizer::update_stats(
    std::unordered_map<std::string, double> const &partial) {
  team_a_partial = 0.0;
  team_b_partial = 0.0;

  for (auto const &[name, percentage] : partial) {
    partials.insert_or_assign(name, percentage);
    if (teams[name] == Team::A) {
      team_a_partial += percentage;
    } else {
      team_b_partial += percentage;
    }
  }

  auto total = team_a_partial + team_b_partial;
  team_a_partial /= total;
  team_b_partial /= total;
}

void Visualizer::draw() {
  draw_separator();
  draw_header();
  draw_separator();
  for (auto const &[name, percentage] : partials) {
    draw_entry(name, percentage);
  }
  draw_separator();
  draw_team_entry(Team::A);
  draw_team_entry(Team::B);
  draw_team_separator();
  *os << std::endl;
}

void Visualizer::draw_separator() {
  *os << std::left << std::setw(2) << std::setfill('-') << '-';
  *os << std::left << std::setw(20) << std::setfill('-') << '-';
  *os << std::left << std::setw(8) << std::setfill('-') << '-';
  *os << std::right << std::setw(15) << std::setfill('-') << '-';
  *os << std::left << std::setw(2) << std::setfill('-') << '-';
  *os << '\n';
}
void Visualizer::draw_header() {
  *os << std::left << std::setw(2) << std::setfill(' ') << '|';
  *os << std::left << std::setw(20) << std::setfill(' ') << "Player";
  *os << std::left << std::setw(8) << std::setfill(' ') << "Team";
  *os << std::right << std::setw(15) << std::setfill(' ') << "Ball Possession";
  *os << std::right << std::setw(2) << std::setfill(' ') << '|';
  *os << "\n";
}

void Visualizer::draw_entry(std::string const &name, double percentage) {
  auto team = teams[name];
  auto percentage_s = fmt::format("{}%", percentage * 100);
  *os << std::left << std::setw(2) << std::setfill(' ') << '|';
  *os << std::left << std::setw(20) << std::setfill(' ') << name;
  *os << std::left << std::setw(8) << std::setfill(' ') << team;
  *os << std::right << std::setw(15) << std::setfill(' ') << percentage_s;
  *os << std::right << std::setw(2) << std::setfill(' ') << '|';
  *os << "\n";
}

void Visualizer::draw_team_entry(Team team) {
  auto percentage = (team == Team::A ? team_a_partial : team_b_partial);
  auto percentage_s = fmt::format("{}%", percentage * 100);

  *os << std::left << std::setw(2) << std::setfill(' ') << ' ';
  *os << std::right << std::setw(20) << std::setfill(' ') << "| Team  ";
  *os << std::left << std::setw(8) << std::setfill(' ') << team;
  *os << std::right << std::setw(15) << std::setfill(' ') << percentage_s;
  *os << std::right << std::setw(2) << std::setfill(' ') << '|';
  *os << "\n";
}

void Visualizer::draw_team_separator() {
  *os << std::left << std::setw(2) << std::setfill(' ') << ' ';
  *os << std::right << std::setw(20) << std::setfill(' ') << "--------";
  *os << std::left << std::setw(8) << std::setfill('-') << '-';
  *os << std::right << std::setw(15) << std::setfill('-') << '-';
  *os << std::right << std::setw(2) << std::setfill('-') << '-';
  *os << "\n";
}

Visualizer::~Visualizer() {
  if (os != &std::cout) {
    delete os;
  }
}

bool details::PartialsCmp::operator()(std::string const &p1,
                                      std::string const &p2) const {
  if (p1 == p2)
    return false; // For all a, comp(a, a) == false

  auto team_1 = (*teams)[p1];
  auto team_2 = (*teams)[p2];
  if (team_1 == team_2)
    return p1 < p2;
  else
    return team_1 == Team::A;
}
} // namespace game