/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "ext_async_mysql.h"

#include <algorithm>
#include <memory>
#include <vector>

#include <squangle/mysql_client/AsyncHelpers.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/ext/mysql/ext_mysql.h"
#include "hphp/runtime/ext/mysql/mysql_common.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/system/systemlib.h"
#include "hphp/util/logger.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define DEFINE_CONSTANT(name)                                                  \
  const int64_t k_##name = name;                                               \
  const StaticString s_##name(#name);                                          \

#define IMPLEMENT_GET_CLASS(cls)                                               \
  Class* cls::getClass() {                                                     \
    if (s_class == nullptr) {                                                  \
      s_class = Unit::lookupClass(s_className.get());                          \
      assert(s_class);                                                         \
    }                                                                          \
  return s_class;                                                              \
  }                                                                            \

// expose the mysql flags
DEFINE_CONSTANT(NOT_NULL_FLAG);
DEFINE_CONSTANT(PRI_KEY_FLAG);
DEFINE_CONSTANT(UNIQUE_KEY_FLAG);
DEFINE_CONSTANT(MULTIPLE_KEY_FLAG);
DEFINE_CONSTANT(UNSIGNED_FLAG);
DEFINE_CONSTANT(ZEROFILL_FLAG);
DEFINE_CONSTANT(BINARY_FLAG);
DEFINE_CONSTANT(AUTO_INCREMENT_FLAG);
DEFINE_CONSTANT(ENUM_FLAG);
DEFINE_CONSTANT(SET_FLAG);
DEFINE_CONSTANT(BLOB_FLAG);
DEFINE_CONSTANT(TIMESTAMP_FLAG);
DEFINE_CONSTANT(NUM_FLAG);
DEFINE_CONSTANT(NO_DEFAULT_VALUE_FLAG);

// expose the mysql field types
DEFINE_CONSTANT(MYSQL_TYPE_TINY);
DEFINE_CONSTANT(MYSQL_TYPE_SHORT);
DEFINE_CONSTANT(MYSQL_TYPE_LONG);
DEFINE_CONSTANT(MYSQL_TYPE_INT24);
DEFINE_CONSTANT(MYSQL_TYPE_LONGLONG);
DEFINE_CONSTANT(MYSQL_TYPE_DECIMAL);
DEFINE_CONSTANT(MYSQL_TYPE_NEWDECIMAL);
DEFINE_CONSTANT(MYSQL_TYPE_FLOAT);
DEFINE_CONSTANT(MYSQL_TYPE_DOUBLE);
DEFINE_CONSTANT(MYSQL_TYPE_BIT);
DEFINE_CONSTANT(MYSQL_TYPE_TIMESTAMP);
DEFINE_CONSTANT(MYSQL_TYPE_DATE);
DEFINE_CONSTANT(MYSQL_TYPE_TIME);
DEFINE_CONSTANT(MYSQL_TYPE_DATETIME);
DEFINE_CONSTANT(MYSQL_TYPE_YEAR);
DEFINE_CONSTANT(MYSQL_TYPE_STRING);
DEFINE_CONSTANT(MYSQL_TYPE_VAR_STRING);
DEFINE_CONSTANT(MYSQL_TYPE_BLOB);
DEFINE_CONSTANT(MYSQL_TYPE_SET);
DEFINE_CONSTANT(MYSQL_TYPE_ENUM);
DEFINE_CONSTANT(MYSQL_TYPE_GEOMETRY);
DEFINE_CONSTANT(MYSQL_TYPE_NULL);

static am::AsyncMysqlClient* getDefaultClient() {
  return am::AsyncMysqlClient::defaultClient();
}

void HHVM_STATIC_METHOD(AsyncMysqlClient, setPoolsConnectionLimit,
                        int64_t limit) {
  getDefaultClient()->setPoolsConnectionLimit(limit);
}

Object HHVM_STATIC_METHOD(AsyncMysqlClient, connect,
                          const String& host,
                          int port,
                          const String& dbname,
                          const String& user,
                          const String& password,
                          int64_t timeout_micros /* = -1 */) {
  auto op = getDefaultClient()->beginConnection(static_cast<std::string>(host),
                                         port,
                                         static_cast<std::string>(dbname),
                                         static_cast<std::string>(user),
                                         static_cast<std::string>(password));
  if (timeout_micros < 0) {
    timeout_micros = mysqlExtension::ConnectTimeout * 1000;
  }
  if (timeout_micros > 0) {
    op->setTimeout(am::Duration(timeout_micros));
  }

  auto event = new AsyncMysqlConnectEvent(op);
  try {
    op->setCallback([event](am::ConnectOperation& op) { event->opFinished(); });
    op->run();

    return event->getWaitHandle();
  }
  catch (...) {
    assert(false);
    event->abandon();
    return nullptr;
  }
}

Object HHVM_STATIC_METHOD(AsyncMysqlClient, adoptConnection,
                          const Variant& connection) {
  auto conn = cast<MySQLResource>(connection)->mysql();
  // mysql connection from ext/mysql/mysql_common.h
  auto raw_conn = conn->eject_mysql();
  auto adopted = getDefaultClient()->adoptConnection(raw_conn,
                                                     conn->m_host,
                                                     conn->m_port,
                                                     conn->m_database,
                                                     conn->m_username,
                                                     conn->m_password);
  return AsyncMysqlConnection::newInstance(std::move(adopted));
}

///////////////////////////////////////////////////////////////////////////////
// class AsyncMysqlConnectionPool

const StaticString AsyncMysqlConnectionPool::s_className(
  "AsyncMysqlConnectionPool");

// `connection_limit` - Defines the limit of opened connections for each set of
// User, Database, Host, etc
// `total_connection_limit` - Defines the total limit of opened connection as a
// whole
// `idle_timeout_micros` - Sets the maximum idle time in microseconds a
// connection can be left in the pool without being killed by the pool
// `age_timeout_micros` - Sets the maximum age (means the time since started) of
// a connection, the pool will then kill this connection when reaches that limit
// `expiration_policy` - We offer 2 policies for the expiration of a
// connection: `IdleTime` and `Age`, in the Idle policy a connection will only
// die after some time being idle; in Age policy we extend the idle one to kill
// also by age
//
const StaticString s_per_key_connection_limit("per_key_connection_limit"),
    s_pool_connection_limit("pool_connection_limit"),
    s_idle_timeout_micros("idle_timeout_micros"),
    s_age_timeout_micros("age_timeout_micros"),
    s_expiration_policy("expiration_policy");

