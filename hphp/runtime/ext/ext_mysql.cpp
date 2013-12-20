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
#include "hphp/runtime/ext/ext_mysql.h"

#include "folly/ScopeGuard.h"

#include "hphp/runtime/ext/ext_preg.h"
#include "hphp/runtime/ext/ext_network.h"
#include "hphp/runtime/ext/mysql_stats.h"
#include "hphp/runtime/base/socket.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/server/server-stats.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/extended-logger.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/util/network.h"
#include "hphp/util/timer.h"
#include "hphp/util/db-mysql.h"
#include "folly/String.h"
#include <netinet/in.h>
#include <netdb.h>

#include "hphp/system/systemlib.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool mysqlExtension::ReadOnly = false;
#ifdef FACEBOOK
bool mysqlExtension::Localize = false;
#endif
int mysqlExtension::ConnectTimeout = 1000;
int mysqlExtension::ReadTimeout = 1000;
int mysqlExtension::WaitTimeout = -1;
int mysqlExtension::SlowQueryThreshold = 1000; // ms
bool mysqlExtension::KillOnTimeout = false;
int mysqlExtension::MaxRetryOpenOnFail = 1;
int mysqlExtension::MaxRetryQueryOnFail = 1;
std::string mysqlExtension::Socket = "";
bool mysqlExtension::TypedResults = true;

mysqlExtension s_mysql_extension;

///////////////////////////////////////////////////////////////////////////////

MySQLResult::MySQLResult(MYSQL_RES *res, bool localized /* = false */)
  : m_res(res)
  , m_current_async_row(nullptr)
  , m_localized(localized)
  , m_fields(nullptr)
  , m_current_field(-1)
  , m_field_count(0)
  , m_conn(nullptr)
{
  if (localized) {
    m_res = nullptr; // ensure that localized results don't have another result
    m_rows = smart::list<smart::vector<Variant>>(1); // sentinel
    m_current_row = m_rows->begin();
    m_row_ready = false;
    m_row_count = 0;
  }
}

MySQLResult::~MySQLResult() {
  close();
  if (m_fields) {
    smart_delete_array(m_fields, m_field_count);
    m_fields = nullptr;
  }
  if (m_conn) {
    m_conn->decRefCount();
    m_conn = nullptr;
  }
}

void MySQLResult::sweep() {
  close();
  // Note that ~MySQLResult is *not* going to run when we are swept.
}

///////////////////////////////////////////////////////////////////////////////

class MySQLStaticInitializer {
public:
  MySQLStaticInitializer() {
    mysql_library_init(0, NULL, NULL);
  }
};
static MySQLStaticInitializer s_mysql_initializer;

class MySQLRequestData : public RequestEventHandler {
public:
  virtual void requestInit() {
    defaultConn.reset();
    readTimeout = mysqlExtension::ReadTimeout;
    totalRowCount = 0;
  }

  virtual void requestShutdown() {
    defaultConn.reset();
    totalRowCount = 0;
  }

  Resource defaultConn;
  int readTimeout;
  int totalRowCount; // from all queries in current request
};
IMPLEMENT_STATIC_REQUEST_LOCAL(MySQLRequestData, s_mysql_data);

///////////////////////////////////////////////////////////////////////////////
// class MySQL statics

int MySQL::s_default_port = 0;

MySQL *MySQL::Get(CVarRef link_identifier) {
  if (link_identifier.isNull()) {
    return GetDefaultConn();
  }
  MySQL *mysql = link_identifier.toResource().getTyped<MySQL>
    (!RuntimeOption::ThrowBadTypeExceptions,
     !RuntimeOption::ThrowBadTypeExceptions);
  return mysql;
}

MYSQL *MySQL::GetConn(CVarRef link_identifier, MySQL **rconn /* = NULL */) {
  MySQL *mySQL = Get(link_identifier);
  MYSQL *ret = nullptr;
  if (mySQL) {
    ret = mySQL->get();
  }
  if (ret == nullptr) {
    raise_warning("supplied argument is not a valid MySQL-Link resource");
  }
  // Don't return a connection where mysql_real_connect() failed to most
  // f_mysql_* APIs (the ones that deal with errno where we do want to do this
  // anyway use MySQL::Get instead) as mysqlclient doesn't support passing
  // connections in that state and it can crash.
  if (mySQL && mySQL->m_last_error_set) {
    ret = nullptr;
  } else if (rconn) {
    *rconn = mySQL;
  }
  return ret;
}

bool MySQL::CloseConn(CVarRef link_identifier) {
  MySQL *mySQL = Get(link_identifier);
  if (mySQL && !mySQL->isPersistent()) {
    mySQL->close();
  }
  return true;
}

int MySQL::GetDefaultPort() {
  if (s_default_port <= 0) {
    s_default_port = MYSQL_PORT;
    char *env = getenv("MYSQL_TCP_PORT");
    if (env && *env) {
      s_default_port = atoi(env);
    } else {
      Variant ret = f_getservbyname("mysql", "tcp");
      if (!same(ret, false)) {
        s_default_port = ret.toInt16();
      }
    }
  }
  return s_default_port;
}

String MySQL::GetDefaultSocket() {
  if (!mysqlExtension::Socket.empty()) {
    return mysqlExtension::Socket;
  }
  return MYSQL_UNIX_ADDR;
}

String MySQL::GetHash(const String& host, int port, const String& socket, const String& username,
                      const String& password, int client_flags) {
  char buf[1024];
  snprintf(buf, sizeof(buf), "%s:%d:%s:%s:%s:%d",
           host.data(), port, socket.data(),
           username.data(), password.data(), client_flags);
  return String(buf, CopyString);
}

MySQL *MySQL::GetCachedImpl(const char *name, const String& host, int port,
                            const String& socket, const String& username, const String& password,
                            int client_flags) {
  String key = GetHash(host, port, socket, username, password, client_flags);
  return dynamic_cast<MySQL*>(g_persistentObjects->get(name, key.data()));
}

void MySQL::SetCachedImpl(const char *name, const String& host, int port,
                          const String& socket, const String& username, const String& password,
                          int client_flags, MySQL *conn) {
  String key = GetHash(host, port, socket, username, password, client_flags);
  g_persistentObjects->set(name, key.data(), conn);
}

