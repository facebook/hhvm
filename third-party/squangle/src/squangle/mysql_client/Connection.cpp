/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/Connection.h"
#include "squangle/mysql_client/ChangeUserOperation.h"
#include "squangle/mysql_client/ResetOperation.h"
#include "squangle/mysql_client/SemiFutureAdapter.h"

using namespace std::chrono_literals;

namespace facebook::common::mysql_client {

namespace {
// Helper function to return QueryException when conn is invalid/null
QueryException getInvalidConnException() {
  return QueryException(
      0,
      OperationResult::Failed,
      static_cast<int>(SquangleErrno::SQ_INVALID_CONN),
      "Invalid argument, connection is null",
      ConnectionKey("", 0, "", "", ""),
      0ms);
}
} // namespace

bool Connection::isSSL() const {
  CHECK_THROW(mysql_connection_ != nullptr, db::InvalidConnectionException);
  return mysql_connection_->isSSL();
}

void Connection::initMysqlOnly() {
  DCHECK(isInEventBaseThread());
  CHECK_THROW(mysql_connection_ == nullptr, db::InvalidConnectionException);
  try {
    mysql_connection_ = std::make_unique<ConnectionHolder>(
        mysql_client_, createInternalConnection(), conn_key_);
    if (!mysql_client_.supportsLocalFiles()) {
      mysql_connection_->disableLocalFiles();
    }

    // Turn off SSL by default for tests that rely on this.
    mysql_connection_->disableSSL();
  } catch (const std::exception& e) {
    LOG(INFO) << "Failed to create internal connection" << e.what();
  }
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
  const auto& client = conn->mysql_client_;
  auto resetOperationPtr = std::make_shared<ResetOperation>(
      client.createSpecialOperationImpl(std::move(conn)));
  Duration timeout =
      resetOperationPtr->connection()->conn_options_.getQueryTimeout();
  if (timeout.count() > 0) {
    resetOperationPtr->setTimeout(timeout);
  }
  return resetOperationPtr;
}

std::shared_ptr<ChangeUserOperation> Connection::changeUser(
    std::unique_ptr<Connection> conn,
    const std::string& user,
    const std::string& password,
    const std::string& database) {
  const auto& client = conn->mysql_client_;
  auto changeUserOperationPtr = std::make_shared<ChangeUserOperation>(
      client.createSpecialOperationImpl(std::move(conn)),
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
  return changeUserOperationPtr;
}

template <>
std::shared_ptr<QueryOperation> Connection::beginQuery(
    std::unique_ptr<Connection> conn,
    Query&& query) {
  return beginAnyQuery<QueryOperation>(
      std::make_unique<OperationImpl::OwnedConnection>(std::move(conn)),
      std::move(query));
}

template <>
std::shared_ptr<MultiQueryOperation> Connection::beginMultiQuery(
    std::unique_ptr<Connection> conn,
    std::vector<Query>&& queries) {
  auto is_queries_empty = queries.empty();
  auto operation = beginAnyQuery<MultiQueryOperation>(
      std::make_unique<OperationImpl::OwnedConnection>(std::move(conn)),
      std::move(queries));
  if (is_queries_empty) {
    operation->setAsyncClientError(
        static_cast<uint16_t>(SquangleErrno::SQ_INVALID_API_USAGE),
        "Given vector of queries is empty");
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
      std::make_unique<OperationImpl::OwnedConnection>(std::move(conn)),
      std::move(queries));
  if (is_queries_empty) {
    operation->setAsyncClientError(
        static_cast<uint16_t>(SquangleErrno::SQ_INVALID_API_USAGE),
        "Given vector of queries is empty");
    operation->cancel();
  }
  return operation;
}

template <typename QueryType, typename QueryArg>
std::shared_ptr<QueryType> Connection::beginAnyQuery(
    std::unique_ptr<OperationImpl::ConnectionProxy> conn_proxy,
    QueryArg&& query) {
  CHECK_THROW(conn_proxy.get(), db::InvalidConnectionException);
  CHECK_THROW(conn_proxy->get()->ok(), db::InvalidConnectionException);
  conn_proxy->get()->checkOperationInProgress();
  const auto& client = conn_proxy->get()->mysql_client_;
  auto ret = std::shared_ptr<QueryType>(new QueryType(
      client.createFetchOperationImpl(std::move(conn_proxy)),
      std::forward<QueryArg>(query)));
  auto& conn = ret->conn();
  Duration timeout = conn.conn_options_.getQueryTimeout();
  if (timeout.count() > 0) {
    ret->setTimeout(timeout);
  }

  conn.mysql_client_.addOperation(ret);
  ret->setPreOperationCallback([&](Operation& op) {
    if (conn.callbacks_.pre_operation_callback_) {
      conn.callbacks_.pre_operation_callback_(op);
    }
  });
  ret->setPostOperationCallback([&](Operation& op) {
    if (conn.callbacks_.post_operation_callback_) {
      conn.callbacks_.post_operation_callback_(op);
    }
  });
  auto opType = ret->getOperationType();
  if (opType == db::OperationType::Query ||
      opType == db::OperationType::MultiQuery) {
    ret->setPreQueryCallback([&](FetchOperation& op) {
      return conn.callbacks_.pre_query_callback_
          ? conn.callbacks_.pre_query_callback_(op)
          : folly::makeSemiFuture(folly::unit);
    });
    ret->setPostQueryCallback([&](AsyncPostQueryResult&& result) {
      return conn.callbacks_.post_query_callback_
          ? conn.callbacks_.post_query_callback_(std::move(result))
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
  if (conn == nullptr) {
    throw getInvalidConnException();
  }
  conn->mergePersistentQueryAttributes(options.getAttributes());
  auto op = beginQuery(std::move(conn), std::move(query));
  op->setAttributes(std::move(options.getAttributes()));
  if (const auto& timeoutOverride = options.getQueryTimeout()) {
    op->setTimeout(*timeoutOverride);
  }
  if (cb) {
    op->setCallback(std::move(cb));
  }
  return toSemiFuture(std::move(op));
}

folly::SemiFuture<DbMultiQueryResult> Connection::multiQuerySemiFuture(
    std::unique_ptr<Connection> conn,
    Query&& args,
    MultiQueryCallback&& cb,
    QueryOptions&& options) {
  if (conn == nullptr) {
    throw getInvalidConnException();
  }
  conn->mergePersistentQueryAttributes(options.getAttributes());
  auto op = beginMultiQuery(std::move(conn), std::move(args));
  op->setAttributes(std::move(options.getAttributes()));
  if (const auto& timeoutOverride = options.getQueryTimeout()) {
    op->setTimeout(*timeoutOverride);
  }
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
  if (conn == nullptr) {
    throw getInvalidConnException();
  }
  conn->mergePersistentQueryAttributes(options.getAttributes());
  auto op = beginMultiQuery(std::move(conn), std::move(args));
  op->setAttributes(std::move(options.getAttributes()));
  if (const auto& timeoutOverride = options.getQueryTimeout()) {
    op->setTimeout(*timeoutOverride);
  }
  if (cb) {
    op->setCallback(std::move(cb));
  }
  return toSemiFuture(std::move(op));
}

// Query

DbQueryResult Connection::internalQuery(
    Query&& query,
    QueryCallback&& cb,
    QueryOptions&& options) {
  auto op = beginAnyQuery<QueryOperation>(
      std::make_unique<OperationImpl::ReferencedConnection>(*this),
      std::move(query));
  mergePersistentQueryAttributes(options.getAttributes());
  op->setAttributes(std::move(options.getAttributes()));
  if (const auto& timeoutOverride = options.getQueryTimeout()) {
    op->setTimeout(*timeoutOverride);
  }
  if (cb) {
    op->setCallback(std::move(cb));
  }

  auto guard = folly::makeGuard([&] { operation_in_progress_ = false; });
  operation_in_progress_ = true;

  if (auto optFut = op->callPreQueryCallback(*op)) {
    optFut->wait();
  }
  op->run().wait();

  if (!op->ok()) {
    throw QueryException(
        op->numQueriesExecuted(),
        op->result(),
        op->mysql_errno(),
        op->mysql_error(),
        getKey(),
        op->opElapsed());
  }
  DbQueryResult result(
      std::move(op->stealQueryResult()),
      op->numQueriesExecuted(),
      op->resultSize(),
      nullptr,
      op->result(),
      op->conn().getKey(),
      op->opElapsed());

  return op->callPostQueryCallback(std::move(result));
}

template <>
DbQueryResult
Connection::query(Query&& query, QueryCallback&& cb, QueryOptions&& options) {
  return internalQuery(std::move(query), std::move(cb), std::move(options));
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

// Query with generator

DbQueryResult Connection::internalQueryWithGenerator(
    QueryGenerator&& query_generator,
    QueryCallback&& cb,
    QueryOptions&& options) {
  return Connection::query(
      query_generator.query(), std::move(cb), std::move(options));
}

template <>
DbQueryResult Connection::queryWithGenerator(
    QueryGenerator&& query_generator,
    QueryCallback&& cb,
    QueryOptions&& options) {
  return internalQueryWithGenerator(
      std::move(query_generator), std::move(cb), std::move(options));
}

template <>
DbQueryResult Connection::queryWithGenerator(
    QueryGenerator&& query_generator,
    QueryOptions&& options) {
  return Connection::queryWithGenerator(
      std::move(query_generator), (QueryCallback) nullptr, std::move(options));
}

template <>
DbQueryResult Connection::queryWithGenerator(QueryGenerator&& query_generator) {
  return Connection::queryWithGenerator(
      std::move(query_generator), QueryOptions());
}

template <>
DbQueryResult Connection::queryWithGenerator(
    QueryGenerator&& query_generator,
    QueryCallback&& cb) {
  return Connection::queryWithGenerator(
      std::move(query_generator), std::move(cb), QueryOptions());
}

// MultiQuery

DbMultiQueryResult Connection::internalMultiQuery(
    std::vector<Query>&& queries,
    MultiQueryCallback&& cb,
    QueryOptions&& options) {
  auto op = beginAnyQuery<MultiQueryOperation>(
      std::make_unique<OperationImpl::ReferencedConnection>(*this),
      std::move(queries));
  mergePersistentQueryAttributes(options.getAttributes());
  op->setAttributes(std::move(options.getAttributes()));
  if (const auto& timeoutOverride = options.getQueryTimeout()) {
    op->setTimeout(*timeoutOverride);
  }
  if (cb) {
    op->setCallback(std::move(cb));
  }

  auto guard = folly::makeGuard([&] { operation_in_progress_ = false; });
  operation_in_progress_ = true;

  if (auto optFut = op->callPreQueryCallback(*op)) {
    optFut->wait();
  }
  op->run().wait();

  if (!op->ok()) {
    throw QueryException(
        op->numQueriesExecuted(),
        op->result(),
        op->mysql_errno(),
        op->mysql_error(),
        getKey(),
        op->opElapsed());
  }

  DbMultiQueryResult result(
      std::move(op->stealQueryResults()),
      op->numQueriesExecuted(),
      op->resultSize(),
      nullptr,
      op->result(),
      op->conn().getKey(),
      op->opElapsed());

  return op->callPostQueryCallback(std::move(result));
}

template <>
DbMultiQueryResult Connection::multiQuery(
    std::vector<Query>&& queries,
    MultiQueryCallback&& cb,
    QueryOptions&& options) {
  return internalMultiQuery(
      std::move(queries), std::move(cb), std::move(options));
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

// Multi query with generators

namespace {

std::vector<Query> generateQueries(
    std::vector<std::unique_ptr<QueryGenerator>>&& query_generators) {
  std::vector<Query> queries;
  queries.reserve(query_generators.size());

  for (const auto& query_generator : query_generators) {
    queries.push_back(query_generator->query());
  }
  return queries;
}

} // namespace

DbMultiQueryResult Connection::internalMultiQueryWithGenerators(
    std::vector<std::unique_ptr<QueryGenerator>>&& query_generators,
    MultiQueryCallback&& cb,
    QueryOptions&& options) {
  return Connection::multiQuery(
      generateQueries(std::move(query_generators)),
      std::move(cb),
      std::move(options));
}

template <>
DbMultiQueryResult Connection::multiQueryWithGenerators(
    std::vector<std::unique_ptr<QueryGenerator>>&& query_generators,
    MultiQueryCallback&& cb,
    QueryOptions&& options) {
  return internalMultiQueryWithGenerators(
      std::move(query_generators), std::move(cb), std::move(options));
}

template <>
DbMultiQueryResult Connection::multiQueryWithGenerators(
    std::vector<std::unique_ptr<QueryGenerator>>&& query_generators,
    MultiQueryCallback&& cb) {
  return Connection::multiQueryWithGenerators(
      std::move(query_generators), std::move(cb), QueryOptions());
}

template <>
DbMultiQueryResult Connection::multiQueryWithGenerators(
    std::vector<std::unique_ptr<QueryGenerator>>&& query_generators,
    QueryOptions&& options) {
  return Connection::multiQueryWithGenerators(
      std::move(query_generators),
      (MultiQueryCallback) nullptr,
      std::move(options));
}

template <>
DbMultiQueryResult Connection::multiQueryWithGenerators(
    std::vector<std::unique_ptr<QueryGenerator>>&& query_generators) {
  return Connection::multiQueryWithGenerators(
      std::move(query_generators), QueryOptions());
}

template <>
DbMultiQueryResult Connection::multiQueryWithGenerator(
    std::unique_ptr<QueryGenerator>&& query_generator) {
  std::vector<std::unique_ptr<QueryGenerator>> query_generators;
  query_generators.push_back(std::move(query_generator));
  return Connection::multiQueryWithGenerators(std::move(query_generators));
}

MultiQueryStreamHandler Connection::streamMultiQuery(
    std::unique_ptr<Connection> conn,
    std::vector<Query>&& queries,
    const AttributeMap& attributes) {
  // MultiQueryStreamHandler needs to be alive while the operation is running.
  // To accomplish that, ~MultiQueryStreamHandler waits until
  // `postOperationEnded` is called.
  auto operation = beginAnyQuery<MultiQueryStreamOperation>(
      std::make_unique<OperationImpl::OwnedConnection>(std::move(conn)),
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
      std::make_unique<OperationImpl::OwnedConnection>(std::move(conn));
  auto* connP = proxy->get();
  auto ret = connP->createOperation(std::move(proxy), std::move(multi_query));
  if (attributes.size() > 0) {
    ret->setAttributes(attributes);
  }
  Duration timeout = ret->connection()->conn_options_.getQueryTimeout();
  if (timeout.count() > 0) {
    ret->setTimeout(timeout);
  }
  ret->connection()->mysql_client_.addOperation(ret);

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
