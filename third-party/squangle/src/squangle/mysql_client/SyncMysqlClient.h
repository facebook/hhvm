/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "squangle/mysql_client/Connection.h"
#include "squangle/mysql_client/MysqlClientBase.h"
#include "squangle/mysql_client/mysql_protocol/MysqlConnection.h"

namespace facebook::common::mysql_client {

class SyncConnection;

class SyncMysqlClient : public MysqlClientBase {
 public:
  SyncMysqlClient() : SyncMysqlClient(nullptr) {}
  explicit SyncMysqlClient(
      std::unique_ptr<db::SquangleLoggerBase> db_logger,
      std::unique_ptr<db::DBCounterBase> db_stats =
          std::make_unique<db::SimpleDbCounter>(),
      std::unique_ptr<const MysqlExceptionBuilder> exception_builder = nullptr)
      : MysqlClientBase(
            adjustLogger(std::move(db_logger)),
            std::move(db_stats),
            std::move(exception_builder)) {}

  static std::unique_ptr<db::SquangleLoggerBase> adjustLogger(
      std::unique_ptr<db::SquangleLoggerBase> logger) {
    if (logger) {
      logger->setLoggingPrefix("cpp_sync");
    }
    return logger;
  }

  // Factory method
  std::unique_ptr<Connection> createConnection(
      std::shared_ptr<const ConnectionKey> conn_key) override;

  void drain(bool /*unused*/) {}

  bool supportsLocalFiles() override {
    return true;
  }

  uint64_t getPoolsConnectionLimit() {
    // This is used by HHVM in the async client.  We don't need it here in the
    // sync client.
    return std::numeric_limits<uint64_t>::max();
  }

  static std::shared_ptr<SyncMysqlClient> defaultClient();

  virtual std::string_view getBaseClient() const noexcept override {
    static constexpr std::string_view kSyncClient{"SyncMysqlClient"};
    return kSyncClient;
  }

 protected:
  // Private methods, primarily used by Operations and its subclasses.
  template <typename Client>
  friend class ConnectionPool;

  std::unique_ptr<ConnectOperationImpl> createConnectOperationImpl(
      MysqlClientBase* client,
      std::shared_ptr<const ConnectionKey> conn_key) const override;

  std::unique_ptr<FetchOperationImpl> createFetchOperationImpl(
      std::unique_ptr<OperationBase::ConnectionProxy> conn,
      LoggingFuncsPtr logging_funcs) const override;

  std::unique_ptr<SpecialOperationImpl> createSpecialOperationImpl(
      std::unique_ptr<OperationBase::ConnectionProxy> conn) const override;
};

// SyncConnection is a specialization of Connection to handle inline loops.
// It has its own EventBase and Operations using it will have events and
// callbacks running with this EventBase.
class SyncConnection : public Connection {
 public:
  SyncConnection(
      MysqlClientBase& client,
      std::shared_ptr<const ConnectionKey> conn_key,
      std::unique_ptr<ConnectionHolder> conn = nullptr)
      : Connection(client, std::move(conn_key), std::move(conn)) {}

  ~SyncConnection() override;

  SyncConnection(const SyncConnection&) = delete;
  SyncConnection& operator=(const SyncConnection&) = delete;

  SyncConnection(SyncConnection&&) = delete;
  SyncConnection& operator=(SyncConnection&&) = delete;

  std::shared_ptr<MultiQueryStreamOperation> createOperation(
      std::unique_ptr<OperationBase::ConnectionProxy> proxy,
      MultiQuery&& multi_query) override {
    auto impl = client().createFetchOperationImpl(std::move(proxy), nullptr);
    return MultiQueryStreamOperation::create(
        std::move(impl), std::move(multi_query));
  }

 protected:
  std::unique_ptr<InternalConnection> createInternalConnection() override {
    return std::make_unique<mysql_protocol::SyncMysqlConnection>();
  }
};

} // namespace facebook::common::mysql_client
