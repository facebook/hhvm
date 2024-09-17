/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/MysqlClientBase.h"
#include "squangle/mysql_client/ConnectOperation.h"
#include "squangle/mysql_client/Connection.h"
#include "squangle/mysql_client/SpecialOperation.h"

namespace {

// Used to initialize the SSL and MySQL libraries once per binary
class InitMysqlLibrary {
 public:
  InitMysqlLibrary() {
    mysql_library_init(-1, nullptr, nullptr);
  }
  ~InitMysqlLibrary() {
    mysql_library_end();
  }
};

} // namespace

namespace facebook::common::mysql_client {

// mysql_library_init() and mysql_library_end() need to run on the same thread
[[maybe_unused]] static InitMysqlLibrary unused;

MysqlClientBase::MysqlClientBase(
    std::unique_ptr<db::SquangleLoggerBase> db_logger,
    std::unique_ptr<db::DBCounterBase> db_stats)
    : db_logger_(std::move(db_logger)), client_stats_(std::move(db_stats)) {}

void MysqlClientBase::logQuerySuccess(
    const db::QueryLoggingData& logging_data,
    const Connection& conn) {
  auto conn_context = conn.getConnectionContext();
  stats()->incrSucceededQueries(conn_context);

  if (db_logger_) {
    db_logger_->logQuerySuccess(
        logging_data, makeSquangleLoggingData(conn.getKey(), conn_context));
  }
}

void MysqlClientBase::logQueryFailure(
    const db::QueryLoggingData& logging_data,
    db::FailureReason reason,
    unsigned int mysqlErrno,
    const std::string& error,
    const Connection& conn) {
  auto conn_context = conn.getConnectionContext();
  stats()->incrFailedQueries(conn_context, mysqlErrno, error);

  if (db_logger_) {
    db_logger_->logQueryFailure(
        logging_data,
        reason,
        mysqlErrno,
        error,
        makeSquangleLoggingData(conn.getKey(), conn_context));
  }
}

void MysqlClientBase::logConnectionSuccess(
    const db::CommonLoggingData& logging_data,
    std::shared_ptr<const ConnectionKey> conn_key,
    const db::ConnectionContextBase* connection_context) {
  if (db_logger_) {
    db_logger_->logConnectionSuccess(
        logging_data,
        makeSquangleLoggingData(std::move(conn_key), connection_context));
  }
}

void MysqlClientBase::logConnectionFailure(
    const db::CommonLoggingData& logging_data,
    db::FailureReason reason,
    std::shared_ptr<const ConnectionKey> conn_key,
    unsigned int mysqlErrno,
    const std::string& error,
    const db::ConnectionContextBase* connection_context) {
  stats()->incrFailedConnections(connection_context, mysqlErrno, error);

  if (db_logger_) {
    db_logger_->logConnectionFailure(
        logging_data,
        reason,
        mysqlErrno,
        error,
        makeSquangleLoggingData(std::move(conn_key), connection_context));
  }
}

std::shared_ptr<ConnectOperation> MysqlClientBase::beginConnection(
    const std::string& host,
    int port,
    const std::string& database_name,
    const std::string& user,
    const std::string& password) {
  return beginConnection(std::make_shared<const MysqlConnectionKey>(
      host, port, database_name, user, password));
}

std::shared_ptr<ConnectOperation> MysqlClientBase::beginConnection(
    std::shared_ptr<const ConnectionKey> conn_key) {
  auto impl = createConnectOperationImpl(this, std::move(conn_key));
  auto ret = ConnectOperation::create(std::move(impl));
  if (connection_cb_) {
    ret->setObserverCallback(connection_cb_);
  }
  addOperation(ret);
  return ret;
}

// Helper versions of the above that take a Connection instead of a
// ConnectionProxy
std::unique_ptr<FetchOperationImpl> MysqlClientBase::createFetchOperationImpl(
    std::unique_ptr<Connection> conn) const {
  return createFetchOperationImpl(
      std::make_unique<OperationBase::OwnedConnection>(std::move(conn)));
}
std::unique_ptr<SpecialOperationImpl>
MysqlClientBase::createSpecialOperationImpl(
    std::unique_ptr<Connection> conn) const {
  return createSpecialOperationImpl(
      std::make_unique<OperationBase::OwnedConnection>(std::move(conn)));
}

} // namespace facebook::common::mysql_client
