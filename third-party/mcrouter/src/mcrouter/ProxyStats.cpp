/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ProxyStats.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

ProxyStats::ProxyStats(const std::vector<std::string>& statsEnabledPools) {
  init_stats(stats_);
  poolStats_.reserve(statsEnabledPools.size());
  for (const auto& curPoolName : statsEnabledPools) {
    poolStats_.emplace_back(curPoolName);
  }
}

void ProxyStats::aggregate(size_t statId) {
  constexpr size_t kBinNum =
      MOVING_AVERAGE_WINDOW_SIZE_IN_SECOND / MOVING_AVERAGE_BIN_SIZE_IN_SECOND;

  if (numBinsUsed_ < kBinNum) {
    ++numBinsUsed_;
  }

  for (int j = 0; j < num_stats; ++j) {
    if (stats_[j].group & rate_stats) {
      statsNumWithinWindow_[j] -= statsBin_[j][statId];
      auto ref = folly::make_atomic_ref(stats_[j].data.uint64);
      statsBin_[j][statId] = ref.load(std::memory_order_relaxed);
      statsNumWithinWindow_[j] += statsBin_[j][statId];
      ref.store(0, std::memory_order_relaxed);
    } else if (stats_[j].group & (max_stats | max_max_stats)) {
      auto ref = folly::make_atomic_ref(stats_[j].data.uint64);
      statsBin_[j][statId] = ref.load(std::memory_order_relaxed);
      ref.store(0, std::memory_order_relaxed);
    }
  }
}

std::unique_lock<std::mutex> ProxyStats::lock() const {
  return std::unique_lock<std::mutex>(mutex_);
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
