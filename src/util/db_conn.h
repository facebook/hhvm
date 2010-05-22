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

#ifndef __DB_CONN_H__
#define __DB_CONN_H__

#include "db_dataset.h"
#include "mutex.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * What it takes to connect to a MySQL server.
 */
DECLARE_BOOST_TYPES(ServerData);
class ServerData {
 public:
  static int DefaultPort;
  static std::string DefaultUsername;
  static std::string DefaultPassword;

  /**
   * Connection string is in this format:
   *
   *   username:password@server:port/database
   */
  static ServerDataPtr Create(const std::string &connection);

 public:
  ServerData();
  ServerData(const char *ip, const char *database, int port = 0,
             const char *username = NULL, const char *password = NULL);

  const std::string &getIP() const { return m_ip;}
  int getPort() const;
  const std::string &getUserName() const;
  const std::string &getPassword() const;
  const std::string &getDatabase() const { return m_database;}

 private:
  std::string m_ip;
  int m_port;
  std::string m_username;
  std::string m_password;
  std::string m_database;
};

typedef std::pair<ServerDataPtr, std::string> ServerQuery;
typedef std::vector<ServerQuery> ServerQueryVec;

///////////////////////////////////////////////////////////////////////////////

/**
 * A connection class that connects to any of our databases.
 */
class DBConn {
 public:
  static unsigned int DefaultWorkerCount; // for parallel executions
  static unsigned int DefaultConnectTimeout;
  static unsigned int DefaultReadTimeout;

 public:
  DBConn();
  ~DBConn();

  /**
   * Open a database by specifying a type and an id.
   */
  void open(ServerDataPtr server, int connectTimeout = -1,
            int readTimeout = -1);

  /**
   * Run an SQL and return number of affected rows. Consider DBQuery class,
   * instead of directly calling this function.
   */
  int execute(const std::string &sql, DBDataSet *ds = NULL,
              bool retryQueryOnFail = true);
  int execute(const char *sql, DBDataSet *ds = NULL,
              bool retryQueryOnFail = true);

  /**
   * Returns newly created id from most recent insertion.
   */
  int getLastInsertId();

  /**
   * Query local dbs in parallel. Returns number of total affecected rows.
   * Use "DBID" for any place in the query that needs to be replaced by dbId.
   * For example, "SELECT DBID as dbid, count(*) as count FROM ...".
   */
  static int parallelExecute
    (const char *sql, DBDataSet &ds, std::map<int, std::string> &errors,
     int maxThread, bool retryQueryOnFail = true,
     int connectTimeout = -1, int readTimeout = -1);
  static int parallelExecute
    (const ServerQueryVec &sqls, DBDataSet &ds,
     std::map<int, std::string> &errors, int maxThread,
     bool retryQueryOnFail = true,
     int connectTimeout = -1, int readTimeout = -1);
  static int parallelExecute
    (const ServerQueryVec &sqls, DBDataSetPtrVec &dss,
     std::map<int, std::string> &errors, int maxThread,
     bool retryQueryOnFail = true,
     int connectTimeout = -1, int readTimeout = -1);

  /**
   * Put a connection back to pool.
   */
  void close();

  /**
   * Whether or not a connection is opened.
   */
  bool isOpened() const { return m_conn != NULL;}

  /**
   * Helper function for escaping strings in SQLs.
   */
  void escapeString(const char *s, std::string &out);
  void escapeString(const char *s, int len, std::string &out);

  static void clearLocalDatabases();
  static void addLocalDB(unsigned int dbId, const char *ip, const char *db,
                         int port, const char *username, const char *password);

 private:
  static Mutex s_mutex;
  static ServerDataPtrVec s_localDatabases;

  MYSQL *m_conn;
  ServerDataPtr m_server;
  unsigned int m_connectTimeout;
  unsigned int m_readTimeout;

  class QueryJob {
  public:
    QueryJob(ServerDataPtr server, const std::string sql, int index,
             Mutex &mutex, DBDataSet &dsResult, bool retryQueryOnFail,
             unsigned int readTimeout, unsigned int connectTimeout)
      : m_server(server), m_sql(sql), m_index(index),
        m_affected(0), m_dsMutex(&mutex), m_dsResult(&dsResult),
        m_retryQueryOnFail(retryQueryOnFail), m_connectTimeout(connectTimeout),
        m_readTimeout(readTimeout) {}

    ServerDataPtr m_server;
    std::string m_sql;
    int m_index;
    int m_affected;
    Mutex *m_dsMutex;
    DBDataSet *m_dsResult;
    std::string m_error;
    bool m_retryQueryOnFail;
    int m_connectTimeout;
    int m_readTimeout;
  };
  DECLARE_BOOST_TYPES(QueryJob);

  class QueryWorker {
  public:
    void onThreadEnter() {}
    void doJob(QueryJobPtr job);
    void onThreadExit() { my_thread_end();}
  };

  static int parallelExecute(QueryJobPtrVec &jobs,
                             std::map<int, std::string> &errors,
                             int maxThread);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __DB_CONN_H__