void HHVM_METHOD(AsyncMysqlConnectionPool, __construct,
                 const Array& options) {

  auto* data = Native::data<AsyncMysqlConnectionPool>(this_);
  am::PoolOptions pool_options;
  if (options.exists(s_per_key_connection_limit)) {
    pool_options.setPerKeyLimit(options[s_per_key_connection_limit].toInt32());
  }
  if (options.exists(s_pool_connection_limit)) {
    pool_options.setPoolLimit(options[s_pool_connection_limit].toInt32());
  }
  if (options.exists(s_idle_timeout_micros)) {
    pool_options.setIdleTimeout(
        am::Duration(options[s_idle_timeout_micros].toInt64()));
  }
  if (options.exists(s_age_timeout_micros)) {
    pool_options.setAgeTimeout(
        am::Duration(options[s_age_timeout_micros].toInt64()));
  }
  if (options.exists(s_expiration_policy)) {
    pool_options.setExpPolicy(options[s_expiration_policy].toString() ==
                                      String::FromCStr("IdleTime")
                                  ? am::ExpirationPolicy::IdleTime
                                  : am::ExpirationPolicy::Age);
  }
  data->m_async_pool = am::AsyncConnectionPool::makePool(
      getDefaultClient(), pool_options);
}

// `created_pool_connections` - Number of connections created by the pool
// `destroyed_pool_connections` - Number of connections destroyed by the pool,
//  be careful with this number, it will only be equal to the above when all
//  created connections have been close. This may not be true by the end of
//  a request.
// `connections_requested` - This number helps with comparing how many
//  connection would have been made if the there were no pooling.
// `pool_hits` - Counts the number of times a request for connection went to
//  the pool and it had a connection ready in cache
// `pool_misses` - Counts the number of times a we needed a connection and
//  none was ready to return
const StaticString s_created_pool_connections("created_pool_connections"),
    s_destroyed_pool_connections("destroyed_pool_connections"),
    s_connections_requested("connections_requested"), s_pool_hits("pool_hits"),
    s_pool_misses("pool_misses");

Array HHVM_METHOD(AsyncMysqlConnectionPool, getPoolStats) {
  auto* data = Native::data<AsyncMysqlConnectionPool>(this_);
  auto* pool_stats = data->m_async_pool->stats();
  Array ret = make_map_array(s_created_pool_connections,
                             pool_stats->numCreatedPoolConnections(),
                             s_destroyed_pool_connections,
                             pool_stats->numDestroyedPoolConnections(),
                             s_connections_requested,
                             pool_stats->numConnectionsRequested(),
                             s_pool_hits,
                             pool_stats->numPoolHits(),
                             s_pool_misses,
                             pool_stats->numPoolMisses());
  return ret;
}

Object HHVM_METHOD(AsyncMysqlConnectionPool, connect,
                   const String& host,
                   int port,
                   const String& dbname,
                   const String& user,
                   const String& password,
                   int64_t timeout_micros,
                   const String& extra_key) {

  auto* data = Native::data<AsyncMysqlConnectionPool>(this_);
  auto op = data->m_async_pool->beginConnection(static_cast<std::string>(host),
                                          port,
                                          static_cast<std::string>(dbname),
                                          static_cast<std::string>(user),
                                          static_cast<std::string>(password),
                                          static_cast<std::string>(extra_key));
  if (timeout_micros < 0) {
    timeout_micros = mysqlExtension::ConnectTimeout * 1000;
  }
  if (timeout_micros > 0) {
    op->setTimeout(am::Duration(timeout_micros));
  }

  auto event = new AsyncMysqlConnectEvent(op);
  try {
    op->setCallback([event](am::ConnectOperation& op) { event->opFinished(); });
    op->run();

    return event->getWaitHandle();
  }
  catch (...) {
    LOG(ERROR) << "Unexpected exception while beginning ConnectPoolOperation";
    assert(false);
    event->abandon();
    return nullptr;
  }
}

///////////////////////////////////////////////////////////////////////////////
// class AsyncMysqlConnection

Class* AsyncMysqlConnection::s_class = nullptr;
const StaticString AsyncMysqlConnection::s_className("AsyncMysqlConnection");

IMPLEMENT_GET_CLASS(AsyncMysqlConnection)

Object AsyncMysqlConnection::newInstance(
    std::unique_ptr<am::Connection> conn) {
  Object ret{getClass()};
  Native::data<AsyncMysqlConnection>(ret)->setConnection(std::move(conn));
  return ret;
}

AsyncMysqlConnection::AsyncMysqlConnection() : m_port(0), m_closed(false) {}

void AsyncMysqlConnection::sweep() {
  m_conn.reset();
}

void AsyncMysqlConnection::setConnection(std::unique_ptr<am::Connection> conn) {
  m_conn = std::move(conn);
  m_host = String(m_conn->host(), CopyString);
  m_port = m_conn->port();
}

void AsyncMysqlConnection::verifyValidConnection() {
  if (UNLIKELY(!m_conn || !m_conn->ok())) {
    if (m_closed) {
      SystemLib::throwInvalidArgumentExceptionObject(
        "attempt to invoke method on a closed connection");
    } else if (m_conn && !m_conn->ok()) {
      SystemLib::throwInvalidArgumentExceptionObject(
        "attempt to invoke method on an invalid connection");
    } else {
      SystemLib::throwInvalidArgumentExceptionObject(
        "attempt to invoke method on a busy connection");
    }
  }
}

Object AsyncMysqlConnection::query(
  ObjectData* this_,
  am::Query query,
  int64_t timeout_micros /* = -1 */) {

  verifyValidConnection();
  auto op = am::Connection::beginQuery(std::move(m_conn), query);
  if (timeout_micros < 0) {
    timeout_micros = mysqlExtension::ReadTimeout * 1000;
  }
  if (timeout_micros > 0) {
    op->setTimeout(am::Duration(timeout_micros));
  }

  auto event = new AsyncMysqlQueryEvent(this_, op);
  try {
    am::QueryAppenderCallback appender_callback = [event](
        am::QueryOperation& op,
        am::QueryResult query_result,
        am::QueryCallbackReason reason) {
      DCHECK(reason != am::QueryCallbackReason::RowsFetched);
      if (!op.done()) {
        LOG(ERROR) << "Invalid state! Callback called as finished "
                   << "but operation didn't finish";
      }
      op.setQueryResult(std::move(query_result));
      event->opFinished();
    };
    op->setCallback(am::resultAppender(appender_callback));
    op->run();

    return event->getWaitHandle();
  }
  catch (...) {
    LOG(ERROR) << "Unexpected exception while beginning ConnectOperation";
    assert(false);
    event->abandon();
    return nullptr;
  }
}

Object HHVM_METHOD(AsyncMysqlConnection, query,
                   const String& query,
                   int64_t timeout_micros /* = -1 */) {
  auto* data = Native::data<AsyncMysqlConnection>(this_);

  return data->query(
    this_,
    am::Query::unsafe(static_cast<std::string>(query)),
    timeout_micros);
}

