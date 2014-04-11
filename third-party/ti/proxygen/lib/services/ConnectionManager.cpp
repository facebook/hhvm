// Copyright 2004-present Facebook.  All rights reserved.
#include "ti/proxygen/lib/services/ConnectionManager.h"

#include "thrift/lib/cpp/async/TEventBase.h"

using apache::thrift::async::TAsyncTimeoutSet;
using apache::thrift::async::TEventBase;
using std::chrono::milliseconds;

namespace facebook { namespace proxygen {

ConnectionManager::ConnectionManager(TEventBase* eventBase,
    milliseconds timeout, Callback* callback)
  : connTimeouts_(new TAsyncTimeoutSet(eventBase, timeout)),
    callback_(callback),
    eventBase_(eventBase),
    idleIterator_(conns_.end()),
    idleLoopCallback_(this) {

}

void
ConnectionManager::addConnection(ManagedConnection* connection,
    bool timeout) {
  CHECK_NOTNULL(connection);
  ConnectionManager* oldMgr = connection->getConnectionManager();
  if (oldMgr != this) {
    if (oldMgr) {
      // 'connection' was being previously managed in a different thread.
      // We must remove it from that manager before adding it to this one.
      oldMgr->removeConnection(connection);
    }
    conns_.push_back(*connection);
    connection->setConnectionManager(this);
    if (callback_) {
      callback_->onConnectionAdded(*this);
    }
  }
  if (timeout) {
    scheduleTimeout(connection);
  }
}

void
ConnectionManager::scheduleTimeout(ManagedConnection* connection) {
  connTimeouts_->scheduleTimeout(connection);
}

void
ConnectionManager::removeConnection(ManagedConnection* connection) {
  if (connection->getConnectionManager() == this) {
    connection->cancelTimeout();
    connection->setConnectionManager(nullptr);

    // Un-link the connection from our list, being careful to keep the iterator
    // that we're using for idle shedding valid
    auto it = conns_.iterator_to(*connection);
    if (it == idleIterator_) {
      ++idleIterator_;
    }
    conns_.erase(it);

    if (callback_) {
      callback_->onConnectionRemoved(*this);
      if (getNumConnections() == 0) {
        callback_->onEmpty(*this);
      }
    }
  }
}

void
ConnectionManager::closeIdleConnections() {
  size_t numCleared = 0;
  size_t numKept = 0;

  auto it = conns_.begin();
  if (idleIterator_ != conns_.end()) {
    it = idleIterator_;
  }

  while (it != conns_.end() && numCleared < 64) {
    ManagedConnection& conn = *it++;
    if (conn.isBusy()) {
      conn.notifyPendingShutdown();
      numKept++;
    } else {
      conn.cancelTimeout();
      conn.timeoutExpired();
      numCleared++;
    }
  }

  VLOG(2) << "Idle connections cleared: " << numCleared <<
      ", busy conns kept: " << numKept;

  if (it != conns_.end()) {
    idleIterator_ = it;
    eventBase_->runInLoop(&idleLoopCallback_);
  }
}

void
ConnectionManager::dropAllConnections() {
  // Iterate through our connection list, and drop each connection.
  VLOG(3) << "connections to drop: " << conns_.size();
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
  idleIterator_ = conns_.end();
  idleLoopCallback_.cancelLoopCallback();

  if (callback_) {
    callback_->onEmpty(*this);
  }
}

}} // facebook::proxygen
