/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/executors/GlobalExecutor.h>
#include <folly/futures/Future.h>
#include <folly/synchronization/CallOnce.h>

#include "squangle/mysql_client/ConnectPoolOperation.h"
#include "squangle/mysql_client/ConnectionPool.h"
#include "squangle/mysql_client/SyncMysqlClient.h"
#include "squangle/mysql_client/mysql_protocol/MysqlConnectPoolOperationImpl.h"

namespace facebook::common::mysql_client {

using SyncConnectPoolOperation = ConnectPoolOperation<SyncMysqlClient>;
using SyncConnectPoolOperationImpl =
    mysql_protocol::MysqlConnectPoolOperationImpl<SyncMysqlClient>;

class SyncConnectionPool : public ConnectionPool<SyncMysqlClient> {
 public:
  // Don't use std::chrono::duration::MAX to avoid overflows
  static std::shared_ptr<SyncConnectionPool> makePool(
      std::shared_ptr<SyncMysqlClient> mysql_client,
      PoolOptions pool_options = PoolOptions());

  std::unique_ptr<Connection> connect(
      const std::string& host,
      int port,
      const std::string& database_name,
      const std::string& user,
      const std::string& password,
      const ConnectionOptions& conn_opts = ConnectionOptions());

  // Don't use the constructor directly, only public to use make_shared
  SyncConnectionPool(
      std::shared_ptr<SyncMysqlClient> mysql_client,
      PoolOptions pool_options)
      : ConnectionPool<SyncMysqlClient>(
            std::move(mysql_client),
            std::move(pool_options)) {}

  ~SyncConnectionPool() override {
    VLOG(2) << "Connection pool dying";

    shutdown();

    VLOG(2) << "Connection pool shutdown completed";
  }

  SyncConnectionPool(const SyncConnectionPool&) = delete;
  SyncConnectionPool& operator=(const SyncConnectionPool&) = delete;

  SyncConnectionPool(SyncConnectionPool&&) = delete;
  SyncConnectionPool& operator=(SyncConnectionPool&&) = delete;

  void shutdown() override {
    bool expected = false;
    if (shutting_down_.compare_exchange_strong(expected, true)) {
      conn_storage_.clearAll();
    }
  }

 private:
  // Arms the periodic cleanup. Deferred to the first connection request rather
  // than done in the constructor because weak_from_this() is empty until a
  // shared_ptr owns the pool, which would end the loop on its first iteration.
  void ensureCleanupScheduled() {
    folly::call_once(cleanup_started_, [this]() {
      try {
        scheduleCleanup();
      } catch (const std::exception& e) {
        // Only the global CPU executor going away at process shutdown, where
        // nothing is left worth cleaning up. Swallowing it keeps an unexpected
        // exception type out of the caller's connect.
        LOG(WARNING) << "not scheduling pool cleanup: " << e.what();
      }
    });
  }

  // Re-arms itself after each pass, so the pool needs no thread of its own. The
  // weak reference ends the loop once the pool is gone; while it is held the
  // pool cannot be destroyed underneath us.
  //
  // Cleanup destroys pooled connections, and removeFromPool() takes a strong
  // reference to do so -- when that is the last one the pool is destroyed on
  // whichever thread ran the cleanup. Safe only because this pool joins no
  // thread.
  void scheduleCleanup() {
    folly::futures::sleep(PoolOptions::kCleanUpTimeout)
        .via(folly::getGlobalCPUExecutor())
        .thenValue([weakSelf = weak_from_this()](auto&&) {
          auto self =
              std::static_pointer_cast<SyncConnectionPool>(weakSelf.lock());
          if (!self || self->isShuttingDown()) {
            return;
          }
          self->conn_storage_.cleanupOperations();
          self->conn_storage_.cleanupConnections();
          self->scheduleCleanup();
        });
  }

  bool isShuttingDown() const override {
    return shutting_down_;
  }

  void validateCorrectThread() const override {
    // The sync connection pool runs everything in the clients' threads so don't
    // do anything here.
  }

  bool runInCorrectThread(std::function<void()>&& func, bool /*wait*/)
      override {
    // The sync connection pool runs everything in the clients' threads.
    func();
    return true;
  }

  std::unique_ptr<Connection> makeNewConnection(
      std::shared_ptr<const ConnectionKey> conn_key,
      std::unique_ptr<MysqlPooledHolder<SyncMysqlClient>> mysqlConn) override {
    return std::make_unique<SyncConnection>(
        *mysql_client_, std::move(conn_key), std::move(mysqlConn));
  }

  void openNewConnectionPrep(SyncConnectPoolOperation& pool_op) override;

  void openNewConnectionFinish(
      SyncConnectPoolOperation& pool_op,
      const PoolKey& pool_key) override;

  std::atomic<bool> shutting_down_{false};

  folly::once_flag cleanup_started_;
};

} // namespace facebook::common::mysql_client
