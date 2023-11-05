/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/ControlMessageRateLimitFilter.h>
#include <proxygen/lib/http/codec/DirectErrorsRateLimitFilter.h>
#include <proxygen/lib/http/codec/HeadersRateLimitFilter.h>
#include <proxygen/lib/http/codec/RateLimitFilter.h>

namespace proxygen {

std::string_view RateLimitFilter::toStr(Type type) {
  switch (type) {
    case Type::HEADERS:
      return "headers";
    case Type::MISC_CONTROL_MSGS:
      return "misc_control_msgs";
    case Type::RSTS:
      return "rsts";
    case Type::DIRECT_ERROR_HANDLING:
      return "direct_error_handling";
    default:
      return "unknown";
  }
}

std::unique_ptr<RateLimitFilter> RateLimitFilter::createRateLimitFilter(
    Type type, folly::HHWheelTimer* timer, HTTPSessionStats* httpSessionStats) {
  switch (type) {
    case Type::HEADERS:
      return std::make_unique<HeadersRateLimitFilter>(timer, httpSessionStats);
    case Type::MISC_CONTROL_MSGS:
      return std::make_unique<ControlMessageRateLimitFilter>(timer,
                                                             httpSessionStats);
    case Type::RSTS:
      return nullptr;
    case Type::DIRECT_ERROR_HANDLING:
      return std::make_unique<DirectErrorsRateLimitFilter>(timer,
                                                           httpSessionStats);
    default:
      return nullptr;
  }
}

bool RateLimitFilter::incrementNumEventsInCurrentInterval() {
  if (numEventsInCurrentInterval_ == 0) {
    // The first control message (or first after a reset) schedules the next
    // reset timer
    CHECK(timer_);
    timer_->scheduleTimeout(this, timeoutDuration_);
  }

  numEventsInCurrentInterval_++;
  bool rateLimitExceeded = (numEventsInCurrentInterval_ > maxEventsInInterval_);
  if (rateLimitExceeded) {
    recordRateLimitBreached();
  }
  return rateLimitExceeded;
}

void RateLimitFilter::setSessionStats(HTTPSessionStats* httpSessionStats) {
  httpSessionStats_ = httpSessionStats;
}

void RateLimitFilter::setParams(uint32_t maxEventsInInterval,
                                std::chrono::milliseconds timeoutDuration) {
  maxEventsInInterval_ = maxEventsInInterval;
  timeoutDuration_ = timeoutDuration;
}

void RateLimitFilter::attachThreadLocals(folly::HHWheelTimer* timer) {
  timer_ = timer;
}

void RateLimitFilter::detachThreadLocals() {
  cancelTimeout();
  timer_ = nullptr;
  // Free pass when switching threads
  numEventsInCurrentInterval_ = 0;
}

void RateLimitFilter::callbackCanceled() noexcept {
}

void RateLimitFilter::timeoutExpired() noexcept {
  recordNumEventsInCurrentInterval(numEventsInCurrentInterval_);
  numEventsInCurrentInterval_ = 0;
}

} // namespace proxygen
