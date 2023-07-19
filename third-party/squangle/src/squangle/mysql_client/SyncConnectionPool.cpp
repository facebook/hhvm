/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/SyncConnectionPool.h"

using namespace std::chrono_literals;

namespace facebook::common::mysql_client {

std::shared_ptr<SyncConnectionPool> SyncConnectionPool::makePool(
    std::shared_ptr<SyncMysqlClient> mysql_client,
    PoolOptions pool_options) {
  auto connectionPool = std::make_shared<SyncConnectionPool>(
      std::move(mysql_client), std::move(pool_options));
  return connectionPool;
}

std::unique_ptr<Connection> SyncConnectionPool::connect(
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

void SyncConnectionPool::openNewConnectionPrep(
    SyncConnectPoolOperation& pool_op) {
  pool_op.prepWait();
}

void SyncConnectionPool::openNewConnectionFinish(
    SyncConnectPoolOperation& pool_op,
    const PoolKey& pool_key) {
  if (!pool_op.syncWait()) {
    if (!conn_storage_.dequeueOperation(pool_key, pool_op)) {
      // The operation was not found in the queue, so someone must be fulfilling
      // the operation.  Wait until that is finished.
      while (pool_op.isActive()) {
        /* sleep_override */ std::this_thread::sleep_for(1ms);
      }

      return;
    }

    pool_op.timeoutTriggered();
  }

  pool_op.cleanupWait();
}

template <>
std::string SyncConnectPoolOperation::createTimeoutErrorMessage(
    const PoolKeyStats& pool_key_stats,
    size_t per_key_limit) {
  auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - start_time_);

  const auto& key = getConnectionKey();
  return fmt::format(
      "[{}]({})Connection to {}:{} timed out in pool "
      "(open {}, opening {}, key limit {}) {}",
      static_cast<uint16_t>(SquangleErrno::SQ_ERRNO_POOL_CONN_TIMEOUT),
      kErrorPrefix,
      key.host(),
      key.port(),
      pool_key_stats.open_connections,
      pool_key_stats.pending_connections,
      per_key_limit,
      timeoutMessage(delta));
}

template <>
SyncConnectPoolOperation* SyncConnectPoolOperation::specializedRun() {
  // No special thread manipulation needed for sync client
  ConnectPoolOperation::specializedRunImpl();
  return this;
}

} // namespace facebook::common::mysql_client
