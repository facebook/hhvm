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

#ifndef incl_EXT_ASYNC_MYSQL_H_
#define incl_EXT_ASYNC_MYSQL_H_

#include <algorithm>
#include <memory>

#include <squangle/mysql_client/AsyncMysqlClient.h>
#include <squangle/mysql_client/AsyncConnectionPool.h>
#include <squangle/mysql_client/Row.h>

#include "hphp/runtime/ext/asio/asio-external-thread-event.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/ext/extension.h"

#include <folly/Format.h>

using folly::StringPiece;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
namespace am = facebook::common::mysql_client;

extern const int64_t k_NOT_NULL_FLAG;
extern const int64_t k_PRI_KEY_FLAG;
extern const int64_t k_UNIQUE_KEY_FLAG;
extern const int64_t k_MULTIPLE_KEY_FLAG;
extern const int64_t k_UNSIGNED_FLAG;
extern const int64_t k_ZEROFILL_FLAG;
extern const int64_t k_BINARY_FLAG;
extern const int64_t k_AUTO_INCREMENT_FLAG;
extern const int64_t k_ENUM_FLAG;
extern const int64_t k_SET_FLAG;
extern const int64_t k_BLOB_FLAG;
extern const int64_t k_TIMESTAMP_FLAG;
extern const int64_t k_NUM_FLAG;
extern const int64_t k_NO_DEFAULT_VALUE_FLAG;

extern const int64_t k_MYSQL_TYPE_TINY;
extern const int64_t k_MYSQL_TYPE_SHORT;
extern const int64_t k_MYSQL_TYPE_LONG;
extern const int64_t k_MYSQL_TYPE_INT24;
extern const int64_t k_MYSQL_TYPE_LONGLONG;
extern const int64_t k_MYSQL_TYPE_DECIMAL;
extern const int64_t k_MYSQL_TYPE_NEWDECIMAL;
extern const int64_t k_MYSQL_TYPE_FLOAT;
extern const int64_t k_MYSQL_TYPE_DOUBLE;
extern const int64_t k_MYSQL_TYPE_BIT;
extern const int64_t k_MYSQL_TYPE_TIMESTAMP;
extern const int64_t k_MYSQL_TYPE_DATE;
extern const int64_t k_MYSQL_TYPE_TIME;
extern const int64_t k_MYSQL_TYPE_DATETIME;
extern const int64_t k_MYSQL_TYPE_YEAR;
extern const int64_t k_MYSQL_TYPE_STRING;
extern const int64_t k_MYSQL_TYPE_VAR_STRING;
extern const int64_t k_MYSQL_TYPE_BLOB;
extern const int64_t k_MYSQL_TYPE_SET;
extern const int64_t k_MYSQL_TYPE_ENUM;
extern const int64_t k_MYSQL_TYPE_GEOMETRY;
extern const int64_t k_MYSQL_TYPE_NULL;

///////////////////////////////////////////////////////////////////////////////
// class AsyncMysqlConnectionPool

class AsyncMysqlConnectionPool {
public:
  AsyncMysqlConnectionPool& operator=(const AsyncMysqlConnectionPool&) = delete;
  std::shared_ptr<am::AsyncConnectionPool> m_async_pool;
  static const StaticString s_className;
};

///////////////////////////////////////////////////////////////////////////////
// class AsyncMysqlConnection

class AsyncMysqlConnection {
public:
  AsyncMysqlConnection();
  AsyncMysqlConnection& operator=(const AsyncMysqlConnection&) = delete;
  void sweep();
  void setConnection(std::unique_ptr<am::Connection> conn);
  void verifyValidConnection();
  static Class* getClass();
  static Object newInstance(std::unique_ptr<am::Connection> conn);
  Object query(ObjectData* this_, am::Query query, int64_t timeout_micros = -1);

  std::unique_ptr<am::Connection> m_conn;
  String m_host;
  int m_port;
  bool m_closed;
  static Class* s_class;
  static const StaticString s_className;
};

///////////////////////////////////////////////////////////////////////////////
// class AsyncMysqlResult

class AsyncMysqlResult {
public:
  virtual ~AsyncMysqlResult() {}
  int64_t elapsedMicros();
  double startTime();
  double endTime();
  virtual am::Operation* op() = 0;
};

///////////////////////////////////////////////////////////////////////////////
// class AsyncMysqlErrorResult

class AsyncMysqlErrorResult : public AsyncMysqlResult {
public:
  virtual ~AsyncMysqlErrorResult() {}
  AsyncMysqlErrorResult& operator=(const AsyncMysqlErrorResult&) = delete;
  void create(std::shared_ptr<am::Operation> op);
  void sweep();
  virtual am::Operation* op();
  static Class* getClass();
  static Object newInstance(std::shared_ptr<am::Operation> op);

  std::shared_ptr<am::Operation> m_op;
  static Class* s_class;
  static const StaticString s_className;
};

///////////////////////////////////////////////////////////////////////////////
// class AsyncMysqlQueryErrorResult

class AsyncMysqlQueryErrorResult {
public:
  AsyncMysqlQueryErrorResult();
  AsyncMysqlQueryErrorResult& operator=(const AsyncMysqlQueryErrorResult&)
    = delete;
  void sweep();
  void create(std::shared_ptr<am::Operation> op, req::ptr<c_Vector> results);
  static Class* getClass();
  static Object newInstance(std::shared_ptr<am::Operation> op,
                            req::ptr<c_Vector> results);

  req::ptr<c_Vector> m_query_results;
  static Class* s_class;
  static const StaticString s_className;

