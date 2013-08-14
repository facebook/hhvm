/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/util/db-query.h"
#include "hphp/util/db-mysql.h"
#include "hphp/util/exception.h"
#include "hphp/util/lock.h"
#include "hphp/util/async-job.h"
#include "util.h"
#include "hphp/util/alloc.h"
#include <boost/lexical_cast.hpp>

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

class DBConnQueryJob {
 public:
  DBConnQueryJob(ServerDataPtr server, const std::string sql, int index,
           Mutex &mutex, DBDataSet &dsResult, bool retryQueryOnFail,
           unsigned int readTimeout, unsigned int connectTimeout,
           int maxRetryOpenOnFail, int maxRetryQueryOnFail)
      : m_server(server), m_sql(sql), m_index(index),
        m_affected(0), m_dsMutex(&mutex), m_dsResult(&dsResult),
        m_retryQueryOnFail(retryQueryOnFail), m_connectTimeout(connectTimeout),
        m_readTimeout(readTimeout),
        m_maxRetryOpenOnFail(maxRetryOpenOnFail),
        m_maxRetryQueryOnFail(maxRetryQueryOnFail) {}

  ServerDataPtr m_server;
  std::string m_sql;
  int m_index;
  int m_affected;
  Mutex *m_dsMutex;
  DBDataSet *m_dsResult;
  DBConn::ErrorInfo m_error;
  bool m_retryQueryOnFail;
  int m_connectTimeout;
  int m_readTimeout;
  int m_maxRetryOpenOnFail;
  int m_maxRetryQueryOnFail;
};

class DBConnQueryWorker {
 public:
  void onThreadEnter() {}
  void doJob(DBConnQueryJobPtr job);
  void onThreadExit() { mysql_thread_end();}
};

static void parseColonPair(std::string &s, size_t pos,
                           std::string &part1, std::string &part2) {
  string tmp = s.substr(0, pos);
  s = s.substr(pos + 1);
  pos = tmp.find(':');
  if (pos == string::npos) {
    part1 = tmp;
  } else {
    part1 = tmp.substr(0, pos);
    part2 = tmp.substr(pos + 1);
  }
}

