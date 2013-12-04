/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/ext_sqlite3.h"
#include "hphp/runtime/ext/ext_stream.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

#include "hphp/system/systemlib.h"

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(sqlite3);
///////////////////////////////////////////////////////////////////////////////

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

static void sqlite3_do_callback(sqlite3_context *context, CVarRef callback,
                                int argc, sqlite3_value **argv, bool is_agg) {
  Array params = Array::Create();
  php_sqlite3_agg_context *agg_context = NULL;
  if (is_agg) {
    agg_context = (php_sqlite3_agg_context *)sqlite3_aggregate_context
      (context, sizeof(php_sqlite3_agg_context));
    params.append(ref(agg_context->context));
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
  c_SQLite3::UserDefinedFunc *udf =
    (c_SQLite3::UserDefinedFunc*)sqlite3_user_data(context);
  sqlite3_do_callback(context, udf->func, argc, argv, false);
}

static void php_sqlite3_callback_step(sqlite3_context *context, int argc,
                                      sqlite3_value **argv) {
  php_sqlite3_agg_context *agg_context =
    (php_sqlite3_agg_context *)sqlite3_aggregate_context
    (context, sizeof(php_sqlite3_agg_context));
  agg_context->row_count++;

  c_SQLite3::UserDefinedFunc *udf =
    (c_SQLite3::UserDefinedFunc*)sqlite3_user_data(context);
  sqlite3_do_callback(context, udf->step, argc, argv, true);
}

static void php_sqlite3_callback_final(sqlite3_context *context) {
  php_sqlite3_agg_context *agg_context =
    (php_sqlite3_agg_context *)sqlite3_aggregate_context
    (context, sizeof(php_sqlite3_agg_context));
  agg_context->row_count = 0;

  c_SQLite3::UserDefinedFunc *udf =
    (c_SQLite3::UserDefinedFunc*)sqlite3_user_data(context);
  sqlite3_do_callback(context, udf->fini, 0, NULL, true);
}

///////////////////////////////////////////////////////////////////////////////
// sqlite3

c_SQLite3::c_SQLite3(Class* cb) :
    ExtObjectData(cb), m_raw_db(NULL) {
}

c_SQLite3::~c_SQLite3() {
  if (m_raw_db) {
    sqlite3_close(m_raw_db);
  }
}

void c_SQLite3::t___construct(const String& filename,
                       int64_t flags /* = k_SQLITE3_OPEN_READWRITE |
                                      k_SQLITE3_OPEN_CREATE */,
                       const String& encryption_key /* = null_string */) {
  t_open(filename, flags, encryption_key);
}

void c_SQLite3::validate() const {
  if (!m_raw_db) {
    throw Exception("SQLite3 object was not initialized");
  }
}

void c_SQLite3::t_open(const String& filename,
                       int64_t flags /* = k_SQLITE3_OPEN_READWRITE |
                                      k_SQLITE3_OPEN_CREATE */,
                       const String& encryption_key /* = null_string */) {
  if (m_raw_db) {
    throw Exception("Already initialized DB Object");
  }

  String fname;
  if (strncmp(filename.data(), ":memory:", 8) != 0) {
    fname = File::TranslatePath(filename);
  } else {
    fname = filename; // in-memory db
  }

  if (sqlite3_open_v2(fname.data(), &m_raw_db, flags, NULL) != SQLITE_OK) {
    throw Exception("Unable to open database: %s", sqlite3_errmsg(m_raw_db));
  }

#ifdef SQLITE_HAS_CODEC
  if (!encryption_key.empty() &&
      sqlite3_key(m_raw_db, encryption_key.data(), encryption_key.size())
      != SQLITE_OK) {
    throw Exception("Unable to open database: %s", sqlite3_errmsg(m_raw_db));
  }
#endif
}

bool c_SQLite3::t_busytimeout(int64_t msecs) {
  validate();
  int errcode = sqlite3_busy_timeout(m_raw_db, msecs);
  if (errcode != SQLITE_OK) {
    raise_warning("Unable to set busy timeout: %d, %s", errcode,
                  sqlite3_errmsg(m_raw_db));
    return false;
  }
  return true;
}

bool c_SQLite3::t_close() {
  if (m_raw_db) {
    int errcode = sqlite3_close(m_raw_db);
    if (errcode != SQLITE_OK) {
      raise_warning("Unable to close database: %d, %s", errcode,
                    sqlite3_errmsg(m_raw_db));
      return false;
    }
    m_raw_db = NULL;
  }
  return true;
}

bool c_SQLite3::t_exec(const String& sql) {
  SYNC_VM_REGS_SCOPED();
  validate();

  char *errtext = NULL;
  if (sqlite3_exec(m_raw_db, sql.data(), NULL, NULL, &errtext) != SQLITE_OK) {
    raise_warning("%s", errtext);
    sqlite3_free(errtext);
    return false;
  }
  return true;
}

const StaticString
  s_versionString("versionString"),
  s_versionNumber("versionNumber");

Array c_SQLite3::t_version() {
  ArrayInit ret(2);
  ret.set(s_versionString, String((char*)sqlite3_libversion(), CopyString));
  ret.set(s_versionNumber, (int64_t)sqlite3_libversion_number());
  return ret.create();
}

int64_t c_SQLite3::t_lastinsertrowid() {
  validate();
  return sqlite3_last_insert_rowid(m_raw_db);
}

int64_t c_SQLite3::t_lasterrorcode() {
  validate();
  return sqlite3_errcode(m_raw_db);
}

String c_SQLite3::t_lasterrormsg() {
  validate();
  return String((char*)sqlite3_errmsg(m_raw_db), CopyString);
}

bool c_SQLite3::t_loadextension(const String& extension) {
  validate();

  String translated = File::TranslatePath(extension);
  if (translated.empty()) {
    raise_warning("Unable to load extension at '%s'", extension.data());
    return false;
  }

  char *errtext = NULL;
  sqlite3_enable_load_extension(m_raw_db, 1);
  if (sqlite3_load_extension(m_raw_db, translated.data(), 0, &errtext)
      != SQLITE_OK) {
    raise_warning("%s", errtext);
    sqlite3_free(errtext);
    sqlite3_enable_load_extension(m_raw_db, 0);
    return false;
  }
  sqlite3_enable_load_extension(m_raw_db, 0);
  return true;
}

int64_t c_SQLite3::t_changes() {
  validate();
  return sqlite3_changes(m_raw_db);
}

String c_SQLite3::ti_escapestring(const String& sql) {
  if (!sql.empty()) {
    char *ret = sqlite3_mprintf("%q", sql.data());
    if (ret) {
      String sret(ret, CopyString);
      sqlite3_free(ret);
      return sret;
    }
  }
  return "";
}

Variant c_SQLite3::t_prepare(const String& sql) {
  validate();
  if (!sql.empty()) {
    c_SQLite3Stmt *stmt = NEWOBJ(c_SQLite3Stmt)();
    Object ret(stmt);
    stmt->t___construct(p_SQLite3(this), sql);
    if (stmt->m_raw_stmt) {
      return ret;
    }
  }
  return false;
}

Variant c_SQLite3::t_query(const String& sql) {
  SYNC_VM_REGS_SCOPED();
  validate();
  if (!sql.empty()) {
    Variant stmt = t_prepare(sql);
    if (!same(stmt, false)) {
      return stmt.toObject().getTyped<c_SQLite3Stmt>()->t_execute();
    }
  }
  return false;
}

Variant c_SQLite3::t_querysingle(const String& sql, bool entire_row /* = false */) {
  SYNC_VM_REGS_SCOPED();
  validate();
  if (!sql.empty()) {
    Variant stmt = t_prepare(sql);
    if (!same(stmt, false)) {
      sqlite3_stmt *pstmt =
        stmt.toObject().getTyped<c_SQLite3Stmt>()->m_raw_stmt;
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
          return Array::Create();
        } else {
          return uninit_null();
        }
      default:
        raise_warning("Unable to execute statement: %s",
                      sqlite3_errmsg(m_raw_db));
      }
    }
  }
  return false;
}

