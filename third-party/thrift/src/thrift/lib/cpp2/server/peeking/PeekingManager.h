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

#include <folly/ExceptionString.h>
#include <folly/io/Cursor.h>
#include <folly/io/async/DecoratedAsyncTransportWrapper.h>
#include <thrift/lib/cpp/server/TServerObserver.h>
#include <thrift/lib/cpp2/server/Cpp2Worker.h>
#include <thrift/lib/cpp2/server/peeking/TLSHelper.h>
#include <wangle/acceptor/Acceptor.h>
#include <wangle/acceptor/ManagedConnection.h>
#include <wangle/acceptor/SocketPeeker.h>

namespace apache {
namespace thrift {

/**
 * The number of bytes that will be read from the socket.
 * TLSHelper currently needs the most bytes. Thus, it's cap
 * it up at the amount that TLSHelper needs.
 */
constexpr uint8_t kPeekBytes = 13;

/**
 * A manager that rejects or accepts connections based on critera
 * added by helper functions. This is useful for cases where
 * clients might be sending different types of protocols
 * over plaintext and it's up to the Acceptor to determine
 * what kind of protocol they are talking to route to the
 * appropriate handlers.
 */
class PeekingManagerBase : public wangle::ManagedConnection {
 public:
  PeekingManagerBase(
      std::shared_ptr<apache::thrift::Cpp2Worker> acceptor,
      const folly::SocketAddress& clientAddr,
      wangle::TransportInfo tinfo,
      apache::thrift::ThriftServer* server)
      : acceptor_(acceptor),
        clientAddr_(clientAddr),
        tinfo_(std::move(tinfo)),
        server_(server) {
    acceptor_->getConnectionManager()->addConnection(this, true);
  }

  void timeoutExpired() noexcept override { dropConnection(); }

  void dropConnection(const std::string& /* errorMsg */ = "") override {
    acceptor_->getConnectionManager()->removeConnection(this);
    destroy();
  }

  void describe(std::ostream& os) const override {
    os << "Peeking the socket " << clientAddr_;
  }

  bool isBusy() const override { return true; }

  void notifyPendingShutdown() override {}

  void closeWhenIdle() override {}

  void dumpConnectionState(uint8_t /* loglevel */) override {}

  const folly::SocketAddress& getPeerAddress() const noexcept override {
    return clientAddr_;
  }

 protected:
  const std::shared_ptr<apache::thrift::Cpp2Worker> acceptor_;
  const folly::SocketAddress clientAddr_;
  wangle::TransportInfo tinfo_;
  ThriftServer* const server_;
};

class CheckTLSPeekingManager : public PeekingManagerBase,
                               public wangle::SocketPeeker::Callback {
 public:
  CheckTLSPeekingManager(
      std::shared_ptr<apache::thrift::Cpp2Worker> acceptor,
      const folly::SocketAddress& clientAddr,
      wangle::TransportInfo tinfo,
      apache::thrift::ThriftServer* server,
      folly::AsyncSocket::UniquePtr socket,
      std::shared_ptr<apache::thrift::server::TServerObserver> obs)
      : PeekingManagerBase(
            std::move(acceptor), clientAddr, std::move(tinfo), server),
        socket_(std::move(socket)),
        observer_(std::move(obs)),
        peeker_(new wangle::SocketPeeker(*socket_, this, kPeekBytes)) {
    peeker_->start();
  }

  ~CheckTLSPeekingManager() override {
    if (socket_) {
      socket_->closeNow();
    }
  }

  void peekSuccess(std::vector<uint8_t> peekBytes) noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    dropConnection();
    if (TLSHelper::looksLikeTLS(peekBytes)) {
      LOG(ERROR) << "Received SSL connection on non SSL port";
      sendPlaintextTLSAlert(peekBytes);
      if (observer_) {
        observer_->protocolError();
      }
      return;
    }
    acceptor_->connectionReady(
        std::move(socket_),
        std::move(clientAddr_),
        {},
        SecureTransportType::NONE,
        tinfo_);
  }

  void sendPlaintextTLSAlert(const std::vector<uint8_t>& peekBytes) {
    uint8_t major = peekBytes[1];
    uint8_t minor = peekBytes[2];
    auto alert = TLSHelper::getPlaintextAlert(
        major, minor, TLSHelper::Alert::UNEXPECTED_MESSAGE);
    socket_->writeChain(nullptr, std::move(alert));
  }

  void peekError(const folly::AsyncSocketException&) noexcept override {
    dropConnection();
  }

