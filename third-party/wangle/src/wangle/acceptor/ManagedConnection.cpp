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

#include <wangle/acceptor/ManagedConnection.h>

#include <wangle/acceptor/ConnectionManager.h>

namespace wangle {

ManagedConnection::ManagedConnection()
    : connectionManager_(nullptr),
      connectionAgeTimeout_{*this},
      creationTime_{std::chrono::steady_clock::now()} {}

ManagedConnection::~ManagedConnection() {
  if (connectionManager_) {
    connectionManager_->removeConnection(this);
  }
  connectionAgeTimeout_.cancelTimeout();
}

void ManagedConnection::resetTimeout() {
  if (connectionManager_) {
    resetTimeoutTo(connectionManager_->getDefaultTimeout());
  }
}

void ManagedConnection::resetTimeoutTo(std::chrono::milliseconds timeout) {
  if (connectionManager_) {
    connectionManager_->scheduleTimeout(this, timeout);
  }
}

void ManagedConnection::scheduleTimeout(
    folly::HHWheelTimer::Callback* callback,
    std::chrono::milliseconds timeout) {
  if (connectionManager_) {
    connectionManager_->scheduleTimeout(callback, timeout);
  }
}

void ManagedConnection::reportActivity() {
  latestActivity_ = std::chrono::steady_clock::now();
}

folly::Optional<std::chrono::milliseconds>
ManagedConnection::getLastActivityElapsedTime() const {
  if (latestActivity_) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - *latestActivity_);
  } else {
    return folly::none;
  }
}

void ConnectionAgeTimeout::timeoutExpired() noexcept {
  if (auto connectionManager = connection_.getConnectionManager()) {
    connectionManager->removeConnection(&connection_);
  }
  connection_.closeWhenIdle();
}

////////////////////// Globals /////////////////////

std::ostream& operator<<(std::ostream& os, const ManagedConnection& conn) {
  conn.describe(os);
  return os;
}

} // namespace wangle
