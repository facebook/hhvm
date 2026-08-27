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
  return blockingConnectHelper(*op);
}

void SyncConnectionPool::openNewConnectionPrep(
    SyncConnectPoolOperation& pool_op) {
  pool_op.prepWait();
}

void SyncConnectionPool::openNewConnectionFinish(
    SyncConnectPoolOperation& pool_op,
    const PoolKey& pool_key) {
  if (pool_op.syncWait()) {
    // Whichever thread got here first left the work to us, so that this
    // operation is only ever driven on the thread that owns it. That is either
    // a connection to complete with, or another attempt to run. See
    // MysqlConnectPoolOperationImpl::connectionCallback and ::attemptFailed.
    pool_op.completeDeferred();
    if (!pool_op.resumeRetry()) {
      pool_op.cleanupWait();
    }
    return;
  }

  // We stopped waiting. Announce that, so that a handoff still in flight knows
  // to complete the operation itself rather than leave it to us.
  if (pool_op.abandonWait()) {
    // We got there first, so no handoff has happened. Either the operation is
    // still queued and we time it out, or it already left the queue another way
    // (failOperations) and is already complete.
    if (conn_storage_.dequeueOperation(pool_key, pool_op)) {
      pool_op.timeoutTriggered();
    }
  } else if (pool_op.abandonRetry()) {
    // attemptFailed() handed a retry back in the race window after its CAS but
    // before it posted the baton, so syncWait() timed out on the pre-retry
    // deadline rather than waking to run it. We have stopped waiting and cannot
    // re-arm the baton to run the retry, so time the operation out instead.
    // failOperations() already removed it from the queue, so complete it
    // directly rather than via dequeueOperation().
    pool_op.timeoutTriggered();
  } else {
    // A connection handoff landed while we were timing out, so finishing it is
    // ours.
    pool_op.completeDeferred();
  }

  // Deliberately no cleanupWait() on either of these paths: the handing-off
  // thread may not have reached signalWaiter() yet, and resetting the baton
  // underneath it would race. It is released with the operation.
}

template <>
void SyncConnectPoolOperationImpl::specializedRun() {
  // No special thread manipulation needed for sync client
  MysqlConnectPoolOperationImpl::specializedRunImpl();
}

template <>
std::unique_ptr<ConnectPoolOperationImpl<SyncMysqlClient>>
createConnectPoolOperationImpl(
    std::weak_ptr<ConnectionPool<SyncMysqlClient>> pool,
    std::shared_ptr<SyncMysqlClient> client,
    std::shared_ptr<const ConnectionKey> conn_key) {
  return std::make_unique<
      mysql_protocol::MysqlConnectPoolOperationImpl<SyncMysqlClient>>(
      std::move(pool), client, std::move(conn_key));
}

} // namespace facebook::common::mysql_client
