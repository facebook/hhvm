/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/SyncMysqlClient.h"

namespace facebook {
namespace common {
namespace mysql_client {

std::unique_ptr<Connection> SyncMysqlClient::createConnection(
    ConnectionKey conn_key,
    MYSQL* mysql_conn) {
  return std::make_unique<SyncConnection>(this, conn_key, mysql_conn);
}

} // namespace mysql_client
} // namespace common
} // namespace facebook
