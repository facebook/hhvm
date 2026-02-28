/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/ResetOperation.h"
#include "squangle/mysql_client/Connection.h"

namespace facebook::common::mysql_client {

InternalConnection::Status ResetOperation::runSpecialOperation() {
  return conn().getInternalConnection().resetConn();
}

} // namespace facebook::common::mysql_client
