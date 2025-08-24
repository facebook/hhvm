/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/coro/HTTPSourceFilter.h>
#include <proxygen/lib/utils/Time.h>

namespace proxygen::coro {

/** A rate limit HTTPSourceFilter
 *
 * This filter limits body read to a preset rate limit. Header isn't limited.
 * Note this is a rate limiter, not a pacer.
 */
class RateLimitFilter : public HTTPSourceFilter {
 public:
  explicit RateLimitFilter(HTTPSource* source);
  explicit RateLimitFilter(HTTPSource* source,
                           std::chrono::seconds maxDelay,
                           uint32_t chunkBytes);

  ~RateLimitFilter() override = default;

  /**
   * Read body byte from underlying HTTPSource, can potentially sleep if it hits
   * the preset limit.
   */
  folly::coro::Task<HTTPBodyEvent> readBodyEvent(uint32_t max) override;

  void setLimit(uint64_t bitsPerSecond) noexcept;

 private:
  folly::coro::Task<HTTPBodyEvent> readAndUpdateBytesCount(
      uint32_t bytes) noexcept;

 private:
  std::chrono::seconds maxDelay_;
  uint32_t chunkBytes_;
  uint64_t bytesPerMs_{0};
  TimePoint startTime_;
  uint64_t readBytesCount_{0};
};

} // namespace proxygen::coro
