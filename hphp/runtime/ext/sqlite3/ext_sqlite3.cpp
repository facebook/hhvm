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

#include "hphp/runtime/ext/sqlite3/ext_sqlite3.h"
#include "hphp/runtime/ext/stream/ext_stream.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/native-data.h"

#include "hphp/system/systemlib.h"

namespace HPHP {

#define PHP_SQLITE3_ASSOC  (1<<0)
#define PHP_SQLITE3_NUM    (1<<1)
#define PHP_SQLITE3_BOTH   (PHP_SQLITE3_ASSOC|PHP_SQLITE3_NUM)

const int64_t k_SQLITE3_ASSOC = PHP_SQLITE3_ASSOC;
const int64_t k_SQLITE3_NUM = PHP_SQLITE3_NUM;
const int64_t k_SQLITE3_BOTH = PHP_SQLITE3_BOTH;
const int64_t k_SQLITE3_INTEGER = SQLITE_INTEGER;
const int64_t k_SQLITE3_FLOAT = SQLITE_FLOAT;
const int64_t k_SQLITE3_TEXT = SQLITE3_TEXT;
const int64_t k_SQLITE3_BLOB = SQLITE_BLOB;
const int64_t k_SQLITE3_NULL = SQLITE_NULL;
const int64_t k_SQLITE3_OPEN_READONLY = SQLITE_OPEN_READONLY;
const int64_t k_SQLITE3_OPEN_READWRITE = SQLITE_OPEN_READWRITE;
const int64_t k_SQLITE3_OPEN_CREATE = SQLITE_OPEN_CREATE;

const StaticString s_SQLITE3_ASSOC("SQLITE3_ASSOC");
const StaticString s_SQLITE3_NUM("SQLITE3_NUM");
const StaticString s_SQLITE3_BOTH("SQLITE3_BOTH");
const StaticString s_SQLITE3_INTEGER("SQLITE3_INTEGER");
const StaticString s_SQLITE3_FLOAT("SQLITE3_FLOAT");
const StaticString s_SQLITE3_TEXT("SQLITE3_TEXT");
const StaticString s_SQLITE3_BLOB("SQLITE3_BLOB");
const StaticString s_SQLITE3_NULL("SQLITE3_NULL");
const StaticString s_SQLITE3_OPEN_READONLY("SQLITE3_OPEN_READONLY");
const StaticString s_SQLITE3_OPEN_READWRITE("SQLITE3_OPEN_READWRITE");
const StaticString s_SQLITE3_OPEN_CREATE("SQLITE3_OPEN_CREATE");

#define IMPLEMENT_GET_CLASS(cls)                                               \
  Class *cls::getClass() {                                                     \
    if (s_class == nullptr) {                                                  \
      s_class = Unit::lookupClass(s_className.get());                          \
      assert(s_class);                                                         \
    }                                                                          \
    return s_class;                                                            \
  }                                                                            \

///////////////////////////////////////////////////////////////////////////////
// helpers

struct php_sqlite3_agg_context {
  Variant context;
  int64_t row_count;
};

static Variant get_column_value(sqlite3_stmt *stmt, int column) {
  assert(stmt);
  Variant data;
  switch (sqlite3_column_type(stmt, column)) {
  case SQLITE_INTEGER:
    data = (int64_t)sqlite3_column_int64(stmt, column);
    break;
  case SQLITE_FLOAT:
    data = (double)sqlite3_column_double(stmt, column);
    break;
  case SQLITE_NULL:
    break;
  case SQLITE3_TEXT:
    data = String((char*)sqlite3_column_text(stmt, column), CopyString);
    break;
  case SQLITE_BLOB:
  default:
    data = String((char*)sqlite3_column_blob(stmt, column),
                  sqlite3_column_bytes(stmt, column), CopyString);
  }
  return data;
}

static Variant get_value(sqlite3_value *argv) {
  Variant value;
  switch (sqlite3_value_type(argv)) {
  case SQLITE_INTEGER:
    value = (int64_t)sqlite3_value_int(argv);
    break;
  case SQLITE_FLOAT:
    value = (double)sqlite3_value_double(argv);
    break;
  case SQLITE_NULL:
    break;
  case SQLITE_BLOB:
  case SQLITE3_TEXT:
  default:
    value = String((char*)sqlite3_value_text(argv),
                   sqlite3_value_bytes(argv), CopyString);
    break;
  }
  return value;
}

static void sqlite3_do_callback(sqlite3_context *context,
                                const Variant& callback,
                                int argc,
                                sqlite3_value **argv,
                                bool is_agg) {
  Array params = Array::Create();
  php_sqlite3_agg_context *agg_context = nullptr;
  if (is_agg) {
    agg_context = (php_sqlite3_agg_context *)sqlite3_aggregate_context
      (context, sizeof(php_sqlite3_agg_context));
    params.appendRef(agg_context->context);
    params.append(agg_context->row_count);
  }
  for (int i = 0; i < argc; i++) {
    params.append(get_value(argv[i]));
  }
  Variant ret = vm_call_user_func(callback, params);

  if (!is_agg || !argv) {
    /* only set the sqlite return value if we are a scalar function,
     * or if we are finalizing an aggregate */
    if (ret.isInteger()) {
      sqlite3_result_int(context, ret.toInt64());
    } else if (ret.isNull()) {
      sqlite3_result_null(context);
    } else if (ret.isDouble()) {
      sqlite3_result_double(context, ret.toDouble());
    } else {
      String sret = ret.toString();
      sqlite3_result_text(context, sret.data(), sret.size(), SQLITE_TRANSIENT);
    }
  } else {
    /* we're stepping in an aggregate; the return value goes into
     * the context */
    agg_context->context = ret;
  }
}

void php_sqlite3_callback_func(sqlite3_context *context, int argc,
                               sqlite3_value **argv) {
  SQLite3::UserDefinedFunc *udf =
    (SQLite3::UserDefinedFunc*)sqlite3_user_data(context);
  sqlite3_do_callback(context, udf->func, argc, argv, false);
}

static void php_sqlite3_callback_step(sqlite3_context *context, int argc,
                                      sqlite3_value **argv) {
  php_sqlite3_agg_context *agg_context =
    (php_sqlite3_agg_context *)sqlite3_aggregate_context
    (context, sizeof(php_sqlite3_agg_context));
  agg_context->row_count++;

  SQLite3::UserDefinedFunc *udf =
    (SQLite3::UserDefinedFunc*)sqlite3_user_data(context);
  sqlite3_do_callback(context, udf->step, argc, argv, true);
}

static void php_sqlite3_callback_final(sqlite3_context *context) {
  php_sqlite3_agg_context *agg_context =
    (php_sqlite3_agg_context *)sqlite3_aggregate_context
    (context, sizeof(php_sqlite3_agg_context));
  agg_context->row_count = 0;

  SQLite3::UserDefinedFunc *udf =
    (SQLite3::UserDefinedFunc*)sqlite3_user_data(context);
  sqlite3_do_callback(context, udf->fini, 0, nullptr, true);
}

///////////////////////////////////////////////////////////////////////////////
// sqlite3

Class *SQLite3::s_class = nullptr;
const StaticString SQLite3::s_className("SQLite3");

IMPLEMENT_GET_CLASS(SQLite3)

SQLite3::SQLite3() : m_raw_db(nullptr) {
}

SQLite3::~SQLite3() {
  if (m_raw_db) {
    sqlite3_close(m_raw_db);
  }
}

void HHVM_METHOD(SQLite3, __construct,
                 const String& filename,
                 int64_t flags /* = SQLITE3_OPEN_READWRITE |
                   SQLITE3_OPEN_CREATE */,
                 const Variant& encryption_key /* = null */) {
  HHVM_MN(SQLite3, open)(this_, filename, flags, encryption_key);
}

void SQLite3::validate() const {
  if (!m_raw_db) {
    throw Exception("SQLite3 object was not initialized");
  }
}

void HHVM_METHOD(SQLite3, open,
                 const String& filename,
                 int64_t flags /* = SQLITE3_OPEN_READWRITE |
                   SQLITE3_OPEN_CREATE */,
                 const Variant& encryption_key /* = null */) {
  auto *data = Native::data<SQLite3>(this_);
  if (data->m_raw_db) {
    throw Exception("Already initialized DB Object");
  }

  String fname;
  if (strncmp(filename.data(), ":memory:", 8) != 0) {
    fname = File::TranslatePath(filename);
  } else {
    fname = filename; // in-memory db
  }

  if (sqlite3_open_v2(fname.data(), &data->m_raw_db, flags, nullptr)
      != SQLITE_OK) {
    throw Exception("Unable to open database: %s",
                    sqlite3_errmsg(data->m_raw_db));
  }

#ifdef SQLITE_HAS_CODEC
  const String& str_encryption_key = encryption_key.isNull()
                                   ? null_string
                                   : encryption_key.toString();
  if (!str_encryption_key.empty() &&
      sqlite3_key(data->m_raw_db, str_encryption_key.data(),
      str_encryption_key.size()) != SQLITE_OK) {
    throw Exception("Unable to open database: %s",
                    sqlite3_errmsg(data->m_raw_db));
  }
#endif
}

bool HHVM_METHOD(SQLite3, busytimeout,
                 int64_t msecs) {
  auto *data = Native::data<SQLite3>(this_);
  data->validate();
  int errcode = sqlite3_busy_timeout(data->m_raw_db, msecs);
  if (errcode != SQLITE_OK) {
    raise_warning("Unable to set busy timeout: %d, %s", errcode,
                  sqlite3_errmsg(data->m_raw_db));
    return false;
  }
  return true;
}

bool HHVM_METHOD(SQLite3, close) {
  auto *data = Native::data<SQLite3>(this_);
  if (data->m_raw_db) {
    int errcode = sqlite3_close(data->m_raw_db);
    if (errcode != SQLITE_OK) {
      raise_warning("Unable to close database: %d, %s", errcode,
                    sqlite3_errmsg(data->m_raw_db));
      return false;
    }
    data->m_raw_db = nullptr;
  }
  return true;
}

bool HHVM_METHOD(SQLite3, exec,
                 const String& sql) {
  auto *data = Native::data<SQLite3>(this_);
  SYNC_VM_REGS_SCOPED();
  data->validate();

  char *errtext = nullptr;
  if (sqlite3_exec(data->m_raw_db, sql.data(), nullptr, nullptr, &errtext)
      != SQLITE_OK) {
    raise_warning("%s", errtext);
    sqlite3_free(errtext);
    return false;
  }
  return true;
}

const StaticString
  s_versionString("versionString"),
  s_versionNumber("versionNumber");

Array HHVM_STATIC_METHOD(SQLite3, version) {
  return make_map_array(
    s_versionString, String((char*)sqlite3_libversion(), CopyString),
    s_versionNumber, (int64_t)sqlite3_libversion_number()
  );
}

int64_t HHVM_METHOD(SQLite3, lastinsertrowid) {
  auto *data = Native::data<SQLite3>(this_);
  data->validate();
  return sqlite3_last_insert_rowid(data->m_raw_db);
}

int64_t HHVM_METHOD(SQLite3, lasterrorcode) {
  auto *data = Native::data<SQLite3>(this_);
  data->validate();
  return sqlite3_errcode(data->m_raw_db);
}

String HHVM_METHOD(SQLite3, lasterrormsg) {
  auto *data = Native::data<SQLite3>(this_);
  data->validate();
  return String((char*)sqlite3_errmsg(data->m_raw_db), CopyString);
}

bool HHVM_METHOD(SQLite3, loadextension,
                 const String& extension) {
  auto *data = Native::data<SQLite3>(this_);
  data->validate();

  String translated = File::TranslatePath(extension);
  if (translated.empty()) {
    raise_warning("Unable to load extension at '%s'", extension.data());
    return false;
  }

  char *errtext = nullptr;
  sqlite3_enable_load_extension(data->m_raw_db, 1);
  if (sqlite3_load_extension(data->m_raw_db, translated.data(), 0, &errtext)
      != SQLITE_OK) {
    raise_warning("%s", errtext);
    sqlite3_free(errtext);
    sqlite3_enable_load_extension(data->m_raw_db, 0);
    return false;
  }
  sqlite3_enable_load_extension(data->m_raw_db, 0);
  return true;
}

int64_t HHVM_METHOD(SQLite3, changes) {
  auto *data = Native::data<SQLite3>(this_);
  data->validate();
  return sqlite3_changes(data->m_raw_db);
}

String HHVM_STATIC_METHOD(SQLite3, escapestring,
                          const String& sql) {
  if (!sql.empty()) {
    char *ret = sqlite3_mprintf("%q", sql.data());
    if (ret) {
      String sret(ret, CopyString);
      sqlite3_free(ret);
      return sret;
    }
  }
  return empty_string();
}

Variant HHVM_METHOD(SQLite3, prepare,
                    const String& sql) {
  auto *data = Native::data<SQLite3>(this_);
  data->validate();
  if (!sql.empty()) {
    Object ret{SQLite3Stmt::getClass()};
    SQLite3Stmt *stmt = Native::data<SQLite3Stmt>(ret);
    HHVM_MN(SQLite3Stmt, __construct)(ret.get(), this_, sql);
    if (stmt->m_raw_stmt) {
      return ret;
    }
  }
  return false;
}

Variant HHVM_METHOD(SQLite3, query,
                    const String& sql) {
  auto *data = Native::data<SQLite3>(this_);
  SYNC_VM_REGS_SCOPED();
  data->validate();
  if (!sql.empty()) {
    Variant stmt = HHVM_MN(SQLite3, prepare)(this_, sql);
    if (!same(stmt, false)) {
      Object obj_stmt = stmt.toObject();
      assert(obj_stmt.instanceof(SQLite3Stmt::getClass()));
      return HHVM_MN(SQLite3Stmt, execute)(obj_stmt.get());
    }
  }
  return false;
}

Variant HHVM_METHOD(SQLite3, querysingle,
                    const String& sql,
                    bool entire_row /* = false */) {
  auto *data = Native::data<SQLite3>(this_);
  SYNC_VM_REGS_SCOPED();
  data->validate();
  if (!sql.empty()) {
    Variant stmt = HHVM_MN(SQLite3, prepare)(this_, sql);
    if (!same(stmt, false)) {
      Object obj_stmt = stmt.toObject();
      assert(obj_stmt.instanceof(SQLite3Stmt::getClass()));
      sqlite3_stmt *pstmt =
        Native::data<SQLite3Stmt>(obj_stmt)->m_raw_stmt;
      switch (sqlite3_step(pstmt)) {
      case SQLITE_ROW: /* Valid Row */
        if (entire_row) {
          Array ret = Array::Create();
          for (int i = 0; i < sqlite3_data_count(pstmt); i++) {
            ret.set(String((char*)sqlite3_column_name(pstmt, i), CopyString),
                    get_column_value(pstmt, i));
          }
          return ret;
        }
        return get_column_value(pstmt, 0);
      case SQLITE_DONE: /* Valid but no results */
        if (entire_row) {
          return empty_array();
        } else {
          return init_null();
        }
      default:
        raise_warning("Unable to execute statement: %s",
                      sqlite3_errmsg(data->m_raw_db));
      }
    }
  }
  return false;
}

bool HHVM_METHOD(SQLite3, createfunction,
                 const String& name,
                 const Variant& callback,
                 int64_t argcount /* = -1 */) {
  auto *data = Native::data<SQLite3>(this_);
  data->validate();
  if (name.empty()) {
    return false;
  }
  if (!is_callable(callback)) {
    raise_warning("Not a valid callback function %s",
                  callback.toString().data());
    return false;
  }

  auto udf = std::make_shared<SQLite3::UserDefinedFunc>();
  if (sqlite3_create_function(data->m_raw_db, name.data(), argcount,
                              SQLITE_UTF8, udf.get(), php_sqlite3_callback_func,
                              nullptr, nullptr) == SQLITE_OK) {
    udf->func = callback;
    udf->argc = argcount;
    data->m_udfs.push_back(udf);
    return true;
  }
  return false;
}

bool HHVM_METHOD(SQLite3, createaggregate,
                 const String& name,
                 const Variant& step,
                 const Variant& final,
                 int64_t argcount /* = -1 */) {
  auto *data = Native::data<SQLite3>(this_);
  data->validate();
  if (name.empty()) {
    return false;
  }
  if (!is_callable(step)) {
    raise_warning("Not a valid callback function %s",
                  step.toString().data());
    return false;
  }
  if (!is_callable(final)) {
    raise_warning("Not a valid callback function %s",
                  final.toString().data());
    return false;
  }

  auto udf = std::make_shared<SQLite3::UserDefinedFunc>();
  if (sqlite3_create_function(data->m_raw_db, name.data(), argcount,
                              SQLITE_UTF8, udf.get(), nullptr,
                              php_sqlite3_callback_step,
                              php_sqlite3_callback_final) == SQLITE_OK) {
    udf->step = step;
    udf->fini = final;
    udf->argc = argcount;
    data->m_udfs.push_back(udf);
    return true;
  }
  return false;
}

bool HHVM_METHOD(SQLite3, openblob,
                 const String& table,
                 const String& column,
                 int64_t rowid,
                 const Variant& dbname /* = null */) {
  throw_not_supported(__func__, "sqlite3 stream");
}

///////////////////////////////////////////////////////////////////////////////

Class *SQLite3Stmt::s_class = nullptr;
const StaticString SQLite3Stmt::s_className("SQLite3Stmt");

IMPLEMENT_GET_CLASS(SQLite3Stmt)

SQLite3Stmt::SQLite3Stmt() : m_raw_stmt(nullptr) {
}

SQLite3Stmt::~SQLite3Stmt() {
  if (m_raw_stmt) {
    sqlite3_finalize(m_raw_stmt);
  }
}

void HHVM_METHOD(SQLite3Stmt, __construct,
                 const Object& dbobject,
                 const String& statement) {
  auto *data = Native::data<SQLite3Stmt>(this_);
  if (!statement.empty()) {
    assert(dbobject.instanceof(SQLite3::getClass()));
    const SQLite3 *db = Native::data<SQLite3>(dbobject);
    db->validate();
    data->m_db = dbobject;

    int errcode = sqlite3_prepare_v2(db->m_raw_db, statement.data(),
                                     statement.size(), &data->m_raw_stmt,
                                     nullptr);
    if (errcode != SQLITE_OK) {
      raise_warning("Unable to prepare statement: %d, %s",
                    errcode, sqlite3_errmsg(db->m_raw_db));
    }
  }
}

void SQLite3Stmt::validate() const {
  if (!m_raw_stmt) {
    throw Exception("SQLite3Stmt object was not initialized");
  }
}

int64_t HHVM_METHOD(SQLite3Stmt, paramcount) {
  auto *data = Native::data<SQLite3Stmt>(this_);
  data->validate();
  return sqlite3_bind_parameter_count(data->m_raw_stmt);
}

bool HHVM_METHOD(SQLite3Stmt, close) {
  auto *data = Native::data<SQLite3Stmt>(this_);
  if (data->m_raw_stmt) {
    sqlite3_finalize(data->m_raw_stmt);
    data->m_raw_stmt = nullptr;
  }
  return true;
}

bool HHVM_METHOD(SQLite3Stmt, reset) {
  auto *data = Native::data<SQLite3Stmt>(this_);
  data->validate();
  if (sqlite3_reset(data->m_raw_stmt) != SQLITE_OK) {
    raise_warning("Unable to reset statement: %s",
                  sqlite3_errmsg(sqlite3_db_handle(data->m_raw_stmt)));
    return false;
  }
  return true;
}

bool HHVM_METHOD(SQLite3Stmt, clear) {
  auto *data = Native::data<SQLite3Stmt>(this_);
  data->validate();
  if (sqlite3_clear_bindings(data->m_raw_stmt) != SQLITE_OK) {
    raise_warning("Unable to clear statement: %s",
                  sqlite3_errmsg(sqlite3_db_handle(data->m_raw_stmt)));
    return false;
  }
  data->m_bound_params.clear();
  return true;
}

bool HHVM_METHOD(SQLite3Stmt, bindparam,
                 const Variant& name,
                 VRefParam parameter,
                 int64_t type /* = SQLITE3_TEXT */) {
  auto *data = Native::data<SQLite3Stmt>(this_);
  auto param = std::make_shared<SQLite3Stmt::BoundParam>();
  param->type = type;
  param->value.assignRef(parameter);

  if (name.isString()) {
    String sname = name.toString();
    if (sname.charAt(0) != ':') {
      /* We need a : prefix to resolve a name to a parameter number */
      sname = String(":") + sname;
    }
    param->index = sqlite3_bind_parameter_index(data->m_raw_stmt, sname.data());
  } else {
    param->index = name.toInt64();
  }
  if (param->index < 1) {
    return false;
  }

  data->m_bound_params.push_back(param);
  return true;
}

bool HHVM_METHOD(SQLite3Stmt, bindvalue,
                 const Variant& name,
                 const Variant& parameter,
                 int64_t type /* = SQLITE3_TEXT */) {
  Variant v = parameter;
  return HHVM_MN(SQLite3Stmt, bindparam)(this_, name, v, type);
}

Variant HHVM_METHOD(SQLite3Stmt, execute) {
  auto *data = Native::data<SQLite3Stmt>(this_);
  SYNC_VM_REGS_SCOPED();
  data->validate();

  for (unsigned int i = 0; i < data->m_bound_params.size(); i++) {
    SQLite3Stmt::BoundParam &p = *data->m_bound_params[i];
    if (p.value.isNull()) {
      sqlite3_bind_null(data->m_raw_stmt, p.index);
      continue;
    }

    switch (p.type) {
    case SQLITE_INTEGER:
      sqlite3_bind_int(data->m_raw_stmt, p.index, p.value.toInt64());
      break;
    case SQLITE_FLOAT:
      sqlite3_bind_double(data->m_raw_stmt, p.index, p.value.toDouble());
      break;
    case SQLITE_BLOB:
      {
        String sblob;
        if (p.value.isResource()) {
          Variant blob = HHVM_FN(stream_get_contents)(p.value.toResource());
          if (same(blob, false)) {
            raise_warning("Unable to read stream for parameter %d",
                          p.index);
            return false;
          }
          sblob = blob.toString();
        } else {
          sblob = p.value.toString();
        }
        sqlite3_bind_blob(data->m_raw_stmt, p.index, sblob.data(), sblob.size(),
                          SQLITE_TRANSIENT);
        break;
      }
    case SQLITE3_TEXT:
      {
        String stext = p.value.toString();
        sqlite3_bind_text(data->m_raw_stmt, p.index, stext.data(), stext.size(),
                          SQLITE_STATIC);
        break;
      }
    case SQLITE_NULL:
      sqlite3_bind_null(data->m_raw_stmt, p.index);
      break;
    default:
      raise_warning("Unknown parameter type: %d for parameter %d",
                    p.type, p.index);
      return false;
    }
  }

  switch (sqlite3_step(data->m_raw_stmt)) {
  case SQLITE_ROW: /* Valid Row */
  case SQLITE_DONE: /* Valid but no results */
    {
      sqlite3_reset(data->m_raw_stmt);
      Object ret{SQLite3Result::getClass()};
      SQLite3Result *result = Native::data<SQLite3Result>(ret);
      result->m_stmt_obj = Object(this_);
      result->m_stmt = data;
      return ret;
    }
  case SQLITE_ERROR:
    sqlite3_reset(data->m_raw_stmt);
    // fall through
  default:
    raise_warning("Unable to execute statement: %s",
                  sqlite3_errmsg(sqlite3_db_handle(data->m_raw_stmt)));
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

Class *SQLite3Result::s_class = nullptr;
const StaticString SQLite3Result::s_className("SQLite3Result");

IMPLEMENT_GET_CLASS(SQLite3Result)

SQLite3Result::SQLite3Result() : m_stmt(nullptr) {
}

void SQLite3Result::validate() const {
  if (!m_stmt) {
    throw Exception("SQLite3Result object was not initialized");
  }
  m_stmt->validate();
}

int64_t HHVM_METHOD(SQLite3Result, numcolumns) {
  auto *data = Native::data<SQLite3Result>(this_);
  data->validate();
  return sqlite3_column_count(data->m_stmt->m_raw_stmt);
}

String HHVM_METHOD(SQLite3Result, columnname,
                   int64_t column) {
  auto *data = Native::data<SQLite3Result>(this_);
  data->validate();
  return String((char*)sqlite3_column_name(data->m_stmt->m_raw_stmt, column),
                CopyString);
}

int64_t HHVM_METHOD(SQLite3Result, columntype,
                    int64_t column) {
  auto *data = Native::data<SQLite3Result>(this_);
  data->validate();
  return sqlite3_column_type(data->m_stmt->m_raw_stmt, column);
}

Variant HHVM_METHOD(SQLite3Result, fetcharray,
                    int64_t mode /* = SQLITE3_BOTH */) {
  auto *data = Native::data<SQLite3Result>(this_);
  SYNC_VM_REGS_SCOPED();
  data->validate();

  switch (sqlite3_step(data->m_stmt->m_raw_stmt)) {
  case SQLITE_ROW:
    if (mode & PHP_SQLITE3_BOTH) {
      Array ret = Array::Create();
      for (int i = 0; i < sqlite3_data_count(data->m_stmt->m_raw_stmt); i++) {
        Variant value = get_column_value(data->m_stmt->m_raw_stmt, i);
        if (mode & PHP_SQLITE3_NUM) {
          ret.set(i, value);
        }
        if (mode & PHP_SQLITE3_ASSOC) {
          ret.set(HHVM_MN(SQLite3Result, columnname)(this_, i), value);
        }
      }
      return ret;
    }
    break;
  case SQLITE_DONE:
    return false;
  default:
    raise_warning("Unable to execute statement: %s",
                  sqlite3_errmsg(sqlite3_db_handle(data->m_stmt->m_raw_stmt)));
  }
  return init_null();
}

bool HHVM_METHOD(SQLite3Result, reset) {
  auto *data = Native::data<SQLite3Result>(this_);
  data->validate();
  return sqlite3_reset(data->m_stmt->m_raw_stmt) == SQLITE_OK;
}

bool HHVM_METHOD(SQLite3Result, finalize) {
  auto *data = Native::data<SQLite3Result>(this_);
  data->validate();
  data->m_stmt_obj.reset();
  data->m_stmt = nullptr;
  return true;
}

///////////////////////////////////////////////////////////////////////////////

#define REGISTER_CONSTANT(name)                                                \
  Native::registerConstant<KindOfInt64>(s_##name.get(), k_##name)              \

static class SQLite3Extension final : public Extension {
public:
  SQLite3Extension() : Extension("sqlite3", "0.7-dev") {}
  void moduleInit() override {
    REGISTER_CONSTANT(SQLITE3_ASSOC);
    REGISTER_CONSTANT(SQLITE3_NUM);
    REGISTER_CONSTANT(SQLITE3_BOTH);
    REGISTER_CONSTANT(SQLITE3_INTEGER);
    REGISTER_CONSTANT(SQLITE3_FLOAT);
    REGISTER_CONSTANT(SQLITE3_TEXT);
    REGISTER_CONSTANT(SQLITE3_BLOB);
    REGISTER_CONSTANT(SQLITE3_NULL);
    REGISTER_CONSTANT(SQLITE3_OPEN_READONLY);
    REGISTER_CONSTANT(SQLITE3_OPEN_READWRITE);
    REGISTER_CONSTANT(SQLITE3_OPEN_CREATE);

    HHVM_ME(SQLite3, __construct);
    HHVM_ME(SQLite3, open);
    HHVM_ME(SQLite3, busytimeout);
    HHVM_ME(SQLite3, close);
    HHVM_ME(SQLite3, exec);
    HHVM_ME(SQLite3, lastinsertrowid);
    HHVM_ME(SQLite3, lasterrorcode);
    HHVM_ME(SQLite3, lasterrormsg);
    HHVM_ME(SQLite3, loadextension);
    HHVM_ME(SQLite3, changes);
    HHVM_ME(SQLite3, prepare);
    HHVM_ME(SQLite3, query);
    HHVM_ME(SQLite3, querysingle);
    HHVM_ME(SQLite3, createfunction);
    HHVM_ME(SQLite3, createaggregate);
    HHVM_ME(SQLite3, openblob);
    HHVM_STATIC_ME(SQLite3, version);
    HHVM_STATIC_ME(SQLite3, escapestring);
    Native::registerNativeDataInfo<SQLite3>(SQLite3::s_className.get(),
                                            Native::NDIFlags::NO_SWEEP);

    HHVM_ME(SQLite3Stmt, __construct);
    HHVM_ME(SQLite3Stmt, paramcount);
    HHVM_ME(SQLite3Stmt, close);
    HHVM_ME(SQLite3Stmt, reset);
    HHVM_ME(SQLite3Stmt, clear);
    HHVM_ME(SQLite3Stmt, bindparam);
    HHVM_ME(SQLite3Stmt, bindvalue);
    HHVM_ME(SQLite3Stmt, execute);
    Native::registerNativeDataInfo<SQLite3Stmt>(SQLite3Stmt::s_className.get(),
                                                Native::NDIFlags::NO_SWEEP);

    HHVM_ME(SQLite3Result, numcolumns);
    HHVM_ME(SQLite3Result, columnname);
    HHVM_ME(SQLite3Result, columntype);
    HHVM_ME(SQLite3Result, fetcharray);
    HHVM_ME(SQLite3Result, reset);
    HHVM_ME(SQLite3Result, finalize);
    Native::registerNativeDataInfo<SQLite3Result>(
      SQLite3Result::s_className.get(), Native::NDIFlags::NO_SWEEP);

    loadSystemlib();
  }
} s_sqlite3_extension;

///////////////////////////////////////////////////////////////////////////////
}
