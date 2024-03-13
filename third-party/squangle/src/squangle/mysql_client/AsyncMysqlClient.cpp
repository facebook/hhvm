/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <fcntl.h>
#include <folly/Memory.h>
#include <folly/Singleton.h>
#include <folly/futures/Future.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/ssl/Init.h>
#include <folly/system/ThreadName.h>
#include <gflags/gflags.h>
#include <mysql.h>
#include <unistd.h>
#include <vector>

#include "squangle/logger/DBEventLogger.h"
#include "squangle/mysql_client/AsyncMysqlClient.h"
#include "squangle/mysql_client/FutureAdapter.h"
#include "squangle/mysql_client/Operation.h"

DECLARE_int64(mysql_mysql_timeout_micros);

namespace facebook {
namespace common {
namespace mysql_client {

namespace {
folly::Singleton<AsyncMysqlClient> client(
    []() { return new AsyncMysqlClient; },
    AsyncMysqlClient::deleter);
} // namespace

std::shared_ptr<AsyncMysqlClient> AsyncMysqlClient::defaultClient() {
  return folly::Singleton<AsyncMysqlClient>::try_get();
}

AsyncMysqlClient::AsyncMysqlClient(
    std::unique_ptr<db::SquangleLoggerBase> db_logger,
    std::unique_ptr<db::DBCounterBase> db_stats)
    : MysqlClientBase(adjustLogger(std::move(db_logger)), std::move(db_stats)),
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

bool AsyncMysqlClient::runInThread(folly::Cob&& fn, bool wait) {
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
      if ((*it)->state() == OperationState::Unstarted) {
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
    thread_.detach();
  }
}

AsyncMysqlClient::~AsyncMysqlClient() {
  shutdownClient();
  VLOG(2) << "AsyncMysqlClient finished destructor";
}

db::SquangleLoggingData AsyncMysqlClient::makeSquangleLoggingData(
    const ConnectionKey* connKey,
    const db::ConnectionContextBase* connContext) {
  db::SquangleLoggingData ret(connKey, connContext);
  ret.clientPerfStats = collectPerfStats();
  return ret;
}

void AsyncMysqlClient::cleanupCompletedOperations() {
  pending_.withWLock([](auto& pending) {
    size_t num_erased = 0, before = pending.operations.size();

    VLOG(11) << "removing pending operations";
    for (auto& op : pending.to_remove) {
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
  return toSemiFuture(std::move(op));
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
  auto conn = blockingConnectHelper(op);
  return conn;
}

std::unique_ptr<Connection> AsyncMysqlClient::createConnection(
    ConnectionKey conn_key,
    MYSQL* mysql_conn) {
  return std::make_unique<AsyncConnection>(
      this, std::move(conn_key), mysql_conn);
}

static inline MysqlHandler::Status toHandlerStatus(net_async_status status) {
  if (status == NET_ASYNC_ERROR) {
    return MysqlHandler::Status::ERROR;
  } else if (status == NET_ASYNC_COMPLETE) {
    return MysqlHandler::Status::DONE;
  } else {
    return MysqlHandler::Status::PENDING;
  }
}

MysqlHandler::Status AsyncMysqlClient::AsyncMysqlHandler::tryConnect(
    MYSQL* mysql,
    const ConnectionOptions& /*opts*/,
    const ConnectionKey& conn_key,
    int flags) {
  const auto usingUnixSocket = !conn_key.unixSocketPath().empty();

  // When using unix socket (AF_UNIX), host/port do not matter.
  return toHandlerStatus(mysql_real_connect_nonblocking(
      mysql,
      usingUnixSocket ? nullptr : conn_key.host().c_str(),
      conn_key.user().c_str(),
      conn_key.password().c_str(),
      conn_key.db_name().c_str(),
      usingUnixSocket ? 0 : conn_key.port(),
      usingUnixSocket ? conn_key.unixSocketPath().c_str() : nullptr,
      flags));
}

MysqlHandler::Status AsyncMysqlClient::AsyncMysqlHandler::runQuery(
    MYSQL* mysql,
    folly::StringPiece queryStmt) {
  return toHandlerStatus(
      mysql_real_query_nonblocking(mysql, queryStmt.begin(), queryStmt.size()));
}

MysqlHandler::Status AsyncMysqlClient::AsyncMysqlHandler::resetConn(
    MYSQL* mysql) {
  return toHandlerStatus(mysql_reset_connection_nonblocking(mysql));
}

MysqlHandler::Status AsyncMysqlClient::AsyncMysqlHandler::changeUser(
    MYSQL* mysql,
    const std::string& user,
    const std::string& password,
    const std::string& database) {
  return toHandlerStatus(mysql_change_user_nonblocking(
      mysql, user.c_str(), password.c_str(), database.c_str()));
}

MysqlHandler::Status AsyncMysqlClient::AsyncMysqlHandler::nextResult(
    MYSQL* mysql) {
  return toHandlerStatus(mysql_next_result_nonblocking(mysql));
}

MysqlHandler::Status AsyncMysqlClient::AsyncMysqlHandler::fetchRow(
    MYSQL_RES* res,
    MYSQL_ROW& row) {
  auto status = toHandlerStatus(mysql_fetch_row_nonblocking(res, &row));
  DCHECK_NE(status, ERROR); // Should never be an error
  return status;
}

MYSQL_RES* AsyncMysqlClient::AsyncMysqlHandler::getResult(MYSQL* mysql) {
  return mysql_use_result(mysql);
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

    auto connHolder = stealMysqlConnectionHolder(true);
    auto conn = std::make_unique<AsyncConnection>(
        client(), *getKey(), std::move(connHolder));
    conn->needToCloneConnection_ = false;
    conn->setConnectionOptions(getConnectionOptions());
    conn->setConnectionDyingCallback(std::move(conn_dying_callback_));
    conn_dying_callback_ = nullptr;

    auto resetOp = Connection::resetConn(std::move(conn));
    runInThread([resetOp = std::move(resetOp)] {
      // addOperation() is necessary here for proper cancelling of reset
      // operation in case of sudden AsyncMysqlClient shutdown
      resetOp->connection()->client()->addOperation(resetOp);
      resetOp->run();
    });
  }
}

} // namespace mysql_client
} // namespace common
} // namespace facebook
