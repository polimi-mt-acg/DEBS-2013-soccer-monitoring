#ifndef GAME_EVENT_H
#define GAME_EVENT_H

#include <chrono>
#include <iostream>
#include <ratio>
#include <tuple>

namespace std {
namespace chrono {
using picoseconds = std::chrono::duration<long long int, std::pico>;
}
} // namespace std

namespace game {
/**
 * @brief The instant of game start (in picoseconds).
 */
constexpr auto game_start = std::chrono::picoseconds{10753295594424116};

/**
 * @brief The instant of game end (in picoseconds).
 */
constexpr auto game_end = std::chrono::picoseconds{14879639146403495};

constexpr auto field_lower_x = 0;
constexpr auto field_upper_x = 52483;
constexpr auto field_lower_y = -33960;
constexpr auto field_upper_y = 33965;

/**
 * @brief A game interruption event.
 */
class InterruptionEvent {
public:
  explicit InterruptionEvent(std::chrono::picoseconds timestamp)
      : timestamp{timestamp} {}

private:
  std::chrono::picoseconds timestamp;
};

/**
 * @brief A game resume event.
 */
class ResumeEvent {
public:
  explicit ResumeEvent(std::chrono::picoseconds timestamp)
      : timestamp{timestamp} {}

private:
  std::chrono::picoseconds timestamp;
};

/**
 * @brief A new position event for a sensor.
 */
class PositionEvent {
public:
  PositionEvent(int sensor_id, std::chrono::picoseconds timestamp, int x, int y,
                int z)
      : sid{sensor_id}, timestamp{timestamp}, x{x}, y{y}, z{z} {}

  int get_sid() const { return sid; }
  std::chrono::picoseconds get_timestamp() const { return timestamp; }
  int get_x() const { return x; }
  int get_y() const { return y; }
  int get_z() const { return z; }
  std::tuple<int, int, int> get_vector() const { return {x, y, z}; }

  friend std::ostream &operator<<(std::ostream &, PositionEvent const &);

private:
  int sid;
  std::chrono::picoseconds timestamp;
  int x;
  int y;
  int z;
};
} // namespace game

#endif