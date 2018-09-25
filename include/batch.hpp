#ifndef SOCCER_MONITORING_BATCH_H
#define SOCCER_MONITORING_BATCH_H

#include <functional>
#include <unordered_map>
#include <utility>
#include <vector>

#include "context.hpp"
#include "event.hpp"
#include "position.hpp"

namespace game {
/**
 * A batch of parsed PositionEvents. It contains the data, a snapshot of the
 * field right before the first PositionEvent in the data and whether it is the
 * last batch for the period.
 */
struct Batch {
  /**
   * Constructs a new Batch. All the fields are default constructed.
   */
  Batch() = default;
  /**
   * Construct a new Batch.
   * @param data The input batch of PositionEvents
   * @param is_period_last_batch True if the input batch is the last one for the
   *        current T time units. False otherwise.
   * @param snapshot The snapshot of the field.
   */
  Batch(std::vector<PositionEvent> const &data, bool is_period_last_batch,
        std::unordered_map<std::string, Positions> const &snapshot,
        std::chrono::picoseconds initial_ts, std::chrono::picoseconds final_ts)
      : data{std::addressof(data)}, is_period_last_batch{is_period_last_batch},
        snapshot{snapshot}, initial_ts{initial_ts}, final_ts{final_ts} {}
  /**
   * Construct a new Batch.
   * @param data The input batch of PositionEvents
   * @param is_period_last_batch True if the input batch is the last one for the
   *        current T time units. False otherwise.
   * @param snapshot The snapshot of the field.
   */
  Batch(std::vector<PositionEvent> const &data, bool is_period_last_batch,
        std::unordered_map<std::string, Positions> &&snapshot,
        std::chrono::picoseconds initial_ts, std::chrono::picoseconds final_ts)
      : data{std::addressof(data)}, is_period_last_batch{is_period_last_batch},
        snapshot{std::move(snapshot)}, initial_ts{initial_ts}, final_ts{
                                                                   final_ts} {}
  /**
   * @return the time interval between first and last event in the batch
   */
  std::chrono::picoseconds get_interval() const {
    return final_ts - initial_ts;
  }
  /**
   * The input batch of PositionEvents
   */
  const std::vector<PositionEvent> *data = nullptr;
  /**
   * True if the input batch is the last one for the current T time units. False
   * otherwise.
   */
  bool is_period_last_batch = false;
  /**
   * The snapshot of the field.
   */
  Snapshot snapshot = {};
  /**
   * The timestamp of the first event in the batch
   */
  std::chrono::picoseconds initial_ts = {};
  /**
   * The timestamp of the last event in the batch
   */
  std::chrono::picoseconds final_ts = {};
};
} // namespace game

#endif // SOCCER_MONITORING_BATCH_H
