/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/req-vector.h"
#include <sqlite3.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct SQLite3 : SystemLib::ClassLoader<"SQLite3"> {
  SQLite3();
  ~SQLite3();
  void validate() const;

  struct UserDefinedFunc {
    int argc;
    Variant func;
    Variant step;
    Variant fini;
  };

public:
  sqlite3 *m_raw_db;
  req::vector<req::shared_ptr<UserDefinedFunc>> m_udfs;
};

void HHVM_METHOD(SQLite3, open,
                 const String& filename,
                 int64_t flags /* = SQLITE3_OPEN_READWRITE |
                   SQLITE3_OPEN_CREATE */,
                 const Variant& encryption_key /* = null */);

///////////////////////////////////////////////////////////////////////////////

struct SQLite3Stmt : SystemLib::ClassLoader<"SQLite3Stmt"> {
  SQLite3Stmt();
  ~SQLite3Stmt();
  void validate() const;

  struct BoundParam {
    int type;
    int index;
    Variant value;
  };

public:
  Object m_db;
  sqlite3_stmt *m_raw_stmt;
  req::vector<req::shared_ptr<BoundParam>> m_bound_params;
};

void HHVM_METHOD(SQLite3Stmt, __construct,
                 const Object& dbobject,
                 const String& statement);
Variant HHVM_METHOD(SQLite3Stmt, execute);

///////////////////////////////////////////////////////////////////////////////

struct SQLite3Result : SystemLib::ClassLoader<"SQLite3Result"> {
  SQLite3Result();
  void validate() const;

public:
  Object m_stmt_obj;
  SQLite3Stmt *m_stmt; // XXX why not scanned?
};

///////////////////////////////////////////////////////////////////////////////
}
