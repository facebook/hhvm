/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/executors/FunctionScheduler.h>

#include "squangle/mysql_client/ConnectPoolOperation.h"
#include "squangle/mysql_client/ConnectionPool.h"
#include "squangle/mysql_client/SyncMysqlClient.h"

namespace facebook::common::mysql_client {

using SyncConnectPoolOperation = ConnectPoolOperation<SyncMysqlClient>;
using SyncConnectPoolOperationImpl = ConnectPoolOperationImpl<SyncMysqlClient>;

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
            std::move(pool_options)) {
    scheduler_.addFunction(
        [&]() {
          conn_storage_.cleanupOperations();
          conn_storage_.cleanupConnections();
        },
        PoolOptions::kCleanUpTimeout,
        "pool_periodic_cleanup");
    scheduler_.start();
  }

  ~SyncConnectionPool() {
    VLOG(2) << "Connection pool dying";

    shutdown();

    VLOG(2) << "Connection pool shutdown completed";
  }

  void shutdown() override {
    bool expected = false;
    if (shutting_down_.compare_exchange_strong(expected, true)) {
      scheduler_.shutdown();
      conn_storage_.clearAll();
    }
  }

 private:
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
      const ConnectionKey& conn_key,
      std::unique_ptr<MysqlPooledHolder<SyncMysqlClient>> mysqlConn) override {
    return std::make_unique<SyncConnection>(
        *mysql_client_, conn_key, std::move(mysqlConn));
  }

  void openNewConnectionPrep(SyncConnectPoolOperation& pool_op) override;

  void openNewConnectionFinish(
      SyncConnectPoolOperation& pool_op,
      const PoolKey& pool_key) override;

  std::atomic<bool> shutting_down_{false};

  folly::FunctionScheduler scheduler_;

  SyncConnectionPool(const SyncConnectionPool&) = delete;
  SyncConnectionPool& operator=(const SyncConnectionPool&) = delete;
};

} // namespace facebook::common::mysql_client
