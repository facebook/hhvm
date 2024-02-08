/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <mutex>

#include <folly/container/F14Map.h>

#include "mcrouter/ExponentialSmoothData.h"
#include "mcrouter/PoolStats.h"
#include "mcrouter/stats.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

class ProxyStats {
 public:
  explicit ProxyStats(const std::vector<std::string>& statsEnabledPools);

  /**
   * Aggregate proxy stat with the given index.
   * Caller must be holding stats lock (i.e. must call lock() before).
   *
   * @param statId   Index of the stat to aggregate.
   */
  void aggregate(size_t statId);

  /**
   * Lock stats for the duration of the lock_guard life.
   */
  std::unique_lock<std::mutex> lock() const;

  ExponentialSmoothData<64>& durationUs() {
    return durationUs_;
  }

  ExponentialSmoothData<64>& durationGetUs() {
    return durationGetUs_;
  }

  ExponentialSmoothData<64>& durationUpdateUs() {
    return durationUpdateUs_;
  }

  /**
   * Tells the interval (in seconds) between closing a connection due to lack
   * of activity and opening it again.
   */
  ExponentialSmoothData<64>& inactiveConnectionClosedIntervalSec() {
    return inactiveConnectionClosedIntervalSec_;
  }

  ExponentialSmoothData<64>& asyncLogDurationUs() {
    return asyncLogDurationUs_;
  }

  ExponentialSmoothData<64>& axonLogDurationUs() {
    return axonLogDurationUs_;
  }

  ExponentialSmoothData<64>& axonProxyDurationUs() {
    return axonProxyDurationUs_;
  }

  size_t numBinsUsed() const {
    return numBinsUsed_;
  }

  /**
   * Return value of stat "statId" in the "timeBinIdx" time bin.
   *
   * @param statId      Index of the stat.
   * @param timeBinIdx  Idx of the time bin.
   */
  uint64_t getStatBinValue(size_t statId, size_t timeBinIdx) const {
    return statsBin_[statId][timeBinIdx];
  }

  /**
   * Return the aggregated value of stat "statId" in the past
   * MOVING_AVERAGE_WINDOW_SIZE_IN_SECOND seconds.
   * NOTE: This is only computed for rate_stats.
   *
   * @param statId      Index of the stat.
   */
  uint64_t getStatValueWithinWindow(size_t statId) const {
    return statsNumWithinWindow_[statId];
  }

  /**
   * Get the rate value of the stat "statId".
   *
   * @param statId      Index of the stat.
   */
  double getRateValue(size_t statId) const;

  /**
   * Increment the stat.
   *
   * @param stat    Stat to increment
   * @param amount  Amount to increment the stat
   */
  void increment(stat_name_t stat, int64_t amount = 1) {
    stat_incr(stats_, stat, amount);
  }

  /**
   * Decrement the stat.
   *
   * @param stat    Stat to decrement
   * @param amount  Amount to decrement the stat
   */
  void decrement(stat_name_t stat, int64_t amount = 1) {
    increment(stat, -amount);
  }

  /**
   * Increment the stat. Thread-safe.
   *
   * @param stat    Stat to increment
   * @param amount  Amount to increment the stat
   */
  void incrementSafe(stat_name_t stat, int64_t amount = 1) {
    stat_fetch_add(stats_, stat, amount);
  }

  /**
   * Decrement the stat. Thread-safe.
   *
   * @param stat    Stat to decrement
   * @param amount  Amount to decrement the stat
   */
  void decrementSafe(stat_name_t stat, int64_t amount = 1) {
    incrementSafe(stat, -amount);
  }

  /**
   * Set the stat value.
   *
   * @param stat      Stat to set
   * @param newValue  New value of the stat
   */
  void setValue(stat_name_t stat, uint64_t newValue) {
    stat_set(stats_, stat, newValue);
  }

