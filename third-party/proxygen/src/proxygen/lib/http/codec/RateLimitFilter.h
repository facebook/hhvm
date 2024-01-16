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
#include <proxygen/lib/http/session/HTTPSessionStats.h>

namespace proxygen {

class RateLimiter : public folly::HHWheelTimer::Callback {
 public:
  enum class Type : uint8_t {
    HEADERS = 0,
    MISC_CONTROL_MSGS = 1,
    RSTS = 2,
    DIRECT_ERROR_HANDLING = 3,
    // Upper bound
    MAX = 4,
  };

  static_assert(folly::to_underlying(Type::DIRECT_ERROR_HANDLING) + 1 ==
                folly::to_underlying(Type::MAX));

  static std::string_view toStr(Type type);

  static std::unique_ptr<RateLimiter> createRateLimiter(
      Type type,
      folly::HHWheelTimer* timer,
      HTTPSessionStats* httpSessionStats);

  RateLimiter(folly::HHWheelTimer* timer, HTTPSessionStats* httpSessionStats)
      : timer_(timer), httpSessionStats_(httpSessionStats) {
  }

  virtual bool incrementNumEventsInCurrentInterval();

  virtual void recordNumEventsInCurrentInterval(uint32_t) = 0;

  virtual void recordRateLimitBreached() = 0;

  virtual uint32_t getMaxEventsPerInvervalLowerBound() const = 0;

  void setSessionStats(HTTPSessionStats* httpSessionStats);

  void setParams(uint32_t maxEventsInInterval,
                 std::chrono::milliseconds timeoutDuration);

  void attachThreadLocals(folly::HHWheelTimer* timer) {
    timer_ = timer;
  }

  void detachThreadLocals() {
    cancelTimeout();
    timer_ = nullptr;
    // Free pass when switching threads
    numEventsInCurrentInterval_ = 0;
  }

  [[nodiscard]] uint32_t numEventsInCurrentInterval() const {
    return numEventsInCurrentInterval_;
  }

 protected:
  void callbackCanceled() noexcept override;

  void timeoutExpired() noexcept override;

  uint32_t numEventsInCurrentInterval_{0};
  uint32_t maxEventsInInterval_{0};
  std::chrono::milliseconds timeoutDuration_;

  folly::HHWheelTimer* timer_;
  HTTPSessionStats* httpSessionStats_;
};

class RateLimitFilter : public PassThroughHTTPCodecFilter {
 public:
  RateLimitFilter(folly::HHWheelTimer* timer,
                  HTTPSessionStats* httpSessionStats)
      : timer_(timer), httpSessionStats_(httpSessionStats) {
  }

  void addRateLimiter(RateLimiter::Type type) {
    CHECK_LT(folly::to_underlying(type),
             folly::to_underlying(RateLimiter::Type::MAX))
        << "Received a rate limit type that exceeded the specified maximum";
    auto index = folly::to_underlying(type);
    if (!rateLimiters_[index]) {
      rateLimiters_[index] =
          RateLimiter::createRateLimiter(type, timer_, httpSessionStats_);
      CHECK(rateLimiters_[index])
          << "Unable to construct a rate limit filter of type "
          << RateLimiter::toStr(type);
    }
  }

  void setRateLimitParams(RateLimiter::Type type,
                          uint32_t maxEventsPerInterval,
                          std::chrono::milliseconds intervalDuration) {
    uint32_t typeIndex = folly::to_underlying(type);
    CHECK_LT(typeIndex, folly::to_underlying(RateLimiter::Type::MAX))
        << "Out of bounds access to rate limit filter array";
    auto& rateLimiter = rateLimiters_.at(typeIndex);
    if (rateLimiter) {
      uint32_t maxEventsPerIntervalLowerBound =
          rateLimiter->getMaxEventsPerInvervalLowerBound();
      if (maxEventsPerInterval < maxEventsPerIntervalLowerBound) {
        LOG(WARNING) << "Invalid maxEventsPerInterval for event "
                     << RateLimiter::toStr(type) << ": "
                     << maxEventsPerInterval;
        maxEventsPerInterval = maxEventsPerIntervalLowerBound;
      }
      rateLimiter->setParams(maxEventsPerInterval, intervalDuration);
    }
  }

