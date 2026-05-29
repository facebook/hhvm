/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/mysql_protocol/MysqlSpecialOperation.h"
#include "squangle/mysql_client/Connection.h"
#include "squangle/mysql_client/mysql_protocol/MysqlSpecialOperationImpl.h"

namespace facebook::common::mysql_client::mysql_protocol {

// MysqlResetOperation implementation

std::shared_ptr<MysqlResetOperation> MysqlResetOperation::create(
    std::unique_ptr<Connection> conn) {
  // Create OwnedConnection wrapper and MysqlSpecialOperationImpl internally
  // This avoids exposing the protected OwnedConnection type to callers
  auto connProxy =
      std::make_unique<OperationBase::OwnedConnection>(std::move(conn));
  auto impl = std::make_unique<MysqlSpecialOperationImpl>(
      std::move(connProxy), db::OperationType::Reset);
  // Use shared_ptr constructor with private ctor via this helper
  return std::shared_ptr<MysqlResetOperation>(
      new MysqlResetOperation(std::move(impl)));
}

MysqlResetOperation::MysqlResetOperation(
    std::unique_ptr<SpecialOperationImpl> impl)
    : SpecialOperation(std::move(impl)) {}

InternalConnection::Status MysqlResetOperation::runSpecialOperation() {
  auto& internalConn = specialOperationImpl()->internalConnection();
  auto status = internalConn.resetConn();
  if (status == ERROR) {
    snapshotMysqlErrors(
        internalConn.getErrno(), internalConn.getErrorMessage());
  }
  return status;
}

// MysqlChangeUserOperation implementation

std::shared_ptr<MysqlChangeUserOperation> MysqlChangeUserOperation::create(
    std::unique_ptr<Connection> conn,
    std::shared_ptr<const ConnectionKey> key) {
  // Create OwnedConnection wrapper and MysqlSpecialOperationImpl internally
  auto connProxy =
      std::make_unique<OperationBase::OwnedConnection>(std::move(conn));
  auto impl = std::make_unique<MysqlSpecialOperationImpl>(
      std::move(connProxy), db::OperationType::ChangeUser);
  return std::shared_ptr<MysqlChangeUserOperation>(
      new MysqlChangeUserOperation(std::move(impl), std::move(key)));
}

MysqlChangeUserOperation::MysqlChangeUserOperation(
    std::unique_ptr<SpecialOperationImpl> impl,
    std::shared_ptr<const ConnectionKey> key)
    : SpecialOperation(std::move(impl)), key_(std::move(key)) {}

InternalConnection::Status MysqlChangeUserOperation::runSpecialOperation() {
  auto& internalConn = specialOperationImpl()->internalConnection();
  auto status = internalConn.changeUser(key_);
  if (status == ERROR) {
    snapshotMysqlErrors(
        internalConn.getErrno(), internalConn.getErrorMessage());
  }
  return status;
}

} // namespace facebook::common::mysql_client::mysql_protocol
