/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPStreamSourceSink.h"

namespace proxygen::coro {

class HTTPStreamSourceSinkFactory {
 public:
  HTTPStreamSourceSinkFactory() = default;
  virtual ~HTTPStreamSourceSinkFactory() = default;

  virtual std::unique_ptr<coro::HTTPStreamSourceUpstreamSink>
  newHTTPStreamSourceSink(folly::EventBase* evb,
                          HTTPSessionContextPtr sessionCtx,
                          HTTPTransactionHandler* handler) {
    return std::make_unique<coro::HTTPStreamSourceUpstreamSink>(
        evb, std::move(sessionCtx), handler);
  }
};

} // namespace proxygen::coro
