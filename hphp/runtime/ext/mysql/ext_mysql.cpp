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
#include "hphp/runtime/ext/mysql/ext_mysql.h"

#include <folly/ScopeGuard.h>
#include <folly/String.h>
#include <folly/portability/Sockets.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/ext/mysql/mysql_common.h"
#include "hphp/runtime/ext/mysql/mysql_stats.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

using std::string;

///////////////////////////////////////////////////////////////////////////////

static Variant
HHVM_FUNCTION(mysql_connect, const String& server, const String& username,
              const String& password, bool /*new_link*/, int64_t client_flags,
              int64_t connect_timeout_ms, int64_t query_timeout_ms,
              const Array& conn_attrs) {
  return Variant(php_mysql_do_connect(
      server,
      username,
      password,
      "",
      client_flags,
      false,
      connect_timeout_ms,
      query_timeout_ms,
      &conn_attrs));
}

static Variant HHVM_FUNCTION(
    mysql_connect_with_ssl,
    const String& server,
    const String& username,
    const String& password,
    const String& database,
    int64_t client_flags,
    int64_t connect_timeout_ms,
    int64_t query_timeout_ms,
    const Variant& sslContextProvider, /* = null */
    const Array& conn_attrs) {
  return Variant(php_mysql_do_connect_with_ssl(
      server,
      username,
      password,
      database,
      client_flags,
      connect_timeout_ms,
      query_timeout_ms,
      &conn_attrs,
      sslContextProvider));
}

static Variant HHVM_FUNCTION(mysql_connect_with_db, const String& server,
                             const String& username, const String& password,
                             const String& database, bool /*new_link*/,
                             int64_t client_flags, int64_t connect_timeout_ms,
                             int64_t query_timeout_ms, const Array& conn_attrs) {
  return Variant(php_mysql_do_connect(
      server,
      username,
      password,
      database,
      client_flags,
      false,
      connect_timeout_ms,
      query_timeout_ms,
      &conn_attrs));
}

static Variant HHVM_FUNCTION(mysql_pconnect,
  const String& server,
  const String& username,
  const String& password,
  int64_t client_flags,
  int64_t connect_timeout_ms,
  int64_t query_timeout_ms,
  const Array& conn_attrs) {
  return php_mysql_do_connect(
    server,
    username,
    password,
    "",
    client_flags,
    true,
    connect_timeout_ms,
    query_timeout_ms,
    &conn_attrs
  );
}

static Variant HHVM_FUNCTION(mysql_pconnect_with_db,
  const String& server,
  const String& username,
  const String& password,
  const String& database,
  int64_t client_flags,
  int64_t connect_timeout_ms,
  int64_t query_timeout_ms,
  const Array& conn_attrs) {
  return php_mysql_do_connect(
    server,
    username,
    password,
    database,
    client_flags,
    true,
    connect_timeout_ms,
    query_timeout_ms,
    &conn_attrs
  );
}

static bool HHVM_FUNCTION(mysql_set_timeout, int64_t query_timeout_ms /* = -1 */,
                          const Variant& /*link_identifier*/ /* = null */) {
  MySQL::SetDefaultReadTimeout(query_timeout_ms);
  return true;
}

static String HHVM_FUNCTION(mysql_escape_string,
                            const String& unescaped_string) {
  String new_str((size_t)unescaped_string.size() * 2 + 1, ReserveString);
  unsigned long new_len = mysql_escape_string(new_str.mutableData(),
                                    unescaped_string.data(),
                                    unescaped_string.size());
  new_str.shrink(new_len);
  return new_str;
}

static Variant HHVM_FUNCTION(mysql_real_escape_string,
                             const String& unescaped_string,
                             const Variant& link_identifier /* = null */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (conn) {
    String new_str((size_t)unescaped_string.size() * 2 + 1, ReserveString);
    unsigned long new_len = mysql_real_escape_string(conn,
                                      new_str.mutableData(),
                                      unescaped_string.data(),
                                      unescaped_string.size());

    new_str.shrink(new_len);
    return new_str;
  }
  return false;
}

String HHVM_FUNCTION(mysql_get_client_info) {
  return String(mysql_get_client_info(), CopyString);
}

