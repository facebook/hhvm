/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <functional>
#include <list>
#include <memory>

#include <folly/Likely.h>
#include <folly/ScopeGuard.h>
#include <folly/SocketAddress.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>

#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp/async/TAsyncSSLSocket.h>

#include "mcrouter/lib/network/McSSLUtil.h"

namespace folly {
class EventBase;
class SocketAddress;
} // namespace folly

namespace facebook {
namespace memcache {

class AsyncTlsToPlaintextSocket final : public folly::AsyncTransportWrapper {
 public:
  using Ptr = std::unique_ptr<AsyncTlsToPlaintextSocket, Destructor>;

  ~AsyncTlsToPlaintextSocket() override = default;

  static Ptr create(folly::AsyncTransportWrapper::UniquePtr impl) {
    return Ptr(new AsyncTlsToPlaintextSocket(std::move(impl)));
  }

  virtual const folly::AsyncTransportWrapper* getWrappedTransport()
      const override {
    return impl_.get();
  }

  void connect(
      folly::AsyncSocket::ConnectCallback* connectCallback,
      const folly::SocketAddress& address,
      std::chrono::milliseconds connectTimeout,
      folly::SocketOptionMap socketOptions);

  // AsyncSocketBase overrides
  folly::EventBase* getEventBase() const override {
    return &evb_;
  }

  // AsyncTransport overrides
  void close() override {
    failAllWrites(folly::AsyncSocketException{
        folly::AsyncSocketException::END_OF_FILE,
        "AsyncTlsToPlaintextSocket closed"});
    if (auto* readCallback = std::exchange(readCallback_, nullptr)) {
      readCallback->readEOF();
    }
    impl_->close();
  }
  void closeNow() override {
    failAllWrites(folly::AsyncSocketException{
        folly::AsyncSocketException::END_OF_FILE,
        "AsyncTlsToPlaintextSocket closed"});
    if (auto* readCallback = std::exchange(readCallback_, nullptr)) {
      readCallback->readEOF();
    }
    impl_->closeNow();
  }
  void shutdownWrite() override {
    LOG(FATAL) << "Unimplemented";
  }
  void shutdownWriteNow() override {
    LOG(FATAL) << "Unimplemented";
  }
  bool good() const override {
    return impl_->good();
  }
  bool readable() const override {
    return impl_->readable();
  }
  bool connecting() const override {
    return impl_->connecting();
  }
  bool error() const override {
    return impl_->error();
  }

  void attachEventBase(folly::EventBase*) override {
    LOG(FATAL) << "Unimplemented";
  }
  void detachEventBase() override {
    LOG(FATAL) << "Unimplemented";
  }
  bool isDetachable() const override {
    LOG(FATAL) << "Unimplemented";
    return false;
  }

  void setSendTimeout(uint32_t sendTimeoutMs) override {
    writeTimeout_ = std::chrono::milliseconds(sendTimeoutMs);
    impl_->setSendTimeout(writeTimeout_.count());
  }
  uint32_t getSendTimeout() const override {
    return writeTimeout_.count();
  }

  void getLocalAddress(folly::SocketAddress* address) const override {
    impl_->getLocalAddress(address);
  }
  void getPeerAddress(folly::SocketAddress* address) const override {
    impl_->getPeerAddress(address);
  }

  bool isEorTrackingEnabled() const override {
    return impl_->isEorTrackingEnabled();
  }
  void setEorTracking(bool track) override {
    impl_->setEorTracking(track);
  }
  size_t getAppBytesWritten() const override {
    return impl_->getAppBytesWritten();
  }
  size_t getRawBytesWritten() const override {
    return impl_->getRawBytesWritten();
  }
  size_t getAppBytesReceived() const override {
    return impl_->getRawBytesWritten();
  }
  size_t getRawBytesReceived() const override {
    return impl_->getRawBytesReceived();
  }

  void setReadCB(
      folly::AsyncTransportWrapper::ReadCallback* callback) override {
    if (state_ == State::CONNECTING) {
      readCallback_ = callback;
    } else {
      impl_->setReadCB(callback);
    }
  }
  folly::AsyncTransportWrapper::ReadCallback* getReadCallback() const override {
    if (state_ == State::CONNECTING) {
      return readCallback_;
    }
    return impl_->getReadCallback();
  }

  void writeChain(
      folly::AsyncTransportWrapper::WriteCallback* callback,
      std::unique_ptr<folly::IOBuf>&& buf,
      folly::WriteFlags flags = folly::WriteFlags::NONE) override {
    if (FOLLY_UNLIKELY(state_ == State::CONNECTING)) {
      bufferedWrites_.push_back(BufferedWrite{callback, std::move(buf)});
    } else {
      impl_->writeChain(callback, std::move(buf), flags);
    }
  }

  void write(
      folly::AsyncTransportWrapper::WriteCallback*,
      const void*,
      size_t,
      folly::WriteFlags) override {
    LOG(FATAL) << "Unimplemented";
  }

  void writev(
      folly::AsyncTransportWrapper::WriteCallback*,
      const iovec*,
      size_t,
      folly::WriteFlags) override {
    LOG(FATAL) << "Unimplemented";
  }

  enum class SessionResumptionStatus : uint8_t {
    RESUMPTION_NOT_ATTEMPTED,
    RESUMPTION_ATTEMPTED_AND_FAILED,
    RESUMPTION_ATTEMPTED_AND_SUCCEEDED,
  };

  SessionResumptionStatus getSessionResumptionStatus() const {
    return resumptionStatus_;
  }

 private:
  folly::AsyncTransportWrapper::UniquePtr impl_;
  folly::EventBase& evb_;

  struct BufferedWrite {
    BufferedWrite(
        folly::AsyncTransportWrapper::WriteCallback* cb,
        std::unique_ptr<folly::IOBuf> b)
        : callback(cb), buf(std::move(b)) {}

    folly::AsyncTransportWrapper::WriteCallback* callback;
    std::unique_ptr<folly::IOBuf> buf;
  };
  std::list<BufferedWrite> bufferedWrites_;

  enum class State : uint8_t {
    CONNECTING,
    CONNECTED,
  };
  State state_{State::CONNECTING};
  folly::AsyncTransportWrapper::ReadCallback* readCallback_{nullptr};
  std::chrono::milliseconds writeTimeout_{std::chrono::milliseconds::zero()};
  SessionResumptionStatus resumptionStatus_{
      SessionResumptionStatus::RESUMPTION_NOT_ATTEMPTED};

  explicit AsyncTlsToPlaintextSocket(
      folly::AsyncTransportWrapper::UniquePtr impl)
      : impl_(std::move(impl)), evb_(*impl_->getEventBase()) {
    DCHECK(
        dynamic_cast<apache::thrift::async::TAsyncSSLSocket*>(impl_.get()) !=
        nullptr);
  }

  void flushWrites();
  void failAllWrites(const folly::AsyncSocketException& ex);

  class ConnectCallback;
};

} // namespace memcache
} // namespace facebook
