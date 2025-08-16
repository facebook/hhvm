/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <cstddef>

#include <proxygen/lib/http/ProxygenErrorEnum.h>

namespace proxygen {

class HTTPMessage;

class HttpServerStatsIf {
 public:
  virtual ~HttpServerStatsIf() = default;
  virtual void recordRequest(const HTTPMessage& msg) = 0;
  virtual void recordResponse(const HTTPMessage& msg) = 0;
  virtual void recordAbort() = 0;
  virtual void recordRequestComplete(std::chrono::milliseconds latency,
                                     ProxygenError err,
                                     size_t requestBodyBytes,
                                     size_t responseBodyBytes) = 0;
};

} // namespace proxygen
