/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/util/db-conn.h"

#include <cstdlib>

#include <boost/algorithm/string.hpp>

#include "folly/Conv.h"

#include "hphp/util/db-query.h"
#include "hphp/util/db-mysql.h"
#include "hphp/util/exception.h"
#include "hphp/util/lock.h"
#include "hphp/util/async-job.h"
#include "hphp/util/alloc.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DatabaseException::DatabaseException(int code,
                                     const char *fmt, ...) : m_code(code) {
  va_list ap; va_start(ap, fmt); format(fmt, ap); va_end(ap);
}

// Class ServerData

int ServerData::DefaultPort = 3306;
std::string ServerData::DefaultUsername = "root";
std::string ServerData::DefaultPassword = "";

static void parseColonPair(std::string &s, size_t pos,
                           std::string &part1, std::string &part2) {
  std::string tmp = s.substr(0, pos);
  s = s.substr(pos + 1);
  pos = tmp.find(':');
  if (pos == std::string::npos) {
    part1 = tmp;
  } else {
    part1 = tmp.substr(0, pos);
    part2 = tmp.substr(pos + 1);
  }
}

std::shared_ptr<ServerData> ServerData::Create(const std::string &connection) {
  auto server = std::make_shared<ServerData>();
  std::string s = connection;

  size_t pos = s.find('@');
  if (pos != std::string::npos) {
    parseColonPair(s, pos, server->m_username, server->m_password);
  }

  pos = s.find('/');
  if (pos != std::string::npos) {
    std::string port;
    parseColonPair(s, pos, server->m_ip, port);
    if (!port.empty()) server->m_port = std::atoi(port.c_str());
  }

  server->m_database = s;
  return server;
}

ServerData::ServerData() : m_port(0) {
}

ServerData::ServerData(const char *ip, const char *database,
                       int port, const char *username,
                       const char *password,
                       const SessionVariableVec &sessionVariables) :
                       m_port(port), m_sessionVariables(sessionVariables) {
  if (ip) m_ip = ip;
  if (database) m_database = database;
  if (username) m_username = username;
  if (password) m_password = password;
}

int ServerData::getPort() const {
  return m_port > 0 ? m_port : DefaultPort;
}

const std::string &ServerData::getUserName() const {
  return m_username.empty() ? DefaultUsername : m_username;
}

const std::string &ServerData::getPassword() const {
  return m_password.empty() ? DefaultPassword : m_password;
}

///////////////////////////////////////////////////////////////////////////////
// static members

unsigned int DBConn::DefaultConnectTimeout = 1000;
unsigned int DBConn::DefaultReadTimeout = 1000;

Mutex DBConn::s_mutex;
DBConn::DatabaseMap DBConn::s_localDatabases;

void DBConn::ClearLocalDatabases() {
  Lock lock(s_mutex);
  s_localDatabases.clear();
}

void DBConn::AddLocalDB(int dbId, const char *ip, const char *db,
                        int port, const char *username, const char *password,
                        const SessionVariableVec &sessionVariables) {
  Lock lock(s_mutex);
  s_localDatabases[dbId] =
    std::make_shared<ServerData>(ip, db, port, username, password,
                                 sessionVariables);
}

///////////////////////////////////////////////////////////////////////////////

DBConn::DBConn(int maxRetryOpenOnFail, int maxRetryQueryOnFail)
  : m_conn(nullptr), m_connectTimeout(DefaultConnectTimeout),
    m_readTimeout(DefaultReadTimeout),
    m_maxRetryOpenOnFail(maxRetryOpenOnFail) {
}

DBConn::~DBConn() {
  close();
}

