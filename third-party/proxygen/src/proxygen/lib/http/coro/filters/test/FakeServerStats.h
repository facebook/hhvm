/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/stats/HttpServerStats.h"
#include <array>
#include <proxygen/lib/http/ProxygenErrorEnum.h>

namespace proxygen {

class FakeHTTPServerStats : public HttpServerStatsIf {
 public:
  explicit FakeHTTPServerStats() = default;
  ~FakeHTTPServerStats() override = default;

  void recordRequest(const HTTPMessage& msg) override;
  void recordResponse(const HTTPMessage& msg) override;
  void recordAbort() override;

  void recordRequestComplete(std::chrono::milliseconds latency,
                             ProxygenError err,
                             size_t requestBodyBytes,
                             size_t responseBodyBytes) override;

  uint64_t reqs{0};
  uint64_t reqBodyBytes{0};
  uint64_t resBodyBytes{0};
  uint64_t aborts{0};
  uint64_t errors{0};

  // 1xx -> 5xx (ignore index 0)
  std::array<uint64_t, 6> responseCodes{};
  // errors categorized by type
  std::array<uint64_t, ProxygenError::kErrorMax> errorTypes{};
};

} // namespace proxygen
