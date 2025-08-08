/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/SpecialOperation.h"

namespace facebook::common::mysql_client {

SpecialOperation& SpecialOperationImpl::getOp() const {
  DCHECK(op_ && dynamic_cast<SpecialOperation*>(op_) != nullptr);
  return *(SpecialOperation*)op_;
}

InternalConnection::Status SpecialOperationImpl::runSpecialOperation() {
  return getOp().runSpecialOperation();
}

} // namespace facebook::common::mysql_client
