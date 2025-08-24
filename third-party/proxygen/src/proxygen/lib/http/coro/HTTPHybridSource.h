/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPSourceFilter.h"
#include "proxygen/lib/http/coro/HTTPSourceHolder.h"

namespace proxygen::coro {

/**
 * A Hybrid source that uses a fixed HTTP message for the headers but another
 * source for body events.
 */
class HTTPHybridSource : public HTTPSourceFilter {
 public:
  HTTPHybridSource(std::unique_ptr<HTTPMessage> headers, HTTPSource* source)
      : headerEvent_(std::move(headers), !source) {
    if (source) {
      setSource(source);
    }
  }

  HTTPHybridSource(std::unique_ptr<HTTPMessage> headers,
                   HTTPSourceHolder source)
      : headerEvent_(std::move(headers), !source) {
    if (source) {
      setSource(source.release());
    }
  }

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override {
    if (headerEventAvailable_) {
      headerEventAvailable_ = false;
      auto guard = folly::makeGuard(lifetime(headerEvent_));
      co_return std::move(headerEvent_);
    } else {
      co_return co_await readHeaderEventImpl();
    }
  }

  void stopReading(folly::Optional<const HTTPErrorCode> error) override {
    if (bool((HTTPSourceFilter&)*this)) {
      HTTPSourceFilter::stopReading(error);
    } else if (heapAllocated_) {
      delete this;
    }
  }

 private:
  bool headerEventAvailable_{true};
  HTTPHeaderEvent headerEvent_;
};

} // namespace proxygen::coro
