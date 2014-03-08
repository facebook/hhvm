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

#include "hphp/runtime/ext/pdo_driver.h"
#include <memory>
#include <vector>
#include "hphp/runtime/ext/ext_sqlite3.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class PDOSqlite : public PDODriver {
public:
  PDOSqlite();

  virtual PDOConnection *createConnectionObject();
};

struct PDOSqliteError {
  const char *file;
  int line;
  unsigned int errcode;
  char *errmsg;
};

class PDOSqliteConnection : public PDOConnection {
public:
  PDOSqliteConnection();
  virtual ~PDOSqliteConnection();
  virtual bool create(const Array& options);
  virtual void sweep();

  int handleError(const char *file, int line, PDOStatement *stmt = nullptr);

  virtual bool support(SupportedMethod method);
  virtual bool closer();
  virtual bool preparer(const String& sql,
                        sp_PDOStatement* stmt,
                        const Variant& options);
  virtual int64_t doer(const String& sql);
  virtual bool quoter(const String& input,
                      String& quoted,
                      PDOParamType paramtype);
  virtual bool begin();
  virtual bool commit();
  virtual bool rollback();
  virtual bool setAttribute(int64_t attr, const Variant& value);
  virtual String lastId(const char *name);
  virtual bool fetchErr(PDOStatement *stmt, Array &info);
  virtual int getAttribute(int64_t attr, Variant &value);
  virtual void persistentShutdown();

  bool createFunction(const String& name, const Variant& callback, int argcount);

private:
  sqlite3 *m_db;
  PDOSqliteError m_einfo;
  std::vector<std::shared_ptr<c_SQLite3::UserDefinedFunc>> m_udfs;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_PDO_SQLITE_H_