bool c_SQLite3::t_createfunction(const String& name, CVarRef callback,
                                 int64_t argcount /* = -1 */) {
  validate();
  if (name.empty()) {
    return false;
  }
  if (!f_is_callable(callback)) {
    raise_warning("Not a valid callback function %s",
                  callback.toString().data());
    return false;
  }

  UserDefinedFuncPtr udf(new UserDefinedFunc());
  if (sqlite3_create_function(m_raw_db, name.data(), argcount, SQLITE_UTF8,
                              udf.get(), php_sqlite3_callback_func,
                              NULL, NULL) == SQLITE_OK) {
    udf->func = callback;
    udf->argc = argcount;
    m_udfs.push_back(udf);
    return true;
  }
  return false;
}

bool c_SQLite3::t_createaggregate(const String& name, CVarRef step, CVarRef final,
                                  int64_t argcount /* = -1 */) {
  validate();
  if (name.empty()) {
    return false;
  }
  if (!f_is_callable(step)) {
    raise_warning("Not a valid callback function %s",
                  step.toString().data());
    return false;
  }
  if (!f_is_callable(final)) {
    raise_warning("Not a valid callback function %s",
                  final.toString().data());
    return false;
  }

  UserDefinedFuncPtr udf(new UserDefinedFunc());
  if (sqlite3_create_function(m_raw_db, name.data(), argcount, SQLITE_UTF8,
                              udf.get(), NULL,
                              php_sqlite3_callback_step,
                              php_sqlite3_callback_final) == SQLITE_OK) {
    udf->step = step;
    udf->fini = final;
    udf->argc = argcount;
    m_udfs.push_back(udf);
    return true;
  }
  return false;
}

