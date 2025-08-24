/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPSourceFilter.h"
#include <proxygen/lib/utils/CompressionFilterUtils.h>

namespace {
using FilterParams = proxygen::CompressionFilterUtils::FilterParams;
}

namespace proxygen::coro {
class CompressionFilter : public HTTPSourceFilter {
 public:
  CompressionFilter(HTTPSource* source,
                    std::shared_ptr<folly::Optional<FilterParams>> params)
      : HTTPSourceFilter(source), params_(std::move(params)) {
    CHECK(params_);
  }

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override;

  folly::coro::Task<HTTPBodyEvent> readBodyEvent(
      uint32_t max = std::numeric_limits<uint32_t>::max()) override;

 private:
  std::shared_ptr<folly::Optional<FilterParams>> params_;
  std::unique_ptr<proxygen::StreamCompressor> compressor_{nullptr};
  bool skipCompression_{false};
  folly::Optional<proxygen::coro::HTTPBodyEvent> pendingBodyEvent_{folly::none};
};
} // namespace proxygen::coro