static Variant HHVM_FUNCTION(mysql_set_charset, const String& charset,
                   const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return init_null();
  return !mysql_set_character_set(conn, charset.data());
}

static Variant HHVM_FUNCTION(mysql_ping,
                   const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return init_null();
  return !mysql_ping(conn);
}
static Variant HHVM_FUNCTION(mysql_client_encoding,
                      const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return String(mysql_character_set_name(conn), CopyString);
}
static bool HHVM_FUNCTION(mysql_close,
                   const Variant& link_identifier /* = uninit_null() */) {
  return MySQL::CloseConn(link_identifier);
}

Variant HHVM_FUNCTION(mysql_errno,
                      const Variant& link_identifier /* = null */) {
  auto mySQL = MySQL::Get(link_identifier);
  if (!mySQL) {
    raise_warning("supplied argument is not a valid MySQL-Link resource");
    return false;
  }
  MYSQL *conn = mySQL->get();
  if (conn) {
    return (int64_t)mysql_errno(conn);
  }
  if (mySQL->m_last_error_set) {
    return (int64_t)mySQL->m_last_errno;
  }
  return false;
}

Variant HHVM_FUNCTION(mysql_error,
                      const Variant& link_identifier /* = null */) {
  auto mySQL = MySQL::Get(link_identifier);
  if (!mySQL) {
    raise_warning("supplied argument is not a valid MySQL-Link resource");
    return false;
  }
  MYSQL *conn = mySQL->get();
  if (conn) {
    return String(mysql_error(conn), CopyString);
  }
  if (mySQL->m_last_error_set) {
    return String(mySQL->m_last_error);
  }
  return false;
}

Variant HHVM_FUNCTION(mysql_warning_count,
                      const Variant& link_identifier /* = null */) {
  auto mySQL = MySQL::Get(link_identifier);
  if (!mySQL) {
    raise_warning("supplied argument is not a valid MySQL-Link resource");
    return false;
  }
  MYSQL *conn = mySQL->get();
  if (conn) {
    return (int64_t)mysql_warning_count(conn);
  }
  return false;
}

Variant HHVM_FUNCTION(mysql_get_host_info,
                      const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return String(mysql_get_host_info(conn), CopyString);
}
Variant HHVM_FUNCTION(mysql_get_proto_info,
                      const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return (int64_t)mysql_get_proto_info(conn);
}
Variant HHVM_FUNCTION(mysql_get_server_info,
                      const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return String(mysql_get_server_info(conn), CopyString);
}
Variant HHVM_FUNCTION(mysql_info,
                      const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return String(mysql_info(conn), CopyString);
}
Variant HHVM_FUNCTION(mysql_insert_id,
                      const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return static_cast<int64_t>(mysql_insert_id(conn));
}
static Variant HHVM_FUNCTION(mysql_stat,
                      const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return String(mysql_stat(conn), CopyString);
}
Variant HHVM_FUNCTION(mysql_thread_id,
                      const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return (int64_t)mysql_thread_id(conn);
}

static bool HHVM_FUNCTION(mysql_select_db, const String& db,
                   const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return mysql_select_db(conn, db.data()) == 0;
}

Variant HHVM_FUNCTION(mysql_affected_rows,
                      const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return static_cast<int64_t>(mysql_affected_rows(conn));
}

///////////////////////////////////////////////////////////////////////////////
// query functions

static Variant HHVM_FUNCTION(mysql_query, const String& query,
                      const Variant& link_identifier /* = null */) {
  return php_mysql_do_query_and_get_result(query, link_identifier, true);
}

static Variant HHVM_FUNCTION(mysql_multi_query, const String& query,
                      const Variant& link_identifier /* = null */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (conn == nullptr) {
    return false;
  }
  auto mySQL = MySQL::Get(link_identifier);
  if (!mySQL) {
    raise_warning("supplied argument is not a valid MySQL-Link resource");
    return false;
  }

  if (!mySQL->m_multi_query &&
      !mysql_set_server_option(conn, MYSQL_OPTION_MULTI_STATEMENTS_ON)) {
    mySQL->m_multi_query = true;
  }

  if (mysql_real_query(conn, query.data(), query.size())) {
#ifdef HHVM_MYSQL_TRACE_MODE
    raise_notice("runtime/ext_mysql: failed executing [%s] [%s]",
                 query.data(), mysql_error(conn));
#endif
    // turning this off clears the errors
    if (!mysql_set_server_option(conn, MYSQL_OPTION_MULTI_STATEMENTS_OFF)) {
      mySQL->m_multi_query = false;
    }
    return false;
  }
  return true;
}

