/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/Connection.h"

#include "squangle/mysql_client/FutureAdapter.h"

namespace facebook::common::mysql_client {

ConnectionSocketHandler::ConnectionSocketHandler(folly::EventBase* base)
    : EventHandler(base), AsyncTimeout(base), op_(nullptr) {}

void ConnectionSocketHandler::timeoutExpired() noexcept {
  op_->timeoutTriggered();
}

void ConnectionSocketHandler::handlerReady(uint16_t /*events*/) noexcept {
  DCHECK(op_->conn()->isInEventBaseThread());
  CHECK_THROW(
      op_->state_ != OperationState::Completed &&
          op_->state_ != OperationState::Unstarted,
      db::OperationStateException);

  if (op_->state() == OperationState::Cancelling) {
    op_->cancel();
  } else {
    op_->invokeSocketActionable();
  }
}

bool Connection::isSSL() const {
  CHECK_THROW(mysql_connection_ != nullptr, db::InvalidConnectionException);
  return mysql_connection_->mysql()->client_flag & CLIENT_SSL;
}

void Connection::initMysqlOnly() {
  DCHECK(isInEventBaseThread());
  CHECK_THROW(mysql_connection_ == nullptr, db::InvalidConnectionException);
  mysql_connection_ = std::make_unique<MysqlConnectionHolder>(
      mysql_client_, mysql_init(nullptr), conn_key_);
  if (!mysql_client_->supportsLocalFiles()) {
    mysql_connection_->mysql()->options.client_flag &= ~CLIENT_LOCAL_FILES;
  }
  // Turn off SSL by default for tests that rely on this.
  enum mysql_ssl_mode ssl_mode = SSL_MODE_DISABLED;
  mysql_options(mysql_connection_->mysql(), MYSQL_OPT_SSL_MODE, &ssl_mode);
}

void Connection::initialize(bool initMysql) {
  if (initMysql) {
    initMysqlOnly();
  }
  initialized_ = true;
}

Connection::~Connection() {
  if (mysql_connection_ && conn_dying_callback_) {
    // Recycle connection, if not needed the client will throw it away
    conn_dying_callback_(std::move(mysql_connection_));
  }
}

std::shared_ptr<ResetOperation> Connection::resetConn(
    std::unique_ptr<Connection> conn) {
  // This function is very similar to beginQuery(), but this does not call
  // addOperation(), which is called by the caller prior to calling
  // resetOp->run(). This is to avoid race condition where shutdownClient() can
  // remove the reset operation from pending_operations_ queue, while the
  // operation still exists in operations_to_remove_ queue; in that case,
  // cleanupCompletedOperations() hits FATAL error.
  auto resetOperationPtr = std::make_shared<ResetOperation>(
      Operation::ConnectionProxy(Operation::OwnedConnection(std::move(conn))));
  Duration timeout =
      resetOperationPtr->connection()->conn_options_.getQueryTimeout();
  if (timeout.count() > 0) {
    resetOperationPtr->setTimeout(timeout);
  }
  resetOperationPtr->connection()->socket_handler_.setOperation(
      resetOperationPtr.get());
  return resetOperationPtr;
}

std::shared_ptr<ChangeUserOperation> Connection::changeUser(
    std::unique_ptr<Connection> conn,
    const std::string& user,
    const std::string& password,
    const std::string& database) {
  auto changeUserOperationPtr = std::make_shared<ChangeUserOperation>(
      Operation::ConnectionProxy(Operation::OwnedConnection(std::move(conn))),
      user,
      password,
      database);
  Duration timeout =
      changeUserOperationPtr->connection()->conn_options_.getTimeout();
  if (timeout.count() > 0) {
    // set its timeout longer than connection timeout to prevent change user
    // operation from hitting timeout earlier than connection timeout itself
    changeUserOperationPtr->setTimeout(timeout + std::chrono::seconds(1));
  }
  changeUserOperationPtr->connection()->socket_handler_.setOperation(
      changeUserOperationPtr.get());
  return changeUserOperationPtr;
}

template <>
std::shared_ptr<QueryOperation> Connection::beginQuery(
    std::unique_ptr<Connection> conn,
    Query&& query) {
  return beginAnyQuery<QueryOperation>(
      Operation::ConnectionProxy(Operation::OwnedConnection(std::move(conn))),
      std::move(query));
}

template <>
std::shared_ptr<MultiQueryOperation> Connection::beginMultiQuery(
    std::unique_ptr<Connection> conn,
    std::vector<Query>&& queries) {
  auto is_queries_empty = queries.empty();
  auto operation = beginAnyQuery<MultiQueryOperation>(
      Operation::ConnectionProxy(Operation::OwnedConnection(std::move(conn))),
      std::move(queries));
  if (is_queries_empty) {
    operation->setAsyncClientError("Given vector of queries is empty");
    operation->cancel();
  }
  return operation;
}

template <>
std::shared_ptr<MultiQueryStreamOperation> Connection::beginMultiQueryStreaming(
    std::unique_ptr<Connection> conn,
    std::vector<Query>&& queries) {
  auto is_queries_empty = queries.empty();
  auto operation = beginAnyQuery<MultiQueryStreamOperation>(
      Operation::ConnectionProxy(Operation::OwnedConnection(std::move(conn))),
      std::move(queries));
  if (is_queries_empty) {
    operation->setAsyncClientError("Given vector of queries is empty");
    operation->cancel();
  }
  return operation;
}

template <typename QueryType, typename QueryArg>
std::shared_ptr<QueryType> Connection::beginAnyQuery(
    Operation::ConnectionProxy&& conn_proxy,
    QueryArg&& query) {
  CHECK_THROW(conn_proxy.get(), db::InvalidConnectionException);
  CHECK_THROW(conn_proxy.get()->ok(), db::InvalidConnectionException);
  conn_proxy.get()->checkOperationInProgress();
  auto ret = std::make_shared<QueryType>(
      std::move(conn_proxy), std::forward<QueryArg>(query));
  Duration timeout = ret->connection()->conn_options_.getQueryTimeout();
  if (timeout.count() > 0) {
    ret->setTimeout(timeout);
  }

  auto* conn = ret->connection();
  conn->mysql_client_->addOperation(ret);
  conn->socket_handler_.setOperation(ret.get());
  ret->setPreOperationCallback([conn](Operation& op) {
    if (conn->callbacks_.pre_operation_callback_) {
      conn->callbacks_.pre_operation_callback_(op);
    }
  });
  ret->setPostOperationCallback([conn](Operation& op) {
    if (conn->callbacks_.post_operation_callback_) {
      conn->callbacks_.post_operation_callback_(op);
    }
  });
  auto opType = ret->getOperationType();
  if (opType == db::OperationType::Query ||
      opType == db::OperationType::MultiQuery) {
    ret->setPreQueryCallback([conn](FetchOperation& op) {
      return conn->callbacks_.pre_query_callback_
          ? conn->callbacks_.pre_query_callback_(op)
          : folly::makeSemiFuture(folly::unit);
    });
    ret->setPostQueryCallback([conn](AsyncPostQueryResult&& result) {
      return conn->callbacks_.post_query_callback_
          ? conn->callbacks_.post_query_callback_(std::move(result))
          : folly::makeSemiFuture(std::move(result));
    });
  }
  return ret;
}

// A query might already be semicolon-separated, so we allow this to
// be a MultiQuery.  Or it might just be one query; that's okay, too.
template <>
std::shared_ptr<MultiQueryOperation> Connection::beginMultiQuery(
    std::unique_ptr<Connection> conn,
    Query&& query) {
  return Connection::beginMultiQuery(
      std::move(conn), std::vector<Query>{std::move(query)});
}

template <>
std::shared_ptr<MultiQueryStreamOperation> Connection::beginMultiQueryStreaming(
    std::unique_ptr<Connection> conn,
    Query&& query) {
  return Connection::beginMultiQueryStreaming(
      std::move(conn), std::vector<Query>{std::move(query)});
}

folly::SemiFuture<DbQueryResult> Connection::querySemiFuture(
    std::unique_ptr<Connection> conn,
    Query&& query,
    QueryCallback&& cb,
    QueryOptions&& options) {
  conn->mergePersistentQueryAttributes(options.getAttributes());
  auto op = beginQuery(std::move(conn), std::move(query));
  op->setAttributes(std::move(options.getAttributes()));
  if (cb) {
    op->setCallback(std::move(cb));
  }
  return toSemiFuture(std::move(op));
}

template <>
folly::Future<DbQueryResult> Connection::queryFuture(
    std::unique_ptr<Connection> conn,
    Query&& query) {
  return toFuture(querySemiFuture(std::move(conn), std::move(query)));
}

folly::SemiFuture<DbMultiQueryResult> Connection::multiQuerySemiFuture(
    std::unique_ptr<Connection> conn,
    Query&& args,
    MultiQueryCallback&& cb,
    QueryOptions&& options) {
  conn->mergePersistentQueryAttributes(options.getAttributes());
  auto op = beginMultiQuery(std::move(conn), std::move(args));
  op->setAttributes(std::move(options.getAttributes()));
  if (cb) {
    op->setCallback(std::move(cb));
  }
  return toSemiFuture(std::move(op));
}

folly::SemiFuture<DbMultiQueryResult> Connection::multiQuerySemiFuture(
    std::unique_ptr<Connection> conn,
    std::vector<Query>&& args,
    MultiQueryCallback&& cb,
    QueryOptions&& options) {
  conn->mergePersistentQueryAttributes(options.getAttributes());
  auto op = beginMultiQuery(std::move(conn), std::move(args));
  op->setAttributes(std::move(options.getAttributes()));
  if (cb) {
    op->setCallback(std::move(cb));
  }
  return toSemiFuture(std::move(op));
}

folly::Future<DbMultiQueryResult> Connection::multiQueryFuture(
    std::unique_ptr<Connection> conn,
    Query&& args) {
  return toFuture(multiQuerySemiFuture(std::move(conn), std::move(args)));
}

folly::Future<DbMultiQueryResult> Connection::multiQueryFuture(
    std::unique_ptr<Connection> conn,
    std::vector<Query>&& args) {
  return toFuture(multiQuerySemiFuture(std::move(conn), std::move(args)));
}

template <>
DbQueryResult
Connection::query(Query&& query, QueryCallback&& cb, QueryOptions&& options) {
  auto op = beginAnyQuery<QueryOperation>(
      Operation::ConnectionProxy(Operation::ReferencedConnection(this)),
      std::move(query));
  mergePersistentQueryAttributes(options.getAttributes());
  op->setAttributes(std::move(options.getAttributes()));
  if (cb) {
    op->setCallback(std::move(cb));
  }
  SCOPE_EXIT {
    operation_in_progress_ = false;
  };
  operation_in_progress_ = true;

  if (op->callbacks_.pre_query_callback_) {
    op->callbacks_.pre_query_callback_(*op).get();
  }
  op->run()->wait();

  if (!op->ok()) {
    throw QueryException(
        op->numQueriesExecuted(),
        op->result(),
        op->mysql_errno(),
        op->mysql_error(),
        *getKey(),
        op->elapsed());
  }
  auto conn_key = *op->connection()->getKey();
  DbQueryResult result(
      std::move(op->stealQueryResult()),
      op->numQueriesExecuted(),
      op->resultSize(),
      nullptr,
      op->result(),
      conn_key,
      op->elapsed());
  if (op->callbacks_.post_query_callback_) {
    // If we have a callback set, wrap (and then unwrap) the result to/from the
    // callback's std::variant wrapper
    return op->callbacks_.post_query_callback_(std::move(result))
        .deferValue([](AsyncPostQueryResult&& result) {
          return std::get<DbQueryResult>(std::move(result));
        })
        .get();
  }
  return result;
}

template <>
DbQueryResult Connection::query(Query&& query) {
  return Connection::query(std::move(query), QueryOptions());
}

template <>
DbQueryResult Connection::query(Query&& query, QueryOptions&& options) {
  return Connection::query(
      std::move(query), (QueryCallback) nullptr, std::move(options));
}

template <>
DbQueryResult Connection::query(Query&& query, QueryCallback&& cb) {
  return Connection::query(std::move(query), std::move(cb), QueryOptions());
}

template <>
DbMultiQueryResult Connection::multiQuery(
    std::vector<Query>&& queries,
    MultiQueryCallback&& cb,
    QueryOptions&& options) {
  auto op = beginAnyQuery<MultiQueryOperation>(
      Operation::ConnectionProxy(Operation::ReferencedConnection(this)),
      std::move(queries));
  mergePersistentQueryAttributes(options.getAttributes());
  op->setAttributes(std::move(options.getAttributes()));
  if (cb) {
    op->setCallback(std::move(cb));
  }
  auto guard = folly::makeGuard([&] { operation_in_progress_ = false; });

  operation_in_progress_ = true;
  if (op->callbacks_.pre_query_callback_) {
    op->callbacks_.pre_query_callback_(*op).get();
  }
  op->run()->wait();

  if (!op->ok()) {
    throw QueryException(
        op->numQueriesExecuted(),
        op->result(),
        op->mysql_errno(),
        op->mysql_error(),
        *getKey(),
        op->elapsed());
  }

  auto conn_key = *op->connection()->getKey();
  DbMultiQueryResult result(
      std::move(op->stealQueryResults()),
      op->numQueriesExecuted(),
      op->resultSize(),
      nullptr,
      op->result(),
      std::move(conn_key),
      op->elapsed());
  if (op->callbacks_.post_query_callback_) {
    // If we have a callback set, wrap (and then unwrap) the result to/from the
    // callback's std::variant wrapper
    return op->callbacks_.post_query_callback_(std::move(result))
        .deferValue([](AsyncPostQueryResult&& result) {
          return std::get<DbMultiQueryResult>(std::move(result));
        })
        .get();
  }
  return result;
}

template <>
DbMultiQueryResult Connection::multiQuery(
    std::vector<Query>&& queries,
    MultiQueryCallback&& cb) {
  return multiQuery(std::move(queries), std::move(cb), QueryOptions());
}

template <>
DbMultiQueryResult Connection::multiQuery(
    std::vector<Query>&& queries,
    QueryOptions&& options) {
  return multiQuery(
      std::move(queries), (MultiQueryCallback) nullptr, std::move(options));
}

template <>
DbMultiQueryResult Connection::multiQuery(std::vector<Query>&& queries) {
  return multiQuery(std::move(queries), QueryOptions());
}

template <>
DbMultiQueryResult Connection::multiQuery(Query&& query) {
  return multiQuery(std::vector<Query>{std::move(query)});
}

template <typename... Args>
DbMultiQueryResult Connection::multiQuery(Args&&... args) {
  return multiQuery(std::vector<Query>{std::forward<Args>(args)...});
}

MultiQueryStreamHandler Connection::streamMultiQuery(
    std::unique_ptr<Connection> conn,
    std::vector<Query>&& queries,
    const AttributeMap& attributes) {
  // MultiQueryStreamHandler needs to be alive while the operation is running.
  // To accomplish that, ~MultiQueryStreamHandler waits until
  // `postOperationEnded` is called.
  auto operation = beginAnyQuery<MultiQueryStreamOperation>(
      Operation::ConnectionProxy(Operation::OwnedConnection(std::move(conn))),
      std::move(queries));
  if (attributes.size() > 0) {
    operation->setAttributes(attributes);
  }
  return MultiQueryStreamHandler(operation);
}

MultiQueryStreamHandler Connection::streamMultiQuery(
    std::unique_ptr<Connection> conn,
    MultiQuery&& multi_query,
    const AttributeMap& attributes) {
  auto proxy =
      Operation::ConnectionProxy(Operation::OwnedConnection(std::move(conn)));
  auto connP = proxy.get();
  auto ret = connP->createOperation(std::move(proxy), std::move(multi_query));
  if (attributes.size() > 0) {
    ret->setAttributes(attributes);
  }
  Duration timeout = ret->connection()->conn_options_.getQueryTimeout();
  if (timeout.count() > 0) {
    ret->setTimeout(timeout);
  }
  ret->connection()->mysql_client_->addOperation(ret);
  ret->connection()->socket_handler_.setOperation(ret.get());

  // MultiQueryStreamHandler needs to be alive while the operation is running.
  // To accomplish that, ~MultiQueryStreamHandler waits until
  // `postOperationEnded` is called.
  return MultiQueryStreamHandler(ret);
}

std::shared_ptr<QueryOperation> Connection::beginTransaction(
    std::unique_ptr<Connection> conn) {
  return beginQuery(std::move(conn), "BEGIN");
}

std::shared_ptr<QueryOperation> Connection::commitTransaction(
    std::unique_ptr<Connection> conn) {
  return beginQuery(std::move(conn), "COMMIT");
}

std::shared_ptr<QueryOperation> Connection::rollbackTransaction(
    std::unique_ptr<Connection> conn) {
  return beginQuery(std::move(conn), "ROLLBACK");
}

std::shared_ptr<QueryOperation> Connection::beginTransaction(
    std::shared_ptr<QueryOperation>& op) {
  return beginQuery(op, "BEGIN");
}

std::shared_ptr<QueryOperation> Connection::commitTransaction(
    std::shared_ptr<QueryOperation>& op) {
  return beginQuery(op, "COMMIT");
}

std::shared_ptr<QueryOperation> Connection::rollbackTransaction(
    std::shared_ptr<QueryOperation>& op) {
  return beginQuery(op, "ROLLBACK");
}

void Connection::mergePersistentQueryAttributes(QueryAttributes& attrs) const {
  for (const auto& [key, value] : getPersistentQueryAttributes()) {
    attrs[key] = value;
  }
}

} // namespace facebook::common::mysql_client
