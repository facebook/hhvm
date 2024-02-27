/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/GMock.h>
#include <proxygen/lib/http/session/HTTPSessionStats.h>

namespace proxygen {

class DummyHTTPSessionStats : public HTTPSessionStats {
 public:
  void recordTransactionOpened() noexcept override{};
  void recordTransactionClosed() noexcept override{};
  void recordTransactionsServed(uint64_t) noexcept override{};
  void recordSessionReused() noexcept override{};
  // virtual void recordSessionIdleTime(std::chrono::seconds) noexcept {};
  void recordTransactionStalled() noexcept override{};
  void recordSessionStalled() noexcept override {
  }
  void recordEgressContentLengthMismatches() noexcept override {
  }
  void recordSessionPeriodicPingProbeTimeout() noexcept override {
  }

  void recordPresendIOSplit() noexcept override {
  }
  void recordPresendExceedLimit() noexcept override {
  }
  void recordTTLBAExceedLimit() noexcept override {
  }
  void recordTTLBANotFound() noexcept override {
  }
  void recordTTLBAReceived() noexcept override {
  }
  void recordTTLBATimeout() noexcept override {
  }
  void recordTTLBATracked() noexcept override {
  }
  void recordTTBTXExceedLimit() noexcept override {
  }
  void recordTTBTXReceived() noexcept override {
  }
  void recordTTBTXTimeout() noexcept override {
  }
  void recordTTBTXNotFound() noexcept override {
  }
  void recordTTBTXTracked() noexcept override {
  }
};

class MockHTTPSessionStats : public DummyHTTPSessionStats {
 public:
  MockHTTPSessionStats() {
  }
  void recordTransactionOpened() noexcept override {
    _recordTransactionOpened();
  }
  MOCK_METHOD(void, _recordTransactionOpened, ());
  void recordTransactionClosed() noexcept override {
    _recordTransactionClosed();
  }
  MOCK_METHOD(void, _recordTransactionClosed, ());
  void recordTransactionsServed(uint64_t num) noexcept override {
    _recordTransactionsServed(num);
  }
  MOCK_METHOD(void, _recordTransactionsServed, (uint64_t));
  void recordSessionReused() noexcept override {
    _recordSessionReused();
  }
  MOCK_METHOD(void, _recordSessionReused, ());
  void recordSessionIdleTime(std::chrono::seconds param) noexcept override {
    _recordSessionIdleTime(param);
  }
  MOCK_METHOD(void, _recordSessionIdleTime, (std::chrono::seconds));
  void recordTransactionStalled() noexcept override {
    _recordTransactionStalled();
  }
  MOCK_METHOD(void, _recordTransactionStalled, ());
  void recordSessionStalled() noexcept override {
    _recordSessionStalled();
  }
  MOCK_METHOD(void, _recordSessionStalled, ());
  void recordEgressContentLengthMismatches() noexcept override {
    _recordEgressContentLengthMismatches();
  }
  MOCK_METHOD(void, _recordEgressContentLengthMismatches, ());
  void recordPendingBufferedReadBytes(int64_t num) noexcept override {
    _recordPendingBufferedReadBytes(num);
  }
  MOCK_METHOD(void, _recordPendingBufferedReadBytes, (int64_t));
  void recordPendingBufferedWriteBytes(int64_t num) noexcept override {
    _recordPendingBufferedWriteBytes(num);
  }
  MOCK_METHOD(void, _recordPendingBufferedWriteBytes, (int64_t));
  void recordSessionPeriodicPingProbeTimeout() noexcept override {
    _recordSessionPeriodicPingProbeTimeout();
  }
  MOCK_METHOD(void, _recordSessionPeriodicPingProbeTimeout, ());
};

class FakeSessionStats : public DummyHTTPSessionStats {
 public:
  ~FakeSessionStats() override {
    EXPECT_EQ(pendingBufferedReadBytes_, 0);
    EXPECT_EQ(pendingBufferedWriteBytes_, 0);
  }

  void recordPendingBufferedReadBytes(int64_t bufferedBytes) noexcept override {
    pendingBufferedReadBytes_ += bufferedBytes;
  }
  void recordPendingBufferedWriteBytes(
      int64_t bufferedBytes) noexcept override {
    pendingBufferedWriteBytes_ += bufferedBytes;
  }

 protected:
  int64_t pendingBufferedReadBytes_{0};
  int64_t pendingBufferedWriteBytes_{0};
};

} // namespace proxygen
