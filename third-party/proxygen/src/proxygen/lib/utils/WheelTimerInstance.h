/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <memory>

#include <folly/io/async/HHWheelTimer.h>

namespace folly {

class EventBase;

}

namespace proxygen {

/*
 * Class to be used to schedule timeouts, has associated HHWheelTimer & timeout
 */
class WheelTimerInstance {
 public:
  // will ignore all scheduleTimeout operations, to be used instead of
  // nullptr for HHWheelTimer
  WheelTimerInstance();

  // will use WheelTimer of the EventBase thread
  explicit WheelTimerInstance(std::chrono::milliseconds defaultTimeoutMS,
                              folly::EventBase* eventBase = nullptr);

  WheelTimerInstance(const WheelTimerInstance& timerInstance);
  WheelTimerInstance(WheelTimerInstance&& timerInstance) noexcept;

  // timer could be nullptr which is correct usecase meaning that timeout
  // will not be scheduled
  explicit WheelTimerInstance(folly::HHWheelTimer* timer);

  std::chrono::milliseconds getDefaultTimeout() const;
  void setDefaultTimeout(std::chrono::milliseconds timeout);

  // These timeout callbacks will be scheduled on the current thread
  void scheduleTimeout(folly::HHWheelTimer::Callback* callback,
                       std::chrono::milliseconds timeout);
  void scheduleTimeout(folly::HHWheelTimer::Callback* callback);

  WheelTimerInstance& operator=(const WheelTimerInstance& timer);
  WheelTimerInstance& operator=(const WheelTimerInstance&& timer);

  // returns true if it is empty
  explicit operator bool() const;

  folly::HHWheelTimer* getWheelTimer() const {
    return wheelTimerPtr_;
  }

 private:
  folly::HHWheelTimer* wheelTimerPtr_{nullptr}; // to support cases when
  // external WheelTimer is
  // specified

  std::chrono::milliseconds defaultTimeoutMS_;
};

} // namespace proxygen
