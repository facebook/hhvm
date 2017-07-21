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

#include "hphp/runtime/vm/repo-helpers.h"

#include "hphp/runtime/base/builtin-functions.h"

#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/repo.h"

#include "hphp/util/logger.h"

namespace HPHP {

TRACE_SET_MOD(hhbc);

//==============================================================================
// Debugging

// Try to look at a SQL statement and figure out which repoId it's targeting.
static int debugComputeRepoIdFromSQL(Repo& repo, const std::string& stmt) {
  for (int i = 0; i < RepoIdCount; ++i) {
    auto name = repo.dbName(i);
    if (stmt.find(folly::format(" {}.", name).str()) != std::string::npos) {
      return i;
    }
  }
  return 0;
}


static void reportDbCorruption(Repo& repo, int repoId,
                               const std::string& where) {

  std::string report = folly::sformat("{} returned SQLITE_CORRUPT.\n", where);

  auto repoPath = sqlite3_db_filename(repo.dbc(), repo.dbName(repoId));
  if (repoPath) {
    report += folly::sformat("Path: '{}'\n", repoPath);

    struct stat repoStat;
    if (stat(repoPath, &repoStat) == 0) {
      time_t now = time(nullptr);
      report += folly::sformat("{} bytes, c_age: {}, m_age: {}\n",
                               repoStat.st_size,
                               now - repoStat.st_ctime,
                               now - repoStat.st_mtime);
    } else {
      report += "stat() failed\n";
    }
  } else {
    report += "sqlite3_db_filename() returned nullptr\n";
  }

  // Use raw SQLite here because we just want to hit the raw DB itself.
  sqlite3_exec(repo.dbc(),
               folly::sformat("PRAGMA {}.integrity_check(4);",
                              repo.dbName(repoId))
                 .c_str(),
               [](void* _report, int columns, char** text, char** /*names*/) {
                 std::string& report = *reinterpret_cast<std::string*>(_report);
                 for (int column = 0; column < columns; ++column) {
                   report += folly::sformat("Integrity Check ({}): {}\n",
                                            column, text[column]);
                 }
                 return 0;
               },
               &report, nullptr);

  Logger::Error(report);
}

//==============================================================================
// RepoStmt.

RepoStmt::RepoStmt(Repo& repo)
  : m_repo(repo), m_stmt(nullptr) {
}

RepoStmt::~RepoStmt() {
  finalize();
}

void RepoStmt::finalize() {
  if (m_stmt != nullptr) {
    m_sql.clear();
    sqlite3_finalize(m_stmt);
    m_stmt = nullptr;
  }
}

void RepoStmt::prepare(const std::string& sql) {
  finalize();
  m_sql = sql;
  int rc = sqlite3_prepare_v2(m_repo.dbc(), sql.c_str(), sql.size(), &m_stmt,
                              nullptr);
  if (rc != SQLITE_OK) {
    std::string errmsg = sqlite3_errmsg(m_repo.dbc());
    if (rc == SQLITE_CORRUPT) {
      auto repoId = debugComputeRepoIdFromSQL(repo(), sql);
      reportDbCorruption(m_repo, repoId, "sqlite3_prepare_v2()");
    }
    throw RepoExc("RepoStmt::%s(repo=%p) error: '%s' --> (%d) %s\n",
                  __func__, &m_repo, sql.c_str(), rc,
                  errmsg.c_str());
  }
}

void RepoStmt::reset() {
  sqlite3_reset(m_stmt);
  sqlite3_clear_bindings(m_stmt);
}

//==============================================================================
// RepoTxn.

RepoTxn::RepoTxn(Repo& repo)
  : m_repo(repo), m_pending(false), m_error(false) {
  // Set m_pending AFTER calling repo.begin() so if repo.begin() fails we are
  // prepared as 'not pending'.
  // Don't need a rollback guard here because what would we be rolling back?
  m_repo.begin();
  m_pending = true;
}

RepoTxn::~RepoTxn() {
  if (m_pending) {
    rollback();
  }
}

template<class F>
void RepoTxn::rollback_guard(const char* func, F f) {
  try {
    f();
  } catch (const std::exception& e) {
    TRACE(4, "RepoTxn::%s(repo=%p) caught '%s'\n", func, &m_repo, e.what());
    rollback();
    throw;
  }
}

// Convienence wrapper to provide __func__ to rollback_guard().
#define ROLLBACK_GUARD(f) rollback_guard(__func__, f)

void RepoTxn::prepare(RepoStmt& stmt, const std::string& sql) {
  ROLLBACK_GUARD([&] {
      stmt.prepare(sql);
    });
}

void RepoTxn::exec(const std::string& sQuery) {
  ROLLBACK_GUARD([&] {
      m_repo.exec(sQuery);
    });
}

void RepoTxn::commit() {
  if (!m_pending) return;
  ROLLBACK_GUARD([&] {
      m_repo.commit();
      // Mark pending false AFTER the commit finishes so if it fails we still
      // attempt a rollback.
      m_pending = false;
    });
}

void RepoTxn::step(RepoQuery& query) {
  ROLLBACK_GUARD([&] {
      query.step();
    });
}

void RepoTxn::exec(RepoQuery& query) {
  ROLLBACK_GUARD([&] {
      query.exec();
    });
}

void RepoTxn::rollback() {
  assert(!m_error);
  assert(m_pending);
  m_error = true;
  m_pending = false;
  m_repo.rollback();
}

#undef ROLLBACK_GUARD

//==============================================================================
// RepoQuery.

void RepoQuery::bindBlob(const char* paramName, const void* blob,
                         size_t size, bool isStatic /* = false */) {
  sqlite3_stmt* stmt = m_stmt.get();
  int rc UNUSED =
    sqlite3_bind_blob(stmt,
                      sqlite3_bind_parameter_index(stmt, paramName),
                      blob, int(size),
                      isStatic ? SQLITE_STATIC : SQLITE_TRANSIENT);
  assert(rc == SQLITE_OK);
}

void RepoQuery::bindBlob(const char* paramName, const BlobEncoder& blob,
                         bool isStatic) {
  return bindBlob(paramName, blob.data(), blob.size(), isStatic);
}

void RepoQuery::bindMd5(const char* paramName, const MD5& md5) {
  char md5nbo[16];
  md5.nbo((void*)md5nbo);
  bindBlob(paramName, md5nbo, sizeof(md5nbo));
}

void RepoQuery::bindTypedValue(const char* paramName, const TypedValue& tv) {
  if (tv.m_type == KindOfUninit) {
    bindBlob(paramName, "", 0, true);
  } else {
    String blob = f_serialize(tvAsCVarRef(&tv));
    bindBlob(paramName, blob.data(), blob.size());
  }
}

void RepoQuery::bindText(const char* paramName, const char* text,
                         size_t size, bool isStatic /* = false */) {
  sqlite3_stmt* stmt = m_stmt.get();
  int rc UNUSED =
    sqlite3_bind_text(stmt,
                      sqlite3_bind_parameter_index(stmt, paramName),
                      text, int(size),
                      isStatic ? SQLITE_STATIC : SQLITE_TRANSIENT);
  assert(rc == SQLITE_OK);
}

void RepoQuery::bindStaticString(const char* paramName, const StringData* sd) {
  if (sd == nullptr) {
    bindNull(paramName);
  } else {
    bindText(paramName, sd->data(), sd->size(), true);
  }
}

void RepoQuery::bindStdString(const char* paramName, const std::string& s) {
  bindText(paramName, s.data(), s.size(), true);
}

void RepoQuery::bindStringPiece(const char* paramName, folly::StringPiece s) {
  bindText(paramName, s.data(), s.size(), true);
}

void RepoQuery::bindDouble(const char* paramName, double val) {
  sqlite3_stmt* stmt = m_stmt.get();
  int rc UNUSED =
    sqlite3_bind_double(stmt,
                        sqlite3_bind_parameter_index(stmt, paramName),
                        val);
  assert(rc == SQLITE_OK);
}

void RepoQuery::bindInt(const char* paramName, int val) {
  sqlite3_stmt* stmt = m_stmt.get();
  int rc UNUSED =
    sqlite3_bind_int(stmt,
                     sqlite3_bind_parameter_index(stmt, paramName),
                     val);
  assert(rc == SQLITE_OK);
}

void RepoQuery::bindId(const char* paramName, Id id) {
  bindInt(paramName, int(id));
}

void RepoQuery::bindOffset(const char* paramName, Offset offset) {
  bindInt(paramName, int(offset));
}

void RepoQuery::bindAttr(const char* paramName, Attr attrs) {
  bindInt(paramName, int(attrs));
}

void RepoQuery::bindBool(const char* paramName, bool b) {
  bindInt(paramName, int(b));
}

void RepoQuery::bindInt64(const char* paramName, int64_t val) {
  sqlite3_stmt* stmt = m_stmt.get();
  int rc UNUSED =
    sqlite3_bind_int64(stmt,
                       sqlite3_bind_parameter_index(stmt, paramName),
                       val);
  assert(rc == SQLITE_OK);
}

void RepoQuery::bindNull(const char* paramName) {
  sqlite3_stmt* stmt = m_stmt.get();
  int rc UNUSED =
    sqlite3_bind_null(stmt,
                      sqlite3_bind_parameter_index(stmt, paramName));
  assert(rc == SQLITE_OK);
}

void RepoQuery::step() {
  if (m_done) {
    throw RepoExc("RepoQuery::%s(repo=%p) error: Query done",
                  __func__, &m_stmt.repo());
  }
  int rc = sqlite3_step(m_stmt.get());
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
    m_row = false;
    m_done = true;
    std::string errmsg = sqlite3_errmsg(m_stmt.repo().dbc());
    if (rc == SQLITE_CORRUPT) {
      auto repoId = debugComputeRepoIdFromSQL(m_stmt.repo(), m_stmt.sql());
      reportDbCorruption(m_stmt.repo(), repoId, "sqlite3_step()");
    }
    throw RepoExc("RepoQuery::%s(repo=%p) error: '%s' --> (%d) %s",
                  __func__, &m_stmt.repo(), m_stmt.sql().c_str(), rc,
                  errmsg.c_str());
  }
}

