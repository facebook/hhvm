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

#include <runtime/ext/ext_mysql.h>
#include <runtime/ext/ext_preg.h>
#include <runtime/ext/ext_network.h>
#include <runtime/base/runtime_option.h>
#include <runtime/base/server/server_stats.h>
#include <runtime/base/util/request_local.h>
#include <util/timer.h>
#include <util/db_mysql.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace std;

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(mysql);
///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP(MySQLResult);

MySQLResult::MySQLResult(MYSQL_RES *res, bool localized /* = false */)
  : m_res(res), m_localized(localized) {
  m_fields = NULL;
  m_field_count = 0;
  m_current_field = -1;
  if (localized) {
    m_res = NULL; // ensure that localized results don't have another result
    m_rows = new std::list<std::vector<Variant *> >(1); //sentinel
    m_current_row = m_rows->begin();
    m_row_ready = false;
    m_row_count = 0;
  } else {
    m_rows = NULL;
  }
}

MySQLResult::~MySQLResult() {
  close();
  if (m_fields) {
    for (int i = 0; i < m_field_count; i++) {
      MySQLFieldInfo &info = m_fields[i];
      if (info.name) {
        DELETE(Variant)(info.name);
        DELETE(Variant)(info.table);
        DELETE(Variant)(info.def);
      }
    }
    delete[] m_fields;
    m_fields = NULL;
  }
  if (m_rows) {
    for (list<vector<Variant *> >::const_iterator it = m_rows->begin();
         it != m_rows->end(); it++) {
      for (unsigned int i = 0; i < it->size(); i++) {
        DELETE(Variant)((*it)[i]);
      }
    }
    delete m_rows;
    m_rows = NULL;
  }
}

void MySQLResult::sweep() {
  close();
  // When a dangling MySQLResult is swept, there is no need to deallocate
  // any Variant object.
  delete[] m_fields;
  delete m_rows;
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
    readTimeout = RuntimeOption::MySQLReadTimeout;
    totalRowCount = 0;
  }

  virtual void requestShutdown() {
    defaultConn.reset();
    totalRowCount = 0;
  }

  Object defaultConn;
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
  MySQL *mysql = link_identifier.toObject().getTyped<MySQL>
    (!RuntimeOption::ThrowBadTypeExceptions,
     !RuntimeOption::ThrowBadTypeExceptions);
  if (mysql) {
    SetDefaultConn(mysql);
  }
  return mysql;
}

MYSQL *MySQL::GetConn(CVarRef link_identifier, MySQL **rconn /* = NULL */) {
  MySQL *mySQL = Get(link_identifier);
  MYSQL *ret = NULL;
  if (mySQL) {
    ret = mySQL->get();
  }
  if (ret == NULL) {
    raise_warning("supplied argument is not a valid MySQL-Link resource");
  }
  if (rconn) {
    *rconn = mySQL;
  }
  return ret;
}

