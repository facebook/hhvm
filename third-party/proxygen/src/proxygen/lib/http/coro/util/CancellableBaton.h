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
 * supports both cancellation and timeout. This class is not thread-safe and is
 * designed to be used within a single EventBase.
 */
class CancellableBaton {
 public:
  CancellableBaton() noexcept = default;
  CancellableBaton(const CancellableBaton&) = delete;
  CancellableBaton& operator=(const CancellableBaton&) = delete;
  CancellableBaton(CancellableBaton&&) = delete;
  CancellableBaton& operator=(CancellableBaton&&) = delete;
  ~CancellableBaton() noexcept {
    XCHECK_EQ(awaiterCtx_, nullptr);
  }

  // wait indefinitely until ::signal is called or cancellation requested
  folly::coro::Task<TimedBaton::Status> wait() noexcept {
    return timedWait(/*evb=*/nullptr, std::chrono::milliseconds::zero());
  }

  // wait for timeout (indefinite if 0ms), ::signal or cancellation requested
  folly::coro::Task<TimedBaton::Status> timedWait(
      folly::EventBase* evb, std::chrono::milliseconds timeout) noexcept;

  void reset() noexcept;

  void signal() noexcept {
    signal(TimedBaton::Status::signalled);
  }

  bool ready() const noexcept {
    return baton_.ready();
  }

 protected:
  folly::HHWheelTimer::Callback* getTimerCb() noexcept;

 private:
  void signal(TimedBaton::Status status) noexcept;

  // A suspending coroutine will allocate an AwaiterCtx on the stack.
  struct AwaiterCtx;
  AwaiterCtx* awaiterCtx_{nullptr};

  folly::coro::Baton baton_;
};

struct DetachableCancellableBaton : public CancellableBaton {
  void detach() noexcept;
  using CancellableBaton::getTimerCb;
};

} // namespace proxygen::coro::detail
