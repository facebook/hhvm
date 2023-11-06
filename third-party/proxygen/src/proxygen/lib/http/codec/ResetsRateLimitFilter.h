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

class ResetsRateLimitFilter : public RateLimitFilter {
 public:
  static const uint32_t kDefaultMaxEventsPerInterval = 200;
  static const uint32_t kMaxEventsPerIntervalLowerBound = 100;
  static constexpr std::chrono::milliseconds kDefaultTimeoutDuration{1000};

  explicit ResetsRateLimitFilter(folly::HHWheelTimer* timer,
                                 HTTPSessionStats* httpSessionStats)
      : RateLimitFilter(timer, httpSessionStats) {
    maxEventsInInterval_ = kDefaultMaxEventsPerInterval;
    timeoutDuration_ = kDefaultTimeoutDuration;
  }

  void onAbort(HTTPCodec::StreamID streamID, ErrorCode code) override {
    if (!incrementNumEventsInCurrentInterval()) {
      callback_->onAbort(streamID, code);
    } else {
      sendErrorCallback(http2::FrameType::RST_STREAM);
    }
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

 private:
  void sendErrorCallback(http2::FrameType frameType) {
    HTTPException ex(
        HTTPException::Direction::INGRESS_AND_EGRESS,
        folly::to<std::string>(
            "dropping connection due to too many control messages, num "
            "control messages = ",
            numEventsInCurrentInterval_,
            ", most recent frame type = ",
            getFrameTypeString(http2::FrameType::RST_STREAM)));
    ex.setProxygenError(kErrorDropped);
    callback_->onError(0, ex, true);
  }
};

} // namespace proxygen
