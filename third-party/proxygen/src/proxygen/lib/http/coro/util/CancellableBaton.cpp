/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/util/CancellableBaton.h"

namespace proxygen::coro::detail {

struct CancellableBaton::AwaiterCtx {
  TimedBaton::Status status{TimedBaton::Status::notReady};
  folly::HHWheelTimer::Callback& timer;
};

void CancellableBaton::signal(TimedBaton::Status status) noexcept {
  XLOG(DBG8) << __func__ << " signalled; status=" << int(status);
  // only resolve status for the first signaller
  if (awaiterCtx_ && awaiterCtx_->status == TimedBaton::Status::notReady) {
    awaiterCtx_->status = status;
  }
  baton_.post();
}

void CancellableBaton::reset() noexcept {
  XLOG(DBG8) << __func__;
  baton_.reset();
}

folly::coro::Task<TimedBaton::Status> CancellableBaton::timedWait(
    folly::EventBase* evb, std::chrono::milliseconds timeout) noexcept {
  // > 0ms timeout must have evb
  XCHECK(evb || timeout.count() == 0);
  XCHECK_EQ(awaiterCtx_, nullptr) << "supports only one awaiting coro";

  struct TimerCallback : public folly::HHWheelTimer::Callback {
    explicit TimerCallback(CancellableBaton& self) : self_(self) {
    }
    void timeoutExpired() noexcept override {
      self_.signal(TimedBaton::Status::timedout);
    }
    CancellableBaton& self_;
  } timerCb{*this};

  AwaiterCtx ctx{.timer = timerCb};
  awaiterCtx_ = &ctx;

  if (timeout.count() > 0) {
    evb->timer().scheduleTimeout(&timerCb, timeout);
  }

  /**
   * ::post baton when cancelled; if cancellation is requested, this will
   * immediately post the baton inline via CancellationCallback.
   *
   * Unfortunately because cancellation is (seldom) requested from another
   * thread, we don't invoke ::signal but instead directly baton_.post (as the
   * latter is thread-safe). We check for cancellation at the end and return the
   * appropriate status.
   */
  const auto& ct = co_await folly::coro::co_current_cancellation_token;
  folly::CancellationCallback cancelCallback{ct, [this]() { baton_.post(); }};
  co_await baton_;

  // possible if posted prior to ::wait(); if so transform status into
  // ::signalled
  auto status = std::exchange(awaiterCtx_, nullptr)->status;
  if (status == TimedBaton::Status::notReady) {
    status = TimedBaton::Status::signalled;
  }

  co_return ct.isCancellationRequested() ? TimedBaton::Status::cancelled
                                         : status;
}

folly::HHWheelTimer::Callback* CancellableBaton::getTimerCb() noexcept {
  return awaiterCtx_ ? &awaiterCtx_->timer : nullptr;
}

void DetachableCancellableBaton::detach() noexcept {
  if (auto* cb = getTimerCb()) {
    cb->cancelTimeout();
  }
}

} // namespace proxygen::coro::detail
