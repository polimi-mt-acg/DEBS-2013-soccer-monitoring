#ifndef SOCCER_MONITORING_GAME_STATISTICS_H
#define SOCCER_MONITORING_GAME_STATISTICS_H

#include "details/game_statistics_impl.hpp"
#include "context.hpp"
#include "event.hpp"

#include <limits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace game {
/**
 * Manage and compute ball possession statistics.
 *
 * A player is considered in possession of the ball when a) he is the player
 * closest to the ball b) he is not farther than MAXIMUM_DISTANCE meters from
 * the ball. The statistics accumulate every T time units as the game unfolds
 * and after T time units partial statistics are computed.
 * Moreover, at any point of time statistics for the whole game are available.
 */
class GameStatistics {
public:
  /**
   * Infinite distance value.
   */
  static constexpr auto infinite_distance = std::numeric_limits<double>::max();

  /**
   * Constructs a GameStatistics object.
   *
   * @param maximum_distance The maximum distance at which any player is still
   *        considered in computing the closest one.
   * @param context The game::Context
   */
  GameStatistics(double maximum_distance, Context &context)
      : maximum_distance{maximum_distance}, context{context} {
    player_names = context.get_player_names();
  }

  /**
   * Accumulate statistics from a batch of game::PositionEvent. Statistics are
   * computed as following: 0) Let B be the ball position before calling
   * accumulate_stats() 1) For each player i, let P_i be the position of the
   * player 2) For each event e in the input batch, if it is a player
   * PositionEvent for player i, then P_i is updated to new location. Otherwise,
   * if it is a ball PositionEvent, B is updated with new ball location and
   * d(P_i, B) is computed as the euclidean distance between player i and the
   * ball. If d(P_i, B) > MAXIMUM_DISTANCE, d(P_i, B) is set to infinite 3)
   * Each evaluated distance is appended to the distances vector of each player
   * 4) For each index j in [0, size-of-distances-vector) let d_min be the
   * minimum distance between a player and the ball at instant j and let p_min
   * be the player such that d(P_i, B) == d_min at instant j; then increment the
   * number of possessions of player i by one.
   *
   * If the input batch is the last one for the current T time units, partial
   * statistics are computed as the weighted average of the number of ball
   * possessions and the accumulator is cleared.
   *
   * @param batch the input batch of PositionEvents
   * @param period_last_batch true if the input batch is the last one for the
   *        current T time units. false otherwise.
   */
  void accumulate_stats(std::vector<PositionEvent> const &batch,
                        bool period_last_batch = false);

  /**
   * @return the accumulated statistics for the current period, for each player.
   *         Each value is percentage of ball possession for a given player in
   *         the current time units slot.
   */
  std::unordered_map<std::string, double> accumulated_stats() const;

  /**
   * @return the last computed partial statistics.
   */
  std::unordered_map<std::string, double> const &last_partial() const;

  /**
   * @return the computed ball possession statistics of the whole game, for each
   *         player. Each value is the percentage of ball possession for a given
   *         player from the beginning of the game up to now.
   */
  std::unordered_map<std::string, double> game_stats() const;

private:
  Context &context;
  double maximum_distance = 0.0;
  std::vector<std::string> player_names = {};
  std::vector<std::unordered_map<std::string, double>> partials = {};
  std::unordered_map<std::string, int> accumulator = {};
  std::unordered_map<std::string, int> game_accumulator = {};
};
} // namespace game
#endif // SOCCER_MONITORING_GAME_STATISTICS_H
