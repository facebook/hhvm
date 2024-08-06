/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/EventBase.h>
#include <mysql.h>

#include "squangle/base/ConnectionKey.h"
#include "squangle/logger/DBEventLogger.h"
#include "squangle/mysql_client/Operation.h"

namespace facebook::common::mysql_client {

class Connection;
class ConnectOperation;
class MysqlHandler;

class MysqlClientBase {
 public:
  virtual ~MysqlClientBase() = default;

  // Initiate a connection to a database.  This is the main entrypoint.
  std::shared_ptr<ConnectOperation> beginConnection(
      const std::string& host,
      int port,
      const std::string& database_name,
      const std::string& user,
      const std::string& password);

  std::shared_ptr<ConnectOperation> beginConnection(ConnectionKey conn_key);

  // Factory method
  virtual std::unique_ptr<Connection> createConnection(
      ConnectionKey conn_key,
      MYSQL* mysql_conn) = 0;

  virtual folly::EventBase* getEventBase() {
    return nullptr;
  }

  void logQuerySuccess(
      const db::QueryLoggingData& logging_data,
      const Connection& conn);

  void logQueryFailure(
      const db::QueryLoggingData& logging_data,
      db::FailureReason reason,
      unsigned int mysqlErrno,
      const std::string& error,
      const Connection& conn);

  void logConnectionSuccess(
      const db::CommonLoggingData& logging_data,
      const ConnectionKey& conn_key,
      const db::ConnectionContextBase* extra_logging_data);

  void logConnectionFailure(
      const db::CommonLoggingData& logging_data,
      db::FailureReason reason,
      const ConnectionKey& conn_key,
      unsigned int mysqlErrno,
      const std::string& error,
      const db::ConnectionContextBase* extra_logging_data);

  db::DBCounterBase* stats() {
    return client_stats_.get();
  }
  db::SquangleLoggerBase* dbLogger() {
    return db_logger_.get();
  }

  // For internal (testing) use only
  std::unique_ptr<db::SquangleLoggerBase> setDBLoggerForTesting(
      std::unique_ptr<db::SquangleLoggerBase> dbLogger) {
    std::swap(db_logger_, dbLogger);
    return dbLogger;
  }
  std::unique_ptr<db::DBCounterBase> setDBCounterForTesting(
      std::unique_ptr<db::DBCounterBase> dbCounter) {
    std::swap(client_stats_, dbCounter);
    return dbCounter;
  }

  void setConnectionCallback(ObserverCallback connection_cb) {
    if (connection_cb_) {
      auto old_cb = connection_cb_;
      connection_cb_ = [old_cb, connection_cb](Operation& op) {
        old_cb(op);
        connection_cb(op);
      };
    } else {
      connection_cb_ = connection_cb;
    }
  }

  explicit MysqlClientBase(
      std::unique_ptr<db::SquangleLoggerBase> db_logger = nullptr,
      std::unique_ptr<db::DBCounterBase> db_stats =
          std::make_unique<db::SimpleDbCounter>());

  virtual bool runInThread(folly::Cob&& fn, bool wait = false) = 0;

  virtual uint32_t numStartedAndOpenConnections() {
    return 0;
  }
  virtual double callbackDelayMicrosAvg() {
    return 0.0;
  }

  virtual bool supportsLocalFiles() = 0;

  static constexpr bool implementsPooling() {
    return false;
  }

 protected:
  friend class Connection;
  friend class Operation;
  friend class ConnectOperation;
  template <typename Class>
  friend class ConnectPoolOperation;
  template <typename Class>
  friend class ConnectionPool;
  friend class FetchOperation;
  friend class SpecialOperation;
  friend class ResetOperation;
  friend class ChangeUserOperation;
  friend class ConnectionHolder;
  friend class AsyncConnection;
  friend class SyncConnection;
  virtual db::SquangleLoggingData makeSquangleLoggingData(
      const ConnectionKey& connKey,
      const db::ConnectionContextBase* connContext) = 0;

  virtual void activeConnectionAdded(const ConnectionKey* key) = 0;
  virtual void activeConnectionRemoved(const ConnectionKey* key) = 0;
  virtual void addOperation(std::shared_ptr<Operation> op) = 0;
  virtual void deferRemoveOperation(Operation* op) = 0;

  virtual MysqlHandler& getMysqlHandler() = 0;

  // Using unique pointer due inheritance virtual calls
  std::unique_ptr<db::SquangleLoggerBase> db_logger_;
  std::unique_ptr<db::DBCounterBase> client_stats_;
  ObserverCallback connection_cb_;
};

} // namespace facebook::common::mysql_client