void RepoQuery::reset() {
  m_stmt.reset();
  m_row = false;
  m_done = false;
}

void RepoQuery::exec() {
  step();
  reset();
}

int64_t RepoQuery::getInsertedRowid() {
  return sqlite3_last_insert_rowid(m_stmt.repo().dbc());
}

bool RepoQuery::isBlob(int iCol) {
  return (iCol < sqlite3_data_count(m_stmt.get())
          && sqlite3_column_type(m_stmt.get(), iCol) == SQLITE_BLOB);
}

bool RepoQuery::isText(int iCol) {
  return (iCol < sqlite3_data_count(m_stmt.get())
          && sqlite3_column_type(m_stmt.get(), iCol) == SQLITE_TEXT);
}

bool RepoQuery::isDouble(int iCol) {
  return (iCol < sqlite3_data_count(m_stmt.get())
          && sqlite3_column_type(m_stmt.get(), iCol) == SQLITE_FLOAT);
}

bool RepoQuery::isInt(int iCol) {
  return (iCol < sqlite3_data_count(m_stmt.get())
          && sqlite3_column_type(m_stmt.get(), iCol) == SQLITE_INTEGER);
}

bool RepoQuery::isNull(int iCol) {
  return (iCol < sqlite3_data_count(m_stmt.get())
          && sqlite3_column_type(m_stmt.get(), iCol) == SQLITE_NULL);
}

