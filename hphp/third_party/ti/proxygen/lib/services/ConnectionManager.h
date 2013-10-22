// Copyright 2004-present Facebook.  All rights reserved.
#pragma once
#include "ti/proxygen/lib/services/ManagedConnection.h"

#include "thrift/lib/cpp/async/TAsyncTimeoutSet.h"
#include "thrift/lib/cpp/async/TEventBase.h"

#include <chrono>

namespace facebook { namespace proxygen {

/**
 * A ConnectionManager keeps track of ManagedConnections.
 */
class ConnectionManager {
 public:

  /**
   * Interface for an optional observer that's notified about
   * various events in a ConnectionManager
   */
  class Callback {
  public:
    virtual ~Callback() {}

    /**
     * Invoked when the number of connections managed by the
     * ConnectionManager changes from nonzero to zero.
     */
    virtual void onEmpty(const ConnectionManager& cm) = 0;

    /**
     * Invoked when a connection is added to the ConnectionManager.
     */
    virtual void onConnectionAdded(const ConnectionManager& cm) = 0;

    /**
     * Invoked when a connection is removed from the ConnectionManager.
     */
    virtual void onConnectionRemoved(const ConnectionManager& cm) = 0;
  };

  ConnectionManager(apache::thrift::async::TEventBase* eventBase,
      std::chrono::milliseconds timeout, Callback* callback = nullptr);

  /**
   * Add a connection to the set of connections managed by this
   * ConnectionManager.
   *
   * @param connection     The connection to add.
   * @param timeout        Whether to immediately register this connection
   *                         for an idle timeout callback.
   */
  void addConnection(ManagedConnection* connection,
      bool timeout = false);

  /**
   * Schedule a timeout callback for a connection.
   */
  void scheduleTimeout(ManagedConnection* connection);

  /**
   * Remove a connection from this ConnectionManager and, if
   * applicable, cancel the pending timeout callback that the
   * ConnectionManager has scheduled for the connection.
   *
   * @note This method does NOT destroy the connection.
   */
  void removeConnection(ManagedConnection* connection);

  /**
   * Destroy all connections managed by this ConnectionManager that
   * are currently idle, as determined by a call to each ManagedConnection's
   * isBusy() method.
   */
  void closeIdleConnections();

  /**
   * Destroy all connections Managed by this ConnectionManager, even
   * the ones that are busy.
   */
  void dropAllConnections();

  size_t getNumConnections() const { return conns_.size(); }

 private:
  class CloseIdleConnsCallback :
    public apache::thrift::async::TEventBase::LoopCallback {
   public:
    explicit CloseIdleConnsCallback(ConnectionManager* manager)
      : manager_(manager) {}

    virtual void runLoopCallback() noexcept {
      manager_->closeIdleConnections();
    }

   private:
    ConnectionManager* manager_;
  };

  ConnectionManager(const ConnectionManager&) = delete;
  ConnectionManager& operator=(ConnectionManager&) = delete;

  /** All connections */
  folly::CountedIntrusiveList<
    ManagedConnection,&ManagedConnection::listHook_> conns_;

  /** Connections that currently are registered for timeouts */
  apache::thrift::async::TAsyncTimeoutSet::UniquePtr connTimeouts_;

  /** Optional callback to notify of state changes */
  Callback* callback_;

  /** Event base in which we run */
  apache::thrift::async::TEventBase *eventBase_;

  /** Iterator to the next connection to shed; used by closeIdleConnections() */
  folly::CountedIntrusiveList<
    ManagedConnection,&ManagedConnection::listHook_>::iterator idleIterator_;
  CloseIdleConnsCallback idleLoopCallback_;
};

}} // facebook::proxygen

