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
#include <functional>
#include <memory>

#include <folly/Executor.h>
#include <folly/SocketAddress.h>
#include <folly/Synchronized.h>
#include <folly/container/F14Map.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <folly/observer/Observer.h>
#include <folly/observer/SimpleObservable.h>
#include <thrift/lib/cpp2/fast_thrift/connection/ConnectionFactory.h>
#include <thrift/lib/cpp2/fast_thrift/connection/ConnectionHandler.h>
#include <thrift/lib/cpp2/fast_thrift/connection/SocketOptions.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerContextBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/security/SSLPolicy.h>

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
 * Shutdown is a single call: stop() stops accept across every handler,
 * triggers close on every live connection, and waits for them all to
 * fully tear down. Each connection bounds its own termination — there
 * is no outer deadline. The dtor calls stop().
 *
 * TLS parameters are owned via a SimpleObservable so setTLSParams() can
 * hot-reload them; every per-EVB ConnectionHandler captures an Observer
 * off this source.
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
      std::shared_ptr<const fast_security::TLSParams> tlsParams,
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
   * Single-call shutdown: stop accept on every handler, trigger close on
   * every live connection, wait for them all to fully tear down, then
   * drop the IOObserver. Each connection bounds its own termination —
   * there is no outer deadline. Idempotent.
   */
  void stop();

  void setEnableReusePortBpfSpread(bool e) noexcept {
    enableReusePortBpfSpread_ = e;
  }

  /**
   * Replace the TLS parameters used by future accepts. A single setValue
   * is observed by every accept on every EVB; no fan-out required.
   * In-flight handshakes keep the previous params alive via the shared_ptr
   * they captured at start.
   *
   * Safe to call from any thread; safe to call before or after start().
   * No-op for handlers built with SSLPolicy::DISABLED.
   */
  void setTLSParams(std::shared_ptr<const fast_security::TLSParams> tlsParams) {
    tlsParamsObservable_.setValue(std::move(tlsParams));
  }

  // Observer over the source-of-truth TLSParams.
  folly::observer::Observer<std::shared_ptr<const fast_security::TLSParams>>
  getTLSParamsObserver() {
    return tlsParamsObservable_.getObserver();
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
      std::shared_ptr<const fast_security::TLSParams> tlsParams,
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
  // Source-of-truth TLS params. setTLSParams is a single setValue,
  // observed by every accept on every EVB. Declaration before handlers_
  // matters: handlers outlive their Observer's source only if this is
  // destroyed last (member dtor order = reverse decl).
  folly::observer::SimpleObservable<
      std::shared_ptr<const fast_security::TLSParams>>
      tlsParamsObservable_;
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
