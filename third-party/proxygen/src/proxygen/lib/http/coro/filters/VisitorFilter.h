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
 * This class provides a simple abstraction for simple filters that just want to
 * observe events (HTTPHeaderEvent, HTTPBodyEvent, or Exception yielded) without
 * mutating via const ref. All events simply passthru this filter. Source can be
 * set in the constructor, or later via setSource.
 */
class VisitorFilter : public HTTPSourceFilter {
 public:
  using HeaderHookFn = std::function<void(const folly::Try<HTTPHeaderEvent>&)>;
  using BodyHookFn = std::function<void(const folly::Try<HTTPBodyEvent>&)>;

  explicit VisitorFilter(HTTPSource* source,
                         HeaderHookFn headerHook = nullptr,
                         BodyHookFn bodyHook = nullptr);
  ~VisitorFilter() override = default;

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override;
  folly::coro::Task<HTTPBodyEvent> readBodyEvent(
      uint32_t max = std::numeric_limits<uint32_t>::max()) override;

 private:
  HeaderHookFn headerHook_;
  BodyHookFn bodyHook_;
};

} // namespace proxygen::coro