  uint64_t getValue(stat_name_t stat) {
    return stat_get_uint64(stats_, stat);
  }
  uint64_t getConfigAge(uint64_t now) {
    return now - stat_get_uint64(stats_, config_last_success_stat);
  }
  stat_t& getStat(size_t statId) {
    return stats_[statId];
  }

  folly::F14NodeMap<std::string, stat_t> getAggregatedPoolStatsMap() const {
    folly::F14NodeMap<std::string, stat_t> poolStatsMap;
    for (const auto& poolStats : poolStats_) {
      for (const auto& stat : poolStats.getStats()) {
        poolStatsMap.emplace(stat.name, stat);
      }
    }
    return poolStatsMap;
  }

  /**
   * Returns pointer to the entry corresponding to the idx in
   * the poolStats vector. If the idx is invalid, nullptr is returned
   *
   * @param  idx
   * @return pointer to poolStats vector entry
   *         nullptr if idx is invalid
   */
  PoolStats* getPoolStats(int32_t idx) {
    if (idx < 0 || static_cast<size_t>(idx) >= poolStats_.size()) {
      return nullptr;
    }
    return &poolStats_[idx];
  }

 private:
  mutable std::mutex mutex_;
  stat_t stats_[num_stats]{};
  // vector of the PoolStats
  std::vector<PoolStats> poolStats_;

  ExponentialSmoothData<64> durationUs_;
  // Duration microseconds, broken down by get-like request type
  ExponentialSmoothData<64> durationGetUs_;
  // Duration microseconds, broken down by update-like request type
  ExponentialSmoothData<64> durationUpdateUs_;

  ExponentialSmoothData<64> inactiveConnectionClosedIntervalSec_;

  // Time spent for asynclog spooling
  ExponentialSmoothData<64> asyncLogDurationUs_;

  // Time spent for axonlog writing
  ExponentialSmoothData<64> axonLogDurationUs_;

  // Time spent for axonlog writing
  ExponentialSmoothData<64> axonProxyDurationUs_;

  // we are wasting some memory here to get faster mapping from stat name to
  // statsBin_[] and statsNumWithinWindow_[] entry. i.e., the statsBin_[]
  // and statsNumWithinWindow_[] entry for non-rate stat are not in use.

  // we maintain some information for calculating average rate in the past
  // MOVING_AVERAGE_WINDOW_SIZE_IN_SECOND seconds for every rate stat.

  /*
   * statsBin_[stat_name] is a circular array associated with stat "stat_name",
   * where each element (statsBin_[stat_name][idx]) is either the count (if it
   * is a rate_stat) or the max (if it is a max_stat) of "stat_name" in the
   * "idx"th time bin. The updater thread updates these circular arrays once
   * every MOVING_AVERAGE_WINDOW_SIZE_IN_SECOND second by setting the oldest
   * time bin to stats[stat_name], and then reset stats[stat_name] to 0.
   */
  uint64_t statsBin_[num_stats]
                    [MOVING_AVERAGE_WINDOW_SIZE_IN_SECOND /
                     MOVING_AVERAGE_BIN_SIZE_IN_SECOND]{};
  /*
   * statsNumWithinWindow_[stat_name] contains the count of stat "stat_name"
   * in the past MOVING_AVERAGE_WINDOW_SIZE_IN_SECOND seconds. This array is
   * also updated by the updater thread once every
   * MOVING_AVERAGE_WINDOW_SIZE_IN_SECOND seconds
   */
  uint64_t statsNumWithinWindow_[num_stats]{};

  /*
   * the number of bins currently used, which is initially set to 0, and is
   * increased by 1 every MOVING_AVERAGE_WINDOW_SIZE_IN_SECOND seconds.
   * numBinsUsed_ is at most MOVING_AVERAGE_WINDOW_SIZE_IN_SECOND /
   * MOVING_AVERAGE_BIN_SIZE_IN_SECOND
   */
  size_t numBinsUsed_{0};
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
