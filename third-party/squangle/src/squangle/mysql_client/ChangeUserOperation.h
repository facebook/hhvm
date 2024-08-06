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

class ChangeUserOperation : public SpecialOperation {
 public:
  explicit ChangeUserOperation(
      std::unique_ptr<ConnectionProxy> conn,
      const std::string& user,
      const std::string& password,
      const std::string& database)
      : SpecialOperation(std::move(conn)),
        user_(user),
        password_(password),
        database_(database) {}

 private:
  MysqlHandler::Status callMysqlHandler() override;

  db::OperationType getOperationType() const override {
    return db::OperationType::ChangeUser;
  }

  const char* getErrorMsg() const override {
    return errorMsg;
  }

  const std::string user_;
  const std::string password_;
  const std::string database_;

  static constexpr const char* errorMsg = "Change user failed: ";
};

} // namespace facebook::common::mysql_client