MySQL *MySQL::GetDefaultConn() {
  return s_mysql_data->defaultConn.getTyped<MySQL>(true);
}

void MySQL::SetDefaultConn(MySQL *conn) {
  s_mysql_data->defaultConn = conn;
}

///////////////////////////////////////////////////////////////////////////////
// class MySQL
static MYSQL *configure_conn(MYSQL* conn) {
  mysql_options(conn, MYSQL_OPT_LOCAL_INFILE, 0);
  if (mysqlExtension::ConnectTimeout) {
    MySQLUtil::set_mysql_timeout(conn, MySQLUtil::ConnectTimeout,
                                 mysqlExtension::ConnectTimeout);
  }
  int readTimeout = s_mysql_data->readTimeout;
  if (readTimeout) {
    MySQLUtil::set_mysql_timeout(conn, MySQLUtil::ReadTimeout, readTimeout);
    MySQLUtil::set_mysql_timeout(conn, MySQLUtil::WriteTimeout, readTimeout);
  }
  return conn;
}

static MYSQL *create_new_conn() {
  return configure_conn(mysql_init(nullptr));
}

MySQL::MySQL(const char *host, int port, const char *username,
             const char *password, const char *database,
             MYSQL* raw_connection)
    : m_port(port), m_last_error_set(false), m_last_errno(0),
      m_xaction_count(0), m_multi_query(false) {
  if (host) m_host = host;
  if (username) m_username = username;
  if (password) m_password = password;
  if (database) m_database = database;

  if (raw_connection) {
    m_conn = configure_conn(raw_connection);
  } else {
    m_conn = create_new_conn();
  }
}

MySQL::~MySQL() {
  close();
}

void MySQL::sweep() {
  // may or may not be smart allocated
  delete this;
}

void MySQL::setLastError(const char *func) {
  assert(m_conn);
  m_last_error_set = true;
  m_last_errno = mysql_errno(m_conn);
  const char *error = mysql_error(m_conn);
  m_last_error = error ? error : "";
  raise_warning("%s(): %s", func, m_last_error.c_str());
}

void MySQL::close() {
  if (!m_conn) {
    return;
  }
  m_last_error_set = false;
  m_last_errno = 0;
  m_xaction_count = 0;
  m_last_error.clear();
  mysql_close(m_conn);
  m_conn = nullptr;
}

bool MySQL::connect(const String& host, int port, const String& socket, const String& username,
                    const String& password, const String& database,
                    int client_flags, int connect_timeout) {
  if (m_conn == NULL) {
    m_conn = create_new_conn();
  }
  if (connect_timeout >= 0) {
    MySQLUtil::set_mysql_timeout(m_conn, MySQLUtil::ConnectTimeout,
                                 connect_timeout);
  }
  if (RuntimeOption::EnableStats && RuntimeOption::EnableSQLStats) {
    ServerStats::Log("sql.conn", 1);
  }
  IOStatusHelper io("mysql::connect", host.data(), port);
  m_xaction_count = 0;
  bool ret = mysql_real_connect(m_conn, host.data(), username.data(),
                            password.data(),
                            (database.empty() ? NULL : database.data()),
                            port,
                            socket.empty() ? NULL : socket.data(),
                            client_flags);
  if (ret && mysqlExtension::WaitTimeout > 0) {
    String query("set session wait_timeout=");
    query += String((int64_t)(mysqlExtension::WaitTimeout / 1000));
    if (mysql_real_query(m_conn, query.data(), query.size())) {
      raise_notice("MySQL::connect: failed setting session wait timeout: %s",
                   mysql_error(m_conn));
    }
  }
  return ret;
}

bool MySQL::reconnect(const String& host, int port, const String& socket, const String& username,
                      const String& password, const String& database,
                      int client_flags, int connect_timeout) {
  if (m_conn == NULL) {
    m_conn = create_new_conn();
    if (connect_timeout >= 0) {
      MySQLUtil::set_mysql_timeout(m_conn, MySQLUtil::ConnectTimeout,
                                   connect_timeout);
    }
    if (RuntimeOption::EnableStats && RuntimeOption::EnableSQLStats) {
      ServerStats::Log("sql.reconn_new", 1);
    }
    IOStatusHelper io("mysql::connect", host.data(), port);
    return mysql_real_connect(m_conn, host.data(), username.data(),
                              password.data(),
                              (database.empty() ? NULL : database.data()),
                              port, socket.data(), client_flags);
  }

  if (!mysql_ping(m_conn)) {
    if (RuntimeOption::EnableStats && RuntimeOption::EnableSQLStats) {
      ServerStats::Log("sql.reconn_ok", 1);
    }
    if (!database.empty()) {
      mysql_select_db(m_conn, database.data());
    }
    return true;
  }

  if (connect_timeout >= 0) {
    MySQLUtil::set_mysql_timeout(m_conn, MySQLUtil::ConnectTimeout,
                                 connect_timeout);
  }
  if (RuntimeOption::EnableStats && RuntimeOption::EnableSQLStats) {
    ServerStats::Log("sql.reconn_old", 1);
  }
  IOStatusHelper io("mysql::connect", host.data(), port);
  m_xaction_count = 0;
  return mysql_real_connect(m_conn, host.data(), username.data(),
                            password.data(),
                            (database.empty() ? NULL : database.data()),
                            port, socket.data(), client_flags);
}

///////////////////////////////////////////////////////////////////////////////
// helpers

static MySQLResult *get_result(CVarRef result) {
  MySQLResult *res = result.toResource().getTyped<MySQLResult>
    (!RuntimeOption::ThrowBadTypeExceptions,
     !RuntimeOption::ThrowBadTypeExceptions);
  if (res == NULL || (res->get() == NULL && !res->isLocalized())) {
    raise_warning("supplied argument is not a valid MySQL result resource");
  }
  return res;
}