Object HHVM_METHOD(AsyncMysqlConnection, queryf,
                   const String& pattern,
                   const Array& args) {
  auto* data = Native::data<AsyncMysqlConnection>(this_);

  // Not directly calling argsv.toFollyDynamic() as that creates a folly
  // dynamic object, not list
  std::vector<am::QueryArgument> query_args;
  query_args.reserve(args.length());

  auto scalarPush = [](
    std::vector<am::QueryArgument>& list, const Variant& arg
  ) {
    if (arg.isInteger()) {
      list.push_back(arg.toInt64());
    } else if (arg.isDouble()) {
      list.push_back(arg.toDouble());
    } else if (arg.isString()) {
      list.push_back(static_cast<std::string>(arg.toString()));
    } else if (arg.isNull()) {
      list.push_back(am::QueryArgument());
    } else {
      return false;
    }
    return true;
  };

  for (ArrayIter iter(args); iter; ++iter) {
    const Variant& arg = iter.second();
    if (scalarPush(query_args, arg)) {
      continue;
    }

    if (arg.isObject()) {
      const Object& obj = arg.asCObjRef();
      if (obj->isCollection() && isVectorCollection(obj->collectionType())) {
        std::vector<am::QueryArgument> out;
        out.reserve(getCollectionSize(obj.get()));
        for (ArrayIter listIter(arg); listIter; ++listIter) {
          const Variant& item = listIter.second();
          if (scalarPush(out, item)) {
            continue;
          }
          throw_invalid_argument(
            "queryf arguments must be scalars, or Vectors of scalars. "
            "Parameter %ld is a Vector containing a %s at index %ld",
            query_args.size(),
            getDataTypeString(item.getType()).c_str(),
            out.size()
          );
        }
        query_args.push_back(out);
        continue;
      }
    }

    throw_invalid_argument(
      "queryf parameters must be scalars, or Vectors of scalars. "
      "Parameter %ld is a %s",
      query_args.size(),
      getDataTypeString(arg.getType()).c_str()
    );
  }

  return data->query(
    this_,
    am::Query(static_cast<std::string>(pattern), query_args));
}

Object HHVM_METHOD(AsyncMysqlConnection, multiQuery,
                   const Array& queries,
                   int64_t timeout_micros /* = -1 */) {
  auto* data = Native::data<AsyncMysqlConnection>(this_);

  data->verifyValidConnection();
  std::vector<am::Query> queries_vec;
  queries_vec.reserve(queries.size());
  for (ArrayIter iter(queries); iter; ++iter) {
    queries_vec.emplace_back(am::Query::unsafe(
        static_cast<std::string>(iter.second().toString().data())));
  }
  auto op = am::Connection::beginMultiQuery(std::move(data->m_conn),
                                            std::move(queries_vec));
  if (timeout_micros < 0) {
    timeout_micros = mysqlExtension::ReadTimeout * 1000;
  }
  if (timeout_micros > 0) {
    op->setTimeout(am::Duration(timeout_micros));
  }

  auto event = new AsyncMysqlMultiQueryEvent(this_, op);
  try {
    am::MultiQueryAppenderCallback appender_callback = [event](
        am::MultiQueryOperation& op,
        std::vector<am::QueryResult> query_results,
        am::QueryCallbackReason reason) {
      DCHECK(reason != am::QueryCallbackReason::RowsFetched);
      DCHECK(reason != am::QueryCallbackReason::QueryBoundary);
      if (!op.done()) {
        LOG(ERROR) << "Invalid state! Callback called as finished "
                   << "but operation didn't finish";
      }
      op.setQueryResults(std::move(query_results));
      event->opFinished();
    };
    op->setCallback(am::resultAppender(appender_callback));
    op->run();

    return event->getWaitHandle();
  }
  catch (...) {
    assert(false);
    event->abandon();
    return nullptr;
  }
}

String HHVM_METHOD(AsyncMysqlConnection, escapeString, const String& input) {
  auto* data = Native::data<AsyncMysqlConnection>(this_);

  String ret = data->m_conn->escapeString(input.data());
  return ret;
}

String HHVM_METHOD(AsyncMysqlConnection, serverInfo) {
  auto* data = Native::data<AsyncMysqlConnection>(this_);

  String ret = "";
  if (data->m_conn && !data->m_closed) {
    ret = data->m_conn->serverInfo();
  }
  return ret;
}

int HHVM_METHOD(AsyncMysqlConnection, warningCount) {
  auto* data = Native::data<AsyncMysqlConnection>(this_);

  int count = 0;
  if (data->m_conn && !data->m_closed) {
    count = data->m_conn->warningCount();
  }
  return count;
}

String HHVM_METHOD(AsyncMysqlConnection, host) {
  auto* data = Native::data<AsyncMysqlConnection>(this_);
  return data->m_host;
}

int HHVM_METHOD(AsyncMysqlConnection, port) {
  auto* data = Native::data<AsyncMysqlConnection>(this_);
  return data->m_port;
}

void HHVM_METHOD(AsyncMysqlConnection, setReusable,
                 bool reusable) {
  auto* data = Native::data<AsyncMysqlConnection>(this_);

  if (data->m_conn) {
    data->m_conn->setReusable(reusable);
  } else {
    LOG(ERROR) << "Accessing closed connection";
  }
}

bool HHVM_METHOD(AsyncMysqlConnection, isReusable) {
  auto* data = Native::data<AsyncMysqlConnection>(this_);

  if (data->m_conn) {
    return data->m_conn->isReusable();
  } else {
    LOG(ERROR) << "Accessing closed connection";
  }
  return false;
}

void HHVM_METHOD(AsyncMysqlConnection, close) {
  auto* data = Native::data<AsyncMysqlConnection>(this_);

  data->m_conn.reset();
  data->m_closed = true;
}

Variant HHVM_METHOD(AsyncMysqlConnection, releaseConnection) {
  auto* data = Native::data<AsyncMysqlConnection>(this_);
  data->verifyValidConnection();

  auto raw_connection = data->m_conn->stealMysql();
  auto host = data->m_conn->host();
  auto port = data->m_conn->port();
  auto username = data->m_conn->user();
  auto database = data->m_conn->database();
  data->m_conn.reset();
  data->m_closed = true;
  return Variant(
    req::make<MySQLResource>(
      std::make_shared<MySQL>(host.c_str(),
                              port,
                              username.c_str(),
                              "",
                              database.c_str(),
                              raw_connection)));
}

///////////////////////////////////////////////////////////////////////////////
// class AsyncMysqlResult

int64_t AsyncMysqlResult::elapsedMicros() {
  return op()->elapsed().count();
}

double AsyncMysqlResult::startTime() {
  am::Duration d = std::chrono::duration_cast<std::chrono::microseconds>(
      op()->startTime().time_since_epoch());
  return d.count() / 1000.0 / 1000.0;
}

double AsyncMysqlResult::endTime() {
  am::Duration d = std::chrono::duration_cast<std::chrono::microseconds>(
      op()->endTime().time_since_epoch());
  return d.count() / 1000.0 / 1000.0;
}

#define DEFINE_PROXY_METHOD(cls, method, type)                                 \
  type HHVM_METHOD(cls, method) { return Native::data<cls>(this_)->method(); } \

#define EXTENDS_ASYNC_MYSQL_RESULT(cls)                                        \
  DEFINE_PROXY_METHOD(cls, elapsedMicros, int64_t)                             \
  DEFINE_PROXY_METHOD(cls, startTime, double)                                  \
  DEFINE_PROXY_METHOD(cls, endTime, double)                                    \

