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

#include <functional>

#include <folly/io/async/AsyncServerSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/common/RocketServerConnection.h>

namespace apache::thrift::fast_thrift::rocket::server::connection {

using RocketServerConnection = rocket::server::RocketServerConnection;

/**
 * ConnectionFactory creates a fully-wired RocketServerConnection for a new
 * accepted socket. The factory receives the socket (from which the EventBase
 * can be obtained via getEventBase()) and returns a RocketServerConnection
 * owning the transport handler and pipeline.
 */
using ConnectionFactory =
    std::function<RocketServerConnection(folly::AsyncSocket::UniquePtr socket)>;

/**
 * ConnectionHandler manages connections for a single EventBase.
 * It holds the server socket and all active connections for that EventBase.
 * It delegates connection creation to a user-provided ConnectionFactory.
 *
 * Implements AcceptCallback to handle accepted socket connections directly.
 */
class ConnectionHandler : public folly::DelayedDestruction,
                          public folly::AsyncServerSocket::AcceptCallback {
 public:
  using Ptr =
      std::unique_ptr<ConnectionHandler, folly::DelayedDestruction::Destructor>;

  explicit ConnectionHandler(
      folly::EventBase& evb, ConnectionFactory connectionFactory)
      : evb_(folly::getKeepAliveToken(&evb)),
        socket_(new folly::AsyncServerSocket(evb_.get())),
        connectionFactory_(std::move(connectionFactory)) {}

  ConnectionHandler(const ConnectionHandler&) = delete;
  ConnectionHandler& operator=(const ConnectionHandler&) = delete;
  ConnectionHandler(ConnectionHandler&&) = delete;
  ConnectionHandler& operator=(ConnectionHandler&&) = delete;

  // AcceptCallback interface
  void connectionAccepted(
      folly::NetworkSocket fd,
      const folly::SocketAddress& clientAddr,
      AcceptInfo) noexcept override {
    if (state_ != State::ACCEPTING) {
      folly::netops::close(fd);
      return;
    }

    XLOG(DBG3) << "Connection accepted from " << clientAddr.describe();

    auto socket = folly::AsyncSocket::newSocket(evb_.get(), fd);
    auto conn = connectionFactory_(std::move(socket));

    auto* rawTransportHandler = conn.transportHandler.get();
    conn.transportHandler->setCloseCallback([this, rawTransportHandler]() {
      removeConnection(rawTransportHandler);
    });

    connections_.emplace_back(std::move(conn));

    rawTransportHandler->onConnect();
  }

  void acceptError(folly::exception_wrapper ew) noexcept override {
    XLOG(ERR) << "Accept error: " << ew.what();
  }

  void acceptStopped() noexcept override {
    DestructorGuard dg(this);
    XLOG(DBG3) << "Accept stopped";
    state_ = State::STOPPED;
    closeAllConnections();

    // Release the guard - this may trigger deferred destruction
    stoppingDestroyGuard_.reset();
  }

  void startAccepting(const folly::SocketAddress& address) {
    DestructorGuard dg(this);

    DCHECK(state_ == State::NONE);
    DCHECK(socket_);
    socket_->setReusePortEnabled(true);
    socket_->bind(address);
    socket_->listen(1024);
    socket_->addAcceptCallback(this, evb_.get());
    socket_->startAccepting();
    state_ = State::ACCEPTING;
  }

  void stopAccepting() {
    if (state_ != State::ACCEPTING) {
      return;
    }

    DCHECK(socket_);
    state_ = State::STOPPING;

    // Hold a guard to keep the object alive until acceptStopped() is called
    stoppingDestroyGuard_.emplace(this);

    socket_->removeAcceptCallback(this, evb_.get());
  }

  void closeAllConnections() {
    for (auto& conn : connections_) {
      conn.close(folly::exception_wrapper{});
    }
    connections_.clear();
  }

  size_t connectionCount() {
    size_t count = 0;
    evb_->runImmediatelyOrRunInEventBaseThreadAndWait(
        [&count, this] { count = connections_.size(); });
    return count;
  }

  void getAddress(folly::SocketAddress* address) const {
    socket_->getAddress(address);
  }

 protected:
  ~ConnectionHandler() override {
    if (FOLLY_LIKELY(socket_)) {
      evb_->runImmediatelyOrRunInEventBaseThreadAndWait(
          [socket = std::move(socket_)]() mutable { socket.reset(); });
    }
  }

  void destroy() override {
    if (state_ == State::STOPPED || state_ == State::NONE) {
      onDelayedDestroy(false);
    } else {
      destroyPending_ = true;
      if (state_ == State::ACCEPTING) {
        // Schedule destruction after acceptStopped callback completes
        evb_->runInEventBaseThread([this] { stopAccepting(); });
        return;
      }
      if (state_ == State::STOPPING) {
        // Still waiting for acceptStopped callback
        return;
      }
    }
  }

  void onDelayedDestroy(bool delayed) override {
    if (delayed && !destroyPending_) {
      return;
    }
    // State is STOPPED or NONE - safe to delete
    delete this;
  }

 private:
  enum class State { NONE, ACCEPTING, STOPPING, STOPPED };

  void removeConnection(transport::TransportHandler* transportHandler) {
    DestructorGuard dg(this);

    auto it = std::find_if(
        connections_.begin(),
        connections_.end(),
        [transportHandler](const RocketServerConnection& conn) {
          return conn.transportHandler.get() == transportHandler;
        });
    if (it != connections_.end()) {
      it->close(folly::exception_wrapper{});
      connections_.erase(it);
    }
  }

  State state_{State::NONE};
  folly::Executor::KeepAlive<folly::EventBase> evb_;
  folly::AsyncServerSocket::UniquePtr socket_;
  ConnectionFactory connectionFactory_;
  std::vector<RocketServerConnection> connections_;
  std::optional<folly::DelayedDestruction::DestructorGuard>
      stoppingDestroyGuard_;
  bool destroyPending_{false};
};

} // namespace apache::thrift::fast_thrift::rocket::server::connection
