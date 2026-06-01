/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/mysql_client/Connection.h"
#include "squangle/mysql_client/ChangeUserOperation.h"
#include "squangle/mysql_client/MultiQueryStreamHandler.h"
#include "squangle/mysql_client/ResetOperation.h"
#include "squangle/mysql_client/SemiFutureAdapter.h"

using namespace std::chrono_literals;

namespace facebook::common::mysql_client {

// Helper to set up pre/post operation callbacks on a FetchOperation
template <typename OpType>
void Connection::setupOperationCallbacks(OpType& op, Connection& conn) {
  op.setPreOperationCallback([&](Operation& opRef) {
    if (conn.callbacks_.pre_operation_callback_) {
      conn.callbacks_.pre_operation_callback_(opRef);
    }
  });
  op.setPostOperationCallback([&](Operation& opRef) {
    if (conn.callbacks_.post_operation_callback_) {
      conn.callbacks_.post_operation_callback_(opRef);
    }
  });

  auto opType = op.getOperationType();
  if (opType == db::OperationType::Query ||
      opType == db::OperationType::MultiQuery) {
    op.setPreQueryCallback([&](FetchOperation& opRef) {
      return conn.callbacks_.pre_query_callback_
          ? conn.callbacks_.pre_query_callback_(opRef)
          : folly::makeSemiFuture(folly::unit);
    });
    op.setPostQueryCallback([&](AsyncPostQueryResult&& result) {
      return conn.callbacks_.post_query_callback_
          ? conn.callbacks_.post_query_callback_(std::move(result))
          : folly::makeSemiFuture(std::move(result));
    });
  }

  if ((opType == db::OperationType::Query ||
       opType == db::OperationType::MultiQuery ||
       opType == db::OperationType::MultiQueryStream) &&
      conn.callbacks_.render_prefix_callback_) {
    op.setRenderPrefixCallback(
        [&]() { return conn.callbacks_.render_prefix_callback_(); });
  }
}

// Explicit template instantiations
template void Connection::setupOperationCallbacks<QueryOperation>(
    QueryOperation& op,
    Connection& conn);
template void Connection::setupOperationCallbacks<MultiQueryOperation>(
    MultiQueryOperation& op,
    Connection& conn);
template void Connection::setupOperationCallbacks<MultiQueryStreamOperation>(
    MultiQueryStreamOperation& op,
    Connection& conn);

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
  const auto& client = conn->mysql_client_;
  Duration timeout = conn->conn_options_.getQueryTimeout();
  auto resetOp = client.createResetOperation(std::move(conn));
  if (timeout.count() > 0) {
    resetOp->setTimeout(timeout);
  }
  // Cast to ResetOperation for backward compatibility
  // The new unified classes (MysqlResetOperation) inherit from SpecialOperation
  // which is compatible with ResetOperation's interface
  return std::static_pointer_cast<ResetOperation>(resetOp);
}

std::shared_ptr<ChangeUserOperation> Connection::changeUser(
    std::unique_ptr<Connection> conn,
    std::shared_ptr<const ConnectionKey> key) {
  const auto& client = conn->mysql_client_;
  Duration timeout = conn->conn_options_.getTimeout();
  auto changeUserOp =
      client.createChangeUserOperation(std::move(conn), std::move(key));
  if (timeout.count() > 0) {
    // set its timeout longer than connection timeout to prevent change user
    // operation from hitting timeout earlier than connection timeout itself
    changeUserOp->setTimeout(timeout + std::chrono::seconds(1));
  }
  // Cast to ChangeUserOperation for backward compatibility
  return std::static_pointer_cast<ChangeUserOperation>(changeUserOp);
}

template <>
std::shared_ptr<QueryOperation> Connection::beginQueryWithLoggingFuncs(
    std::unique_ptr<Connection> conn,
    LoggingFuncsPtr logging_funcs,
    Query&& query) {
  CHECK_THROW(conn.get(), db::InvalidConnectionException);
  CHECK_THROW(conn->ok(), db::InvalidConnectionException);
  conn->checkOperationInProgress();

  const auto& client = conn->mysql_client_;
  Duration timeout = conn->conn_options_.getQueryTimeout();
  auto* connPtr = conn.get(); // Capture before move

  auto ret = client.createQueryOperation(
      std::move(conn), std::move(query), std::move(logging_funcs));

  if (timeout.count() > 0) {
    ret->setTimeout(timeout);
  }

  connPtr->setupOperationCallbacks(*ret, *connPtr);
  return ret;
}

