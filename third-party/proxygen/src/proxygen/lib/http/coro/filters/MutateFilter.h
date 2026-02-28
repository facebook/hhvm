/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/coro/HTTPSourceFilter.h>

namespace proxygen::coro {

/**
 * This class provides a simple abstraction for filters that just want to
 * execute simple transformations on events (HTTPHeaderEvent and HTTPBodyEvent).
 * This filter is essentially a passthru filter in two cases:
 *  1. on error since there is no event to mutate.
 *  2. if nullptr is passed for both functions.
 */
class MutateFilter : public HTTPSourceFilter {
 public:
  using HeaderMutateFn = std::function<void(HTTPHeaderEvent&)>;
  using BodyMutateFn = std::function<void(HTTPBodyEvent&)>;

  explicit MutateFilter(HTTPSource* source,
                        HeaderMutateFn headerHook = nullptr,
                        BodyMutateFn bodyHook = nullptr);
  ~MutateFilter() override = default;

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override;
  folly::coro::Task<HTTPBodyEvent> readBodyEvent(
      uint32_t max = std::numeric_limits<uint32_t>::max()) override;

 private:
  HeaderMutateFn headerHook_;
  BodyMutateFn bodyHook_;
};

} // namespace proxygen::coro