///////////////////////////////////////////////////////////////////////////////
// class AsyncMysqlErrorResult

Class* AsyncMysqlErrorResult::s_class = nullptr;
const StaticString AsyncMysqlErrorResult::s_className("AsyncMysqlErrorResult");

IMPLEMENT_GET_CLASS(AsyncMysqlErrorResult)

Object AsyncMysqlErrorResult::newInstance(
    std::shared_ptr<am::Operation> op) {
  Object ret{getClass()};
  Native::data<AsyncMysqlErrorResult>(ret)->create(op);
  return ret;
}

void AsyncMysqlErrorResult::create(std::shared_ptr<am::Operation> op) {
  m_op = op;
}

void AsyncMysqlErrorResult::sweep() {
  m_op.reset();
}

am::Operation* AsyncMysqlErrorResult::op() {
  if (m_op.get() == nullptr) {
    // m_op is null if this object is directly created. It is possible if
    // a derived class is defined that does not call this class' constructor.
    SystemLib::throwInvalidOperationExceptionObject(
      "AsyncMysqlErrorResult object is not properly initialized.");
  }
  return m_op.get();
}

EXTENDS_ASYNC_MYSQL_RESULT(AsyncMysqlErrorResult)

int HHVM_METHOD(AsyncMysqlErrorResult, mysql_errno) {
  auto* data = Native::data<AsyncMysqlErrorResult>(this_);
  return data->m_op->mysql_errno();
}

String HHVM_METHOD(AsyncMysqlErrorResult, mysql_error) {
  auto* data = Native::data<AsyncMysqlErrorResult>(this_);
  return data->m_op->mysql_error();
}

String HHVM_METHOD(AsyncMysqlErrorResult, failureType) {
  auto* data = Native::data<AsyncMysqlErrorResult>(this_);
  return data->m_op->resultString().toString();
}

///////////////////////////////////////////////////////////////////////////////
// class AsyncMysqlQueryErrorResult

Class* AsyncMysqlQueryErrorResult::s_class = nullptr;
const StaticString AsyncMysqlQueryErrorResult::s_className(
  "AsyncMysqlQueryErrorResult");

IMPLEMENT_GET_CLASS(AsyncMysqlQueryErrorResult)

Object AsyncMysqlQueryErrorResult::newInstance(
    std::shared_ptr<am::Operation> op, req::ptr<c_Vector> results) {
  Object ret{getClass()};
  Native::data<AsyncMysqlQueryErrorResult>(ret)->create(op, results);
  return ret;
}

AsyncMysqlQueryErrorResult::AsyncMysqlQueryErrorResult() {
  static_assert(offsetof(AsyncMysqlQueryErrorResult, m_parent) +
    sizeof(AsyncMysqlErrorResult) == sizeof(AsyncMysqlQueryErrorResult),
    "m_parent must be the last member of AsyncMysqlQueryErrorResult");
}

void AsyncMysqlQueryErrorResult::sweep() {
  m_parent.sweep();
}

void AsyncMysqlQueryErrorResult::create(std::shared_ptr<am::Operation> op,
                                        req::ptr<c_Vector> results) {
  m_parent.create(op);
  m_query_results = results;
}

int HHVM_METHOD(AsyncMysqlQueryErrorResult, numSuccessfulQueries) {
  auto* data = Native::data<AsyncMysqlQueryErrorResult>(this_);
  return std::dynamic_pointer_cast<am::FetchOperation>(data->m_parent.m_op)
      ->numQueriesExecuted();
}

Object HHVM_METHOD(AsyncMysqlQueryErrorResult, getSuccessfulResults) {
  auto* data = Native::data<AsyncMysqlQueryErrorResult>(this_);
  return Object(data->m_query_results);
}

///////////////////////////////////////////////////////////////////////////////
// class AsyncMysqlQueryResult

Class* AsyncMysqlQueryResult::s_class = nullptr;
const StaticString AsyncMysqlQueryResult::s_className("AsyncMysqlQueryResult");

IMPLEMENT_GET_CLASS(AsyncMysqlQueryResult)

void AsyncMysqlQueryResult::sweep() {
  m_op.reset();
  m_query_result.reset();
}

Object AsyncMysqlQueryResult::newInstance(
    std::shared_ptr<am::Operation> op, am::QueryResult query_result) {
  Object ret{getClass()};
  Native::data<AsyncMysqlQueryResult>(ret)
    ->create(op, std::move(query_result));
  return ret;
}

void AsyncMysqlQueryResult::create(std::shared_ptr<am::Operation> op,
                                   am::QueryResult query_result) {
  m_op = op;
  m_query_result =
      folly::make_unique<am::QueryResult>(std::move(query_result));
  m_field_index =
      req::make_shared<FieldIndex>(m_query_result->getRowFields());
}

am::Operation* AsyncMysqlQueryResult::op() {
  return m_op.get();
}

EXTENDS_ASYNC_MYSQL_RESULT(AsyncMysqlQueryResult)

int64_t HHVM_METHOD(AsyncMysqlQueryResult, lastInsertId) {
  auto* data = Native::data<AsyncMysqlQueryResult>(this_);
  return data->m_query_result->lastInsertId();
}

int64_t HHVM_METHOD(AsyncMysqlQueryResult, numRowsAffected) {
  auto* data = Native::data<AsyncMysqlQueryResult>(this_);
  return data->m_query_result->numRowsAffected();
}

int64_t HHVM_METHOD(AsyncMysqlQueryResult, numRows) {
  auto* data = Native::data<AsyncMysqlQueryResult>(this_);
  return data->m_query_result->numRows();
}
Object HHVM_METHOD(AsyncMysqlQueryResult, vectorRows) {
  auto* data = Native::data<AsyncMysqlQueryResult>(this_);
  return data->buildRows(false /* as_maps */, false /* typed */);
}

Object HHVM_METHOD(AsyncMysqlQueryResult, mapRows) {
  auto* data = Native::data<AsyncMysqlQueryResult>(this_);
  return data->buildRows(true /* as_maps */, false /* typed */);
}

Object HHVM_METHOD(AsyncMysqlQueryResult, vectorRowsTyped) {
  auto* data = Native::data<AsyncMysqlQueryResult>(this_);
  return data->buildRows(false /* as_maps */, true /* typed */);
}

Object HHVM_METHOD(AsyncMysqlQueryResult, mapRowsTyped) {
  auto* data = Native::data<AsyncMysqlQueryResult>(this_);
  return data->buildRows(true /* as_maps */, true /* typed */);
}

Object HHVM_METHOD(AsyncMysqlQueryResult, rowBlocks) {
  auto* data = Native::data<AsyncMysqlQueryResult>(this_);
  auto ret = req::make<c_Vector>();
  auto row_blocks = data->m_query_result->stealRows();
  ret->reserve(row_blocks.size());

  for (auto& row_block : row_blocks) {
    ret->t_add(AsyncMysqlRowBlock::newInstance(&row_block,
                                               data->m_field_index));
  }
  return Object{std::move(ret)};
}

