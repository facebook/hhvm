/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/futures/Future.h>
#include <folly/io/async/EventBase.h>
#include <mysql.h>

#include "squangle/base/Base.h"
#include "squangle/mysql_client/MysqlClientBase.h"
#include "squangle/mysql_client/MysqlConnectionHolder.h"
#include "squangle/mysql_client/Operation.h"
#include "squangle/mysql_client/Query.h"

namespace facebook::common::mysql_client {

class QueryOperation;
class MultiQueryOperation;
class MultiQueryStreamOperation;

using ConnectionDyingCallback =
    std::function<void(std::unique_ptr<MysqlConnectionHolder>)>;

// A helper class to interface with the EventBase.  Each connection
// has an instance of this class and this class is what is invoked
// when sockets become readable/writable or when a timeout occurs.
// This is a separate class to avoid polluting the class hierarchy.
class ConnectionSocketHandler : public folly::EventHandler,
                                public folly::AsyncTimeout {
 public:
  explicit ConnectionSocketHandler(folly::EventBase* base);
  void timeoutExpired() noexcept override;
  void handlerReady(uint16_t events) noexcept override;
  void setOperation(Operation* op) {
    op_ = op;
  }

 private:
  Operation* op_;

  ConnectionSocketHandler() = delete;
  ConnectionSocketHandler(const ConnectionSocketHandler&) = delete;
  ConnectionSocketHandler& operator=(const ConnectionSocketHandler&) = delete;
};

// Connection is a thin wrapper around a MYSQL object, associating it
// with an AsyncMysqlClient.  Its primary purpose is to manage that
// connection and initiate queries.
//
// It also holds a notification descriptor, used across queries, to
// signal their completion.  Operation::wait blocks on this fd.
class Connection {
 public:
  Connection(
      MysqlClientBase* mysql_client,
      ConnectionKey conn_key,
      std::unique_ptr<MysqlConnectionHolder> conn)
      : mysql_connection_(std::move(conn)),
        conn_key_(std::move(conn_key)),
        mysql_client_(mysql_client),
        socket_handler_(mysql_client_->getEventBase()),
        initialized_(false) {}

  Connection(
      MysqlClientBase* mysql_client,
      ConnectionKey conn_key,
      MYSQL* existing_conn)
      : conn_key_(std::move(conn_key)),
        mysql_client_(mysql_client),
        socket_handler_(mysql_client_->getEventBase()),
        initialized_(false) {
    if (existing_conn) {
      mysql_connection_ = std::make_unique<MysqlConnectionHolder>(
          mysql_client_, existing_conn, conn_key_);
    }
  }

  virtual ~Connection();

  // Like beginConnection, this is how you start a query.  Note that
  // ownership of the Connection is passed into this function; the
  // returned QueryOperation allows access to it (once the query
  // completes).  This is a limitation of MySQL as you cannot perform
  // operations while a query is in progress.  We use unique_ptr to
  // represent this connection-level statefulness.
  //
  // To run subsequent queries, after query_op->wait() returns, you
  // can call query_op->releaseConnection() to retrieve the connection
  // itself and run further queries.
  //
  // The query itself is constructed from args....  If args... is a single
  // Query object, it is used directly; otherwise a Query object is
  // constructed via Query(args...) and that is used for the query.
  template <typename... Args>
  static std::shared_ptr<QueryOperation> beginQuery(
      std::unique_ptr<Connection> conn,
      Args&&... args);

  template <typename... Args>
  static std::shared_ptr<MultiQueryOperation> beginMultiQuery(
      std::unique_ptr<Connection> conn,
      Args&&... args);

  FOLLY_NODISCARD static folly::SemiFuture<DbQueryResult> querySemiFuture(
      std::unique_ptr<Connection> conn,
      Query&& query,
      QueryCallback&& cb,
      QueryOptions&& options = QueryOptions());

  FOLLY_NODISCARD static folly::SemiFuture<DbQueryResult> querySemiFuture(
      std::unique_ptr<Connection> conn,
      Query&& query,
      QueryOptions&& options = QueryOptions()) {
    return querySemiFuture(
        std::move(conn), std::move(query), nullptr, std::move(options));
  }

