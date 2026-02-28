/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <gmock/gmock.h>

#include "proxygen/lib/http/coro/client/HTTPClient.h"

namespace proxygen::coro {

class MockHTTPClient {
 public:
  MOCK_METHOD5(get,
               folly::coro::Task<HTTPClient::Response>(
                   folly::EventBase* evb,
                   std::string url,
                   std::chrono::milliseconds timeout,
                   bool useQuic,
                   HTTPClient::RequestHeaderMap requestHeaders));
};

} // namespace proxygen::coro
