/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/RateLimitFilter.h>

namespace proxygen {

class DirectErrorsRateLimiter : public RateLimiter {
 public:
  static const uint32_t kDefaultMaxEventsPerInterval = 100;
  static const uint32_t kMaxEventsPerIntervalLowerBound = 50;
  static constexpr std::chrono::milliseconds kDefaultTimeoutDuration{100};

  explicit DirectErrorsRateLimiter(folly::HHWheelTimer* timer,
                                   HTTPSessionStats* httpSessionStats)
      : RateLimiter(timer, httpSessionStats) {
    maxEventsInInterval_ = kDefaultMaxEventsPerInterval;
    timeoutDuration_ = kDefaultTimeoutDuration;
  }

  void recordNumEventsInCurrentInterval(uint32_t /* numEvents */) override {
    // We don't currently record the number of direct errors in an interval
  }

  void recordRateLimitBreached() override {
    // We don't currently record how frequenlty we breach the direct errors
    // rate limit in an interval
  }

  uint32_t getMaxEventsPerInvervalLowerBound() const override {
    return kMaxEventsPerIntervalLowerBound;
  }
};

} // namespace proxygen