ServerDataPtr ServerData::Create(const std::string &connection) {
  ServerDataPtr server(new ServerData());
  string s = connection;

  size_t pos = s.find('@');
  if (pos != string::npos) {
    parseColonPair(s, pos, server->m_username, server->m_password);
  }

  pos = s.find('/');
  if (pos != string::npos) {
    string port;
    parseColonPair(s, pos, server->m_ip, port);
    if (!port.empty()) server->m_port = atoi(port.c_str());
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

unsigned int DBConn::DefaultWorkerCount = 50;
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
    ServerDataPtr(new ServerData(ip, db, port, username, password,
                                 sessionVariables));
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

void DBConn::open(ServerDataPtr server, int connectTimeout /* = -1 */,
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
    string smsg = msg ? msg : "";
    mysql_close(m_conn);
    m_conn = nullptr;
    throw DBConnectionException(code, server->getIP().c_str(),
                                server->getDatabase().c_str(),
                                smsg.c_str());
  }

  // Setting session variables
  if (server->getSessionVariables().size()) {
    string sessionCmd = string("SET ");
    for (SessionVariableVec::const_iterator iter =
         server->getSessionVariables().begin(); iter !=
         server->getSessionVariables().end(); iter++) {
      if (iter != server->getSessionVariables().begin()) {
        sessionCmd += ", ";
      }
      sessionCmd += string("SESSION ") + iter->first + string("=") +
                    iter->second;
    }

    char *sessionVarSQL = (char*)Util::safe_malloc(sessionCmd.length() * 2 + 1);
    mysql_real_escape_string(m_conn, sessionVarSQL, sessionCmd.c_str(),
      sessionCmd.length());
    bool failure = mysql_query(m_conn, sessionVarSQL);
    Util::safe_free(sessionVarSQL);
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

int DBConn::parallelExecute(const char *sql, DBDataSet &ds,
                            ErrorInfoMap &errors, int maxThread,
                            bool retryQueryOnFail, int connectTimeout,
                            int readTimeout,
                            int maxRetryOpenOnFail,
                            int maxRetryQueryOnFail) {
  assert(sql && *sql);

  if (s_localDatabases.empty()) {
    return -1;
  }

  DBConnQueryJobPtrVec jobs;
  Mutex mutex;
  jobs.reserve(s_localDatabases.size());
  string ssql = sql; // so we have copy-on-write in the loop
  for (DatabaseMap::const_iterator iter = s_localDatabases.begin();
       iter != s_localDatabases.end(); ++iter) {
    jobs.push_back(DBConnQueryJobPtr(
                     new DBConnQueryJob(iter->second, ssql, iter->first,
                                        mutex, ds,
                                        retryQueryOnFail, connectTimeout,
                                        readTimeout,
                                        maxRetryOpenOnFail,
                                        maxRetryQueryOnFail)));
  }
  return parallelExecute(jobs, errors, maxThread);
}

int DBConn::parallelExecute(const ServerQueryVec &sqls, DBDataSet &ds,
                            ErrorInfoMap &errors, int maxThread,
                            bool retryQueryOnFail, int connectTimeout,
                            int readTimeout,
                            int maxRetryOpenOnFail,
                            int maxRetryQueryOnFail) {
  if (sqls.empty()) {
    return 0;
  }

  DBConnQueryJobPtrVec jobs;
  Mutex mutex;
  jobs.reserve(sqls.size());
  for (unsigned int i = 0; i < sqls.size(); i++) {
    const ServerQuery &query = sqls[i];

    DBConnQueryJobPtr job(
      new DBConnQueryJob(query.first, query.second, i, mutex, ds,
                         retryQueryOnFail, connectTimeout,
                         readTimeout,
                         maxRetryOpenOnFail, maxRetryQueryOnFail));
    jobs.push_back(job);
  }
  return parallelExecute(jobs, errors, maxThread);
}

int DBConn::parallelExecute(const ServerQueryVec &sqls, DBDataSetPtrVec &dss,
                            ErrorInfoMap &errors, int maxThread,
                            bool retryQueryOnFail, int connectTimeout,
                            int readTimeout,
                            int maxRetryOpenOnFail,
                            int maxRetryQueryOnFail) {
  assert(sqls.size() == dss.size());

  if (sqls.empty()) {
    return 0;
  }

  DBConnQueryJobPtrVec jobs;
  Mutex mutex;
  jobs.reserve(sqls.size());
  for (unsigned int i = 0; i < sqls.size(); i++) {
    const ServerQuery &query = sqls[i];

    DBConnQueryJobPtr job(
      new DBConnQueryJob(query.first, query.second, i, mutex,
                         *dss[i], retryQueryOnFail, connectTimeout,
                         readTimeout,
                         maxRetryOpenOnFail, maxRetryQueryOnFail));
    jobs.push_back(job);
  }
  return parallelExecute(jobs, errors, maxThread);
}

int DBConn::parallelExecute(DBConnQueryJobPtrVec &jobs, ErrorInfoMap &errors,
                            int maxThread) {
  if (maxThread <= 0) maxThread = DefaultWorkerCount;
  JobDispatcher<DBConnQueryJob, DBConnQueryWorker>(jobs, maxThread).run();

  int affected = 0;
  for (unsigned int i = 0; i < jobs.size(); i++) {
    DBConnQueryJobPtr job = jobs[i];

    int count = job->m_affected;
    if (count >= 0) {
      affected += count;
    } else {
      errors[job->m_index] = job->m_error;
    }
  }
  return affected;
}

void DBConnQueryWorker::doJob(DBConnQueryJobPtr job) {
  string &sql = job->m_sql;
  Util::replaceAll(sql, "INDEX", lexical_cast<string>(job->m_index).c_str());

  if (!job->m_server) {
    job->m_affected = -1;
    job->m_error.code = -1;
    job->m_error.msg = "(server info missing)";
    return;
  }

  try {
    DBConn conn;
    int count = 0;
  retry:
    try {
      count++;
      conn.open(job->m_server, job->m_connectTimeout, job->m_readTimeout);
    } catch (DatabaseException &e) {
      if (job->m_retryQueryOnFail &&
          count <= job->m_maxRetryQueryOnFail) {
        goto retry;
      } else {
        throw;
      }
    }

    if (job->m_dsResult) {
      DBDataSet ds;
      job->m_affected = conn.execute(sql.c_str(), &ds,
                                     job->m_retryQueryOnFail);
      Lock lock(*job->m_dsMutex);
      job->m_dsResult->addDataSet(ds);
    } else {
      job->m_affected = conn.execute(sql.c_str(), nullptr,
                                     job->m_retryQueryOnFail);
    }
  } catch (DatabaseException &e) {
    job->m_affected = -1;
    job->m_error.code = e.m_code;
    job->m_error.msg = e.getMessage();
  } catch (Exception &e) {
    job->m_affected = -1;
    job->m_error.code = -1;
    job->m_error.msg = e.getMessage();
  } catch (std::exception &e) {
    job->m_affected = -1;
    job->m_error.code = -1;
    job->m_error.msg = e.what();
  } catch (...) {
    job->m_affected = -1;
    job->m_error.code = -1;
    job->m_error.msg = "(unknown exception)";
  }
}

///////////////////////////////////////////////////////////////////////////////
}
