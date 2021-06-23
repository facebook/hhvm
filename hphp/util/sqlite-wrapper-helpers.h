/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

#include "hphp/util/optional.h"

struct sqlite3_stmt;

namespace HPHP {

struct SQLite;
struct SQLiteExc;
struct SQLiteQuery;
struct SQLiteStmt;
struct SQLiteTxn;

//==============================================================================

/**
 * RAII wrapper around a SQLite transaction.
 *
 * Rolls the transaction back when destroyed unless commit() is called first.
 */
struct SQLiteTxn {
  explicit SQLiteTxn(SQLite& db);
  SQLiteTxn(SQLiteTxn&&) noexcept;
  SQLiteTxn& operator=(SQLiteTxn&&) noexcept;
  ~SQLiteTxn();

  /**
   * Return a query object to run the given statement.
   *
   * Only one query object may exist for any given statement at any given time.
   */
  SQLiteQuery query(SQLiteStmt& stmt) noexcept;

  /**
   * Run a one-off query without storing the prepared statement or
   * returning results.
   */
  void exec(std::string_view sql);  // throws(SQLiteExc)

  /**
   * Complete the transaction.
   *
   * If you don't call this method, all modifications made to the
   * database while this transaction was alive will be rolled back
   * when this object is destroyed.
   */
  void commit();  // throws(SQLiteExc)

 private:
  SQLiteTxn() = delete;
  SQLiteTxn(const SQLiteTxn&) = delete;
  SQLiteTxn& operator=(const SQLiteTxn&) = delete;
  void* operator new(size_t) = delete;

  SQLite* m_db;
  bool m_pending = true;
};

//==============================================================================

/**
 * RAII wrapper around a sqlite3_stmt statement object.
 */
struct SQLiteStmt {
  SQLiteStmt(SQLiteStmt&&) noexcept;
  SQLiteStmt& operator=(SQLiteStmt&&) noexcept;
  ~SQLiteStmt();

  /**
   * Return the SQL string used to prepare this statement.
   */
  std::string_view sql() const noexcept;

 private:
  friend struct SQLite;
  friend struct SQLiteQuery;
  friend struct SQLiteTxn;

  SQLiteStmt() = delete;
  SQLiteStmt(const SQLiteStmt&) = delete;
  SQLiteStmt& operator=(const SQLiteStmt&) = delete;
  void* operator new(size_t) = delete;

  SQLiteStmt(SQLite& db, const std::string_view sql);  // throws(SQLiteExc)

  /**
   * Return a query object to run this statement.
   *
   * Only one query object may exist at a time.
   */
  SQLiteQuery query() noexcept;

  /**
   * Reinitialize this statement to run a new query.
   *
   * Called by SQLiteQuery's destructor.
   */
  void reset() noexcept;

  sqlite3_stmt* m_stmt = nullptr;
  bool m_queryExists = false;
};

//==============================================================================

/**
 * Represents a single query run on a sqlite3_stmt.
 *
 * Resets the sqlite3_stmt when destroyed.
 */
struct SQLiteQuery {
  SQLiteQuery(SQLiteQuery&&) noexcept;
  SQLiteQuery& operator=(SQLiteQuery&&) noexcept;
  ~SQLiteQuery();

  bool row() const noexcept { return m_row; }

  bool done() const noexcept { return m_done; }

  std::string_view sql() const noexcept;

  void step();  // throws(SQLiteExc)

  void bindBlob(const char* paramName, const void* blob, int size,
                bool isStatic = false) noexcept;
  void bindText(const char* paramName, const char* text, int size,
                bool isStatic = false) noexcept;
  void bindString(const char* paramName, const std::string_view s) noexcept;
  void bindDouble(const char* paramName, double val) noexcept;
  void bindInt(const char* paramName, int val) noexcept;
  void bindBool(const char* paramName, bool b) noexcept;
  void bindInt64(const char* paramName, int64_t val) noexcept;
  void bindNull(const char* paramName) noexcept;

  // Get the column value as the named type. If the value cannot be converted
  // into the named type then an error is thrown.
  bool getBool(int iCol);                                   // throws(SQLiteExc)
  int getInt(int iCol);                                     // throws(SQLiteExc)
  int64_t getInt64(int iCol);                               // throws(SQLiteExc)
  double getDouble(int iCol);                               // throws(SQLiteExc)
  void getBlob(int iCol, const void*& blob, size_t& size);  // throws(SQLiteExc)
  const std::string_view getString(int iCol);               // throws(SQLiteExc)
  Optional<const std::string_view> getNullableString(
      int iCol);                                            // throws(SQLiteExc)

 private:
  friend struct SQLite;
  friend struct SQLiteStmt;
  friend struct SQLiteTxn;

  SQLiteQuery() = delete;
  SQLiteQuery(SQLiteQuery&) = delete;
  SQLiteQuery& operator=(SQLiteQuery&) = delete;
  void* operator new(size_t) = delete;

  explicit SQLiteQuery(SQLiteStmt& stmt);

  SQLiteStmt* m_stmt;
  bool m_row = false;
  bool m_done = false;
};

//==============================================================================

/**
 * Exception encapsulating a SQLite error
 */
struct SQLiteExc : std::runtime_error::runtime_error {
  SQLiteExc(int code, std::string sql);
  SQLiteExc() = delete;

  // Nonexhaustive list of SQLite error codes.
  enum class Code {
    OK = 0,
    ERROR = 1,
    INTERNAL = 2,
    PERM = 3,
    ABORT = 4,
    BUSY = 5,
    LOCKED = 6,
    NOMEM = 7,
    READONLY = 8,
    INTERRUPT = 9,
    IOERR = 10,
    CORRUPT = 11,
    NOTFOUND = 12,
    FULL = 13,
    CANTOPEN = 14,
    PROTOCOL = 15,
    EMPTY = 16,
    SCHEMA = 17,
    TOOBIG = 18,
    CONSTRAINT = 19,
    MISMATCH = 20,
    MISUSE = 21,
    NOLFS = 22,
    AUTH = 23,
    FORMAT = 24,
    RANGE = 25,
    NOTADB = 26,
    NOTICE = 27,
    WARNING = 28,
    __NumberOfCodes // must be last element
  };

  /**
   * Return the error code as an enum value.
   *
   * Unknown error codes will be coerced to ERROR.
   */
  Code code() const noexcept {
    if (m_code < 0 || m_code > static_cast<int>(Code::__NumberOfCodes)) {
      return Code::ERROR;
    }
    return static_cast<Code>(m_code);
  }

  int m_code;
  std::string m_sql;
};

}  // namespace HPHP
