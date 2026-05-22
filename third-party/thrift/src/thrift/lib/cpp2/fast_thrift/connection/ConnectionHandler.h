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

#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include <fizz/server/FizzServerContext.h>
#include <folly/Executor.h>
#include <folly/io/async/AsyncServerSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/fast_thrift/connection/SocketOptions.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/common/RocketServerConnection.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzHandshakeHelper.h>
#include <thrift/lib/cpp2/fast_thrift/security/PendingHandshakes.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersContext.h>

namespace apache::thrift::fast_thrift::connection {

using RocketServerConnection = rocket::server::RocketServerConnection;

/**
 * ConnectionFactory creates a fully-wired RocketServerConnection for a new
 * accepted (and, if TLS is enabled, handshake-completed) transport. The
 * factory receives an AsyncTransport — either a raw AsyncSocket for plain
 * TCP, or a fizz::server::AsyncFizzServer for TLS connections.
 */
using ConnectionFactory = std::function<RocketServerConnection(
    folly::AsyncTransport::UniquePtr socket)>;

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

  ConnectionHandler(
      folly::EventBase& evb,
      ConnectionFactory connectionFactory,
      std::shared_ptr<const fizz::server::FizzServerContext> fizzContext,
      std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams,
      std::chrono::milliseconds tlsHandshakeTimeout,
      SocketOptions socketOptions);

  ConnectionHandler(const ConnectionHandler&) = delete;
  ConnectionHandler& operator=(const ConnectionHandler&) = delete;
  ConnectionHandler(ConnectionHandler&&) = delete;
  ConnectionHandler& operator=(ConnectionHandler&&) = delete;

  // Attach a cBPF program on the SO_REUSEPORT group at startAccepting()
  // time to replace the kernel's default 4-tuple-hash selection with
  // uniform random across worker listening sockets. Must be called before
  // startAccepting(); reading after that is a no-op.
  void setEnableReusePortBpfSpread(bool e) noexcept {
    enableReusePortBpfSpread_ = e;
  }

  // AcceptCallback interface
  void connectionAccepted(
      folly::NetworkSocket fd,
      const folly::SocketAddress& clientAddr,
      AcceptInfo) noexcept override;

  void acceptError(folly::exception_wrapper ew) noexcept override;

  void acceptStopped() noexcept override;

  void startAccepting(const folly::SocketAddress& address);

  void stopAccepting();

  void closeAllConnections();

  size_t connectionCount();

  void getAddress(folly::SocketAddress* address) const;

 protected:
  ~ConnectionHandler() override;

  void destroy() override;

  void onDelayedDestroy(bool delayed) override;

 private:
  enum class State { NONE, ACCEPTING, STOPPING, STOPPED };

  void installConnection(folly::AsyncTransport::UniquePtr socket);

  void removeConnection(transport::TransportHandler* transportHandler);

  void attachReusePortBpfSpread();

  State state_{State::NONE};
  folly::Executor::KeepAlive<folly::EventBase> evb_;
  folly::AsyncServerSocket::UniquePtr socket_;
  ConnectionFactory connectionFactory_;
  std::shared_ptr<const fizz::server::FizzServerContext> fizzContext_;
  std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams_;
  std::chrono::milliseconds tlsHandshakeTimeout_;
  SocketOptions socketOptions_;
  security::PendingHandshakes pendingHandshakes_;
  std::vector<RocketServerConnection> connections_;
  std::optional<folly::DelayedDestruction::DestructorGuard>
      stoppingDestroyGuard_;
  bool destroyPending_{false};
  bool enableReusePortBpfSpread_{false};
};

} // namespace apache::thrift::fast_thrift::connection
