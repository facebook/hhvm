/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/EventBase.h>

#include "squangle/base/ConnectionKey.h"
#include "squangle/logger/DBEventLogger.h"
#include "squangle/mysql_client/MysqlExceptionBuilder.h"
#include "squangle/mysql_client/Operation.h"

namespace facebook::common::mysql_client {

class Connection;
class ConnectOperation;
class ConnectOperationImpl;
template <typename Client>
class ConnectPoolOperationImpl;
class FetchOperationImpl;
class SpecialOperationImpl;

class MysqlClientBase {
 public:
  virtual ~MysqlClientBase() = default;

  // Initiate a connection to a database.  This is the main entrypoint.
  virtual std::shared_ptr<ConnectOperation> beginConnection(
      const std::string& host,
      int port,
      const std::string& database_name,
      const std::string& user,
      const std::string& password);

  std::shared_ptr<ConnectOperation> beginConnection(
      std::shared_ptr<const ConnectionKey> conn_key);

  // Factory method
  virtual std::unique_ptr<Connection> createConnection(
      std::shared_ptr<const ConnectionKey> conn_key) = 0;

  virtual folly::EventBase* getEventBase() {
    return nullptr;
  }

  virtual const folly::EventBase* getEventBase() const {
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
      std::shared_ptr<const ConnectionKey> conn_key,
      const db::ConnectionContextBase* extra_logging_data);

  void logConnectionFailure(
      const db::CommonLoggingData& logging_data,
      db::FailureReason reason,
      std::shared_ptr<const ConnectionKey> conn_key,
      unsigned int mysqlErrno,
      const std::string& error,
      const db::ConnectionContextBase* extra_logging_data);

  db::DBCounterBase* stats() {
    return client_stats_.get();
  }
  db::SquangleLoggerBase* dbLogger() {
    return db_logger_.get();
  }
  const MysqlExceptionBuilder& exceptionBuilder() {
    return *exception_builder_;
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
      connection_cb_ = [old_cb = connection_cb_,
                        new_cb = std::move(connection_cb)](Operation& op) {
        old_cb(op);
        new_cb(op);
      };
    } else {
      connection_cb_ = std::move(connection_cb);
    }
  }

  explicit MysqlClientBase(
      std::unique_ptr<db::SquangleLoggerBase> db_logger = nullptr,
      std::unique_ptr<db::DBCounterBase> db_stats =
          std::make_unique<db::SimpleDbCounter>(),
      std::unique_ptr<const MysqlExceptionBuilder> exception_builder = nullptr);

  virtual bool runInThread(std::function<void()>&& fn, bool /*wait*/ = false) {
    fn();
    return true;
  }

  virtual uint32_t numStartedAndOpenConnections() {
    return 0;
  }
  virtual Duration callbackDelayAvg() const {
    return Duration(0);
  }

  virtual bool supportsLocalFiles() = 0;

  static constexpr bool implementsPooling() {
    return false;
  }

  virtual std::unique_ptr<ConnectOperationImpl> createConnectOperationImpl(
      MysqlClientBase* client,
      std::shared_ptr<const ConnectionKey> conn_key) const = 0;
  virtual std::unique_ptr<FetchOperationImpl> createFetchOperationImpl(
      std::unique_ptr<OperationBase::ConnectionProxy> conn) const = 0;
  virtual std::unique_ptr<SpecialOperationImpl> createSpecialOperationImpl(
      std::unique_ptr<OperationBase::ConnectionProxy> conn) const = 0;

  // Helper versions of the above that take a Connection instead of a
  // ConnectionProxy
  std::unique_ptr<FetchOperationImpl> createFetchOperationImpl(
      std::unique_ptr<Connection> conn) const;
  std::unique_ptr<SpecialOperationImpl> createSpecialOperationImpl(
      std::unique_ptr<Connection> conn) const;

  virtual void activeConnectionAdded(
      std::shared_ptr<const ConnectionKey> /*key*/) {}
  virtual void activeConnectionRemoved(
      std::shared_ptr<const ConnectionKey> /*key*/) {}

  virtual bool isInCorrectThread(bool /*expectMysqlThread*/) const {
    return true;
  }

  virtual std::string_view getBaseClient() const noexcept = 0;

 protected:
  friend class Connection;
  friend class OperationBase;
  friend class OperationImpl;
  friend class ConnectOperationImpl;
  template <typename Class>
  friend class ConnectPoolOperation;
  template <typename Class>
  friend class ConnectionPool;
  friend class FetchOperationImpl;
  friend class SpecialOperationImpl;
  friend class ResetOperation;
  friend class ChangeUserOperation;
  friend class ConnectionHolder;
  friend class AsyncConnection;
  friend class SyncConnection;
  virtual db::SquangleLoggingData makeSquangleLoggingData(
      std::shared_ptr<const ConnectionKey> connKey,
      const db::ConnectionContextBase* connContext) {
    return db::SquangleLoggingData(std::move(connKey), connContext);
  }

  virtual void addOperation(std::shared_ptr<Operation> /*op*/) {}
  virtual void deferRemoveOperation(Operation* /*op*/) {}

  // Using unique pointer due inheritance virtual calls
  std::unique_ptr<db::SquangleLoggerBase> db_logger_;
  std::unique_ptr<db::DBCounterBase> client_stats_;
  ObserverCallback connection_cb_;
  std::unique_ptr<const MysqlExceptionBuilder> exception_builder_;
};

} // namespace facebook::common::mysql_client