  void onHeadersComplete(StreamID stream,
                         std::unique_ptr<HTTPMessage> msg) override {
    auto& rateLimiter =
        rateLimiters_[folly::to_underlying(RateLimiter::Type::HEADERS)];
    if (!rateLimiter || !rateLimiter->incrementNumEventsInCurrentInterval()) {
      callback_->onHeadersComplete(stream, std::move(msg));
    } else {
      callback_->onGoaway(http2::kMaxStreamID, ErrorCode::NO_ERROR);
    }
  }
  void onAbort(HTTPCodec::StreamID streamID, ErrorCode code) override {
    auto& rateLimiter =
        rateLimiters_[folly::to_underlying(RateLimiter::Type::RSTS)];
    if (!rateLimiter || !rateLimiter->incrementNumEventsInCurrentInterval()) {
      callback_->onAbort(streamID, code);
    } else {
      sendErrorCallback(http2::FrameType::RST_STREAM,
                        rateLimiter->numEventsInCurrentInterval());
    }
  }
  void onPingRequest(uint64_t data) override {
    auto& rateLimiter = rateLimiters_[folly::to_underlying(
        RateLimiter::Type::MISC_CONTROL_MSGS)];
    if (!rateLimiter || !rateLimiter->incrementNumEventsInCurrentInterval()) {
      callback_->onPingRequest(data);
    } else {
      sendErrorCallback(http2::FrameType::PING,
                        rateLimiter->numEventsInCurrentInterval());
    }
  }
  void onSettings(const SettingsList& settings) override {
    auto& rateLimiter = rateLimiters_[folly::to_underlying(
        RateLimiter::Type::MISC_CONTROL_MSGS)];
    if (!rateLimiter || !rateLimiter->incrementNumEventsInCurrentInterval()) {
      callback_->onSettings(settings);
    } else {
      sendErrorCallback(http2::FrameType::SETTINGS,
                        rateLimiter->numEventsInCurrentInterval());
    }
  }
  void onPriority(HTTPCodec::StreamID streamID,
                  const HTTPMessage::HTTP2Priority& pri) override {
    auto& rateLimiter = rateLimiters_[folly::to_underlying(
        RateLimiter::Type::MISC_CONTROL_MSGS)];
    if (!rateLimiter || !rateLimiter->incrementNumEventsInCurrentInterval()) {
      callback_->onPriority(streamID, pri);
    } else {
      sendErrorCallback(http2::FrameType::PRIORITY,
                        rateLimiter->numEventsInCurrentInterval());
    }
  }

  void onPriority(StreamID streamID, const HTTPPriority& pri) override {
    auto& rateLimiter = rateLimiters_[folly::to_underlying(
        RateLimiter::Type::MISC_CONTROL_MSGS)];
    if (!rateLimiter || !rateLimiter->incrementNumEventsInCurrentInterval()) {
      callback_->onPriority(streamID, pri);
    } else {
      sendErrorCallback(http2::FrameType::PRIORITY,
                        rateLimiter->numEventsInCurrentInterval());
    }
  }
  void onError(HTTPCodec::StreamID streamID,
               const HTTPException& error,
               bool newTxn) override {
    auto& rateLimiter = rateLimiters_[folly::to_underlying(
        RateLimiter::Type::DIRECT_ERROR_HANDLING)];
    // We only rate limit stream errors with no codec status code.
    // These may trigger a direct HTTP response.
    if (!rateLimiter || streamID == 0 || error.hasCodecStatusCode()) {
      callback_->onError(streamID, error, newTxn);
    } else {
      if (rateLimiter->incrementNumEventsInCurrentInterval()) {
        HTTPException ex(
            HTTPException::Direction::INGRESS_AND_EGRESS,
            folly::to<std::string>(
                "dropping connection due to too many newly created txns  when "
                "directly handling errors, num direct error handling cases = ",
                rateLimiter->numEventsInCurrentInterval()));
        ex.setProxygenError(kErrorDropped);
        callback_->onError(0, ex, true);
      } else {
        callback_->onError(streamID, error, newTxn);
      }
    }
  }

  void setSessionStats(HTTPSessionStats* httpSessionStats) {
    httpSessionStats_ = httpSessionStats;
    for (auto& rateLimiter : rateLimiters_) {
      if (rateLimiter) {
        rateLimiter->setSessionStats(httpSessionStats_);
      }
    }
  }

  void attachThreadLocals(folly::HHWheelTimer* timer) {
    timer_ = timer;
    for (auto& rateLimiter : rateLimiters_) {
      if (rateLimiter) {
        rateLimiter->attachThreadLocals(timer);
      }
    }
  }

  void detachThreadLocals() {
    for (auto& rateLimiter : rateLimiters_) {
      if (rateLimiter) {
        rateLimiter->detachThreadLocals();
      }
    }
    timer_ = nullptr;
  }

 private:
  folly::HHWheelTimer* timer_;
  HTTPSessionStats* httpSessionStats_;

  void sendErrorCallback(http2::FrameType frameType,
                         uint64_t numEventsInCurrentInterval) {
    HTTPException ex(
        HTTPException::Direction::INGRESS_AND_EGRESS,
        folly::to<std::string>(
            "dropping connection due to too many control messages, num "
            "control messages = ",
            numEventsInCurrentInterval,
            ", most recent frame type = ",
            getFrameTypeString(frameType)));
    ex.setProxygenError(kErrorDropped);
    callback_->onError(0, ex, true);
  }

  std::array<std::unique_ptr<RateLimiter>,
             folly::to_underlying(RateLimiter::Type::MAX)>
      rateLimiters_{};
};

} // namespace proxygen
