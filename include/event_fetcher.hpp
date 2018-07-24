#ifndef GAME_EVENT_FETCHER_H
#define GAME_EVENT_FETCHER_H

#include "event.hpp"
#include "stream_types.hpp"

#include <exception>
#include <fstream>
#include <memory>
#include <regex>
#include <sstream>
#include <string_view>
#include <variant>
#include <vector>

#include "fmt/format.h"

namespace game {
class EventFetcher {
public:
  template <typename Stream>
  EventFetcher(std::string const &, Stream, std::size_t batch_size);

  PositionEvent parse_next_event();
  std::vector<PositionEvent> const &parse_batch();

private:
  std::unique_ptr<std::istream> is = {};
  bool game_paused = false;
  std::vector<PositionEvent> batch = {};
  std::size_t batch_size = 0;
};

// ==-----------------------------------------------------------------------==
//                              Dataset parsing
// ==-----------------------------------------------------------------------==

struct Dataset {
  static constexpr auto game_interruption_id = 2010;
  static constexpr auto game_resume_id = 2011;

  static const std::regex se_re;
  static constexpr auto se_re_sid_idx = 1;
  static constexpr auto se_re_timestamp_idx = 2;
  static constexpr auto se_re_x_idx = 3;
  static constexpr auto se_re_y_idx = 4;
  static constexpr auto se_re_z_idx = 5;

  static const std::regex gi_re;
  static constexpr auto gi_re_event_id_idx = 1;
  static constexpr auto gi_re_timestamp_idx = 4;
};

struct unknown_game_interruption_event_error : public std::runtime_error {
  explicit unknown_game_interruption_event_error(int event_id)
      : runtime_error(""), event_id{event_id} {}

  const char *what() const throw() {
    auto str = fmt::format("Unknown event id {}", event_id);
    return strdup(str.c_str());
  }

private:
  int event_id;
};

struct unknown_event_error : public std::runtime_error {
  explicit unknown_event_error(std::string const &line)
      : runtime_error(""), line{line} {}

  const char *what() const throw() {
    auto str = fmt::format("Line \"{}\" matches no known event", line);
    return strdup(str.c_str());
  }

private:
  std::string line;
};

struct unexpected_event_error : public std::logic_error {
  explicit unexpected_event_error() : logic_error("") {}

  const char *what() const throw() { return "Unexpected event type"; }

private:
  std::string line;
};

std::variant<std::monostate, PositionEvent, InterruptionEvent, ResumeEvent>
parse_event_line(std::string const &);

// ==-----------------------------------------------------------------------==
//                           Template definitions
// ==-----------------------------------------------------------------------==

} // namespace game

#endif