namespace {
Variant buildTypedValue(const am::RowFields* row_fields,
                        const am::Row& row,
                        int field_num,
                        bool typed_values) {
  if (row.isNull(field_num)) {
    return init_null();
  }

  // The underlying library may return zero length null ptr's to
  // indicate an empty string (since the isNull check above would tell
  // if it were actually NULL).
  String string_value =
      (row[field_num].data() == nullptr && row[field_num].size() == 0)
          ? empty_string()
          : String(row[field_num].data(), row[field_num].size(), CopyString);

  if (!typed_values) {
    return string_value;
  }

  return mysql_makevalue(string_value, row_fields->getFieldType(field_num));
}
}

Object AsyncMysqlQueryResult::buildRows(bool as_maps, bool typed_values) {
  auto ret = req::make<c_Vector>();
  ret->reserve(m_query_result->numRows());
  for (const auto& row : *m_query_result) {
    if (as_maps) {
      auto row_map = req::make<c_Map>();
      for (int i = 0; i < row.size(); ++i) {
        row_map->t_set(
            m_field_index->getFieldString(i),
            buildTypedValue(
                m_query_result->getRowFields(), row, i, typed_values));
      }
      ret->t_add(Variant(std::move(row_map)));
    } else {
      auto row_vector = req::make<c_Vector>();
      row_vector->reserve(row.size());
      for (int i = 0; i < row.size(); ++i) {
        row_vector->t_add(buildTypedValue(
            m_query_result->getRowFields(), row, i, typed_values));
      }
      ret->t_add(Variant(std::move(row_vector)));
    }
  }
  return Object(std::move(ret));
}

FieldIndex::FieldIndex(const am::RowFields* row_fields) {
  if (row_fields == nullptr)
    return;
  field_names_.reserve(row_fields->numFields());
  for (int i = 0; i < row_fields->numFields(); ++i) {
    auto name = String(row_fields->fieldName(i).str());
    field_names_.push_back(name);
    field_name_map_[name] = i;
  }
}

size_t FieldIndex::getFieldIndex(String field_name) const {
  auto it = field_name_map_.find(field_name);
  if (it == field_name_map_.end()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      "Given field name doesn't exist");
  }
  return it->second;
}

String FieldIndex::getFieldString(size_t field_index) const {
  // Leaving out of bounds to be thrown by the vector in case needed
  return field_names_.at(field_index);
}

namespace {
void throwAsyncMysqlException(const char* exception_type,
                              std::shared_ptr<am::Operation> op) {
  auto error = AsyncMysqlErrorResult::newInstance(op);

  assert(op->result() == am::OperationResult::Failed ||
         op->result() == am::OperationResult::TimedOut ||
         op->result() == am::OperationResult::Cancelled);

  Array params;
  params.append(std::move(error));
  throw create_object(exception_type, params, true /* init */);
}

void throwAsyncMysqlQueryException(const char* exception_type,
                                   std::shared_ptr<am::Operation> op,
                                   req::ptr<c_Vector> res) {
  auto error = AsyncMysqlQueryErrorResult::newInstance(op, res);

  assert(op->result() == am::OperationResult::Failed ||
         op->result() == am::OperationResult::TimedOut ||
         op->result() == am::OperationResult::Cancelled);

  Array params;
  params.append(std::move(error));
  throw create_object(exception_type, params, true /* init */);
}
}

void AsyncMysqlConnectEvent::unserialize(Cell& result) {
  if (m_op->ok()) {
    auto ret = AsyncMysqlConnection::newInstance(
      m_op->releaseConnection());
    cellCopy(make_tv<KindOfObject>(ret.detach()), result);
  } else {
    throwAsyncMysqlException("AsyncMysqlConnectException", m_op);
  }
}

void AsyncMysqlQueryEvent::unserialize(Cell& result) {
  // Retrieve the original conn and return the underlying connection
  // to it.
  assert(getPrivData()->instanceof(AsyncMysqlConnection::getClass()));
  auto* conn = Native::data<AsyncMysqlConnection>(getPrivData());
  conn->setConnection(m_query_op->releaseConnection());

  if (m_query_op->ok()) {
    auto query_result = m_query_op->stealQueryResult();
    auto ret = AsyncMysqlQueryResult::newInstance(m_query_op,
      std::move(query_result));
    cellCopy(make_tv<KindOfObject>(ret.detach()), result);
  } else {
    throwAsyncMysqlQueryException(
        "AsyncMysqlQueryException",
        m_query_op,
        req::make<c_Vector>());
  }
}

void AsyncMysqlMultiQueryEvent::unserialize(Cell& result) {
  // Same as unserialize from AsyncMysqlQueryEvent but the result is a
  // vector of query results
  assert(getPrivData()->instanceof(AsyncMysqlConnection::getClass()));
  auto* conn = Native::data<AsyncMysqlConnection>(getPrivData());
  conn->setConnection(m_multi_op->releaseConnection());

  // Retrieving the results for all executed queries
  auto results = req::make<c_Vector>();
  std::vector<am::QueryResult> query_results = m_multi_op->stealQueryResults();
  results->reserve(query_results.size());
  for (int i = 0; i < query_results.size(); ++i) {
    auto ret = AsyncMysqlQueryResult::newInstance(m_multi_op,
      std::move(query_results[i]));
    results->t_add(std::move(ret));
  }
  query_results.clear();

  if (m_multi_op->ok()) {
    cellDup(make_tv<KindOfObject>(results.get()), result);
  } else {
    throwAsyncMysqlQueryException(
        "AsyncMysqlQueryException", m_multi_op, results);
  }
}

///////////////////////////////////////////////////////////////////////////////
// class AsyncMysqlRowBlock

Class* AsyncMysqlRowBlock::s_class = nullptr;
const StaticString AsyncMysqlRowBlock::s_className("AsyncMysqlRowBlock");

IMPLEMENT_GET_CLASS(AsyncMysqlRowBlock)

Object AsyncMysqlRowBlock::newInstance(am::RowBlock* row_block,
    std::shared_ptr<FieldIndex> indexer) {
  Object ret{AsyncMysqlRowBlock::getClass()};
  auto* data = Native::data<AsyncMysqlRowBlock>(ret);
  data->m_row_block.reset(new am::RowBlock(std::move(*row_block)));
  data->m_field_index = indexer;
  return ret;
}

void AsyncMysqlRowBlock::sweep() {
  m_row_block.reset();
}

size_t AsyncMysqlRowBlock::getIndexFromVariant(const Variant& field) {
  if (field.isInteger()) {
    return field.toInt64();
  } else if (field.isString()) {
    return m_field_index->getFieldIndex(field.toString());
  }
  SystemLib::throwInvalidArgumentExceptionObject(
    "Only integer or string field names may be used with RowBlock");
}

