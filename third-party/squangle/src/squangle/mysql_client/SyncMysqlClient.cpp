/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Singleton.h>
#include <memory>

#include "squangle/mysql_client/ResetOperation.h" // IWYU pragma: keep
#include "squangle/mysql_client/SyncMysqlClient.h"
#include "squangle/mysql_client/mysql_protocol/MysqlConnectOperation.h"
#include "squangle/mysql_client/mysql_protocol/MysqlConnectOperationImpl.h"
#include "squangle/mysql_client/mysql_protocol/MysqlFetchOperation.h"
#include "squangle/mysql_client/mysql_protocol/MysqlFetchOperationImpl.h"
#include "squangle/mysql_client/mysql_protocol/MysqlSpecialOperation.h"
#include "squangle/mysql_client/mysql_protocol/MysqlSpecialOperationImpl.h"

namespace facebook::common::mysql_client {

namespace detail {

struct SyncMysqlClientSingletonTag {};

const folly::Singleton<SyncMysqlClient, SyncMysqlClientSingletonTag>
    defaultSyncMysqlClientSingleton;

} // namespace detail

std::shared_ptr<SyncMysqlClient> SyncMysqlClient::defaultClient() {
  return detail::defaultSyncMysqlClientSingleton.try_get();
}

std::unique_ptr<ConnectOperationImpl>
SyncMysqlClient::createConnectOperationImpl(
    MysqlClientBase* client_base,
    std::shared_ptr<const ConnectionKey> conn_key) const {
  return std::make_unique<mysql_protocol::MysqlConnectOperationImpl>(
      client_base, std::move(conn_key));
}

std::unique_ptr<FetchOperationImpl> SyncMysqlClient::createFetchOperationImpl(
    std::unique_ptr<OperationBase::ConnectionProxy> conn,
    db::OperationType operation_type,
    LoggingFuncsPtr logging_funcs) const {
  return std::make_unique<mysql_protocol::MysqlFetchOperationImpl>(
      std::move(conn), operation_type, std::move(logging_funcs));
}

std::unique_ptr<SpecialOperationImpl>
SyncMysqlClient::createSpecialOperationImpl(
    std::unique_ptr<OperationBase::ConnectionProxy> conn,
    db::OperationType operation_type) const {
  return std::make_unique<mysql_protocol::MysqlSpecialOperationImpl>(
      std::move(conn), operation_type);
}

std::unique_ptr<Connection> SyncMysqlClient::createConnection(
    std::shared_ptr<const ConnectionKey> conn_key) {
  return std::make_unique<SyncConnection>(*this, std::move(conn_key));
}

// Return unified MySQL special operation classes
std::shared_ptr<SpecialOperation> SyncMysqlClient::createResetOperation(
    std::unique_ptr<Connection> conn) const {
  return mysql_protocol::MysqlResetOperation::create(std::move(conn));
}

std::shared_ptr<SpecialOperation> SyncMysqlClient::createChangeUserOperation(
    std::unique_ptr<Connection> conn,
    std::shared_ptr<const ConnectionKey> key) const {
  return mysql_protocol::MysqlChangeUserOperation::create(
      std::move(conn), std::move(key));
}

std::shared_ptr<QueryOperation> SyncMysqlClient::createQueryOperation(
    std::unique_ptr<Connection> conn,
    Query&& query,
    LoggingFuncsPtr logging_funcs) const {
  return mysql_protocol::MysqlQueryOperation::create(
      std::move(conn), std::move(query), std::move(logging_funcs));
}

std::shared_ptr<MultiQueryOperation> SyncMysqlClient::createMultiQueryOperation(
    std::unique_ptr<Connection> conn,
    std::vector<Query>&& queries,
    LoggingFuncsPtr logging_funcs) const {
  return mysql_protocol::MysqlMultiQueryOperation::create(
      std::move(conn), std::move(queries), std::move(logging_funcs));
}

// ConnectionProxy overloads for sync operations (non-owning reference)
std::shared_ptr<QueryOperation> SyncMysqlClient::createQueryOperation(
    std::unique_ptr<OperationBase::ConnectionProxy> conn_proxy,
    Query&& query,
    LoggingFuncsPtr logging_funcs) const {
  // Use the unified class with createWithProxy()
  return mysql_protocol::MysqlQueryOperation::createWithProxy(
      std::move(conn_proxy), std::move(query), std::move(logging_funcs));
}

std::shared_ptr<MultiQueryOperation> SyncMysqlClient::createMultiQueryOperation(
    std::unique_ptr<OperationBase::ConnectionProxy> conn_proxy,
    std::vector<Query>&& queries,
    LoggingFuncsPtr logging_funcs) const {
  // Use the unified class with createWithProxy()
  return mysql_protocol::MysqlMultiQueryOperation::createWithProxy(
      std::move(conn_proxy), std::move(queries), std::move(logging_funcs));
}

std::shared_ptr<ConnectOperation> SyncMysqlClient::createConnectOperation(
    std::shared_ptr<const ConnectionKey> conn_key) {
  auto ret =
      mysql_protocol::MysqlConnectOperation::create(this, std::move(conn_key));
  if (connection_cb_) {
    ret->setObserverCallback(connection_cb_);
  }
  return ret;
}

SyncConnection::~SyncConnection() {
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

    auto connHolder = stealConnectionHolder(true);
    auto conn = std::make_unique<SyncConnection>(
        client(), getKey(), std::move(connHolder));
    conn->needToCloneConnection_ = false;
    conn->setConnectionOptions(getConnectionOptions());
    conn->setConnectionDyingCallback(std::move(conn_dying_callback_));
    conn_dying_callback_ = nullptr;
    auto resetOp = Connection::resetConn(std::move(conn));
    resetOp->run().wait();
  }
}

} // namespace facebook::common::mysql_client
