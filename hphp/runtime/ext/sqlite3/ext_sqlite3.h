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

#ifndef incl_HPHP_EXT_SQLITE3_H_
#define incl_HPHP_EXT_SQLITE3_H_

#include <memory>
#include <vector>

#include <sqlite3.h>

#include "hphp/runtime/ext/extension.h"
namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

extern const int64_t k_SQLITE3_ASSOC;
extern const int64_t k_SQLITE3_NUM;
extern const int64_t k_SQLITE3_BOTH;
extern const int64_t k_SQLITE3_INTEGER;
extern const int64_t k_SQLITE3_FLOAT;
extern const int64_t k_SQLITE3_TEXT;
extern const int64_t k_SQLITE3_BLOB;
extern const int64_t k_SQLITE3_NULL;
extern const int64_t k_SQLITE3_OPEN_READONLY;
extern const int64_t k_SQLITE3_OPEN_READWRITE;
extern const int64_t k_SQLITE3_OPEN_CREATE;

///////////////////////////////////////////////////////////////////////////////
// class SQLite3

class SQLite3 {
public:
  SQLite3();
  ~SQLite3();
  void validate() const;
  static Class *getClass();

  struct UserDefinedFunc {
    int argc;
    Variant func;
    Variant step;
    Variant fini;
  };

  sqlite3 *m_raw_db;
  std::vector<std::shared_ptr<UserDefinedFunc>> m_udfs;
  static Class *s_class;
  static const StaticString s_className;
};

void HHVM_METHOD(SQLite3, __construct,
                 const String& filename,
                 int64_t flags /* = SQLITE3_OPEN_READWRITE |
                   SQLITE3_OPEN_CREATE */,
                 const Variant& encryption_key /* = null */);
void HHVM_METHOD(SQLite3, open,
                 const String& filename,
                 int64_t flags /* = SQLITE3_OPEN_READWRITE |
                   SQLITE3_OPEN_CREATE */,
                 const Variant& encryption_key /* = null */);
bool HHVM_METHOD(SQLite3, busytimeout,
                 int64_t msecs);
bool HHVM_METHOD(SQLite3, close);
bool HHVM_METHOD(SQLite3, exec,
                 const String& sql);
Array HHVM_STATIC_METHOD(SQLite3, version);
int64_t HHVM_METHOD(SQLite3, lastinsertrowid);
int64_t HHVM_METHOD(SQLite3, lasterrorcode);
String HHVM_METHOD(SQLite3, lasterrormsg);
bool HHVM_METHOD(SQLite3, loadextension,
                 const String& extension);
int64_t HHVM_METHOD(SQLite3, changes);
String HHVM_STATIC_METHOD(SQLite3, escapestring,
                          const String& sql);
Variant HHVM_METHOD(SQLite3, prepare,
                    const String& sql);
Variant HHVM_METHOD(SQLite3, query,
                    const String& sql);
Variant HHVM_METHOD(SQLite3, querysingle,
                    const String& sql,
                    bool entire_row /* = false */);
bool HHVM_METHOD(SQLite3, createfunction,
                 const String& name,
                 const Variant& callback,
                 int64_t argcount /* = -1 */);
bool HHVM_METHOD(SQLite3, createaggregate,
                 const String& name,
                 const Variant& step,
                 const Variant& final,
                 int64_t argcount /* = -1 */);
bool HHVM_METHOD(SQLite3, openblob,
                 const String& table,
                 const String& column,
                 int64_t rowid,
                 const Variant& dbname /* = null */);

///////////////////////////////////////////////////////////////////////////////
// class SQLite3Stmt

class SQLite3Stmt {
public:
  SQLite3Stmt();
  ~SQLite3Stmt();
  void validate() const;
  static Class *getClass();

  struct BoundParam {
    int type;
    int index;
    Variant value;
  };

  Object m_db;
  sqlite3_stmt *m_raw_stmt;
  std::vector<std::shared_ptr<BoundParam>> m_bound_params;
  static Class *s_class;
  static const StaticString s_className;
};

void HHVM_METHOD(SQLite3Stmt, __construct,
                 const Object& dbobject,
                 const String& statement);
int64_t HHVM_METHOD(SQLite3Stmt, paramcount);
bool HHVM_METHOD(SQLite3Stmt, close);
bool HHVM_METHOD(SQLite3Stmt, reset);
bool HHVM_METHOD(SQLite3Stmt, clear);
bool HHVM_METHOD(SQLite3Stmt, bindparam,
                 const Variant& name,
                 VRefParam parameter,
                 int64_t type /* = SQLITE3_TEXT */);
bool HHVM_METHOD(SQLite3Stmt, bindvalue,
                 const Variant& name,
                 const Variant& parameter,
                 int64_t type /* = SQLITE3_TEXT */);
Variant HHVM_METHOD(SQLite3Stmt, execute);

///////////////////////////////////////////////////////////////////////////////
// class SQLite3Result

class SQLite3Result {
public:
  SQLite3Result();
  void validate() const;
  static Class *getClass();

  Object m_stmt_obj;
  SQLite3Stmt *m_stmt;
  static Class *s_class;
  static const StaticString s_className;
};

int64_t HHVM_METHOD(SQLite3Result, numcolumns);
String HHVM_METHOD(SQLite3Result, columnname,
                   int64_t column);
int64_t HHVM_METHOD(SQLite3Result, columntype,
                    int64_t column);
Variant HHVM_METHOD(SQLite3Result, fetcharray,
                    int64_t mode /* = SQLITE3_BOTH */);
bool HHVM_METHOD(SQLite3Result, reset);
bool HHVM_METHOD(SQLite3Result, finalize);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_SQLITE3_H_
