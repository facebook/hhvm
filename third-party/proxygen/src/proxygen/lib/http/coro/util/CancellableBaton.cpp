/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/util/CancellableBaton.h"

namespace proxygen::coro::detail {

void CancellableBaton::signal(TimedBaton::Status status) noexcept {
  XLOG(DBG8) << __func__ << " signalled; status=" << int(status);
  // only resolve status for the first signaller
  auto* pStatus = std::exchange(status_, nullptr);
  if (pStatus && *pStatus == TimedBaton::Status::notReady) {
    *pStatus = status;
  }
  baton_.post();
}

folly::coro::Task<TimedBaton::Status> CancellableBaton::wait() noexcept {
  XCHECK_EQ(status_, nullptr) << "supports only one awaiting coro";

  TimedBaton::Status status = TimedBaton::Status::notReady;
  status_ = &status;

  /**
   * __Does not support cancellation from another thread__
   * ::post baton when cancelled; if cancellation is requested, this will
   * immediately post the baton inline via CancellationCallback.
   */
  const auto& ct = co_await folly::coro::co_current_cancellation_token;
  folly::CancellationCallback cancellationCallback{
      ct, [this]() { signal(TimedBaton::Status::cancelled); }};

  // folly::coro::Baton implements short-circuit eval to avoid suspension if
  // already posted; never yields exception, no need to wrap in co_awaitTry
  co_await baton_;

  status_ = nullptr;

  // possible if posted prior to ::wait(); if so transform status into
  // ::signalled
  if (status == TimedBaton::Status::notReady) {
    status = TimedBaton::Status::signalled;
  }

  XCHECK(status == TimedBaton::Status::cancelled ||
         status == TimedBaton::Status::signalled);

  co_return ct.isCancellationRequested() ? TimedBaton::Status::cancelled
                                         : status;
}

folly::coro::Task<TimedBaton::Status> CancellableBaton::timedWaitImpl(
    folly::EventBase* evb, std::chrono::milliseconds timeout) {
  // > 0ms timeout must have evb
  XCHECK(evb && timeout.count() > 0);
  XCHECK_EQ(status_, nullptr) << "supports only one awaiting coro";

  TimedBaton::Status status{TimedBaton::Status::notReady};
  status_ = &status;

  const auto& ct = co_await folly::coro::co_current_cancellation_token;
  folly::CancellationCallback cancellationCallback{
      ct, [this]() { signal(TimedBaton::Status::cancelled); }};

  evb->timer().scheduleTimeout(&timerCb_, timeout);

  co_await baton_;

  status_ = nullptr;
  timerCb_.cancelTimeout();

  // possible if posted prior to ::wait(); if so transform status into
  // ::signalled
  if (status == TimedBaton::Status::notReady) {
    status = TimedBaton::Status::signalled;
  }

  co_return status;
}

void DetachableCancellableBaton::detach() noexcept {
  getTimerCb().cancelTimeout();
}

} // namespace proxygen::coro::detail
