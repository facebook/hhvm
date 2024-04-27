/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>

#include <folly/ThreadLocal.h>

#include "eden/common/telemetry/DurationScope.h"
#include "eden/common/telemetry/Stats.h"
#include "eden/common/telemetry/StatsGroup.h"
#include "eden/common/utils/RefPtr.h"

namespace watchman {

using StatsGroupBase = facebook::eden::StatsGroupBase;
using TelemetryStats = facebook::eden::TelemetryStats;

class WatchmanStats : public facebook::eden::RefCounted {
 public:
  /**
   * Records a specified elapsed duration. Updates thread-local storage, and
   * aggregates into the fb303 ServiceData in the background and on reads.
   */
  template <typename T, typename Rep, typename Period>
  void addDuration(
      StatsGroupBase::Duration T::*duration,
      std::chrono::duration<Rep, Period> elapsed) {
    (getStatsForCurrentThread<T>().*duration).addDuration(elapsed);
  }

  template <typename T>
  void increment(StatsGroupBase::Counter T::*counter, double value = 1.0) {
    (getStatsForCurrentThread<T>().*counter).addValue(value);
  }

  /**
   * Aggregates thread-locals into fb303's ServiceData.
   *
   * This function can be called on any thread.
   */
  void flush();

  template <typename T>
  T& getStatsForCurrentThread() = delete;

 private:
  class ThreadLocalTag {};

  template <typename T>
  using ThreadLocal = folly::ThreadLocal<T, ThreadLocalTag, void>;

  ThreadLocal<TelemetryStats> telemetryStats_;
};

using WatchmanStatsPtr = facebook::eden::RefPtr<WatchmanStats>;

template <>
inline TelemetryStats&
WatchmanStats::getStatsForCurrentThread<TelemetryStats>() {
  return *telemetryStats_.get();
}

WatchmanStatsPtr getWatchmanStats();

} // namespace watchman
