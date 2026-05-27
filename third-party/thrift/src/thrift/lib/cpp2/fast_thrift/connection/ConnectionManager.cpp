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

#include <thrift/lib/cpp2/fast_thrift/connection/ConnectionManager.h>

#include <utility>
#include <vector>

namespace apache::thrift::fast_thrift::connection {

ConnectionManager::Ptr ConnectionManager::create(
    folly::SocketAddress address,
    folly::Executor::KeepAlive<folly::IOThreadPoolExecutorBase> executor,
    fast_security::SSLPolicy sslPolicy,
    std::shared_ptr<const fizz::server::FizzServerContext> fizzContext,
    std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams,
    std::optional<std::chrono::milliseconds> tlsHandshakeTimeout,
    SocketOptions socketOptions) {
  return Ptr(new ConnectionManager(
      std::move(address),
      std::move(executor),
      sslPolicy,
      std::move(fizzContext),
      std::move(thriftParams),
      tlsHandshakeTimeout,
      socketOptions));
}

ConnectionManager::ConnectionManager(
    folly::SocketAddress address,
    folly::Executor::KeepAlive<folly::IOThreadPoolExecutorBase> executor,
    fast_security::SSLPolicy sslPolicy,
    std::shared_ptr<const fizz::server::FizzServerContext> fizzContext,
    std::shared_ptr<apache::thrift::ThriftParametersContext> thriftParams,
    std::optional<std::chrono::milliseconds> tlsHandshakeTimeout,
    SocketOptions socketOptions)
    : address_(std::move(address)),
      executor_(std::move(executor)),
      sslPolicy_(sslPolicy),
      fizzContext_(std::move(fizzContext)),
      thriftParams_(std::move(thriftParams)),
      tlsHandshakeTimeout_(tlsHandshakeTimeout),
      socketOptions_(socketOptions),
      observer_(std::make_shared<IOObserver>(*this)) {}

void ConnectionManager::start() {
  CHECK(configureHandler_)
      << "ConnectionManager::start called before setConnectionFactory";
  DCHECK(state_.load() != State::STARTED);
  // Set STARTED before addObserver — addObserver synchronously invokes
  // registerEventBase on every IO thread, and that path gates on state_.
  state_.store(State::STARTED);
  executor_->addObserver(observer_);
}

void ConnectionManager::stop(std::chrono::milliseconds drainTimeout) {
  State expected = State::STARTED;
  if (!state_.compare_exchange_strong(expected, State::STOPPED)) {
    return;
  }
  // Snapshot handler pointers under the rlock so we can drive each
  // through its full shutdown without holding the lock across EVB hops.
  std::vector<std::pair<folly::EventBase*, ConnectionHandler*>> snapshot;
  handlers_.withRLock([&](const auto& map) {
    snapshot.reserve(map.size());
    for (const auto& [evb, handler] : map) {
      snapshot.emplace_back(evb, handler.get());
    }
  });
  for (auto& [_, handler] : snapshot) {
    // handler->stop() bounces to its EVB for the synchronous phases and
    // waits off-EVB for the drain — safe to call from this thread.
    handler->stop(drainTimeout);
  }
  // Drop the IOObserver last: unregisterEventBase only erases the map
  // entry (the handler is already torn down) so it's cheap.
  executor_->removeObserver(observer_);
}

ConnectionManager::~ConnectionManager() {
  stop();
}

folly::SocketAddress ConnectionManager::getAddress() const {
  folly::SocketAddress address;
  handlers_.withRLock([&](const auto& map) {
    if (!map.empty()) {
      address = map.begin()->second->getAddress();
    }
  });
  return address;
}

void ConnectionManager::registerEventBase(folly::EventBase& evb) {
  DestructorGuard dg(this);

  // After stop() / dtor, refuse to spin up listeners on EVBs the
  // executor adds during shutdown.
  if (state_.load(std::memory_order_acquire) != State::STARTED) {
    return;
  }

  auto handler = std::make_unique<ConnectionHandler>(
      evb,
      address_,
      sslPolicy_,
      fizzContext_,
      thriftParams_,
      tlsHandshakeTimeout_,
      socketOptions_,
      enableReusePortBpfSpread_);
  // Build the pipeline using the stored factory + onAccept; pulls the
  // template-instantiated machinery out of ConnectionManager.
  configureHandler_(*handler);

  handlers_.withWLock([&](auto& map) {
    auto [_, inserted] = map.emplace(&evb, std::move(handler));
    if (!inserted) {
      LOG(FATAL) << "EventBase already registered";
    }
  });
}

void ConnectionManager::unregisterEventBase(folly::EventBase& evb) {
  DestructorGuard dg(this);

  // Contract: caller has already driven the handler through stop(); we
  // just drop the map entry.
  handlers_.withWLock([&](auto& map) { map.erase(&evb); });
}

} // namespace apache::thrift::fast_thrift::connection
