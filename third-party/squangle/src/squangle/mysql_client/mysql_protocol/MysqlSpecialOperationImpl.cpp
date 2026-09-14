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
    invokeCallbackOnce(result);
  }
}

void MysqlSpecialOperationImpl::completeOperationFromCallbackFailure(
    OperationResult result) {
  // completeOperation() runs consumer callbacks of its own and can throw, and
  // the callback is owed either way: for a reset operation the pool's whole
  // continuation lives in it, so skipping it leaves the pooled connect behind
  // the reset waiting out its timeout.  Rethrowing hands the completion failure
  // to runCallbackGuarded.
  //
  // The callback gets result(), not the result passed in.  Once the operation
  // has completed, completeOperation() is a no-op and the recorded result is
  // the real one -- reporting the recovery's failureResult instead would tell
  // the callback a succeeded operation had failed.  On the path that does
  // complete here the two are the same, because completeOperationInner() sets
  // the result on its second line, before anything that can throw.
  try {
    completeOperation(result);
  } catch (...) {
    invokeCallbackOnce(this->result());
    throw;
  }
  invokeCallbackOnce(this->result());
}

void MysqlSpecialOperationImpl::invokeCallbackOnce(OperationResult result) {
  if (callbackInvoked_ || !callback_) {
    return;
  }
  callbackInvoked_ = true;
  callback_(getOp(), result);
}

void MysqlSpecialOperationImpl::specializedCompleteOperation() {
  conn().notify();
}

void MysqlSpecialOperationImpl::specializedTimeoutTriggered() {
  completeOperation(OperationResult::TimedOut);
  // Same reason as the recovery path: completing without firing callback_
  // strands whoever is waiting on it.  A timing-out reset operation would
  // otherwise leave the pool operation behind it with neither a connection nor
  // a failure.
  invokeCallbackOnce(OperationResult::TimedOut);
}

void MysqlSpecialOperationImpl::specializedRun() {
  // Initialize EventHandler/AsyncTimeout now that we're in the event base
  // thread
  initializeFromConnection();

  const auto* mysql_conn = getMysqlConnection();
  changeHandlerFD(
      folly::NetworkSocket::fromFd(mysql_conn->getSocketDescriptor()));
  actionable();
}

} // namespace facebook::common::mysql_client::mysql_protocol