static const char *php_mysql_get_field_name(int field_type) {
  switch (field_type) {
  case FIELD_TYPE_STRING:
  case FIELD_TYPE_VAR_STRING:
    return "string";
  case FIELD_TYPE_TINY:
  case FIELD_TYPE_SHORT:
  case FIELD_TYPE_LONG:
  case FIELD_TYPE_LONGLONG:
  case FIELD_TYPE_INT24:
    return "int";
  case FIELD_TYPE_FLOAT:
  case FIELD_TYPE_DOUBLE:
  case FIELD_TYPE_DECIMAL:
    //case FIELD_TYPE_NEWDECIMAL:
    return "real";
  case FIELD_TYPE_TIMESTAMP:
    return "timestamp";
  case FIELD_TYPE_YEAR:
    return "year";
  case FIELD_TYPE_DATE:
  case FIELD_TYPE_NEWDATE:
    return "date";
  case FIELD_TYPE_TIME:
    return "time";
  case FIELD_TYPE_SET:
    return "set";
  case FIELD_TYPE_ENUM:
    return "enum";
  case FIELD_TYPE_GEOMETRY:
    return "geometry";
  case FIELD_TYPE_DATETIME:
    return "datetime";
  case FIELD_TYPE_TINY_BLOB:
  case FIELD_TYPE_MEDIUM_BLOB:
  case FIELD_TYPE_LONG_BLOB:
  case FIELD_TYPE_BLOB:
    return "blob";
  case FIELD_TYPE_NULL:
    return "null";
  default:
    break;
  }
  return "unknown";
}

#define PHP_MYSQL_FIELD_NAME  1
#define PHP_MYSQL_FIELD_TABLE 2
#define PHP_MYSQL_FIELD_LEN   3
#define PHP_MYSQL_FIELD_TYPE  4
#define PHP_MYSQL_FIELD_FLAGS 5

static Variant php_mysql_field_info(CVarRef result, int field,
                                    int entry_type) {
  MySQLResult *res = get_result(result);
  if (res == NULL) return false;

  if (!res->seekField(field)) return false;

  MySQLFieldInfo *info;
  if (!(info = res->fetchFieldInfo())) return false;

  switch (entry_type) {
  case PHP_MYSQL_FIELD_NAME:
    return info->name;
  case PHP_MYSQL_FIELD_TABLE:
    return info->table;
  case PHP_MYSQL_FIELD_LEN:
    return info->length;
  case PHP_MYSQL_FIELD_TYPE:
    return php_mysql_get_field_name(info->type);
  case PHP_MYSQL_FIELD_FLAGS:
    {
      char buf[512];
      buf[0] = '\0';
      unsigned int flags = info->flags;
#ifdef IS_NOT_NULL
      if (IS_NOT_NULL(flags)) {
        strcat(buf, "not_null ");
      }
#endif
#ifdef IS_PRI_KEY
      if (IS_PRI_KEY(flags)) {
        strcat(buf, "primary_key ");
      }
#endif
#ifdef UNIQUE_KEY_FLAG
      if (flags & UNIQUE_KEY_FLAG) {
        strcat(buf, "unique_key ");
      }
#endif
#ifdef MULTIPLE_KEY_FLAG
      if (flags & MULTIPLE_KEY_FLAG) {
        strcat(buf, "multiple_key ");
      }
#endif
#ifdef IS_BLOB
      if (IS_BLOB(flags)) {
        strcat(buf, "blob ");
      }
#endif
#ifdef UNSIGNED_FLAG
      if (flags & UNSIGNED_FLAG) {
        strcat(buf, "unsigned ");
      }
#endif
#ifdef ZEROFILL_FLAG
      if (flags & ZEROFILL_FLAG) {
        strcat(buf, "zerofill ");
      }
#endif
#ifdef BINARY_FLAG
      if (flags & BINARY_FLAG) {
        strcat(buf, "binary ");
      }
#endif
#ifdef ENUM_FLAG
      if (flags & ENUM_FLAG) {
        strcat(buf, "enum ");
      }
#endif
#ifdef SET_FLAG
      if (flags & SET_FLAG) {
        strcat(buf, "set ");
      }
#endif
#ifdef AUTO_INCREMENT_FLAG
      if (flags & AUTO_INCREMENT_FLAG) {
        strcat(buf, "auto_increment ");
      }
#endif
#ifdef TIMESTAMP_FLAG
      if (flags & TIMESTAMP_FLAG) {
        strcat(buf, "timestamp ");
      }
#endif
      int len = strlen(buf);
      /* remove trailing space, if present */
      if (len && buf[len-1] == ' ') {
        buf[len-1] = 0;
        len--;
      }

      return String(buf, len, CopyString);
    }
  default:
    break;
  }
  return false;
}

static Variant php_mysql_do_connect(String server, String username,
                                    String password, String database,
                                    int client_flags, bool persistent,
                                    bool async,
                                    int connect_timeout_ms,
                                    int query_timeout_ms) {
  if (connect_timeout_ms < 0) {
    connect_timeout_ms = mysqlExtension::ConnectTimeout;
  }
  if (query_timeout_ms < 0) {
    query_timeout_ms = s_mysql_data->readTimeout;
  }
  if (server.empty()) server = MySQL::GetDefaultServer();
  if (username.empty()) username = MySQL::GetDefaultUsername();
  if (password.empty()) password = MySQL::GetDefaultPassword();
  if (database.empty()) database = MySQL::GetDefaultDatabase();

  // server format: hostname[:port][:/path/to/socket]
  // ipv6 hostname:port is of the form [1:2:3:4:5]:port
  String host, socket;
  int port;

  auto slash_pos = server.find('/');
  if (slash_pos != string::npos) {
    socket = server.substr(slash_pos);
    server = server.substr(0, slash_pos - 1);
  }

  Util::HostURL hosturl(std::string(server), MySQL::GetDefaultPort());
  if (hosturl.isValid()) {
    host = hosturl.getHost();
    port = hosturl.getPort();
  } else {
    host = server;
    port = MySQL::GetDefaultPort();
  }

  if (socket.empty()) {
    socket = MySQL::GetDefaultSocket();
  }

  Resource ret;
  MySQL *mySQL = NULL;
  if (persistent) {
    mySQL = MySQL::GetPersistent(host, port, socket, username, password,
                                 client_flags);
  }

  if (mySQL == NULL) {
    mySQL = new MySQL(host.c_str(), port, username.c_str(), password.c_str(),
                      database.c_str());
    ret = mySQL;
    if (async) {
#ifdef FACEBOOK
      if (!mySQL->async_connect(host, port, socket, username, password,
                                database)) {
        MySQL::SetDefaultConn(mySQL); // so we can report errno by mysql_errno()
        mySQL->setLastError("mysql_real_connect_nonblocking_init");
        return false;
      }
#else
      throw NotImplementedException("mysql_async_connect_start");
#endif
    } else {
      if (!mySQL->connect(host, port, socket, username, password,
                          database, client_flags, connect_timeout_ms)) {
        MySQL::SetDefaultConn(mySQL); // so we can report errno by mysql_errno()
        mySQL->setLastError("mysql_connect");
        return false;
      }
    }
  } else {
    ret = mySQL;
    if (!mySQL->reconnect(host, port, socket, username, password,
                          database, client_flags, connect_timeout_ms)) {
      MySQL::SetDefaultConn(mySQL); // so we can report errno by mysql_errno()
      mySQL->setLastError("mysql_connect");
      return false;
    }
  }

  if (persistent) {
    MySQL::SetPersistent(host, port, socket, username, password,
                         client_flags, mySQL);
  }
  MySQL::SetDefaultConn(mySQL);
  return ret;
}

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
                         CVarRef link_identifier /* = null */) {
  if (query_timeout_ms < 0) {
    query_timeout_ms = mysqlExtension::ReadTimeout;
  }
  s_mysql_data->readTimeout = query_timeout_ms;
  return true;
}

