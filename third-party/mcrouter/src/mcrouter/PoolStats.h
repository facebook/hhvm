/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/container/F14Map.h>

#include "mcrouter/ExponentialSmoothData.h"
#include "mcrouter/stats.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

class PoolStats {
 public:
  PoolStats(folly::StringPiece poolName)
      : requestsCountStatName_(
            folly::to<std::string>(poolName, ".requests.sum")),
        finalResultErrorStatName_(
            folly::to<std::string>(poolName, ".final_result_error.sum")),
        nConnectionsStatName_(folly::to<std::string>(poolName, ".connections")),
        durationUsStatName_(
            folly::to<std::string>(poolName, ".duration_us.avg")),
        totalDurationUsStatName_(
            folly::to<std::string>(poolName, ".total_duration_us.avg")) {
    initStat(requestCountStat_, requestsCountStatName_);
    initStat(finalResultErrorStat_, finalResultErrorStatName_);
    initStat(nConnectionsStat_, nConnectionsStatName_);
  }

  std::vector<stat_t> getStats() const {
    stat_t durationStat;
    stat_t totalDurationStat;
    initStat(durationStat, durationUsStatName_);
    initStat(totalDurationStat, totalDurationUsStatName_);
    folly::make_atomic_ref(durationStat.data.uint64)
        .store(durationUsStat_.value(), std::memory_order_relaxed);
    folly::make_atomic_ref(totalDurationStat.data.uint64)
        .store(totalDurationUsStat_.value(), std::memory_order_relaxed);

    return {
        requestCountStat_,
        finalResultErrorStat_,
        nConnectionsStat_,
        std::move(durationStat),
        std::move(totalDurationStat)};
  }

  void incrementRequestCount(uint64_t amount = 1) {
    auto ref = folly::make_atomic_ref(requestCountStat_.data.uint64);
    ref.store(
        ref.load(std::memory_order_relaxed) + amount,
        std::memory_order_relaxed);
  }

  void incrementFinalResultErrorCount(uint64_t amount = 1) {
    auto ref = folly::make_atomic_ref(finalResultErrorStat_.data.uint64);
    ref.store(
        ref.load(std::memory_order_relaxed) + amount,
        std::memory_order_relaxed);
  }

  void addDurationSample(int64_t duration) {
    durationUsStat_.insertSample(duration);
  }

  void updateConnections(int64_t amount = 1) {
    auto ref = folly::make_atomic_ref(nConnectionsStat_.data.uint64);
    ref.store(
        ref.load(std::memory_order_relaxed) + amount,
        std::memory_order_relaxed);
  }

  void addTotalDurationSample(int64_t duration) {
    totalDurationUsStat_.insertSample(duration);
  }

 private:
  void initStat(stat_t& stat, folly::StringPiece name) const {
    stat.name = name;
    stat.group = ods_stats | count_stats;
    stat.type = stat_uint64;
    stat.aggregate = 0;
    folly::make_atomic_ref(stat.data.uint64)
        .store(0, std::memory_order_relaxed);
  }

  const std::string requestsCountStatName_;
  const std::string finalResultErrorStatName_;
  const std::string nConnectionsStatName_;
  const std::string durationUsStatName_;
  const std::string totalDurationUsStatName_;
  stat_t nConnectionsStat_;
  stat_t requestCountStat_;
  stat_t finalResultErrorStat_;
  ExponentialSmoothData<64> totalDurationUsStat_;
  ExponentialSmoothData<64> durationUsStat_;
};

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