bool c_SQLite3::t_openblob(const String& table, const String& column, int64_t rowid,
                           const String& dbname /* = null_string */) {
  throw NotSupportedException(__func__, "sqlite3 stream");
}

///////////////////////////////////////////////////////////////////////////////

c_SQLite3Stmt::c_SQLite3Stmt(Class* cb) :
    ExtObjectData(cb), m_raw_stmt(NULL) {
}

c_SQLite3Stmt::~c_SQLite3Stmt() {
  if (m_raw_stmt) {
    sqlite3_finalize(m_raw_stmt);
  }
}

void c_SQLite3Stmt::t___construct(CObjRef dbobject, const String& statement) {
  if (!statement.empty()) {
    c_SQLite3 *db = dbobject.getTyped<c_SQLite3>();
    db->validate();
    m_db = db;

    int errcode = sqlite3_prepare_v2(db->m_raw_db, statement.data(),
                                     statement.size(), &m_raw_stmt, NULL);
    if (errcode != SQLITE_OK) {
      raise_warning("Unable to prepare statement: %d, %s",
                    errcode, sqlite3_errmsg(db->m_raw_db));
    }
  }
}

void c_SQLite3Stmt::validate() const {
  if (!m_raw_stmt) {
    throw Exception("SQLite3Stmt object was not initialized");
  }
}

int64_t c_SQLite3Stmt::t_paramcount() {
  validate();
  return sqlite3_bind_parameter_count(m_raw_stmt);
}

bool c_SQLite3Stmt::t_close() {
  if (m_raw_stmt) {
    sqlite3_finalize(m_raw_stmt);
    m_raw_stmt = NULL;
  }
  return true;
}

bool c_SQLite3Stmt::t_reset() {
  validate();
  if (sqlite3_reset(m_raw_stmt) != SQLITE_OK) {
    raise_warning("Unable to reset statement: %s",
                  sqlite3_errmsg(sqlite3_db_handle(m_raw_stmt)));
    return false;
  }
  return true;
}

bool c_SQLite3Stmt::t_clear() {
  validate();
  if (sqlite3_clear_bindings(m_raw_stmt) != SQLITE_OK) {
    raise_warning("Unable to clear statement: %s",
                  sqlite3_errmsg(sqlite3_db_handle(m_raw_stmt)));
    return false;
  }
  m_bound_params.clear();
  return true;
}

bool c_SQLite3Stmt::t_bindparam(CVarRef name, VRefParam parameter,
                                int64_t type /* = k_SQLITE3_TEXT */) {
  BoundParamPtr param(new BoundParam());
  param->type = type;
  param->value.assignRef(parameter);

  if (name.isString()) {
    String sname = name.toString();
    if (sname.charAt(0) != ':') {
      /* We need a : prefix to resolve a name to a parameter number */
      sname = String(":") + sname;
    }
    param->index = sqlite3_bind_parameter_index(m_raw_stmt, sname.data());
  } else {
    param->index = name.toInt64();
  }
  if (param->index < 1) {
    return false;
  }

  m_bound_params.push_back(param);
  return true;
}

bool c_SQLite3Stmt::t_bindvalue(CVarRef name, CVarRef parameter,
                                int64_t type /* = k_SQLITE3_TEXT */) {
  Variant v = parameter;
  return t_bindparam(name, v, type);
}

