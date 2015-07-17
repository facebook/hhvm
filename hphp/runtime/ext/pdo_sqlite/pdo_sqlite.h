/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#ifndef incl_HPHP_PDO_SQLITE_H_
#define incl_HPHP_PDO_SQLITE_H_

#include "hphp/runtime/ext/pdo/pdo_driver.h"

#include "hphp/runtime/ext/sqlite3/ext_sqlite3.h"

#include <memory>
#include <string>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct PDOSqlite : PDODriver {
  PDOSqlite();
  req::ptr<PDOResource> createResourceImpl() override;
  req::ptr<PDOResource> createResourceImpl(const sp_PDOConnection&) override;
};

struct PDOSqliteError {
  const char *file;
  int line;
  unsigned int errcode;
  char *errmsg;
};

///////////////////////////////////////////////////////////////////////////////

struct PDOSqliteConnection : PDOConnection {
  friend class PDOSqliteResource;

  PDOSqliteConnection();
  virtual ~PDOSqliteConnection();

  bool create(const Array& options) override;

  bool support(SupportedMethod method) override;
  bool closer() override;
  bool preparer(const String& sql,
                sp_PDOStatement* stmt,
                const Variant& options) override;
  int64_t doer(const String& sql) override;
  bool quoter(const String& input,
              String& quoted,
              PDOParamType paramtype) override;
  bool begin() override;
  bool commit() override;
  bool rollback() override;
  bool setAttribute(int64_t attr, const Variant& value) override;
  String lastId(const char *name) override;
  bool fetchErr(PDOStatement *stmt, Array &info) override;
  int getAttribute(int64_t attr, Variant &value) override;

  bool createFunction(const String& name,
                      const Variant& callback,
                      int argcount);
  void clearFunctions();

  int handleError(const char *file, int line, PDOStatement *stmt = nullptr);

  /////////////////////////////////////////////////////////////////////////////

private:
  struct UDF : SQLite3::UserDefinedFunc {
    std::string name;
  };

  sqlite3 *m_db;
  PDOSqliteError m_einfo;
  std::vector<std::shared_ptr<UDF>> m_udfs;
};

///////////////////////////////////////////////////////////////////////////////

struct PDOSqliteResource : PDOResource {
  explicit PDOSqliteResource(std::shared_ptr<PDOSqliteConnection> conn)
    : PDOResource(std::dynamic_pointer_cast<PDOConnection>(conn))
  {}

  std::shared_ptr<PDOSqliteConnection> conn() const {
    return std::dynamic_pointer_cast<PDOSqliteConnection>(m_conn);
  }

  void sweep() override;
  void persistentSave() override;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_PDO_SQLITE_H_
