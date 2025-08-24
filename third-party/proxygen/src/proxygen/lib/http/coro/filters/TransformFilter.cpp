/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/filters/TransformFilter.h"

namespace proxygen::coro {

TransformFilter::TransformFilter(HTTPSource* source,
                                 TransformFilter::HeaderTransformFn headerHook,
                                 TransformFilter::BodyTransformFn bodyHook)
    : HTTPSourceFilter(source),
      headerHook_(std::move(headerHook)),
      bodyHook_(std::move(bodyHook)) {
}

using HTTPHeaderEventTask = folly::coro::Task<HTTPHeaderEvent>;
HTTPHeaderEventTask TransformFilter::readHeaderEvent() {
  auto headerEvent =
      co_await co_awaitTry(readHeaderEventImpl(/*deleteOnDone=*/false));
  auto guard = folly::makeGuard(lifetime(headerEvent));
  if (headerHook_) {
    co_return headerHook_(std::move(headerEvent));
  }
  co_return headerEvent;
}

using HTTPBodyEventTask = folly::coro::Task<HTTPBodyEvent>;
HTTPBodyEventTask TransformFilter::readBodyEvent(uint32_t max) {
  auto bodyEvent =
      co_await co_awaitTry(readBodyEventImpl(max, /*deleteOnDone=*/false));
  auto guard = folly::makeGuard(lifetime(bodyEvent));
  if (bodyHook_) {
    co_return bodyHook_(std::move(bodyEvent));
  }
  co_return bodyEvent;
}

} // namespace proxygen::coro
