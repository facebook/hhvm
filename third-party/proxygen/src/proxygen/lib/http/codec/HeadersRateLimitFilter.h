/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/HTTP2Constants.h>
#include <proxygen/lib/http/codec/RateLimitFilter.h>

namespace proxygen {

class HeadersRateLimitFilter : public RateLimitFilter {
 public:
  static const uint32_t kDefaultMaxEventsPerInterval = 50000;
  static const uint32_t kMaxEventsPerIntervalLowerBound = 100;
  static constexpr std::chrono::milliseconds kDefaultTimeoutDuration{100};

  explicit HeadersRateLimitFilter(folly::HHWheelTimer* timer,
                                  HTTPSessionStats* httpSessionStats)
      : RateLimitFilter(timer, httpSessionStats) {
    maxEventsInInterval_ = kDefaultMaxEventsPerInterval;
    timeoutDuration_ = kDefaultTimeoutDuration;
  }

  void onHeadersComplete(StreamID stream,
                         std::unique_ptr<HTTPMessage> msg) override {
    if (!incrementNumEventsInCurrentInterval()) {
      callback_->onHeadersComplete(stream, std::move(msg));
    } else {
      callback_->onGoaway(http2::kMaxStreamID, ErrorCode::NO_ERROR);
    }
  }

  void recordNumEventsInCurrentInterval(uint32_t numEvents) override {
    if (httpSessionStats_) {
      httpSessionStats_->recordHeadersInInterval(numEvents);
    }
  }

  void recordRateLimitBreached() override {
    if (httpSessionStats_) {
      httpSessionStats_->recordHeadersRateLimited();
    }
  }

  uint32_t getMaxEventsPerInvervalLowerBound() const override {
    return kMaxEventsPerIntervalLowerBound;
  }
};

} // namespace proxygen
