/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "squangle/mysql_client/SpecialOperation.h"

namespace facebook::common::mysql_client {

// This is for sending COM_RESET_CONNECTION command before returning an idle
// connection back to connection pool
class ResetOperation : public SpecialOperation {
 public:
  explicit ResetOperation(std::unique_ptr<ConnectionProxy> conn)
      : SpecialOperation(std::move(conn)) {}

 private:
  MysqlHandler::Status callMysqlHandler() override;

  db::OperationType getOperationType() const override {
    return db::OperationType::Reset;
  }

  const char* getErrorMsg() const override {
    return errorMsg;
  }

  static constexpr const char* errorMsg = "Reset connection failed: ";
};

} // namespace facebook::common::mysql_client
