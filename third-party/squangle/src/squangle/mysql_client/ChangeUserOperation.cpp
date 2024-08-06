/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/ChangeUserOperation.h"
#include "squangle/mysql_client/Connection.h"

namespace facebook::common::mysql_client {

MysqlHandler::Status ChangeUserOperation::callMysqlHandler() {
  auto& handler = conn().client().getMysqlHandler();
  return handler.changeUser(
      conn().getInternalConnection(), user_, password_, database_);
}

} // namespace facebook::common::mysql_client
