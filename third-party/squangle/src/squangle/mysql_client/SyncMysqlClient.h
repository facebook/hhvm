/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "squangle/mysql_client/AsyncMysqlClient.h"

namespace facebook {
namespace common {
namespace mysql_client {

class SyncConnection;

class SyncMysqlClient : public MysqlClientBase {
 public:
  SyncMysqlClient() : SyncMysqlClient(nullptr) {}
  explicit SyncMysqlClient(
      std::unique_ptr<db::SquangleLoggerBase> db_logger,
      std::unique_ptr<db::DBCounterBase> db_stats =
          std::make_unique<db::SimpleDbCounter>())
      : MysqlClientBase(
            adjustLogger(std::move(db_logger)),
            std::move(db_stats)) {}

  static std::unique_ptr<db::SquangleLoggerBase> adjustLogger(
      std::unique_ptr<db::SquangleLoggerBase> logger) {
    if (logger) {
      logger->setLoggingPrefix("cpp_sync");
    }
    return logger;
  }

  db::SquangleLoggingData makeSquangleLoggingData(
      const ConnectionKey* connKey,
      const db::ConnectionContextBase* connContext) override {
    return db::SquangleLoggingData(connKey, connContext);
  }

  // Factory method
  std::unique_ptr<Connection> createConnection(
      ConnectionKey conn_key,
      MYSQL* mysql_conn) override;

  void drain(bool /*unused*/) {}

  bool supportsLocalFiles() override {
    return true;
  }

  uint64_t getPoolsConnectionLimit() {
    // This is used by HHVM in the async client.  We don't need it here in the
    // sync client.
    return std::numeric_limits<uint64_t>::max();
  }

  static std::shared_ptr<SyncMysqlClient> defaultClient();

 protected:
  // Private methods, primarily used by Operations and its subclasses.
  template <typename Client>
  friend class ConnectionPool;

  bool runInThread(folly::Cob&& fn, bool /*wait*/) override {
    fn();
    return true;
  }

  // These functions matter more to Async client, which keeps track of
  // existing operations.
  void activeConnectionAdded(const ConnectionKey*) override {}
  void activeConnectionRemoved(const ConnectionKey*) override {}
  void addOperation(std::shared_ptr<Operation>) override {}
  void deferRemoveOperation(Operation*) override {}

 private:
  MysqlHandler& getMysqlHandler() override {
    return mysql_handler_;
  }

  // Sync implementation of mysql handler interface
  class SyncMysqlHandler : public MysqlHandler {
    Status tryConnect(
        MYSQL* mysql,
        const ConnectionOptions& opts,
        const ConnectionKey& key,
        int flags) override {
      auto qtmo = std::chrono::duration_cast<std::chrono::milliseconds>(
                      opts.getQueryTimeout())
                      .count();
      auto ctmo = std::chrono::duration_cast<std::chrono::milliseconds>(
                      opts.getTimeout())
                      .count();

      mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT_MS, &ctmo);
      mysql_options(mysql, MYSQL_OPT_READ_TIMEOUT_MS, &qtmo);
      mysql_options(mysql, MYSQL_OPT_WRITE_TIMEOUT_MS, &qtmo);

      const auto usingUnixSocket = !key.unixSocketPath().empty();

      // When using unix socket (AF_UNIX), host/port do not matter.
      const auto rv = mysql_real_connect(
          mysql,
          usingUnixSocket ? nullptr : key.host().c_str(),
          key.user().c_str(),
          key.password().c_str(),
          key.db_name().c_str(),
          usingUnixSocket ? 0 : key.port(),
          usingUnixSocket ? key.unixSocketPath().c_str() : nullptr,
          flags);
      return rv == nullptr ? ERROR : DONE;
    }

    Status runQuery(MYSQL* mysql, folly::StringPiece queryStmt) override {
      return mysql_real_query(mysql, queryStmt.begin(), queryStmt.size())
          ? ERROR
          : DONE;
    }
    Status nextResult(MYSQL* mysql) override {
      return mysql_next_result(mysql) ? ERROR : DONE;
    }
    Status fetchRow(MYSQL_RES* res, MYSQL_ROW& row) override {
      row = mysql_fetch_row(res);
      return DONE;
    }
    MYSQL_RES* getResult(MYSQL* mysql) override {
      return mysql_store_result(mysql);
    }
    Status resetConn(MYSQL* mysql) override {
      return mysql_reset_connection(mysql) ? ERROR : DONE;
    }
    Status changeUser(
        MYSQL* mysql,
        const std::string& user,
        const std::string& password,
        const std::string& database) override {
      return mysql_change_user(
                 mysql, user.c_str(), password.c_str(), database.c_str())
          ? ERROR
          : DONE;
    }
  } mysql_handler_;
};

// SyncConnection is a specialization of Connection to handle inline loops.
// It has its own EventBase and Operations using it will have events and
// callbacks running with this EventBase.
class SyncConnection : public Connection {
 public:
  SyncConnection(
      MysqlClientBase* client,
      ConnectionKey conn_key,
      std::unique_ptr<MysqlConnectionHolder> conn)
      : Connection(client, conn_key, std::move(conn)) {}

  SyncConnection(MysqlClientBase* client, ConnectionKey conn_key, MYSQL* conn)
      : Connection(client, conn_key, conn) {}

  ~SyncConnection();

  // Operations call these methods as the operation becomes unblocked, as
  // callers want to wait for completion, etc.
  void notify() override {
    // Nop
  }

  void wait() override {
    // Nop
  }

  // Called when a new operation is being started.
  void resetActionable() override {
    // Nop
  }

  bool runInThread(folly::Cob&& fn) override {
    fn();
    return true;
  }

  std::shared_ptr<MultiQueryStreamOperation> createOperation(
      Operation::ConnectionProxy&& proxy,
      MultiQuery&& multi_query) override {
    return std::make_shared<MultiQueryStreamOperation>(
        std::move(proxy), std::move(multi_query));
  }
};
} // namespace mysql_client
} // namespace common
} // namespace facebook
