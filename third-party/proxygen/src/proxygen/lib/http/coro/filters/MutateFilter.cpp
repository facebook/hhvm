/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/filters/MutateFilter.h"

namespace proxygen::coro {

MutateFilter::MutateFilter(HTTPSource* source,
                           MutateFilter::HeaderMutateFn headerHook,
                           MutateFilter::BodyMutateFn bodyHook)
    : HTTPSourceFilter(source),
      headerHook_(std::move(headerHook)),
      bodyHook_(std::move(bodyHook)) {
}

using HTTPHeaderEventTask = folly::coro::Task<HTTPHeaderEvent>;
HTTPHeaderEventTask MutateFilter::readHeaderEvent() {
  auto headerEvent =
      co_await co_awaitTry(readHeaderEventImpl(/*deleteOnDone=*/false));
  auto guard = folly::makeGuard(lifetime(headerEvent));
  if (headerEvent.hasException()) {
    co_yield folly::coro::co_error(std::move(headerEvent.exception()));
  }
  if (headerHook_) {
    headerHook_(headerEvent.value());
  }
  co_return headerEvent;
}

using HTTPBodyEventTask = folly::coro::Task<HTTPBodyEvent>;
HTTPBodyEventTask MutateFilter::readBodyEvent(uint32_t max) {
  auto bodyEvent =
      co_await co_awaitTry(readBodyEventImpl(max, /*deleteOnDone=*/false));
  auto guard = folly::makeGuard(lifetime(bodyEvent));
  if (bodyEvent.hasException()) {
    co_yield folly::coro::co_error(std::move(bodyEvent.exception()));
  }
  if (bodyHook_) {
    bodyHook_(bodyEvent.value());
  }
  co_return bodyEvent;
}

} // namespace proxygen::coro
