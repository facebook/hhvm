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

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <optional>

#include <fizz/server/FizzServerContext.h>
#include <folly/Executor.h>
#include <folly/SocketAddress.h>
#include <folly/Synchronized.h>
#include <folly/container/F14Map.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/fast_thrift/connection/ConnectionFactory.h>
#include <thrift/lib/cpp2/fast_thrift/connection/ConnectionHandler.h>
#include <thrift/lib/cpp2/fast_thrift/connection/SocketOptions.h>
#include <thrift/lib/cpp2/fast_thrift/security/SSLPolicy.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersContext.h>

namespace apache::thrift::fast_thrift::connection {

// Disambiguate from sibling apache::thrift::fast_thrift::connection::security
// namespace.
namespace fast_security = ::apache::thrift::fast_thrift::security;

/**
 * ConnectionManager owns one ConnectionHandler per registered EventBase.
 * The IOObserver bounces register / unregister calls onto the owning EVB,
 * so each handler is constructed, configured, and torn down on its own
 * thread.
 *
 * The factory + optional onAccept callback are registered once via the
 * templated setConnectionFactory<F>(); the manager type-erases them so
 * the class itself stays non-templated.
 *
 * Shutdown is a single call: stop(timeout) stops accept across every
 * handler, initiates graceful drain on every live connection, waits up
 * to `timeout` for the drain to complete, then force-closes any
 * stragglers. The dtor calls stop() with the default timeout.
 */
class ConnectionManager : public folly::DelayedDestruction {
 public:
  using Ptr =
      std::unique_ptr<ConnectionManager, folly::DelayedDestruction::Destructor>;

  ConnectionManager(const ConnectionManager&) = delete;
  ConnectionManager& operator=(const ConnectionManager&) = delete;
  ConnectionManager(ConnectionManager&&) = delete;
  ConnectionManager& operator=(ConnectionManager&&) = delete;

  class IOObserver : public folly::IOThreadPoolExecutorBase::IOObserver {
   public:
    explicit IOObserver(ConnectionManager& manager) : manager_(manager) {}

    void registerEventBase(folly::EventBase& evb) noexcept override {
      evb.runImmediatelyOrRunInEventBaseThreadAndWait(
          [this, &evb]() { manager_.registerEventBase(evb); });
    }

    void unregisterEventBase(folly::EventBase& evb) noexcept override {
      evb.runImmediatelyOrRunInEventBaseThreadAndWait(
          [this, &evb]() { manager_.unregisterEventBase(evb); });
    }

   private:
    ConnectionManager& manager_;
  };

  static Ptr create(
      folly::SocketAddress address,
      folly::Executor::KeepAlive<folly::IOThreadPoolExecutorBase> executor,
      fast_security::SSLPolicy sslPolicy,
      std::shared_ptr<const fizz::server::FizzServerContext> fizzContext,
      std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams,
      std::optional<std::chrono::milliseconds> tlsHandshakeTimeout,
      SocketOptions socketOptions);

  /**
   * Register the factory (and optional onAccept hook) used to build
   * connections on every per-EVB handler. Must be called before start().
   *
   * The factory is owned by the manager (shared_ptr) so all per-EVB
   * handlers can safely borrow it.
   */
  template <ConnectionFactory F>
  void setConnectionFactory(
      F factory, std::function<void(FactoryConnectionType<F>&)> onAccept = {}) {
    auto shared = std::make_shared<F>(std::move(factory));
    configureHandler_ = [shared,
                         onAccept = std::move(onAccept)](ConnectionHandler& h) {
      h.setConnectionFactory<F>(*shared, onAccept);
    };
  }

  void start();

  /**
   * Single-call shutdown: stop accept on every handler, initiate
   * graceful drain on every live connection, wait up to `drainTimeout`
   * for the drain to complete, then force-close any stragglers and tear
   * the IOObserver down. Idempotent.
   */
  void stop(std::chrono::milliseconds drainTimeout = std::chrono::seconds{30});

  void setEnableReusePortBpfSpread(bool e) noexcept {
    enableReusePortBpfSpread_ = e;
  }

  folly::SocketAddress getAddress() const;

  size_t numHandlers() const noexcept {
    return handlers_.withRLock([](const auto& map) { return map.size(); });
  }

  size_t connectionCount() const noexcept {
    return handlers_.withRLock([](const auto& map) {
      size_t total = 0;
      for (const auto& [_, h] : map) {
        total += h->connectionCount();
      }
      return total;
    });
  }

 protected:
  ConnectionManager(
      folly::SocketAddress address,
      folly::Executor::KeepAlive<folly::IOThreadPoolExecutorBase> executor,
      fast_security::SSLPolicy sslPolicy,
      std::shared_ptr<const fizz::server::FizzServerContext> fizzContext,
      std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams,
      std::optional<std::chrono::milliseconds> tlsHandshakeTimeout,
      SocketOptions socketOptions);

  ~ConnectionManager() override;

 private:
  friend class IOObserver;

  enum class State { NONE, STARTED, STOPPED };

  void registerEventBase(folly::EventBase& evb);
  void unregisterEventBase(folly::EventBase& evb);

  std::atomic<State> state_{State::NONE};
  folly::SocketAddress address_;
  folly::Executor::KeepAlive<folly::IOThreadPoolExecutorBase> executor_;
  fast_security::SSLPolicy sslPolicy_;
  std::shared_ptr<const fizz::server::FizzServerContext> fizzContext_;
  std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams_;
  std::optional<std::chrono::milliseconds> tlsHandshakeTimeout_;
  SocketOptions socketOptions_;
  std::shared_ptr<IOObserver> observer_;

  // Configured by setConnectionFactory; applied to each new handler.
  // Captures shared_ptr<F> so the factory outlives every handler.
  std::function<void(ConnectionHandler&)> configureHandler_;

  folly::Synchronized<
      folly::F14FastMap<folly::EventBase*, ConnectionHandler::Ptr>>
      handlers_;
  bool enableReusePortBpfSpread_{false};
};

} // namespace apache::thrift::fast_thrift::connection
