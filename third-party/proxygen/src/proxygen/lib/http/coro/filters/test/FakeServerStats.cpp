/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/filters/test/FakeServerStats.h"

#include <proxygen/lib/http/HTTPMessage.h>

namespace proxygen {

void FakeHTTPServerStats::recordRequest(const HTTPMessage& msg) {
  reqs++;
}

void FakeHTTPServerStats::recordResponse(const HTTPMessage& msg) {
  const auto status = msg.getStatusCode();
  responseCodes[status / 100]++;
}

void FakeHTTPServerStats::recordRequestComplete(
    std::chrono::milliseconds latency,
    ProxygenError err,
    size_t requestBodyBytes,
    size_t responseBodyBytes) {
  reqBodyBytes += requestBodyBytes;
  resBodyBytes += responseBodyBytes;
  errors += err != ProxygenError::kErrorNone;
  errorTypes[static_cast<uint8_t>(err)]++;
}

void FakeHTTPServerStats::recordAbort() {
  aborts++;
}

} // namespace proxygen
