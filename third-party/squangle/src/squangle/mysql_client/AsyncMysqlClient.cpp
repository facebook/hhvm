/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Memory.h>
#include <folly/Singleton.h>
#include <folly/futures/Future.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/system/ThreadName.h>
#include <mysql.h>

#include "squangle/logger/DBEventLogger.h"
#include "squangle/mysql_client/AsyncMysqlClient.h"
#include "squangle/mysql_client/ConnectOperation.h"
#include "squangle/mysql_client/ResetOperation.h" // IWYU pragma: keep
#include "squangle/mysql_client/SemiFutureAdapter.h"
#include "squangle/mysql_client/mysql_protocol/MysqlConnectOperationImpl.h"
#include "squangle/mysql_client/mysql_protocol/MysqlFetchOperationImpl.h"
#include "squangle/mysql_client/mysql_protocol/MysqlSpecialOperationImpl.h"

namespace facebook::common::mysql_client {

namespace detail {

struct AsyncMysqlClientSingletonTag {};

folly::Singleton<AsyncMysqlClient, AsyncMysqlClientSingletonTag>
    defaultAsyncMysqlClientSingleton(
        [] { return new AsyncMysqlClient; },
        AsyncMysqlClient::deleter);

} // namespace detail

std::shared_ptr<AsyncMysqlClient> AsyncMysqlClient::defaultClient() {
  return detail::defaultAsyncMysqlClientSingleton.try_get();
}

AsyncMysqlClient::AsyncMysqlClient(
    std::unique_ptr<db::SquangleLoggerBase> db_logger,
    std::unique_ptr<db::DBCounterBase> db_stats,
    std::unique_ptr<const MysqlExceptionBuilder> exception_builder)
    : MysqlClientBase(
          adjustLogger(std::move(db_logger)),
          std::move(db_stats),
          std::move(exception_builder)),
      pools_conn_limit_(std::numeric_limits<uint64_t>::max()),
      stats_tracker_(std::make_shared<StatsTracker>()) {
  init();
}

AsyncMysqlClient::AsyncMysqlClient()
    : AsyncMysqlClient(nullptr, std::make_unique<db::SimpleDbCounter>()) {}

void AsyncMysqlClient::init() {
  auto eventBase = getEventBase();
  eventBase->setObserver(stats_tracker_);
  thread_ = std::thread([eventBase]() {
#ifdef __GLIBC__
    folly::setThreadName("async-mysql");
#endif
    folly::EventBaseManager::get()->setEventBase(eventBase, false);
    eventBase->loopForever();
    mysql_thread_end();
  });
  eventBase->waitUntilRunning();
}

bool AsyncMysqlClient::runInThread(std::function<void()>&& fn, bool wait) {
  auto scheduleTime = std::chrono::steady_clock::now();
  auto func = [fn = std::move(fn), scheduleTime, this]() mutable {
    auto delay = std::chrono::duration_cast<std::chrono::microseconds>(
                     std::chrono::steady_clock::now() - scheduleTime)
                     .count();
    stats_tracker_->callbackDelayAvg.addSample(delay);
    fn();
  };
  if (wait) {
    getEventBase()->runInEventBaseThreadAndWait(std::move(func));
  } else {
    getEventBase()->runInEventBaseThread(std::move(func));
  }
  return true;
}

void AsyncMysqlClient::drain(bool also_block_operations) {
  pending_.withWLock([&](auto& pending) {
    pending.block_operations = also_block_operations;

    auto it = pending.operations.begin();
    // Clean out any unstarted operations.
    while (it != pending.operations.end()) {
      // So here the Operation `run` was not called
      // We don't need to lock the state change in the operation here since the
      // cancelling process is going to fire not matter in which part it is.
      if ((*it)->state() == OperationState::Unstarted &&
          pending.to_remove.contains(*it)) {
        (*it)->cancel();
        it = pending.operations.erase(it);
      } else {
        ++it;
      }
    }
  });

  // Now wait for any started operations to complete.
  std::unique_lock<std::mutex> counter_lock(this->counters_mutex_);
  active_connections_closed_cv_.wait(
      counter_lock, [&also_block_operations, this] {
        if (also_block_operations) {
          VLOG(11)
              << "Waiting for " << this->active_connection_counter_
              << " connections to be released before shutting client down ";
        }
        return this->active_connection_counter_ == 0;
      });
}

void AsyncMysqlClient::shutdownClient() {
  DCHECK(std::this_thread::get_id() != threadId());
  if (is_shutdown_.exchange(true)) {
    return;
  }
  // Drain anything we currently have, and if those operations make
  // new operations, that's okay.
  drain(false);
  // Once that pass is done, finish anything that happened to sneak
  // in, but guarantee no new operations will come along.
  drain(true);

  CHECK_EQ(numStartedAndOpenConnections(), 0);

  DCHECK(connection_references_.size() == 0);

  // TODO: Maybe add here a runInThread to cancel the AsyncTimeout

  // All operations are done.  Shut the thread down.
  getEventBase()->terminateLoopSoon();
  if (std::this_thread::get_id() != threadId()) {
    thread_.join();
  } else {
    LOG(ERROR) << "shutdownClient() called from AsyncMysql thread";
    // @lint-ignore CLANGTIDY facebook-hte-BadCall-detach
    thread_.detach();
  }
}

AsyncMysqlClient::~AsyncMysqlClient() {
  shutdownClient();
  VLOG(2) << "AsyncMysqlClient finished destructor";
}

db::SquangleLoggingData AsyncMysqlClient::makeSquangleLoggingData(
    std::shared_ptr<const ConnectionKey> connKey,
    const db::ConnectionContextBase* connContext) {
  return db::SquangleLoggingData(
      std::move(connKey), connContext, collectPerfStats());
}

std::unique_ptr<ConnectOperationImpl>
AsyncMysqlClient::createConnectOperationImpl(
    MysqlClientBase* client_base,
    std::shared_ptr<const ConnectionKey> conn_key) const {
  return std::make_unique<mysql_protocol::MysqlConnectOperationImpl>(
      client_base, std::move(conn_key));
}

std::unique_ptr<FetchOperationImpl> AsyncMysqlClient::createFetchOperationImpl(
    std::unique_ptr<OperationBase::ConnectionProxy> conn,
    LoggingFuncsPtr logging_funcs) const {
  return std::make_unique<mysql_protocol::MysqlFetchOperationImpl>(
      std::move(conn), std::move(logging_funcs));
}

std::unique_ptr<SpecialOperationImpl>
AsyncMysqlClient::createSpecialOperationImpl(
    std::unique_ptr<OperationBase::ConnectionProxy> conn) const {
  return std::make_unique<mysql_protocol::MysqlSpecialOperationImpl>(
      std::move(conn));
}

void AsyncMysqlClient::cleanupCompletedOperations() {
  pending_.withWLock([](auto& pending) {
    size_t num_erased = 0, before = pending.operations.size();

    VLOG(11) << "removing pending operations";
    for (const auto& op : pending.to_remove) {
      if (pending.operations.erase(op) > 0) {
        ++num_erased;
      } else {
        LOG(DFATAL) << "asked to remove non-pending operation";
      }
    }

    pending.to_remove.clear();

    VLOG(11) << "erased: " << num_erased << ", before: " << before
             << ", after: " << pending.operations.size();
  });
}

folly::SemiFuture<ConnectResult> AsyncMysqlClient::connectSemiFuture(
    const std::string& host,
    int port,
    const std::string& database_name,
    const std::string& user,
    const std::string& password,
    const ConnectionOptions& conn_opts) {
  auto op = beginConnection(host, port, database_name, user, password);
  op->setConnectionOptions(conn_opts);
  return toSemiFuture(op);
}

std::unique_ptr<Connection> AsyncMysqlClient::connect(
    const std::string& host,
    int port,
    const std::string& database_name,
    const std::string& user,
    const std::string& password,
    const ConnectionOptions& conn_opts) {
  auto op = beginConnection(host, port, database_name, user, password);
  op->setConnectionOptions(conn_opts);
  // This will throw (intended behavour) in case the operation didn't succeed
  auto conn = blockingConnectHelper(*op);
  return conn;
}

std::unique_ptr<Connection> AsyncMysqlClient::createConnection(
    std::shared_ptr<const ConnectionKey> conn_key) {
  return std::make_unique<AsyncConnection>(*this, std::move(conn_key), nullptr);
}

AsyncConnection::~AsyncConnection() {
  if (mysql_connection_ && conn_dying_callback_ && needToCloneConnection_ &&
      isReusable() && !inTransaction() &&
      getConnectionOptions().isEnableResetConnBeforeClose()) {
    // We clone this Connection object to send COM_RESET_CONNECTION command
    // via the connection before returning it to the connection pool.
    // The callback function points to recycleMysqlConnection(), which is
    // responsible for recyclining the connection.
    // This object's callback is set to null and the cloned object's
    // callback instead points to the original callback function, which will
    // be called after COM_RESET_CONNECTION.

    auto connHolder = stealConnectionHolder(true);
    auto conn = std::make_unique<AsyncConnection>(
        client(), getKey(), std::move(connHolder));
    conn->needToCloneConnection_ = false;
    conn->setConnectionOptions(getConnectionOptions());
    conn->setConnectionDyingCallback(std::move(conn_dying_callback_));
    conn_dying_callback_ = nullptr;

    auto resetOp = Connection::resetConn(std::move(conn));
    runInThread([resetOp = std::move(resetOp)] { resetOp->run(); });
  }
}

} // namespace facebook::common::mysql_client