void RepoQuery::getBlob(int iCol, const void*& blob, size_t& size) {
  if (!isBlob(iCol)) {
    throw RepoExc(
      "RepoQuery::%s(repo=%p) error: Column %d is not a blob in '%s'",
      __func__, &m_stmt.repo(), iCol, m_stmt.sql().c_str());
  }
  blob = sqlite3_column_blob(m_stmt.get(), iCol);
  size = size_t(sqlite3_column_bytes(m_stmt.get(), iCol));
}

BlobDecoder RepoQuery::getBlob(int iCol) {
  const void* vp;
  size_t sz;
  getBlob(iCol, vp, sz);
  return BlobDecoder(vp, sz);
}

void RepoQuery::getMd5(int iCol, MD5& md5) {
  const void* blob;
  size_t size;
  getBlob(iCol, blob, size);
  if (size != 16) {
    throw RepoExc(
      "RepoQuery::%s(repo=%p) error: Column %d is the wrong size"
      " (expected 16, got %zu) in '%s'",
      __func__, &m_stmt.repo(), iCol, size, m_stmt.sql().c_str());
  }
  new (&md5) MD5(blob, size);
}

void RepoQuery::getTypedValue(int iCol, TypedValue& tv) {
  const void* blob;
  size_t size;
  getBlob(iCol, blob, size);
  tvWriteUninit(&tv);
  if (size > 0) {
    String s = String((const char*)blob, size, CopyString);
    Variant v = unserialize_from_string(s);
    if (v.isString()) {
      v = String(makeStaticString(v.asCStrRef().get()));
    } else if (v.isArray()) {
      v = Array(ArrayData::GetScalarArray(v.asCArrRef().get()));
    } else {
      // Serialized variants and objects shouldn't ever make it into the repo.
      assert(!isRefcountedType(v.getType()));
    }
    tvAsVariant(&tv) = v;
  }
}

