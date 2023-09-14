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

#include "hphp/runtime/ext/pdo/pdo_driver.h"

#include "hphp/util/hphp-config.h"
#ifdef ENABLE_EXTENSION_PDO_SQLITE
#include "hphp/runtime/ext/pdo_sqlite/pdo_sqlite.h"
#endif
#ifdef ENABLE_EXTENSION_PDO_MYSQL
#include "hphp/runtime/ext/pdo_mysql/pdo_mysql.h"
#endif
#include "hphp/runtime/ext/std/ext_std_variable.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/zend-string.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// This needs to get created first.
PDODriverMap PDODriver::s_drivers;

// We will have to list them all here for proper static initialization.
#ifdef ENABLE_EXTENSION_PDO_SQLITE
static PDOSqlite s_sqlite_driver;
#endif
#ifdef ENABLE_EXTENSION_PDO_MYSQL
static PDOMySql s_mysql_driver;
#endif

const StaticString s_general_error_code("HY000");

PDODriver::PDODriver(const char *name) : m_name(name) {
  s_drivers[name] = this;
}

req::ptr<PDOResource> PDODriver::createResource(const String& datasource,
                                                const String& username,
                                                const String& password,
                                                const Array& options) {
  auto const rsrc = createResourceImpl();
  auto const& conn = rsrc->conn();

  conn->data_source = datasource.toCppString();
  conn->username = username.toCppString();
  conn->password = password.toCppString();

  if (options.exists(PDO_ATTR_AUTOCOMMIT)) {
    conn->auto_commit = options[PDO_ATTR_AUTOCOMMIT].toInt64();
  } else {
    conn->auto_commit = 1;
  }

  if (!conn->create(options)) {
    Array err;
    bool hasError = conn->fetchErr(nullptr, err);

    if (hasError && !err.empty()) {
      throw_pdo_exception(uninit_null(),
                          "[%" PRId64 "]: %s",
                          err[0].toInt64(), err[1].toString().data());
    }
    return nullptr;
  }
  return rsrc;
}

///////////////////////////////////////////////////////////////////////////////
// PDOConnection

bool PDOConnection::support(SupportedMethod /*method*/) {
  return false;
}

int PDOConnection::parseDataSource(const char *data_source,
                                   int data_source_len,
                                   struct pdo_data_src_parser *parsed,
                                   int nparams,
                                   folly::StringPiece separators/* = ";" */) {
  int i, j;
  int valstart = -1;
  int semi = -1;
  int optstart = 0;
  int nlen;
  int n_matches = 0;

  char flags[256];
  string_charmask(separators.data(), separators.size(), flags);

  // Can always end with \0
  flags[0] = 1;

  i = 0;
  while (i < data_source_len) {
    /* looking for NAME= */

    if (data_source[i] == '\0') {
      break;
    }

    if (data_source[i] != '=') {
      ++i;
      continue;
    }

    valstart = ++i;

    /* now we're looking for VALUE<separator> or just VALUE<NUL> */
    semi = -1;
    while (i < data_source_len) {
      if (flags[(unsigned char)data_source[i]]) {
        semi = i++;
        break;
      }
      ++i;
    }

    if (semi == -1) {
      semi = i;
    }

    /* find the entry in the array */
    nlen = valstart - optstart - 1;
    for (j = 0; j < nparams; j++) {
      if (0 == strncmp(data_source + optstart, parsed[j].optname, nlen) &&
          parsed[j].optname[nlen] == '\0') {
        /* got a match */
        if (parsed[j].freeme) {
          free(parsed[j].optval);
        }
        parsed[j].optval = strndup(data_source + valstart, semi - valstart);
        parsed[j].freeme = 1;
        ++n_matches;
        break;
      }
    }

    while (i < data_source_len && isspace(data_source[i])) {
      i++;
    }

    optstart = i;
  }

  return n_matches;
}

bool PDOConnection::closer() {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return false;
}

bool PDOConnection::preparer(const String& /*sql*/, sp_PDOStatement* /*stmt*/,
                             const Variant& /*options*/) {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return false;
}

int64_t PDOConnection::doer(const String& /*sql*/) {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return 0;
}

bool PDOConnection::quoter(const String& /*input*/, String& /*quoted*/,
                           PDOParamType /*paramtype*/) {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return false;
}

bool PDOConnection::begin() {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return false;
}

bool PDOConnection::commit() {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return false;
}

bool PDOConnection::rollback() {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return false;
}

bool PDOConnection::setAttribute(int64_t /*attr*/, const Variant& /*value*/) {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return false;
}

String PDOConnection::lastId(const char* /*name*/) {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return String();
}

bool PDOConnection::fetchErr(PDOStatement* /*stmt*/, Array& /*info*/) {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return false;
}

int PDOConnection::getAttribute(int64_t /*attr*/, Variant& /*value*/) {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return -1;
}

bool PDOConnection::checkLiveness() {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// PDOResource

void PDOResource::sweep() {
  def_stmt_ctor_args.releaseForSweep();
  this->~PDOResource();
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
  : paramno(0)
  , param_type(PDO_PARAM_NULL)
  , driver_ext_data(nullptr)
{}

PDOBoundParam::~PDOBoundParam() {
  sweep();
}

void PDOBoundParam::sweep() {
  /* tell the driver that it is going away */
  if (stmt && stmt->support(PDOStatement::MethodParamHook)) {
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
}

PDOStatement::~PDOStatement() {
  if (dbh && dbh->query_stmt == this) {
    dbh->query_stmt = NULL;
  }
}

void PDOStatement::sweep() {
  // nothing, but kids can overwrite
}

bool PDOStatement::support(SupportedMethod /*method*/) {
  return false;
}

bool PDOStatement::executer() {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return false;
}

bool PDOStatement::fetcher(PDOFetchOrientation /*ori*/, long /*offset*/) {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return false;
}

bool PDOStatement::describer(int /*colno*/) {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return false;
}

bool PDOStatement::getColumn(int /*colno*/, Variant& /*value*/) {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return false;
}

bool PDOStatement::paramHook(PDOBoundParam* /*param*/,
                             PDOParamEvent /*event_type*/) {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return false;
}

bool PDOStatement::setAttribute(int64_t /*attr*/, const Variant& /*value*/) {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return false;
}

int PDOStatement::getAttribute(int64_t /*attr*/, Variant& /*value*/) {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return -1;
}

bool PDOStatement::getColumnMeta(int64_t /*colno*/, Array& /*return_value*/) {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return false;
}

bool PDOStatement::nextRowset() {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return false;
}

bool PDOStatement::cursorCloser() {
  throw_pdo_exception(uninit_null(),
                      "This driver doesn't support %s", __func__);
  return false;
}

///////////////////////////////////////////////////////////////////////////////
}
