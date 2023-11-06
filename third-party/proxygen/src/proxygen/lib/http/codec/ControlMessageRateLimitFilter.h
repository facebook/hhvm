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

class ControlMessageRateLimitFilter : public RateLimitFilter {
 public:
  static const uint32_t kDefaultMaxEventsPerInterval = 50000;
  static const uint32_t kMaxEventsPerIntervalLowerBound = 100;
  static constexpr std::chrono::milliseconds kDefaultTimeoutDuration{100};

  explicit ControlMessageRateLimitFilter(folly::HHWheelTimer* timer,
                                         HTTPSessionStats* httpSessionStats)
      : RateLimitFilter(timer, httpSessionStats) {
    maxEventsInInterval_ = kDefaultMaxEventsPerInterval;
    timeoutDuration_ = kDefaultTimeoutDuration;
  }

  void onPingRequest(uint64_t data) override {
    if (!incrementNumEventsInCurrentInterval()) {
      callback_->onPingRequest(data);
    } else {
      sendErrorCallback(http2::FrameType::PING);
    }
  }
  void onSettings(const SettingsList& settings) override {
    if (!incrementNumEventsInCurrentInterval()) {
      callback_->onSettings(settings);
    } else {
      sendErrorCallback(http2::FrameType::SETTINGS);
    }
  }
  void onPriority(HTTPCodec::StreamID streamID,
                  const HTTPMessage::HTTP2Priority& pri) override {
    if (!incrementNumEventsInCurrentInterval()) {
      callback_->onPriority(streamID, pri);
    } else {
      sendErrorCallback(http2::FrameType::PRIORITY);
    }
  }

  void onPriority(StreamID streamID, const HTTPPriority& pri) override {
    if (!incrementNumEventsInCurrentInterval()) {
      callback_->onPriority(streamID, pri);
    } else {
      sendErrorCallback(http2::FrameType::PRIORITY);
    }
  }

  void recordNumEventsInCurrentInterval(uint32_t numEvents) override {
    if (httpSessionStats_) {
      httpSessionStats_->recordControlMsgsInInterval(numEvents);
    }
  }

  void recordRateLimitBreached() override {
    if (httpSessionStats_) {
      httpSessionStats_->recordControlMsgRateLimited();
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
            getFrameTypeString(frameType)));
    ex.setProxygenError(kErrorDropped);
    callback_->onError(0, ex, true);
  }
};

} // namespace proxygen