void RepoQuery::getText(int iCol, const char*& text) {
  if (!isText(iCol)) {
    throw RepoExc(
      "RepoQuery::%s(repo=%p) error: Column %d is not text in '%s'",
      __func__, &m_stmt.repo(), iCol, m_stmt.sql().c_str());
  }
  text = (const char*)sqlite3_column_text(m_stmt.get(), iCol);
}

void RepoQuery::getText(int iCol, const char*& text, size_t& size) {
  getText(iCol, text);
  size = size_t(sqlite3_column_bytes(m_stmt.get(), iCol));
}

void RepoQuery::getStdString(int iCol, std::string& s) {
  const char* text;
  size_t size;
  getText(iCol, text, size);
  s = std::string(text, size);
}

void RepoQuery::getStaticString(int iCol, StringData*& s) {
  if (isNull(iCol)) {
    s = nullptr;
  } else {
    const char* text;
    size_t size;
    getText(iCol, text, size);
    s = makeStaticString(text, size);
  }
}

void RepoQuery::getDouble(int iCol, double& val) {
  if (!isDouble(iCol)) {
    throw RepoExc(
      "RepoQuery::%s(repo=%p) error: Column %d is not a double in '%s'",
      __func__, &m_stmt.repo(), iCol, m_stmt.sql().c_str());
  }
  val = sqlite3_column_double(m_stmt.get(), iCol);
}

void RepoQuery::getInt(int iCol, int& val) {
  if (!isInt(iCol)) {
    throw RepoExc(
      "RepoQuery::%s(repo=%p) error: Column %d is not an integer in '%s'",
      __func__, &m_stmt.repo(), iCol, m_stmt.sql().c_str());
  }
  val = sqlite3_column_int(m_stmt.get(), iCol);
}

void RepoQuery::getId(int iCol, Id& id) {
  int val;
  getInt(iCol, val);
  id = Id(val);
}

void RepoQuery::getOffset(int iCol, Offset& offset) {
  int val;
  getInt(iCol, val);
  offset = Offset(val);
}

void RepoQuery::getAttr(int iCol, Attr& attrs) {
  int val;
  getInt(iCol, val);
  attrs = Attr(val);
}

void RepoQuery::getBool(int iCol, bool& b) {
  int val;
  getInt(iCol, val);
  b = bool(val);
}

void RepoQuery::getInt64(int iCol, int64_t& val) {
  if (!isInt(iCol)) {
    throw RepoExc(
      "RepoQuery::%s(repo=%p) error: Column %d is not an integer in '%s'",
      __func__, &m_stmt.repo(), iCol, m_stmt.sql().c_str());
  }
  val = sqlite3_column_int64(m_stmt.get(), iCol);
}

//==============================================================================
// RepoTxnQuery.

void RepoTxnQuery::step() {
  assert(!m_txn.error());
  m_txn.step(*this);
}

void RepoTxnQuery::exec() {
  assert(!m_txn.error());
  m_txn.exec(*this);
}

 } // HPHP::VM
