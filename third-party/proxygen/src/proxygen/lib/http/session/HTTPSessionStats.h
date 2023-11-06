/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <inttypes.h>
#include <proxygen/lib/http/session/TTLBAStats.h>

namespace proxygen {

// This may be retired with a byte events refactor
class HTTPSessionStats : public TTLBAStats {
 public:
  ~HTTPSessionStats() noexcept override {
  }

  virtual void recordTransactionOpened() noexcept = 0;
  virtual void recordTransactionClosed() noexcept = 0;
  virtual void recordTransactionsServed(uint64_t) noexcept = 0;
  virtual void recordSessionReused() noexcept = 0;
  virtual void recordSessionIdleTime(std::chrono::seconds) noexcept {
  }
  virtual void recordTransactionStalled() noexcept = 0;
  virtual void recordSessionStalled() noexcept = 0;
  virtual void recordPendingBufferedReadBytes(int64_t) noexcept = 0;
  virtual void recordPendingBufferedWriteBytes(int64_t) noexcept {
  }
  virtual void recordEgressContentLengthMismatches() noexcept = 0;
  virtual void recordSessionPeriodicPingProbeTimeout() noexcept = 0;

  virtual void recordControlMsgsInInterval(int64_t) noexcept {
  }
  virtual void recordControlMsgRateLimited() noexcept {
  }
  virtual void recordHeadersInInterval(int64_t) noexcept {
  }
  virtual void recordHeadersRateLimited() noexcept {
  }
};

} // namespace proxygen
