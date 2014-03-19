/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_VM_REPO_HELPERS_H_
#define incl_HPHP_VM_REPO_HELPERS_H_

#include "hphp/runtime/base/types.h"

#include "hphp/util/md5.h"
#include "hphp/util/portability.h"

#include <boost/noncopyable.hpp>
#include <sqlite3.h>

namespace HPHP {

// Forward declaration.
class BlobDecoder;
class BlobEncoder;
struct StringData;
struct TypedValue;
class Repo;

enum RepoId {
  RepoIdInvalid = -1,
  RepoIdCentral =  0,
  RepoIdLocal   =  1,

  RepoIdCount   =  2 // Number of database IDs.
};

class RepoExc : public std::exception {
 public:
  RepoExc(const char* fmt, ...) ATTRIBUTE_PRINTF(2, 3) {
    va_list(ap);
    va_start(ap, fmt);
    char* msg;
    if (vasprintf(&msg, fmt, ap) == -1) {
      m_msg = "";
    } else {
      m_msg = msg;
      free(msg);
    }
    va_end(ap);
  }
  ~RepoExc() throw() {}
  const std::string& msg() const { return m_msg; }
  const char* what() const throw() { return m_msg.c_str(); }

 private:
  std::string m_msg;
};

class RepoStmt {
 public:
  explicit RepoStmt(Repo& repo);
  ~RepoStmt();
 private:
  void finalize();
 public:
  bool prepared() const { return (m_stmt != nullptr); }
  void prepare(const std::string& sql);
  void reset();
  Repo& repo() const { return m_repo; }
  sqlite3_stmt*& get() { return m_stmt; }
  const std::string& sql() const { return m_sql; }

 protected:
  Repo& m_repo;
  std::string m_sql;
  sqlite3_stmt* m_stmt;
};

// Under most circumstances RepoTxnQuery should be used instead of RepoQuery;
// queries outside of transactions are fraught with peril.
class RepoQuery {
 public:
  explicit RepoQuery(RepoStmt& stmt)
    : m_stmt(stmt), m_row(false), m_done(false) {
    assert(m_stmt.prepared());
  }
  ~RepoQuery() { m_stmt.reset(); }

  void bindBlob(const char* paramName, const void* blob, size_t size,
                bool isStatic=false);
  void bindBlob(const char* paramName, const BlobEncoder& blob,
                bool isStatic=false);
  void bindMd5(const char* paramName, const MD5& md5);
  void bindTypedValue(const char* paramName, const TypedValue& tv);
  void bindText(const char* paramName, const char* text, size_t size,
                bool isStatic=false);
  void bindStaticString(const char* paramName, const StringData* sd);
  void bindStdString(const char* paramName, const std::string& s);
  void bindDouble(const char* paramName, double val);
  void bindInt(const char* paramName, int val);
  void bindId(const char* paramName, Id id);
  void bindOffset(const char* paramName, Offset offset);
  void bindAttr(const char* paramName, Attr attrs);
  void bindBool(const char* paramName, bool b);
  void bindInt64(const char* paramName, int64_t val);
  void bindNull(const char* paramName);

  void step();
  bool row() const { return m_row; }
  bool done() const { return m_done; }
  void reset();
  void exec();

  // rowid() returns the row id associated with the most recent successful
  // insert.  Thus the rowid is irrelevant for non-insert queries.
  int64_t getInsertedRowid();
  bool isBlob(int iCol);
  bool isText(int iCol);
  bool isDouble(int iCol);
  bool isInt(int iCol);
  bool isNull(int iCol);
  void getBlob(int iCol, const void*& blob, size_t& size);
  BlobDecoder getBlob(int iCol);
  void getMd5(int iCol, MD5& md5);
  void getTypedValue(int iCol, TypedValue& tv);
  void getText(int iCol, const char*& text);
  void getText(int iCol, const char*& text, size_t& size);
  void getStaticString(int iCol, StringData*& s);
  void getStdString(int iCol, std::string& s);
  void getDouble(int iCol, double& val);
  void getInt(int iCol, int& val);
  void getId(int iCol, Id& id);
  void getOffset(int iCol, Offset& offset);
  void getAttr(int iCol, Attr& attrs);
  void getBool(int iCol, bool& b);
  void getInt64(int iCol, int64_t& val);

 protected:
  RepoStmt& m_stmt;
  bool m_row;
  bool m_done;
};

/*
 * Transaction guard object.
 *
 * Semantics: the guard object will rollback the transaction unless
 * you tell it not to.  Call .commit() when you want things to stay.
 */
class RepoTxn : boost::noncopyable {
 public:
  explicit RepoTxn(Repo& repo);
  ~RepoTxn();

  /*
   * All these routines may throw if there is an error accessing the
   * repo.  The RepoTxn object will rollback the entire transaction in
   * any of these cases (which technically means they only provide the
   * basic exception safety guarantee, even though the whole point is
   * to behave transactionally. ;)
   */
  void prepare(RepoStmt& stmt, const std::string& sql);
  void exec(const std::string& sQuery);
  void commit();

  bool error() const { return m_error; }

 private:
  friend class RepoTxnQuery;
  void step(RepoQuery& query);
  void exec(RepoQuery& query);
  void rollback(); // nothrow

  Repo& m_repo;
  bool m_pending;
  bool m_error;
};

class RepoTxnQuery : public RepoQuery {
 public:
  RepoTxnQuery(RepoTxn& txn, RepoStmt& stmt)
    : RepoQuery(stmt), m_txn(txn) {
  }

  void step();
  void exec();

 private:
  RepoTxn& m_txn;
};

class RepoProxy {
 public:
  explicit RepoProxy(Repo& repo) : m_repo(repo) {}
  ~RepoProxy() {}

 protected:
  class Stmt : public RepoStmt {
   public:
    Stmt(Repo& repo, int repoId) : RepoStmt(repo), m_repoId(repoId) {}
   protected:
    int m_repoId;
  };

  Repo& m_repo;
};

}

#endif
