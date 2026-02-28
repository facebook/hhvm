/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <unordered_map>

#include "watchman/WatchmanConfig.h"
#include "watchman/telemetry/LogEvent.h"

namespace watchman {

std::pair<int64_t, int64_t> getLogEventCounters(const LogEventType& type) {
  static std::unordered_map<LogEventType, std::atomic_int64_t> eventCounters;
  static int64_t samplingRate = cfg_get_int("scribe-sampling-rate", 100);

  // Find event counter or add if missing - init to 0;
  auto it = eventCounters.find(type);
  if (it == eventCounters.end()) {
    it = eventCounters.emplace(type, 0).first;
  }

  // Return sampling rate and event count
  auto& eventCounter = it->second;
  auto eventCount = ++eventCounter % samplingRate;
  return std::make_pair(samplingRate, eventCount ? eventCount : samplingRate);
}

} // namespace watchman