template <>
std::shared_ptr<MultiQueryOperation>
Connection::beginMultiQueryWithLoggingFuncs(
    std::unique_ptr<Connection> conn,
    LoggingFuncsPtr logging_funcs,
    std::vector<Query>&& queries) {
  CHECK_THROW(conn.get(), db::InvalidConnectionException);
  CHECK_THROW(conn->ok(), db::InvalidConnectionException);
  conn->checkOperationInProgress();

  auto is_queries_empty = queries.empty();
  const auto& client = conn->mysql_client_;
  Duration timeout = conn->conn_options_.getQueryTimeout();
  auto* connPtr = conn.get(); // Capture before move

  auto ret = client.createMultiQueryOperation(
      std::move(conn), std::move(queries), std::move(logging_funcs));

  if (timeout.count() > 0) {
    ret->setTimeout(timeout);
  }

  connPtr->setupOperationCallbacks(*ret, *connPtr);

  if (is_queries_empty) {
    ret->setAsyncClientError(
        static_cast<uint16_t>(SquangleErrno::SQ_INVALID_API_USAGE),
        "Given vector of queries is empty");
    ret->cancel();
  }
  return ret;
}

template <>
std::shared_ptr<MultiQueryStreamOperation> Connection::beginMultiQueryStreaming(
    std::unique_ptr<Connection> conn,
    std::vector<Query>&& queries) {
  CHECK_THROW(conn.get(), db::InvalidConnectionException);
  CHECK_THROW(conn->ok(), db::InvalidConnectionException);
  conn->checkOperationInProgress();

  auto is_queries_empty = queries.empty();
  auto* connPtr = conn.get();
  Duration timeout = conn->conn_options_.getQueryTimeout();

  // MultiQueryStreamOperation still uses the legacy pattern since
  // MysqlMultiQueryStreamOperation::create takes ConnectionProxy, not
  // Connection
  auto operation = connPtr->createOperation(
      std::make_unique<OperationBase::OwnedConnection>(std::move(conn)),
      MultiQuery{std::move(queries)});

  if (timeout.count() > 0) {
    operation->setTimeout(timeout);
  }

  connPtr->setupOperationCallbacks(*operation, *connPtr);

  if (is_queries_empty) {
    operation->setAsyncClientError(
        static_cast<uint16_t>(SquangleErrno::SQ_INVALID_API_USAGE),
        "Given vector of queries is empty");
    operation->cancel();
  }
  return operation;
}

