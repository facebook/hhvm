/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "squangle/mysql_client/MysqlHandler.h"
#include "squangle/mysql_client/Operation.h"

namespace facebook::common::mysql_client {

class ConnectionProxy;
class SpecialOperation;

using SpecialOperationCallback =
    std::function<void(SpecialOperation&, OperationResult)>;

// SpecialOperation means operations like COM_RESET_CONNECTION,
// COM_CHANGE_USER, etc.
class SpecialOperation : public Operation {
 public:
  explicit SpecialOperation(std::unique_ptr<ConnectionProxy> conn)
      : Operation(std::move(conn)) {}

  void setCallback(SpecialOperationCallback callback) {
    callback_ = std::move(callback);
  }

 protected:
  void actionable() override;
  void specializedCompleteOperation() override;
  void specializedTimeoutTriggered() override;
  SpecialOperation& specializedRun() override;
  void mustSucceed() override;

  SpecialOperationCallback callback_{nullptr};
  friend class Connection;

 private:
  virtual MysqlHandler::Status callMysqlHandler() = 0;
  virtual const char* getErrorMsg() const = 0;
};

} // namespace facebook::common::mysql_client
