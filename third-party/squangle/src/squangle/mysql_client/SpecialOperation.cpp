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

void SpecialOperation::actionable() {
  auto status = callMysqlHandler();
  if (status == PENDING) {
    waitForActionable();
  } else {
    auto result = (status == DONE) ? OperationResult::Succeeded
                                   : OperationResult::Failed; // ERROR
    completeOperation(result);
    if (callback_) {
      callback_(*this, result);
    }
  }
}

void SpecialOperation::mustSucceed() {
  run();
  wait();
  if (!ok()) {
    throw db::RequiredOperationFailedException(getErrorMsg() + mysql_error_);
  }
}

void SpecialOperation::specializedCompleteOperation() {
  conn().notify();
}

void SpecialOperation::specializedTimeoutTriggered() {
  completeOperation(OperationResult::TimedOut);
}

SpecialOperation& SpecialOperation::specializedRun() {
  changeHandlerFD(folly::NetworkSocket::fromFd(conn().getSocketDescriptor()));
  actionable();
  return *this;
}

} // namespace facebook::common::mysql_client
