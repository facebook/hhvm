/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/MysqlConnectionHolder.h"
#include "squangle/mysql_client/AsyncMysqlClient.h"

namespace facebook::common::mysql_client {

MysqlConnectionHolder::MysqlConnectionHolder(
    MysqlClientBase* client,
    MYSQL* mysql,
    ConnectionKey conn_key,
    bool already_open)
    : client_(client),
      mysql_(mysql),
      conn_key_(std::move(conn_key)),
      conn_context_(nullptr),
      connection_opened_(already_open),
      can_reuse_(true) {
  client_->activeConnectionAdded(&conn_key_);
  creation_time_ = std::chrono::steady_clock::now();
}

MysqlConnectionHolder::MysqlConnectionHolder(
    std::unique_ptr<MysqlConnectionHolder> from_holder,
    std::optional<ConnectionKey> connKey)
    : client_(from_holder->client_),
      conn_key_(
          connKey ? std::move(*connKey) : folly::copy(from_holder->conn_key_)),
      conn_context_(from_holder->conn_context_),
      creation_time_(from_holder->creation_time_),
      last_activity_time_(from_holder->last_activity_time_),
      connection_opened_(from_holder->connection_opened_),
      can_reuse_(from_holder->can_reuse_) {
  mysql_ = from_holder->stealMysql();
  client_->activeConnectionAdded(&conn_key_);
}

bool MysqlConnectionHolder::inTransaction() {
  return mysql_->server_status & SERVER_STATUS_IN_TRANS;
}

MysqlConnectionHolder::~MysqlConnectionHolder() {
  if (close_fd_on_destroy_ && mysql_) {
    // Close our connection in the thread from which it was created.
    if (!client_->runInThread([mysql = mysql_]() {
          // Unregister server cert validation callback
          const void* callback{nullptr};
          mysql_options(mysql, MYSQL_OPT_TLS_CERT_CALLBACK, &callback);
          mysql_close(mysql);
        })) {
      LOG(DFATAL)
          << "Mysql connection couldn't be closed: error in folly::EventBase";
    }
    if (connection_opened_) {
      client_->stats()->incrClosedConnections(conn_context_.get());
    }
  }
  client_->activeConnectionRemoved(&conn_key_);
}

void MysqlConnectionHolder::connectionOpened() {
  connection_opened_ = true;
  last_activity_time_ = std::chrono::steady_clock::now();

  client_->stats()->incrOpenedConnections(conn_context_.get());
}

} // namespace facebook::common::mysql_client