static int64_t HHVM_FUNCTION(mysql_next_result,
                  const Variant& link_identifier /* = null */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (conn == nullptr) {
    return 2006 /* CR_SERVER_GONE_ERROR */;
  }
  if (!mysql_more_results(conn)) {
    raise_strict_warning("There is no next result set. "
      "Please, call mysql_more_results() to check "
      "whether to call this function/method");
  }
  return mysql_next_result(conn);
}

static bool HHVM_FUNCTION(mysql_more_results,
                   const Variant& link_identifier /* = null */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (conn == nullptr) {
    return false;
  }
  return mysql_more_results(conn);
}

static Variant HHVM_FUNCTION(mysql_fetch_result,
                      const Variant& link_identifier /* = null */) {
    MYSQL *conn = MySQL::GetConn(link_identifier);
    if (conn == nullptr) {
      return false;
    }
    MYSQL_RES *mysql_result;

    mysql_result = mysql_store_result(conn);

    if (!mysql_result) {
      if (mysql_field_count(conn) > 0) {
        raise_warning("Unable to save result set");
        return false;
      }
      return true;
    }

    return Variant(req::make<MySQLResult>(mysql_result));
}

static Variant HHVM_FUNCTION(mysql_unbuffered_query, const String& query,
                      const Variant& link_identifier /* = null */) {
  return php_mysql_do_query_and_get_result(query, link_identifier, false);
}

static Variant HHVM_FUNCTION(mysql_list_dbs,
                      const Variant& link_identifier /* = null */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  MYSQL_RES *res = mysql_list_dbs(conn, nullptr);
  if (!res) {
    raise_warning("Unable to save MySQL query result");
    return false;
  }
  return Variant(req::make<MySQLResult>(res));
}

static Variant HHVM_FUNCTION(mysql_list_tables, const String& database,
                      const Variant& link_identifier /* = null */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  if (mysql_select_db(conn, database.data())) {
    return false;
  }
  MYSQL_RES *res = mysql_list_tables(conn, nullptr);
  if (!res) {
    raise_warning("Unable to save MySQL query result");
    return false;
  }
  return Variant(req::make<MySQLResult>(res));
}

static Variant HHVM_FUNCTION(mysql_list_processes,
                      const Variant& link_identifier /* = null */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  MYSQL_RES *res = mysql_list_processes(conn);
  if (!res) {
    raise_warning("Unable to save MySQL query result");
    return false;
  }
  return Variant(req::make<MySQLResult>(res));
}

///////////////////////////////////////////////////////////////////////////////
// row operations

static bool HHVM_FUNCTION(mysql_data_seek, const OptResource& result, int64_t row) {
  auto res = php_mysql_extract_result(result);
  if (res == nullptr) return false;

  return res->seekRow(row);
}

static Variant HHVM_FUNCTION(mysql_fetch_array, const OptResource& result,
                                         int64_t result_type /* = 3 */) {
  return php_mysql_fetch_hash(result, result_type);
}

static Variant HHVM_FUNCTION(mysql_fetch_object,
                      const Variant& var_result,
                      const String& class_name /* = "stdClass" */,
                      const Variant& params /* = null */) {

  OptResource result = var_result.isResource() ? var_result.toResource()
                                            : null_resource;
  Variant properties = php_mysql_fetch_hash(result, PHP_MYSQL_ASSOC);
  if (!same(properties, false)) {
    Object obj;

    const auto paramsArray = params.isArray()
      ? params.asCArrRef()
      : Array();

    // We need to create an object without initialization (constructor call),
    // and set the fetched fields as dynamic properties on the object prior
    // calling the constructor.
    obj = create_object_only(class_name);

    // Set the fields.
    obj->o_setArray(properties.toArray());

    // And finally initialize the object by calling the constructor.
    obj = init_object(class_name, paramsArray, obj.get());

    return obj;
  }

  return false;
}

