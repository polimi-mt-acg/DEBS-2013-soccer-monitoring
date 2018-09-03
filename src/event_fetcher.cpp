#include "event_fetcher.hpp"

#include <boost/algorithm/string.hpp>
#include <event_fetcher.hpp>
#include <optional>
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
      snapshot{context.take_snapshot()}, period_start{game_start} {
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
      snapshot{context.take_snapshot()}, period_start{game_start} {
  // Open a string stream
  is = std::make_unique<std::istringstream>(dataset);

  // Reserve storage in batch
  batch.reserve(batch_size);
}

std::optional<PositionEvent> EventFetcher::parse_next_event() {
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
        return {};
      }
    } else {
      return {};
    }
  }
}

Batch EventFetcher::parse_batch() {
  batch.clear();
  while (true) {
    if (auto position_event = parse_next_event()) {
      // If an in-game position event
      if (is_in_game(*position_event)) {
        last_in_game_ts = position_event->get_timestamp();
        if (is_period_over(*position_event)) {
          return batch_period_over(*position_event);
        }

        if (game_paused && !batch.empty()) {
          return batch_game_paused(*position_event);
        }

        if (!game_paused) {
          add_event_to_batch(*position_event);
        }

        if (batch.size() >= batch_size) {
          // Batch full
          return batch_full_size();
        }
      } else {
        if (is_break(*position_event) && !batch.empty()) {
          return batch_game_break(*position_event);
        } else {
          // Just update sensor position
          update_sensor_position(*position_event);
        }
      }
    } else {
      return batch_game_over();
    }
  }
}

EventFetcher::iterator EventFetcher::begin() { return iterator{*this}; }

EventFetcher::iterator EventFetcher::end() { return iterator::end(); }

bool EventFetcher::is_in_game(PositionEvent const &event) const {
  auto first_half = game_start <= event.get_timestamp() &&
                    event.get_timestamp() <= break_start;
  auto second_half =
      break_end <= event.get_timestamp() && event.get_timestamp() <= game_end;
  return first_half || second_half;
}

bool EventFetcher::is_break(PositionEvent const &event) const {
  return break_start < event.get_timestamp() &&
         event.get_timestamp() < break_end;
}

bool EventFetcher::is_period_over(PositionEvent const &event) {
  auto event_ts = event.get_timestamp();
  auto elapsed_time = event_ts - period_start;
  return elapsed_time >= std::chrono::seconds(time_units);
}

Batch EventFetcher::batch_period_over(PositionEvent const &event) {
  period_start += std::chrono::seconds(time_units);
  auto prev_snapshot = snapshot;
  if (!game_paused) {
    snapshot = context.take_snapshot();
    bucket.push_back(event);
  }
  update_sensor_position(event);
  auto initial_ts =
      batch.empty() ? event.get_timestamp() : batch.front().get_timestamp();
  auto final_ts =
      batch.empty() ? event.get_timestamp() : batch.back().get_timestamp();
  return {batch, true, std::move(prev_snapshot), initial_ts, final_ts};
}

Batch EventFetcher::batch_game_paused(PositionEvent const &event) {
  auto prev_snapshot = snapshot;
  update_sensor_position(event);
  auto initial_ts =
      batch.empty() ? event.get_timestamp() : batch.front().get_timestamp();
  auto final_ts =
      batch.empty() ? event.get_timestamp() : batch.back().get_timestamp();
  return {batch, false, std::move(prev_snapshot), initial_ts, final_ts};
}

Batch EventFetcher::batch_game_break(PositionEvent const &event) {
  auto prev_snapshot = snapshot;
  auto &position = context.get_position(event.get_sid());
  ::game::update_sensor_position(position, event);
  auto initial_ts =
      batch.empty() ? event.get_timestamp() : batch.front().get_timestamp();
  auto final_ts =
      batch.empty() ? event.get_timestamp() : batch.back().get_timestamp();
  return {batch, true, std::move(prev_snapshot), initial_ts, final_ts};
}

Batch EventFetcher::batch_game_over() {
  game_over = true;
  auto prev_snapshot = snapshot;
  auto initial_ts =
      batch.empty() ? last_in_game_ts : batch.front().get_timestamp();
  auto final_ts =
      batch.empty() ? last_in_game_ts : batch.back().get_timestamp();
  return {batch, true, std::move(prev_snapshot), initial_ts, final_ts};
}

Batch EventFetcher::batch_full_size() {
  auto initial_ts = batch.front().get_timestamp();
  auto final_ts = batch.back().get_timestamp();
  return {batch, false, snapshot, initial_ts, final_ts};
}

void EventFetcher::add_event_to_batch(PositionEvent const &event) {
  if (batch.empty()) {
    if (bucket.empty()) {
      // No data left to add to batch. Just take a snapshot.
      snapshot = context.take_snapshot();
    } else {
      // There are events left to add. Snapshot have already been taken.
      // Just move data to batch.
      std::move(bucket.begin(), bucket.end(), std::back_inserter(batch));
      bucket.clear();
    }
  }
  // In any case, add the new event to batch.
  batch.push_back(event);
  update_sensor_position(event);
}

void EventFetcher::update_sensor_position(PositionEvent const &event) {
  auto &position = context.get_position(event.get_sid());
  ::game::update_sensor_position(position, event);
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