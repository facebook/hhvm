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
 * execute arbitrary transformations on events (HTTPHeaderEvent and
 * HTTPBodyEvent). Acts as a passthru filter if nullptr is passed in for both
 * hooks.
 */
class TransformFilter : public HTTPSourceFilter {
 public:
  using HeaderTransformFn =
      std::function<folly::Try<HTTPHeaderEvent>(folly::Try<HTTPHeaderEvent>&&)>;
  using BodyTransformFn =
      std::function<folly::Try<HTTPBodyEvent>(folly::Try<HTTPBodyEvent>&&)>;

  explicit TransformFilter(HTTPSource* source,
                           HeaderTransformFn headerHook = nullptr,
                           BodyTransformFn bodyHook = nullptr);
  ~TransformFilter() override = default;

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override;
  folly::coro::Task<HTTPBodyEvent> readBodyEvent(
      uint32_t max = std::numeric_limits<uint32_t>::max()) override;

 private:
  HeaderTransformFn headerHook_;
  BodyTransformFn bodyHook_;
};

} // namespace proxygen::coro