Variant HHVM_FUNCTION(mysql_fetch_lengths, const OptResource& result) {
  auto res = php_mysql_extract_result(result);
  if (res == nullptr) return false;

  if (res->isLocalized()) {
    if (!res->isRowReady()) return false;

    Array ret;
    for (int i = 0; i < res->getFieldCount(); i++) {
      MySQLFieldInfo *info = res->getFieldInfo(i);
      if (info->type == MYSQL_TYPE_YEAR) {
        // special case for years, because of leading zeros
        ret.set(i, info->length);
      } else {
        // convert fields back to Strings to get lengths
        ret.set(i, res->getField(i).toString().length());
      }
    }
    return ret;
  }

  MYSQL_RES *mysql_result = res->get();
  unsigned long *lengths = mysql_fetch_lengths(mysql_result);
  if (!lengths) {
    return false;
  }

  Array ret;
  int num_fields = mysql_num_fields(mysql_result);
  for (int i = 0; i < num_fields; i++) {
    ret.set(i, (int)lengths[i]);
  }
  return ret;
}

static Variant HHVM_FUNCTION(mysql_result, const OptResource& result, int64_t row,
                                    const Variant& field /* = 0 */) {
  auto res = php_mysql_extract_result(result);
  if (res == nullptr) return false;

  MYSQL_RES *mysql_result = nullptr;
  MYSQL_ROW sql_row = nullptr;
  unsigned long *sql_row_lengths = nullptr;

  if (res->isLocalized()) {
    if (!res->seekRow(row)) return false;
    if (!res->fetchRow()) return false;
  } else {
    mysql_result = res->get();
    if (row < 0 || row >= (int)mysql_num_rows(mysql_result)) {
      raise_warning("Unable to jump to row %ld on MySQL result index %d",
                      row, result->getId());
      return false;
    }
    mysql_data_seek(mysql_result, row);

    sql_row = mysql_fetch_row(mysql_result);
    if (!sql_row) {
      return false;
    }
    sql_row_lengths = mysql_fetch_lengths(mysql_result);
    if (!sql_row_lengths) {
      return false;
    }
  }

  int field_offset = 0;
  if (!field.isNull()) {
    if (field.isString()) {
      String sfield = field.toString();
      const char *tmp = strchr(sfield.data(), '.');
      String table_name, field_name;
      if (tmp) {
        int pos = tmp - sfield.data();
        table_name = sfield.substr(0, pos);
        field_name = sfield.substr(pos + 1);
      } else {
        field_name = sfield;
      }

      int i = 0;
      bool found = false;
      res->seekField(0);
      while (i < res->getFieldCount()) {
        MySQLFieldInfo *info = res->getFieldInfo(i);
        if ((table_name.empty() || table_name.same(info->table)) &&
            field_name.same(info->name)) {
          field_offset = i;
          found = true;
          break;
        }
        i++;
      }
      if (!found) { /* no match found */
        raise_warning("%s%s%s not found in MySQL result index %d",
                        table_name.data(), (table_name.empty() ? "" : "."),
                        field_name.data(), result->getId());
        return false;
      }
    } else {
      field_offset = (int)field.toInt64();
      if (field_offset < 0 ||
          field_offset >= (int)res->getFieldCount()) {
        raise_warning("Bad column offset specified");
        return false;
      }
    }
  }

  if (res->isLocalized()) {
    Variant f = res->getField(field_offset);
    if (!f.isNull()) {
      return f.toString();
    }
  } else {
    if (sql_row[field_offset]) {
      return String(sql_row[field_offset], sql_row_lengths[field_offset],
                    CopyString);
    }
  }
  return init_null();
}

///////////////////////////////////////////////////////////////////////////////
// result functions

Variant HHVM_FUNCTION(mysql_num_fields, const OptResource& result) {
  auto res = php_mysql_extract_result(result);
  if (res) {
    return res->getFieldCount();
  }
  return false;
}

Variant HHVM_FUNCTION(mysql_num_rows, const OptResource& result) {
  auto res = php_mysql_extract_result(result);
  if (res) {
    return res->getRowCount();
  }
  return false;
}