  void dropConnection(const std::string& errorMsg = "") override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    peeker_ = nullptr;
    PeekingManagerBase::dropConnection(errorMsg);
  }

 private:
  folly::AsyncSocket::UniquePtr socket_;
  std::shared_ptr<apache::thrift::server::TServerObserver> observer_;
  typename wangle::SocketPeeker::UniquePtr peeker_;
};

/**
 * A DecoratedAsyncTransportWrapper that also implements ReadCallback so it can
 * intercept and replay reads in order to deliver pre-received bytes.
 *
 * This callback supports both the getReadBuffer/readDataAvailable and
 * getBufferAvailable APIs, allowing the underlying socket implementation to
 * interact with it the same way it would've with the real ReadCallback.
 *
 * NOTE: Pre-received data is delivered lazily, on an ensuing read from the
 * wrapped transport. This could mean bytes are never delivered. In practice,
 * the peeked bytes represent only a small portion of the bytes received from
 * the peer and therefore more reads will be triggered to consume what remains.
 */
class PreReceivedDataAsyncTransportWrapper
    : public folly::DecoratedAsyncTransportWrapper<folly::AsyncTransport>,
      public folly::AsyncTransport::ReadCallback {
  using Base = folly::DecoratedAsyncTransportWrapper<folly::AsyncTransport>;

 public:
  using UniquePtr = std::unique_ptr<AsyncTransport, Destructor>;

  static UniquePtr create(
      folly::AsyncTransport::UniquePtr socket,
      std::vector<uint8_t> preReceivedData) {
    DCHECK(!socket->getReadCallback());
    return UniquePtr(new PreReceivedDataAsyncTransportWrapper(
        std::move(socket), std::move(preReceivedData)));
  }

  folly::AsyncTransport::ReadCallback* getReadCallback() const override {
    /**
     * Some ReadCallback implementations look back at their AsyncTransport and
     * its registered ReadCallback beforing unregistering themselves. This needs
     * to return the ReadCallback instance it plans to delegate to.
     */
    return readCallback_;
  }

  void setReadCB(folly::AsyncTransport::ReadCallback* callback) override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    readCallback_ = callback;
    if (preReceivedData_ != nullptr) {
      if (readCallback_ == nullptr) {
        // unregister read callback on the underlying transport as well
        Base::setReadCB(nullptr);
        return;
      }
      // Set self as read callback, so we can intercept the next read(s) and
      // replay the pre-received data
      Base::setReadCB(this);
    } else {
      /**
       * This should only be called after the pre-received bytes have been
       * delivered. Just pass the call through to the underlying transport.
       *
       * NOTE: This might be called while
       * PreReceivedDataAsyncTransportWrapper::readDataAvailable and callbacks
       * further down the stack are being invoked.
       */
      Base::setReadCB(callback);
    }
  }

  void getReadBuffer(void** bufReturn, size_t* lenReturn) override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    /**
     * We prepend the pre-received data to a buffer retrieved from the real
     * callback. We must ensure we have enough bytes to read while also
     * returning a non-0 length to the underlying transport.
     */
    void* buf;
    size_t bufSize;
    readCallback_->getReadBuffer(&buf, &bufSize);

    size_t preReceivedDataLength = preReceivedData_->size();
    // we need enough space to read the pre-received data and then some
    if (bufSize <= preReceivedDataLength) {
      // otherwise, fail to produce a buffer
      *bufReturn = nullptr;
      *lenReturn = 0;
      return;
    }

    // read our pre-received data into the real callback's buffer
    std::memcpy(buf, preReceivedData_->data(), preReceivedDataLength);
    // return the buffer, offset by the number of bytes we read into it
    *bufReturn = static_cast<char*>(buf) + preReceivedDataLength;
    *lenReturn = bufSize - preReceivedDataLength;
  }

  void readDataAvailable(size_t len) noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    size_t preReceivedDataLength = preReceivedData_->size();
    // Null out as a signal to setReadCB that we've replayed the data.
    // Must be done before we hand off the data to the real callback.
    preReceivedData_ = nullptr;
    readCallback_->readDataAvailable(len + preReceivedDataLength);
    /**
     * CAUTION: The readDataAvailable call above, internally, might unset
     * and/or replace this instance as the transport's read callback. Only
     * unregister the instance if it's still the active read callback.
     *
     * Note also that readDataAvailable could delete the readCallback_, but this
     * shouldn't happen without unsetting this transport's read callback and
     * therefore the code below should be safe.
     */
    if (Base::getReadCallback() == this) {
      Base::setReadCB(readCallback_);
    }
  }

  bool isBufferMovable() noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    return readCallback_->isBufferMovable();
  }

  void readBufferAvailable(
      std::unique_ptr<IOBuf> newContent) noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    // Concat the two data buffers to pass them along to the real callback.
    auto preReceivedDataBuffer = folly::IOBuf::copyBuffer(
        preReceivedData_->data(), preReceivedData_->size());
    preReceivedDataBuffer->appendToChain(std::move(newContent));
    // Null out as a signal to setReadCB that we've replayed the data.
    // Must be done before we hand off the data to the real callback.
    preReceivedData_ = nullptr;
    readCallback_->readBufferAvailable(std::move(preReceivedDataBuffer));

    // See the comment marked CAUTION in readDataAvailable, the same applies.
    if (Base::getReadCallback() == this) {
      Base::setReadCB(readCallback_);
    }
  }

  void readEOF() noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    readCallback_->readEOF();
  }

  void readErr(const folly::AsyncSocketException& ex) noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    readCallback_->readErr(ex);
  }

  std::unique_ptr<IOBuf> takePreReceivedData() override {
    if (!preReceivedData_ || preReceivedData_->empty()) {
      return {};
    }
    auto freeVec = [](void*, void* userData) {
      delete reinterpret_cast<std::vector<uint8_t>*>(userData);
    };
    std::vector<uint8_t>* ptr = preReceivedData_.release();
    return IOBuf::takeOwnership(
        static_cast<void*>(ptr->data()),
        ptr->size(),
        0,
        ptr->size(),
        freeVec,
        ptr);
  }

 private:
  PreReceivedDataAsyncTransportWrapper(
      folly::AsyncTransport::UniquePtr socket,
      std::vector<uint8_t> preReceivedData)
      : Base(std::move(socket)),
        preReceivedData_(
            preReceivedData.size() ? std::make_unique<std::vector<uint8_t>>(
                                         std::move(preReceivedData))
                                   : std::unique_ptr<std::vector<uint8_t>>()) {}

  std::unique_ptr<std::vector<uint8_t>> preReceivedData_;
  folly::AsyncTransport::ReadCallback* readCallback_{};
};

