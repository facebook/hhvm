/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/SpecialOperation.h"
#include "squangle/mysql_client/Connection.h"

namespace facebook::common::mysql_client {

SpecialOperation& SpecialOperationImpl::getOp() const {
  DCHECK(op_ && dynamic_cast<SpecialOperation*>(op_) != nullptr);
  return *(SpecialOperation*)op_;
}

void SpecialOperationImpl::actionable() {
  auto& op = getOp();
  auto status = op.callMysqlHandler();
  if (status == PENDING) {
    waitForActionable();
  } else {
    auto result = (status == DONE) ? OperationResult::Succeeded
                                   : OperationResult::Failed; // ERROR
    completeOperation(result);
    if (callback_) {
      callback_(op, result);
    }
  }
}

void SpecialOperation::mustSucceed() {
  run();
  wait();
  if (!ok()) {
    throw db::RequiredOperationFailedException(getErrorMsg() + mysql_error());
  }
}

void SpecialOperationImpl::specializedCompleteOperation() {
  conn().notify();
}

void SpecialOperationImpl::specializedTimeoutTriggered() {
  completeOperation(OperationResult::TimedOut);
}

void SpecialOperationImpl::specializedRun() {
  changeHandlerFD(folly::NetworkSocket::fromFd(conn().getSocketDescriptor()));
  actionable();
}

} // namespace facebook::common::mysql_client