  FOLLY_NODISCARD static folly::SemiFuture<DbMultiQueryResult>
  multiQuerySemiFuture(
      std::unique_ptr<Connection> conn,
      Query&& query,
      MultiQueryCallback&& cb,
      QueryOptions&& options = QueryOptions());

  FOLLY_NODISCARD static folly::SemiFuture<DbMultiQueryResult>
  multiQuerySemiFuture(
      std::unique_ptr<Connection> conn,
      Query&& query,
      QueryOptions&& options = QueryOptions()) {
    return multiQuerySemiFuture(
        std::move(conn), std::move(query), nullptr, std::move(options));
  }

  FOLLY_NODISCARD static folly::SemiFuture<DbMultiQueryResult>
  multiQuerySemiFuture(
      std::unique_ptr<Connection> conn,
      std::vector<Query>&& queries,
      MultiQueryCallback&& cb,
      QueryOptions&& options = QueryOptions());

  FOLLY_NODISCARD static folly::SemiFuture<DbMultiQueryResult>
  multiQuerySemiFuture(
      std::unique_ptr<Connection> conn,
      std::vector<Query>&& queries,
      QueryOptions&& options = QueryOptions()) {
    return multiQuerySemiFuture(
        std::move(conn), std::move(queries), nullptr, std::move(options));
  }

  // An alternate interface that allows for easier re-use of an
  // existing query_op, moving the Connection from the old op and into
  // the new one.  See details above for what args... are.
  template <typename... Args>
  static std::shared_ptr<QueryOperation> beginQuery(
      std::shared_ptr<QueryOperation>& op,
      Args&&... args) {
    CHECK_THROW(op->done(), db::OperationStateException);
    auto conn = std::move(op->releaseConnection());
    op = beginQuery(std::move(conn), std::forward<Args>(args)...);
    return op;
  }

  // Experimental
  virtual std::shared_ptr<MultiQueryStreamOperation> createOperation(
      Operation::ConnectionProxy&& proxy,
      MultiQuery&& multi_query) {
    return std::make_shared<MultiQueryStreamOperation>(
        std::move(proxy), std::move(multi_query));
  }

  template <typename... Args>
  static std::shared_ptr<MultiQueryStreamOperation> beginMultiQueryStreaming(
      std::unique_ptr<Connection> conn,
      Args&&... args);

  // Synchronous calls
  template <typename... Args>
  DbQueryResult query(Args&&... args);

  template <typename... Args>
  DbMultiQueryResult multiQuery(Args&&... args);

  // EXPERIMENTAL

  // StreamResultHandler
  static MultiQueryStreamHandler streamMultiQuery(
      std::unique_ptr<Connection> connection,
      std::vector<Query>&& queries,
      const AttributeMap& attributes = AttributeMap());

  static MultiQueryStreamHandler streamMultiQuery(
      std::unique_ptr<Connection> connection,
      MultiQuery&& multi_query,
      const AttributeMap& attributes = AttributeMap());

  // variant that takes a QueryOperation for more convenient chaining of
  // queries.
  //
  // These return QueryOperations that are used to verify success or
  // failure.
  static std::shared_ptr<QueryOperation> beginTransaction(
      std::unique_ptr<Connection> conn);
  static std::shared_ptr<QueryOperation> rollbackTransaction(
      std::unique_ptr<Connection> conn);
  static std::shared_ptr<QueryOperation> commitTransaction(
      std::unique_ptr<Connection> conn);

  static std::shared_ptr<QueryOperation> beginTransaction(
      std::shared_ptr<QueryOperation>& op);
  static std::shared_ptr<QueryOperation> rollbackTransaction(
      std::shared_ptr<QueryOperation>& op);
  static std::shared_ptr<QueryOperation> commitTransaction(
      std::shared_ptr<QueryOperation>& op);

  // Called in the libevent thread to create the MYSQL* client.
  void initMysqlOnly();
  void initialize(bool initMysql = true);

  bool hasInitialized() const {
    return initialized_;
  }

  bool ok() const {
    return mysql_connection_ != nullptr;
  }

  void close() {
    if (mysql_connection_) {
      mysql_connection_.reset();
    }
  }