  // extends AsyncMysqlErrorResult
  //
  // The address of native data is computed using the size of the native data
  // as an offset from the object data. To ensure that the offset to the parent
  // class remains the same, we simulate inheritance by placing the parent
  // class below the object instead of using c++ inheritance.
  AsyncMysqlErrorResult m_parent;
};

///////////////////////////////////////////////////////////////////////////////
// This class is shared across the QueryResult and RowBlocks for faster
// indexing of row columns.
// By using HPHP::String instead of other string types, we are able
// to build maps where we just reuse them so we avoid copying.
// Thus, eliminating repetition of map keys and also gain speed.

class FieldIndex {
 public:
  explicit FieldIndex(const am::RowFields* row_fields);

  size_t getFieldIndex(String field_name) const;
  String getFieldString(size_t field_index) const;

 private:
  req::vector<String> field_names_;
  req::hash_map<
    String,
    size_t,
    hphp_string_hash,
    hphp_string_same> field_name_map_;
};

///////////////////////////////////////////////////////////////////////////////
// class AsyncMysqlQueryResult

class AsyncMysqlQueryResult : public AsyncMysqlResult {
public:
  virtual ~AsyncMysqlQueryResult() {}
  AsyncMysqlQueryResult& operator=(const AsyncMysqlQueryResult&) = delete;
  void sweep();
  void create(std::shared_ptr<am::Operation> op, am::QueryResult query_result);
  Object buildRows(bool as_maps, bool typed_values);
  virtual am::Operation* op();
  static Class* getClass();
  static Object newInstance(std::shared_ptr<am::Operation> op,
                            am::QueryResult query_result);

  // Holding the operation just for operation elapsed time
  std::shared_ptr<am::Operation> m_op;
  std::unique_ptr<am::QueryResult> m_query_result;

  // Created here for buildRows and passed to RowBlocks
  std::shared_ptr<FieldIndex> m_field_index;
  static Class* s_class;
  static const StaticString s_className;
};

///////////////////////////////////////////////////////////////////////////////
// Async events

class AsyncMysqlConnectEvent final : public AsioExternalThreadEvent {
 public:
  explicit AsyncMysqlConnectEvent(std::shared_ptr<am::ConnectOperation> op) {
    m_op = op;
  }

  void opFinished() { markAsFinished(); }

 protected:
  void unserialize(Cell& result) override final;

 private:
  std::shared_ptr<am::ConnectOperation> m_op;
};

class AsyncMysqlQueryEvent final : public AsioExternalThreadEvent {
 public:
  AsyncMysqlQueryEvent(ObjectData* conn,
                       std::shared_ptr<am::QueryOperation> op)
      : AsioExternalThreadEvent(conn) {
    m_query_op = std::move(op);
  }

  void opFinished() { markAsFinished(); }

 protected:
  void unserialize(Cell& result) override final;

 private:
  std::shared_ptr<am::QueryOperation> m_query_op;
};

class AsyncMysqlMultiQueryEvent final : public AsioExternalThreadEvent {
 public:
  AsyncMysqlMultiQueryEvent(ObjectData* conn,
                            std::shared_ptr<am::MultiQueryOperation> op)
      : AsioExternalThreadEvent(conn) {
    m_multi_op = op;
  }

  void opFinished() { markAsFinished(); }

 protected:
  void unserialize(Cell& result) override final;

 private:
  std::shared_ptr<am::MultiQueryOperation> m_multi_op;
};

///////////////////////////////////////////////////////////////////////////////
// class AsyncMysqlRowBlock

class AsyncMysqlRowBlock {
 public:
  AsyncMysqlRowBlock& operator=(const AsyncMysqlRowBlock&) = delete;
  void sweep();
  size_t getIndexFromVariant(const Variant& field);
  template <typename FieldType>
  FieldType getFieldAs(int64_t row, const Variant& field);
  static Class* getClass();
  static Object newInstance(am::RowBlock* row_block,
                            std::shared_ptr<FieldIndex> indexer);

  std::unique_ptr<am::RowBlock> m_row_block;
  std::shared_ptr<FieldIndex> m_field_index;
  static Class* s_class;
  static const StaticString s_className;
};

///////////////////////////////////////////////////////////////////////////////
// class AsyncMysqlRowBlockIterator

class AsyncMysqlRowBlockIterator {
 public:
  AsyncMysqlRowBlockIterator& operator=(const AsyncMysqlRowBlockIterator&) =
      delete;
  static Class* getClass();
  static Object newInstance(Object row_block, size_t row_number);

  Object m_row_block;
  size_t m_row_number;
  static Class* s_class;
  static const StaticString s_className;
};

///////////////////////////////////////////////////////////////////////////////
// class AsyncMysqlRow

class AsyncMysqlRow {
public:
  AsyncMysqlRow& operator=(const AsyncMysqlRow&) = delete;
  static Class* getClass();
  static Object newInstance(Object row_block, size_t row_number);

  Object m_row_block;
  size_t m_row_number;
  static Class* s_class;
  static const StaticString s_className;
};

///////////////////////////////////////////////////////////////////////////////
// class AsyncMysqlRowIterator

class AsyncMysqlRowIterator {
public:
  AsyncMysqlRowIterator& operator=(const AsyncMysqlRowIterator&) = delete;
  static Class* getClass();
  static Object newInstance(Object row, size_t field_number);

  Object m_row;
  size_t m_field_number;
  static Class* s_class;
  static const StaticString s_className;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_EXT_ASYNC_MYSQL_H_
