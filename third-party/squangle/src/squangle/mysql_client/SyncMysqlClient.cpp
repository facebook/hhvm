/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Singleton.h>
#include <memory>

#include "squangle/mysql_client/ResetOperation.h"
#include "squangle/mysql_client/SyncMysqlClient.h"

namespace facebook::common::mysql_client {

namespace {
folly::Singleton<SyncMysqlClient> client([]() { return new SyncMysqlClient; });
} // namespace

std::shared_ptr<SyncMysqlClient> SyncMysqlClient::defaultClient() {
  return folly::Singleton<SyncMysqlClient>::try_get();
}

std::unique_ptr<Connection> SyncMysqlClient::createConnection(
    std::shared_ptr<const ConnectionKey> conn_key) {
  return std::make_unique<SyncConnection>(*this, std::move(conn_key));
}

SyncConnection::~SyncConnection() {
  if (mysql_connection_ && conn_dying_callback_ && needToCloneConnection_ &&
      isReusable() && !inTransaction() &&
      getConnectionOptions().isEnableResetConnBeforeClose()) {
    // We clone this Connection object to send COM_RESET_CONNECTION command
    // via the connection before returning it to the connection pool.
    // The callback function points to recycleMysqlConnection(), which is
    // responsible for recyclining the connection.
    // This object's callback is set to null and the cloned object's
    // callback instead points to the original callback function, which will
    // be called after COM_RESET_CONNECTION.

    auto connHolder = stealConnectionHolder(true);
    auto conn = std::make_unique<SyncConnection>(
        client(), getKey(), std::move(connHolder));
    conn->needToCloneConnection_ = false;
    conn->setConnectionOptions(getConnectionOptions());
    conn->setConnectionDyingCallback(std::move(conn_dying_callback_));
    conn_dying_callback_ = nullptr;
    auto resetOp = Connection::resetConn(std::move(conn));
    // addOperation() is necessary here for proper cancelling of reset
    // operation in case of sudden SyncMysqlClient shutdown
    client().addOperation(resetOp);
    resetOp->run().wait();
  }
}

MysqlHandler::Status SyncMysqlClient::SyncMysqlHandler::tryConnect(
    const InternalConnection& conn,
    const ConnectionOptions& opts,
    std::shared_ptr<const ConnectionKey> key,
    int flags) {
  auto qtmo = std::chrono::duration_cast<Millis>(opts.getQueryTimeout());
  auto ctmo = std::chrono::duration_cast<Millis>(opts.getTimeout());

  conn.setConnectTimeout(ctmo);
  conn.setReadTimeout(qtmo);
  conn.setWriteTimeout(qtmo);

  return conn.tryConnect(opts, std::move(key), flags);
}

} // namespace facebook::common::mysql_client
