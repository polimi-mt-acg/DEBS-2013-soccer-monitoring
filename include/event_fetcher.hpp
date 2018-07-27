#ifndef GAME_EVENT_FETCHER_H
#define GAME_EVENT_FETCHER_H

#include "context.hpp"
#include "event.hpp"
#include "stream_types.hpp"

#include <exception>
#include <fstream>
#include <functional>
#include <memory>
#include <regex>
#include <sstream>
#include <string_view>
#include <variant>
#include <vector>

#include "fmt/format.h"

namespace game {

/**
 * @brief Class to consume the streaming game events.
 * Initialized with the dataset source and the batch size it returns a batch of
 * parsed events, skipping those between game interruptions.
 */
class EventFetcher {
public:
  /**
   * @brief Construct a new EventFetcher object with given batch size.
   *
   * @tparam Stream The type of stream. Can either be game::file_stream or
   *         game::string_stream. If game::file_stream, first parameter must be
   *         the file path. Else, first parameter must be a string of the
   *         dataset.
   * @param batch_size The size of the batch of parsed PositionEvent.
   */
  template <typename Stream>
  EventFetcher(std::string const &, Stream, std::size_t batch_size,
               Context &context);

  /**
   * @brief Parses next in-game PositionEvent. If next event is an
   * InterruptionEvent, PositionEvents are skipped until a ResumEvent is found.
   *
   * @throws std::ios_base::failure if stream reached EOF
   * @return next in-game PositionEvent
   */
  PositionEvent parse_next_event();

  /**
   * @brief Parses next in-game batch of PositionEvent.
   * If batch_size PositionEvent are parsed, it returns a batch of batch_size
   * PositionEvents. If EOF is reached, remaining PositionEvents are returned.
   * Only in-game events are considered, which means events with a timestep t
   * such that game_start < t < half_1_end and half_2_start < t < game_end.
   *
   * @return the batch of PositionEvent.
   */
  std::vector<PositionEvent> const &parse_batch();

private:
  Context &context;
  std::unique_ptr<std::istream> is = {};
  bool game_paused = false;
  std::vector<PositionEvent> batch = {};
  std::size_t batch_size = 0;
};

// ==-----------------------------------------------------------------------==
//                              Dataset parsing
// ==-----------------------------------------------------------------------==

/**
 * @brief Dataset constants for parsing
 */
struct Dataset {
  /**
   * @brief Game interruption id of GI event.
   */
  static constexpr auto game_interruption_id = 2010;

  /**
   * @brief Game resume id of GI event.
   */
  static constexpr auto game_resume_id = 2011;

  /**
   * @brief Regex for parsing SE events.
   */
  static const std::regex se_re;

  // =------------------------------------------=
  //        SE regex's group indices
  // =------------------------------------------=
  static constexpr auto se_re_sid_idx = 1;
  static constexpr auto se_re_timestamp_idx = 2;
  static constexpr auto se_re_x_idx = 3;
  static constexpr auto se_re_y_idx = 4;
  static constexpr auto se_re_z_idx = 5;

  /**
   * @brief Regex for parsing GI events.
   */
  static const std::regex gi_re;

  // =------------------------------------------=
  //        GI regex's group indices
  // =------------------------------------------=
  static constexpr auto gi_re_event_id_idx = 1;
  static constexpr auto gi_re_timestamp_idx = 4;
};

/**
 * @brief Report an error in parsing GI event, in case event_id is unknown.
 */
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

/**
 * @brief Report an error in parsing a game event, in case it matches no regular
 * expressions.
 */
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

/**
 * @brief Report an error in parsing a game event, in case it's none of the
 * known ones.
 */
struct unexpected_event_error : public std::logic_error {
  explicit unexpected_event_error() : logic_error("") {}

  const char *what() const throw() { return "Unexpected event type"; }

private:
  std::string line;
};

/**
 * @brief Parse a single event line into an event.
 *
 * @param line the game event line from dataset.
 * @throws game::unknown_game_interruption_event_error if a GI event has an
 * unknown event_id.
 * @throws game::unknown_event_error if @p line matches no known pattern.
 * @return The parsed event as a std::variant of PositionEvent,
 * InterruptionEvent or ResumeEvent in case @p line is a known event type.
 */
std::variant<std::monostate, PositionEvent, InterruptionEvent, ResumeEvent>
parse_event_line(std::string const &line);

} // namespace game

#endif