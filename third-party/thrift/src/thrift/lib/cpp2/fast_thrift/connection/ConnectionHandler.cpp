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

#include <thrift/lib/cpp2/fast_thrift/connection/ConnectionHandler.h>

#include <utility>
#include <vector>

namespace apache::thrift::fast_thrift::connection {

ConnectionHandler::ConnectionHandler(
    folly::EventBase& evb,
    folly::SocketAddress address,
    fast_security::SSLPolicy sslPolicy,
    folly::observer::Observer<std::shared_ptr<const fast_security::TLSParams>>
        tlsParamsObserver,
    SocketOptions socketOptions,
    bool enableReusePortBpfSpread)
    : evb_(folly::getKeepAliveToken(&evb)),
      address_(std::move(address)),
      socketOptions_(socketOptions),
      enableReusePortBpfSpread_(enableReusePortBpfSpread),
      sslPolicy_(sslPolicy),
      tlsParamsObserver_(std::move(tlsParamsObserver)),
      listener_(
          ConnectionListener::Ptr(new ConnectionListener(
              evb_.get(),
              address_,
              socketOptions_,
              enableReusePortBpfSpread_))) {}

ConnectionHandler::~ConnectionHandler() {
  // Defensive: ensure shutdown runs at least once. From the EVB we can't
  // wait for graceful drain (would block the loop that fires close
  // callbacks), so fall back to a synchronous teardown — same as the
  // original behavior of this dtor.
  if (evb_->inRunningEventBaseThread()) {
    stopAcceptingOnEvb();
    closeAllConnectionsOnEvb();
    listener_.reset();
  } else {
    stop();
    evb_->runInEventBaseThreadAndWait([this] { listener_.reset(); });
  }
}

void ConnectionHandler::stop(std::chrono::milliseconds drainTimeout) {
  DCHECK(!evb_->inRunningEventBaseThread())
      << "ConnectionHandler::stop must not be called from the owning EVB; "
      << "the drain wait would block the loop that fires close callbacks";

  // Phase 1: stop accepting new connections.
  evb_->runImmediatelyOrRunInEventBaseThreadAndWait(
      [this] { stopAcceptingOnEvb(); });

  // Phase 2: initiate graceful drain on every live connection.
  evb_->runImmediatelyOrRunInEventBaseThreadAndWait(
      [this] { drainAllOnEvb(); });

  // Phase 3: wait for the drain to complete (close callbacks decrement
  // connections_ and post drainedBaton_ when the last one drops). Off-EVB
  // so the loop is free to fire those callbacks.
  if (!drainedBaton_.try_wait_for(drainTimeout)) {
    XLOG(WARN) << "ConnectionHandler::stop drain timed out after "
               << drainTimeout.count() << "ms; force-closing remaining";
  }

  // Phase 4: force-close any stragglers.
  evb_->runImmediatelyOrRunInEventBaseThreadAndWait(
      [this] { closeAllConnectionsOnEvb(); });
}

void ConnectionHandler::stopAcceptingOnEvb() {
  if (!pipeline_) {
    return;
  }
  listener_->stop();
  listener_->resetPipeline();
  pipeline_.reset();
  installer_.reset();
  listener_.reset();
}

void ConnectionHandler::drainAllOnEvb() {
  draining_.store(true, std::memory_order_release);
  if (connections_.empty()) {
    postDrainedOnce();
    return;
  }
  // Snapshot keys so iteration is safe across re-entrant erases (drain()
  // may fire the close callback synchronously for connection types
  // without async drain work).
  std::vector<uint64_t> ids;
  ids.reserve(connections_.size());
  for (const auto& [id, _] : connections_) {
    ids.push_back(id);
  }
  for (auto id : ids) {
    auto it = connections_.find(id);
    if (it != connections_.end()) {
      it->second.drain();
    }
  }
}

void ConnectionHandler::closeAllConnectionsOnEvb() {
  // Move the map out before tearing down: each close() can synchronously
  // fire its close callback, which re-enters onConnectionClosed and tries
  // to erase its own entry. Reentering an erase on the entry currently
  // being closed yields use-after-free.
  auto victims = std::move(connections_);
  // close() below will fire each connection's close callback, which calls
  // onConnectionClosed and tries to erase from the (now-empty) map. The
  // erase is a no-op, so its `> 0` branch won't run; zero the counter
  // explicitly here to keep it in sync with the moved-out map.
  connectionCount_.store(0, std::memory_order_relaxed);
  for (auto& [_, conn] : victims) {
    conn.close();
  }
}

void ConnectionHandler::onConnectionClosed(uint64_t connId) noexcept {
  if (connections_.erase(connId) > 0) {
    connectionCount_.fetch_sub(1, std::memory_order_relaxed);
  }
  if (draining_.load(std::memory_order_acquire) && connections_.empty()) {
    postDrainedOnce();
  }
}

void ConnectionHandler::postDrainedOnce() noexcept {
  bool expected = false;
  if (drainedPosted_.compare_exchange_strong(expected, true)) {
    drainedBaton_.post();
  }
}

folly::SocketAddress ConnectionHandler::getAddress() const {
  CHECK(listener_) << "ConnectionHandler::getAddress called after stop()";
  return listener_->getAddress();
}

} // namespace apache::thrift::fast_thrift::connection
