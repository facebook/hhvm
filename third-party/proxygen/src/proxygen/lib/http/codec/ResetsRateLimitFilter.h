/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/HTTP2Framer.h>
#include <proxygen/lib/http/codec/RateLimitFilter.h>

namespace proxygen {

class ResetsRateLimiter : public RateLimiter {
 public:
  static const uint32_t kDefaultMaxEventsPerInterval = 10000;
  static const uint32_t kMaxEventsPerIntervalLowerBound = 100;
  static constexpr std::chrono::milliseconds kDefaultTimeoutDuration{1000};

  explicit ResetsRateLimiter(folly::HHWheelTimer* timer,
                             HTTPSessionStats* httpSessionStats)
      : RateLimiter(timer, httpSessionStats) {
    maxEventsInInterval_ = kDefaultMaxEventsPerInterval;
    timeoutDuration_ = kDefaultTimeoutDuration;
  }

  void recordNumEventsInCurrentInterval(uint32_t numEvents) override {
    if (httpSessionStats_) {
      httpSessionStats_->recordResetsInInterval(numEvents);
    }
  }

  void recordRateLimitBreached() override {
    if (httpSessionStats_) {
      httpSessionStats_->recordResetsRateLimited();
    }
  }

  uint32_t getMaxEventsPerInvervalLowerBound() const override {
    return kMaxEventsPerIntervalLowerBound;
  }
};

} // namespace proxygen
