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

#include "hphp/runtime/ext/mysql/mysql_common.h"

#include <algorithm>
#include <cassert>
#include <unordered_set>
#include <vector>

#include <folly/Conv.h>
#include <folly/ScopeGuard.h>
#include <folly/String.h>
#include <folly/portability/Sockets.h>

#include "hphp/util/network.h"
#include "hphp/util/text-util.h"
#include "hphp/util/timer.h"

#include "hphp/system/systemlib.h"

#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/extended-logger.h"
#include "hphp/runtime/base/preg.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/socket.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/vm/jit/translator-inline.h"

#include "hphp/runtime/ext/mysql/ext_mysql.h"
#include "hphp/runtime/ext/mysql/mysql_stats.h"
#include "hphp/runtime/ext/pcre/ext_pcre.h"
#include "hphp/runtime/ext/std/ext_std_network.h"
#include "hphp/runtime/server/server-stats.h"

#include "hphp/runtime/ext/async_mysql/ext_async_mysql.h"
#include "hphp/runtime/vm/native-data.h"

namespace HPHP {

using facebook::common::mysql_client::SSLOptionsProviderBase;

const StaticString s_mysqli_result("mysqli_result");

struct MySQLStaticInitializer {
  MySQLStaticInitializer() {
    mysql_library_init(0, NULL, NULL);
  }
};
static MySQLStaticInitializer s_mysql_initializer;

///////////////////////////////////////////////////////////////////////////////

int MySQLUtil::set_mysql_timeout(MYSQL *mysql,
                                 MySQLUtil::TimeoutType type,
                                 int ms) {
#ifdef __APPLE__
  // Work around a bug in webscalesql where setting a read or write timeout
  // causes most mysql connections to fail (depending on the exact timing of
  // packets). See https://github.com/webscalesql/webscalesql-5.6/issues/23
  return 0;
#endif

  mysql_option opt = MYSQL_OPT_CONNECT_TIMEOUT;
#ifdef MYSQL_MILLISECOND_TIMEOUT
  switch (type) {
   case MySQLUtil::ConnectTimeout: opt = MYSQL_OPT_CONNECT_TIMEOUT_MS; break;
   case MySQLUtil::ReadTimeout: opt =  MYSQL_OPT_READ_TIMEOUT_MS; break;
   case MySQLUtil::WriteTimeout: opt =  MYSQL_OPT_WRITE_TIMEOUT_MS; break;
   default: assert(false); break;
  }
#else
  switch (type) {
    case MySQLUtil::ConnectTimeout: opt = MYSQL_OPT_CONNECT_TIMEOUT; break;
    case MySQLUtil::ReadTimeout: opt =  MYSQL_OPT_READ_TIMEOUT; break;
    case MySQLUtil::WriteTimeout: opt =  MYSQL_OPT_WRITE_TIMEOUT; break;
    default: assert(false); break;
  }
  ms = (ms + 999) / 1000;
#endif

  return mysql_options(mysql, opt, (const char*)&ms);
}

///////////////////////////////////////////////////////////////////////////////

void MySQLRequestData::requestInit() {
  defaultConn.reset();
  readTimeout = mysqlExtension::ReadTimeout;
  totalRowCount = 0;
}

IMPLEMENT_STATIC_REQUEST_LOCAL(MySQLRequestData, s_mysql_data);

///////////////////////////////////////////////////////////////////////////////
// class MySQL statics

int MySQL::s_default_port = 0;
bool MySQL::s_allow_reconnect = false;
bool MySQL::s_allow_persistent = true;
int MySQL::s_cur_num_persistent = 0;
int MySQL::s_max_num_persistent = -1;

std::shared_ptr<MySQL> MySQL::Get(const Variant& link_identifier) {
  if (link_identifier.isNull()) {
    return GetDefaultConn();
  }
  auto res = dyn_cast_or_null<MySQLResource>(link_identifier);
  return res ? res->mysql() : nullptr;
}

MYSQL* MySQL::GetConn(const Variant& link_identifier,
                      std::shared_ptr<MySQL>* rconn /* = nullptr */) {
  auto mySQL = Get(link_identifier);
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

bool MySQL::CloseConn(const Variant& link_identifier) {
  auto mySQL = Get(link_identifier);
  if (mySQL) {
    if (!mySQL->isPersistent()) {
      mySQL->close();
    } else {
      s_cur_num_persistent--;
    }
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
      Variant ret = HHVM_FN(getservbyname)("mysql", "tcp");
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

std::string MySQL::GetHash(const String& host, int port, const String& socket,
                           const String& username, const String& password,
                           int client_flags) {
  char buf[1024];
  snprintf(buf, sizeof(buf), "%s:%d:%s:%s:%s:%d",
           host.data(), port, socket.data(),
           username.data(), password.data(), client_flags);
  return std::string(buf);
}

namespace {
thread_local std::unordered_map<std::string,
                                std::shared_ptr<MySQL>> s_connections;
}

std::shared_ptr<MySQL> MySQL::GetCachedImpl(const String& host, int port,
                                            const String& socket,
                                            const String& username,
                                            const String& password,
                                            int client_flags) {
  auto key = GetHash(host, port, socket, username, password, client_flags);
  return s_connections[key];
}

void MySQL::SetCachedImpl(const String& host, int port,
                          const String& socket,
                          const String& username,
                          const String& password,
                          int client_flags,
                          std::shared_ptr<MySQL> conn) {
  auto key = GetHash(host, port, socket, username, password, client_flags);
  s_connections[key] = conn;
}

size_t MySQL::NumCachedConnections() {
  return s_connections.size();
}

std::shared_ptr<MySQL> MySQL::GetDefaultConn() {
  if (s_mysql_data->defaultConn == nullptr) {
    return nullptr;
  }
  return s_mysql_data->defaultConn->mysql();
}

void MySQL::SetDefaultConn(std::shared_ptr<MySQL> conn) {
  s_mysql_data->defaultConn = req::make<MySQLResource>(std::move(conn));
}

int MySQL::GetDefaultReadTimeout() {
  return s_mysql_data->readTimeout;
}

void MySQL::SetDefaultReadTimeout(int timeout_ms) {
  if (timeout_ms < 0) {
    timeout_ms = mysqlExtension::ReadTimeout;
  }
  s_mysql_data->readTimeout = timeout_ms;
}

///////////////////////////////////////////////////////////////////////////////
// class MySQL

namespace {

MYSQL* configure_conn(MYSQL* conn) {
  mysql_options(conn, MYSQL_OPT_LOCAL_INFILE, 0);
  if (mysqlExtension::ConnectTimeout) {
    MySQLUtil::set_mysql_timeout(conn, MySQLUtil::ConnectTimeout,
                                 mysqlExtension::ConnectTimeout);
  }
  int readTimeout = MySQL::GetDefaultReadTimeout();
  if (readTimeout) {
    MySQLUtil::set_mysql_timeout(conn, MySQLUtil::ReadTimeout, readTimeout);
    MySQLUtil::set_mysql_timeout(conn, MySQLUtil::WriteTimeout, readTimeout);
  }
  return conn;
}

MYSQL* create_new_conn() {
  return configure_conn(mysql_init(nullptr));
}

}

MySQL::MySQL(const char *host, int port, const char *username,
             const char *password, const char *database,
             MYSQL* raw_connection)
  : m_port(port)
  , m_last_error_set(false)
  , m_last_errno(0)
  , m_xaction_count(0)
  , m_multi_query(false)
  , m_state(MySQLState::INITED)
{
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
  m_state = MySQLState::CLOSED;
}

bool MySQL::connect(const String& host, int port, const String& socket,
                    const String& username, const String& password,
                    const String& database, int client_flags,
                    int connect_timeout) {
  if (m_conn == nullptr) {
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
  if (m_host.empty()) m_host = static_cast<std::string>(host);
  if (m_username.empty()) m_username = static_cast<std::string>(username);
  if (m_password.empty()) m_password = static_cast<std::string>(password);
  if (m_socket.empty()) m_socket = static_cast<std::string>(socket);
  if (m_database.empty()) m_database = static_cast<std::string>(database);
  if (!m_port) m_port = port;
  bool ret = mysql_real_connect(m_conn, host.data(), username.data(),
                            password.data(),
                            (database.empty() ? nullptr : database.data()),
                            port,
                            socket.empty() ? nullptr : socket.data(),
                            client_flags);
  if (ret && mysqlExtension::WaitTimeout > 0) {
    String query("set session wait_timeout=");
    query += String((int64_t)(mysqlExtension::WaitTimeout / 1000));
    if (mysql_real_query(m_conn, query.data(), query.size())) {
      raise_notice("MySQL::connect: failed setting session wait timeout: %s",
                   mysql_error(m_conn));
    }
  }
  m_state = (ret) ? MySQLState::CONNECTED : MySQLState::CLOSED;
  return ret;
}

bool MySQL::reconnect(const String& host, int port, const String& socket,
                      const String& username, const String& password,
                      const String& database, int client_flags,
                      int connect_timeout) {
  bool ret = false;
  if (m_conn == nullptr) {
    m_conn = create_new_conn();
    if (connect_timeout >= 0) {
      MySQLUtil::set_mysql_timeout(m_conn, MySQLUtil::ConnectTimeout,
                                   connect_timeout);
    }
    if (RuntimeOption::EnableStats && RuntimeOption::EnableSQLStats) {
      ServerStats::Log("sql.reconn_new", 1);
    }
    IOStatusHelper io("mysql::connect", host.data(), port);
    ret = mysql_real_connect(m_conn, host.data(), username.data(),
                             password.data(),
                             (database.empty() ? nullptr : database.data()),
                             port, socket.data(), client_flags);
  } else if (m_state == MySQLState::CONNECTED && !mysql_ping(m_conn)) {
    if (RuntimeOption::EnableStats && RuntimeOption::EnableSQLStats) {
      ServerStats::Log("sql.reconn_ok", 1);
    }
    if (!database.empty()) {
      mysql_select_db(m_conn, database.data());
    }
    return true;
  } else {
    if (connect_timeout >= 0) {
      MySQLUtil::set_mysql_timeout(m_conn, MySQLUtil::ConnectTimeout,
                                   connect_timeout);
    }
    if (RuntimeOption::EnableStats && RuntimeOption::EnableSQLStats) {
      ServerStats::Log("sql.reconn_old", 1);
    }
    IOStatusHelper io("mysql::connect", host.data(), port);
    m_xaction_count = 0;
    ret = mysql_real_connect(m_conn, host.data(), username.data(),
                             password.data(),
                             (database.empty() ? nullptr : database.data()),
                             port, socket.data(), client_flags);
  }

  m_state = (ret) ? MySQLState::CONNECTED : MySQLState::CLOSED;
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// MySQLResource

IMPLEMENT_RESOURCE_ALLOCATION(MySQLResource)

///////////////////////////////////////////////////////////////////////////////
// helpers

namespace {

template <typename T>
req::ptr<MySQLResult> php_mysql_extract_result_helper(const T& result) {
  auto const res = dyn_cast_or_null<MySQLResult>(result);
  if (res == nullptr || res->isInvalid()) {
    raise_warning("supplied argument is not a valid MySQL result resource");
    return nullptr;
  }
  return res;
}

}

req::ptr<MySQLResult> php_mysql_extract_result(const Resource& result) {
  return php_mysql_extract_result_helper(result);
}

req::ptr<MySQLResult> php_mysql_extract_result(const Variant& result) {
  return php_mysql_extract_result_helper(result);
}

const char *php_mysql_get_field_name(int field_type) {
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

Variant php_mysql_field_info(const Resource& result, int field,
                             int entry_type) {
  auto res = php_mysql_extract_result(result);
  if (!res) return false;

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

Variant php_mysql_do_connect(
    const String& server,
    const String& username,
    const String& password,
    const String& database,
    int client_flags,
    bool persistent,
    bool async,
    int connect_timeout_ms,
    int query_timeout_ms) {
  return php_mysql_do_connect_on_link(
      nullptr,
      server,
      username,
      password,
      database,
      client_flags,
      persistent,
      async,
      connect_timeout_ms,
      query_timeout_ms);
}

Variant php_mysql_do_connect_with_ssl(
    const String& server,
    const String& username,
    const String& password,
    const String& database,
    int client_flags,
    int connect_timeout_ms,
    int query_timeout_ms,
    const Variant& sslContextProvider /* = null */) {
  std::shared_ptr<SSLOptionsProviderBase> ssl_provider;
  if (!sslContextProvider.isNull()) {
    auto* obj =
        Native::data<HPHP::MySSLContextProvider>(sslContextProvider.toObject());
    ssl_provider = obj->getSSLProvider();
  }

  return php_mysql_do_connect_on_link(
      nullptr,
      server,
      username,
      password,
      database,
      client_flags,
      false,
      false,
      connect_timeout_ms,
      query_timeout_ms,
      ssl_provider);
}

static void mysql_set_ssl_options(
    std::shared_ptr<MySQL> mySQL,
    std::shared_ptr<SSLOptionsProviderBase> ssl_provider) {
  if (!ssl_provider || !mySQL || mySQL->get() == nullptr) {
    return;
  }
  ssl_provider->setMysqlSSLOptions(mySQL->get());
}

static void mysql_store_ssl_session(
    std::shared_ptr<MySQL> mySQL,
    std::shared_ptr<SSLOptionsProviderBase> ssl_provider) {
  if (!ssl_provider || !mySQL || mySQL->get() == nullptr) {
    return;
  }
  ssl_provider->storeMysqlSSLSession(mySQL->get());
}

Variant php_mysql_do_connect_on_link(
    std::shared_ptr<MySQL> mySQL,
    String server,
    String username,
    String password,
    String database,
    int client_flags,
    bool persistent,
    bool async,
    int connect_timeout_ms,
    int query_timeout_ms,
    std::shared_ptr<SSLOptionsProviderBase> ssl_provider) {
  if (connect_timeout_ms < 0) {
    connect_timeout_ms = mysqlExtension::ConnectTimeout;
  }
  if (query_timeout_ms < 0) {
    query_timeout_ms = MySQL::GetDefaultReadTimeout();
  }
  if (server.empty()) server = MySQL::GetDefaultServer();
  if (username.empty()) username = MySQL::GetDefaultUsername();
  if (password.empty()) password = MySQL::GetDefaultPassword();
  if (database.empty()) database = MySQL::GetDefaultDatabase();

  // server format: hostname[:port][:/path/to/socket]
  // ipv6 hostname:port is of the form [1:2:3:4:5]:port
  String host, socket;
  int port;
  int savePersistent = false;

  auto slash_pos = server.find('/');
  if (slash_pos != std::string::npos) {
    socket = server.substr(slash_pos);
    server = server.substr(0, slash_pos - 1);
  }

  HostURL hosturl(std::string(server), MySQL::GetDefaultPort());
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

  if (MySQL::IsAllowPersistent() &&
      MySQL::GetCurrentNumPersistent() < MySQL::GetMaxNumPersistent() &&
      persistent) {
    auto p_mySQL = MySQL::GetPersistent(host, port, socket, username,
                                        password, client_flags);

    if (p_mySQL != nullptr) {
      mySQL = p_mySQL;
    } else {
      savePersistent = true;
    }
  }

  if (mySQL == nullptr) {
    mySQL = std::make_shared<MySQL>(
        host.c_str(),
        port,
        username.c_str(),
        password.c_str(),
        database.c_str());
  }

  // set SSL Options
  mysql_set_ssl_options(mySQL, ssl_provider);

  if (mySQL->getState() == MySQLState::INITED) {
    if (async) {
#ifdef FACEBOOK
      if (!mySQL->async_connect(host, port, socket, username, password,
                                database)) {
        MySQL::SetDefaultConn(mySQL); // so we can report errno by mysql_errno()
        mySQL->setLastError("mysql_real_connect_nonblocking_init");
        return false;
      }
#else
      throw_not_implemented("mysql_async_connect_start");
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
    if (!MySQL::IsAllowReconnect()) {
      raise_warning("MySQL: Reconnects are not allowed");
      return false;
    }
    if (!mySQL->reconnect(host, port, socket, username, password,
                          database, client_flags, connect_timeout_ms)) {
      MySQL::SetDefaultConn(mySQL); // so we can report errno by mysql_errno()
      mySQL->setLastError("mysql_connect");
      return false;
    }
  }

  // store SSL Session
  mysql_store_ssl_session(mySQL, ssl_provider);

  if (savePersistent) {
    MySQL::SetPersistent(
        host, port, socket, username, password, client_flags, mySQL);
    MySQL::SetCurrentNumPersistent(MySQL::GetCurrentNumPersistent() + 1);
  }
  MySQL::SetDefaultConn(mySQL);
  return Variant(req::make<MySQLResource>(mySQL));
}

///////////////////////////////////////////////////////////////////////////////
// MySQLResult

MySQLResult::MySQLResult(MYSQL_RES *res, bool localized /* = false */)
  : m_res(res)
  , m_current_async_row(nullptr)
  , m_localized(localized)
  , m_current_field(-1)
  , m_conn(nullptr)
{
  if (localized) {
    m_res = nullptr; // ensure that localized results don't have another result
    m_rows = req::list<req::vector<Variant>>(1); // sentinel
    m_current_row = m_rows->begin();
    m_row_ready = false;
    m_row_count = 0;
  }
}

MySQLResult::~MySQLResult() {
  close();
  if (m_conn) {
    m_conn = nullptr;
  }
}

void MySQLResult::sweep() {
  if (m_res) {
    mysql_free_result(m_res);
    m_res = nullptr;
  }
}

void MySQLResult::addRow() {
  m_row_count++;
  m_rows->push_back(req::vector<Variant>());
  m_rows->back().reserve(getFieldCount());
}

void MySQLResult::addField(Variant&& value) {
  m_rows->back().push_back(std::move(value));
}

void MySQLResult::setFieldCount(int64_t fields) {
  assert(m_fields.empty());
  m_fields.resize(fields);
}

void MySQLResult::setFieldInfo(int64_t f, MYSQL_FIELD *field) {
  MySQLFieldInfo &info = m_fields[f];
  info.name       = String(field->name, CopyString);
  info.org_name   = String(field->org_name, CopyString);
  info.table      = String(field->table, CopyString);
  info.org_table  = String(field->org_table, CopyString);
  info.def        = String(field->def, CopyString);
  info.db         = String(field->db, CopyString);
  info.max_length = (int64_t)field->max_length;
  info.length     = (int64_t)field->length;
  info.type       = (int)field->type;
  info.flags      = field->flags;
  info.decimals   = field->decimals;
  info.charsetnr  = field->charsetnr;
}

MySQLFieldInfo *MySQLResult::getFieldInfo(int64_t field) {
  if (field < 0 || field >= getFieldCount()) {
    return NULL;
  }

  if (!m_localized && m_fields.empty()) {
    if (m_res->fields == NULL) return NULL;
    // cache field info
    setFieldCount(getFieldCount());
    for (int i = 0; i < getFieldCount(); i++) {
      setFieldInfo(i, m_res->fields + i);
    }
  }
  return &m_fields[field];
}

Variant MySQLResult::getField(int64_t field) const {
  if (!m_localized || field < 0 || field >= (int64_t)m_current_row->size()) {
    return init_null();
  }
  return (*m_current_row)[field];
}

int64_t MySQLResult::getFieldCount() const {
  if (!m_localized) {
    return (int64_t)mysql_num_fields(m_res);
  }
  return m_fields.size();
}

int64_t MySQLResult::getRowCount() const {
  if (!m_localized) {
    return (int64_t)mysql_num_rows(m_res);
  }
  return m_row_count;
}

bool MySQLResult::seekRow(int64_t row) {
  if (row < 0 || row >= getRowCount()) {
    if (row != 0) {
      raise_warning("Unable to jump to row %"
                      PRId64 " on MySQL result index %d",
                      row, getId());
    }
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
  // If not localized, use standard mysql functions on m_res
  assert(isLocalized());
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
                    field, getId());
    return false;
  }

  if (!m_localized) {
    mysql_field_seek(m_res, (MYSQL_FIELD_OFFSET)field);
  }
  m_current_field = field - 1;
  return true;
}

int64_t MySQLResult::tellField() {
  if (!m_localized) {
    return mysql_field_tell(m_res);
  }
  return m_current_field;
}

MySQLFieldInfo *MySQLResult::fetchFieldInfo() {
  if (!m_localized) {
    mysql_fetch_field(m_res);
  }
  if (m_current_field < getFieldCount()) m_current_field++;
  return getFieldInfo(m_current_field);
}


///////////////////////////////////////////////////////////////////////////////
// MySQLStmtVariables

MySQLStmtVariables::MySQLStmtVariables(const Array& arr): m_arr(arr) {
  int count = m_arr.size();
  m_vars   = req::calloc_raw_array<MYSQL_BIND>(count);
  m_null   = req::calloc_raw_array<my_bool>(count);
  m_length = req::calloc_raw_array<unsigned long>(count);

  for (int i = 0; i < count; i++) {
    m_null[i] = false;
    m_length[i] = 0;

    MYSQL_BIND *b = &m_vars[i];
    b->is_null = &m_null[i];
    b->length  = &m_length[i];
    b->buffer = nullptr;
    b->buffer_length = 0;
    b->buffer_type = MYSQL_TYPE_STRING;
  }
}

MySQLStmtVariables::~MySQLStmtVariables() {
  for (int i = 0; i < m_arr.size(); i++) {
    auto buf = &m_vars[i];
    if (buf->buffer_length > 0) {
      req::free(buf->buffer);
    }
  }

  req::free(m_vars);
  req::free(m_null);
  req::free(m_length);
}

bool MySQLStmtVariables::bind_result(MYSQL_STMT *stmt) {
  assert(m_arr.size() == mysql_stmt_field_count(stmt));

  MYSQL_RES *res = mysql_stmt_result_metadata(stmt);
  MYSQL_FIELD *fields = mysql_fetch_fields(res);
  for(int i = 0; i < m_arr.size(); i++) {
    MYSQL_BIND *b = &m_vars[i];
    b->is_unsigned = (fields[i].flags & UNSIGNED_FLAG) ? 1 : 0;

    switch (fields[i].type) {
      case MYSQL_TYPE_NULL:
        b->buffer_type = MYSQL_TYPE_NULL;
      case MYSQL_TYPE_DOUBLE:
      case MYSQL_TYPE_FLOAT:
        b->buffer_type = MYSQL_TYPE_DOUBLE;
        b->buffer_length = sizeof(double);
        break;
      case MYSQL_TYPE_LONGLONG:
#if MYSQL_VERSION_ID > 50002
      case MYSQL_TYPE_BIT:
#endif
      case MYSQL_TYPE_LONG:
      case MYSQL_TYPE_INT24:
      case MYSQL_TYPE_SHORT:
      case MYSQL_TYPE_YEAR:
      case MYSQL_TYPE_TINY:
        b->buffer_type = MYSQL_TYPE_LONGLONG;
        b->buffer_length = sizeof(int64_t);
        break;
      case MYSQL_TYPE_DATE:
      case MYSQL_TYPE_NEWDATE:
      case MYSQL_TYPE_DATETIME:
      case MYSQL_TYPE_TIMESTAMP:
      case MYSQL_TYPE_TIME:
      case MYSQL_TYPE_STRING:
      case MYSQL_TYPE_VARCHAR:
      case MYSQL_TYPE_VAR_STRING:
      case MYSQL_TYPE_ENUM:
      case MYSQL_TYPE_SET:
      case MYSQL_TYPE_LONG_BLOB:
      case MYSQL_TYPE_MEDIUM_BLOB:
      case MYSQL_TYPE_BLOB:
      case MYSQL_TYPE_TINY_BLOB:
      case MYSQL_TYPE_GEOMETRY:
      case MYSQL_TYPE_DECIMAL:
      case MYSQL_TYPE_NEWDECIMAL:
        b->buffer_type = MYSQL_TYPE_STRING;
        b->buffer_length = fields[i].max_length ?
                             fields[i].max_length :
                             fields[i].length;
        break;
      default:
        // There exists some more types in this enum like MYSQL_TYPE_TIMESTAMP2,
        // MYSQL_TYPE_DATETIME2, MYSQL_TYPE_TIME2 but they are just used on the
        // server
        assert(false);
    }

    if (b->buffer_length > 0) {
      b->buffer = req::calloc(b->buffer_length, 1);
    }
  }
  mysql_free_result(res);

  return !mysql_stmt_bind_result(stmt, m_vars);
}

void MySQLStmtVariables::update_result() {
  for (int i = 0; i < m_arr.size(); i++) {
    MYSQL_BIND *b = &m_vars[i];
    Variant v;

    if (!*b->is_null && b->buffer_type != MYSQL_TYPE_NULL) {
      switch (b->buffer_type) {
        case MYSQL_TYPE_DOUBLE:
          v = *(double*)b->buffer;
          break;
        case MYSQL_TYPE_LONGLONG:
          v = *(int64_t*)b->buffer;
          break;
        case MYSQL_TYPE_STRING:
          v = String((char *)b->buffer, *b->length, CopyString);
          break;
        default:
          // We never ask for anything else than DOUBLE, LONGLONG and STRING
          // so in the case we get something else back something is really wrong
          assert(false);
      }
    }

    *m_arr.lvalAt(i).getRefData() = v;
  }
}

bool MySQLStmtVariables::init_params(MYSQL_STMT *stmt, const String& types) {
  assert(m_arr.size() == types.size());

  for (int i = 0; i < types.size(); i++) {
    MYSQL_BIND *b = &m_vars[i];
    switch (types[i]) {
      case 'i':
        b->buffer_type = MYSQL_TYPE_LONGLONG;
        break;
      case 'd':
        b->buffer_type = MYSQL_TYPE_DOUBLE;
        break;
      case 's':
        b->buffer_type = MYSQL_TYPE_STRING;
        break;
      case 'b':
        b->buffer_type = MYSQL_TYPE_LONG_BLOB;
        break;
      default:
        assert(false);
    }
  }

  return !mysql_stmt_bind_param(stmt, m_vars);
}

bool MySQLStmtVariables::bind_params(MYSQL_STMT *stmt) {
  m_value_arr.clear();
  for (int i = 0; i < m_arr.size(); i++) {
    MYSQL_BIND *b = &m_vars[i];
    auto const& var = m_arr.lvalAt(i);
    Variant v;
    if (var.isNull()) {
      *b->is_null = 1;
    } else {
      switch (b->buffer_type) {
        case MYSQL_TYPE_LONGLONG:
          {
            m_value_arr.push_back(var.toInt64());
            b->buffer = m_value_arr.back().getInt64Data();
          }
          break;
        case MYSQL_TYPE_DOUBLE:
          {
            m_value_arr.push_back(var.toDouble());
            b->buffer = m_value_arr.back().getDoubleData();
          }
          break;
        case MYSQL_TYPE_STRING:
          {
            m_value_arr.push_back(var.toString());
            StringData *sd = m_value_arr.back().getStringData();
            b->buffer = (void *)sd->data();
            // FIXME: setting buffer_length will cause the destructor to free
            // memory owned by the string
            *b->length = sd->size();
          }
          break;
        case MYSQL_TYPE_LONG_BLOB:
          // The value are set using send_long_data so we don't have to do
          // anything here
          break;
        default:
          assert(false);
      }
    }
  }

  return !mysql_stmt_bind_param(stmt, m_vars);
}

///////////////////////////////////////////////////////////////////////////////
// MySQLStmt

#define VALIDATE_STMT                                                          \
  if (!m_stmt) {                                                               \
    raise_warning("Couldn't fetch mysqli_stmt");                               \
    return init_null();                                                        \
  }

#define VALIDATE_PREPARED                                                      \
  VALIDATE_STMT                                                                \
  if (!m_prepared) {                                                           \
    raise_warning("invalid object or resource");                               \
    return init_null();                                                        \
  }

MySQLStmt::MySQLStmt(MYSQL *mysql)
  : m_stmt(mysql_stmt_init(mysql)), m_prepared(false)
{}

MySQLStmt::~MySQLStmt() {
  close();
}

void MySQLStmt::sweep() {
  close();
  // Note that ~MySQLStmt is *not* going to run when we are swept.
}

Variant MySQLStmt::affected_rows() {
  VALIDATE_PREPARED
  return (int64_t)mysql_stmt_affected_rows(m_stmt);
}

Variant MySQLStmt::attr_get(int64_t attr) {
  VALIDATE_PREPARED

  int64_t value = 0;

  if (mysql_stmt_attr_get(m_stmt, (enum_stmt_attr_type)attr, &value)) {
    return false;
  }

#if MYSQL_VERSION_ID >= 50107
  if ((enum_stmt_attr_type)attr == STMT_ATTR_UPDATE_MAX_LENGTH) {
    value = *(my_bool *)&value;
  }
#endif

  return value;
}

Variant MySQLStmt::attr_set(int64_t attr, int64_t value) {
  VALIDATE_PREPARED

#if MYSQL_VERSION_ID >= 50107
  if ((enum_stmt_attr_type)attr == STMT_ATTR_UPDATE_MAX_LENGTH) {
    value = (my_bool)value;
  }
#endif
  return !mysql_stmt_attr_set(m_stmt, (enum_stmt_attr_type)attr, &value);
}

Variant MySQLStmt::bind_param(const String& types, const Array& vars) {
  VALIDATE_PREPARED

  m_param_vars = req::make_unique<MySQLStmtVariables>(vars);
  return m_param_vars->init_params(m_stmt, types);
}

Variant MySQLStmt::bind_result(const Array& vars) {
  VALIDATE_PREPARED

  m_result_vars = req::make_unique<MySQLStmtVariables>(vars);
  return m_result_vars->bind_result(m_stmt);
}

Variant MySQLStmt::data_seek(int64_t offset) {
  VALIDATE_PREPARED

  mysql_stmt_data_seek(m_stmt, offset);
  return init_null();
}

Variant MySQLStmt::get_errno() {
  VALIDATE_STMT
  return (int64_t)mysql_stmt_errno(m_stmt);
}

Variant MySQLStmt::get_error() {
  VALIDATE_STMT
  return String(mysql_stmt_error(m_stmt), CopyString);
}

Variant MySQLStmt::close() {
  m_prepared = false;
  if (m_stmt) {
    bool ret = !mysql_stmt_close(m_stmt);
    m_stmt = nullptr;
    return ret;
  }

  return true;
}

Variant MySQLStmt::execute() {
  VALIDATE_PREPARED

  if (m_param_vars) {
    m_param_vars->bind_params(m_stmt);
  }

  return !mysql_stmt_execute(m_stmt);
}

Variant MySQLStmt::fetch() {
  VALIDATE_PREPARED

  int64_t ret = mysql_stmt_fetch(m_stmt);

  if (ret == MYSQL_DATA_TRUNCATED || ret == MYSQL_NO_DATA) {
    return init_null();
  }

  if (ret) {
    return false;
  }

  if (m_result_vars) {
    m_result_vars->update_result();
  }

  return true;
}

Variant MySQLStmt::field_count() {
  VALIDATE_PREPARED
  return (int64_t)mysql_stmt_field_count(m_stmt);
}

Variant MySQLStmt::free_result() {
  VALIDATE_PREPARED
  return mysql_stmt_free_result(m_stmt);
}

Variant MySQLStmt::insert_id() {
  VALIDATE_PREPARED
  return (int64_t)mysql_stmt_insert_id(m_stmt);
}

Variant MySQLStmt::num_rows() {
  VALIDATE_PREPARED
  return (int64_t)mysql_stmt_num_rows(m_stmt);
}

Variant MySQLStmt::param_count() {
  VALIDATE_PREPARED
  return (int64_t)mysql_stmt_param_count(m_stmt);
}

Variant MySQLStmt::prepare(const String& query) {
  VALIDATE_STMT

  // Cleaning up just in case they have been set before
  m_param_vars.reset();
  m_result_vars.reset();

  m_prepared = !mysql_stmt_prepare(m_stmt, query.c_str(), query.size());
  return m_prepared;
}

Variant MySQLStmt::reset() {
  VALIDATE_PREPARED
  return !mysql_stmt_reset(m_stmt);
}

Variant MySQLStmt::result_metadata() {
  VALIDATE_PREPARED

  MYSQL_RES *mysql_result = mysql_stmt_result_metadata(m_stmt);
  if (!mysql_result) {
    return false;
  }

  Array args;
  args.append(Variant(req::make<MySQLResult>(mysql_result)));

  auto cls = Unit::lookupClass(s_mysqli_result.get());
  Object obj{cls};

  tvDecRefGen(
    g_context->invokeFunc(cls->getCtor(), args, obj.get())
  );
  return obj;
}

Variant MySQLStmt::send_long_data(int64_t param_idx, const String& data) {
  VALIDATE_PREPARED
  return !mysql_stmt_send_long_data(m_stmt, param_idx, data.c_str(),
                                    data.size());
}

Variant MySQLStmt::sqlstate() {
  VALIDATE_STMT
  return String(mysql_stmt_sqlstate(m_stmt), CopyString);
}

Variant MySQLStmt::store_result() {
  VALIDATE_PREPARED
  return !mysql_stmt_store_result(m_stmt);
}

#undef VALIDATE_STMT
#undef VALIDATE_PREPARED

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
    return init_null();
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

static bool php_mysql_read_rows(MYSQL *mysql, const Variant& result) {
  unsigned long pkt_len;
  unsigned char *cp;
  unsigned int fields = mysql->field_count;
  NET *net = &mysql->net;
  auto res = php_mysql_extract_result(result);

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
    // consistent with php_mysql_do_query_and_get_result
    return true;
  }
  mysql->status = MYSQL_STATUS_READY;
  Variant result(req::make<MySQLResult>(nullptr, true));
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

MySQLQueryReturn php_mysql_do_query(const String& query, const Variant& link_id,
                                    bool async_mode) {
  SYNC_VM_REGS_SCOPED();
  if (mysqlExtension::ReadOnly &&
      same(preg_match("/^((\\/\\*.*?\\*\\/)|\\(|\\s)*select/i", query),
           0)) {
    raise_notice("runtime/ext_mysql: write query not executed [%s]",
                    query.data());
    return MySQLQueryReturn::OK; // pretend it worked
  }

  std::shared_ptr<MySQL> rconn = nullptr;
  MYSQL* conn = MySQL::GetConn(link_id, &rconn);
  if (!conn || !rconn) return MySQLQueryReturn::FAIL;

  if (RuntimeOption::EnableStats && RuntimeOption::EnableSQLStats) {
    ServerStats::Log("sql.query", 1);

    // removing comments, which can be wrong actually if some string field's
    // value has /* or */ in it.
    Variant result;
    String q = preg_replace(result, "/\\/\\*.*?\\*\\//", " ", query) ?
      result.toString() : query;

    Variant matches;
    preg_match("/^(?:\\(|\\s)*(?:"
               "(insert).*?\\s+(?:into\\s+)?([^\\s\\(,]+)|"
               "(update|set|show)\\s+([^\\s\\(,]+)|"
               "(replace).*?\\s+into\\s+([^\\s\\(,]+)|"
               "(delete).*?\\s+from\\s+([^\\s\\(,]+)|"
               "(select).*?[\\s`]+from\\s+([^\\s\\(,]+)|"
               "(create|alter|drop).*?\\s+table\\s+([^\\s\\(,]+))/is",
               q, &matches);
    auto marray = matches.toArray();
    int size = marray.size();
    if (size > 2) {
      auto verb = toLower(marray[size - 2].toString().slice());
      auto table = toLower(marray[size - 1].toString().slice());
      if (!table.empty() && table[0] == '`') {
        table = table.substr(1, table.length() - 2);
      }
      ServerStats::Log(std::string("sql.query.") + table + "." + verb, 1);
      if (RuntimeOption::EnableStats && RuntimeOption::EnableSQLTableStats) {
        MySqlStats::Record(verb, rconn->m_xaction_count, table);
        if (verb == "update") {
          preg_match("/([^\\s,]+)\\s*=\\s*([^\\s,]+)[\\+\\-]/",
                     q, &matches);
          marray = matches.toArray();
          size = marray.size();
          if (size > 2 && same(marray[1], marray[2])) {
            MySqlStats::Record("incdec", rconn->m_xaction_count, table);
          }
        }
        // we only bump it up when we're in the middle of a transaction
        if (rconn->m_xaction_count) {
          ++rconn->m_xaction_count;
        }
      }
    } else {
      preg_match("/^(?:(?:\\/\\*.*?\\*\\/)|\\(|\\s)*"
                 "(start transaction|begin|commit|rollback|select)/is",
                 query, &matches);
      auto marray = matches.toArray();
      size = marray.size();
      if (size == 2) {
        auto verb = toLower(marray[1].toString().slice());
        rconn->m_xaction_count = ((verb == "begin" ||
                                   verb == "start transaction") ? 1 : 0);
        ServerStats::Log(std::string("sql.query.") + verb, 1);
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
  auto mySQL = MySQL::Get(link_id);
  if (!mySQL) {
    raise_warning("supplied argument is not a valid MySQL-Link resource");
    return MySQLQueryReturn::FAIL;
  }

  if (mySQL->m_multi_query && !mysql_set_server_option(conn, MYSQL_OPTION_MULTI_STATEMENTS_OFF)) {
    mySQL->m_multi_query = false;
  }

  if (async_mode) {
#ifdef FACEBOOK
    mySQL->m_async_query = query.toCppString();
    return MySQLQueryReturn::OK;
#else
    throw_not_implemented("mysql_async_query_start");
#endif
  }

  if (mysql_real_query(conn, query.data(), query.size())) {
#ifdef HHVM_MYSQL_TRACE_MODE
    if (RuntimeOption::EnableHipHopSyntax) {
      raise_notice("runtime/ext_mysql: failed executing [%s] [%s]",
                   query.data(), mysql_error(conn));
    }
#endif

    // When we are timed out, and we're SELECT-ing, we're potentially
    // running a long query on the server without waiting for any results
    // back, wasting server resource. So we're sending a KILL command
    // to see if we can stop the query execution.
    if (tid && mysqlExtension::KillOnTimeout) {
      unsigned int errcode = mysql_errno(conn);
      if (errcode == 2058 /* CR_NET_READ_INTERRUPTED */ ||
          errcode == 2059 /* CR_NET_WRITE_INTERRUPTED */) {
        Variant ret =
          preg_match("/^((\\/\\*.*?\\*\\/)|\\(|\\s)*select/is", query);
        if (!same(ret, false)) {
          MYSQL *new_conn = create_new_conn();
          IOStatusHelper io2("mysql::kill", rconn->m_host.c_str(),
                             rconn->m_port);
          MYSQL *connected = mysql_real_connect
            (new_conn, rconn->m_host.c_str(), rconn->m_username.c_str(),
             rconn->m_password.c_str(), nullptr, rconn->m_port, nullptr, 0);
          if (connected) {
            std::string killsql = "KILL " + folly::to<std::string>(tid);
            if (mysql_real_query(connected, killsql.c_str(), killsql.size())) {
              raise_warning("Unable to kill thread %lu", tid);
            }
          }
          mysql_close(new_conn);
        }
      }
    }

    return MySQLQueryReturn::FAIL;
  }
  Logger::Verbose("runtime/ext_mysql: successfully executed [%dms] [%s]",
                  (int)timer.getTime(), query.data());
  if (mysql_field_count(conn) == 0) {
    return MySQLQueryReturn::OK;
  } else {
    return MySQLQueryReturn::OK_FETCH_RESULT;
  }
}

Variant php_mysql_get_result(const Variant& link_id, bool use_store) {
  std::shared_ptr<MySQL> rconn = nullptr;
  MYSQL *conn = MySQL::GetConn(link_id, &rconn);
  if (!conn || !rconn) return false;

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

  auto r = req::make<MySQLResult>(mysql_result);

  if (RuntimeOption::MaxSQLRowCount > 0 &&
      (s_mysql_data->totalRowCount += r->getRowCount())
      > RuntimeOption::MaxSQLRowCount) {
    ExtendedLogger::Error(
      "MaxSQLRowCount is over: fetching at least %d rows",
      s_mysql_data->totalRowCount
    );
    s_mysql_data->totalRowCount = 0; // so no repetitive logging
  }

  return Variant(std::move(r));
}

Variant php_mysql_do_query_and_get_result(const String& query, const Variant& link_id,
                                          bool use_store, bool async_mode) {
  MySQLQueryReturn result = php_mysql_do_query(query, link_id, async_mode);

  switch (result) {
    case MySQLQueryReturn::OK_FETCH_RESULT:
      return php_mysql_get_result(link_id, use_store);
    case MySQLQueryReturn::OK:
      return true;
    case MySQLQueryReturn::FAIL:
      return false;
  }

  not_reached();
}

///////////////////////////////////////////////////////////////////////////////
// row operations

Variant php_mysql_fetch_hash(const Resource& result, int result_type) {
  if ((result_type & PHP_MYSQL_BOTH) == 0) {
    throw_invalid_argument("result_type: %d", result_type);
    return false;
  }

  auto res = php_mysql_extract_result(result);
  if (!res) return false;

  Array ret;
  if (res->isLocalized()) {
    if (!res->fetchRow()) return false;

    for (int i = 0; i < res->getFieldCount(); i++) {
      if (result_type & PHP_MYSQL_NUM) {
        ret.set(i, res->getField(i));
      }
      if (result_type & PHP_MYSQL_ASSOC) {
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
    if (result_type & PHP_MYSQL_NUM) {
      ret.set(i, data);
    }
    if (result_type & PHP_MYSQL_ASSOC) {
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
  if (m_conn == nullptr) {
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
  bool ret = mysql_real_connect_nonblocking_init(
               m_conn, m_host.c_str(), m_username.c_str(), m_password.c_str(),
               (m_database.empty() ? nullptr : m_database.c_str()), port,
               m_socket.empty() ? nullptr : m_socket.c_str(),
               CLIENT_INTERACTIVE);

  m_state = (ret) ? MySQLState::CONNECTED : MySQLState::CLOSED;
  return ret;
}

#else  // FACEBOOK

// Bogus values for non-facebook libmysqlclients.
const int64_t k_ASYNC_OP_INVALID = 0;
const int64_t k_ASYNC_OP_UNSET = -1;
const int64_t k_ASYNC_OP_CONNECT = -2;
const int64_t k_ASYNC_OP_QUERY = -3;

#endif

}
