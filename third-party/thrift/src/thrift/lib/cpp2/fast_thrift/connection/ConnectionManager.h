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
#include <memory>

#include <fizz/server/FizzServerContext.h>
#include <folly/Executor.h>
#include <folly/Synchronized.h>
#include <folly/container/F14Map.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/futures/Future.h>
#include <folly/io/async/AsyncServerSocket.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/logging/xlog.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/fast_thrift/connection/ConnectionHandler.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersContext.h>

namespace apache::thrift::fast_thrift::connection {

/**
 * ConnectionManager accepts connections using SO_REUSEPORT and delegates
 * connection handling to ConnectionHandler instances (one per EventBase).
 *
 * Users provide a ConnectionFactory to customize how connections are
 * constructed for new accepted sockets.
 */
class ConnectionManager : public folly::DelayedDestruction {
 public:
  using Ptr =
      std::unique_ptr<ConnectionManager, folly::DelayedDestruction::Destructor>;

  ConnectionManager(const ConnectionManager&) = delete;
  ConnectionManager& operator=(const ConnectionManager&) = delete;
  ConnectionManager(ConnectionManager&&) = delete;
  ConnectionManager& operator=(ConnectionManager&&) = delete;

  /**
   * IOObserver handles EventBase registration/unregistration events from the
   * IOThreadPoolExecutor. It delegates to ConnectionManager for creating
   * and destroying ConnectionHandler instances.
   */
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
      ConnectionFactory connectionFactory,
      std::shared_ptr<const fizz::server::FizzServerContext> fizzContext,
      std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams,
      std::chrono::milliseconds tlsHandshakeTimeout,
      SocketOptions socketOptions) {
    return Ptr(new ConnectionManager(
        std::move(address),
        std::move(executor),
        std::move(connectionFactory),
        std::move(fizzContext),
        std::move(thriftParams),
        tlsHandshakeTimeout,
        socketOptions));
  }

  void start() {
    DCHECK(state_.load() != State::STARTED);
    // Set STARTED before addObserver — addObserver synchronously invokes
    // registerEventBase on every IO thread, and that path gates on state_.
    state_.store(State::STARTED);
    executor_->addObserver(observer_);
  }

  // Stops accepting on every handler synchronously; existing connections
  // stay alive until closeConnections().
  void stopAccepting() {
    State expected = State::STARTED;
    if (!state_.compare_exchange_strong(
            expected, State::STOP_ACCEPTING_CONNECTIONS)) {
      return;
    }
    constexpr std::chrono::seconds kAcceptStoppedTimeout{30};
    auto snapshot = snapshotHandlers();
    for (auto& [evb, handler] : snapshot) {
      DCHECK(!evb->inRunningEventBaseThread())
          << "must not be called from an IO EVB thread; would deadlock";
      folly::Baton<> done;
      evb->runImmediatelyOrRunInEventBaseThreadAndWait(
          [handler, &done] { handler->stopAccepting(done); });
      CHECK(done.try_wait_for(kAcceptStoppedTimeout))
          << "acceptStopped did not fire within "
          << kAcceptStoppedTimeout.count() << "s";
    }
  }

  // Tear down every ConnectionHandler and its existing rocket connections.
  // Caller must have stopped accept first (via stopAccepting() or stop());
  // ConnectionHandler's dtor DCHECKs that state is STOPPED.
  void closeConnections() {
    State prev = state_.exchange(State::STOPPED);
    if (prev == State::NONE || prev == State::STOPPED) {
      return;
    }
    auto snapshot = snapshotHandlers();
    for (auto& [evb, handler] : snapshot) {
      evb->runImmediatelyOrRunInEventBaseThreadAndWait(
          [handler] { handler->closeAllConnections(); });
    }
    executor_->removeObserver(observer_);
  }

  // Toggle SO_REUSEPORT cBPF random-spread attachment for newly registered
  // EventBases. Must be called before start(); the flag is read by
  // registerEventBase() and applied to each handler before startAccepting().
  void setEnableReusePortBpfSpread(bool e) noexcept {
    enableReusePortBpfSpread_ = e;
  }