  // Default timeout for queries created by this client.
  void setDefaultQueryTimeout(Duration t) {
    conn_options_.setQueryTimeout(t);
  }
  // TODO #9834064
  void setQueryTimeout(Duration t) {
    conn_options_.setQueryTimeout(t);
  }

  // set last successful query time to MysqlConnectionHolder
  void setLastActivityTime(Timepoint last_activity_time) {
    CHECK_THROW(mysql_connection_ != nullptr, db::InvalidConnectionException);
    mysql_connection_->setLastActivityTime(last_activity_time);
  }

  Timepoint getLastActivityTime() const {
    CHECK_THROW(mysql_connection_ != nullptr, db::InvalidConnectionException);
    return mysql_connection_->getLastActivityTime();
  }

  // Returns the MySQL server version. If the connection has been closed
  // an error is generated.
  std::string serverInfo() const {
    CHECK_THROW(mysql_connection_ != nullptr, db::InvalidConnectionException);
    auto ret = mysql_get_server_info(mysql_connection_->mysql());
    return std::string(ret);
  }

  // Returns whether or not the SSL session was reused from a previous
  // connection.
  // If the connection isn't SSL, it will return false as well.
  bool sslSessionReused() const {
    CHECK_THROW(mysql_connection_ != nullptr, db::InvalidConnectionException);
    return mysql_get_ssl_session_reused(mysql_connection_->mysql());
  }

  // Checks if `client_flag` is set for SSL.
  bool isSSL() const;

  // Escape the provided string using mysql_real_escape_string(). You almost
  // certainly don't want to use this - look at the Query class instead.
  //
  // This is provided so that non-Facebook users of the HHVM extension have
  // a familiar API.
  std::string escapeString(folly::StringPiece unescaped) const {
    CHECK_THROW(mysql_connection_ != nullptr, db::InvalidConnectionException);
    return Query::escapeString<std::string>(
        mysql_connection_->mysql(), unescaped);
  }

  // Returns the number of errors, warnings, and notes generated during
  // execution of the previous SQL statement
  int warningCount() const {
    CHECK_THROW(mysql_connection_ != nullptr, db::InvalidConnectionException);
    return mysql_warning_count(mysql_connection_->mysql());
  }

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

  MysqlClientBase* client() const {
    return mysql_client_;
  }

  long mysqlThreadId() const {
    return mysql_thread_id(mysql_connection_->mysql());
  }

  void disableCloseOnDestroy() {
    if (mysql_connection_) {
      mysql_connection_->disableCloseOnDestroy();
    }
  }

  MYSQL* stealMysql() {
    if (mysql_connection_) {
      auto ret = mysql_connection_->stealMysql();
      mysql_connection_.reset();
      return ret;
    } else {
      return nullptr;
    }
  }

  MysqlConnectionHolder* mysql_for_testing_only() const {
    return mysql_connection_.get();
  }

  std::unique_ptr<MysqlConnectionHolder> stealMysqlConnectionHolder(
      bool skipCheck = false) {
    if (!skipCheck) {
      DCHECK(isInEventBaseThread());
    }
    return std::move(mysql_connection_);
  }

  const ConnectionKey* getKey() const {
    return &conn_key_;
  }

  void setReusable(bool reusable) {
    if (mysql_connection_) {
      mysql_connection_->setReusable(reusable);
    }
  }

  bool isReusable() const {
    if (mysql_connection_) {
      return mysql_connection_->isReusable();
    }
    return false;
  }

  // Don't actually store the new schema at this point - just the presence of an
  // updated schema is sufficient to indicate we shouldn't reuse the connection
  // for pooling as it means that someone ran a "USE <dbname>" command.
  void setCurrentSchema(std::string_view /*schema*/) const {
    CHECK_THROW(mysql_connection_ != nullptr, db::InvalidConnectionException);
    mysql_connection_->setReusable(false);
  }

  bool inTransaction() const {
    CHECK_THROW(mysql_connection_ != nullptr, db::InvalidConnectionException);
    return mysql_connection_->inTransaction();
  }

  const ConnectionOptions& getConnectionOptions() const {
    return conn_options_;
  }

