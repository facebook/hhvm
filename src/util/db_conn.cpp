/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include "db_conn.h"
#include "db_query.h"
#include "db_mysql.h"
#include "exception.h"
#include "lock.h"
#include "async_job.h"
#include "util.h"
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// Class ServerData

int ServerData::DefaultPort = 3306;
std::string ServerData::DefaultUsername = "root";
std::string ServerData::DefaultPassword = "";

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
                       int port /* = 0 */, const char *username /* = NULL */,
                       const char *password /* = NULL */) : m_port(port) {
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
ServerDataPtrVec DBConn::s_localDatabases;

void DBConn::clearLocalDatabases() {
  Lock lock(s_mutex);
  s_localDatabases.clear();
  s_localDatabases.reserve(32 * 1024);
}

void DBConn::addLocalDB(unsigned int dbId, const char *ip, const char *db,
                        int port, const char *username, const char *password) {
  if (dbId < 102400) {
    Lock lock(s_mutex);
    if (s_localDatabases.size() <= dbId) {
      s_localDatabases.resize(dbId + 1);
    }
    s_localDatabases[dbId] =
      ServerDataPtr(new ServerData(ip, db, port, username, password));
  }
}

///////////////////////////////////////////////////////////////////////////////

DBConn::DBConn()
  : m_conn(NULL), m_connectTimeout(DefaultConnectTimeout),
    m_readTimeout(DefaultReadTimeout) {
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

  m_conn = mysql_init(NULL);
  MySQLUtil::set_mysql_timeout(m_conn, MySQLUtil::ConnectTimeout,
                               connectTimeout);
  MySQLUtil::set_mysql_timeout(m_conn, MySQLUtil::ReadTimeout, readTimeout);
  MYSQL *ret = mysql_real_connect(m_conn, server->getIP().c_str(),
                                  server->getUserName().c_str(),
                                  server->getPassword().c_str(),
                                  server->getDatabase().c_str(),
                                  server->getPort(), NULL, 0);
  if (!ret) {
    const char *msg = mysql_error(m_conn);
    string smsg = msg ? msg : "";
    mysql_close(m_conn);
    m_conn = NULL;
    throw DBConnectionException(server->getIP().c_str(),
                                server->getDatabase().c_str(),
                                smsg.c_str());
  }

  m_server = server;
  m_connectTimeout = connectTimeout;
  m_readTimeout = readTimeout;
}

void DBConn::close() {
  if (isOpened()) {
    mysql_close(m_conn);
    m_conn = NULL;
    m_server.reset();
  }
}

void DBConn::escapeString(const char *s, std::string &out) {
  escapeString(s, strlen(s), out);
}

void DBConn::escapeString(const char *s, int len, std::string &out) {
  ASSERT(s);
  ASSERT(isOpened());

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
  ASSERT(sql && *sql);
  ASSERT(isOpened());

  {
    bool failure;
    if ((failure = mysql_query(m_conn, sql))) {
      if (retryQueryOnFail) {
        open(m_server, m_connectTimeout, m_readTimeout);
        failure = mysql_query(m_conn, sql);
      }
      if (failure) {
        throw DatabaseException("Failed to execute SQL '%s': %s", sql,
                                mysql_error(m_conn));
      }
    }
  }

  MYSQL_RES *result = mysql_store_result(m_conn);
  if (!result && mysql_errno(m_conn)) {
    throw DatabaseException("Failed to execute SQL '%s': %s", sql,
                            mysql_error(m_conn));
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
  ASSERT(isOpened());
  return mysql_insert_id(m_conn);
}

int DBConn::parallelExecute(const char *sql, DBDataSet &ds,
                            map<int, string> &errors, int maxThread,
                            bool retryQueryOnFail, int connectTimeout,
                            int readTimeout) {
  ASSERT(sql && *sql);

  if (s_localDatabases.empty()) {
    return -1;
  }

  QueryJobPtrVec jobs;
  Mutex mutex;
  jobs.reserve(s_localDatabases.size());
  string ssql = sql; // so we have copy-on-write in the loop
  for (unsigned int i = 0; i < s_localDatabases.size(); i++) {
    ServerDataPtr server = s_localDatabases[i];
    if (server) {
      jobs.push_back(QueryJobPtr(new QueryJob(server, ssql, i, mutex, ds,
                                              retryQueryOnFail, connectTimeout,
                                              readTimeout)));
    }
  }
  return parallelExecute(jobs, errors, maxThread);
}

int DBConn::parallelExecute(const ServerQueryVec &sqls, DBDataSet &ds,
                            map<int, string> &errors, int maxThread,
                            bool retryQueryOnFail, int connectTimeout,
                            int readTimeout) {
  if (sqls.empty()) {
    return 0;
  }

  QueryJobPtrVec jobs;
  Mutex mutex;
  jobs.reserve(sqls.size());
  for (unsigned int i = 0; i < sqls.size(); i++) {
    const ServerQuery &query = sqls[i];

    QueryJobPtr job(new QueryJob(query.first, query.second, i, mutex, ds,
                                 retryQueryOnFail, connectTimeout,
                                 readTimeout));
    jobs.push_back(job);
  }
  return parallelExecute(jobs, errors, maxThread);
}

int DBConn::parallelExecute(const ServerQueryVec &sqls,
                            DBDataSetPtrVec &dss,
                            map<int, string> &errors, int maxThread,
                            bool retryQueryOnFail, int connectTimeout,
                            int readTimeout) {
  ASSERT(sqls.size() == dss.size());

  if (sqls.empty()) {
    return 0;
  }

  QueryJobPtrVec jobs;
  Mutex mutex;
  jobs.reserve(sqls.size());
  for (unsigned int i = 0; i < sqls.size(); i++) {
    const ServerQuery &query = sqls[i];

    QueryJobPtr job(new QueryJob(query.first, query.second, i, mutex,
                                 *dss[i], retryQueryOnFail, connectTimeout,
                                 readTimeout));
    jobs.push_back(job);
  }
  return parallelExecute(jobs, errors, maxThread);
}

int DBConn::parallelExecute(QueryJobPtrVec &jobs,
                            map<int, string> &errors, int maxThread) {
  if (maxThread <= 0) maxThread = DefaultWorkerCount;
  JobDispatcher<QueryJob, QueryWorker>(jobs, maxThread).run();

  int affected = 0;
  for (unsigned int i = 0; i < jobs.size(); i++) {
    QueryJobPtr job = jobs[i];

    int count = job->m_affected;
    if (count >= 0) {
      affected += count;
    } else {
      errors[job->m_index] = job->m_error;
    }
  }
  return affected;
}

void DBConn::QueryWorker::doJob(QueryJobPtr job) {
  string &sql = job->m_sql;
  Util::replaceAll(sql, "INDEX", lexical_cast<string>(job->m_index).c_str());

  if (!job->m_server) {
    job->m_affected = -1;
    job->m_error = "(server info missing)";
    return;
  }

  try {
    DBConn conn;
    conn.open(job->m_server, job->m_connectTimeout, job->m_readTimeout);

    if (job->m_dsResult) {
      DBDataSet ds;
      job->m_affected = conn.execute(sql.c_str(), &ds,
                                     job->m_retryQueryOnFail);
      Lock lock(*job->m_dsMutex);
      job->m_dsResult->addDataSet(ds);
    } else {
      job->m_affected = conn.execute(sql.c_str(), NULL,
                                     job->m_retryQueryOnFail);
    }
  } catch (Exception e) {
    job->m_affected = -1;
    job->m_error = e.getMessage();
  } catch (std::exception &e) {
    job->m_affected = -1;
    job->m_error = e.what();
  } catch (...) {
    job->m_affected = -1;
    job->m_error = "(unknown exception)";
  }
}

///////////////////////////////////////////////////////////////////////////////
}
