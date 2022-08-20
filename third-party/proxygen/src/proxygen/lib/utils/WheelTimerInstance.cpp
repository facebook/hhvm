/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/WheelTimerInstance.h>

#include <folly/Singleton.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventBaseManager.h>

namespace proxygen {

WheelTimerInstance::WheelTimerInstance() {
}

WheelTimerInstance::WheelTimerInstance(folly::HHWheelTimer* timer)
    : wheelTimerPtr_(timer) {
  if (timer) {
    // If you use an external timer with no default timeout set, you will get
    // a check failed if you attempt to schedule a default timeout.
    defaultTimeoutMS_ = timer->getDefaultTimeout();
  }
}

WheelTimerInstance::WheelTimerInstance(
    std::chrono::milliseconds defaultTimeoutMS, folly::EventBase* eventBase)
    : defaultTimeoutMS_(defaultTimeoutMS) {
  if (!eventBase) {
    eventBase = folly::EventBaseManager::get()->getEventBase();
  }
  wheelTimerPtr_ = &eventBase->timer();
}

WheelTimerInstance::WheelTimerInstance(const WheelTimerInstance& timerInstance)
    : wheelTimerPtr_(timerInstance.wheelTimerPtr_),
      defaultTimeoutMS_(timerInstance.defaultTimeoutMS_) {
}

WheelTimerInstance::WheelTimerInstance(
    WheelTimerInstance&& timerInstance) noexcept
    : wheelTimerPtr_(std::move(timerInstance.wheelTimerPtr_)),
      defaultTimeoutMS_(std::move(timerInstance.defaultTimeoutMS_)) {
}

std::chrono::milliseconds WheelTimerInstance::getDefaultTimeout() const {
  return defaultTimeoutMS_;
}

void WheelTimerInstance::setDefaultTimeout(std::chrono::milliseconds timeout) {
  defaultTimeoutMS_ = timeout;
}

void WheelTimerInstance::scheduleTimeout(
    folly::HHWheelTimer::Callback* callback,
    std::chrono::milliseconds timeout) {
  if (wheelTimerPtr_) {
    wheelTimerPtr_->scheduleTimeout(callback, timeout);
  } else {
    VLOG(2) << "Ingoring scheduleTimeout on an empty WheelTimerInstance";
  }
}

void WheelTimerInstance::scheduleTimeout(
    folly::HHWheelTimer::Callback* callback) {
  CHECK_GE(defaultTimeoutMS_.count(), 0);
  scheduleTimeout(callback, defaultTimeoutMS_);
}

WheelTimerInstance& WheelTimerInstance::operator=(const WheelTimerInstance& t) {
  wheelTimerPtr_ = t.wheelTimerPtr_;
  defaultTimeoutMS_ = t.defaultTimeoutMS_;
  return *this;
}

WheelTimerInstance& WheelTimerInstance::operator=(
    const WheelTimerInstance&& timer) {
  wheelTimerPtr_ = std::move(timer.wheelTimerPtr_);
  defaultTimeoutMS_ = std::move(timer.defaultTimeoutMS_);
  return *this;
}

WheelTimerInstance::operator bool() const {
  return (wheelTimerPtr_ != nullptr);
}

} // namespace proxygen
