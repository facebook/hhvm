/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/filters/Status1xxFilter.h"
#include <folly/logging/xlog.h>

namespace proxygen::coro {

folly::coro::Task<HTTPHeaderEvent> Status1xxFilter::readHeaderEvent() {
  auto headerEvent = co_await HTTPSourceFilter::readHeaderEvent();
  while (!headerEvent.isFinal()) {
    // Ignore this event and wait for the next one.
    XLOG(DBG4)
        << fmt::format("A response with status code {} has been filtered out",
                       headerEvent.headers->getStatusCode());
    headerEvent = co_await HTTPSourceFilter::readHeaderEvent();
  }
  auto guard = folly::makeGuard(lifetime(headerEvent));
  co_return headerEvent;
}

} // namespace proxygen::coro
