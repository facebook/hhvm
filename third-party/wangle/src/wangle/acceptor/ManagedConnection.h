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

#include <folly/IntrusiveList.h>
#include <folly/SocketAddress.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/HHWheelTimer.h>

#include <ostream>
#include <string>

namespace wangle {

class ConnectionManager;
class ManagedConnection;

class ConnectionAgeTimeout : public folly::HHWheelTimer::Callback {
 public:
  void timeoutExpired() noexcept override;
  explicit ConnectionAgeTimeout(ManagedConnection& connection)
      : connection_{connection} {}

 private:
  ManagedConnection& connection_;
};
/**
 * Interface describing a connection that can be managed by a
 * container such as an Acceptor.
 */
class ManagedConnection : public folly::HHWheelTimer::Callback,
                          public folly::DelayedDestruction {
 public:
  /**
   * ManagedConnection can be in two state either it's active or idle.
   * This is in connection with ConnectionManager which based on activy
   * moves connections between two state.
   */
  enum ActivationState { ACTIVE, IDLE };
  ManagedConnection();

  class Callback {
   public:
    virtual ~Callback() = default;

    /* Invoked when this connection becomes busy */
    virtual void onActivated(ManagedConnection& conn) = 0;

    /* Invoked when a connection becomes idle */
    virtual void onDeactivated(ManagedConnection& conn) = 0;
  };

  // HHWheelTimer::Callback API (left for subclasses to implement).
  void timeoutExpired() noexcept override = 0;

  /**
   * Print a human-readable description of the connection.
   * @param os Destination stream.
   */
  virtual void describe(std::ostream& os) const = 0;

  /**
   * Check whether the connection has any requests outstanding.
   */
  virtual bool isBusy() const = 0;

  /**
   * Returns socket address of the peer
   */
  [[nodiscard]] virtual const folly::SocketAddress& getPeerAddress()
      const noexcept = 0;

  /**
   * Get the idle time of the connection. If it returning 0, that means this
   * idle connection will never be dropped during pre load shedding stage.
   */
  virtual std::chrono::milliseconds getIdleTime() const {
    return std::chrono::milliseconds(0);
  }

  /**
   * Notify the connection that a shutdown is pending. This method will be
   * called at the beginning of graceful shutdown.
   */
  virtual void notifyPendingShutdown() = 0;

  void fireNotifyPendingShutdown() {
    if (state_ == DrainState::NONE) {
      state_ = DrainState::SENT_NOTIFY_PENDING_SHUTDOWN;
      notifyPendingShutdown();
    }
  }

  /**
   * Instruct the connection that it should shutdown as soon as it is
   * safe. This is called after notifyPendingShutdown().
   */
  virtual void closeWhenIdle() = 0;

  void fireCloseWhenIdle(bool force_to_close = false) {
    if (force_to_close || state_ == DrainState::SENT_NOTIFY_PENDING_SHUTDOWN) {
      state_ = DrainState::SENT_CLOSE_WHEN_IDLE;
      closeWhenIdle();
    }
  }

  /**
   * Forcibly drop a connection.
   *
   * If a request is in progress, this should cause the connection to be
   * closed with a reset.
   */
  virtual void dropConnection(const std::string& errorMsg = "") = 0;

  /**
   * Dump the state of the connection to the log
   */
  virtual void dumpConnectionState(uint8_t loglevel) = 0;

  /**
   * If the connection has a connection manager, reset the timeout countdown to
   * connection manager's default timeout.
   * @note If the connection manager doesn't have the connection scheduled
   *       for a timeout already, this method will schedule one.  If the
   *       connection manager does have the connection connection scheduled
   *       for a timeout, this method will push back the timeout to N msec
   *       from now, where N is the connection manager's timer interval.
   */
  virtual void resetTimeout();

  /**
   * If the connection has a connection manager, reset the timeout countdown to
   * user specified timeout.
   */
  void resetTimeoutTo(std::chrono::milliseconds);

  // Schedule an arbitrary timeout on the HHWheelTimer
  virtual void scheduleTimeout(
      folly::HHWheelTimer::Callback* callback,
      std::chrono::milliseconds timeout);

  ConnectionManager* getConnectionManager() {
    return connectionManager_;
  }

  virtual void reportActivity();

  virtual folly::Optional<std::chrono::milliseconds>
  getLastActivityElapsedTime() const;

  [[nodiscard]] std::chrono::steady_clock::time_point getCreationTime() const {
    return creationTime_;
  }

  void setActivationState(ManagedConnection::ActivationState state) {
    activationState_ = state;
  }

  const ManagedConnection::ActivationState& getActivationState() const {
    return activationState_;
  }

 protected:
  ~ManagedConnection() override;

 private:
  enum class DrainState {
    NONE,
    SENT_NOTIFY_PENDING_SHUTDOWN,
    SENT_CLOSE_WHEN_IDLE,
  };

  DrainState state_{DrainState::NONE};

  friend class ConnectionManager;

  void setConnectionManager(ConnectionManager* mgr) {
    connectionManager_ = mgr;
  }

  ConnectionManager* connectionManager_;
  ConnectionAgeTimeout connectionAgeTimeout_;
  folly::Optional<std::chrono::steady_clock::time_point> latestActivity_;

  const std::chrono::steady_clock::time_point creationTime_;

  folly::SafeIntrusiveListHook listHook_;

  // When connection is created we can assume it to be in active state.
  // it will only later can be moved to idle state.
  ActivationState activationState_{ActivationState::IDLE};
};

std::ostream& operator<<(std::ostream& os, const ManagedConnection& conn);

} // namespace wangle
