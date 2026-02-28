/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

#include <folly/Range.h>

#include "mcrouter/TkoCounters.h"
#include "mcrouter/lib/carbon/Result.h"

namespace facebook {
namespace memcache {

struct AccessPoint;

namespace mcrouter {

enum class TkoLogEvent {
  MarkHardTko,
  MarkSoftTko,
  RemoveFromConfig,
  UnMarkTko
};

struct TkoLog {
  TkoLog(const AccessPoint& ap, const TkoCounters& gt);

  std::string eventName() const;

  TkoLogEvent event{TkoLogEvent::MarkHardTko};
  uintptr_t curSumFailures{0};
  bool isHardTko{false};
  bool isSoftTko{false};
  carbon::Result result;
  size_t probesSent{0};
  double avgLatency{0.0};
  const AccessPoint& accessPoint;
  const TkoCounters& globalTkos;
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
