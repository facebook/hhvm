/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/HHWheelTimer.h>
#include <proxygen/lib/http/codec/HTTP2Framer.h>
#include <proxygen/lib/http/codec/HTTPCodecFilter.h>

namespace proxygen {

// These constants define the rate at which we limit certain events.
constexpr uint32_t kDefaultMaxControlMsgsPerInterval = 500;
constexpr std::chrono::milliseconds kDefaultControlMsgDuration{100};

constexpr uint32_t kDefaultMaxDirectErrorHandlingPerInterval = 100;
constexpr std::chrono::milliseconds kDefaultDirectErrorHandlingDuration{100};

/**
 * This class implements the rate limiting logic for control messages and
 * stream errors (that might produce HTTP error pages).  If a rate limit is
 * exeeded, the callback is converted to a session level error with
 * ProxygenError = kErrorDropped.  This is a signal to the codec callback that
 * the codec would like the connection dropped.
 */
class ControlMessageRateLimitFilter : public PassThroughHTTPCodecFilter {
 public:
  explicit ControlMessageRateLimitFilter(folly::HHWheelTimer* timer)
      : timer_(timer) {
  }

  void setParams(
      uint32_t maxControlMsgsPerInterval,
      uint32_t maxDirectErrorHandlingPerInterval,
      std::chrono::milliseconds controlMsgIntervalDuration,
      std::chrono::milliseconds directErrorHandlingIntervalDuration) {
    maxControlMsgsPerInterval_ = maxControlMsgsPerInterval;
    maxDirectErrorHandlingPerInterval_ = maxDirectErrorHandlingPerInterval;
    controlMsgIntervalDuration_ = controlMsgIntervalDuration;
    directErrorHandlingIntervalDuration_ = directErrorHandlingIntervalDuration;
  }

  // Filter functions
  void onAbort(HTTPCodec::StreamID streamID, ErrorCode code) override {
    if (!incrementNumControlMsgsInCurInterval(http2::FrameType::RST_STREAM)) {
      callback_->onAbort(streamID, code);
    }
  }
  void onPingRequest(uint64_t data) override {
    if (!incrementNumControlMsgsInCurInterval(http2::FrameType::PING)) {
      callback_->onPingRequest(data);
    }
  }
  void onSettings(const SettingsList& settings) override {
    if (!incrementNumControlMsgsInCurInterval(http2::FrameType::SETTINGS)) {
      callback_->onSettings(settings);
    }
  }
  void onPriority(HTTPCodec::StreamID streamID,
                  const HTTPMessage::HTTP2Priority& pri) override {
    if (!incrementNumControlMsgsInCurInterval(http2::FrameType::PRIORITY)) {
      callback_->onPriority(streamID, pri);
    }
  }

  void onPriority(StreamID streamID, const HTTPPriority& pri) override {
    if (!incrementNumControlMsgsInCurInterval(http2::FrameType::PRIORITY)) {
      callback_->onPriority(streamID, pri);
    }
  }

  void onError(HTTPCodec::StreamID streamID,
               const HTTPException& error,
               bool newTxn) override {
    // We only rate limit stream errors with no codec status code.
    // These may trigger a direct HTTP response.
    if (streamID == 0 || error.hasCodecStatusCode() ||
        !incrementDirectErrorHandlingInCurInterval()) {
      callback_->onError(streamID, error, newTxn);
    }
  }

  void attachThreadLocals(folly::HHWheelTimer* timer) {
    timer_ = timer;
  }

  void detachThreadLocals() {
    resetControlMessages_.cancelTimeout();
    resetDirectErrors_.cancelTimeout();
    timer_ = nullptr;
    // Free pass when switching threads
    numControlMsgsInCurrentInterval_ = 0;
    numDirectErrorHandlingInCurrentInterval_ = 0;
  }

 private:
  bool incrementNumControlMsgsInCurInterval(http2::FrameType frameType) {
    if (numControlMsgsInCurrentInterval_ == 0) {
      // The first control message (or first after a reset) schedules the next
      // reset timer
      CHECK(timer_);
      timer_->scheduleTimeout(&resetControlMessages_,
                              controlMsgIntervalDuration_);
    }

    if (++numControlMsgsInCurrentInterval_ > maxControlMsgsPerInterval_) {
      HTTPException ex(
          HTTPException::Direction::INGRESS_AND_EGRESS,
          folly::to<std::string>(
              "dropping connection due to too many control messages, num "
              "control messages = ",
              numControlMsgsInCurrentInterval_,
              ", most recent frame type = ",
              getFrameTypeString(frameType)));
      ex.setProxygenError(kErrorDropped);
      callback_->onError(0, ex, true);
      return true;
    }

    return false;
  }

  bool incrementDirectErrorHandlingInCurInterval() {
    if (numDirectErrorHandlingInCurrentInterval_ == 0) {
      // The first control message (or first after a reset) schedules the next
      // reset timer
      CHECK(timer_);
      timer_->scheduleTimeout(&resetDirectErrors_,
                              directErrorHandlingIntervalDuration_);
    }

    if (++numDirectErrorHandlingInCurrentInterval_ >
        maxDirectErrorHandlingPerInterval_) {
      HTTPException ex(
          HTTPException::Direction::INGRESS_AND_EGRESS,
          folly::to<std::string>(
              "dropping connection due to too many newly created txns  when "
              "directly handling errors, num direct error handling cases = ",
              numDirectErrorHandlingInCurrentInterval_));
      ex.setProxygenError(kErrorDropped);
      callback_->onError(0, ex, true);
      return true;
    }

    return false;
  }

  class ResetCounterTimeout : public folly::HHWheelTimer::Callback {
   public:
    explicit ResetCounterTimeout(uint32_t& counter) : counter_(counter) {
    }

    void timeoutExpired() noexcept override {
      counter_ = 0;
    }
    void callbackCanceled() noexcept override {
    }

   private:
    uint32_t& counter_;
  };

  /**
   * The two variables below keep track of the number of Control messages,
   * and the number of error handling events that are handled by a newly
   * created transaction handler seen in the current interval, respectively.
   */
  uint32_t numControlMsgsInCurrentInterval_{0};
  uint32_t maxControlMsgsPerInterval_{kDefaultMaxControlMsgsPerInterval};

  uint32_t numDirectErrorHandlingInCurrentInterval_{0};
  uint32_t maxDirectErrorHandlingPerInterval_{
      kDefaultMaxDirectErrorHandlingPerInterval};

  std::chrono::milliseconds controlMsgIntervalDuration_{
      kDefaultControlMsgDuration};
  std::chrono::milliseconds directErrorHandlingIntervalDuration_{
      kDefaultDirectErrorHandlingDuration};

  ResetCounterTimeout resetControlMessages_{numControlMsgsInCurrentInterval_};
  ResetCounterTimeout resetDirectErrors_{
      numDirectErrorHandlingInCurrentInterval_};
  folly::HHWheelTimer* timer_{nullptr};
};

} // namespace proxygen
