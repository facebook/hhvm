/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "squangle/mysql_client/SpecialOperation.h"

namespace facebook::common::mysql_client::mysql_protocol {

// MysqlResetOperation is a UNIFIED class that combines ResetOperation logic
// with MySQL protocol implementation.
//
// This is part of the Operation hierarchy refactoring using COMPOSITION:
// - Inherits from SpecialOperation (no diamond inheritance)
// - Creates MysqlSpecialOperationImpl internally (composition, not inheritance)
// - The impl handles MySQL protocol; this class provides Reset-specific logic
//
// Key difference from legacy pattern:
// - Legacy: Caller creates MysqlSpecialOperationImpl, passes to ResetOperation
// - Unified: MysqlResetOperation creates its own impl internally,
// self-contained
//
class MysqlResetOperation : public SpecialOperation {
 public:
  // Factory method that takes a Connection and handles wrapping internally
  static std::shared_ptr<MysqlResetOperation> create(
      std::unique_ptr<Connection> conn);

  db::OperationType getOperationType() const override {
    return db::OperationType::Reset;
  }

 private:
  explicit MysqlResetOperation(std::unique_ptr<SpecialOperationImpl> impl);

  InternalConnection::Status runSpecialOperation() override;

  const char* getErrorMsg() const override {
    return errorMsg;
  }

  static constexpr const char* errorMsg = "Reset connection failed: ";
};

// MysqlChangeUserOperation is a UNIFIED class that combines ChangeUserOperation
// logic with MySQL protocol implementation.
//
// Same composition pattern as MysqlResetOperation.
//
class MysqlChangeUserOperation : public SpecialOperation {
 public:
  // Factory method that takes a Connection and handles wrapping internally
  static std::shared_ptr<MysqlChangeUserOperation> create(
      std::unique_ptr<Connection> conn,
      std::shared_ptr<const ConnectionKey> key);

  db::OperationType getOperationType() const override {
    return db::OperationType::ChangeUser;
  }

 private:
  MysqlChangeUserOperation(
      std::unique_ptr<SpecialOperationImpl> impl,
      std::shared_ptr<const ConnectionKey> key);

  InternalConnection::Status runSpecialOperation() override;

  const char* getErrorMsg() const override {
    return errorMsg;
  }

  std::shared_ptr<const ConnectionKey> key_;

  static constexpr const char* errorMsg = "Change user failed: ";
};

} // namespace facebook::common::mysql_client::mysql_protocol