  /**
   * Get the server's bound address from the first connection handler.
   * Returns the address of any listening socket, useful when binding to port 0.
   */
  folly::SocketAddress getAddress() const {
    folly::SocketAddress address;
    connectionHandlers_.withRLock([&](const auto& handlerMap) {
      if (!handlerMap.empty()) {
        handlerMap.begin()->second->getAddress(&address);
      }
    });
    return address;
  }

 protected:
  ConnectionManager(
      folly::SocketAddress address,
      folly::Executor::KeepAlive<folly::IOThreadPoolExecutorBase> executor,
      ConnectionFactory connectionFactory,
      std::shared_ptr<const fizz::server::FizzServerContext> fizzContext,
      std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams,
      std::chrono::milliseconds tlsHandshakeTimeout,
      SocketOptions socketOptions)
      : address_(std::move(address)),
        executor_(std::move(executor)),
        connectionFactory_(std::move(connectionFactory)),
        fizzContext_(std::move(fizzContext)),
        thriftParams_(std::move(thriftParams)),
        tlsHandshakeTimeout_(tlsHandshakeTimeout),
        socketOptions_(socketOptions),
        observer_(std::make_shared<IOObserver>(*this)) {}

  // Both calls are idempotent — safe whether the caller already invoked
  // them explicitly or is relying on the dtor to drive shutdown.
  ~ConnectionManager() override {
    stopAccepting();
    closeConnections();
  }

 private:
  friend class IOObserver;

  enum class State { NONE, STARTED, STOP_ACCEPTING_CONNECTIONS, STOPPED };

  // Snapshot (evb, handler) pairs under the rlock so callers can iterate
  // without holding the lock across EVB hops (which would deadlock).
  std::vector<std::pair<folly::EventBase*, ConnectionHandler*>>
  snapshotHandlers() {
    std::vector<std::pair<folly::EventBase*, ConnectionHandler*>> snapshot;
    connectionHandlers_.withRLock([&](const auto& handlerMap) {
      snapshot.reserve(handlerMap.size());
      for (const auto& [evb, handler] : handlerMap) {
        snapshot.emplace_back(evb, handler.get());
      }
    });
    return snapshot;
  }

  void registerEventBase(folly::EventBase& evb) {
    DestructorGuard dg(this);

    // After stopAccepting() / stop(), refuse to spin up listeners on EVBs
    // that the executor adds during shutdown.
    if (state_.load(std::memory_order_acquire) != State::STARTED) {
      return;
    }

    connectionHandlers_.withWLock([&](auto& handlerMap) {
      ConnectionHandler::Ptr connectionHandler(new ConnectionHandler(
          evb,
          connectionFactory_,
          fizzContext_,
          thriftParams_,
          tlsHandshakeTimeout_,
          socketOptions_));
      connectionHandler->setEnableReusePortBpfSpread(enableReusePortBpfSpread_);
      auto [it, inserted] =
          handlerMap.emplace(&evb, std::move(connectionHandler));
      if (inserted) {
        it->second->startAccepting(address_);
      } else {
        LOG(FATAL) << "EventBase already registered";
      }
    });
  }

  void unregisterEventBase(folly::EventBase& evb) {
    DestructorGuard dg(this);

    connectionHandlers_.withWLock([&](auto& handlerMap) {
      if (auto handlerIt = handlerMap.find(&evb);
          handlerIt != handlerMap.end()) {
        // Contract: caller has driven the handler to STOPPED via
        // stopAccepting() and closed its connections via closeConnections()
        // before reaching here. ConnectionHandler's dtor DCHECKs both.
        handlerMap.erase(handlerIt);
      }
    });
  }

  std::atomic<State> state_{State::NONE};
  folly::SocketAddress address_;
  folly::Executor::KeepAlive<folly::IOThreadPoolExecutorBase> executor_;
  ConnectionFactory connectionFactory_;
  std::shared_ptr<const fizz::server::FizzServerContext> fizzContext_;
  std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams_;
  std::chrono::milliseconds tlsHandshakeTimeout_;
  SocketOptions socketOptions_;
  std::shared_ptr<IOObserver> observer_;

  folly::Synchronized<
      folly::F14FastMap<folly::EventBase*, ConnectionHandler::Ptr>>
      connectionHandlers_;
  // Set-once before start(); consumed by registerEventBase to apply at each
  // new handler's startAccepting() time.
  bool enableReusePortBpfSpread_{false};
};

} // namespace apache::thrift::fast_thrift::connection
