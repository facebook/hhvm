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
    return "SyncMysqlClient";
  }

 protected:
  // Private methods, primarily used by Operations and its subclasses.
  template <typename Client>
  friend class ConnectionPool;
  friend class SyncConnection;

  std::unique_ptr<ConnectOperationImpl> createConnectOperationImpl(
      MysqlClientBase* client,
      std::shared_ptr<const ConnectionKey> conn_key) const override;

  std::unique_ptr<FetchOperationImpl> createFetchOperationImpl(
      std::unique_ptr<OperationBase::ConnectionProxy> conn,
      db::OperationType operation_type,
      LoggingFuncsPtr logging_funcs) const override;

  std::unique_ptr<SpecialOperationImpl> createSpecialOperationImpl(
      std::unique_ptr<OperationBase::ConnectionProxy> conn,
      db::OperationType operation_type) const override;

  /***************************************************************************
   * Special code for handling MultiQueryStreamedHandler
   ***************************************************************************/

  bool useDirectStreamMode() const override {
    return true;
  }

  // Override to return unified MySQL special operation classes
  std::shared_ptr<SpecialOperation> createResetOperation(
      std::unique_ptr<Connection> conn) const override;
  std::shared_ptr<SpecialOperation> createChangeUserOperation(
      std::unique_ptr<Connection> conn,
      std::shared_ptr<const ConnectionKey> key) const override;

  // Unified factory methods using MysqlQueryOperation/MysqlMultiQueryOperation
  // Bring base class overloads with logging_funcs into scope
  using MysqlClientBase::createMultiQueryOperation;
  using MysqlClientBase::createQueryOperation;
  std::shared_ptr<QueryOperation> createQueryOperation(
      std::unique_ptr<Connection> conn,
      Query&& query,
      LoggingFuncsPtr logging_funcs = nullptr) const override;
  std::shared_ptr<MultiQueryOperation> createMultiQueryOperation(
      std::unique_ptr<Connection> conn,
      std::vector<Query>&& queries,
      LoggingFuncsPtr logging_funcs = nullptr) const override;

  // Overloads for sync operations (with ConnectionProxy)
  std::shared_ptr<QueryOperation> createQueryOperation(
      std::unique_ptr<OperationBase::ConnectionProxy> conn_proxy,
      Query&& query,
      LoggingFuncsPtr logging_funcs = nullptr) const override;
  std::shared_ptr<MultiQueryOperation> createMultiQueryOperation(
      std::unique_ptr<OperationBase::ConnectionProxy> conn_proxy,
      std::vector<Query>&& queries,
      LoggingFuncsPtr logging_funcs = nullptr) const override;

  // Unified factory method using MysqlConnectOperation
  std::shared_ptr<ConnectOperation> createConnectOperation(
      std::shared_ptr<const ConnectionKey> conn_key) override;
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
    auto impl = client().createFetchOperationImpl(
        std::move(proxy), db::OperationType::MultiQueryStream, nullptr);
    return MultiQueryStreamOperation::create(
        std::move(impl), std::move(multi_query));
  }

 protected:
  std::unique_ptr<InternalConnection> createInternalConnection() override {
    return std::make_unique<mysql_protocol::SyncMysqlConnection>();
  }
};

} // namespace facebook::common::mysql_client
