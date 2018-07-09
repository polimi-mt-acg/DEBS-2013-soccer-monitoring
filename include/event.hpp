#ifndef GAME_EVENT_H
#define GAME_EVENT_H

#include <chrono>
#include <ratio>

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

/**
 * @brief A game interruption event.
 */
class InterruptionEvent {
public:
  explicit InterruptionEvent(std::chrono::picoseconds timestamp)
      : game_time{timestamp - game_start} {}

private:
  std::chrono::picoseconds game_time;
};

/**
 * @brief A game resume event.
 */
class ResumeEvent {
public:
  explicit ResumeEvent(std::chrono::picoseconds timestamp)
      : game_time{timestamp - game_start} {}

private:
  std::chrono::picoseconds game_time;
};

/**
 * @brief A new position event for a sensor.
 */
class PositionEvent {
  PositionEvent(int sensor_id, std::chrono::picoseconds timestamp, int x, int y,
                int z)
      : sid{sid}, game_time{timestamp - game_start}, x{x}, y{y}, z{z} {}

private:
  int sid;
  std::chrono::picoseconds game_time;
  int x;
  int y;
  int z;
};
} // namespace game

#endif