/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/HTTPSourceFilter.h"

namespace proxygen::coro {

HTTPSourceFilter::~HTTPSourceFilter() {
  if (source_) {
    source_->stopReading();
  }
}

folly::coro::Task<HTTPHeaderEvent> HTTPSourceFilter::readHeaderEventImpl(
    bool deleteOnDone) {
  XCHECK(source_);
  auto event = co_await co_awaitTry(source_->readHeaderEvent());
  if (event.hasException() || event->eom) {
    source_ = nullptr;
    if (heapAllocated_ && deleteOnDone) {
      delete this;
    }
  }
  co_return event;
}

folly::coro::Task<HTTPBodyEvent> HTTPSourceFilter::readBodyEventImpl(
    uint32_t max, bool deleteOnDone) {
  XCHECK(source_);
  auto event = co_await co_awaitTry(source_->readBodyEvent(max));
  if (event.hasException() || event->eom) {
    source_ = nullptr;
    if (heapAllocated_ && deleteOnDone) {
      delete this;
    }
  }
  co_return event;
}

void HTTPSourceFilter::stopReading(folly::Optional<const HTTPErrorCode> error) {
  XCHECK(source_);
  source_->stopReading(error);
  source_ = nullptr;
  if (heapAllocated_) {
    delete this;
  }
}

} // namespace proxygen::coro
