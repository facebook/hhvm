/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/util/TimedBaton.h"

namespace proxygen::coro::detail {

/**
 * A simple single waiter single poster wrapper around folly::coro::Baton that
 * supports both cancellation and timeout
 */
class CancellableBaton {
 public:
  CancellableBaton() = default;
  ~CancellableBaton() {
    XCHECK_EQ(status_, nullptr);
  }

  // wait indefinitely until ::signal is called or cancellation requested
  folly::coro::Task<TimedBaton::Status> wait() noexcept;

  // wait for timeout (indefinite if 0ms), ::signal or cancellation requested
  folly::coro::Task<TimedBaton::Status> timedWait(
      folly::EventBase* evb, std::chrono::milliseconds timeout) noexcept {
    // zero ms timeout considered indefinite timeout
    return timeout.count() == 0 ? wait() : timedWaitImpl(evb, timeout);
  }

  void reset() {
    XLOG(DBG8) << __func__;
    status_ = nullptr;
    baton_.reset();
  }

  void signal() {
    signal(TimedBaton::Status::signalled);
  }

  bool ready() const {
    return baton_.ready();
  }

 protected:
  folly::HHWheelTimer::Callback& getTimerCb() {
    return timerCb_;
  }

 private:
  folly::coro::Task<TimedBaton::Status> timedWaitImpl(
      folly::EventBase* evb, std::chrono::milliseconds timeout);

  void signal(TimedBaton::Status status) noexcept;

  struct TimerCallback : public folly::HHWheelTimer::Callback {
    explicit TimerCallback(CancellableBaton& self) : self_(self) {
    }
    void timeoutExpired() noexcept override {
      self_.signal(TimedBaton::Status::timedout);
    }
    CancellableBaton& self_;
  } timerCb_{*this};

  TimedBaton::Status* status_{nullptr};
  folly::coro::Baton baton_;
};

struct DetachableCancellableBaton : public CancellableBaton {
  void detach() noexcept;
};

} // namespace proxygen::coro::detail
