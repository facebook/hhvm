/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

//
// Asynchronous Connection Pool based on our async MySQL client.
//
// This pool offers na async way to acquire a connection by creating new ones
// or recycling an existing one. Also provides a way to limit the number of
// open connections per database/user and for the client.
//
// AsyncConnectionPool - This pool holds multiple MySQL connections and
//   manages them to make sure only healthy connections are given back.
//   The interface to request a connection works just like the
//   AsyncMysqlClient, an ConnectPoolOperation is started by `beginConnection`.
//
// ConnectPoolOperation - An abstraction of ConnectOperation that instead of
//   opening a new connection, requests a connection to the pool it was created
//   by. The usage and error treat are the same.

#pragma once

#include <folly/String.h>
#include <folly/container/F14Map.h>
#include <folly/futures/Future.h>
#include <folly/synchronization/Baton.h>
#include <memory>

#include "squangle/mysql_client/AsyncMysqlClient.h"
#include "squangle/mysql_client/ConnectPoolOperation.h"
#include "squangle/mysql_client/ConnectionPool.h"
#include "squangle/mysql_client/Operation.h"
#include "squangle/mysql_client/mysql_protocol/MysqlConnectPoolOperationImpl.h"

namespace facebook::common::mysql_client {

template <typename Client>
class ConnectPoolOperation;
class AsyncConnectionPool;
class PoolKey;

using AsyncConnectPoolOperation = ConnectPoolOperation<AsyncMysqlClient>;
using AsyncConnectPoolOperationImpl =
    mysql_protocol::MysqlConnectPoolOperationImpl<AsyncMysqlClient>;

class AsyncConnectionPool : public ConnectionPool<AsyncMysqlClient> {
 public:
  // Don't use std::chrono::duration::MAX to avoid overflows
  static std::shared_ptr<AsyncConnectionPool> makePool(
      std::shared_ptr<AsyncMysqlClient> mysql_client,
      PoolOptions pool_options = PoolOptions());

  // The destructor will start the shutdown phase
  ~AsyncConnectionPool() override;

  // Don't allow copy or move
  AsyncConnectionPool(const AsyncConnectionPool&) = delete;
  AsyncConnectionPool& operator=(const AsyncConnectionPool&) = delete;

  AsyncConnectionPool(AsyncConnectionPool&&) = delete;
  AsyncConnectionPool& operator=(AsyncConnectionPool&&) = delete;

  FOLLY_NODISCARD folly::SemiFuture<ConnectResult> connectSemiFuture(
      const std::string& host,
      int port,
      const std::string& database_name,
      const std::string& user,
      const std::string& password,
      const ConnectionOptions& conn_opts = ConnectionOptions());

  FOLLY_NODISCARD folly::SemiFuture<ConnectResult> connectSemiFuture(
      const std::string& host,
      int port,
      const std::string& database_name,
      const std::string& user,
      const std::string& password,
      const std::string& special_tag,
      const ConnectionOptions& conn_opts = ConnectionOptions());

  std::unique_ptr<Connection> connect(
      const std::string& host,
      int port,
      const std::string& database_name,
      const std::string& user,
      const std::string& password,
      const ConnectionOptions& conn_opts = ConnectionOptions());

  // Don't use the constructor directly, only public to use make_shared
  AsyncConnectionPool(
      std::shared_ptr<AsyncMysqlClient> mysql_client,
      PoolOptions pool_options);

  // It will clean the pool and block any new connections or operations
  // Shutting down phase:
  // The remaining connections or operations that are linked to this pool
  // will know (using their weak_pointer to this pool) that the pool is dead
  // and proceed without the pool.
  void shutdown() override;

  // for debugging, return number of pool keys in the pool
  FOLLY_NODISCARD size_t getNumKey() const noexcept {
    return conn_storage_.getNumKey();
  }

 private:
  template <typename Client>
  friend class ConnectPoolOperation;

  class CleanUpTimer : public folly::AsyncTimeout {
   public:
    explicit CleanUpTimer(
        folly::EventBase* base,
        PoolStorage<AsyncMysqlClient>& pool)
        : folly::AsyncTimeout(base), pool_(pool) {}

    void timeoutExpired() noexcept override {
      scheduleTimeout(PoolOptions::kCleanUpTimeout);
      pool_.cleanupOperations();
      pool_.cleanupConnections();
    }

   private:
    PoolStorage<AsyncMysqlClient>& pool_;
  } cleanup_timer_;

  // The AsyncConnectionPool needs to make sure certain things run in the mysql
  // thread.
  void validateCorrectThread() const override {
    DCHECK_EQ(std::this_thread::get_id(), mysql_client_->threadId());
  }

  bool runInCorrectThread(std::function<void()>&& func, bool /*wait*/)
      override {
    return mysql_client_->runInThread([func = std::move(func)]() { func(); });
  }

  std::unique_ptr<Connection> makeNewConnection(
      std::shared_ptr<const ConnectionKey> conn_key,
      std::unique_ptr<MysqlPooledHolder<AsyncMysqlClient>> mysqlConn) override {
    return std::make_unique<AsyncConnection>(
        *mysql_client_, std::move(conn_key), std::move(mysqlConn));
  }

  struct ShutdownData {
    bool shutting_down = false;
    bool finished_shutdown = false;
  };

  folly::Synchronized<ShutdownData> shutdown_data_;

  bool isShuttingDown() const override {
    return shutdown_data_.rlock()->shutting_down;
  }

  // Nothing needed here - the async connection pool should not wait.
  void openNewConnectionPrep(AsyncConnectPoolOperation& /*pool_op*/) override {}
  void openNewConnectionFinish(
      AsyncConnectPoolOperation& /*pool_op*/,
      const PoolKey& /*pool_key*/) override {}
};

} // namespace facebook::common::mysql_client
