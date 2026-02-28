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
      std::unique_ptr<SpecialOperationImpl> impl,
      std::shared_ptr<const ConnectionKey> key)
      : SpecialOperation(std::move(impl)), key_(std::move(key)) {}

 private:
  InternalConnection::Status runSpecialOperation() override;

  db::OperationType getOperationType() const override {
    return db::OperationType::ChangeUser;
  }

  const char* getErrorMsg() const override {
    return errorMsg;
  }

  std::shared_ptr<const ConnectionKey> key_;

  static constexpr const char* errorMsg = "Change user failed: ";
};

} // namespace facebook::common::mysql_client
