/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/webtransport/FlowController.h>

#include <folly/io/IOBufQueue.h>

namespace proxygen::coro {

/**
 * This is needed due to the asynchronous nature of CoroWtSession in
 * proxygen::coro – when writing to an egress handle, data is flushed to the
 * transport at a later time (e.g. next evb loop). This is an extremely thin
 * wrapper around proxygen/lib/http/webtransport/FlowController.h; simple keeps
 * track of an additional bufferedOffset_ (invariant bufferedOffset_ >=
 * currentOffset_). We apply egress backpressure (e.g. return Blocked) if the
 * the application has buffered more than 64KiB bytes.
 */
struct BufferedFlowController {
  BufferedFlowController(uint64_t initMax = 0) : window_(initMax) {
  }

  // advances the bufferedOffset_ by len bytes – returns true if buffering len
  // bytes has exceeded send window
  bool buffer(uint64_t len) {
    bufferedOffset_ += len; // TODO(@damlaj) overflow
    return isBlocked();
  }

  // adv currentOffset_ by len bytes (must be <= bufferedOffset_ after
  // increment)
  void commit(uint64_t len) {
    window_.reserve(len);
    CHECK_LE(getCurrentOffset(), getBufferedOffset());
  }

  bool grant(uint64_t offset) {
    return window_.grant(offset);
  }

  [[nodiscard]] bool isBlocked() const {
    return getBufferAvailable() == 0;
  }

  uint64_t getBufferedOffset() const {
    return bufferedOffset_;
  }

  uint64_t getCurrentOffset() const {
    return window_.getCurrentOffset();
  }

  uint64_t getMaxOffset() const {
    return window_.getMaxOffset();
  }

  uint64_t getBufferAvailable() const {
    const auto bufferedBytes = bufferedOffset_ - getCurrentOffset();
    return bufferedBytes >= kMaxEgressBuf ? 0 : (kMaxEgressBuf - bufferedBytes);
  }

  uint64_t getAvailable() const {
    return window_.getAvailable();
  }

 private:
  static constexpr auto kMaxEgressBuf = std::numeric_limits<uint16_t>::max();
  FlowController window_;
  uint64_t bufferedOffset_{0};
};

class WtEgressContainer {
 public:
  WtEgressContainer(uint64_t initMax = 0) : window_(initMax) {
  }

  const BufferedFlowController& window() {
    return window_;
  }

  /**
   * enqueues data into the container's buffer – returns true if the egress is
   * now flow control blocked
   */
  bool enqueue(std::unique_ptr<folly::IOBuf> data, bool fin) noexcept;

  struct DequeueResult {
    std::unique_ptr<folly::IOBuf> data;
    bool fin{false};
  };
  /**
   * dequeues data from the container's buffer, returning min(atMost,
   * window_available) bytes
   */
  DequeueResult dequeue(uint64_t atMost) noexcept;

  bool grant(uint64_t offset) noexcept {
    return window_.grant(offset);
  }

 private:
  folly::IOBufQueue data_{folly::IOBufQueue::cacheChainLength()};
  bool fin_{false};
  BufferedFlowController window_;
};

} // namespace proxygen::coro
