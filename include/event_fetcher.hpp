#include <utility>

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

namespace details {
class event_fetcher_iterator;
}

/**
 * @brief Class to consume the streaming game events.
 * Initialized with the dataset source and the batch size it returns a batch of
 * parsed events, skipping those between game interruptions.
 */
class EventFetcher {
public:
  friend class details::event_fetcher_iterator;
  using iterator = details::event_fetcher_iterator;

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
  EventFetcher(std::string const &, Stream, int time_units,
               std::size_t batch_size, Context &context);

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
  std::pair<std::reference_wrapper<const std::vector<PositionEvent>>, bool>
  parse_batch();

  bool is_valid_event(PositionEvent const &event) const;

  iterator begin();
  iterator end();

private:
  Context &context;
  std::unique_ptr<std::istream> is = {};
  bool game_paused = false;
  std::vector<PositionEvent> batch = {};
  int time_units = 0;
  std::chrono::picoseconds period_start;
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
  static constexpr auto first_half_interruption_id = 2010;
  static constexpr auto second_half_interruption_id = 6014;

  /**
   * @brief Game resume id of GI event.
   */
  static constexpr auto first_half_resume_id = 2011;
  static constexpr auto second_half_resume_id = 6015;

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
  explicit unknown_event_error(std::string line)
      : runtime_error(""), line{std::move(line)} {}

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

namespace details {
class event_fetcher_iterator {
public:
  using difference_type = std::ptrdiff_t;
  using value_type =
      std::pair<std::reference_wrapper<const std::vector<PositionEvent>>, bool>;
  using reference = const value_type &;
  using pointer = std::add_pointer_t<reference>;
  using iterator_category = std::input_iterator_tag;

  using iterator = event_fetcher_iterator;

  explicit event_fetcher_iterator(EventFetcher &f, bool set_end = false)
      : fetcher{std::addressof(f)}, value{std::make_pair(
                                        std::cref(fetcher->batch), false)} {
    // Set to end if set_end == true or if fetcher input stream failbit is set
    // (i.e. EOF may be reached)
    is_end = set_end ? true : !(*fetcher->is);
  }

  // CopyConstructible
  event_fetcher_iterator(iterator const &other)
      : fetcher{other.fetcher}, is_end{other.is_end}, value{other.value} {}

  // CopyAssignable
  iterator &operator=(iterator const &other) {
    fetcher = other.fetcher;
    is_end = other.is_end;
    value = other.value;
    return *this;
  }

  // Dereferenceable (convertible to value_type)
  reference operator*() { return value; }
  value_type operator*() const { return value; }

  iterator &operator++() {
    value = fetcher->parse_batch();
    is_end = !(*fetcher->is);
    return *this;
  }

  iterator operator++(int) {
    auto it = *this;
    this->operator++();
    return it;
  }

  pointer operator->() const { return &value; }

  friend bool operator==(iterator const &lhs, iterator const &rhs) {
    return lhs.is_end == rhs.is_end;
  }

  friend bool operator!=(iterator const &lhs, iterator const &rhs) {
    return !(lhs == rhs);
  }

private:
  EventFetcher *fetcher;
  std::pair<std::reference_wrapper<const std::vector<PositionEvent>>, bool>
      value;
  bool is_end;
};
} // namespace details

} // namespace game

#endif