Variant c_SQLite3Stmt::t_execute() {
  SYNC_VM_REGS_SCOPED();
  validate();

  for (unsigned int i = 0; i < m_bound_params.size(); i++) {
    BoundParam &p = *m_bound_params[i];
    if (p.value.isNull()) {
      sqlite3_bind_null(m_raw_stmt, p.index);
      continue;
    }

    switch (p.type) {
    case SQLITE_INTEGER:
      sqlite3_bind_int(m_raw_stmt, p.index, p.value.toInt64());
      break;
    case SQLITE_FLOAT:
      sqlite3_bind_double(m_raw_stmt, p.index, p.value.toDouble());
      break;
    case SQLITE_BLOB:
      {
        String sblob;
        if (p.value.isResource()) {
          Variant blob = f_stream_get_contents(p.value.toResource());
          if (same(blob, false)) {
            raise_warning("Unable to read stream for parameter %d",
                          p.index);
            return false;
          }
          sblob = blob.toString();
        } else {
          sblob = p.value.toString();
        }
        sqlite3_bind_blob(m_raw_stmt, p.index, sblob.data(), sblob.size(),
                          SQLITE_TRANSIENT);
        break;
      }
    case SQLITE3_TEXT:
      {
        String stext = p.value.toString();
        sqlite3_bind_text(m_raw_stmt, p.index, stext.data(), stext.size(),
                          SQLITE_STATIC);
        break;
      }
    case SQLITE_NULL:
      sqlite3_bind_null(m_raw_stmt, p.index);
      break;
    default:
      raise_warning("Unknown parameter type: %d for parameter %d",
                    p.type, p.index);
      return false;
    }
  }

  switch (sqlite3_step(m_raw_stmt)) {
  case SQLITE_ROW: /* Valid Row */
  case SQLITE_DONE: /* Valid but no results */
    {
      sqlite3_reset(m_raw_stmt);
      c_SQLite3Result *result = NEWOBJ(c_SQLite3Result)();
      result->m_stmt = p_SQLite3Stmt(this);
      return Object(result);
    }
  case SQLITE_ERROR:
    sqlite3_reset(m_raw_stmt);
    // fall through
  default:
    raise_warning("Unable to execute statement: %s",
                  sqlite3_errmsg(sqlite3_db_handle(m_raw_stmt)));
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

c_SQLite3Result::c_SQLite3Result(Class* cb) :
    ExtObjectData(cb) {
}

c_SQLite3Result::~c_SQLite3Result() {
}

void c_SQLite3Result::t___construct() {
}

void c_SQLite3Result::validate() const {
  if (!m_stmt.get()) {
    throw Exception("SQLite3Result object was not initialized");
  }
  m_stmt->validate();
}

int64_t c_SQLite3Result::t_numcolumns() {
  validate();
  return sqlite3_column_count(m_stmt->m_raw_stmt);
}

String c_SQLite3Result::t_columnname(int64_t column) {
  validate();
  return String((char*)sqlite3_column_name(m_stmt->m_raw_stmt, column),
                CopyString);
}

int64_t c_SQLite3Result::t_columntype(int64_t column) {
  validate();
  return sqlite3_column_type(m_stmt->m_raw_stmt, column);
}

Variant c_SQLite3Result::t_fetcharray(int64_t mode /* = k_SQLITE3_BOTH */) {
  SYNC_VM_REGS_SCOPED();
  validate();

  switch (sqlite3_step(m_stmt->m_raw_stmt)) {
  case SQLITE_ROW:
    if (mode & PHP_SQLITE3_BOTH) {
      Array ret = Array::Create();
      for (int i = 0; i < sqlite3_data_count(m_stmt->m_raw_stmt); i++) {
        Variant data = get_column_value(m_stmt->m_raw_stmt, i);
        if (mode & PHP_SQLITE3_NUM) {
          ret.set(i, data);
        }
        if (mode & PHP_SQLITE3_ASSOC) {
          ret.set(t_columnname(i), data);
        }
      }
      return ret;
    }
    break;
  case SQLITE_DONE:
    return false;
  default:
    raise_warning("Unable to execute statement: %s",
                  sqlite3_errmsg(sqlite3_db_handle(m_stmt->m_raw_stmt)));
  }
  return uninit_null();
}

bool c_SQLite3Result::t_reset() {
  validate();
  return sqlite3_reset(m_stmt->m_raw_stmt) == SQLITE_OK;
}

bool c_SQLite3Result::t_finalize() {
  validate();
  m_stmt.reset();
  return true;
}

///////////////////////////////////////////////////////////////////////////////
}
