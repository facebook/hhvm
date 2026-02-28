/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/webtransport/FlowController.h>
// TODO(@joannajo): Remove WebTransport.h dependency. Consider extracting
// ByteEventCallback to a separate header.
#include <proxygen/lib/http/webtransport/WebTransport.h>

#include <folly/io/IOBufQueue.h>
#include <list>

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
   *
   * Each write is tracked separately with its callback. Consecutive writes
   * without callbacks are merged.
   */
  using FcRes = BufferedFlowController::FcRes;
  FcRes enqueue(std::unique_ptr<folly::IOBuf> data,
                bool fin,
                WebTransport::ByteEventCallback* callback = nullptr) noexcept;

  struct DequeueResult {
    std::unique_ptr<folly::IOBuf> data;
    bool fin{false};
    WebTransport::ByteEventCallback* deliveryCallback{nullptr};
  };

  /**
   * Dequeues data from the container's buffer, returning min(atMost,
   * window_available, bytes_enqueued) bytes.
   */
  DequeueResult dequeue(uint64_t atMost) noexcept;

  // returns true if there's only a pending fin
  [[nodiscard]] bool onlyFinPending() const;

  bool grant(uint64_t offset) noexcept {
    return window_.grant(offset);
  }

  [[nodiscard]] bool hasData() const {
    return !pendingWrites_.empty();
  }

  // we can send data if there is either data & available stream fc, or only fin
  // pending
  [[nodiscard]] bool canSendData() const {
    return (hasData() && window_.getAvailable()) || onlyFinPending();
  }

  void clear(uint64_t id) noexcept;

 private:
  /**
   * Stores a single buffered write with its associated callback.
   * Consecutive writes without callbacks can be merged into the previous
   * PendingWrite (see enqueue implementation).
   */
  struct PendingWrite {
    folly::IOBufQueue buf;
    proxygen::WebTransport::ByteEventCallback* deliveryCallback;
    uint64_t offset;
    bool fin;

    PendingWrite(std::unique_ptr<folly::IOBuf> data,
                 proxygen::WebTransport::ByteEventCallback* callback,
                 uint64_t offset,
                 bool finFlag) noexcept;
  };

  BufferedFlowController window_;
  std::list<PendingWrite> pendingWrites_;
};

} // namespace proxygen::detail