// The String conversion allows `NULL` to ""
template <>
folly::StringPiece AsyncMysqlRowBlock::getFieldAs(int64_t row,
                                                  const Variant& field) {
  auto index = getIndexFromVariant(field);
  try {
    // Note that for String before you return to PHP you need to copy it into
    // HPHP::String.
    return m_row_block->getField<folly::StringPiece>(row, index);
  }
  catch (std::range_error& excep) {
    SystemLib::throwBadMethodCallExceptionObject(
      std::string("Error during conversion: ") + excep.what());
  }
}

template <typename FieldType>
FieldType AsyncMysqlRowBlock::getFieldAs(int64_t row, const Variant& field) {
  auto index = getIndexFromVariant(field);

  if (m_row_block->isNull(row, index)) {
    SystemLib::throwBadMethodCallExceptionObject(
        "Field value needs to be non-null.");
  }
  try {
    return m_row_block->getField<FieldType>(row, index);
  }
  catch (std::range_error& excep) {
    SystemLib::throwBadMethodCallExceptionObject(
      std::string("Error during conversion: ") + excep.what());
  }
}

Variant HHVM_METHOD(AsyncMysqlRowBlock, at, int64_t row, const Variant& field) {
  auto* data = Native::data<AsyncMysqlRowBlock>(this_);
  auto col_index = data->getIndexFromVariant(field);
  return buildTypedValue(data->m_row_block->getRowFields(),
                         data->m_row_block->getRow(row),
                         col_index,
                         true);
}

int64_t HHVM_METHOD(AsyncMysqlRowBlock,
                    getFieldAsInt,
                    int64_t row,
                    const Variant& field) {
  auto* data = Native::data<AsyncMysqlRowBlock>(this_);
  return data->getFieldAs<int64_t>(row, field);
}

double HHVM_METHOD(AsyncMysqlRowBlock,
                   getFieldAsDouble,
                   int64_t row,
                   const Variant& field) {
  auto* data = Native::data<AsyncMysqlRowBlock>(this_);
  return data->getFieldAs<double>(row, field);
}

String HHVM_METHOD(AsyncMysqlRowBlock,
                   getFieldAsString,
                   int64_t row,
                   const Variant& field) {
  auto* data = Native::data<AsyncMysqlRowBlock>(this_);
  auto val = data->getFieldAs<StringPiece>(row, field);
  // Cannot use the String constructor directly, as it has subtle different
  // behavior in the case where ptr is null, and length is 0, and it breaks flib
  // to change that.
  return String::attach(StringData::Make(val.data(), val.size(), CopyString));
}

bool HHVM_METHOD(AsyncMysqlRowBlock, isNull,
                 int64_t row,
                 const Variant& field) {
  auto* data = Native::data<AsyncMysqlRowBlock>(this_);
  return data->m_row_block->isNull(row, data->getIndexFromVariant(field));
}

int64_t HHVM_METHOD(AsyncMysqlRowBlock, fieldType,
                    const Variant& field) {
  auto* data = Native::data<AsyncMysqlRowBlock>(this_);
  return data->m_row_block->getFieldType(data->getIndexFromVariant(field));
}

int64_t HHVM_METHOD(AsyncMysqlRowBlock, fieldFlags,
                    const Variant& field) {
  auto* data = Native::data<AsyncMysqlRowBlock>(this_);
  return data->m_row_block->getFieldFlags(data->getIndexFromVariant(field));
}

String HHVM_METHOD(AsyncMysqlRowBlock, fieldName,
                   int64_t field_id) {
  auto* data = Native::data<AsyncMysqlRowBlock>(this_);
  return data->m_field_index->getFieldString(field_id);
}

bool HHVM_METHOD(AsyncMysqlRowBlock, isEmpty) {
  auto* data = Native::data<AsyncMysqlRowBlock>(this_);
  return data->m_row_block->empty();
}

int64_t HHVM_METHOD(AsyncMysqlRowBlock, fieldsCount) {
  auto* data = Native::data<AsyncMysqlRowBlock>(this_);
  return data->m_row_block->numFields();
}

int64_t HHVM_METHOD(AsyncMysqlRowBlock, count) {
  auto* data = Native::data<AsyncMysqlRowBlock>(this_);
  return data->m_row_block->numRows();
}

Object HHVM_METHOD(AsyncMysqlRowBlock, getRow,
                   int64_t row_no) {
  return AsyncMysqlRow::newInstance(this_, row_no);
}

Object HHVM_METHOD(AsyncMysqlRowBlock, getIterator) {
  return AsyncMysqlRowBlockIterator::newInstance(this_, 0);
}

///////////////////////////////////////////////////////////////////////////////
// class AsyncMysqlRowBlockIterator

Class* AsyncMysqlRowBlockIterator::s_class = nullptr;
const StaticString AsyncMysqlRowBlockIterator::s_className(
  "AsyncMysqlRowBlockIterator");

IMPLEMENT_GET_CLASS(AsyncMysqlRowBlockIterator)

Object AsyncMysqlRowBlockIterator::newInstance(Object row_block,
                                               size_t row_number) {
  Object ret{getClass()};
  auto* data = Native::data<AsyncMysqlRowBlockIterator>(ret);
  data->m_row_block = row_block;
  data->m_row_number = row_number;
  return ret;
}

void HHVM_METHOD(AsyncMysqlRowBlockIterator, next) {
  auto* data = Native::data<AsyncMysqlRowBlockIterator>(this_);
  data->m_row_number++;
}

bool HHVM_METHOD(AsyncMysqlRowBlockIterator, valid) {
  auto* data = Native::data<AsyncMysqlRowBlockIterator>(this_);

  static_assert(std::is_unsigned<decltype(data->m_row_number)>::value,
                "m_row_number should be unsigned");
  int64_t count = HHVM_MN(AsyncMysqlRowBlock, count)(
    data->m_row_block.get());
  return data->m_row_number < count;
}

Object HHVM_METHOD(AsyncMysqlRowBlockIterator, current) {
  auto* data = Native::data<AsyncMysqlRowBlockIterator>(this_);

  if (!HHVM_MN(AsyncMysqlRowBlockIterator, valid)(this_)) {
    throw_iterator_not_valid();
  }
  return HHVM_MN(AsyncMysqlRowBlock, getRow)(data->m_row_block.get(),
                                             data->m_row_number);
}

int64_t HHVM_METHOD(AsyncMysqlRowBlockIterator, key) {
  auto* data = Native::data<AsyncMysqlRowBlockIterator>(this_);
  return data->m_row_number;
}

void HHVM_METHOD(AsyncMysqlRowBlockIterator, rewind) {
  auto* data = Native::data<AsyncMysqlRowBlockIterator>(this_);
  data->m_row_number = 0;
}

///////////////////////////////////////////////////////////////////////////////
// class AsyncMysqlRow

Class* AsyncMysqlRow::s_class = nullptr;
const StaticString AsyncMysqlRow::s_className("AsyncMysqlRow");

IMPLEMENT_GET_CLASS(AsyncMysqlRow)

Object AsyncMysqlRow::newInstance(Object row_block, size_t row_number) {
  Object ret{getClass()};
  auto* data = Native::data<AsyncMysqlRow>(ret);
  data->m_row_block = row_block;
  data->m_row_number = row_number;
  return ret;
}