String f_mysql_escape_string(const String& unescaped_string) {
  char *new_str = (char *)malloc(unescaped_string.size() * 2 + 1);
  int new_len = mysql_escape_string(new_str, unescaped_string.data(),
                                    unescaped_string.size());
  return String(new_str, new_len, AttachString);
}

Variant f_mysql_real_escape_string(const String& unescaped_string,
                                   CVarRef link_identifier /* = null */) {
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
                                   CVarRef link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return uninit_null();
  return !mysql_set_character_set(conn, charset.data());
}
Variant f_mysql_ping(CVarRef link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return uninit_null();
  return !mysql_ping(conn);
}
Variant f_mysql_client_encoding(CVarRef link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return String(mysql_character_set_name(conn), CopyString);
}
Variant f_mysql_close(CVarRef link_identifier /* = uninit_null() */) {
  return MySQL::CloseConn(link_identifier);
}

Variant f_mysql_errno(CVarRef link_identifier /* = null */) {
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

Variant f_mysql_error(CVarRef link_identifier /* = null */) {
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

Variant f_mysql_warning_count(CVarRef link_identifier /* = null */) {
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

Variant f_mysql_get_host_info(CVarRef link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return String(mysql_get_host_info(conn), CopyString);
}
Variant f_mysql_get_proto_info(CVarRef link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return (int64_t)mysql_get_proto_info(conn);
}
Variant f_mysql_get_server_info(CVarRef link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return String(mysql_get_server_info(conn), CopyString);
}
Variant f_mysql_info(CVarRef link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return String(mysql_info(conn), CopyString);
}
Variant f_mysql_insert_id(CVarRef link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return static_cast<int64_t>(mysql_insert_id(conn));
}
Variant f_mysql_stat(CVarRef link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return String(mysql_stat(conn), CopyString);
}
Variant f_mysql_thread_id(CVarRef link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return (int64_t)mysql_thread_id(conn);
}
Variant f_mysql_create_db(const String& db,
                                 CVarRef link_identifier /* = uninit_null() */) {
  throw NotSupportedException
    (__func__, "Deprecated. Use mysql_query(CREATE DATABASE) instead.");
}
Variant f_mysql_select_db(const String& db,
                                 CVarRef link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return mysql_select_db(conn, db.data()) == 0;
}
Variant f_mysql_drop_db(const String& db,
                               CVarRef link_identifier /* = uninit_null() */) {
  throw NotSupportedException
    (__func__, "Deprecated. Use mysql_query(DROP DATABASE) instead.");
}
Variant f_mysql_affected_rows(CVarRef link_identifier /* = uninit_null() */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return static_cast<int64_t>(mysql_affected_rows(conn));
}

///////////////////////////////////////////////////////////////////////////////
// query functions

// Zend returns strings and NULL only, not integers or floats.  We
// return ints (and, sometimes, actual doubles). This behavior can be
// disabled with MySQL { TypedResults = false } runtime option.
Variant mysql_makevalue(const String& data, MYSQL_FIELD *mysql_field) {
  return mysql_makevalue(data, mysql_field->type);
}

Variant mysql_makevalue(const String& data, enum_field_types field_type) {
  if (field_type == MYSQL_TYPE_NULL) {
    return uninit_null();
  } else if (mysqlExtension::TypedResults) {
    switch (field_type) {
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_LONGLONG:
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_YEAR:
      return data.toInt64();
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE:
      //case MYSQL_TYPE_NEWDECIMAL:
      return data.toDouble();
    default:
      break;
    }
  }
  return data;
}

#ifdef FACEBOOK
extern "C" {
struct MEM_ROOT;
unsigned long cli_safe_read(MYSQL *);
unsigned long net_field_length(unsigned char **);
void free_root(::MEM_ROOT *, int);
}

static bool php_mysql_read_rows(MYSQL *mysql, CVarRef result) {
  unsigned long pkt_len;
  unsigned char *cp;
  unsigned int fields = mysql->field_count;
  NET *net = &mysql->net;
  MySQLResult *res = get_result(result);

  if ((pkt_len = cli_safe_read(mysql)) == packet_error) {
    return false;
  }

  res->setFieldCount((int64_t)fields);

  // localizes all the rows
  while (*(cp = net->read_pos) != 254 || pkt_len >= 8) {
    res->addRow();
    for (unsigned int i = 0; i < fields; i++) {
      unsigned long len = net_field_length(&cp);
      Variant data;
      if (len != NULL_LENGTH) {
        data = mysql_makevalue(String((char *)cp, len, CopyString),
                               mysql->fields + i);
        cp += len;
        if (mysql->fields) {
          if (mysql->fields[i].max_length < len)
            mysql->fields[i].max_length = len;
        }
      }
      res->addField(std::move(data));
    }
    if ((pkt_len = cli_safe_read(mysql)) == packet_error) {
      return false;
    }
  }

  // localizes all the field info
  for (unsigned int i = 0; i < fields; i++) {
    res->setFieldInfo((int64_t)i, mysql->fields + i);
  }

  return true;
}

static Variant php_mysql_localize_result(MYSQL *mysql) {
#if MYSQL_VERSION_ID <= 50138
  mysql = mysql->last_used_con;
#endif

  if (!mysql->fields) return true;
  if (mysql->status != MYSQL_STATUS_GET_RESULT) {
    // consistent with php_mysql_do_query_general
    return true;
  }
  mysql->status = MYSQL_STATUS_READY;
  Variant result = Resource(NEWOBJ(MySQLResult)(nullptr, true));
  if (!php_mysql_read_rows(mysql, result)) {
    return false;
  }

  // clean up
  if (mysql->fields) {
    free_root(&mysql->field_alloc, 0);
  }
  mysql->unbuffered_fetch_owner = 0;

  return result;
}
#endif // FACEBOOK

static Variant php_mysql_do_query_general(const String& query, CVarRef link_id,
                                          bool use_store, bool async_mode) {
  SYNC_VM_REGS_SCOPED();
  if (mysqlExtension::ReadOnly &&
      same(f_preg_match("/^((\\/\\*.*?\\*\\/)|\\(|\\s)*select/i", query), 0)) {
    raise_notice("runtime/ext_mysql: write query not executed [%s]",
                    query.data());
    return true; // pretend it worked
  }

  MySQL *rconn = NULL;
  MYSQL *conn = MySQL::GetConn(link_id, &rconn);
  if (!conn || !rconn) return false;

  if (RuntimeOption::EnableStats && RuntimeOption::EnableSQLStats) {
    ServerStats::Log("sql.query", 1);

    // removing comments, which can be wrong actually if some string field's
    // value has /* or */ in it.
    String q = f_preg_replace("/\\/\\*.*?\\*\\//", " ", query).toString();

    Variant matches;
    f_preg_match("/^(?:\\(|\\s)*(?:"
                 "(insert).*?\\s+(?:into\\s+)?([^\\s\\(,]+)|"
                 "(update|set|show)\\s+([^\\s\\(,]+)|"
                 "(replace).*?\\s+into\\s+([^\\s\\(,]+)|"
                 "(delete).*?\\s+from\\s+([^\\s\\(,]+)|"
                 "(select).*?[\\s`]+from\\s+([^\\s\\(,]+))/is",
                 q, ref(matches));
    int size = matches.toArray().size();
    if (size > 2) {
      string verb = Util::toLower(matches[size - 2].toString().data());
      string table = Util::toLower(matches[size - 1].toString().data());
      if (!table.empty() && table[0] == '`') {
        table = table.substr(1, table.length() - 2);
      }
      ServerStats::Log(string("sql.query.") + table + "." + verb, 1);
      if (RuntimeOption::EnableStats && RuntimeOption::EnableSQLTableStats) {
        MySqlStats::Record(verb, rconn->m_xaction_count, table);
        if (verb == "update") {
          f_preg_match("([^\\s,]+)\\s*=\\s*([^\\s,]+)[\\+\\-]",
                       q, ref(matches));
          size = matches.toArray().size();
          if (size > 2 && same(matches[1], matches[2])) {
            MySqlStats::Record("incdec", rconn->m_xaction_count, table);
          }
        }
        // we only bump it up when we're in the middle of a transaction
        if (rconn->m_xaction_count) {
          ++rconn->m_xaction_count;
        }
      }
    } else {
      f_preg_match("/^(?:(?:\\/\\*.*?\\*\\/)|\\(|\\s)*"
                   "(begin|commit|rollback)/is",
                   query, ref(matches));
      size = matches.toArray().size();
      if (size == 2) {
        string verb = Util::toLower(matches[1].toString().data());
        rconn->m_xaction_count = ((verb == "begin") ? 1 : 0);
        ServerStats::Log(string("sql.query.") + verb, 1);
        if (RuntimeOption::EnableStats && RuntimeOption::EnableSQLTableStats) {
          MySqlStats::Record(verb);
        }
      } else {
        raise_warning("Unable to record MySQL stats with: %s", query.data());
        ServerStats::Log("sql.query.unknown", 1);
      }
    }
  }

  SlowTimer timer(mysqlExtension::SlowQueryThreshold,
                  "runtime/ext_mysql: slow query", query.data());
  IOStatusHelper io("mysql::query", rconn->m_host.c_str(), rconn->m_port);
  unsigned long tid = mysql_thread_id(conn);

  // disable explicitly
  MySQL *mySQL = MySQL::Get(link_id);
  if (mySQL->m_multi_query && !mysql_set_server_option(conn, MYSQL_OPTION_MULTI_STATEMENTS_OFF)) {
    mySQL->m_multi_query = false;
  }

  if (async_mode) {
#ifdef FACEBOOK
    mySQL->m_async_query = query;
    return true;
#else
    throw NotImplementedException("mysql_async_query_start");
#endif
  }

  if (mysql_real_query(conn, query.data(), query.size())) {
#ifdef HHVM_MYSQL_TRACE_MODE
    raise_notice("runtime/ext_mysql: failed executing [%s] [%s]", query.data(),
                 mysql_error(conn));
#endif

    // When we are timed out, and we're SELECT-ing, we're potentially
    // running a long query on the server without waiting for any results
    // back, wasting server resource. So we're sending a KILL command
    // to see if we can stop the query execution.
    if (tid && mysqlExtension::KillOnTimeout) {
      unsigned int errcode = mysql_errno(conn);
      if (errcode == 2058 /* CR_NET_READ_INTERRUPTED */ ||
          errcode == 2059 /* CR_NET_WRITE_INTERRUPTED */) {
        Variant ret = f_preg_match("/^((\\/\\*.*?\\*\\/)|\\(|\\s)*select/is",
                                   query);
        if (!same(ret, false)) {
          MYSQL *new_conn = create_new_conn();
          IOStatusHelper io("mysql::kill", rconn->m_host.c_str(),
                            rconn->m_port);
          MYSQL *connected = mysql_real_connect
            (new_conn, rconn->m_host.c_str(), rconn->m_username.c_str(),
             rconn->m_password.c_str(), NULL, rconn->m_port, NULL, 0);
          if (connected) {
            string killsql = "KILL " + boost::lexical_cast<string>(tid);
            if (mysql_real_query(connected, killsql.c_str(), killsql.size())) {
              raise_warning("Unable to kill thread %lu", tid);
            }
          }
          mysql_close(new_conn);
        }
      }
    }

    return false;
  }
  Logger::Verbose("runtime/ext_mysql: successfully executed [%dms] [%s]",
                  (int)timer.getTime(), query.data());

  MYSQL_RES *mysql_result;
  if (use_store) {
#ifdef FACEBOOK
    // Facebook specific optimization which depends
    // on versions of MySQL which allow access to the
    // grotty internals of libmysqlclient
    //
    // If php_mysql_localize_result ever gets rewritten
    // to use standard APIs, this can be opened up to everyone.
    if (mysqlExtension::Localize) {
      return php_mysql_localize_result(conn);
    }
#endif
    mysql_result = mysql_store_result(conn);
  } else {
    mysql_result = mysql_use_result(conn);
  }
  if (!mysql_result) {
    if (mysql_field_count(conn) > 0) {
      raise_warning("Unable to save result set");
      return false;
    }
    return true;
  }

  MySQLResult *r = NEWOBJ(MySQLResult)(mysql_result);
  Resource ret(r);

  if (RuntimeOption::MaxSQLRowCount > 0 &&
      (s_mysql_data->totalRowCount += r->getRowCount())
      > RuntimeOption::MaxSQLRowCount) {
    ExtendedLogger::Error
      ("MaxSQLRowCount is over: fetching at least %d rows: %s",
       s_mysql_data->totalRowCount, query.data());
    s_mysql_data->totalRowCount = 0; // so no repetitive logging
  }

  return ret;
}

Variant f_mysql_query(const String& query, CVarRef link_identifier /* = null */) {
  return php_mysql_do_query_general(query, link_identifier, true, false);
}

Variant f_mysql_multi_query(const String& query, CVarRef link_identifier /* = null */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  MySQL *mySQL = MySQL::Get(link_identifier);
  if (!mySQL->m_multi_query && !mysql_set_server_option(conn, MYSQL_OPTION_MULTI_STATEMENTS_ON)) {
    mySQL->m_multi_query = true;
  }

  if (mysql_real_query(conn, query.data(), query.size())) {
#ifdef HHVM_MYSQL_TRACE_MODE
    raise_notice("runtime/ext_mysql: failed executing [%s] [%s]", query.data(),
                  mysql_error(conn));
#endif
      // turning this off clears the errors
      if (!mysql_set_server_option(conn, MYSQL_OPTION_MULTI_STATEMENTS_OFF)) {
        mySQL->m_multi_query = false;
      }
      return false;
  }
  return true;
}

int f_mysql_next_result(CVarRef link_identifier /* = null */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!mysql_more_results(conn)) {
    raise_strict_warning("There is no next result set. "
      "Please, call mysql_more_results() to check "
      "whether to call this function/method");
  }
  return mysql_next_result(conn);
}

bool f_mysql_more_results(CVarRef link_identifier /* = null */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  return mysql_more_results(conn);
}

Variant f_mysql_fetch_result(CVarRef link_identifier /* = null */) {
    MYSQL *conn = MySQL::GetConn(link_identifier);
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
                                 CVarRef link_identifier /* = null */) {
  return php_mysql_do_query_general(query, link_identifier, false, false);
}

Variant f_mysql_db_query(const String& database, const String& query,
                         CVarRef link_identifier /* = uninit_null() */) {
  throw NotSupportedException
    (__func__, "Deprecated. Use mysql_query() instead.");
}

Variant f_mysql_list_dbs(CVarRef link_identifier /* = null */) {
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
                            CVarRef link_identifier /* = null */) {
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
                            CVarRef link_identifier /* = uninit_null() */) {
  throw NotSupportedException
    (__func__, "Deprecated. Use mysql_query(SHOW COLUMNS FROM table "
     "[LIKE 'name']) instead.");
}

Variant f_mysql_list_processes(CVarRef link_identifier /* = null */) {
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
// row operations

bool f_mysql_data_seek(CVarRef result, int row) {
  MySQLResult *res = get_result(result);
  if (res == NULL) return false;

  return res->seekRow(row);
}

#define MYSQL_ASSOC  1 << 0
#define MYSQL_NUM    1 << 1
#define MYSQL_BOTH   (MYSQL_ASSOC|MYSQL_NUM)

static Variant php_mysql_fetch_hash(CVarRef result, int result_type) {
  if ((result_type & MYSQL_BOTH) == 0) {
    throw_invalid_argument("result_type: %d", result_type);
    return false;
  }

  MySQLResult *res = get_result(result);
  if (res == NULL) return false;

  Array ret;
  if (res->isLocalized()) {
    if (!res->fetchRow()) return false;

    for (int i = 0; i < res->getFieldCount(); i++) {
      if (result_type & MYSQL_NUM) {
        ret.set(i, res->getField(i));
      }
      if (result_type & MYSQL_ASSOC) {
        MySQLFieldInfo *info = res->getFieldInfo(i);
        ret.set(info->name, res->getField(i));
      }
    }
    return ret;
  }

  MYSQL_RES *mysql_result = res->get();
  MYSQL_ROW mysql_row = mysql_fetch_row(mysql_result);
  if (!mysql_row) {
    return false;
  }
  unsigned long *mysql_row_lengths = mysql_fetch_lengths(mysql_result);
  if (!mysql_row_lengths) {
    return false;
  }

  mysql_field_seek(mysql_result, 0);

  MYSQL_FIELD *mysql_field;
  int i;
  for (mysql_field = mysql_fetch_field(mysql_result), i = 0; mysql_field;
       mysql_field = mysql_fetch_field(mysql_result), i++) {
    Variant data;
    if (mysql_row[i]) {
      data = mysql_makevalue(String(mysql_row[i], mysql_row_lengths[i],
                                    CopyString), mysql_field);
    }
    if (result_type & MYSQL_NUM) {
      ret.set(i, data);
    }
    if (result_type & MYSQL_ASSOC) {
      ret.set(String(mysql_field->name, CopyString), data);
    }
  }
  return ret;
}

/* The mysql_*_nonblocking calls are Facebook extensions to
   libmysqlclient; for now, protect with an ifdef.  Once open sourced,
   the client will be detectable via its own ifdef. */
#ifdef FACEBOOK

const int64_t k_ASYNC_OP_INVALID = 0;
const int64_t k_ASYNC_OP_UNSET = ASYNC_OP_UNSET;
const int64_t k_ASYNC_OP_CONNECT = ASYNC_OP_CONNECT;
const int64_t k_ASYNC_OP_QUERY = ASYNC_OP_QUERY;

bool MySQL::async_connect(const String& host, int port, const String& socket,
                          const String& username, const String& password,
                          const String& database) {
  if (m_conn == NULL) {
    m_conn = create_new_conn();
  }
  if (RuntimeOption::EnableStats && RuntimeOption::EnableSQLStats) {
    ServerStats::Log("sql.conn", 1);
  }
  IOStatusHelper io("mysql::async_connect", host.data(), port);
  m_xaction_count = 0;
  m_host = static_cast<std::string>(host);
  m_username = static_cast<std::string>(username);
  m_password = static_cast<std::string>(password);
  m_socket = static_cast<std::string>(socket);
  m_database = static_cast<std::string>(database);
  if (!mysql_real_connect_nonblocking_init(
        m_conn, m_host.c_str(), m_username.c_str(), m_password.c_str(),
        (m_database.empty() ? NULL : m_database.c_str()), port,
        m_socket.empty() ? NULL : m_socket.c_str(), CLIENT_INTERACTIVE)) {
    return false;
  }
  return true;
}

Variant f_mysql_async_connect_start(const String& server /* = null_string */,
                                    const String& username /* = null_string */,
                                    const String& password /* = null_string */,
                                    const String& database /* = null_string */) {
  return php_mysql_do_connect(server, username, password, database,
                              0, false, true, 0, 0);
}

bool f_mysql_async_connect_completed(CVarRef link_identifier) {
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

bool f_mysql_async_query_start(const String& query, CVarRef link_identifier) {
  MYSQL* conn = MySQL::GetConn(link_identifier);
  if (!conn) {
    return false;
  }

  if (conn->async_op_status != ASYNC_OP_UNSET) {
    raise_warning("runtime/ext_mysql: attempt to run async query while async "
                  "operation already pending");
    return false;
  }
  Variant ret = php_mysql_do_query_general(query, link_identifier, true, true);
  if (ret.getRawType() != KindOfBoolean) {
    raise_warning("runtime/ext_mysql: unexpected return from "
                  "php_mysql_do_query_general");
    return false;
  }
  return ret.toBooleanVal();
}

Variant f_mysql_async_query_result(CVarRef link_identifier) {
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

bool f_mysql_async_query_completed(CVarRef result) {
  MySQLResult *res = result.toResource().getTyped<MySQLResult>
    (!RuntimeOption::ThrowBadTypeExceptions,
     !RuntimeOption::ThrowBadTypeExceptions);
  return !res || res->get() == NULL;
}

Variant f_mysql_async_fetch_array(CVarRef result, int result_type /* = 1 */) {
  if ((result_type & MYSQL_BOTH) == 0) {
    throw_invalid_argument("result_type: %d", result_type);
    return false;
  }

  MySQLResult* res = get_result(result);
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
    if (result_type & MYSQL_NUM) {
      ret.set(i, data);
    }
    if (result_type & MYSQL_ASSOC) {
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
Variant f_mysql_async_wait_actionable(CVarRef items, double timeout) {
  size_t count = items.toArray().size();
  if (count == 0 || timeout < 0) {
    return Array::Create();
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
      return Array::Create();
    }

    MySQL* mySQL = entry.rvalAt(0).toResource().getTyped<MySQL>();
    MYSQL* conn = mySQL->get();
    if (conn->async_op_status == ASYNC_OP_UNSET) {
      raise_warning("runtime/ext_mysql: no pending async operation in "
                    "progress");
      return Array::Create();
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
    return Array::Create();
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
      return Array::Create();
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

int64_t f_mysql_async_status(CVarRef link_identifier) {
  MySQL *mySQL = MySQL::Get(link_identifier);
  if (!mySQL || !mySQL->get()) {
    raise_warning("supplied argument is not a valid MySQL-Link resource");
    return -1;
  }

  return mySQL->get()->async_op_status;
}

#else  // FACEBOOK

// Bogus values for non-facebook libmysqlclients.
const int64_t k_ASYNC_OP_INVALID = 0;
const int64_t k_ASYNC_OP_UNSET = -1;
const int64_t k_ASYNC_OP_CONNECT = -2;
const int64_t k_ASYNC_OP_QUERY = -3;

Variant f_mysql_async_connect_start(const String& server,
                                    const String& username,
                                    const String& password,
                                    const String& database) {
  throw NotImplementedException(__func__);
}

bool f_mysql_async_connect_completed(CVarRef link_identifier) {
  throw NotImplementedException(__func__);
}

bool f_mysql_async_query_start(const String& query, CVarRef link_identifier) {
  throw NotImplementedException(__func__);
}

Variant f_mysql_async_query_result(CVarRef link_identifier) {
  throw NotImplementedException(__func__);
}

bool f_mysql_async_query_completed(CVarRef result) {
  throw NotImplementedException(__func__);
}

Variant f_mysql_async_fetch_array(CVarRef result, int result_type /* = 1 */) {
  throw NotImplementedException(__func__);
}

Variant f_mysql_async_wait_actionable(CVarRef items, double timeout) {
  throw NotImplementedException(__func__);
}

int64_t f_mysql_async_status(CVarRef link_identifier) {
  throw NotImplementedException(__func__);
}

#endif

Variant f_mysql_fetch_row(CVarRef result) {
  return php_mysql_fetch_hash(result, MYSQL_NUM);
}

Variant f_mysql_fetch_assoc(CVarRef result) {
  return php_mysql_fetch_hash(result, MYSQL_ASSOC);
}

Variant f_mysql_fetch_array(CVarRef result, int result_type /* = 3 */) {
  return php_mysql_fetch_hash(result, result_type);
}

Variant f_mysql_fetch_object(CVarRef result,
                             const String& class_name /* = "stdClass" */,
                             CArrRef params /* = null */) {
  Variant properties = php_mysql_fetch_hash(result, MYSQL_ASSOC);
  if (!same(properties, false)) {
    Object obj = create_object(class_name, params);
    obj->o_setArray(properties.toArray());

    return obj;
  }
  return false;
}

Variant f_mysql_fetch_lengths(CVarRef result) {
  MySQLResult *res = get_result(result);
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

Variant f_mysql_result(CVarRef result, int row,
                       CVarRef field /* = null_variant */) {
  MySQLResult *res = get_result(result);
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


Variant f_mysql_db_name(CVarRef result, int row,
                        CVarRef field /* = null_variant */) {
  return f_mysql_result(result, row, field);
}
Variant f_mysql_tablename(CVarRef result, int i) {
  return f_mysql_result(result, i);
}

Variant f_mysql_num_fields(CVarRef result) {
  MySQLResult *res = get_result(result);
  if (res) {
    return res->getFieldCount();
  }
  return false;
}

Variant f_mysql_num_rows(CVarRef result) {
  MySQLResult *res = get_result(result);
  if (res) {
    return res->getRowCount();
  }
  return false;
}

Variant f_mysql_free_result(CVarRef result) {
  MySQLResult *res = get_result(result);
  if (res) {
    res->close();
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////
// field info

Variant f_mysql_fetch_field(CVarRef result, int field /* = -1 */) {
  MySQLResult *res = get_result(result);
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

bool f_mysql_field_seek(CVarRef result, int field /* = 0 */) {
  MySQLResult *res = get_result(result);
  if (res == NULL) return false;
  res->seekField(field);
  return true;
}

Variant f_mysql_field_name(CVarRef result, int field /* = 0 */) {
  return php_mysql_field_info(result, field, PHP_MYSQL_FIELD_NAME);
}
Variant f_mysql_field_table(CVarRef result, int field /* = 0 */) {
  return php_mysql_field_info(result, field, PHP_MYSQL_FIELD_TABLE);
}
Variant f_mysql_field_len(CVarRef result, int field /* = 0 */) {
  return php_mysql_field_info(result, field, PHP_MYSQL_FIELD_LEN);
}
Variant f_mysql_field_type(CVarRef result, int field /* = 0 */) {
  return php_mysql_field_info(result, field, PHP_MYSQL_FIELD_TYPE);
}
Variant f_mysql_field_flags(CVarRef result, int field /* = 0 */) {
  return php_mysql_field_info(result, field, PHP_MYSQL_FIELD_FLAGS);
}

///////////////////////////////////////////////////////////////////////////////
// MySQLResult

void MySQLResult::addRow() {
  m_row_count++;
  m_rows->push_back(smart::vector<Variant>());
  m_rows->back().reserve(getFieldCount());
}

void MySQLResult::addField(Variant&& value) {
  m_rows->back().push_back(std::move(value));
}

void MySQLResult::setFieldCount(int64_t fields) {
  m_field_count = fields;
  assert(!m_fields);
  m_fields = smart_new_array<MySQLFieldInfo>(fields);
}

void MySQLResult::setFieldInfo(int64_t f, MYSQL_FIELD *field) {
  MySQLFieldInfo &info = m_fields[f];
  info.name = String(field->name, CopyString);
  info.table = String(field->table, CopyString);
  info.def = String(field->def, CopyString);
  info.max_length = (int64_t)field->max_length;
  info.length = (int64_t)field->length;
  info.type = (int)field->type;
  info.flags = field->flags;
}

MySQLFieldInfo *MySQLResult::getFieldInfo(int64_t field) {
  if (field < 0 || field >= getFieldCount()) {
    return NULL;
  }

  if (!m_localized && !m_fields) {
    if (m_res->fields == NULL) return NULL;
    // cache field info
    setFieldCount(getFieldCount());
    for (int i = 0; i < getFieldCount(); i++) {
      setFieldInfo(i, m_res->fields + i);
    }
  }
  return m_fields + field;
}

Variant MySQLResult::getField(int64_t field) const {
  if (!m_localized || field < 0 || field >= (int64_t)m_current_row->size()) {
    return uninit_null();
  }
  return (*m_current_row)[field];
}

int64_t MySQLResult::getFieldCount() const {
  if (!m_localized) {
    return (int64_t)mysql_num_fields(m_res);
  }
  return m_field_count;
}

int64_t MySQLResult::getRowCount() const {
  if (!m_localized) {
    return (int64_t)mysql_num_rows(m_res);
  }
  return m_row_count;
}

bool MySQLResult::seekRow(int64_t row) {
  if (row < 0 || row >= getRowCount()) {
    raise_warning("Unable to jump to row %" PRId64 " on MySQL result index %d",
                    row, o_getId());
    return false;
  }

  if (!m_localized) {
    mysql_data_seek(m_res, (my_ulonglong)row);
  } else {
    m_current_row = m_rows->begin();
    for (int i = 0; i < row; i++) m_current_row++;
    m_row_ready = false;
  }
  return true;
}

bool MySQLResult::fetchRow() {
  if (m_current_row != m_rows->end()) m_current_row++;
  if (m_current_row != m_rows->end()) {
    m_row_ready = true;
    return true;
  }
  return false;
}

bool MySQLResult::seekField(int64_t field) {
  if (field < 0 || field >= getFieldCount()) {
    raise_warning("Field %" PRId64 " is invalid for MySQL result index %d",
                    field, o_getId());
    return false;
  }

  if (!m_localized) {
    mysql_field_seek(m_res, (MYSQL_FIELD_OFFSET)field);
  }
  m_current_field = field - 1;
  return true;
}

MySQLFieldInfo *MySQLResult::fetchFieldInfo() {
  if (!m_localized) {
    mysql_fetch_field(m_res);
  }
  if (m_current_field < getFieldCount()) m_current_field++;
  return getFieldInfo(m_current_field);
}

///////////////////////////////////////////////////////////////////////////////
}
