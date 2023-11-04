/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/HHWheelTimer.h>
#include <proxygen/lib/http/codec/HTTPCodecFilter.h>
#include <proxygen/lib/http/session/HTTPSessionStats.h>

namespace proxygen {

class RateLimitFilter
    : public PassThroughHTTPCodecFilter
    , public folly::HHWheelTimer::Callback {
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

  static std::unique_ptr<RateLimitFilter> createRateLimitFilter(
      Type type,
      folly::HHWheelTimer* timer,
      HTTPSessionStats* httpSessionStats);

  RateLimitFilter(folly::HHWheelTimer* timer,
                  HTTPSessionStats* httpSessionStats)
      : timer_(timer), httpSessionStats_(httpSessionStats) {
  }

  virtual bool incrementNumEventsInCurrentInterval();

  virtual void recordNumEventsInCurrentInterval(uint32_t) = 0;

  virtual void recordRateLimitBreached() = 0;

  virtual uint32_t getMaxEventsPerInvervalLowerBound() const = 0;

  void setSessionStats(HTTPSessionStats* httpSessionStats);

  void setParams(uint32_t maxEventsInInterval,
                 std::chrono::milliseconds timeoutDuration);

  void attachThreadLocals(folly::HHWheelTimer* timer);

  void detachThreadLocals();

 protected:
  void callbackCanceled() noexcept override;

  void timeoutExpired() noexcept override;

  uint32_t numEventsInCurrentInterval_{0};
  uint32_t maxEventsInInterval_{0};
  std::chrono::milliseconds timeoutDuration_;

  folly::HHWheelTimer* timer_;
  HTTPSessionStats* httpSessionStats_;
};

} // namespace proxygen
