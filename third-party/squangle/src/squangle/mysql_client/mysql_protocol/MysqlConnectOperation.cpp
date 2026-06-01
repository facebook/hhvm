/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/mysql_protocol/MysqlConnectOperation.h"
#include "squangle/mysql_client/mysql_protocol/MysqlConnectOperationImpl.h"

namespace facebook::common::mysql_client::mysql_protocol {

std::shared_ptr<MysqlConnectOperation> MysqlConnectOperation::create(
    MysqlClientBase* client,
    std::shared_ptr<const ConnectionKey> conn_key) {
  auto impl =
      std::make_unique<MysqlConnectOperationImpl>(client, std::move(conn_key));
  return std::shared_ptr<MysqlConnectOperation>(
      new MysqlConnectOperation(std::move(impl)));
}

MysqlConnectOperation::MysqlConnectOperation(
    std::unique_ptr<ConnectOperationImpl> impl)
    : ConnectOperation(std::move(impl)) {}

} // namespace facebook::common::mysql_client::mysql_protocol
