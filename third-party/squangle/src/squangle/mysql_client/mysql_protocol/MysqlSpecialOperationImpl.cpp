/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/mysql_protocol/MysqlSpecialOperationImpl.h"
#include "squangle/mysql_client/Connection.h"
#include "squangle/mysql_client/mysql_protocol/MysqlConnection.h"

namespace facebook::common::mysql_client::mysql_protocol {

void MysqlSpecialOperationImpl::actionable() {
  auto status = runSpecialOperation();
  if (status == PENDING) {
    waitForActionable();
  } else {
    auto result = (status == DONE) ? OperationResult::Succeeded
                                   : OperationResult::Failed; // ERROR
    completeOperation(result);
    if (callback_) {
      callback_(getOp(), result);
    }
  }
}

void MysqlSpecialOperationImpl::specializedCompleteOperation() {
  conn().notify();
}

void MysqlSpecialOperationImpl::specializedTimeoutTriggered() {
  completeOperation(OperationResult::TimedOut);
}

void MysqlSpecialOperationImpl::specializedRun() {
  const auto* mysql_conn = getMysqlConnection();
  changeHandlerFD(
      folly::NetworkSocket::fromFd(mysql_conn->getSocketDescriptor()));
  actionable();
}

} // namespace facebook::common::mysql_client::mysql_protocol
