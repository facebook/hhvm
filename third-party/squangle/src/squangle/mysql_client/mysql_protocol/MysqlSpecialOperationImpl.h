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
  explicit MysqlSpecialOperationImpl(std::unique_ptr<ConnectionProxy> conn)
      : OperationBase(std::move(conn)) {}

 protected:
  void actionable() override;
  void specializedCompleteOperation() override;
  void specializedTimeoutTriggered() override;
  void specializedRun() override;
};

} // namespace facebook::common::mysql_client::mysql_protocol
