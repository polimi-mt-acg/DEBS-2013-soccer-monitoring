#include "event_fetcher.hpp"

#include <boost/algorithm/string.hpp>
#include <regex>
#include <string>

#include "event.hpp"

namespace game {
// ==-----------------------------------------------------------------------==
//              Event lines parsers - Regex-based and Custom
// ==-----------------------------------------------------------------------==
template <>
std::variant<std::monostate, PositionEvent, InterruptionEvent, ResumeEvent>
parse_event_line(std::string const &line, game::parser_regex) {
  auto match = std::smatch{};

  if (std::regex_match(line, match, Dataset::se_re)) {
    // If Position event get fields
    auto sid = std::stoi(match[Dataset::se_re_sid_idx].str());
    auto ts = std::stoll(match[Dataset::se_re_timestamp_idx].str());
    auto x = std::stoi(match[Dataset::se_re_x_idx].str());
    auto y = std::stoi(match[Dataset::se_re_y_idx].str());
    auto z = std::stoi(match[Dataset::se_re_z_idx].str());

    return PositionEvent{sid, std::chrono::picoseconds{ts}, x, y, z};
  } else if (std::regex_match(line, match, Dataset::gi_re)) {
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
  } else {
    throw unknown_event_error{line};
  }
}

template <>
std::variant<std::monostate, PositionEvent, InterruptionEvent, ResumeEvent>
parse_event_line(std::string const &line, game::parser_custom) {
  using boost::is_any_of, boost::token_compress_on;
  auto tokens = std::vector<std::string>{};

  boost::split(tokens, line, is_any_of(","));

  auto event_type = tokens[Dataset::event_type_idx];
  if (event_type == "SE") {
    // Unpack SE fields
    auto sid = std::stoi(tokens[Dataset::se_sid_idx]);
    auto ts = std::stoll(tokens[Dataset::se_timestamp_idx]);
    auto x = std::stoi(tokens[Dataset::se_x_idx]);
    auto y = std::stoi(tokens[Dataset::se_y_idx]);
    auto z = std::stoi(tokens[Dataset::se_z_idx]);

    return PositionEvent{sid, std::chrono::picoseconds{ts}, x, y, z};
  } else if (event_type == "GI") {
    // Unpack GI fields
    auto event_id = std::stoi(tokens[Dataset::gi_event_id_idx]);
    auto ts = std::stoll(tokens[Dataset::gi_timestamp_idx]);

    if (event_id == Dataset::first_half_interruption_id ||
        event_id == Dataset::second_half_interruption_id) {
      return InterruptionEvent{std::chrono::picoseconds{ts}};
    } else if (event_id == Dataset::first_half_resume_id ||
               event_id == Dataset::second_half_resume_id) {
      return ResumeEvent{std::chrono::picoseconds{ts}};
    } else {
      throw unknown_game_interruption_event_error{event_id};
    }
  } else {
    throw unknown_event_error{line};
  }
}

// ==-----------------------------------------------------------------------==
//                     EventFetcher implementation
// ==-----------------------------------------------------------------------==

template <>
EventFetcher::EventFetcher(std::string const &path, game::file_stream,
                           int time_units, std::size_t batch_size,
                           Context &context)
    : time_units{time_units}, batch_size{batch_size}, context{context},
      period_start{game_start} {
  // Open a file stream
  is = std::make_unique<std::ifstream>(path, std::ios_base::in);

  // Reserve storage in batch
  batch.reserve(batch_size);
}

template <>
EventFetcher::EventFetcher(std::string const &dataset, game::string_stream,
                           int time_units, std::size_t batch_size,
                           Context &context)
    : time_units{time_units}, batch_size{batch_size}, context{context},
      period_start{game_start} {
  // Open a string stream
  is = std::make_unique<std::istringstream>(dataset);

  // Reserve storage in batch
  batch.reserve(batch_size);
}
PositionEvent EventFetcher::parse_next_event() {
  while (true) {
    if (*is) {
      // Get an event line
      std::string line{};
      auto &read_ok = std::getline(*is, line);

      if (read_ok) {
        auto event = parse_event_line(line, game::parser_custom{});

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
            return pos_event;
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
}

std::pair<std::reference_wrapper<const std::vector<PositionEvent>>, bool>
EventFetcher::parse_batch() {
  batch.clear(); // Delete previous batch
  while (true) {
    try {
      auto position_event = parse_next_event();

      // In any case, we need to update its sensor position, otherwise when
      // the game resumes, the sensors would still be found at the very
      // old position (which would be wrong)
      auto &position = context.get_position(position_event.get_sid());
      std::visit(
          [&position_event](auto &&pos) {
            pos.update_sensor(position_event.get_sid(),
                              position_event.get_vector());
          },
          position);

      // If an in-game position event
      if (is_valid_event(position_event)) {
        auto event_ts = position_event.get_timestamp();
        auto elapsed_time = event_ts - period_start;

        if (elapsed_time > std::chrono::seconds(time_units)) {
          period_start = event_ts;
          return {std::cref(batch), true};
        }

        if (!game_paused) {
          batch.push_back(position_event);
        }

        if (batch.size() == batch_size) {
          return {std::cref(batch), false};
        }
      }
      // Otherwise, update positions to have them updated before game start
    } catch (std::ios_base::failure &ex) {
      return {std::cref(batch), true};
    }
  }
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
const std::regex Dataset::gi_re =
    std::regex{"GI,(\\d+),[ "
               "\\w]+,(?:0|\\d{2}:\\d{2}:\\d{2}\\.\\d{3}),(\\d+),.*"};
} // namespace game