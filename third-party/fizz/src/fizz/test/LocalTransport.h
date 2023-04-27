/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/IOBufQueue.h>
#include <folly/io/async/AsyncTransport.h>

namespace fizz {
namespace test {

class LocalTransport : public folly::AsyncTransportWrapper {
 public:
  using UniquePtr = std::unique_ptr<LocalTransport, Destructor>;

  /**
   * Deliver the data one byte at a time. Callback will be invoked before
   * delivering each byte.
   *
   * Note that this does not affect how data within encrypted records is
   * delivered after decryption.
   */
  void setTrickle(bool trickle, folly::Function<void()> callback = nullptr) {
    trickle_ = trickle;
    trickleCallback_ = std::move(callback);
  }

  void setPeer(LocalTransport* peer) {
    peer_ = peer;
  }

  void receiveData(std::unique_ptr<folly::IOBuf> buf) {
    received_ += buf->computeChainDataLength();
    readBuf_.append(std::move(buf));
    deliverData();
  }

  ReadCallback* getReadCallback() const override {
    return callback_;
  }

  void setReadCB(ReadCallback* callback) override {
    callback_ = callback;
    if (callback_) {
      CHECK(callback->isBufferMovable());
      deliverData();
    }
  }

  void write(
      WriteCallback* /*callback*/,
      const void* /*buf*/,
      size_t /*bytes*/,
      folly::WriteFlags /*flags*/ = folly::WriteFlags::NONE) override {
    LOG(FATAL) << "only writeChain() supported";
  }

  void writeChain(
      WriteCallback* callback,
      std::unique_ptr<folly::IOBuf>&& buf,
      folly::WriteFlags /*flags*/ = folly::WriteFlags::NONE) override {
    written_ += buf->computeChainDataLength();
    peer_->receiveData(std::move(buf));
    if (callback) {
      callback->writeSuccess();
    }
  }

  void writev(
      WriteCallback* /*callback*/,
      const iovec* /*vec*/,
      size_t /*bytes*/,
      folly::WriteFlags /*flags*/ = folly::WriteFlags::NONE) override {
    LOG(FATAL) << "only writeChain() supported";
  }

  folly::EventBase* getEventBase() const override {
    return evb_;
  }

  void attachEventBase(folly::EventBase* eventBase) override {
    evb_ = eventBase;
  }

  void close() override {
    peer_ = nullptr;
  }

  void closeNow() override {
    peer_ = nullptr;
  }

  void closeWithReset() override {
    peer_ = nullptr;
  }

  bool connecting() const override {
    return false;
  }

  void detachEventBase() override {
    evb_ = nullptr;
  }

  bool error() const override {
    return false;
  }

  size_t getAppBytesReceived() const override {
    return received_;
  }

  size_t getAppBytesWritten() const override {
    return written_;
  }

  void getLocalAddress(folly::SocketAddress*) const override {}

  void getPeerAddress(folly::SocketAddress*) const override {}

  size_t getRawBytesReceived() const override {
    return received_;
  }

  size_t getRawBytesWritten() const override {
    return written_;
  }

  uint32_t getSendTimeout() const override {
    return 0;
  }

  bool good() const override {
    return true;
  }

  bool isDetachable() const override {
    return true;
  }

  bool isEorTrackingEnabled() const override {
    return false;
  }

  bool readable() const override {
    return true;
  }

  void setEorTracking(bool) override {}

  void setSendTimeout(uint32_t) override {}

  void shutdownWrite() override {}

  void shutdownWriteNow() override {}

 private:
  void deliverData() {
    while (callback_ && !readBuf_.empty()) {
      if (!trickle_) {
        callback_->readBufferAvailable(readBuf_.move());
      } else {
        if (trickleCallback_) {
          trickleCallback_();
        }
        callback_->readBufferAvailable(readBuf_.split(1));
      }
    }
  }

  LocalTransport* peer_{nullptr};
  folly::EventBase* evb_{nullptr};
  size_t received_{0};
  size_t written_{0};
  folly::IOBufQueue readBuf_{folly::IOBufQueue::cacheChainLength()};
  ReadCallback* callback_{nullptr};

  bool trickle_{false};
  folly::Function<void()> trickleCallback_;
};
} // namespace test
} // namespace fizz
