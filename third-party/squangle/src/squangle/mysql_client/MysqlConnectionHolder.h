/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <mysql.h>

#include "squangle/base/Base.h"
#include "squangle/base/ConnectionKey.h"
#include "squangle/logger/DBEventLogger.h"

namespace facebook::common::mysql_client {

class MysqlClientBase;
class AsyncConnectionPool;

// Holds the mysql connection for easier re use
class MysqlConnectionHolder {
 public:
  MysqlConnectionHolder(
      MysqlClientBase* client,
      MYSQL* mysql,
      ConnectionKey conn_key,
      bool connection_already_open = false);

  // Closes the connection in hold
  virtual ~MysqlConnectionHolder();
  const std::string& host() const {
    return conn_key_.host();
  }
  int port() const {
    return conn_key_.port();
  }
  const std::string& user() const {
    return conn_key_.user();
  }
  const std::string& database() const {
    return conn_key_.db_name();
  }
  const std::string& password() const {
    return conn_key_.password();
  }
  MYSQL* mysql() const {
    return mysql_;
  }

  void setCreationTime(Timepoint creation_time) {
    creation_time_ = creation_time;
  }

  void setReusable(bool reusable) {
    can_reuse_ = reusable;
  }

  bool isReusable() {
    return can_reuse_ && mysql_errno(mysql()) == 0;
  }

  // Don't close the mysql fd in the destructor. Useful when connections
  // are managed outside this library.
  void disableCloseOnDestroy() {
    close_fd_on_destroy_ = false;
  }

  // Returns whether or not the connection is in a transaction based on server
  // status
  bool inTransaction();

  Timepoint getCreationTime() {
    return creation_time_;
  }

  const ConnectionKey* getKey() {
    return &conn_key_;
  }

  void connectionOpened();

  bool isConnectionOpened() {
    return connection_opened_;
  }

  Timepoint getLastActivityTime() {
    return last_activity_time_;
  }

  void setLastActivityTime(Timepoint last_activity_time) {
    last_activity_time_ = last_activity_time;
  }

  void setConnectionContext(
      std::shared_ptr<db::ConnectionContextBase> conn_context) {
    conn_context_ = std::move(conn_context);
  }

  // Useful for removing the raw mysql connection and leaving this class to be
  // destroyed without closing it
  MYSQL* stealMysql() {
    auto ret = mysql_;
    mysql_ = nullptr;
    return ret;
  }

  void setNeedResetBeforeReuse() {
    needResetBeforeReuse_ = true;
  }

  bool needResetBeforeReuse() {
    return needResetBeforeReuse_;
  }

  // This constructor takes ownership of the origin holder and copies the data
  // from it, then steals the ownership of the MYSQL* connection. After that the
  // origin is deleted.
  explicit MysqlConnectionHolder(
      std::unique_ptr<MysqlConnectionHolder> from_holder,
      std::optional<ConnectionKey> connKey = std::nullopt);

 protected:
  MysqlClientBase* client_;

 private:
  // Our MYSQL handle as well as a file descriptor used for
  // notification of completed operations.
  MYSQL* mysql_;
  const ConnectionKey conn_key_;
  std::shared_ptr<db::ConnectionContextBase> conn_context_;
  Timepoint creation_time_;
  Timepoint last_activity_time_;
  bool connection_opened_ = false;
  bool close_fd_on_destroy_ = true;
  bool needResetBeforeReuse_ = false;

  bool can_reuse_;
  std::optional<std::string> current_schema_;

  // copy not allowed
  MysqlConnectionHolder() = delete;
  MysqlConnectionHolder(const MysqlConnectionHolder&) = delete;
  MysqlConnectionHolder& operator=(const MysqlConnectionHolder&) = delete;
};

} // namespace facebook::common::mysql_client
