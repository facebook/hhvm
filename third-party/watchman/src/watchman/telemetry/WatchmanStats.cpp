/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/telemetry/WatchmanStats.h"

#include <memory>

#include <fb303/ServiceData.h>
#include <fb303/ThreadCachedServiceData.h>

namespace watchman {

void WatchmanStats::flush() {
  // Counters accumulate in thread-local cells that the ThreadCachedServiceData
  // publish thread drains periodically; durations are quantile stats that are
  // aggregated on read. A reader that needs this instant's values calls this.
  facebook::fb303::ThreadCachedServiceData::get()->publishStats();
  facebook::fb303::ServiceData::get()->getQuantileStatMap()->flushAll();
}

WatchmanStatsPtr getWatchmanStats() {
  // A running Watchman daemon only needs a single WatchmanStats instance. Avoid
  // atomic reference counts with RefPtr::singleton. We could use
  // folly::Singleton but that makes unit testing harder.
  static WatchmanStats* gWatchmanStats = new WatchmanStats;
  return WatchmanStatsPtr::singleton(*gWatchmanStats);
}

} // namespace watchman