#define ROW_BLOCK(method, ...) HHVM_MN(AsyncMysqlRowBlock, method)(            \
  data->m_row_block.get(), __VA_ARGS__)                                        \

Variant HHVM_METHOD(AsyncMysqlRow, at,
                    const Variant& field) {
  auto* data = Native::data<AsyncMysqlRow>(this_);
  return ROW_BLOCK(at, data->m_row_number, field);
}

int64_t HHVM_METHOD(AsyncMysqlRow, getFieldAsInt,
                    const Variant& field) {
  auto* data = Native::data<AsyncMysqlRow>(this_);
  return ROW_BLOCK(getFieldAsInt, data->m_row_number, field);
}

double HHVM_METHOD(AsyncMysqlRow, getFieldAsDouble,
                   const Variant& field) {
  auto* data = Native::data<AsyncMysqlRow>(this_);
  return ROW_BLOCK(getFieldAsDouble, data->m_row_number, field);
}

String HHVM_METHOD(AsyncMysqlRow, getFieldAsString,
                   const Variant& field) {
  auto* data = Native::data<AsyncMysqlRow>(this_);
  return ROW_BLOCK(getFieldAsString, data->m_row_number, field);
}

bool HHVM_METHOD(AsyncMysqlRow, isNull,
                 const Variant& field) {
  auto* data = Native::data<AsyncMysqlRow>(this_);
  return ROW_BLOCK(isNull, data->m_row_number, field);
}

int64_t HHVM_METHOD(AsyncMysqlRow, fieldType,
                    const Variant& field) {
  auto* data = Native::data<AsyncMysqlRow>(this_);
  return ROW_BLOCK(fieldType, field);
}

int64_t HHVM_METHOD(AsyncMysqlRow, count) {
  auto* data = Native::data<AsyncMysqlRow>(this_);
  return HHVM_MN(AsyncMysqlRowBlock, fieldsCount)(data->m_row_block.get());
}

Object HHVM_METHOD(AsyncMysqlRow, getIterator) {
  return AsyncMysqlRowIterator::newInstance(this_, 0);
}

#undef ROW_BLOCK

///////////////////////////////////////////////////////////////////////////////
// class AsyncMysqlRowIterator

Class* AsyncMysqlRowIterator::s_class = nullptr;
const StaticString AsyncMysqlRowIterator::s_className("AsyncMysqlRowIterator");

IMPLEMENT_GET_CLASS(AsyncMysqlRowIterator)

Object AsyncMysqlRowIterator::newInstance(Object row,
                                          size_t field_number) {
  Object ret{getClass()};
  auto* data = Native::data<AsyncMysqlRowIterator>(ret);
  data->m_row = row;
  data->m_field_number = field_number;
  return ret;
}

void HHVM_METHOD(AsyncMysqlRowIterator, next) {
  auto* data = Native::data<AsyncMysqlRowIterator>(this_);
  data->m_field_number++;
}

bool HHVM_METHOD(AsyncMysqlRowIterator, valid) {
  auto* data = Native::data<AsyncMysqlRowIterator>(this_);

  static_assert(std::is_unsigned<decltype(data->m_field_number)>::value,
                "m_field_number should be unsigned");
  int64_t count = HHVM_MN(AsyncMysqlRow, count)(data->m_row.get());
  return data->m_field_number < count;
}

/*?? return as string? */
String HHVM_METHOD(AsyncMysqlRowIterator, current) {
  auto* data = Native::data<AsyncMysqlRowIterator>(this_);
  return HHVM_MN(AsyncMysqlRow, getFieldAsString)(
    data->m_row.get(),
    (uint64_t)data->m_field_number
  );
}

int64_t HHVM_METHOD(AsyncMysqlRowIterator, key) {
  auto* data = Native::data<AsyncMysqlRowIterator>(this_);
  return data->m_field_number;
}

void HHVM_METHOD(AsyncMysqlRowIterator, rewind) {
  auto* data = Native::data<AsyncMysqlRowIterator>(this_);
  data->m_field_number = 0;
}

///////////////////////////////////////////////////////////////////////////////

