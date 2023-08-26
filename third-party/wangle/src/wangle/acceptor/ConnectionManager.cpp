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

#include <wangle/acceptor/ConnectionManager.h>

#include <folly/ConstexprMath.h>
#include <folly/io/async/EventBase.h>
#include <glog/logging.h>
#include <wangle/acceptor/ManagedConnection.h>
#include <chrono>

using std::chrono::milliseconds;

namespace wangle {

ConnectionManager::ConnectionManager(
    folly::EventBase* eventBase,
    milliseconds idleTimeout,
    Callback* callback)
    : callback_(callback),
      eventBase_(eventBase),
      drainIterator_(conns_.end()),
      idleIterator_(conns_.end()),
      drainHelper_(*this),
      idleTimeout_(idleTimeout),
      connectionAgeTimeout_(std::chrono::milliseconds(0)),
      idleConnEarlyDropThreshold_(idleTimeout / 2) {}

ConnectionManager::ConnectionManager(
    folly::EventBase* eventBase,
    milliseconds idleTimeout,
    milliseconds connAgeTimeout,
    Callback* callback)
    : callback_(callback),
      eventBase_(eventBase),
      drainIterator_(conns_.end()),
      idleIterator_(conns_.end()),
      drainHelper_(*this),
      idleTimeout_(idleTimeout),
      connectionAgeTimeout_(connAgeTimeout),
      idleConnEarlyDropThreshold_(idleTimeout / 2) {}

void ConnectionManager::addConnection(
    ManagedConnection* connection,
    bool idleTimeout,
    bool connectionAgeTimeout) {
  CHECK_NOTNULL(connection);
  ConnectionManager* oldMgr = connection->getConnectionManager();
  if (oldMgr != this) {
    if (oldMgr) {
      // 'connection' was being previously managed in a different thread.
      // We must remove it from that manager before adding it to this one.
      oldMgr->removeConnection(connection);
    }

    // put the connection into busy part first.  This should not matter at all
    // because the last callback for an idle connection must be onDeactivated(),
    // so the connection must be moved to idle part then.
    conns_.push_front(*connection);

    connection->setConnectionManager(this);
    if (callback_) {
      callback_->onConnectionAdded(connection);
    }
  }
  if (idleTimeout) {
    scheduleTimeout(connection, idleTimeout_);
  }
  if (connectionAgeTimeout) {
    scheduleTimeout(&connection->connectionAgeTimeout_, connectionAgeTimeout_);
  }

  if (drainHelper_.getShutdownState() >=
          ShutdownState::NOTIFY_PENDING_SHUTDOWN &&
      notifyPendingShutdown_) {
    connection->fireNotifyPendingShutdown();
  }

  if (drainHelper_.getShutdownState() >= ShutdownState::CLOSE_WHEN_IDLE) {
    // closeWhenIdle can delete the connection (it was just created, so it's
    // probably idle).  Delay the closeWhenIdle call until the end of the loop
    // where it will be safer to terminate the conn.
    // Hold a DestructorGuard to the end of the loop for this
    eventBase_->runInLoop([connection,
                           this,
                           cmDg = DestructorGuard(this),
                           connDg = DestructorGuard(connection)] {
      if (connection->listHook_.is_linked()) {
        auto it = conns_.iterator_to(*connection);
        DCHECK(it != conns_.end());
        connection->fireCloseWhenIdle(!notifyPendingShutdown_);
      }
    });
  }
}

void ConnectionManager::scheduleTimeout(
    ConnectionAgeTimeout* connection,
    std::chrono::milliseconds timeout) {
  if (timeout > std::chrono::milliseconds(0)) {
    eventBase_->timer().scheduleTimeout(connection, timeout);
  }
}

void ConnectionManager::scheduleTimeout(
    ManagedConnection* const connection,
    std::chrono::milliseconds timeout) {
  if (timeout > std::chrono::milliseconds(0)) {
    eventBase_->timer().scheduleTimeout(connection, timeout);
  }
}

void ConnectionManager::scheduleTimeout(
    folly::HHWheelTimer::Callback* callback,
    std::chrono::milliseconds timeout) {
  eventBase_->timer().scheduleTimeout(callback, timeout);
}

void ConnectionManager::removeConnection(ManagedConnection* connection) {
  if (connection->getConnectionManager() == this) {
    connection->cancelTimeout();
    connection->setConnectionManager(nullptr);

    // Un-link the connection from our list, being careful to keep the iterator
    // that we're using for idle shedding valid
    auto it = conns_.iterator_to(*connection);
    if (it == drainIterator_) {
      ++drainIterator_;
    }
    if (it == idleIterator_) {
      ++idleIterator_;
    }
    conns_.erase(it);

    if (callback_) {
      callback_->onConnectionRemoved(connection);
      if (getNumConnections() == 0) {
        callback_->onEmpty(*this);
      }
    }
  }
}

void ConnectionManager::initiateGracefulShutdown(
    std::chrono::milliseconds idleGrace) {
  VLOG(3) << this << " initiateGracefulShutdown with nconns=" << conns_.size();
  if (drainHelper_.getShutdownState() != ShutdownState::NONE) {
    VLOG(3) << "Ignoring redundant call to initiateGracefulShutdown";
    return;
  }
  drainHelper_.startDrainAll(idleGrace);
}

void ConnectionManager::drainConnections(
    double pct,
    std::chrono::milliseconds idleGrace) {
  if (drainHelper_.getShutdownState() != ShutdownState::NONE) {
    VLOG(3) << "Ignoring partial drain with full drain in progress";
    return;
  }
  drainHelper_.startDrainPartial(pct, idleGrace);
}

void ConnectionManager::DrainHelper::startDrainPartial(
    double pct,
    std::chrono::milliseconds idleGrace) {
  all_ = false;
  pct_ = pct;
  startDrain(idleGrace);
}

void ConnectionManager::DrainHelper::startDrainAll(
    std::chrono::milliseconds idleGrace) {
  all_ = true;
  pct_ = 1.0;
  if (isScheduled()) {
    // if we are in the middle of a partial, abort and convert to all
    cancelTimeout();
  }
  startDrain(idleGrace);
}

void ConnectionManager::DrainHelper::startDrain(
    std::chrono::milliseconds idleGrace) {
  if (idleGrace.count() > 0) {
    shutdownState_ = ShutdownState::NOTIFY_PENDING_SHUTDOWN;
    scheduleTimeout(idleGrace);
    VLOG(3) << "Scheduling idle grace period of " << idleGrace.count() << "ms";
  } else {
    manager_.notifyPendingShutdown_ = false;
    shutdownState_ = ShutdownState::CLOSE_WHEN_IDLE;
    VLOG(3) << "proceeding directly to closing idle connections";
  }
  manager_.drainIterator_ = drainStartIterator();
  drainConnections();
}

void ConnectionManager::DrainHelper::drainConnections() {
  DestructorGuard g(&manager_);
  size_t numCleared = 0;
  size_t numKept = 0;

  auto it = manager_.drainIterator_;

  CHECK(
      shutdownState_ == ShutdownState::NOTIFY_PENDING_SHUTDOWN ||
      shutdownState_ == ShutdownState::CLOSE_WHEN_IDLE);
  while (it != manager_.conns_.end() && (numKept + numCleared) < 64) {
    ManagedConnection& conn = *it++;
    if (shutdownState_ == ShutdownState::NOTIFY_PENDING_SHUTDOWN) {
      conn.fireNotifyPendingShutdown();
      numKept++;
    } else { // CLOSE_WHEN_IDLE
      // Second time around: close idle sessions. If they aren't idle yet,
      // have them close when they are idle
      if (conn.isBusy()) {
        numKept++;
      } else {
        numCleared++;
      }
      conn.fireCloseWhenIdle(!manager_.notifyPendingShutdown_);
    }
  }

  if (shutdownState_ == ShutdownState::CLOSE_WHEN_IDLE) {
    VLOG(2) << "Idle connections cleared: " << numCleared
            << ", busy conns kept: " << numKept;
  } else {
    VLOG(3) << this << " notified n=" << numKept;
  }
  manager_.drainIterator_ = it;
  if (it != manager_.conns_.end()) {
    manager_.eventBase_->runInLoop(this);
  } else {
    if (shutdownState_ == ShutdownState::NOTIFY_PENDING_SHUTDOWN) {
      VLOG(3) << this << " finished notify_pending_shutdown";
      shutdownState_ = ShutdownState::NOTIFY_PENDING_SHUTDOWN_COMPLETE;
      if (!isScheduled()) {
        // The idle grace timer already fired, start over immediately
        shutdownState_ = ShutdownState::CLOSE_WHEN_IDLE;
        manager_.drainIterator_ = drainStartIterator();
        manager_.eventBase_->runInLoop(this);
      }
    } else {
      shutdownState_ = ShutdownState::CLOSE_WHEN_IDLE_COMPLETE;
    }
  }
}

void ConnectionManager::DrainHelper::idleGracefulTimeoutExpired() {
  VLOG(2) << this << " idleGracefulTimeoutExpired";
  if (shutdownState_ == ShutdownState::NOTIFY_PENDING_SHUTDOWN_COMPLETE) {
    shutdownState_ = ShutdownState::CLOSE_WHEN_IDLE;
    manager_.drainIterator_ = drainStartIterator();
    drainConnections();
  } else {
    VLOG(4) << this
            << " idleGracefulTimeoutExpired during "
               "NOTIFY_PENDING_SHUTDOWN, ignoring";
  }
}

void ConnectionManager::stopDrainingForShutdown() {
  drainHelper_.setShutdownState(ShutdownState::CLOSE_WHEN_IDLE_COMPLETE);
  drainHelper_.cancelTimeout();
}

void ConnectionManager::dropAllConnections() {
  DestructorGuard g(this);

  // Signal the drain helper in case that has not happened before.
  stopDrainingForShutdown();

  // Iterate through our connection list, and drop each connection.
  VLOG_IF(4, conns_.empty()) << "no connections to drop";
  VLOG_IF(2, !conns_.empty()) << "connections to drop: " << conns_.size();

  unsigned i = 0;
  while (!conns_.empty()) {
    ManagedConnection& conn = conns_.front();
    conns_.pop_front();
    conn.cancelTimeout();
    conn.setConnectionManager(nullptr);
    // For debugging purposes, dump information about the first few
    // connections.
    static const unsigned MAX_CONNS_TO_DUMP = 2;
    if (++i <= MAX_CONNS_TO_DUMP) {
      conn.dumpConnectionState(3);
    }
    conn.dropConnection();
  }
  drainIterator_ = conns_.end();
  idleIterator_ = conns_.end();
  drainHelper_.cancelLoopCallback();

  if (callback_) {
    callback_->onEmpty(*this);
  }
}

void ConnectionManager::dropConnections(double pct) {
  DestructorGuard g(this);

  // Signal the drain helper in case that has not happened before.
  stopDrainingForShutdown();

  const size_t N = conns_.size();
  const size_t numToDrop = N * folly::constexpr_clamp(pct, 0., 1.);
  for (size_t i = 0; i < numToDrop && !conns_.empty(); i++) {
    ManagedConnection& conn = conns_.front();
    removeConnection(&conn);
    conn.dropConnection();
  }
}

void ConnectionManager::dropEstablishedConnections(
    double pct,
    const std::function<bool(ManagedConnection*)>& filter) {
  const size_t N = conns_.size();
  auto front = conns_.iterator_to(conns_.front());
  if (idleIterator_ == front || N == 0) {
    return;
  }
  const size_t numToDrop = N * folly::constexpr_clamp(pct, 0., 1.);
  size_t droppedConns = 0;
  auto it = --idleIterator_;

  bool last{false};
  while (!conns_.empty() && droppedConns < numToDrop) {
    // We are traversing linked list from middle to the left towards front
    // we want to know when we reach the begining of the list.
    last = it == front;

    ManagedConnection& conn = *(it--);
    if (filter(&conn)) {
      conn.dropConnection();
      droppedConns++;
    }

    if (last) {
      return;
    }
  }
}

void ConnectionManager::reportActivity(ManagedConnection& conn) {
  conn.reportActivity();
  onActivated(conn);
}

void ConnectionManager::onActivated(ManagedConnection& conn) {
  auto it = conns_.iterator_to(conn);
  if (it == idleIterator_) {
    idleIterator_++;
  }
  conns_.erase(it);
  conns_.push_front(conn);
}

void ConnectionManager::onDeactivated(ManagedConnection& conn) {
  auto it = conns_.iterator_to(conn);
  bool moveDrainIter = false;
  if (it == drainIterator_) {
    drainIterator_++;
    moveDrainIter = true;
  }
  conns_.erase(it);
  conns_.push_back(conn);
  if (idleIterator_ == conns_.end()) {
    idleIterator_--;
  }
  if (moveDrainIter && drainIterator_ == conns_.end()) {
    drainIterator_--;
  }
}

/**
 * Note that the idle ones are organized in the decreasing idle time order
 */
size_t ConnectionManager::dropIdleConnections(size_t num) {
  VLOG(4) << "attempt to drop " << num << " idle connections";
  if (idleConnEarlyDropThreshold_ >= idleTimeout_) {
    return 0;
  }

  size_t count = 0;
  while (count < num && idleIterator_ != conns_.end()) {
    auto it = idleIterator_;
    auto idleTimeMs = it->getIdleTime();
    if (idleTimeMs == std::chrono::milliseconds(0) ||
        idleTimeMs <= idleConnEarlyDropThreshold_) {
      VLOG(4) << "conn's idletime: " << idleTimeMs.count()
              << ", in-activity threshold: "
              << idleConnEarlyDropThreshold_.count() << ", dropped " << count
              << "/" << num;
      return count;
    }
    ManagedConnection& conn = *it;
    idleIterator_++;
    conn.dropConnection();
    count++;
  }

  return count;
}

size_t ConnectionManager::dropIdleConnectionsBasedOnTimeout(
    std::chrono::milliseconds targetIdleTimeMs,
    const std::function<void(size_t)>& droppedConnectionsCB) {
  VLOG(4)
      << "attempt to drop all the connections for which idle time is greater or equal to "
      << targetIdleTimeMs.count();

  // Idle connection are sorted in decreasing order from left to right.
  size_t count = 0;
  while (idleIterator_ != conns_.end()) {
    auto idleTimeMs = idleIterator_->getIdleTime();
    if (idleTimeMs <= targetIdleTimeMs) {
      VLOG(4) << "conn's idletime: " << idleTimeMs.count()
              << ", in-activity threshold: " << targetIdleTimeMs.count()
              << ", dropped " << count << "/" << count;
      break;
    }
    ManagedConnection& conn = *idleIterator_;
    idleIterator_++;
    conn.dropConnection();
    count++;
  }
  droppedConnectionsCB(count);
  return count;
}

} // namespace wangle
