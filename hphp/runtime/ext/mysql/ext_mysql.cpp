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
#include "hphp/runtime/ext/mysql/ext_mysql.h"

#include <boost/lexical_cast.hpp>

#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>

#include "folly/ScopeGuard.h"
#include "folly/String.h"

#include "hphp/runtime/ext/mysql/mysql_common.h"
#include "hphp/runtime/ext/mysql/mysql_stats.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

using std::string;

bool mysqlExtension::ReadOnly = false;
#ifdef FACEBOOK
bool mysqlExtension::Localize = false;
#endif
int mysqlExtension::ConnectTimeout = 1000;
int mysqlExtension::ReadTimeout = 60000;
int mysqlExtension::WaitTimeout = -1;
int mysqlExtension::SlowQueryThreshold = 1000; // ms
bool mysqlExtension::KillOnTimeout = false;
int mysqlExtension::MaxRetryOpenOnFail = 1;
int mysqlExtension::MaxRetryQueryOnFail = 1;
std::string mysqlExtension::Socket = "";
bool mysqlExtension::TypedResults = true;

mysqlExtension s_mysql_extension;

///////////////////////////////////////////////////////////////////////////////

Variant f_mysql_connect(const String& server /* = null_string */,
                        const String& username /* = null_string */,
                        const String& password /* = null_string */,
                        bool new_link /* = false */,
                        int client_flags /* = 0 */,
                        int connect_timeout_ms /* = -1 */,
                        int query_timeout_ms /* = -1 */) {
  return php_mysql_do_connect(server, username, password, "",
                              client_flags, false, false,
                              connect_timeout_ms, query_timeout_ms);
}

Variant f_mysql_connect_with_db(const String& server /* = null_string */,
                        const String& username /* = null_string */,
                        const String& password /* = null_string */,
                        const String& database /* = null_string */,
                        bool new_link /* = false */,
                        int client_flags /* = 0 */,
                        int connect_timeout_ms /* = -1 */,
                        int query_timeout_ms /* = -1 */) {
  return php_mysql_do_connect(server, username, password, database,
                              client_flags, false, false,
                              connect_timeout_ms, query_timeout_ms);
}

Variant f_mysql_pconnect(const String& server /* = null_string */,
                         const String& username /* = null_string */,
                         const String& password /* = null_string */,
                         int client_flags /* = 0 */,
                         int connect_timeout_ms /* = -1 */,
                         int query_timeout_ms /* = -1 */) {
  return php_mysql_do_connect(server, username, password, "",
                              client_flags, true, false,
                              connect_timeout_ms, query_timeout_ms);
}

Variant f_mysql_pconnect_with_db(const String& server /* = null_string */,
                         const String& username /* = null_string */,
                         const String& password /* = null_string */,
                         const String& database /* = null_string */,
                         int client_flags /* = 0 */,
                         int connect_timeout_ms /* = -1 */,
                         int query_timeout_ms /* = -1 */) {
  return php_mysql_do_connect(server, username, password, database,
                              client_flags, true, false,
                              connect_timeout_ms, query_timeout_ms);
}

bool f_mysql_set_timeout(int query_timeout_ms /* = -1 */,
                         const Variant& link_identifier /* = null */) {
  MySQL::SetDefaultReadTimeout(query_timeout_ms);
  return true;
}

String f_mysql_escape_string(const String& unescaped_string) {
  char *new_str = (char *)malloc(unescaped_string.size() * 2 + 1);
  int new_len = mysql_escape_string(new_str, unescaped_string.data(),
                                    unescaped_string.size());
  return String(new_str, new_len, AttachString);
}

Variant f_mysql_real_escape_string(const String& unescaped_string,
                                   const Variant& link_identifier /* = null */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (conn) {
    char *new_str = (char *)malloc(unescaped_string.size() * 2 + 1);
    int new_len = mysql_real_escape_string(conn, new_str,
                                           unescaped_string.data(),
                                           unescaped_string.size());
    return String(new_str, new_len, AttachString);
  }
  return false;
}

