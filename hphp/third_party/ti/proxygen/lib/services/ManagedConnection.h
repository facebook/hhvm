// Copyright 2004-present Facebook.  All rights reserved.
#pragma once

#include "folly/IntrusiveList.h"
#include "thrift/lib/cpp/async/TAsyncTimeoutSet.h"
#include "thrift/lib/cpp/async/TDelayedDestruction.h"
#include "thrift/lib/cpp/transport/TSocketAddress.h"

#include <ostream>

namespace facebook { namespace proxygen {

class ConnectionManager;

/**
 * Interface describing a connection that can be managed by a
 * container such as an Acceptor.
 */
class ManagedConnection:
    public apache::thrift::async::TAsyncTimeoutSet::Callback,
    public apache::thrift::async::TDelayedDestruction {

 public:
  ManagedConnection();

  // TAsyncTimeoutSet::Callback API (left for subclasses to implement).
  virtual void timeoutExpired() noexcept = 0;

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
   * Notify a connection that a shutdown is pending.
   */
  virtual void notifyPendingShutdown() = 0;

  /**
   * Forcibly drop a connection.
   *
   * If a request is in progress, this should cause the connection to be
   * closed with a reset.
   */
  virtual void dropConnection() = 0;

  /**
   * Dump the state of the connection to the log
   */
  virtual void dumpConnectionState(uint8_t loglevel) = 0;

  /**
   * If the connection has a connection manager, reset the timeout
   * countdown.
   * @note If the connection manager doesn't have the connection scheduled
   *       for a timeout already, this method will schedule one.  If the
   *       connection manager does have the connection connection scheduled
   *       for a timeout, this method will push back the timeout to N msec
   *       from now, where N is the connection manager's timer interval.
   */
  virtual void resetTimeout();

  ConnectionManager* getConnectionManager() {
    return connectionManager_;
  }

 protected:
  virtual ~ManagedConnection();

 private:
  friend class ConnectionManager;

  void setConnectionManager(ConnectionManager* mgr) {
    connectionManager_ = mgr;
  }

  ConnectionManager* connectionManager_;

  folly::SafeIntrusiveListHook listHook_;
};

std::ostream& operator<<(std::ostream& os, const ManagedConnection& conn);

}} // facebook::proxygen