  void setConnectionOptions(const ConnectionOptions& conn_options) {
    conn_options_ = conn_options;
  }

  void setKillOnQueryTimeout(bool killOnQueryTimeout) {
    killOnQueryTimeout_ = killOnQueryTimeout;
  }

  bool getKillOnQueryTimeout() {
    return killOnQueryTimeout_;
  }

  void setConnectionDyingCallback(ConnectionDyingCallback callback) {
    conn_dying_callback_ = callback;
  }

  // Note that the chained callback is invoked in the MySQL client thread
  // and so any callback should execute *very* quickly and not block
  void setPreOperationCallback(ChainedCallback&& callback) {
    callbacks_.pre_operation_callback_ = setCallback(
        std::move(callbacks_.pre_operation_callback_), std::move(callback));
  }

  void setPostOperationCallback(ChainedCallback&& callback) {
    callbacks_.post_operation_callback_ = setCallback(
        std::move(callbacks_.post_operation_callback_), std::move(callback));
  }

  void setCallbacks(Operation::Callbacks&& callbacks) {
    callbacks_.pre_operation_callback_ = setCallback(
        std::move(callbacks_.pre_operation_callback_),
        std::move(callbacks.pre_operation_callback_));
    callbacks_.post_operation_callback_ = setCallback(
        std::move(callbacks_.post_operation_callback_),
        std::move(callbacks.post_operation_callback_));
    callbacks_.pre_query_callback_ = Operation::appendCallback(
        std::move(callbacks_.pre_query_callback_),
        std::move(callbacks.pre_query_callback_));
    callbacks_.post_query_callback_ = Operation::appendCallback(
        std::move(callbacks_.post_query_callback_),
        std::move(callbacks.post_query_callback_));
  }

  const folly::EventBase* getEventBase() const {
    return client()->getEventBase();
  }

  folly::EventBase* getEventBase() {
    return client()->getEventBase();
  }

  bool isInEventBaseThread() const {
    auto eb = getEventBase();
    return eb == nullptr || eb->isInEventBaseThread();
  }

  virtual bool runInThread(folly::Cob&& fn) {
    return client()->runInThread(std::move(fn));
  }

  template <typename TOp, typename... F, typename... T>
  bool runInThread(TOp* op, void (TOp::*f)(F...), T&&... v) {
    // short circuit
    if (isInEventBaseThread()) {
      (op->*f)(std::forward<T>(v)...);
      return true;
    } else {
      return runInThread(std::bind(f, op, v...));
    }
  }

  // Operations call these methods as the operation becomes unblocked, as
  // callers want to wait for completion, etc.
  virtual void notify() = 0;
  virtual void wait() = 0;
  // Called when a new operation is being started.
  virtual void resetActionable() = 0;

  void setMysqlConnectionHolder(
      std::unique_ptr<MysqlConnectionHolder> mysql_connection) {
    CHECK_THROW(mysql_connection_ == nullptr, db::InvalidConnectionException);
    CHECK_THROW(
        conn_key_ == *mysql_connection->getKey(),
        db::InvalidConnectionException);
    mysql_connection_ = std::move(mysql_connection);
  }

  // If this is set and other necessary conditions are met, we clone Connection
  // object in its destructor to properly reset the connection by sending
  // COM_RESET_CONNECTION before it is completed destructed.
  bool needToCloneConnection_{true};

  static std::shared_ptr<ResetOperation> resetConn(
      std::unique_ptr<Connection> conn);

  static std::shared_ptr<ChangeUserOperation> changeUser(
      std::unique_ptr<Connection> conn,
      const std::string& user,
      const std::string& password,
      const std::string& database);

  const db::ConnectionContextBase* getConnectionContext() const {
    return connection_context_.get();
  }

  std::string getTlsVersion() const {
    auto version = mysql_get_ssl_version(mysql_connection_->mysql());
    if (version) {
      return std::string(version);
    }

    return "";
  }

  void setPersistentQueryAttributes(QueryAttributes attrs) {
    persistentQueryAttributes_ = std::move(attrs);
  }

  const QueryAttributes& getPersistentQueryAttributes() const {
    return persistentQueryAttributes_;
  }