String f_mysql_get_client_info() {
  return String(mysql_get_client_info(), CopyString);
}
Variant f_mysql_set_charset(const String& charset,
                                   const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return uninit_null();
  return !mysql_set_character_set(conn, charset.data());
}
Variant f_mysql_ping(const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return uninit_null();
  return !mysql_ping(conn);
}
Variant f_mysql_client_encoding(const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return String(mysql_character_set_name(conn), CopyString);
}
Variant f_mysql_close(const Variant& link_identifier /* = uninit_null() */) {
  return MySQL::CloseConn(link_identifier);
}

Variant f_mysql_errno(const Variant& link_identifier /* = null */) {
  MySQL *mySQL = MySQL::Get(link_identifier);
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

Variant f_mysql_error(const Variant& link_identifier /* = null */) {
  MySQL *mySQL = MySQL::Get(link_identifier);
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

Variant f_mysql_warning_count(const Variant& link_identifier /* = null */) {
  MySQL *mySQL = MySQL::Get(link_identifier);
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

Variant f_mysql_get_host_info(const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return String(mysql_get_host_info(conn), CopyString);
}
Variant f_mysql_get_proto_info(const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return (int64_t)mysql_get_proto_info(conn);
}
Variant f_mysql_get_server_info(const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return String(mysql_get_server_info(conn), CopyString);
}
Variant f_mysql_info(const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return String(mysql_info(conn), CopyString);
}
Variant f_mysql_insert_id(const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return static_cast<int64_t>(mysql_insert_id(conn));
}
Variant f_mysql_stat(const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return String(mysql_stat(conn), CopyString);
}
Variant f_mysql_thread_id(const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return (int64_t)mysql_thread_id(conn);
}
Variant f_mysql_create_db(const String& db,
                                 const Variant& link_identifier /* = uninit_null() */) {
  throw NotSupportedException
    (__func__, "Deprecated. Use mysql_query(CREATE DATABASE) instead.");
}
Variant f_mysql_select_db(const String& db,
                                 const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return mysql_select_db(conn, db.data()) == 0;
}
Variant f_mysql_drop_db(const String& db,
                               const Variant& link_identifier /* = uninit_null() */) {
  throw NotSupportedException
    (__func__, "Deprecated. Use mysql_query(DROP DATABASE) instead.");
}
Variant f_mysql_affected_rows(const Variant& link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return static_cast<int64_t>(mysql_affected_rows(conn));
}

///////////////////////////////////////////////////////////////////////////////
// query functions

Variant f_mysql_query(const String& query, const Variant& link_identifier /* = null */) {
  return php_mysql_do_query_and_get_result(query, link_identifier, true, false);
}

Variant f_mysql_multi_query(const String& query, const Variant& link_identifier /* = null */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (conn == nullptr) {
    return false;
  }
  MySQL *mySQL = MySQL::Get(link_identifier);
  if (!mySQL->m_multi_query && !mysql_set_server_option(conn, MYSQL_OPTION_MULTI_STATEMENTS_ON)) {
    mySQL->m_multi_query = true;
  }

  if (mysql_real_query(conn, query.data(), query.size())) {
#ifdef HHVM_MYSQL_TRACE_MODE
    if (RuntimeOption::EnableHipHopSyntax) {
      raise_notice("runtime/ext_mysql: failed executing [%s] [%s]",
                   query.data(), mysql_error(conn));
    }
#endif
      // turning this off clears the errors
      if (!mysql_set_server_option(conn, MYSQL_OPTION_MULTI_STATEMENTS_OFF)) {
        mySQL->m_multi_query = false;
      }
      return false;
  }
  return true;
}

int f_mysql_next_result(const Variant& link_identifier /* = null */) {
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

bool f_mysql_more_results(const Variant& link_identifier /* = null */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (conn == nullptr) {
    return false;
  }
  return mysql_more_results(conn);
}

Variant f_mysql_fetch_result(const Variant& link_identifier /* = null */) {
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

    return Resource(NEWOBJ(MySQLResult)(mysql_result));
}

Variant f_mysql_unbuffered_query(const String& query,
                                 const Variant& link_identifier /* = null */) {
  return php_mysql_do_query_and_get_result(query, link_identifier, false, false);
}

Variant f_mysql_db_query(const String& database, const String& query,
                         const Variant& link_identifier /* = uninit_null() */) {
  throw NotSupportedException
    (__func__, "Deprecated. Use mysql_query() instead.");
}

Variant f_mysql_list_dbs(const Variant& link_identifier /* = null */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  MYSQL_RES *res = mysql_list_dbs(conn, NULL);
  if (!res) {
    raise_warning("Unable to save MySQL query result");
    return false;
  }
  return Resource(NEWOBJ(MySQLResult)(res));
}

Variant f_mysql_list_tables(const String& database,
                            const Variant& link_identifier /* = null */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  if (mysql_select_db(conn, database.data())) {
    return false;
  }
  MYSQL_RES *res = mysql_list_tables(conn, NULL);
  if (!res) {
    raise_warning("Unable to save MySQL query result");
    return false;
  }
  return Resource(NEWOBJ(MySQLResult)(res));
}

Variant f_mysql_list_fields(const String& database_name, const String& table_name,
                            const Variant& link_identifier /* = uninit_null() */) {
  throw NotSupportedException
    (__func__, "Deprecated. Use mysql_query(SHOW COLUMNS FROM table "
     "[LIKE 'name']) instead.");
}

Variant f_mysql_list_processes(const Variant& link_identifier /* = null */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  MYSQL_RES *res = mysql_list_processes(conn);
  if (!res) {
    raise_warning("Unable to save MySQL query result");
    return false;
  }
  return Resource(NEWOBJ(MySQLResult)(res));
}

///////////////////////////////////////////////////////////////////////////////
// async

/* The mysql_*_nonblocking calls are Facebook extensions to
   libmysqlclient; for now, protect with an ifdef.  Once open sourced,
   the client will be detectable via its own ifdef. */
#ifdef FACEBOOK

Variant f_mysql_async_connect_start(const String& server /* = null_string */,
                                    const String& username /* = null_string */,
                                    const String& password /* = null_string */,
                                    const String& database /* = null_string */) {
  return php_mysql_do_connect(server, username, password, database,
                              0, false, true, 0, 0);
}

bool f_mysql_async_connect_completed(const Variant& link_identifier) {
  MySQL* mySQL = MySQL::Get(link_identifier);
  if (!mySQL) {
    raise_warning("supplied argument is not a valid MySQL-Link resource");
    return true;
  }

  MYSQL* conn = mySQL->get();
  if (conn->async_op_status != ASYNC_OP_CONNECT) {
    // Don't warn if we're in UNSET state (ie between queries, etc)
    if (conn->async_op_status != ASYNC_OP_UNSET) {
      raise_warning("runtime/ext_mysql: no pending async connect in progress");
    }
    return true;
  }

  int error = 0;
  auto status = mysql_real_connect_nonblocking_run(conn, &error);
  return status == NET_ASYNC_COMPLETE;
}

bool f_mysql_async_query_start(const String& query, const Variant& link_identifier) {
  MYSQL* conn = MySQL::GetConn(link_identifier);
  if (!conn) {
    return false;
  }

  if (conn->async_op_status != ASYNC_OP_UNSET) {
    raise_warning("runtime/ext_mysql: attempt to run async query while async "
                  "operation already pending");
    return false;
  }
  Variant ret = php_mysql_do_query_and_get_result(query, link_identifier, true, true);
  if (ret.getRawType() != KindOfBoolean) {
    raise_warning("runtime/ext_mysql: unexpected return from "
                  "php_mysql_do_query_and_get_result");
    return false;
  }
  return ret.toBooleanVal();
}

Variant f_mysql_async_query_result(const Variant& link_identifier) {
  MySQL* mySQL = MySQL::Get(link_identifier);
  if (!mySQL) {
    raise_warning("supplied argument is not a valid MySQL-Link resource");
    return Variant(Variant::NullInit());
  }
  MYSQL* conn = mySQL->get();
  if (!conn || (conn->async_op_status != ASYNC_OP_QUERY &&
                conn->async_op_status != ASYNC_OP_UNSET)) {
    raise_warning("runtime/ext_mysql: attempt to check query result when query "
                  "not executing");
    return Variant(Variant::NullInit());
  }

  int error = 0;
  auto status = mysql_real_query_nonblocking(
    conn, mySQL->m_async_query.data(), mySQL->m_async_query.size(), &error);

  if (status != NET_ASYNC_COMPLETE) {
    return Variant(Variant::NullInit());
  }

  if (error) {
    return Variant(Variant::NullInit());
  }

  mySQL->m_async_query.reset();

  MYSQL_RES* mysql_result = mysql_use_result(conn);
  MySQLResult *r = NEWOBJ(MySQLResult)(mysql_result);
  r->setAsyncConnection(mySQL);
  Resource ret(r);
  return ret;
}

bool f_mysql_async_query_completed(const Variant& result) {
  MySQLResult *res = result.toResource().getTyped<MySQLResult>
    (!RuntimeOption::ThrowBadTypeExceptions,
     !RuntimeOption::ThrowBadTypeExceptions);
  return !res || res->get() == NULL;
}

Variant f_mysql_async_fetch_array(const Variant& result, int result_type /* = 1 */) {
  if ((result_type & PHP_MYSQL_BOTH) == 0) {
    throw_invalid_argument("result_type: %d", result_type);
    return false;
  }

  MySQLResult* res = php_mysql_extract_result(result);
  if (!res) {
    return false;
  }

  MYSQL_RES* mysql_result = res->get();
  if (!mysql_result) {
    raise_warning("invalid parameter to mysql_async_fetch_array");
    return false;
  }

  MYSQL_ROW mysql_row = NULL;
  int status = mysql_fetch_row_nonblocking(mysql_result, &mysql_row);
  // Last row, or no row yet available.
  if (status != NET_ASYNC_COMPLETE) {
    return false;
  }
  if (mysql_row == NULL) {
    res->close();
    return false;
  }

  unsigned long *mysql_row_lengths = mysql_fetch_lengths(mysql_result);
  if (!mysql_row_lengths) {
    return false;
  }

  mysql_field_seek(mysql_result, 0);

  Array ret;
  MYSQL_FIELD *mysql_field;
  int i;
  for (mysql_field = mysql_fetch_field(mysql_result), i = 0; mysql_field;
       mysql_field = mysql_fetch_field(mysql_result), i++) {
    Variant data;
    if (mysql_row[i]) {
      data = mysql_makevalue(String(mysql_row[i], mysql_row_lengths[i],
                                    CopyString), mysql_field);
    }
    if (result_type & PHP_MYSQL_NUM) {
      ret.set(i, data);
    }
    if (result_type & PHP_MYSQL_ASSOC) {
      ret.set(String(mysql_field->name, CopyString), data);
    }
  }

  return ret;
}

// This function takes an array of arrays, each of which is of the
// form array($dbh, ...).  The only thing that matters in the inner
// arrays is the first element being a MySQL instance.  It then
// procedes to block for up to 'timeout' seconds, waiting for the
// first actionable descriptor(s), which it then returns in the form
// of the original arrays passed in.  The intention is the caller
// would include other information they care about in the tail of the
// array so they can decide how to act on the
// potentially-now-queryable descriptors.
//
// This function is a poor shadow of how the async library can be
// used; for more complex cases, we'd use libevent and share our event
// loop with other IO operations such as memcache ops, thrift calls,
// etc.  That said, this function is reasonably efficient for most use
// cases.
Variant f_mysql_async_wait_actionable(const Variant& items, double timeout) {
  size_t count = items.toArray().size();
  if (count == 0 || timeout < 0) {
    return empty_array;
  }

  struct pollfd* fds = (struct pollfd*)calloc(count, sizeof(struct pollfd));
  SCOPE_EXIT { free(fds); };

  // Walk our input, determine what kind of poll() operation is
  // necessary for the descriptor in question, and put an entry into
  // fds.
  int nfds = 0;
  for (ArrayIter iter(items.toArray()); iter; ++iter) {
    Array entry = iter.second().toArray();
    if (entry.size() < 1) {
      raise_warning("element %d did not have at least one entry",
                   nfds);
      return empty_array;
    }

    MySQL* mySQL = entry.rvalAt(0).toResource().getTyped<MySQL>();
    MYSQL* conn = mySQL->get();
    if (conn->async_op_status == ASYNC_OP_UNSET) {
      raise_warning("runtime/ext_mysql: no pending async operation in "
                    "progress");
      return empty_array;
    }

    pollfd* fd = &fds[nfds++];
    fd->fd = mysql_get_file_descriptor(conn);
    if (conn->net.async_blocking_state == NET_NONBLOCKING_READ) {
      fd->events = POLLIN;
    } else {
      fd->events = POLLOUT;
    }
    fd->revents = 0;
  }

  // The poll itself; either the timeout is hit or one or more of the
  // input fd's is ready.
  int timeout_millis = static_cast<long>(timeout * 1000);
  int res = poll(fds, nfds, timeout_millis);
  if (res == -1) {
    raise_warning("unable to poll [%d]: %s", errno,
                  folly::errnoStr(errno).c_str());
    return empty_array;
  }

  // Now just find the ones that are ready, and copy the corresponding
  // arrays from our input array into our return value.
  Array ret = Array::Create();
  nfds = 0;
  for (ArrayIter iter(items.toArray()); iter; ++iter) {
    Array entry = iter.second().toArray();
    if (entry.size() < 1) {
      raise_warning("element %d did not have at least one entry",
                   nfds);
      return empty_array;
    }
    MySQL* mySQL = entry.rvalAt(0).toResource().getTyped<MySQL>();
    MYSQL* conn = mySQL->get();

    pollfd* fd = &fds[nfds++];
    if (fd->fd != mysql_get_file_descriptor(conn)) {
      raise_warning("poll returned events out of order wtf");
      continue;
    }
    if (fd->revents != 0) {
      ret.append(iter.second());
    }
  }

  return ret;
}

int64_t f_mysql_async_status(const Variant& link_identifier) {
  MySQL *mySQL = MySQL::Get(link_identifier);
  if (!mySQL || !mySQL->get()) {
    raise_warning("supplied argument is not a valid MySQL-Link resource");
    return -1;
  }

  return mySQL->get()->async_op_status;
}

#else  // FACEBOOK

Variant f_mysql_async_connect_start(const String& server,
                                    const String& username,
                                    const String& password,
                                    const String& database) {
  throw NotImplementedException(__func__);
}

bool f_mysql_async_connect_completed(const Variant& link_identifier) {
  throw NotImplementedException(__func__);
}

bool f_mysql_async_query_start(const String& query, const Variant& link_identifier) {
  throw NotImplementedException(__func__);
}

Variant f_mysql_async_query_result(const Variant& link_identifier) {
  throw NotImplementedException(__func__);
}

bool f_mysql_async_query_completed(const Variant& result) {
  throw NotImplementedException(__func__);
}

Variant f_mysql_async_fetch_array(const Variant& result, int result_type /* = 1 */) {
  throw NotImplementedException(__func__);
}

Variant f_mysql_async_wait_actionable(const Variant& items, double timeout) {
  throw NotImplementedException(__func__);
}

int64_t f_mysql_async_status(const Variant& link_identifier) {
  throw NotImplementedException(__func__);
}

#endif

///////////////////////////////////////////////////////////////////////////////
// row operations

bool f_mysql_data_seek(const Variant& result, int row) {
  MySQLResult *res = php_mysql_extract_result(result);
  if (res == NULL) return false;

  return res->seekRow(row);
}

Variant f_mysql_fetch_row(const Variant& result) {
  return php_mysql_fetch_hash(result, PHP_MYSQL_NUM);
}

Variant f_mysql_fetch_assoc(const Variant& result) {
  return php_mysql_fetch_hash(result, PHP_MYSQL_ASSOC);
}

Variant f_mysql_fetch_array(const Variant& result, int result_type /* = 3 */) {
  return php_mysql_fetch_hash(result, result_type);
}

Variant f_mysql_fetch_object(const Variant& result,
                             const String& class_name /* = "stdClass" */,
                             const Array& params /* = null */) {
  Variant properties = php_mysql_fetch_hash(result, PHP_MYSQL_ASSOC);
  if (!same(properties, false)) {
    Object obj = create_object(class_name, params);
    obj->o_setArray(properties.toArray());

    return obj;
  }
  return false;
}

Variant f_mysql_fetch_lengths(const Variant& result) {
  MySQLResult *res = php_mysql_extract_result(result);
  if (res == NULL) return false;

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

Variant f_mysql_result(const Variant& result, int row,
                       const Variant& field /* = null_variant */) {
  MySQLResult *res = php_mysql_extract_result(result);
  if (res == NULL) return false;

  MYSQL_RES *mysql_result = NULL;
  MYSQL_ROW sql_row = NULL;
  unsigned long *sql_row_lengths = NULL;

  if (res->isLocalized()) {
    if (!res->seekRow(row)) return false;
    if (!res->fetchRow()) return false;
  } else {
    mysql_result = res->get();
    if (row < 0 || row >= (int)mysql_num_rows(mysql_result)) {
      raise_warning("Unable to jump to row %d on MySQL result index %d",
                      row, result.toResource()->o_getId());
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
                        field_name.data(), result.toResource()->o_getId());
        return false;
      }
    } else {
      field_offset = field.toInt32();
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
  return uninit_null();
}

///////////////////////////////////////////////////////////////////////////////
// result functions


Variant f_mysql_db_name(const Variant& result, int row,
                        const Variant& field /* = null_variant */) {
  return f_mysql_result(result, row, field);
}
Variant f_mysql_tablename(const Variant& result, int i) {
  return f_mysql_result(result, i);
}

Variant f_mysql_num_fields(const Variant& result) {
  MySQLResult *res = php_mysql_extract_result(result);
  if (res) {
    return res->getFieldCount();
  }
  return false;
}

Variant f_mysql_num_rows(const Variant& result) {
  MySQLResult *res = php_mysql_extract_result(result);
  if (res) {
    return res->getRowCount();
  }
  return false;
}

Variant f_mysql_free_result(const Variant& result) {
  MySQLResult *res = php_mysql_extract_result(result);
  if (res) {
    res->close();
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// field info

Variant f_mysql_fetch_field(const Variant& result, int field /* = -1 */) {
  MySQLResult *res = php_mysql_extract_result(result);
  if (res == NULL) return false;

  if (field != -1) {
    if (!res->seekField(field)) return false;
  }
  MySQLFieldInfo *info;
  if (!(info = res->fetchFieldInfo())) return false;

  Object obj(SystemLib::AllocStdClassObject());
  obj->o_set("name",         info->name);
  obj->o_set("table",        info->table);
  obj->o_set("def",          info->def);
  obj->o_set("max_length",   (int)info->max_length);
  obj->o_set("not_null",     IS_NOT_NULL(info->flags)? 1 : 0);
  obj->o_set("primary_key",  IS_PRI_KEY(info->flags)? 1 : 0);
  obj->o_set("multiple_key", info->flags & MULTIPLE_KEY_FLAG? 1 : 0);
  obj->o_set("unique_key",   info->flags & UNIQUE_KEY_FLAG? 1 : 0);
  obj->o_set("numeric",      IS_NUM(info->type)? 1 : 0);
  obj->o_set("blob",         IS_BLOB(info->flags)? 1 : 0);
  obj->o_set("type",         php_mysql_get_field_name(info->type));
  obj->o_set("unsigned",     info->flags & UNSIGNED_FLAG? 1 : 0);
  obj->o_set("zerofill",     info->flags & ZEROFILL_FLAG? 1 : 0);
  return obj;
}

bool f_mysql_field_seek(const Variant& result, int field /* = 0 */) {
  MySQLResult *res = php_mysql_extract_result(result);
  if (res == NULL) return false;
  return res->seekField(field);
}

Variant f_mysql_field_name(const Variant& result, int field /* = 0 */) {
  return php_mysql_field_info(result, field, PHP_MYSQL_FIELD_NAME);
}
Variant f_mysql_field_table(const Variant& result, int field /* = 0 */) {
  return php_mysql_field_info(result, field, PHP_MYSQL_FIELD_TABLE);
}
Variant f_mysql_field_len(const Variant& result, int field /* = 0 */) {
  return php_mysql_field_info(result, field, PHP_MYSQL_FIELD_LEN);
}
Variant f_mysql_field_type(const Variant& result, int field /* = 0 */) {
  return php_mysql_field_info(result, field, PHP_MYSQL_FIELD_TYPE);
}
Variant f_mysql_field_flags(const Variant& result, int field /* = 0 */) {
  return php_mysql_field_info(result, field, PHP_MYSQL_FIELD_FLAGS);
}

///////////////////////////////////////////////////////////////////////////////
}
