/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPSourceFilter.h"

namespace proxygen::coro {

/**
 * This class must only be used by a single thread; it enables consuming from an
 * HTTPSource that a foreign/remote evb is producing events for. In other words,
 * it's effecitvely a wrapper to provide SPSC (SingleProducerSingleConsumer)
 * semantics.
 */
class ExecutorSourceFilter : public HTTPSourceFilter {
 public:
  static std::unique_ptr<HTTPSourceFilter> make(folly::EventBase* evb) {
    auto* executorSource = new ExecutorSourceFilter(*CHECK_NOTNULL(evb));
    executorSource->setHeapAllocated();
    return std::unique_ptr<HTTPSourceFilter>(executorSource);
  }

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override;
  folly::coro::Task<HTTPBodyEvent> readBodyEvent(uint32_t max) override;
  void stopReading(
      folly::Optional<const HTTPErrorCode> error = folly::none) override;

  // const access, ok to read
  folly::Optional<uint64_t> getStreamID() const override {
    return HTTPSourceFilter::getStreamID();
  }
  void setReadTimeout(std::chrono::milliseconds timeout) override {
    evb_.runImmediatelyOrRunInEventBaseThread(
        [this, timeout]() { HTTPSourceFilter::setReadTimeout(timeout); });
  }

 private:
  explicit ExecutorSourceFilter(folly::EventBase& evb) : evb_(evb) {
    setHeapAllocated();
  }
  folly::EventBase& evb_;
};

} // namespace proxygen::coro
