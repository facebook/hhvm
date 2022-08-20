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

class PreReceivedDataAsyncTransportWrapper
    : public folly::DecoratedAsyncTransportWrapper<folly::AsyncTransport> {
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

  ReadCallback* getReadCallback() const override { return readCallback_; }

  void setReadCB(folly::AsyncTransport::ReadCallback* callback) override {
    folly::DelayedDestruction::DestructorGuard dg(this);
    readCallback_ = callback;
    if (preReceivedData_) {
      if (!readCallback_) {
        return;
      }
      const auto preReceivedData = std::exchange(preReceivedData_, {});
      void* buf;
      size_t bufSize;
      callback->getReadBuffer(&buf, &bufSize);
      CHECK(callback == readCallback_);
      CHECK(bufSize >= preReceivedData->size());
      std::memcpy(buf, preReceivedData->data(), preReceivedData->size());
      callback->readDataAvailable(preReceivedData->size());
    }
    if (readCallback_ == callback) {
      Base::setReadCB(callback);
    }
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
      bool acceptedHandler = false;
      for (const auto& handler : *server_->getRoutingHandlers()) {
        if (handler->canAcceptConnection(peekBytes, tinfo_)) {
          handler->handleConnection(
              acceptor_->getConnectionManager(),
              std::move(transport),
              &clientAddr_,
              tinfo_,
              acceptor_);
          acceptedHandler = true;
          break;
        }
      }

      // Default to Header Transport
      if (!acceptedHandler) {
        acceptor_->handleHeader(std::move(transport), &clientAddr_, tinfo_);
      }
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