bool MySQL::CloseConn(CVarRef link_identifier) {
  MySQL *mySQL = Get(link_identifier);
  if (mySQL) {
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
  return MYSQL_UNIX_ADDR;
}

String MySQL::GetHash(CStrRef host, int port, CStrRef socket, CStrRef username,
                      CStrRef password, int client_flags) {
  char buf[1024];
  snprintf(buf, sizeof(buf), "%s:%d:%s:%s:%s:%d",
           host.data(), port, socket.data(),
           username.data(), password.data(), client_flags);
  return String(buf, CopyString);
}

MySQL *MySQL::GetCachedImpl(const char *name, CStrRef host, int port,
                            CStrRef socket, CStrRef username, CStrRef password,
                            int client_flags) {
  String key = GetHash(host, port, socket, username, password, client_flags);
  return dynamic_cast<MySQL*>(g_persistentObjects->get(name, key.data()));
}

void MySQL::SetCachedImpl(const char *name, CStrRef host, int port,
                          CStrRef socket, CStrRef username, CStrRef password,
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
static MYSQL *create_new_conn() {
  MYSQL *ret = mysql_init(NULL);
  mysql_options(ret, MYSQL_OPT_LOCAL_INFILE, 0);
  if (RuntimeOption::MySQLConnectTimeout) {
    MySQLUtil::set_mysql_timeout(ret, MySQLUtil::ConnectTimeout,
                                 RuntimeOption::MySQLConnectTimeout);
  }
  int readTimeout = s_mysql_data->readTimeout;
  if (readTimeout) {
    MySQLUtil::set_mysql_timeout(ret, MySQLUtil::ReadTimeout, readTimeout);
    MySQLUtil::set_mysql_timeout(ret, MySQLUtil::WriteTimeout, readTimeout);
  }
  return ret;
}

MySQL::MySQL(const char *host, int port, const char *username,
             const char *password)
  : m_port(port), m_last_error_set(false), m_last_errno(0) {
  if (host) m_host = host;
  if (username) m_username = username;
  if (password) m_password = password;

  m_conn = create_new_conn();
}

MySQL::~MySQL() {
  close();
}

void MySQL::setLastError(const char *func) {
  ASSERT(m_conn);
  m_last_error_set = true;
  m_last_errno = mysql_errno(m_conn);
  const char *error = mysql_error(m_conn);
  m_last_error = error ? error : "";
  raise_warning("%s(): %s", func, m_last_error.c_str());
}

void MySQL::close() {
  if (m_conn) {
    m_last_error_set = false;
    m_last_errno = 0;
    m_last_error.clear();
    mysql_close(m_conn);
    m_conn = NULL;
  }
}

bool MySQL::connect(CStrRef host, int port, CStrRef socket, CStrRef username,
                    CStrRef password, int client_flags, int connect_timeout) {
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
  return mysql_real_connect(m_conn, host.data(), username.data(),
                            password.data(), NULL, port,
                            socket.empty() ? NULL : socket.data(),
                            client_flags);
}

bool MySQL::reconnect(CStrRef host, int port, CStrRef socket, CStrRef username,
                      CStrRef password, int client_flags,
                      int connect_timeout) {
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
                              password.data(), NULL, port, socket.data(),
                              client_flags);
  }

  if (!mysql_ping(m_conn)) {
    if (RuntimeOption::EnableStats && RuntimeOption::EnableSQLStats) {
      ServerStats::Log("sql.reconn_ok", 1);
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
  return mysql_real_connect(m_conn, host.data(), username.data(),
                            password.data(), NULL, port, socket.data(),
                            client_flags);
}

///////////////////////////////////////////////////////////////////////////////
// helpers

static MySQLResult *get_result(CVarRef result) {
  MySQLResult *res = result.toObject().getTyped<MySQLResult>
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
    return *(info->name);
  case PHP_MYSQL_FIELD_TABLE:
    return *(info->table);
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
                                    String password, int client_flags,
                                    bool persistent,
                                    int connect_timeout_ms,
                                    int query_timeout_ms) {
  if (connect_timeout_ms < 0) {
    connect_timeout_ms = RuntimeOption::MySQLConnectTimeout;
  }
  if (query_timeout_ms < 0) {
    query_timeout_ms = s_mysql_data->readTimeout;
  }
  if (server.empty()) server = MySQL::GetDefaultServer();
  if (username.empty()) username = MySQL::GetDefaultUsername();
  if (password.empty()) password = MySQL::GetDefaultPassword();

  // server format: hostname[:port][:/path/to/socket]
  String host, socket;
  int port = MYSQL_PORT;
  int pos = server.find(':');
  if (pos >= 0) {
    host = server.substr(0, pos);
    if (server.charAt(pos + 1) != '/') {
      String sport = server.substr(pos + 1);
      port = sport.toInt32();
      pos = sport.find(':');
      if (pos >= 0) {
        socket = sport.substr(pos + 1);
      }
    } else {
      socket = server.substr(pos + 1);
    }
  } else {
    host = server;
    port = MySQL::GetDefaultPort();
  }
  if (socket.empty()) {
    socket = MySQL::GetDefaultSocket();
  }

  Object ret;
  MySQL *mySQL = NULL;
  if (persistent) {
    mySQL = MySQL::GetPersistent(host, port, socket, username, password,
                                 client_flags);
  }

  if (mySQL == NULL) {
    mySQL = new MySQL(host, port, username, password);
    ret = mySQL;
    if (!mySQL->connect(host, port, socket, username, password,
                        client_flags, connect_timeout_ms)) {
      MySQL::SetDefaultConn(mySQL);
      mySQL->setLastError("mysql_connect");
      return false;
    }
  } else {
    ret = mySQL;
    if (!mySQL->reconnect(host, port, socket, username, password,
                          client_flags, connect_timeout_ms)) {
      MySQL::SetDefaultConn(mySQL);
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

Variant f_mysql_connect(CStrRef server /* = null_string */,
                        CStrRef username /* = null_string */,
                        CStrRef password /* = null_string */,
                        bool new_link /* = false */,
                        int client_flags /* = 0 */,
                        int connect_timeout_ms /* = -1 */,
                        int query_timeout_ms /* = -1 */) {
  return php_mysql_do_connect(server, username, password, client_flags, false,
                              connect_timeout_ms, query_timeout_ms);
}

Variant f_mysql_pconnect(CStrRef server /* = null_string */,
                         CStrRef username /* = null_string */,
                         CStrRef password /* = null_string */,
                         int client_flags /* = 0 */,
                         int connect_timeout_ms /* = -1 */,
                         int query_timeout_ms /* = -1 */) {
  return php_mysql_do_connect(server, username, password, client_flags, true,
                              connect_timeout_ms, query_timeout_ms);
}

bool f_mysql_set_timeout(int query_timeout_ms /* = -1 */,
                         CVarRef link_identifier /* = null */) {
  if (query_timeout_ms < 0) {
    query_timeout_ms = RuntimeOption::MySQLReadTimeout;
  }
  s_mysql_data->readTimeout = query_timeout_ms;
  return true;
}

String f_mysql_escape_string(CStrRef unescaped_string) {
  char *new_str = (char *)malloc(unescaped_string.size() * 2 + 1);
  int new_len = mysql_escape_string(new_str, unescaped_string.data(),
                                    unescaped_string.size());
  return String(new_str, new_len, AttachString);
}

Variant f_mysql_real_escape_string(CStrRef unescaped_string,
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

Variant f_mysql_errno(CVarRef link_identifier /* = null */) {
  MySQL *mySQL = MySQL::Get(link_identifier);
  if (!mySQL) {
    raise_warning("supplied argument is not a valid MySQL-Link resource");
    return false;
  }
  MYSQL *conn = mySQL->get();
  if (conn) {
    return (int64)mysql_errno(conn);
  }
  if (mySQL->m_last_error_set) {
    return (int64)mySQL->m_last_errno;
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

///////////////////////////////////////////////////////////////////////////////
// query functions

static Variant mysql_makevalue(CStrRef data, MYSQL_FIELD *mysql_field) {
  switch (mysql_field->type) {
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
  case MYSQL_TYPE_NULL:
    return null;
  default:
    break;
  }
  return data;
}

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

  res->setFieldCount((int64)fields);

  // localizes all the rows
  while (*(cp = net->read_pos) != 254 || pkt_len >= 8) {
    res->addRow();
    for (unsigned int i = 0; i < fields; i++) {
      unsigned long len = net_field_length(&cp);
      Variant *data = NEW(Variant)();
      if (len != NULL_LENGTH) {
        *data = mysql_makevalue(String((char *)cp, len, CopyString),
                                mysql->fields + i);
        cp += len;
        if (mysql->fields) {
          if (mysql->fields[i].max_length < len)
            mysql->fields[i].max_length = len;
        }
      }
      res->addField(data);
    }
    if ((pkt_len = cli_safe_read(mysql)) == packet_error) {
      return false;
    }
  }

  // localizes all the field info
  for (unsigned int i = 0; i < fields; i++) {
    res->setFieldInfo((int64)i, mysql->fields + i);
  }

  return true;
}

static Variant php_mysql_localize_result(MYSQL *mysql) {
  mysql = mysql->last_used_con;
  if (!mysql->fields) return true;
  if (mysql->status != MYSQL_STATUS_GET_RESULT) {
    // consistent with php_mysql_do_query_general
    return true;
  }
  mysql->status = MYSQL_STATUS_READY;
  Variant result = Object(NEW(MySQLResult)(NULL, true));
  if (!php_mysql_read_rows(mysql, result)) {
    return true;
  }

  // clean up
  if (mysql->fields) {
    free_root(&mysql->field_alloc, 0);
  }
  mysql->unbuffered_fetch_owner = 0;

  return result;
}

static Variant php_mysql_do_query_general(CStrRef query, CVarRef link_id,
                                          bool use_store) {
  if (RuntimeOption::MySQLReadOnly &&
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

    Variant matches;
    f_preg_match("/^(?:(?:\\/\\*.*?\\*\\/)|\\(|\\s)*(?:"
                 "(insert).*?\\s+(?:into\\s+)?([^\\s]+)|"
                 "(update|set|show)\\s+([^\\s]+)|"
                 "(replace).*?\\s+into\\s+([^\\s]+)|"
                 "(delete).*?\\s+from\\s+([^\\s]+)|"
                 "(select).*?[\\s`]+from\\s+([^\\s]+))/is",
                 query, ref(matches));
    int size = matches.toArray().size();
    if (size > 2) {
      string verb = Util::toLower(matches[size - 2].toString().data());
      string table = Util::toLower(matches[size - 1].toString().data());
      if (!table.empty() && table[0] == '`') {
        table = table.substr(1, table.length() - 2);
      }
      ServerStats::Log(string("sql.query.") + table + "." + verb, 1);
    } else {
      f_preg_match("/^(?:(?:\\/\\*.*?\\*\\/)|\\(|\\s)*"
                   "(begin|commit|rollback)/is",
                   query, ref(matches));
      size = matches.toArray().size();
      if (size == 2) {
        string verb = Util::toLower(matches[1].toString().data());
        ServerStats::Log(string("sql.query.") + verb, 1);
      } else {
        raise_warning("Unable to record MySQL stats with: %s", query.data());
        ServerStats::Log("sql.query.unknown", 1);
      }
    }
  }

  SlowTimer timer(RuntimeOption::MySQLSlowQueryThreshold,
                  "runtime/ext_mysql: slow query", query.data());
  IOStatusHelper io("mysql::query", rconn->m_host.c_str(), rconn->m_port);
  unsigned long tid = mysql_thread_id(conn);
  if (mysql_real_query(conn, query.data(), query.size())) {
    raise_notice("runtime/ext_mysql: failed executing [%s] [%s]", query.data(),
                 mysql_error(conn));

    // When we are timed out, and we're SELECT-ing, we're potentially
    // running a long query on the server without waiting for any results
    // back, wasting server resource. So we're sending a KILL command
    // to see if we can stop the query execution.
    if (tid && RuntimeOption::MySQLKillOnTimeout) {
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
              raise_warning("Unable to kill thread %llu", tid);
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
    if (RuntimeOption::MySQLLocalize) {
      return php_mysql_localize_result(conn);
    }
    mysql_result = mysql_store_result(conn);
  } else {
    mysql_result = mysql_use_result(conn);
  }
  if (!mysql_result) {
    return true;
  }

  MySQLResult *r = NEW(MySQLResult)(mysql_result);
  Object ret(r);

  if (RuntimeOption::MaxSQLRowCount > 0 &&
      (s_mysql_data->totalRowCount += r->getRowCount())
      > RuntimeOption::MaxSQLRowCount) {
    Logger::Error("MaxSQLRowCount is over: fetching at least %d rows: %s",
                  s_mysql_data->totalRowCount, query.data());
    s_mysql_data->totalRowCount = 0; // so no repetitive logging
  }

  return ret;
}

Variant f_mysql_query(CStrRef query, CVarRef link_identifier /* = null */) {
  return php_mysql_do_query_general(query, link_identifier, true);
}

Variant f_mysql_unbuffered_query(CStrRef query,
                                 CVarRef link_identifier /* = null */) {
  return php_mysql_do_query_general(query, link_identifier, false);
}

Variant f_mysql_list_dbs(CVarRef link_identifier /* = null */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  MYSQL_RES *res = mysql_list_dbs(conn, NULL);
  if (!res) {
    raise_warning("Unable to save MySQL query result");
    return false;
  }
  return Object(NEW(MySQLResult)(res));
}

Variant f_mysql_list_tables(CStrRef database,
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
  return Object(NEW(MySQLResult)(res));
}

Variant f_mysql_list_processes(CVarRef link_identifier /* = null */) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  MYSQL_RES *res = mysql_list_processes(conn);
  if (!res) {
    raise_warning("Unable to save MySQL query result");
    return false;
  }
  return Object(NEW(MySQLResult)(res));
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
        ret.set(info->name->toString(), res->getField(i));
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
                             CStrRef class_name /* = "stdClass" */,
                             CArrRef params /* = null */) {
  Variant properties = php_mysql_fetch_hash(result, MYSQL_ASSOC);
  if (!same(properties, false)) {
    Object obj = create_object(class_name.data(), params);
    obj->o_set(properties);
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
      raise_warning("Unable to jump to row %ld on MySQL result index %ld",
                      row, result.toObject()->o_getId());
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
        if ((table_name.empty() || table_name.same(info->table->toString())) &&
            field_name.same(info->name->toString())) {
          field_offset = i;
          found = true;
          break;
        }
        i++;
      }
      if (!found) { /* no match found */
        raise_warning("%s%s%s not found in MySQL result index %ld",
                        table_name.data(), (table_name.empty() ? "" : "."),
                        field_name.data(), result.toObject()->o_getId());
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
  return null;
}

///////////////////////////////////////////////////////////////////////////////
// result functions

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

  Object obj(NEW(c_stdclass)());
  obj->set("name",         *(info->name));
  obj->set("table",        *(info->table));
  obj->set("def",          *(info->def));
  obj->set("max_length",   (int)info->max_length);
  obj->set("not_null",     IS_NOT_NULL(info->flags)? 1 : 0);
  obj->set("primary_key",  IS_PRI_KEY(info->flags)? 1 : 0);
  obj->set("multiple_key", info->flags & MULTIPLE_KEY_FLAG? 1 : 0);
  obj->set("unique_key",   info->flags & UNIQUE_KEY_FLAG? 1 : 0);
  obj->set("numeric",      IS_NUM(info->type)? 1 : 0);
  obj->set("blob",         IS_BLOB(info->flags)? 1 : 0);
  obj->set("type",         php_mysql_get_field_name(info->type));
  obj->set("unsigned",     info->flags & UNSIGNED_FLAG? 1 : 0);
  obj->set("zerofill",     info->flags & ZEROFILL_FLAG? 1 : 0);
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
  m_rows->push_back(vector<Variant *>());
  m_rows->back().reserve(getFieldCount());
}

void MySQLResult::addField(Variant *value) {
  m_rows->back().push_back(value);
}

void MySQLResult::setFieldCount(int64 fields) {
  m_field_count = fields;
  m_fields = new MySQLFieldInfo[fields];
}

void MySQLResult::setFieldInfo(int64 f, MYSQL_FIELD *field) {
  MySQLFieldInfo &info = m_fields[f];
  info.name = NEW(Variant)(String(field->name, CopyString));
  info.table = NEW(Variant)(String(field->table, CopyString));
  info.def = NEW(Variant)(String(field->def, CopyString));
  info.max_length = (int64)field->max_length;
  info.length = (int64)field->length;
  info.type = (int)field->type;
  info.flags = field->flags;
}

MySQLFieldInfo *MySQLResult::getFieldInfo(int64 field) {
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

Variant MySQLResult::getField(int64 field) const {
  if (!m_localized || field < 0 || field >= (int64)m_current_row->size()) {
    return null;
  }
  return *(*m_current_row)[field];
}

int64 MySQLResult::getFieldCount() const {
  if (!m_localized) {
    return (int64)mysql_num_fields(m_res);
  }
  return m_field_count;
}

int64 MySQLResult::getRowCount() const {
  if (!m_localized) {
    return (int64)mysql_num_rows(m_res);
  }
  return m_row_count;
}

bool MySQLResult::seekRow(int64 row) {
  if (row < 0 || row >= getRowCount()) {
    raise_warning("Unable to jump to row %ld on MySQL result index %ld",
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

bool MySQLResult::seekField(int64 field) {
  if (field < 0 || field >= getFieldCount()) {
    raise_warning("Field %ld is invalid for MySQL result index %ld",
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