#define REGISTER_CONSTANT(name)                                                \
  Native::registerConstant<KindOfInt64>(s_##name.get(), k_##name)              \

static const int64_t DISABLE_COPY_AND_SWEEP = Native::NDIFlags::NO_COPY |
  Native::NDIFlags::NO_SWEEP;

static class AsyncMysqlExtension final : public Extension {
public:
  AsyncMysqlExtension() : Extension("async_mysql") {}
  void moduleInit() override {
    REGISTER_CONSTANT(NOT_NULL_FLAG);
    REGISTER_CONSTANT(PRI_KEY_FLAG);
    REGISTER_CONSTANT(UNIQUE_KEY_FLAG);
    REGISTER_CONSTANT(MULTIPLE_KEY_FLAG);
    REGISTER_CONSTANT(UNSIGNED_FLAG);
    REGISTER_CONSTANT(ZEROFILL_FLAG);
    REGISTER_CONSTANT(BINARY_FLAG);
    REGISTER_CONSTANT(AUTO_INCREMENT_FLAG);
    REGISTER_CONSTANT(ENUM_FLAG);
    REGISTER_CONSTANT(SET_FLAG);
    REGISTER_CONSTANT(BLOB_FLAG);
    REGISTER_CONSTANT(TIMESTAMP_FLAG);
    REGISTER_CONSTANT(NUM_FLAG);
    REGISTER_CONSTANT(NO_DEFAULT_VALUE_FLAG);

    REGISTER_CONSTANT(MYSQL_TYPE_TINY);
    REGISTER_CONSTANT(MYSQL_TYPE_SHORT);
    REGISTER_CONSTANT(MYSQL_TYPE_LONG);
    REGISTER_CONSTANT(MYSQL_TYPE_INT24);
    REGISTER_CONSTANT(MYSQL_TYPE_LONGLONG);
    REGISTER_CONSTANT(MYSQL_TYPE_DECIMAL);
    REGISTER_CONSTANT(MYSQL_TYPE_NEWDECIMAL);
    REGISTER_CONSTANT(MYSQL_TYPE_FLOAT);
    REGISTER_CONSTANT(MYSQL_TYPE_DOUBLE);
    REGISTER_CONSTANT(MYSQL_TYPE_BIT);
    REGISTER_CONSTANT(MYSQL_TYPE_TIMESTAMP);
    REGISTER_CONSTANT(MYSQL_TYPE_DATE);
    REGISTER_CONSTANT(MYSQL_TYPE_TIME);
    REGISTER_CONSTANT(MYSQL_TYPE_DATETIME);
    REGISTER_CONSTANT(MYSQL_TYPE_YEAR);
    REGISTER_CONSTANT(MYSQL_TYPE_STRING);
    REGISTER_CONSTANT(MYSQL_TYPE_VAR_STRING);
    REGISTER_CONSTANT(MYSQL_TYPE_BLOB);
    REGISTER_CONSTANT(MYSQL_TYPE_SET);
    REGISTER_CONSTANT(MYSQL_TYPE_ENUM);
    REGISTER_CONSTANT(MYSQL_TYPE_GEOMETRY);
    REGISTER_CONSTANT(MYSQL_TYPE_NULL);

    HHVM_STATIC_ME(AsyncMysqlClient, setPoolsConnectionLimit);
    HHVM_STATIC_ME(AsyncMysqlClient, connect);
    HHVM_STATIC_ME(AsyncMysqlClient, adoptConnection);

    HHVM_ME(AsyncMysqlConnectionPool, __construct);
    HHVM_ME(AsyncMysqlConnectionPool, getPoolStats);
    HHVM_ME(AsyncMysqlConnectionPool, connect);
    Native::registerNativeDataInfo<AsyncMysqlConnectionPool>(
      AsyncMysqlConnectionPool::s_className.get(), DISABLE_COPY_AND_SWEEP);

    HHVM_ME(AsyncMysqlConnection, query);
    HHVM_ME(AsyncMysqlConnection, queryf);
    HHVM_ME(AsyncMysqlConnection, multiQuery);
    HHVM_ME(AsyncMysqlConnection, escapeString);
    HHVM_ME(AsyncMysqlConnection, close);
    HHVM_ME(AsyncMysqlConnection, releaseConnection);
    HHVM_ME(AsyncMysqlConnection, serverInfo);
    HHVM_ME(AsyncMysqlConnection, warningCount);
    HHVM_ME(AsyncMysqlConnection, host);
    HHVM_ME(AsyncMysqlConnection, port);
    HHVM_ME(AsyncMysqlConnection, setReusable);
    HHVM_ME(AsyncMysqlConnection, isReusable);
    Native::registerNativeDataInfo<AsyncMysqlConnection>(
      AsyncMysqlConnection::s_className.get(), Native::NDIFlags::NO_COPY);

    HHVM_ME(AsyncMysqlErrorResult, elapsedMicros);
    HHVM_ME(AsyncMysqlErrorResult, startTime);
    HHVM_ME(AsyncMysqlErrorResult, endTime);
    HHVM_ME(AsyncMysqlErrorResult, mysql_errno);
    HHVM_ME(AsyncMysqlErrorResult, mysql_error);
    HHVM_ME(AsyncMysqlErrorResult, failureType);
    Native::registerNativeDataInfo<AsyncMysqlErrorResult>(
      AsyncMysqlErrorResult::s_className.get(), Native::NDIFlags::NO_COPY);

    HHVM_ME(AsyncMysqlQueryErrorResult, numSuccessfulQueries);
    HHVM_ME(AsyncMysqlQueryErrorResult, getSuccessfulResults);
    Native::registerNativeDataInfo<AsyncMysqlQueryErrorResult>(
      AsyncMysqlQueryErrorResult::s_className.get(),
      Native::NDIFlags::NO_COPY);

    HHVM_ME(AsyncMysqlQueryResult, elapsedMicros);
    HHVM_ME(AsyncMysqlQueryResult, startTime);
    HHVM_ME(AsyncMysqlQueryResult, endTime);
    HHVM_ME(AsyncMysqlQueryResult, numRowsAffected);
    HHVM_ME(AsyncMysqlQueryResult, lastInsertId);
    HHVM_ME(AsyncMysqlQueryResult, numRows);
    HHVM_ME(AsyncMysqlQueryResult, mapRows);
    HHVM_ME(AsyncMysqlQueryResult, vectorRows);
    HHVM_ME(AsyncMysqlQueryResult, mapRowsTyped);
    HHVM_ME(AsyncMysqlQueryResult, vectorRowsTyped);
    HHVM_ME(AsyncMysqlQueryResult, rowBlocks);
    Native::registerNativeDataInfo<AsyncMysqlQueryResult>(
      AsyncMysqlQueryResult::s_className.get(), Native::NDIFlags::NO_COPY);

    HHVM_ME(AsyncMysqlRowBlock, at);
    HHVM_ME(AsyncMysqlRowBlock, getFieldAsInt);
    HHVM_ME(AsyncMysqlRowBlock, getFieldAsDouble);
    HHVM_ME(AsyncMysqlRowBlock, getFieldAsString);
    HHVM_ME(AsyncMysqlRowBlock, isNull);
    HHVM_ME(AsyncMysqlRowBlock, fieldType);
    HHVM_ME(AsyncMysqlRowBlock, fieldFlags);
    HHVM_ME(AsyncMysqlRowBlock, fieldName);
    HHVM_ME(AsyncMysqlRowBlock, isEmpty);
    HHVM_ME(AsyncMysqlRowBlock, fieldsCount);
    HHVM_ME(AsyncMysqlRowBlock, count);
    HHVM_ME(AsyncMysqlRowBlock, getIterator);
    HHVM_ME(AsyncMysqlRowBlock, getRow);
    Native::registerNativeDataInfo<AsyncMysqlRowBlock>(
      AsyncMysqlRowBlock::s_className.get(), Native::NDIFlags::NO_COPY);

    HHVM_ME(AsyncMysqlRowBlockIterator, valid);
    HHVM_ME(AsyncMysqlRowBlockIterator, next);
    HHVM_ME(AsyncMysqlRowBlockIterator, current);
    HHVM_ME(AsyncMysqlRowBlockIterator, key);
    HHVM_ME(AsyncMysqlRowBlockIterator, rewind);
    Native::registerNativeDataInfo<AsyncMysqlRowBlockIterator>(
      AsyncMysqlRowBlockIterator::s_className.get(),
      DISABLE_COPY_AND_SWEEP);

    HHVM_ME(AsyncMysqlRow, at);
    HHVM_ME(AsyncMysqlRow, getFieldAsInt);
    HHVM_ME(AsyncMysqlRow, getFieldAsDouble);
    HHVM_ME(AsyncMysqlRow, getFieldAsString);
    HHVM_ME(AsyncMysqlRow, isNull);
    HHVM_ME(AsyncMysqlRow, fieldType);
    HHVM_ME(AsyncMysqlRow, count);
    HHVM_ME(AsyncMysqlRow, getIterator);
    Native::registerNativeDataInfo<AsyncMysqlRow>(
      AsyncMysqlRow::s_className.get(), DISABLE_COPY_AND_SWEEP);

    HHVM_ME(AsyncMysqlRowIterator, valid);
    HHVM_ME(AsyncMysqlRowIterator, next);
    HHVM_ME(AsyncMysqlRowIterator, current);
    HHVM_ME(AsyncMysqlRowIterator, key);
    HHVM_ME(AsyncMysqlRowIterator, rewind);
    Native::registerNativeDataInfo<AsyncMysqlRowIterator>(
      AsyncMysqlRowIterator::s_className.get(), DISABLE_COPY_AND_SWEEP);

    loadSystemlib("mysqlrow");
    loadSystemlib("async_mysql_exceptions");
    loadSystemlib();
  }
} s_async_mysql_extension;

///////////////////////////////////////////////////////////////////////////////
}