 protected:
  // Methods primarily invoked by Operations and AsyncMysqlClient.
  friend class AsyncMysqlClient;
  friend class SyncMysqlClient;
  friend class MysqlClientBase;
  friend class Operation;
  friend class ConnectOperation;
  template <typename Client>
  friend class ConnectPoolOperation;
  friend class FetchOperation;
  friend class QueryOperation;
  friend class MultiQueryOperation;
  friend class MultiQueryStreamOperation;
  friend class SpecialOperation;
  friend class ResetOperation;
  friend class ChangeUserOperation;

  ChainedCallback setCallback(
      ChainedCallback orgCallback,
      ChainedCallback newCallback) {
    if (!orgCallback) {
      return newCallback;
    }

    if (!newCallback) {
      return orgCallback;
    }

    return [orgCallback = std::move(orgCallback),
            newCallback = std::move(newCallback)](Operation& op) mutable {
      orgCallback(op);
      newCallback(op);
    };
  }

  ConnectionSocketHandler* socketHandler() {
    return &socket_handler_;
  }

  MYSQL* mysql() const {
    DCHECK(isInEventBaseThread());
    if (mysql_connection_) {
      return mysql_connection_->mysql();
    } else {
      return nullptr;
    }
  }

  MysqlConnectionHolder* mysqlConnection() const {
    DCHECK(isInEventBaseThread());
    return mysql_connection_.get();
  }

  // Helper function that will begin multiqueries or single queries depending
  // on the specified in the templates. Being used to avoid duplicated code
  // that both need to do.
  template <typename QueryType, typename QueryArg>
  static std::shared_ptr<QueryType> beginAnyQuery(
      Operation::ConnectionProxy&& conn_proxy,
      QueryArg&& query);

  void checkOperationInProgress() {
    if (operation_in_progress_) {
      throw db::InvalidConnectionException(
          "Attempting to run parallel queries in same connection");
    }
  }

  void setConnectionContext(std::shared_ptr<db::ConnectionContextBase> e) {
    connection_context_ = std::move(e);
  }

  void mergePersistentQueryAttributes(QueryAttributes& attrs) const;

  std::unique_ptr<MysqlConnectionHolder> mysql_connection_;

  const ConnectionKey conn_key_;
  ConnectionOptions conn_options_;

  bool killOnQueryTimeout_ = false;

  // Context information for logging purposes.
  std::shared_ptr<db::ConnectionContextBase> connection_context_;

  // Unowned pointer to the client we're from.
  MysqlClientBase* mysql_client_;

  ConnectionSocketHandler socket_handler_;

  ConnectionDyingCallback conn_dying_callback_;

  Operation::Callbacks callbacks_;

  bool initialized_;

  QueryAttributes persistentQueryAttributes_;

  // Used for signing that the connection is being used in a synchronous call,
  // eg. `query`. MySQL doesn't allow more than one query being made through
  // the same connection at the same time. So same logic goes here.
  // We don't track for async calls, for async calls the unique Connection
  // gets moved to the operation, so the protection is guaranteed.
  bool operation_in_progress_ = false;

  Connection(const Connection&) = delete;
  Connection& operator=(const Connection&) = delete;
};

template <>
DbQueryResult Connection::query(Query&& args);

template <>
DbQueryResult Connection::query(Query&& args, QueryOptions&& options);

template <>
DbQueryResult Connection::query(Query&& args, QueryCallback&& cb);

template <>
DbQueryResult
Connection::query(Query&& args, QueryCallback&& cb, QueryOptions&& options);

template <typename... Args>
DbQueryResult Connection::query(Args&&... args) {
  Query query_obj{std::forward<Args>(args)...};
  return query(std::move(query_obj));
}

template <>
std::shared_ptr<QueryOperation> Connection::beginQuery(
    std::unique_ptr<Connection> conn,
    Query&& args);

template <typename... Args>
std::shared_ptr<QueryOperation> Connection::beginQuery(
    std::unique_ptr<Connection> conn,
    Args&&... args) {
  Query query{std::forward<Args>(args)...};
  return beginQuery(std::move(conn), std::move(query));
}

} // namespace facebook::common::mysql_client
