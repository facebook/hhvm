/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/telemetry/WatchmanStats.h"

#include <memory>

namespace watchman {

void WatchmanStats::flush() {
  // This method is only really useful while testing to ensure that the service
  // data singleton instance has the latest stats. Since all our stats are now
  // quantile stat based, flushing the quantile stat map is sufficient for that
  // use case.
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