void DBConn::open(std::shared_ptr<ServerData> server,
                  int connectTimeout /* = -1 */,
                  int readTimeout /* = -1 */) {
  if (isOpened()) {
    close();
  }

  if (connectTimeout <= 0) connectTimeout = DefaultConnectTimeout;
  if (readTimeout <= 0) readTimeout = DefaultReadTimeout;

  m_conn = mysql_init(nullptr);
  MySQLUtil::set_mysql_timeout(m_conn, MySQLUtil::ConnectTimeout,
                               connectTimeout);
  MySQLUtil::set_mysql_timeout(m_conn, MySQLUtil::ReadTimeout, readTimeout);
  MYSQL *ret = mysql_real_connect(m_conn, server->getIP().c_str(),
                                  server->getUserName().c_str(),
                                  server->getPassword().c_str(),
                                  server->getDatabase().c_str(),
                                  server->getPort(), nullptr, 0);
  if (!ret) {
    int code = mysql_errno(m_conn);
    const char *msg = mysql_error(m_conn);
    std::string smsg = msg ? msg : "";
    mysql_close(m_conn);
    m_conn = nullptr;
    throw DBConnectionException(code, server->getIP().c_str(),
                                server->getDatabase().c_str(),
                                smsg.c_str());
  }

  // Setting session variables
  if (server->getSessionVariables().size()) {
    auto sessionCmd = std::string("SET ");
    for (auto iter = server->getSessionVariables().begin();
        iter != server->getSessionVariables().end();
        iter++) {
      if (iter != server->getSessionVariables().begin()) {
        sessionCmd += ", ";
      }
      sessionCmd += std::string("SESSION ") + iter->first + std::string("=") +
                    iter->second;
    }

    char *sessionVarSQL = (char*)safe_malloc(sessionCmd.length() * 2 + 1);
    mysql_real_escape_string(m_conn, sessionVarSQL, sessionCmd.c_str(),
      sessionCmd.length());
    bool failure = mysql_query(m_conn, sessionVarSQL);
    safe_free(sessionVarSQL);
    if (failure) {
      int code = mysql_errno(m_conn);
      throw DatabaseException(code, "Failed to execute SQL '%s': %s (%d)",
                              sessionCmd.c_str(), mysql_error(m_conn), code);
    }
  }

  m_server = server;
  m_connectTimeout = connectTimeout;
  m_readTimeout = readTimeout;
}

void DBConn::close() {
  if (isOpened()) {
    mysql_close(m_conn);
    m_conn = nullptr;
    m_server.reset();
  }
}

void DBConn::escapeString(const char *s, std::string &out) {
  escapeString(s, strlen(s), out);
}

void DBConn::escapeString(const char *s, int len, std::string &out) {
  assert(s);
  assert(isOpened());

  if (len) {
    char *buffer = (char*)malloc(len * 2 + 1);
    mysql_real_escape_string(m_conn, buffer, s, len);
    out = buffer;
    free(buffer);
  }
}

int DBConn::execute(const std::string &sql, DBDataSet *ds /* = NULL */,
                    bool retryQueryOnFail /* = true */) {
  return execute(sql.c_str(), ds, retryQueryOnFail);
}

int DBConn::execute(const char *sql, DBDataSet *ds /* = NULL */,
                    bool retryQueryOnFail /* = true */) {
  assert(sql && *sql);
  assert(isOpened());

  {
    bool failure;
    if ((failure = mysql_query(m_conn, sql))) {
      if (retryQueryOnFail) {
        for (int count = 0; count < m_maxRetryOpenOnFail; count++) {
          open(m_server, m_connectTimeout, m_readTimeout);
          failure = mysql_query(m_conn, sql);
          if (!failure) break;
        }
      }
      if (failure) {
        int code = mysql_errno(m_conn);
        throw DatabaseException(code, "Failed to execute SQL '%s': %s (%d)",
                                sql, mysql_error(m_conn), code);
      }
    }
  }

  MYSQL_RES *result = mysql_store_result(m_conn);
  if (!result) {
    int code = mysql_errno(m_conn);
    if (code) {
      throw DatabaseException(code, "Failed to execute SQL '%s': %s (%d)", sql,
                              mysql_error(m_conn), code);
    }
  }

  int affected = mysql_affected_rows(m_conn);
  if (ds) {
    ds->addResult(m_conn, result);
  } else {
    mysql_free_result(result);
  }
  return affected;
}

int DBConn::getLastInsertId() {
  assert(isOpened());
  return mysql_insert_id(m_conn);
}

///////////////////////////////////////////////////////////////////////////////
}