// A query might already be semicolon-separated, so we allow this to
// be a MultiQuery.  Or it might just be one query; that's okay, too.
template <>
std::shared_ptr<MultiQueryOperation>
Connection::beginMultiQueryWithLoggingFuncs(
    std::unique_ptr<Connection> conn,
    LoggingFuncsPtr logging_funcs,
    Query&& query) {
  return Connection::beginMultiQueryWithLoggingFuncs(
      std::move(conn),
      std::move(logging_funcs),
      std::vector<Query>{std::move(query)});
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
    throw db::InvalidConnectionException("null connection supplied");
  }
  conn->mergePersistentQueryAttributes(options.getAttributes());
  auto op = beginQueryWithLoggingFuncs(
      std::move(conn), options.stealLoggingFuncs(), std::move(query));
  op->setAttributes(std::move(options.getAttributes()));
  checkForQueryTimeoutOverride(*op, options.getQueryTimeout());
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
    throw db::InvalidConnectionException("null connection supplied");
  }
  conn->mergePersistentQueryAttributes(options.getAttributes());
  auto op = beginMultiQueryWithLoggingFuncs(
      std::move(conn), options.stealLoggingFuncs(), std::move(args));
  op->setAttributes(std::move(options.getAttributes()));
  checkForQueryTimeoutOverride(*op, options.getQueryTimeout());
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
    throw db::InvalidConnectionException("null connection supplied");
  }
  conn->mergePersistentQueryAttributes(options.getAttributes());
  auto op = beginMultiQueryWithLoggingFuncs(
      std::move(conn), options.stealLoggingFuncs(), std::move(args));
  op->setAttributes(std::move(options.getAttributes()));
  checkForQueryTimeoutOverride(*op, options.getQueryTimeout());
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
  // Use the new unified factory method with ConnectionProxy
  auto op = mysql_client_.createQueryOperation(
      std::make_unique<OperationBase::ReferencedConnection>(*this),
      std::move(query),
      options.stealLoggingFuncs());

  // Apply connection-level query timeout
  Duration timeout = conn_options_.getQueryTimeout();
  if (timeout.count() > 0) {
    op->setTimeout(timeout);
  }

  mergePersistentQueryAttributes(options.getAttributes());
  op->setAttributes(std::move(options.getAttributes()));
  checkForQueryTimeoutOverride(*op, options.getQueryTimeout());
  if (cb) {
    op->setCallback(std::move(cb));
  }

  // Set up callbacks from the connection
  setupOperationCallbacks(*op, *this);

  auto guard = folly::makeGuard([&] { operation_in_progress_ = false; });
  operation_in_progress_ = true;

  if (auto optFut = op->callPreQueryCallback(*op)) {
    optFut->wait();
  }
  op->run().wait();

  if (!op->ok()) {
    client()
        .exceptionBuilder()
        .buildQueryException(
            op->numQueriesExecuted(),
            op->result(),
            op->mysql_errno(),
            op->mysql_error(),
            getKey(),
            op->opElapsed())
        .throw_exception();
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

DbQueryResult Connection::internalQueryWithGenerator(
    QueryGenerator& query_generator,
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
    QueryGenerator& query_generator,
    QueryCallback&& cb,
    QueryOptions&& options) {
  return internalQueryWithGenerator(
      query_generator, std::move(cb), std::move(options));
}

template <>
DbQueryResult Connection::queryWithGenerator(
    QueryGenerator&& query_generator,
    QueryOptions&& options) {
  return Connection::queryWithGenerator(
      std::move(query_generator), (QueryCallback) nullptr, std::move(options));
}

template <>
DbQueryResult Connection::queryWithGenerator(
    QueryGenerator& query_generator,
    QueryOptions&& options) {
  return Connection::queryWithGenerator(
      query_generator, (QueryCallback) nullptr, std::move(options));
}

template <>
DbQueryResult Connection::queryWithGenerator(QueryGenerator&& query_generator) {
  return Connection::queryWithGenerator(
      std::move(query_generator), QueryOptions());
}

template <>
DbQueryResult Connection::queryWithGenerator(QueryGenerator& query_generator) {
  return Connection::queryWithGenerator(query_generator, QueryOptions());
}

template <>
DbQueryResult Connection::queryWithGenerator(
    QueryGenerator&& query_generator,
    QueryCallback&& cb) {
  return Connection::queryWithGenerator(
      std::move(query_generator), std::move(cb), QueryOptions());
}

template <>
DbQueryResult Connection::queryWithGenerator(
    QueryGenerator& query_generator,
    QueryCallback&& cb) {
  return Connection::queryWithGenerator(
      query_generator, std::move(cb), QueryOptions());
}

// MultiQuery

DbMultiQueryResult Connection::internalMultiQuery(
    std::vector<Query>&& queries,
    MultiQueryCallback&& cb,
    QueryOptions&& options) {
  // Use the new unified factory method with ConnectionProxy
  auto op = mysql_client_.createMultiQueryOperation(
      std::make_unique<OperationBase::ReferencedConnection>(*this),
      std::move(queries),
      options.stealLoggingFuncs());

  // Apply connection-level query timeout
  Duration timeout = conn_options_.getQueryTimeout();
  if (timeout.count() > 0) {
    op->setTimeout(timeout);
  }

  mergePersistentQueryAttributes(options.getAttributes());
  op->setAttributes(std::move(options.getAttributes()));
  checkForQueryTimeoutOverride(*op, options.getQueryTimeout());
  if (cb) {
    op->setCallback(std::move(cb));
  }

  // Set up callbacks from the connection
  setupOperationCallbacks(*op, *this);

  auto guard = folly::makeGuard([&] { operation_in_progress_ = false; });
  operation_in_progress_ = true;

  if (auto optFut = op->callPreQueryCallback(*op)) {
    optFut->wait();
  }
  op->run().wait();

  if (!op->ok()) {
    client()
        .exceptionBuilder()
        .buildQueryException(
            op->numQueriesExecuted(),
            op->result(),
            op->mysql_errno(),
            op->mysql_error(),
            getKey(),
            op->opElapsed())
        .throw_exception();
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

std::unique_ptr<MultiQueryStreamHandler> Connection::streamMultiQuery(
    std::unique_ptr<Connection> conn,
    std::vector<Query> queries,
    const AttributeMap& attributes) {
  CHECK_THROW(conn.get(), db::InvalidConnectionException);
  CHECK_THROW(conn->ok(), db::InvalidConnectionException);
  conn->checkOperationInProgress();

  // MultiQueryStreamHandler needs to be alive while the operation is running.
  // To accomplish that, ~MultiQueryStreamHandler waits until
  // `postOperationEnded` is called.
  auto& client = conn->client();
  auto* connPtr = conn.get();
  Duration timeout = conn->conn_options_.getQueryTimeout();

  auto op = connPtr->createOperation(
      std::make_unique<OperationBase::OwnedConnection>(std::move(conn)),
      MultiQuery{std::move(queries)});

  if (timeout.count() > 0) {
    op->setTimeout(timeout);
  }

  connPtr->setupOperationCallbacks(*op, *connPtr);

  if (attributes.size() > 0) {
    op->setAttributes(attributes);
  }
  return MultiQueryStreamHandler::create(client, std::move(op));
}

/*static*/ std::unique_ptr<MultiQueryStreamHandler>
Connection::streamMultiQuery(
    std::unique_ptr<Connection> connection,
    Query query,
    const AttributeMap& attributes) {
  return streamMultiQuery(
      std::move(connection), std::vector<Query>{std::move(query)}, attributes);
}

void Connection::mergePersistentQueryAttributes(QueryAttributes& attrs) const {
  for (const auto& [key, value] : getPersistentQueryAttributes()) {
    attrs[key] = value;
  }
}

} // namespace facebook::common::mysql_client
