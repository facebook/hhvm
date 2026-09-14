/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "squangle/mysql_client/SpecialOperation.h"
#include "squangle/mysql_client/mysql_protocol/MysqlOperationImpl.h"

namespace facebook::common::mysql_client::mysql_protocol {

class MysqlSpecialOperationImpl : public MysqlOperationImpl,
                                  public SpecialOperationImpl {
 public:
  explicit MysqlSpecialOperationImpl(
      std::unique_ptr<ConnectionProxy> conn,
      db::OperationType operation_type)
      : OperationBase(std::move(conn)), SpecialOperationImpl(operation_type) {}

 protected:
  void actionable() override;
  void specializedCompleteOperation() override;
  void specializedTimeoutTriggered() override;
  void specializedRun() override;

  // callback_ is invoked from actionable(), not from completeOperation(), so a
  // completion that does not run through actionable() would never fire it.
  // For a reset operation that strands a second operation: the pool puts its
  // whole continuation in this callback (ConnectionPool::resetConnection), so
  // the pool operation waiting on the reset would get neither a connection nor
  // a failure.
  void completeOperationFromCallbackFailure(OperationResult result) override;

 private:
  // Fires callback_ at most once, whichever path gets there first.  The
  // recovery above cannot tell "completed but the callback never ran" from
  // "the callback itself threw" by state alone.
  void invokeCallbackOnce(OperationResult result);

  bool callbackInvoked_ = false;
};

} // namespace facebook::common::mysql_client::mysql_protocol
