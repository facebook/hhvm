/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

//
// The Asynchronous MySQL Client, a high-performance, nonblocking
// client for MySQL.
//
// This client exposes a fully asynchronous MySQL interface.  With it,
// you can connect and run queries simultaneously across multiple
// databases without creating threads.
//
// The interface itself is split across multiple classes:
//
// AsyncMysqlClient - the client itself.  This client manages connections
//   to *multiple* databases.  In general, one needs only one client,
//   regardless of the number of databases connected to.  When in doubt,
//   simply use AsyncMysqlClient::defaultClient rather than constructing
//   your own.  All methods of AsyncMysqlClient are thread safe; however,
//   resulting Operations should not be shared across threads.
//
// Connection - a representation of a living, active MySQL connection.
//   Returned by a successful ConnectOperation (see below).
//
// Operation / ConnectOperation / QueryOperation / MultiQueryOperation
//   - these are the  primary ways of interacting with MySQL databases.
//   Operations represent a pending or completed MySQL action such as
//   connecting or performing a query.  Operations are returned when
//   queries or connections are begun, and can be waited for.  Alternatively,
//   callbacks can be associated with operations.
//
// QueryResult - holds the result data of a query and provides simple ways to
//   to process it.
//
// RowBlock - this is the buffer rows are returned in.  Rather than a
//   row at a time, data from MySQL comes in blocks.  RowBlock is an
//   efficient representation of this, and exposes methods to interact
//   with the contained rows and columns.
//
// For more detail and examples, please see the README file.

#pragma once

#include <folly/Exception.h>
#include <folly/Portability.h>
#include <folly/futures/Future.h>
#include <folly/io/async/EventBase.h>
#include <folly/ssl/OpenSSLPtrTypes.h>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

#include "squangle/logger/DBEventCounter.h"
#include "squangle/logger/DBEventLogger.h"
#include "squangle/mysql_client/Connection.h"
#include "squangle/mysql_client/ConnectionHolder.h"
#include "squangle/mysql_client/DbResult.h"
#include "squangle/mysql_client/MysqlClientBase.h"
#include "squangle/mysql_client/Operation.h"
#include "squangle/mysql_client/Query.h"
#include "squangle/mysql_client/mysql_protocol/MysqlConnection.h"

namespace facebook::common::mysql_client {

class AsyncConnection;
class AsyncMysqlClient;
class SyncMysqlClient;
class Operation;
class ConnectOperation;
class ConnectionKey;

// The client itself.  As mentioned above, in general, it isn't
// necessary to create a client; instead, simply call defaultClient()
// and use the client it returns, which is shared process-wide.
class AsyncMysqlClient : public MysqlClientBase {
 public:
  // Having this type (`uses_one_thread`) tells the pool storage later that we
  // don't need synchronization - see PoolStorage.h
  using uses_one_thread = void;

  AsyncMysqlClient();
  ~AsyncMysqlClient() override;

  // Don't allow copy or move
  AsyncMysqlClient(const AsyncMysqlClient&) = delete;
  AsyncMysqlClient& operator=(const AsyncMysqlClient&) = delete;

  AsyncMysqlClient(AsyncMysqlClient&&) = delete;
  AsyncMysqlClient& operator=(AsyncMysqlClient&&) = delete;

  static std::unique_ptr<db::SquangleLoggerBase> adjustLogger(
      std::unique_ptr<db::SquangleLoggerBase> logger) {
    if (logger) {
      logger->setLoggingPrefix("cpp_async");
    }
    return logger;
  }

  std::unique_ptr<Connection> createConnection(
      std::shared_ptr<const ConnectionKey> conn_key) override;

  static void deleter(AsyncMysqlClient* client) {
    // If we are dying in the thread we own, spin up a new thread to
    // call delete. This allows the asyncmysql thread to terminate safely
    if (std::this_thread::get_id() == client->threadId()) {
      std::thread myThread{[client]() { delete client; }};
      // @lint-ignore CLANGTIDY facebook-hte-BadCall-detach
      myThread.detach();
    } else {
      delete client;
    }
  }

  static std::shared_ptr<AsyncMysqlClient> defaultClient();

  [[nodiscard]] folly::SemiFuture<ConnectResult> connectSemiFuture(
      const std::string& host,
      int port,
      const std::string& database_name,
      const std::string& user,
      const std::string& password,
      const ConnectionOptions& conn_opts = ConnectionOptions());

  // Synchronous call to acquire a connection, the caller thread will be blocked
  // until the operation has finished.
  // In case the we fail to acquire the connection, MysqlException will be
  // thrown.
  [[nodiscard]] std::unique_ptr<Connection> connect(
      const std::string& host,
      int port,
      const std::string& database_name,
      const std::string& user,
      const std::string& password,
      const ConnectionOptions& conn_opts = ConnectionOptions());

