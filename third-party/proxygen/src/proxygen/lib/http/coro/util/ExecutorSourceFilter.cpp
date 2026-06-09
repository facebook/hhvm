/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/util/ExecutorSourceFilter.h"

using folly::coro::co_nothrow;

namespace proxygen::coro {

folly::coro::Task<HTTPHeaderEvent> ExecutorSourceFilter::readHeaderEvent() {
  co_return co_await co_nothrow(
      co_withExecutor(&evb_, readHeaderEventImpl(/*deleteOnDone=*/true)));
}

folly::coro::Task<HTTPBodyEvent> ExecutorSourceFilter::readBodyEvent(
    uint32_t max) {
  auto ev = co_await co_nothrow(
      co_withExecutor(&evb_, readBodyEventImpl(max, /*deleteOnDone=*/true)));
  // SUSPEND returns a task that must run in the producer's evb
  while (ev.eventType == HTTPBodyEvent::SUSPEND) {
    co_await co_awaitTry(co_withExecutor(&evb_, std::move(ev.event.resume)));
    ev = co_await co_nothrow(
        co_withExecutor(&evb_, readBodyEventImpl(max, /*deleteOnDone=*/true)));
  }
  co_return ev;
}

void ExecutorSourceFilter::stopReading(
    folly::Optional<const HTTPErrorCode> error) noexcept {
  evb_.runImmediatelyOrRunInEventBaseThread(
      [this, error]() { HTTPSourceFilter::stopReading(error); });
}

} // namespace proxygen::coro
