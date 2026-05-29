/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>

namespace apache::thrift::fast_thrift::transport::test {

/**
 * TestAsyncTransport - In-memory AsyncTransport for testing.
 *
 * This transport stores written data in memory and allows test code to:
 * - Read captured outbound data via getWrittenData()
 * - Inject inbound data via injectReadData()
 *
 * No actual sockets are involved - everything is in-memory.
 */
class TestAsyncTransport : public folly::AsyncTransport {
 public:
  explicit TestAsyncTransport(folly::EventBase* evb) : evb_(evb) {}

  void writeChain(
      WriteCallback* callback,
      std::unique_ptr<folly::IOBuf>&& buf,
      folly::WriteFlags /*flags*/ = folly::WriteFlags::NONE) override {
    writtenData_.append(std::move(buf));
    if (callback) {
      evb_->runInLoop([callback]() { callback->writeSuccess(); });
    }
  }

  void write(
      WriteCallback* callback,
      const void* buf,
      size_t bytes,
      folly::WriteFlags flags = folly::WriteFlags::NONE) override {
    writeChain(callback, folly::IOBuf::copyBuffer(buf, bytes), flags);
  }

  void writev(
      WriteCallback* callback,
      const iovec* vec,
      size_t count,
      folly::WriteFlags flags = folly::WriteFlags::NONE) override {
    auto buf = folly::IOBuf::wrapIov(vec, count);
    writeChain(callback, std::move(buf), flags);
  }

  void setReadCB(ReadCallback* callback) override { readCallback_ = callback; }

  ReadCallback* getReadCallback() const override { return readCallback_; }

  /**
   * Inject data to simulate receiving from the network.
   * The data will be delivered to the ReadCallback.
   */
  void injectReadData(std::unique_ptr<folly::IOBuf> data) {
    if (readCallback_ && data) {
      if (readCallback_->isBufferMovable()) {
        readCallback_->readBufferAvailable(std::move(data));
      } else {
        void* buf;
        size_t len;
        readCallback_->getReadBuffer(&buf, &len);
        auto coalesced = data->coalesce();
        size_t toCopy = std::min(len, coalesced.size());
        std::memcpy(buf, coalesced.data(), toCopy);
        readCallback_->readDataAvailable(toCopy);
      }
    }
  }

  /**
   * Get all data that has been written to this transport.
   */
  std::unique_ptr<folly::IOBuf> getWrittenData() { return writtenData_.move(); }

  /**
   * Clear all written data.
   */
  void clearWrittenData() { writtenData_.move(); }

  void close() override { closed_ = true; }
  void closeNow() override { closed_ = true; }
  void closeWithReset() override { closed_ = true; }
  void shutdownWrite() override {}
  void shutdownWriteNow() override {}

  bool good() const override { return !closed_ && !error_; }
  bool readable() const override { return good(); }
  bool connecting() const override { return false; }
  bool error() const override { return error_; }

  void attachEventBase(folly::EventBase* evb) override { evb_ = evb; }
  void detachEventBase() override { evb_ = nullptr; }
  bool isDetachable() const override { return true; }
  folly::EventBase* getEventBase() const override { return evb_; }

  void setSendTimeout(uint32_t /*ms*/) override {}
  uint32_t getSendTimeout() const override { return 0; }

  void getLocalAddress(folly::SocketAddress* addr) const override {
    *addr = folly::SocketAddress("127.0.0.1", 0);
  }
  void getPeerAddress(folly::SocketAddress* addr) const override {
    *addr = folly::SocketAddress("127.0.0.1", 0);
  }

  size_t getAppBytesWritten() const override {
    return writtenData_.chainLength();
  }
  size_t getRawBytesWritten() const override {
    return writtenData_.chainLength();
  }
  size_t getAppBytesReceived() const override { return 0; }
  size_t getRawBytesReceived() const override { return 0; }
  size_t getAppBytesBuffered() const override { return 0; }
  size_t getRawBytesBuffered() const override { return 0; }

  bool isEorTrackingEnabled() const override { return false; }
  void setEorTracking(bool /*track*/) override {}

  const folly::AsyncTransport* getWrappedTransport() const override {
    return nullptr;
  }
  bool isReplaySafe() const override { return true; }
  void setReplaySafetyCallback(
      folly::AsyncTransport::ReplaySafetyCallback* /*callback*/) override {}
  std::string getSecurityProtocol() const override { return ""; }
  std::string getApplicationProtocol() const noexcept override { return ""; }

 private:
  folly::EventBase* evb_;
  ReadCallback* readCallback_{nullptr};
  folly::IOBufQueue writtenData_{folly::IOBufQueue::cacheChainLength()};
  bool closed_{false};
  bool error_{false};
};

} // namespace apache::thrift::fast_thrift::transport::test
