/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/ext/pdo_driver.h>
#include <runtime/ext/pdo_sqlite.h>
#include <runtime/ext/pdo_mysql.h>
#include <runtime/ext/ext_variable.h>

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// This needs to get created first.
PDODriverMap PDODriver::s_drivers;

// We will have to list them all here for proper static initialization.
static PDOSqlite s_sqlite_driver;
static PDOMySql s_mysql_driver;

PDODriver::PDODriver(const char *name) : m_name(name) {
  s_drivers[name] = this;
}

PDOConnection *PDODriver::createConnection(CStrRef datasource,
                                           CStrRef username,
                                           CStrRef password, CArrRef options) {
  PDOConnection *conn = createConnectionObject();
  conn->data_source = string(datasource.data(), datasource.size());
  conn->username = string(username.data(), username.size());
  conn->password = string(password.data(), password.size());
  if (!conn->create(options)) {
    delete conn;
    return NULL;
  }
  return conn;
}

///////////////////////////////////////////////////////////////////////////////
// PDOConnection

const char *PDOConnection::PersistentKey = "pdo_connection";

PDOConnection::PDOConnection()
    : is_persistent(0), auto_commit(0), is_closed(0), alloc_own_columns(0),
      in_txn(0), max_escaped_char_length(0), oracle_nulls(0), stringify(0),
      _reserved_flags(0), error_mode(PDO_ERRMODE_SILENT),
      native_case(PDO_CASE_NATURAL), desired_case(PDO_CASE_NATURAL),
      driver(NULL), default_fetch_type(PDO_FETCH_USE_DEFAULT) {
  memset(error_code, 0, sizeof(error_code));
}

PDOConnection::~PDOConnection() {
}

void PDOConnection::persistentSave() {
  String serialized = f_serialize(def_stmt_ctor_args);
  serialized_def_stmt_ctor_args = string(serialized.data(), serialized.size());
  def_stmt_ctor_args.reset();
}

void PDOConnection::persistentRestore() {
  if (!serialized_def_stmt_ctor_args.empty()) {
    def_stmt_ctor_args = f_unserialize(serialized_def_stmt_ctor_args);
  }
}

bool PDOConnection::support(SupportedMethod method) {
  return false;
}

bool PDOConnection::closer() {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return false;
}

bool PDOConnection::preparer(CStrRef sql, sp_PDOStatement *stmt,
                             CVarRef options) {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return false;
}

int64 PDOConnection::doer(CStrRef sql) {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return 0;
}

bool PDOConnection::quoter(CStrRef input, String &quoted,
                           PDOParamType paramtype) {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return false;
}

bool PDOConnection::begin() {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return false;
}

bool PDOConnection::commit() {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return false;
}

bool PDOConnection::rollback() {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return false;
}

bool PDOConnection::setAttribute(int64 attr, CVarRef value) {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return false;
}

String PDOConnection::lastId(const char *name) {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return String();
}

bool PDOConnection::fetchErr(PDOStatement *stmt, Array &info) {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return false;
}

int PDOConnection::getAttribute(int64 attr, Variant &value) {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return -1;
}

bool PDOConnection::checkLiveness() {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return false;
}

void PDOConnection::persistentShutdown() {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
}

///////////////////////////////////////////////////////////////////////////////
// PDOColumn

PDOColumn::PDOColumn()
    : maxlen(0), param_type(PDO_PARAM_NULL), precision(0) {
}

PDOColumn::~PDOColumn() {
}

///////////////////////////////////////////////////////////////////////////////
// PDOBoundParam

PDOBoundParam::PDOBoundParam()
    : paramno(0), max_value_len(0), param_type(PDO_PARAM_NULL),
      is_param(false), driver_data(NULL) {
}

PDOBoundParam::~PDOBoundParam() {
  /* tell the driver that it is going away */
  if (stmt.get() && stmt->support(PDOStatement::MethodParamHook)) {
    stmt->paramHook(this, PDO_PARAM_EVT_FREE);
  }
}

///////////////////////////////////////////////////////////////////////////////
// PDOStatement

PDOStatement::PDOStatement()
    : executed(0), supports_placeholders(0), _reserved(0), column_count(0),
      row_count(0), default_fetch_type(PDO_FETCH_USE_DEFAULT),
      named_rewrite_template(NULL) {
  memset(error_code, 0, sizeof(error_code));
  fetch.column = 0;
  fetch.constructor = NULL;
}

PDOStatement::~PDOStatement() {
  if (dbh.get() && dbh->query_stmt == this) {
    dbh->query_stmt = NULL;
  }
}

bool PDOStatement::support(SupportedMethod method) {
  return false;
}

bool PDOStatement::executer() {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return false;
}

bool PDOStatement::fetcher(PDOFetchOrientation ori, long offset) {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return false;
}

bool PDOStatement::describer(int colno) {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return false;
}

bool PDOStatement::getColumn(int colno, Variant &value) {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return false;
}

bool PDOStatement::paramHook(PDOBoundParam *param, PDOParamEvent event_type) {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return false;
}

bool PDOStatement::setAttribute(int64 attr, CVarRef value) {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return false;
}

int PDOStatement::getAttribute(int64 attr, Variant &value) {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return -1;
}

bool PDOStatement::getColumnMeta(int64 colno, Array &return_value) {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return false;
}

bool PDOStatement::nextRowset() {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return false;
}

bool PDOStatement::cursorCloser() {
  throw_pdo_exception(null, null, "This driver doesn't support %s", __func__);
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}

