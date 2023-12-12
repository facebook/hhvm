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

#include <sqlite3.h>

#include "hphp/util/assertions.h"
#include "hphp/util/sqlite-wrapper.h"

namespace HPHP {

//==============================================================================
// SQLite.

namespace {

std::string_view journalModeName(
  SQLite::JournalMode mode) noexcept {
  switch (mode) {
    case SQLite::JournalMode::DELETE:
      return "DELETE";
    case SQLite::JournalMode::TRUNCATE:
      return "TRUNCATE";
    case SQLite::JournalMode::PERSIST:
      return "PERSIST";
    case SQLite::JournalMode::MEMORY:
      return "MEMORY";
    case SQLite::JournalMode::WAL:
      return "WAL";
    case SQLite::JournalMode::OFF:
      return "OFF";
  }
  not_reached();
}

} // namespace

SQLite::SQLite(SQLite&& old) noexcept
    : m_dbc{old.m_dbc},
      m_beginStmt{std::move(old.m_beginStmt)},
      m_rollbackStmt{std::move(old.m_rollbackStmt)},
      m_commitStmt{std::move(old.m_commitStmt)} {
  old.m_dbc = nullptr;

  // No outstanding transactions
  assertx(old.m_txDepth == 0);
  assertx(old.m_rollback == false);
}

SQLite& SQLite::operator=(SQLite&& old) noexcept {
  if (this == &old) {
    return *this;
  }

  // Outstanding transactions would be invalidated; make sure none
  // exist
  assertx(m_txDepth == 0);
  assertx(m_rollback == false);
  assertx(old.m_txDepth == 0);
  assertx(old.m_rollback == false);

  sqlite3_close_v2(m_dbc);
  m_dbc = old.m_dbc;
  old.m_dbc = nullptr;

  m_beginStmt = std::move(old.m_beginStmt);
  m_rollbackStmt = std::move(old.m_rollbackStmt);
  m_commitStmt = std::move(old.m_commitStmt);

  return *this;
}

SQLite::~SQLite() {
  sqlite3_close_v2(m_dbc);
  m_dbc = nullptr;
}

void SQLite::analyze() {
  int rc = sqlite3_exec(m_dbc, "ANALYZE", nullptr, nullptr, nullptr);
  if (rc != SQLITE_OK) {
    throw SQLiteExc{rc, "ANALYZE"};
  }
}

SQLite SQLite::connect(const std::string& path, OpenMode mode) {
  return connect(path.c_str(), mode);
}

SQLite SQLite::connect(const char* path, OpenMode mode) {

  int flags = [&] {
    switch (mode) {
      case OpenMode::ReadOnly:
        return SQLITE_OPEN_READONLY;
      case OpenMode::ReadWrite:
        return SQLITE_OPEN_READWRITE;
      case OpenMode::ReadWriteCreate:
        return SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    }
    not_reached();
  }();

  sqlite3* dbc = nullptr;
  int rc = sqlite3_open_v2(
      path, &dbc,
      SQLITE_OPEN_NOMUTEX | flags,
      nullptr);
  if (rc) {
    throw SQLiteExc{rc, ""};
  }
  return SQLite{dbc};
}

/**
 * Compile the given SQL query into a statement object which can run and rerun
 * the query.
 */
SQLiteStmt SQLite::prepare(const std::string_view sql) {
  return {*this, sql};
}

SQLiteTxn SQLite::begin() { return SQLiteTxn{*this}; }

void SQLite::setBusyTimeout(int ms) noexcept {
  sqlite3_busy_timeout(m_dbc, ms);
}

void SQLite::setJournalMode(JournalMode mode) {
  SQLiteStmt journalModeStmt{
    *this, folly::sformat("PRAGMA journal_mode = {}", journalModeName(mode))};
  journalModeStmt.query().step();
}

void SQLite::setSynchronousLevel(SynchronousLevel lvl) {
  SQLiteStmt stmt{
    *this,
    folly::sformat("PRAGMA synchronous = {}", static_cast<int>(lvl))};
  stmt.query().step();
}

bool SQLite::isReadOnly() const {
  return isReadOnly("main");
}

bool SQLite::isReadOnly(const std::string& dbName) const {
  return isReadOnly(dbName.c_str());
}

bool SQLite::isReadOnly(const char* dbName) const {
  return bool(sqlite3_db_readonly(m_dbc, dbName));
}

std::string SQLite::errMsg() const noexcept { return {sqlite3_errmsg(m_dbc)}; }

SQLite::SQLite(sqlite3* dbc)
    : m_dbc{dbc},
      m_beginStmt{*this, "BEGIN"},
      m_rollbackStmt{*this, "ROLLBACK"},
      m_commitStmt{*this, "COMMIT"} {
  setBusyTimeout(60'000);
  SQLiteStmt foreignKeysStmt{*this, "PRAGMA foreign_keys = ON"};
  foreignKeysStmt.query().step();
}

void SQLite::txPush() {
  if (m_txDepth == 0) {
    SQLiteQuery query{m_beginStmt};
    query.step();
  }
  m_txDepth++;
}

void SQLite::txPop() {
  // We mix the concept of rollback with a normal commit so that if we try to
  // rollback an inner transaction we eventually end up rolling back the outer
  // transaction instead (Sqlite doesn't support rolling back partial
  // transactions).
  assertx(m_txDepth > 0);
  if (m_txDepth > 1) {
    m_txDepth--;
    return;
  }
  if (!m_rollback) {
    SQLiteQuery query{m_commitStmt};
    query.step();
  } else {
    // We're in the outermost transaction - so clear the rollback flag.
    m_rollback = false;
    SQLiteQuery query{m_rollbackStmt};
    try {
      query.step();
    } catch (const SQLiteExc& ) {
      /*
       * Having a rollback fail is actually a normal, expected case,
       * so just swallow this.
       *
       * In particular, according to the docs, if we got an I/O error
       * while doing a commit, the rollback will often fail with "no
       * transaction in progress", because the commit will have
       * automatically been rolled back.  Recommended practice is
       * still to execute a rollback statement and ignore the error.
       */
    }
  }
  // Decrement depth after query execution, in case an exception occurs during
  // commit.  This allows for subsequent rollback of the failed commit.
  m_txDepth--;
}

void SQLite::rollback() noexcept {
  m_rollback = true;
  // NOTE: A try/catch isn't necessary - txPop() handles rollback as a nothrow.
  txPop();
}

void SQLite::commit() { txPop(); }

//==============================================================================
// SQLiteTxn.

SQLiteTxn::SQLiteTxn(SQLite& db) : m_db{&db} { m_db->txPush(); }

SQLiteTxn::SQLiteTxn(SQLiteTxn&& old) noexcept
    : m_db{old.m_db}, m_pending{old.m_pending} {
  old.m_db = nullptr;
  old.m_pending = false;
}

SQLiteTxn& SQLiteTxn::operator=(SQLiteTxn&& old) noexcept {
  if (this == &old) {
    return *this;
  }
  assertx(!m_pending);
  m_db = old.m_db;
  old.m_db = nullptr;
  m_pending = old.m_pending;
  old.m_pending = false;
  return *this;
}

SQLiteTxn::~SQLiteTxn() {
  if (m_pending) {
    assertx(m_db != nullptr);
    m_db->rollback();
  }
}

SQLiteQuery SQLiteTxn::query(SQLiteStmt& stmt) noexcept { return stmt.query(); }

void SQLiteTxn::exec(std::string_view sql) {
  assertx(m_db != nullptr);
  SQLiteStmt stmt{m_db->prepare(sql)};
  SQLiteQuery query{stmt.query()};
  query.step();
}

void SQLiteTxn::commit() {
  assertx(m_db != nullptr);
  m_db->commit();
  m_pending = false;
}

//==============================================================================
// SQLiteStmt.

SQLiteStmt::SQLiteStmt(SQLiteStmt&& old) noexcept
    : m_stmt{old.m_stmt}, m_queryExists{old.m_queryExists} {
  assertx(!old.m_queryExists);
  old.m_stmt = nullptr;
}

SQLiteStmt& SQLiteStmt::operator=(SQLiteStmt&& old) noexcept {
  if (this == &old) {
    return *this;
  }

  // Outstanding queries would be invalidated; make sure none exist
  assertx(!m_queryExists);
  assertx(!old.m_queryExists);

  sqlite3_finalize(m_stmt);
  m_stmt = old.m_stmt;
  old.m_stmt = nullptr;

  return *this;
}

SQLiteStmt::~SQLiteStmt() {
  assertx(!m_queryExists);
  sqlite3_finalize(m_stmt);
  m_stmt = nullptr;
}

std::string_view SQLiteStmt::sql() const noexcept {
#if SQLITE_VERSION_NUMBER >= 3014000
  return std::string_view{sqlite3_expanded_sql(m_stmt)};
#else
  return std::string_view{sqlite3_sql(m_stmt)};
#endif
}

SQLiteStmt::SQLiteStmt(SQLite& db, const std::string_view sql) {
  int rc =
      sqlite3_prepare_v2(db.m_dbc, sql.data(), sql.size(), &m_stmt, nullptr);
  if (rc) {
    throw SQLiteExc{rc, std::string{sql}};
  }
  assertx(m_stmt != nullptr);
}

SQLiteQuery SQLiteStmt::query() noexcept {
  assertx(!m_queryExists);
  assertx(m_stmt != nullptr);
  m_queryExists = true;
  return SQLiteQuery{*this};
}

void SQLiteStmt::reset() noexcept {
  m_queryExists = false;
  sqlite3_reset(m_stmt);
  sqlite3_clear_bindings(m_stmt);
}

//==============================================================================
// SQLiteQuery.

SQLiteQuery::SQLiteQuery(SQLiteQuery&& old) noexcept
    : m_stmt{old.m_stmt}, m_row{old.m_row}, m_done{old.m_done} {
  old.m_stmt = nullptr;
}

SQLiteQuery& SQLiteQuery::operator=(SQLiteQuery&& old) noexcept {
  if (this == &old) {
    return *this;
  }

  if (m_stmt != nullptr) {
    m_stmt->reset();
  }
  m_stmt = old.m_stmt;
  m_row = old.m_row;
  m_done = old.m_done;

  old.m_stmt = nullptr;
  return *this;
}

SQLiteQuery::~SQLiteQuery() {
  if (m_stmt != nullptr) {
    m_stmt->reset();
  }
}

std::string_view SQLiteQuery::sql() const noexcept {
  assertx(m_stmt != nullptr);
  return m_stmt->sql();
}

void SQLiteQuery::step() {
  assertx(m_stmt != nullptr);
  int rc = sqlite3_step(m_stmt->m_stmt);
  switch (rc) {
    case SQLITE_DONE:
      m_row = false;
      m_done = true;
      break;
    case SQLITE_ROW:
      m_row = true;
      m_done = false;
      break;
    default:
      throw SQLiteExc{rc, std::string{sql()}};
  }
}

void SQLiteQuery::bindBlob(const char* paramName, const void* blob, int size,
                           bool isStatic /* = false */) noexcept {
  assertx(m_stmt != nullptr);
  sqlite3_stmt* stmt = m_stmt->m_stmt;
  int UNUSED rc = sqlite3_bind_blob(
      stmt, sqlite3_bind_parameter_index(stmt, paramName), blob, size,
      isStatic ? SQLITE_STATIC : SQLITE_TRANSIENT);
  assertx(rc == SQLITE_OK);
}

void SQLiteQuery::bindText(const char* paramName, const char* text, int size,
                           bool isStatic /* = false */) noexcept {
  assertx(m_stmt != nullptr);
  assertx(size >= 0);
  sqlite3_stmt* stmt = m_stmt->m_stmt;
  int UNUSED rc = sqlite3_bind_text(
      stmt, sqlite3_bind_parameter_index(stmt, paramName), text, int(size),
      isStatic ? SQLITE_STATIC : SQLITE_TRANSIENT);
  assertx(rc == SQLITE_OK);
}

void SQLiteQuery::bindString(const char* paramName,
                             const std::string_view s) noexcept {
  bindText(paramName, s.data(), s.size(), true);
}

void SQLiteQuery::bindDouble(const char* paramName, double val) noexcept {
  assertx(m_stmt != nullptr);
  sqlite3_stmt* stmt = m_stmt->m_stmt;
  int UNUSED rc = sqlite3_bind_double(
      stmt, sqlite3_bind_parameter_index(stmt, paramName), val);
  assertx(rc == SQLITE_OK);
}

void SQLiteQuery::bindInt(const char* paramName, int val) noexcept {
  assertx(m_stmt != nullptr);
  sqlite3_stmt* stmt = m_stmt->m_stmt;
  int UNUSED rc = sqlite3_bind_int(
      stmt, sqlite3_bind_parameter_index(stmt, paramName), val);
  assertx(rc == SQLITE_OK);
}

void SQLiteQuery::bindBool(const char* paramName, bool b) noexcept {
  bindInt(paramName, int(b));
}

void SQLiteQuery::bindInt64(const char* paramName, int64_t val) noexcept {
  assertx(m_stmt != nullptr);
  sqlite3_stmt* stmt = m_stmt->m_stmt;
  int UNUSED rc = sqlite3_bind_int64(
      stmt, sqlite3_bind_parameter_index(stmt, paramName), val);
  assertx(rc == SQLITE_OK);
}

void SQLiteQuery::bindNull(const char* paramName) noexcept {
  assertx(m_stmt != nullptr);
  sqlite3_stmt* stmt = m_stmt->m_stmt;
  int UNUSED rc =
      sqlite3_bind_null(stmt, sqlite3_bind_parameter_index(stmt, paramName));
  assertx(rc == SQLITE_OK);
}

// Get the column value as the named type. If the value cannot be converted
// into the named type then an error is thrown.
bool SQLiteQuery::getBool(int iCol) { return static_cast<bool>(getInt(iCol)); }

int SQLiteQuery::getInt(int iCol) {
  assertx(m_stmt != nullptr);
  return sqlite3_column_int(m_stmt->m_stmt, iCol);
}

int64_t SQLiteQuery::getInt64(int iCol) {
  assertx(m_stmt != nullptr);
  return sqlite3_column_int64(m_stmt->m_stmt, iCol);
}

double SQLiteQuery::getDouble(int iCol) {
  assertx(m_stmt != nullptr);
  return sqlite3_column_double(m_stmt->m_stmt, iCol);
}

void SQLiteQuery::getBlob(int iCol, const void*& blob, size_t& size) {
  assertx(m_stmt != nullptr);
  sqlite3_stmt* stmt = m_stmt->m_stmt;
  blob = sqlite3_column_blob(stmt, iCol);
  size = sqlite3_column_bytes(stmt, iCol);
}

const std::string_view SQLiteQuery::getString(int iCol) {
  assertx(m_stmt != nullptr);
  sqlite3_stmt* stmt = m_stmt->m_stmt;
  const char* text =
      reinterpret_cast<const char*>(sqlite3_column_text(stmt, iCol));
  int size = sqlite3_column_bytes(stmt, iCol);
  assertx(size >= 0);
  return {text, static_cast<size_t>(size)};
}

Optional<const std::string_view> SQLiteQuery::getNullableString(
    int iCol) {
  assertx(m_stmt != nullptr);
  sqlite3_stmt* stmt = m_stmt->m_stmt;
  if (sqlite3_column_type(stmt, iCol) == SQLITE_NULL) {
    return {};
  }
  return {getString(iCol)};
}

std::string_view SQLite::openModeName(SQLite::OpenMode mode) noexcept {
  switch(mode) {
    case SQLite::OpenMode::ReadOnly:
      return "READ";
    case SQLite::OpenMode::ReadWrite:
      return "READ/WRITE";
    case SQLite::OpenMode::ReadWriteCreate:
      return "READ/WRITE/CREATE";
  }
  not_reached();
}

SQLiteQuery::SQLiteQuery(SQLiteStmt& stmt) : m_stmt{&stmt} {}

//==============================================================================
// SQLiteExc.

SQLiteExc::SQLiteExc(int code, std::string sql)
    : std::runtime_error{sql.empty()
                             ? folly::to<std::string>(
                                   "SQLiteExc [", code,
                                   "]: ", sqlite3_errstr(code))
                             : folly::to<std::string>(
                                   "SQLiteExc [", code, "] while running ", sql,
                                   ": ", sqlite3_errstr(code))},
      m_code{code} {}

}  // namespace HPHP
