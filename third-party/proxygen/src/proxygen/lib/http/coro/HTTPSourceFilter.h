/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPSource.h"
#include <folly/logging/xlog.h>

namespace proxygen::coro {

/**
 * A source that reads from another source.
 */
class HTTPSourceFilter : public HTTPSource {
 public:
  HTTPSourceFilter() = default;
  ~HTTPSourceFilter() override;

  /* implicit */ HTTPSourceFilter(HTTPSource* source) : source_(source) {
  }

  // Delete implicit move/copy constructors and assignment operator overloads.
  HTTPSourceFilter(HTTPSourceFilter&&) = delete;
  HTTPSourceFilter& operator=(HTTPSourceFilter&&) = delete;
  HTTPSourceFilter(const HTTPSourceFilter&) = delete;
  HTTPSourceFilter& operator=(const HTTPSourceFilter&) = delete;

  operator bool() const {
    return readable();
  }

  [[nodiscard]] bool readable() const {
    return source_;
  }

  // Don't call this while awaiting!
  HTTPSource* release() {
    return std::exchange(source_, nullptr);
  }

  void setSource(HTTPSource* source) {
    if (auto* prevSource = std::exchange(source_, source)) {
      prevSource->stopReading();
    }
  }

  // HTTPSource overrides
  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override {
    return readHeaderEventImpl(/*deleteOnDone=*/true);
  }

  folly::coro::Task<HTTPBodyEvent> readBodyEvent(
      uint32_t max = std::numeric_limits<uint32_t>::max()) override {
    return readBodyEventImpl(max, /*deleteOnDone=*/true);
  }

  void stopReading(
      folly::Optional<const HTTPErrorCode> error = folly::none) override;

  folly::Optional<uint64_t> getStreamID() const override {
    XCHECK(source_);
    return source_->getStreamID();
  }

  void setReadTimeout(std::chrono::milliseconds timeout) override {
    XCHECK(source_);
    source_->setReadTimeout(timeout);
  }

 protected:
  folly::coro::Task<HTTPHeaderEvent> readHeaderEventImpl(
      bool deleteOnDone = false);

  folly::coro::Task<HTTPBodyEvent> readBodyEventImpl(
      uint32_t max = std::numeric_limits<uint32_t>::max(),
      bool deleteOnDone = false);

 private:
  HTTPSource* source_{nullptr};
};
} // namespace proxygen::coro
