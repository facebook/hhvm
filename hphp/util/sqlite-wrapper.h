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

#include <string>

#include "hphp/util/sqlite-wrapper-helpers.h"

struct sqlite3;

namespace HPHP {

/**
 * RAII wrapper around a sqlite3 database connection.
 *
 * Use with:
 *
 *   SQLite& db = SQLite::get(":memory:");
 *
 *   SQLiteTxn txn = db.begin();
 *   txn.exec("CREATE TABLE foo (bar)");
 *
 *   SQLiteStmt insertStmt = db.prepare("INSERT INTO foo VALUES (@v)");
 *   for (auto i = 0; i < 10; i++) {
 *     SQLiteQuery query = db.query(insertStmt);
 *
 *     query.bindInt("@v", i);
 *     query.step();
 *   }
 *
 *   SQLiteStmt selectStmt = db.prepare("SELECT bar FROM foo;");
 *   SQLiteQuery query = txn.query(selectStmt);
 *   for (; !query.done(); query.step()) {
 *     std::cout << query.getInt(0) << std::endl;
 *   }
 *
 */
struct SQLite {
  SQLite(SQLite&&) noexcept;
  SQLite& operator=(SQLite&&) noexcept;
  ~SQLite();

  /**
   * Return a new SQLite connection, creating the DB file if necessary.
   *
   * path is the location of the DB in your filesystem, or ":memory:"
   * if you want to store data in memory instead.
   */
  static SQLite connect(const folly::StringPiece path);

  /**
   * Compile the given SQL query into a statement object which can run and rerun
   * the query.
   */
  SQLiteStmt prepare(const folly::StringPiece sql);  // throws(SQLiteExc)

  /**
   * Begin a SQLite transaction to run queries within.
   */
  SQLiteTxn begin();

  /**
   * Sleep for a time up to the given maximum when the DB is locked.
   *
   * Setting a number equal to 0 or less than zero will disable the
   * busy timeout.
   */
  void setBusyTimeout(int ms) noexcept;

  enum class SynchronousLevel {
    // Trust the filesystem to fsync for you. This may result in database
    // corruption if power loss occurs.
    OFF = 0,

    // Sync often enough to guarantee consistency in WAL mode.
    NORMAL = 1,

    // Sync on every write.
    FULL = 2,

    // On every write, sync the SQLite DB, its journal or WAL, and the
    // directory containing the files.
    EXTRA = 3,
  };

  /**
   * Tell SQLite when to fsync the DB.
   *
   * https://www.sqlite.org/pragma.html#pragma_synchronous
   */
  void setSynchronousLevel(SynchronousLevel lvl);

  /**
   * Return the most recent error message from SQLite.
   */
  std::string errMsg() const noexcept;

 private:
  friend struct SQLiteStmt;
  friend struct SQLiteTxn;

  explicit SQLite(sqlite3* dbc);

  SQLite() = delete;
  SQLite(const SQLite&) = delete;
  SQLite& operator=(const SQLite&) = delete;

  void txPush();  // throws(SQLiteExc)
  void txPop();   // throws(SQLiteExc)
  void rollback() noexcept;
  void commit();  // throws(SQLiteExc)

  sqlite3* m_dbc = nullptr;
  unsigned m_txDepth = 0;   // Transaction nesting depth.
  bool m_rollback = false;  // If true, rollback rather than commit.

  SQLiteStmt m_beginStmt;
  SQLiteStmt m_rollbackStmt;
  SQLiteStmt m_commitStmt;
};

}  // namespace HPHP
