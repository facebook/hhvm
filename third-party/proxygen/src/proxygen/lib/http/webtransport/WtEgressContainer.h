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

namespace proxygen::detail {

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
  explicit BufferedFlowController(uint64_t initMax = 0) : window_(initMax) {
  }

  // advances the bufferedOffset_ by len bytes – returns true if buffering len
  // bytes has exceeded send window
  enum FcRes : uint8_t { Unblocked = 0, Blocked = 1 };
  [[nodiscard]] FcRes buffer(uint64_t len) {
    bufferedOffset_ += len; // TODO(@damlaj) overflow
    return isBlocked() ? FcRes::Blocked : FcRes::Unblocked;
  }

  // adv currentOffset_ by len bytes (must be <= bufferedOffset_ after
  // increment)
  void commit(uint64_t len) {
    window_.reserve(len);
    CHECK_LE(getCurrentOffset(), getBufferedOffset());
  }

  [[nodiscard]] bool grant(uint64_t offset) {
    return window_.grant(offset);
  }

  [[nodiscard]] bool isBlocked() const {
    return getBufferAvailable() == 0;
  }

  [[nodiscard]] uint64_t getBufferedOffset() const {
    return bufferedOffset_;
  }

  [[nodiscard]] uint64_t getCurrentOffset() const {
    return window_.getCurrentOffset();
  }

  [[nodiscard]] uint64_t getMaxOffset() const {
    return window_.getMaxOffset();
  }

  [[nodiscard]] uint64_t getBufferAvailable() const {
    const auto bufferedBytes = bufferedOffset_ - getCurrentOffset();
    return bufferedBytes >= kMaxEgressBuf ? 0 : (kMaxEgressBuf - bufferedBytes);
  }

  [[nodiscard]] uint64_t getAvailable() const {
    return window_.getAvailable();
  }

 private:
  static constexpr auto kMaxEgressBuf = std::numeric_limits<uint16_t>::max();
  FlowController window_;
  uint64_t bufferedOffset_{0};
};

class WtBufferedStreamData {
 public:
  explicit WtBufferedStreamData(uint64_t initMax = 0) : window_(initMax) {
  }

  const BufferedFlowController& window() {
    return window_;
  }

  /**
   * enqueues data into the container's buffer – returns FcRes::Blocked if the
   * egress is now flow control blocked, FcRes::Unblocked otherwise
   */
  using FcRes = BufferedFlowController::FcRes;
  FcRes enqueue(std::unique_ptr<folly::IOBuf> data, bool fin) noexcept;

  struct DequeueResult {
    std::unique_ptr<folly::IOBuf> data;
    bool fin{false};
  };

  /**
   * Dequeues data from the container's buffer, returning min(atMost,
   * window_available, bytes_enqueued) bytes.
   */
  DequeueResult dequeue(uint64_t atMost) noexcept;

  bool grant(uint64_t offset) noexcept {
    return window_.grant(offset);
  }

  bool hasData() const {
    return !data_.empty() || fin_;
  }

  // we can send data if there is either data & available stream fc, or only fin
  // pending
  bool canSendData() const {
    return (hasData() && window_.getAvailable()) || (data_.empty() && fin_);
  }

  // returns true if there's only a pending fin
  bool onlyFinPending() const {
    return data_.empty() && fin_;
  }

 private:
  folly::IOBufQueue data_{folly::IOBufQueue::cacheChainLength()};
  bool fin_{false};
  BufferedFlowController window_;
};

} // namespace proxygen::detail
