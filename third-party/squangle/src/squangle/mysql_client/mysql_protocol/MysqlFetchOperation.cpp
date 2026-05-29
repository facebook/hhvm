/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/mysql_protocol/MysqlFetchOperation.h"
#include "squangle/mysql_client/Connection.h"
#include "squangle/mysql_client/mysql_protocol/MysqlFetchOperationImpl.h"

namespace facebook::common::mysql_client::mysql_protocol {

// MysqlQueryOperation implementation

std::shared_ptr<MysqlQueryOperation> MysqlQueryOperation::create(
    std::unique_ptr<Connection> conn,
    Query&& query,
    LoggingFuncsPtr logging_funcs) {
  // Create OwnedConnection wrapper and delegate to createWithProxy
  auto connProxy =
      std::make_unique<OperationBase::OwnedConnection>(std::move(conn));
  return createWithProxy(
      std::move(connProxy), std::move(query), std::move(logging_funcs));
}

std::shared_ptr<MysqlQueryOperation> MysqlQueryOperation::createWithProxy(
    std::unique_ptr<ConnectionProxy> conn_proxy,
    Query&& query,
    LoggingFuncsPtr logging_funcs) {
  // Create MysqlFetchOperationImpl with the ConnectionProxy directly
  auto impl = std::make_unique<MysqlFetchOperationImpl>(
      std::move(conn_proxy),
      db::OperationType::Query,
      std::move(logging_funcs));
  return std::shared_ptr<MysqlQueryOperation>(
      new MysqlQueryOperation(std::move(impl), std::move(query)));
}

MysqlQueryOperation::MysqlQueryOperation(
    std::unique_ptr<FetchOperationImpl> impl,
    Query&& query)
    : QueryOperation(std::move(impl), std::move(query)) {}

// MysqlMultiQueryOperation implementation

std::shared_ptr<MysqlMultiQueryOperation> MysqlMultiQueryOperation::create(
    std::unique_ptr<Connection> conn,
    std::vector<Query>&& queries,
    LoggingFuncsPtr logging_funcs) {
  // Create OwnedConnection wrapper and delegate to createWithProxy
  auto connProxy =
      std::make_unique<OperationBase::OwnedConnection>(std::move(conn));
  return createWithProxy(
      std::move(connProxy), std::move(queries), std::move(logging_funcs));
}

std::shared_ptr<MysqlMultiQueryOperation>
MysqlMultiQueryOperation::createWithProxy(
    std::unique_ptr<ConnectionProxy> conn_proxy,
    std::vector<Query>&& queries,
    LoggingFuncsPtr logging_funcs) {
  // Create MysqlFetchOperationImpl with the ConnectionProxy directly
  auto impl = std::make_unique<MysqlFetchOperationImpl>(
      std::move(conn_proxy),
      db::OperationType::MultiQuery,
      std::move(logging_funcs));
  return std::shared_ptr<MysqlMultiQueryOperation>(
      new MysqlMultiQueryOperation(std::move(impl), std::move(queries)));
}

MysqlMultiQueryOperation::MysqlMultiQueryOperation(
    std::unique_ptr<FetchOperationImpl> impl,
    std::vector<Query>&& queries)
    : MultiQueryOperation(std::move(impl), std::move(queries)) {}

// MysqlMultiQueryStreamOperation implementation

std::shared_ptr<MysqlMultiQueryStreamOperation>
MysqlMultiQueryStreamOperation::create(
    std::unique_ptr<ConnectionProxy> conn,
    MultiQuery&& multi_query) {
  auto impl = std::make_unique<MysqlFetchOperationImpl>(
      std::move(conn), db::OperationType::MultiQueryStream, nullptr);
  return std::shared_ptr<MysqlMultiQueryStreamOperation>(
      new MysqlMultiQueryStreamOperation(
          std::move(impl), std::move(multi_query)));
}

MysqlMultiQueryStreamOperation::MysqlMultiQueryStreamOperation(
    std::unique_ptr<FetchOperationImpl> impl,
    MultiQuery&& multi_query)
    : MultiQueryStreamOperation(std::move(impl), std::move(multi_query)) {}

} // namespace facebook::common::mysql_client::mysql_protocol
