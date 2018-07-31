#include "event_fetcher.hpp"

#include <event_fetcher.hpp>
#include <regex>
#include <string>

#include "event.hpp"

namespace game {

// ==-----------------------------------------------------------------------==
//                     EventFetcher implementation
// ==-----------------------------------------------------------------------==

template <>
EventFetcher::EventFetcher(std::string const &path, game::file_stream,
                           std::size_t batch_size, Context &context)
    : batch_size{batch_size}, context{context} {
  // Open a file stream
  is = std::make_unique<std::ifstream>(path, std::ios_base::in);

  // Reserve storage in batch
  batch.reserve(batch_size);
}

template <>
EventFetcher::EventFetcher(std::string const &dataset, game::string_stream,
                           std::size_t batch_size, Context &context)
    : batch_size{batch_size}, context{context} {
  // Open a string stream
  is = std::make_unique<std::istringstream>(dataset);

  // Reserve storage in batch
  batch.reserve(batch_size);
}

PositionEvent EventFetcher::parse_next_event() {
  bool got_position_event = false;
  auto event = std::variant<std::monostate, PositionEvent, InterruptionEvent,
                            ResumeEvent>{};
  while (!got_position_event) {
    if (*is) {
      // Get an event line
      std::string line{};
      auto &read_ok = std::getline(*is, line);

      if (read_ok) {
        event = parse_event_line(line);

        if (std::holds_alternative<InterruptionEvent>(event)) {
          // If interruption, go next
          if (!game_paused) {
            game_paused = true;
          }
        } else if (std::holds_alternative<ResumeEvent>(event)) {
          // If resume, go next
          if (game_paused) {
            game_paused = false;
          }
        } else if (std::holds_alternative<PositionEvent>(event)) {
          auto pos_event = std::get<PositionEvent>(event);
          auto event_sid = pos_event.get_sid();
          if (context.get_balls().is_ball(event_sid) ||
              context.get_players().is_player(event_sid)) {
            if (!game_paused) {
              got_position_event = true;
            } else {
              // If the game is paused, we do not return the parsed event.
              // Still, we need to update its sensor position, otherwise when
              // the game resumes, the sensors would still be found at the very
              // old position (which would be wrong)
              auto &position = context.get_position(pos_event.get_sid());
              std::visit(
                  [&pos_event](auto &&pos) {
                    pos.update_sensor(pos_event.get_sid(),
                                      pos_event.get_vector());
                  },
                  position);
            }
          }
        } else {
          throw unexpected_event_error{};
        }
      } else {
        // Cannot read a new line.
        throw std::ios_base::failure{"No new line available. Reached EOF."};
      }
    } else {
      throw std::ios_base::failure{"No new line available. Reached EOF."};
    }
  }
  return std::get<PositionEvent>(event);
} // namespace game

std::vector<PositionEvent> const &EventFetcher::parse_batch() {
  bool is_batch_full = false;
  batch.clear(); // Delete previous batch
  while (!is_batch_full) {
    try {
      auto position_event = parse_next_event();
      // If an in-game position event
      if (is_valid_event(position_event)) {
        batch.push_back(position_event);
        if (batch.size() == batch_size) {
          is_batch_full = true;
        }
      }
      // Otherwise, update positions to have them updated before game start
    } catch (std::ios_base::failure &ex) {
      return batch;
    }
  }
  return batch;
}

EventFetcher::iterator EventFetcher::begin() { return iterator{*this}; }

EventFetcher::iterator EventFetcher::end() { return iterator{*this, true}; }

bool EventFetcher::is_valid_event(PositionEvent const &event) const {
  auto first_half = game_start <= event.get_timestamp() &&
                    event.get_timestamp() <= break_start;
  auto second_half =
      break_end <= event.get_timestamp() && event.get_timestamp() <= game_end;
  return first_half || second_half;
}

// ==-----------------------------------------------------------------------==
//                     Static variables initialization
// ==-----------------------------------------------------------------------==

const std::regex Dataset::se_re =
    std::regex{R"(SE,(\d+),(\d+),(-?\d+),(-?\d+),(-?\d+),.*)"};
const std::regex Dataset::gi_re = std::regex{
    "GI,(\\d+),([ "
    "\\w]+),(0|\\d{2}:\\d{2}:\\d{2}\\.\\d{3}),(\\d+),(\\d+),([ \\w]+)"};

// ==-----------------------------------------------------------------------==
//                      Functions definition
// ==-----------------------------------------------------------------------==

std::variant<std::monostate, PositionEvent, InterruptionEvent, ResumeEvent>
parse_event_line(std::string const &line) {
  auto match = std::smatch{};

  if (std::regex_match(line, match, Dataset::gi_re)) {
    // If Game Interruption event, get event id and timestamp
    auto event_id = std::stoi(match[Dataset::gi_re_event_id_idx].str());
    auto ts = std::stoll(match[Dataset::gi_re_timestamp_idx].str());

    if (event_id == Dataset::first_half_interruption_id ||
        event_id == Dataset::second_half_interruption_id) {
      return InterruptionEvent{std::chrono::picoseconds{ts}};
    } else if (event_id == Dataset::first_half_resume_id ||
               event_id == Dataset::second_half_resume_id) {
      return ResumeEvent{std::chrono::picoseconds{ts}};
    } else {
      throw unknown_game_interruption_event_error{event_id};
    }
  } else if (std::regex_match(line, match, Dataset::se_re)) {
    // If Position event get fields
    auto sid = std::stoi(match[Dataset::se_re_sid_idx].str());
    auto ts = std::stoll(match[Dataset::se_re_timestamp_idx].str());
    auto x = std::stoi(match[Dataset::se_re_x_idx].str());
    auto y = std::stoi(match[Dataset::se_re_y_idx].str());
    auto z = std::stoi(match[Dataset::se_re_z_idx].str());

    return PositionEvent{sid, std::chrono::picoseconds{ts}, x, y, z};
  } else {
    throw unknown_event_error{line};
  }
}
} // namespace game