  // Stop accepting new queries and connections.
  void blockIncomingOperations() {
    pending_.wlock()->block_operations = true;
  }

  // Do not call this from inside the AsyncMysql thread
  void shutdownClient();

  // Drain any remaining operations.  If also_block_operations is true, then
  // any attempt to add operations during or after this drain will
  // fail harshly.
  void drain(bool also_block_operations);

  folly::EventBase* getEventBase() override {
    return &event_base_;
  }

  const folly::EventBase* getEventBase() const override {
    return &event_base_;
  }

  const std::thread::id threadId() const {
    return thread_.get_id();
  }

  void setPoolsConnectionLimit(uint64_t limit) {
    pools_conn_limit_.store(limit, std::memory_order_relaxed);
  }

  uint64_t getPoolsConnectionLimit() {
    return pools_conn_limit_.load(std::memory_order_relaxed);
  }

  db::ClientPerfStats collectPerfStats() {
    db::ClientPerfStats ret;
    ret.callbackDelayMicrosAvg = stats_tracker_->callbackDelayAvg.value();
    ret.ioEventLoopMicrosAvg = getEventBase()->getAvgLoopTime();
    ret.notificationQueueSize = getEventBase()->getNotificationQueueSize();
    ret.ioThreadBusyTime = stats_tracker_->ioThreadBusyTime.value();
    ret.ioThreadIdleTime = stats_tracker_->ioThreadIdleTime.value();
    return ret;
  }

  bool supportsLocalFiles() override {
    // The async client does not yet support local files for LOAD DATA
    return false;
  }

  virtual std::string_view getBaseClient() const noexcept override {
    return "AsyncMysqlClient";
  }

 protected:
  AsyncMysqlClient(
      std::unique_ptr<db::SquangleLoggerBase> db_logger,
      std::unique_ptr<db::DBCounterBase> db_stats,
      std::unique_ptr<const MysqlExceptionBuilder> exception_builder = nullptr);

  bool runInThread(std::function<void()>&& fn, bool wait = false) override;

  db::SquangleLoggingData makeSquangleLoggingData(
      std::shared_ptr<const ConnectionKey> connKey,
      const db::ConnectionContextBase* connContext) override;

  void activeConnectionAdded(
      std::shared_ptr<const ConnectionKey> key) override {
    std::unique_lock<std::mutex> l(counters_mutex_);
    ++active_connection_counter_;
    ++connection_references_[key];
  }

  // Called in ConnectionHolder and ConnectOperation. The ref count should
  // be in incremented when a connection exists or is about to exist.
  // ConnectOperation decrements it when the connection is acquired.
  // ConnectionHolder is counted during its lifetime.
  void activeConnectionRemoved(
      std::shared_ptr<const ConnectionKey> key) override {
    std::unique_lock<std::mutex> l(counters_mutex_);
    // Sanity check, if the old value was 0, then the counter overflowed
    DCHECK(active_connection_counter_ != 0);
    --active_connection_counter_;
    if (active_connection_counter_ == 0) {
      active_connections_closed_cv_.notify_one();
    }

    auto ref_iter = connection_references_.find(key);
    DCHECK(ref_iter != connection_references_.end());

    if (--ref_iter->second == 0) {
      connection_references_.erase(ref_iter);
    }
  }

  // Add a pending operation to the client.
  void addOperation(std::shared_ptr<Operation> op) override {
    pending_.withWLock([op = std::move(op)](auto& pending) mutable {
      if (pending.block_operations) {
        LOG(ERROR) << "Attempt to start operation when client is shutting down";
        op->cancel();
      }
      pending.operations.insert(std::move(op));
    });
  }

  // We remove operations from pending_operations_ after an iteration
  // of the event loop to ensure we don't delete an object that is
  // executing one of its methods (ie handling an event or cancel
  // call).
  void deferRemoveOperation(Operation* op) override {
    pending_.withWLock([&](auto& pending) {
      auto sptr = op->getSharedPointer();
      // Make sure the pending operation is still in the list - it might not be
      // if the operation was drained.
      if (pending.operations.contains(sptr)) {
        // If the queue to remove is empty, schedule a cleanup to occur after
        // this pass through the event loop.
        if (pending.to_remove.empty()) {
          if (!runInThread([&]() { cleanupCompletedOperations(); })) {
            LOG(DFATAL)
                << "Operation could not be cleaned: error in folly::EventBase";
          }
        }

        pending.to_remove.insert(std::move(sptr));
      }
    });
  }

  std::unique_ptr<ConnectOperationImpl> createConnectOperationImpl(
      MysqlClientBase* client,
      std::shared_ptr<const ConnectionKey> conn_key) const override;