class TransportPeekingManager : public PeekingManagerBase,
                                public wangle::SocketPeeker::Callback {
 public:
  TransportPeekingManager(
      std::shared_ptr<apache::thrift::Cpp2Worker> acceptor,
      const folly::SocketAddress& clientAddr,
      wangle::TransportInfo tinfo,
      apache::thrift::ThriftServer* server,
      folly::AsyncTransport::UniquePtr socket)
      : PeekingManagerBase(
            std::move(acceptor), clientAddr, std::move(tinfo), server),
        socket_(std::move(socket)),
        peeker_(new wangle::TransportPeeker(*socket_, this, kPeekBytes)) {
    peeker_->start();
  }

  ~TransportPeekingManager() override {
    if (socket_) {
      socket_->closeNow();
    }
  }

  void peekSuccess(std::vector<uint8_t> peekBytes) noexcept override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    dropConnection();

    // This is possible when acceptor is stopped between taking a new
    // connection and calling back to this function.
    if (acceptor_->isStopping()) {
      return;
    }

    auto transport = PreReceivedDataAsyncTransportWrapper::create(
        std::move(socket_), peekBytes);

    try {
      // Check for new transports
      for (const auto& handler : *server_->getRoutingHandlers()) {
        if (handler->canAcceptConnection(peekBytes, tinfo_)) {
          handler->handleConnection(
              acceptor_->getConnectionManager(),
              std::move(transport),
              &clientAddr_,
              tinfo_,
              acceptor_);
          return;
        }
      }

      // Default to Header Transport
      acceptor_->handleHeader(std::move(transport), &clientAddr_, tinfo_);
    } catch (...) {
      LOG(ERROR) << __func__ << " failed, dropping connection: "
                 << folly::exceptionStr(std::current_exception());
    }
  }

  void peekError(const folly::AsyncSocketException&) noexcept override {
    dropConnection();
  }

  void dropConnection(const std::string& errorMsg = "") override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    peeker_ = nullptr;
    PeekingManagerBase::dropConnection(errorMsg);
  }

 private:
  folly::AsyncTransport::UniquePtr socket_;
  typename wangle::TransportPeeker::UniquePtr peeker_;
};

} // namespace thrift
} // namespace apache
