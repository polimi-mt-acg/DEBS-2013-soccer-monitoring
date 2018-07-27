#include "metadata.hpp"
#include "position.hpp"

#include <algorithm>
#include <array>
#include <exception>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace game {

// ==-----------------------------------------------------------------------==
//                        PlayerMap implementation
// ==-----------------------------------------------------------------------==

void PlayerMap::add_sensor(int sensor_id, std::string const &player) {
  this->player.insert({sensor_id, player});
}

bool PlayerMap::is_player(int sensor_id) const {
  auto search = player.find(sensor_id);
  return search != player.end();
}

std::string const &PlayerMap::operator[](int const &sensor_id) const {
  return player.at(sensor_id);
}

// ==-----------------------------------------------------------------------==
//                        TeamMap implementation
// ==-----------------------------------------------------------------------==

void TeamMap::add_player(std::string const &player, Team team) {
  if (team == Team::A) {
    team_A.push_back(player);
  } else {
    team_B.push_back(player);
  }
}

Team TeamMap::operator[](std::string const &player) const {
  if (std::find(team_A.cbegin(), team_A.cend(), player) != team_A.cend()) {
    return Team::A;
  } else if (std::find(team_B.cbegin(), team_B.cend(), player) !=
             team_B.cend()) {
    return Team::B;
  } else {
    throw std::out_of_range{fmt::format("Unknown player \"{}\"", player)};
  }
}

// ==-----------------------------------------------------------------------==
//                        BallMap implementation
// ==-----------------------------------------------------------------------==

void BallMap::add_ball(int sensor_id) { balls.push_back(sensor_id); }

bool BallMap::is_ball(int sensor_id) const {
  auto search = std::find(balls.cbegin(), balls.cend(), sensor_id);
  return search != balls.cend();
}

// ==-----------------------------------------------------------------------==
//                    Static variables definition
// ==-----------------------------------------------------------------------==

const std::regex ParseMetadata::ball_re = std::regex{"BALL,(\\d+),(\\d+)"};
const std::regex ParseMetadata::player_re =
    std::regex{"PLAYER,([AB]),([ \\w]+),(\\d+),(\\d+),(\\d+),(\\d+)"};


// ==-----------------------------------------------------------------------==
//                        Functions definition
// ==-----------------------------------------------------------------------==


Metadata parse_metadata_file(std::string const &path) {
  namespace fs = std::filesystem;

  auto p = fs::path{path};
  if (!fs::is_regular_file(p)) {
    throw file_not_found_error{path};
  }

  auto buffer = std::stringstream{};
  auto input = std::ifstream{p.c_str()};
  buffer << input.rdbuf();

  return parse_metadata_string(buffer.str());
}

Metadata parse_metadata_string(std::string const &metadata) {
  auto ss = std::stringstream{metadata};
  auto match = std::smatch{};

  auto players = PlayerMap{};
  auto teams = TeamMap{};
  auto balls = BallMap{};
  auto positions = std::vector<Positions>();
  positions.emplace_back(BallPosition{});
  auto &ball_position = positions.back();

  for (auto line = std::string{}; std::getline(ss, line);) {
    if (std::regex_match(line, match, ParseMetadata::ball_re)) {
      // Get sensor id for the ball and it to ball map if not already present
      auto ball_sid = std::stoi(match[ParseMetadata::ball_re_sid_idx].str());
      if (!balls.is_ball(ball_sid)) {
        balls.add_ball(ball_sid);
        std::visit([ball_sid](auto &&pos) { pos.add_sensor(ball_sid); },
                   ball_position);
      }
    } else if (std::regex_match(line, match, ParseMetadata::player_re)) {
      auto team =
          (match[ParseMetadata::player_re_team_idx].str() == "A" ? Team::A
                                                                 : Team::B);
      auto name = match[ParseMetadata::player_re_name_idx].str();

      // Parse sensor ids for player
      auto sids = std::vector<int>{};
      for (std::size_t i = ParseMetadata::player_re_sid_start_idx;
           i <= ParseMetadata::player_re_sid_end_idx; ++i) {
        auto sid = std::stoi(match[i].str());
        if (sid != 0) {
          sids.push_back(sid);
        }
      }

      // Fill maps
      teams.add_player(name, team);
      auto position = PlayerPosition{};
      for (auto sid : sids) {
        players.add_sensor(sid, name);
        position.add_sensor(sid);
      }
      positions.emplace_back(std::move(position));
    } else {
      fmt::print(stderr, "Unable to parse line:\n  {}\n  Skipping...\n", line);
    }
  }

  return {players, teams, balls, positions};
}
} // namespace game