  std::unique_ptr<FetchOperationImpl> createFetchOperationImpl(
      std::unique_ptr<OperationBase::ConnectionProxy> conn,
      LoggingFuncsPtr logging_funcs) const override;

  std::unique_ptr<SpecialOperationImpl> createSpecialOperationImpl(
      std::unique_ptr<OperationBase::ConnectionProxy> conn) const override;

  bool isInCorrectThread(bool expectMysqlThread) const override {
    auto eb = getEventBase();
    return expectMysqlThread == (eb == nullptr || eb->isInEventBaseThread());
  }

 private:
  // Private methods, primarily used by Operations and its subclasses.
  friend class AsyncConnectionPool;
  template <typename Client>
  friend class ConnectionPool;

  void init();

  // Gives the number of connections being created (started) and the ones that
  // are already open for a ConnectionKey
  uint32_t numStartedAndOpenConnections(
      std::shared_ptr<const ConnectionKey> conn_key) {
    std::unique_lock<std::mutex> l(counters_mutex_);
    return connection_references_[conn_key];
  }

  // Similar to the above function, but returns the total number of connections
  // being and already opened.
  uint32_t numStartedAndOpenConnections() override {
    std::unique_lock<std::mutex> l(counters_mutex_);
    return active_connection_counter_;
  }

  Duration callbackDelayAvg() const override {
    return Duration((uint64_t)stats_tracker_->callbackDelayAvg.value());
  }

  void cleanupCompletedOperations();

  // event base running the event loop
  folly::EventBase event_base_;

  // thread_ is where loop() runs and most of the class does its work.
  std::thread thread_;

  struct PendingOperations {
    // The client must keep a reference (via a shared_ptr) to any active
    // Operation as the op's creator may have released their reference.
    // We do this via a map of shared_ptr's, where the keys are raw
    // pointers.
    std::unordered_set<std::shared_ptr<Operation>> operations;
    // See comment for deferRemoveOperation.
    std::unordered_set<std::shared_ptr<Operation>> to_remove;
    // Are we accepting new connections
    bool block_operations{false};
  };

  folly::Synchronized<PendingOperations> pending_;

  // Used to guard thread destruction
  std::atomic<bool> is_shutdown_{false};

  // We count the number of references we have from Connections and
  // ConnectionOperations.  This is used for draining and destruction;
  // ~AsyncMysqlClient blocks until this value becomes zero.
  uint32_t active_connection_counter_ = 0;
  std::unordered_map<std::shared_ptr<const ConnectionKey>, uint32_t>
      connection_references_;
  // Protects the look ups and writes to both counters
  std::mutex counters_mutex_;
  std::condition_variable active_connections_closed_cv_;

  // For testing purposes
  bool delicate_connection_failure_ = false;

  // This only works if you are using AsyncConnectionPool
  std::atomic<uint64_t> pools_conn_limit_;

  class StatsTracker : public folly::EventBaseObserver {
   public:
    void loopSample(int64_t busy_time, int64_t idle_time) override {
      ioThreadBusyTime.addSample(static_cast<double>(busy_time));
      ioThreadIdleTime.addSample(static_cast<double>(idle_time));
    }

    uint32_t getSampleRate() const override {
      // Avoids this being called every loop
      return 16;
    }

    // Average time between a callback being scheduled in the IO Thread and the
    // time it runs
    db::ExponentialMovingAverage callbackDelayAvg{1.0 / 16.0};
    db::ExponentialMovingAverage ioThreadBusyTime{1.0 / 16.0};
    db::ExponentialMovingAverage ioThreadIdleTime{1.0 / 16.0};
  };
  std::shared_ptr<StatsTracker> stats_tracker_;
};

// Don't these directly. Used to separate the Connection synchronization
// between AsyncMysqlClient or SyncMysqlClient.
class AsyncConnection : public AsyncConnectionHelper {
 public:
  AsyncConnection(
      MysqlClientBase& mysql_client,
      std::shared_ptr<const ConnectionKey> conn_key,
      std::unique_ptr<ConnectionHolder> conn = nullptr)
      : AsyncConnectionHelper(
            mysql_client,
            std::move(conn_key),
            std::move(conn)) {}

  virtual ~AsyncConnection() override;

  // Don't allow copy or move
  AsyncConnection(const AsyncConnection&) = delete;
  AsyncConnection& operator=(const AsyncConnection&) = delete;

  AsyncConnection(AsyncConnection&&) = delete;
  AsyncConnection& operator=(AsyncConnection&&) = delete;

 protected:
  std::unique_ptr<InternalConnection> createInternalConnection() override {
    return std::make_unique<mysql_protocol::AsyncMysqlConnection>();
  }
};

} // namespace facebook::common::mysql_client
