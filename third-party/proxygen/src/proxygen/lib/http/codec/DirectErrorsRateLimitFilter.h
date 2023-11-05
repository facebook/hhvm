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

class DirectErrorsRateLimitFilter : public RateLimitFilter {
 public:
  static const uint32_t kDefaultMaxEventsPerInterval = 100;
  static const uint32_t kMaxEventsPerIntervalLowerBound = 50;
  static constexpr std::chrono::milliseconds kDefaultTimeoutDuration{100};

  explicit DirectErrorsRateLimitFilter(folly::HHWheelTimer* timer,
                                       HTTPSessionStats* httpSessionStats)
      : RateLimitFilter(timer, httpSessionStats) {
    maxEventsInInterval_ = kDefaultMaxEventsPerInterval;
    timeoutDuration_ = kDefaultTimeoutDuration;
  }

  void onError(HTTPCodec::StreamID streamID,
               const HTTPException& error,
               bool newTxn) override {
    // We only rate limit stream errors with no codec status code.
    // These may trigger a direct HTTP response.
    if (streamID == 0 || error.hasCodecStatusCode()) {
      callback_->onError(streamID, error, newTxn);
    } else {
      if (incrementNumEventsInCurrentInterval()) {
        HTTPException ex(
            HTTPException::Direction::INGRESS_AND_EGRESS,
            folly::to<std::string>(
                "dropping connection due to too many newly created txns  when "
                "directly handling errors, num direct error handling cases = ",
                numEventsInCurrentInterval_));
        ex.setProxygenError(kErrorDropped);
        callback_->onError(0, ex, true);
      } else {
        callback_->onError(streamID, error, newTxn);
      }
    }
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
