/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/AsyncConnectionPool.h"
#include "squangle/mysql_client/AsyncMysqlClient.h"
#include "squangle/mysql_client/FutureAdapter.h"

#include <cstddef>
#include <memory>

#include <folly/MapUtil.h>
#include <folly/Memory.h>
#include <folly/synchronization/Baton.h>

#include <vector>

#include <mysql.h>
#include <chrono>

namespace facebook {
namespace common {
namespace mysql_client {

std::shared_ptr<AsyncConnectionPool> AsyncConnectionPool::makePool(
    std::shared_ptr<AsyncMysqlClient> mysql_client,
    PoolOptions pool_options) {
  auto connectionPool = std::make_shared<AsyncConnectionPool>(
      std::move(mysql_client), std::move(pool_options));
  return connectionPool;
}

AsyncConnectionPool::AsyncConnectionPool(
    std::shared_ptr<AsyncMysqlClient> mysql_client,
    PoolOptions pool_options)
    : ConnectionPool<AsyncMysqlClient>(
          std::move(mysql_client),
          std::move(pool_options)),
      cleanup_timer_(mysql_client_->getEventBase(), conn_storage_) {
  if (!mysql_client_->runInThread([this]() {
        cleanup_timer_.scheduleTimeout(PoolOptions::kCleanUpTimeout);
      })) {
    LOG(DFATAL) << "Unable to schedule timeout due Thrift event issue";
  }
}

AsyncConnectionPool::~AsyncConnectionPool() {
  VLOG(2) << "Connection pool dying";

  if (!shutdown_data_.rlock()->finished_shutdown) {
    shutdown();
  }

  VLOG(2) << "Connection pool shutdown completed";
}

void AsyncConnectionPool::shutdown() {
  VLOG(2) << "Shutting down";

  auto shutdown_func = [&](auto& shutdown_data) {
    cleanup_timer_.cancelTimeout();
    conn_storage_.clearAll();
    shutdown_data.finished_shutdown = true;
    VLOG(1) << "Shutting down in mysql_client thread";
  };

  // New scope to limit the lifetime of the write lock
  {
    auto shutdown_data = shutdown_data_.wlock();
    if (shutdown_data->shutting_down) {
      return;
    }

    // Will block adding anything to the pool
    shutdown_data->shutting_down = true;

    // cancelTimeout can only be ran in the mysql_client thread
    if (std::this_thread::get_id() == mysql_client_->threadId()) {
      shutdown_func(*shutdown_data);
      return;
    }
  }

  // We aren't already in the right thread, cause the shutdown to get run in the
  // correct thread and wait for it to complete.
  mysql_client_->runInThread(
      [&]() { shutdown_func(*shutdown_data_.wlock()); }, /*wait*/ true);
}

folly::SemiFuture<ConnectResult> AsyncConnectionPool::connectSemiFuture(
    const std::string& host,
    int port,
    const std::string& database_name,
    const std::string& user,
    const std::string& password,
    const ConnectionOptions& conn_opts) {
  return connectSemiFuture(
      host, port, database_name, user, password, "", conn_opts);
}

folly::SemiFuture<ConnectResult> AsyncConnectionPool::connectSemiFuture(
    const std::string& host,
    int port,
    const std::string& database_name,
    const std::string& user,
    const std::string& password,
    const std::string& special_tag,
    const ConnectionOptions& conn_opts) {
  auto op =
      beginConnection(host, port, database_name, user, password, special_tag);
  op->setConnectionOptions(conn_opts);
  return toSemiFuture(std::move(op));
}

std::unique_ptr<Connection> AsyncConnectionPool::connect(
    const std::string& host,
    int port,
    const std::string& database_name,
    const std::string& user,
    const std::string& password,
    const ConnectionOptions& conn_opts) {
  auto op = beginConnection(host, port, database_name, user, password);
  op->setConnectionOptions(conn_opts);
  // This will throw (intended behaviour) in case the operation didn't succeed
  return blockingConnectHelper(std::move(op));
}

template <>
AsyncConnectPoolOperation* AsyncConnectPoolOperation::specializedRun() {
  std::weak_ptr<Operation> weakSelf = getSharedPointer();
  if (!client()->runInThread([weakSelf]() {
        // There is a race confition that allows a cancelled or completed
        // operation getting here. The self ptr check ensures that the client
        // has not freed the reference to the operation, and the state() check
        // verifies whether other relevant memebers have been cleaned up by
        // connect callbacks
        auto self =
            std::static_pointer_cast<ConnectPoolOperation>(weakSelf.lock());
        if (!self || (self->state() == OperationState::Completed)) {
          LOG(ERROR) << "ConnectPoolOperation freed before running";
          return;
        }

        self->specializedRunImpl();
      })) {
    completeOperationInner(OperationResult::Failed);
  }
  return this;
}

std::ostream& operator<<(std::ostream& os, ExpirationPolicy policy) {
  auto str = (policy == ExpirationPolicy::Age)
      ? "Age"
      : (policy == ExpirationPolicy::IdleTime ? "IdleTime" : "<invalid>");
  return os << str;
}

std::ostream& operator<<(std::ostream& os, const PoolOptions& options) {
  return os << "{per key limit:" << options.getPerKeyLimit()
            << ",pool limit:" << options.getPoolLimit()
            << ",idle timeout:" << options.getIdleTimeout().count()
            << "us,age timeout:" << options.getAgeTimeout().count()
            << "us,expiration policy:" << options.getExpPolicy()
            << ",pool per instance:" << options.poolPerMysqlInstance() << "}";
}

std::ostream& operator<<(std::ostream& os, const PoolKey& key) {
  return os << "{key:" << key.getConnectionKey().getDisplayString()
            << ",options:" << key.getConnectionOptions().getDisplayString()
            << "}";
}

template <>
std::string AsyncConnectPoolOperation::createTimeoutErrorMessage(
    const PoolKeyStats& pool_key_stats,
    size_t per_key_limit) {
  auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - start_time_);
  auto cbDelayUs = client()->callbackDelayMicrosAvg();
  bool stalled = (cbDelayUs >= kCallbackDelayStallThresholdUs);

  std::vector<std::string> parts;
  const auto& key = getConnectionKey();
  parts.push_back(fmt::format(
      "[{}]({})Connection to {}:{} timed out in pool",
      static_cast<uint16_t>(SquangleErrno::SQ_ERRNO_POOL_CONN_TIMEOUT),
      kErrorPrefix,
      key.host(),
      key.port()));
  parts.push_back(fmt::format(
      "(open {}, opening {}, key limit {})",
      pool_key_stats.open_connections,
      pool_key_stats.pending_connections,
      per_key_limit));
  parts.push_back(timeoutMessage(delta));
  if (stalled) {
    parts.push_back(threadOverloadMessage(cbDelayUs));
  }
  return folly::join(" ", parts);
}

} // namespace mysql_client
} // namespace common
} // namespace facebook