static bool HHVM_FUNCTION(mysql_free_result, const OptResource& result) {
  auto res = php_mysql_extract_result(result);
  if (res) {
    res->close();
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// field info

namespace {

StaticString
  s_name("name"),
  s_table("table"),
  s_def("def"),
  s_max_length("max_length"),
  s_not_null("not_null"),
  s_primary_key("primary_key"),
  s_multiple_key("multiple_key"),
  s_unique_key("unique_key"),
  s_numeric("numeric"),
  s_blob("blob"),
  s_type("type"),
  s_unsigned("unsigned"),
  s_zerofill("zerofill");
}

static Variant HHVM_FUNCTION(mysql_fetch_field, const OptResource& result,
                                         int64_t field /* = -1 */) {
  auto res = php_mysql_extract_result(result);
  if (res == nullptr) return false;

  if (field != -1) {
    if (!res->seekField(field)) return false;
  }
  MySQLFieldInfo *info;
  if (!(info = res->fetchFieldInfo())) return false;

  DictInit props(13);
  props.set(s_name,         info->name);
  props.set(s_table,        info->table);
  props.set(s_def,          info->def);
  props.set(s_max_length,   (int)info->max_length);
  props.set(s_not_null,     IS_NOT_NULL(info->flags)? 1 : 0);
  props.set(s_primary_key,  IS_PRI_KEY(info->flags)? 1 : 0);
  props.set(s_multiple_key, info->flags & MULTIPLE_KEY_FLAG? 1 : 0);
  props.set(s_unique_key,   info->flags & UNIQUE_KEY_FLAG? 1 : 0);
  props.set(s_numeric,      IS_NUM(info->type)? 1 : 0);
  props.set(s_blob,         IS_BLOB(info->flags)? 1 : 0);
  props.set(s_type,         php_mysql_get_field_name(info->type));
  props.set(s_unsigned,     info->flags & UNSIGNED_FLAG? 1 : 0);
  props.set(s_zerofill,     info->flags & ZEROFILL_FLAG? 1 : 0);
  return ObjectData::FromArray(props.create());
}

static bool HHVM_FUNCTION(mysql_field_seek, const OptResource& result, int64_t field) {
  auto res = php_mysql_extract_result(result);
  if (res == nullptr) return false;
  return res->seekField(field);
}

static Variant HHVM_FUNCTION(mysql_field_name, const OptResource& result,
                                               int64_t field) {
  return php_mysql_field_info(result, field, PHP_MYSQL_FIELD_NAME);
}
static Variant HHVM_FUNCTION(mysql_field_table, const OptResource& result,
                                                int64_t field) {
  return php_mysql_field_info(result, field, PHP_MYSQL_FIELD_TABLE);
}
static Variant HHVM_FUNCTION(mysql_field_len, const OptResource& result,
                                              int64_t field) {
  return php_mysql_field_info(result, field, PHP_MYSQL_FIELD_LEN);
}
static Variant HHVM_FUNCTION(mysql_field_type, const OptResource& result,
                                               int64_t field) {
  return php_mysql_field_info(result, field, PHP_MYSQL_FIELD_TYPE);
}
static Variant HHVM_FUNCTION(mysql_field_flags, const OptResource& result,
                                                int64_t field) {
  return php_mysql_field_info(result, field, PHP_MYSQL_FIELD_FLAGS);
}

///////////////////////////////////////////////////////////////////////////////

void mysqlExtension::moduleRegisterNative() {
  HHVM_FE(mysql_connect);
  HHVM_FE(mysql_connect_with_db);
  HHVM_FE(mysql_connect_with_ssl);
  HHVM_FE(mysql_pconnect);
  HHVM_FE(mysql_pconnect_with_db);
  HHVM_FE(mysql_set_timeout);
  HHVM_FE(mysql_escape_string);
  HHVM_FE(mysql_real_escape_string);
  HHVM_FE(mysql_get_client_info);
  HHVM_FE(mysql_set_charset);
  HHVM_FE(mysql_ping);
  HHVM_FE(mysql_client_encoding);
  HHVM_FE(mysql_close);
  HHVM_FE(mysql_errno);
  HHVM_FE(mysql_error);
  HHVM_FE(mysql_warning_count);
  HHVM_FE(mysql_get_host_info);
  HHVM_FE(mysql_get_proto_info);
  HHVM_FE(mysql_get_server_info);
  HHVM_FE(mysql_info);
  HHVM_FE(mysql_insert_id);
  HHVM_FE(mysql_stat);
  HHVM_FE(mysql_thread_id);
  HHVM_FE(mysql_select_db);
  HHVM_FE(mysql_affected_rows);
  HHVM_FE(mysql_query);
  HHVM_FE(mysql_multi_query);
  HHVM_FE(mysql_next_result);
  HHVM_FE(mysql_more_results);
  HHVM_FE(mysql_fetch_result);
  HHVM_FE(mysql_unbuffered_query);
  HHVM_FE(mysql_list_dbs);
  HHVM_FE(mysql_list_tables);
  HHVM_FE(mysql_list_processes);
  HHVM_FE(mysql_data_seek);
  HHVM_FE(mysql_fetch_array);
  HHVM_FE(mysql_fetch_object);
  HHVM_FE(mysql_fetch_lengths);
  HHVM_FE(mysql_result);
  HHVM_FE(mysql_num_fields);
  HHVM_FE(mysql_num_rows);
  HHVM_FE(mysql_free_result);
  HHVM_FE(mysql_fetch_field);
  HHVM_FE(mysql_field_seek);
  HHVM_FE(mysql_field_name);
  HHVM_FE(mysql_field_table);
  HHVM_FE(mysql_field_len);
  HHVM_FE(mysql_field_type);
  HHVM_FE(mysql_field_flags);

  HHVM_RC_INT(MYSQL_ASSOC, PHP_MYSQL_ASSOC);
  HHVM_RC_INT(MYSQL_BOTH, PHP_MYSQL_BOTH);
  HHVM_RC_INT(MYSQL_NUM, PHP_MYSQL_NUM);

  // The following MySQL client errors all come from include/errmsg.h in the MySQL source
  //
  // Declare a new Hack constant: HHVM_RC_INT(HackConstantName, CppDefinition);
  HHVM_RC_INT(MYSQL_CLIENT_CR_UNKNOWN_ERROR, CR_UNKNOWN_ERROR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_SOCKET_CREATE_ERROR, CR_SOCKET_CREATE_ERROR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_CONNECTION_ERROR, CR_CONNECTION_ERROR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_CONN_HOST_ERROR, CR_CONN_HOST_ERROR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_IPSOCK_ERROR, CR_IPSOCK_ERROR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_UNKNOWN_HOST, CR_UNKNOWN_HOST)
  HHVM_RC_INT(MYSQL_CLIENT_CR_SERVER_GONE_ERROR, CR_SERVER_GONE_ERROR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_VERSION_ERROR, CR_VERSION_ERROR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_OUT_OF_MEMORY, CR_OUT_OF_MEMORY)
  HHVM_RC_INT(MYSQL_CLIENT_CR_WRONG_HOST_INFO, CR_WRONG_HOST_INFO)
  HHVM_RC_INT(MYSQL_CLIENT_CR_LOCALHOST_CONNECTION, CR_LOCALHOST_CONNECTION)
  HHVM_RC_INT(MYSQL_CLIENT_CR_TCP_CONNECTION, CR_TCP_CONNECTION)
  HHVM_RC_INT(MYSQL_CLIENT_CR_SERVER_HANDSHAKE_ERR, CR_SERVER_HANDSHAKE_ERR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_SERVER_LOST, CR_SERVER_LOST)
  HHVM_RC_INT(MYSQL_CLIENT_CR_COMMANDS_OUT_OF_SYNC, CR_COMMANDS_OUT_OF_SYNC)
  HHVM_RC_INT(MYSQL_CLIENT_CR_NAMEDPIPE_CONNECTION, CR_NAMEDPIPE_CONNECTION)
  HHVM_RC_INT(MYSQL_CLIENT_CR_NAMEDPIPEWAIT_ERROR, CR_NAMEDPIPEWAIT_ERROR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_NAMEDPIPEOPEN_ERROR, CR_NAMEDPIPEOPEN_ERROR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_NAMEDPIPESETSTATE_ERROR, CR_NAMEDPIPESETSTATE_ERROR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_CANT_READ_CHARSET, CR_CANT_READ_CHARSET)
  HHVM_RC_INT(MYSQL_CLIENT_CR_NET_PACKET_TOO_LARGE, CR_NET_PACKET_TOO_LARGE)
  HHVM_RC_INT(MYSQL_CLIENT_CR_EMBEDDED_CONNECTION, CR_EMBEDDED_CONNECTION)
  HHVM_RC_INT(MYSQL_CLIENT_CR_PROBE_SLAVE_STATUS, CR_PROBE_SLAVE_STATUS)
  HHVM_RC_INT(MYSQL_CLIENT_CR_PROBE_SLAVE_HOSTS, CR_PROBE_SLAVE_HOSTS)
  HHVM_RC_INT(MYSQL_CLIENT_CR_PROBE_SLAVE_CONNECT, CR_PROBE_SLAVE_CONNECT)
  HHVM_RC_INT(MYSQL_CLIENT_CR_PROBE_MASTER_CONNECT, CR_PROBE_MASTER_CONNECT)
  HHVM_RC_INT(MYSQL_CLIENT_CR_SSL_CONNECTION_ERROR, CR_SSL_CONNECTION_ERROR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_MALFORMED_PACKET, CR_MALFORMED_PACKET)
  HHVM_RC_INT(MYSQL_CLIENT_CR_WRONG_LICENSE, CR_WRONG_LICENSE)
  HHVM_RC_INT(MYSQL_CLIENT_CR_NULL_POINTER, CR_NULL_POINTER)
  HHVM_RC_INT(MYSQL_CLIENT_CR_NO_PREPARE_STMT, CR_NO_PREPARE_STMT)
  HHVM_RC_INT(MYSQL_CLIENT_CR_PARAMS_NOT_BOUND, CR_PARAMS_NOT_BOUND)
  HHVM_RC_INT(MYSQL_CLIENT_CR_DATA_TRUNCATED, CR_DATA_TRUNCATED)
  HHVM_RC_INT(MYSQL_CLIENT_CR_NO_PARAMETERS_EXISTS, CR_NO_PARAMETERS_EXISTS)
  HHVM_RC_INT(MYSQL_CLIENT_CR_INVALID_PARAMETER_NO, CR_INVALID_PARAMETER_NO)
  HHVM_RC_INT(MYSQL_CLIENT_CR_INVALID_BUFFER_USE, CR_INVALID_BUFFER_USE)
  HHVM_RC_INT(MYSQL_CLIENT_CR_UNSUPPORTED_PARAM_TYPE, CR_UNSUPPORTED_PARAM_TYPE)
  HHVM_RC_INT(MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECTION, CR_SHARED_MEMORY_CONNECTION)
  HHVM_RC_INT(MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_REQUEST_ERROR, CR_SHARED_MEMORY_CONNECT_REQUEST_ERROR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_ANSWER_ERROR, CR_SHARED_MEMORY_CONNECT_ANSWER_ERROR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_FILE_MAP_ERROR, CR_SHARED_MEMORY_CONNECT_FILE_MAP_ERROR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_MAP_ERROR, CR_SHARED_MEMORY_CONNECT_MAP_ERROR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_SHARED_MEMORY_FILE_MAP_ERROR, CR_SHARED_MEMORY_FILE_MAP_ERROR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_SHARED_MEMORY_MAP_ERROR, CR_SHARED_MEMORY_MAP_ERROR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_SHARED_MEMORY_EVENT_ERROR, CR_SHARED_MEMORY_EVENT_ERROR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_ABANDONED_ERROR, CR_SHARED_MEMORY_CONNECT_ABANDONED_ERROR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_SHARED_MEMORY_CONNECT_SET_ERROR, CR_SHARED_MEMORY_CONNECT_SET_ERROR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_CONN_UNKNOW_PROTOCOL, CR_CONN_UNKNOW_PROTOCOL)
  HHVM_RC_INT(MYSQL_CLIENT_CR_INVALID_CONN_HANDLE, CR_INVALID_CONN_HANDLE)
  HHVM_RC_INT(MYSQL_CLIENT_CR_UNUSED_1, CR_UNUSED_1)
  HHVM_RC_INT(MYSQL_CLIENT_CR_FETCH_CANCELED, CR_FETCH_CANCELED)
  HHVM_RC_INT(MYSQL_CLIENT_CR_NO_DATA, CR_NO_DATA)
  HHVM_RC_INT(MYSQL_CLIENT_CR_NO_STMT_METADATA, CR_NO_STMT_METADATA)
  HHVM_RC_INT(MYSQL_CLIENT_CR_NO_RESULT_SET, CR_NO_RESULT_SET)
  HHVM_RC_INT(MYSQL_CLIENT_CR_NOT_IMPLEMENTED, CR_NOT_IMPLEMENTED)
  HHVM_RC_INT(MYSQL_CLIENT_CR_SERVER_LOST_EXTENDED, CR_SERVER_LOST_EXTENDED)
  HHVM_RC_INT(MYSQL_CLIENT_CR_STMT_CLOSED, CR_STMT_CLOSED)
  HHVM_RC_INT(MYSQL_CLIENT_CR_NEW_STMT_METADATA, CR_NEW_STMT_METADATA)
  HHVM_RC_INT(MYSQL_CLIENT_CR_ALREADY_CONNECTED, CR_ALREADY_CONNECTED)
  HHVM_RC_INT(MYSQL_CLIENT_CR_AUTH_PLUGIN_CANNOT_LOAD, CR_AUTH_PLUGIN_CANNOT_LOAD)
  HHVM_RC_INT(MYSQL_CLIENT_CR_DUPLICATE_CONNECTION_ATTR, CR_DUPLICATE_CONNECTION_ATTR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_AUTH_PLUGIN_ERR, CR_AUTH_PLUGIN_ERR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_INSECURE_API_ERR, CR_INSECURE_API_ERR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_FILE_NAME_TOO_LONG, CR_FILE_NAME_TOO_LONG)
  HHVM_RC_INT(MYSQL_CLIENT_CR_SSL_FIPS_MODE_ERR, CR_SSL_FIPS_MODE_ERR)
  HHVM_RC_INT(MYSQL_CLIENT_CR_COMPRESSION_NOT_SUPPORTED, CR_COMPRESSION_NOT_SUPPORTED)

  // We can't add the following yet as some builds are not using a new enough MySQL client

  // Added in MySQL 8.0.18
  // HHVM_RC_INT(MYSQL_CLIENT_CR_COMPRESSION_WRONGLY_CONFIGURED, CR_COMPRESSION_WRONGLY_CONFIGURED)
  // Added in MySQL 8.0.20
  // HHVM_RC_INT(MYSQL_CLIENT_CR_KERBEROS_USER_NOT_FOUND, CR_KERBEROS_USER_NOT_FOUND)
}

mysqlExtension s_mysql_extension;

bool mysqlExtension::ReadOnly = false;
#ifdef HHVM_FACEBOOK
bool mysqlExtension::Localize = false;
#endif
int mysqlExtension::ConnectTimeout = 1000;
int mysqlExtension::ReadTimeout = 60000;
int mysqlExtension::WaitTimeout = -1;
int mysqlExtension::SlowQueryThreshold = 1000; // ms
int mysqlExtension::MaxRetryOpenOnFail = 1;
int mysqlExtension::MaxRetryQueryOnFail = 1;
std::string mysqlExtension::Socket = "";
bool mysqlExtension::TypedResults = true;

void mysqlExtension::moduleLoad(const IniSetting::Map& ini, Hdf config) {
  Config::Bind(ReadOnly, ini, config, "MySQL.ReadOnly", false);
#ifdef HHVM_FACEBOOK
  Config::Bind(Localize, ini, config, "MySQL.Localize", false);
#endif
  Config::Bind(ConnectTimeout, ini, config, "MySQL.ConnectTimeout", 1000);
  Config::Bind(ReadTimeout, ini, config, "MySQL.ReadTimeout", 60000);
  Config::Bind(WaitTimeout, ini, config, "MySQL.WaitTimeout", -1);
  Config::Bind(SlowQueryThreshold, ini, config, "MySQL.SlowQueryThreshold",
               1000);
  Config::Bind(MaxRetryOpenOnFail, ini, config, "MySQL.MaxRetryOpenOnFail", 1);
  Config::Bind(MaxRetryQueryOnFail, ini, config, "MySQL.MaxRetryQueryOnFail",
               1);
  Config::Bind(Socket, ini, config, "MySQL.Socket", "");
  Config::Bind(TypedResults, ini, config, "MySQL.TypedResults", true);
}

}
