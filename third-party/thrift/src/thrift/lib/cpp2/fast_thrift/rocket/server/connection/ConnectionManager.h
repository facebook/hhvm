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
#include <thrift/lib/cpp2/fast_thrift/rocket/server/connection/ConnectionHandler.h>

namespace apache::thrift::fast_thrift::rocket::server::connection {

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
      folly::Executor::KeepAlive<folly::IOThreadPoolExecutor> executor,
      ConnectionFactory connectionFactory) {
    return Ptr(new ConnectionManager(
        std::move(address), std::move(executor), std::move(connectionFactory)));
  }

  void start() {
    DCHECK(state_.load() != State::STARTED);
    executor_->addObserver(observer_);
    state_.store(State::STARTED);
  }

  void stop() {
    if (state_.exchange(State::STOPPED) != State::STARTED) {
      return;
    }
    executor_->removeObserver(observer_);
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
      folly::Executor::KeepAlive<folly::IOThreadPoolExecutor> executor,
      ConnectionFactory connectionFactory)
      : address_(std::move(address)),
        executor_(std::move(executor)),
        connectionFactory_(std::move(connectionFactory)),
        observer_(std::make_shared<IOObserver>(*this)) {}

  ~ConnectionManager() override = default;

  void onDelayedDestroy(bool delayed) override {
    if (delayed && !getDestroyPending()) {
      return;
    }
    stop();
    delete this;
  }

 private:
  friend class IOObserver;

  enum class State { NONE, STARTED, STOPPED };

  void registerEventBase(folly::EventBase& evb) {
    DestructorGuard dg(this);

    connectionHandlers_.withWLock([&](auto& handlerMap) {
      ConnectionHandler::Ptr connectionHandler(
          new ConnectionHandler(evb, connectionFactory_));
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
        handlerIt->second->stopAccepting();
        handlerMap.erase(handlerIt);
      }
    });
  }

  std::atomic<State> state_{State::NONE};
  folly::SocketAddress address_;
  folly::Executor::KeepAlive<folly::IOThreadPoolExecutor> executor_;
  ConnectionFactory connectionFactory_;
  std::shared_ptr<IOObserver> observer_;

  folly::Synchronized<
      folly::F14FastMap<folly::EventBase*, ConnectionHandler::Ptr>>
      connectionHandlers_;
};

} // namespace apache::thrift::fast_thrift::rocket::server::connection
