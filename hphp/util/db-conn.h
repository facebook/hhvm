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

#ifndef incl_HPHP_DB_CONN_H_
#define incl_HPHP_DB_CONN_H_

#include "hphp/util/db-dataset.h"
#include "hphp/util/exception.h"
#include "hphp/util/mutex.h"
#include "hphp/util/async-job.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class DatabaseException : public Exception {
public:
  DatabaseException(int code, const char *fmt, ...) ATTRIBUTE_PRINTF(3,4);
  int m_code;
  EXCEPTION_COMMON_IMPL(DatabaseException);
};

class DBConnectionException : public DatabaseException {
public:
  DBConnectionException(int code, const char *ip, const char *database,
                        const char *msg)
      : DatabaseException(code, "Failed to connect to %s %s: %s (%d)",
                          ip, database, msg, code) {
  }
  EXCEPTION_COMMON_IMPL(DBConnectionException);
};

///////////////////////////////////////////////////////////////////////////////

/**
 * What it takes to connect to a MySQL server.
 */
DECLARE_BOOST_TYPES(ServerData);
DECLARE_BOOST_TYPES(DBConnQueryJob);
template <> class WorkerInfo<DBConnQueryJob> {
 public:
  enum { DoInit = false };
};

typedef std::vector<std::pair<std::string, std::string> > SessionVariableVec;

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
  ServerData(const char *ip, const char *database, int port,
             const char *username, const char *password,
             const SessionVariableVec &sessionVariables);

  const std::string &getIP() const { return m_ip;}
  int getPort() const;
  const std::string &getUserName() const;
  const std::string &getPassword() const;
  const std::string &getDatabase() const { return m_database;}
  const SessionVariableVec &getSessionVariables() const
    { return m_sessionVariables; }

 private:
  std::string m_ip;
  int m_port;
  std::string m_username;
  std::string m_password;
  std::string m_database;
  SessionVariableVec m_sessionVariables;
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
  explicit DBConn(int maxRetryOpenOnFail = 0, int maxRetryQueryOnFail = 1);
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
  int execute(const std::string &sql, DBDataSet *ds = nullptr,
              bool retryQueryOnFail = true);
  int execute(const char *sql, DBDataSet *ds = nullptr,
              bool retryQueryOnFail = true);

  /**
   * Returns newly created id from most recent insertion.
   */
  int getLastInsertId();

  class ErrorInfo {
  public:
    ErrorInfo() : code(0) {}
    int code;
    std::string msg;
  };
  typedef std::map<int, ErrorInfo> ErrorInfoMap;

  /**
   * Query local dbs in parallel. Returns number of total affecected rows.
   * Use "DBID" for any place in the query that needs to be replaced by dbId.
   * For example, "SELECT DBID as dbid, count(*) as count FROM ...".
   */
  static int parallelExecute
    (const char *sql, DBDataSet &ds,
     ErrorInfoMap &errors, int maxThread, bool retryQueryOnFail = true,
     int connectTimeout = -1, int readTimeout = -1,
     int maxRetryOpenOnFail = 1, int maxRetryQueryOnFail = 1
     );
  static int parallelExecute
    (const ServerQueryVec &sqls, DBDataSet &ds,
     ErrorInfoMap &errors, int maxThread, bool retryQueryOnFail = true,
     int connectTimeout = -1, int readTimeout = -1,
     int maxRetryOpenOnFail = 1, int maxRetryQueryOnFail = 1);
  static int parallelExecute
    (const ServerQueryVec &sqls, DBDataSetPtrVec &dss,
     ErrorInfoMap &errors, int maxThread, bool retryQueryOnFail = true,
     int connectTimeout = -1, int readTimeout = -1,
     int maxRetryOpenOnFail = 1, int maxRetryQueryOnFail = 1);

  /**
   * Put a connection back to pool.
   */
  void close();

  /**
   * Whether or not a connection is opened.
   */
  bool isOpened() const { return m_conn != nullptr;}

  /**
   * Helper function for escaping strings in SQLs.
   */
  void escapeString(const char *s, std::string &out);
  void escapeString(const char *s, int len, std::string &out);

  static void ClearLocalDatabases();
  static void AddLocalDB(int dbId, const char *ip, const char *db,
                         int port, const char *username, const char *password,
                         const SessionVariableVec &sessionVariables);

 private:
  static Mutex s_mutex;
  typedef std::map<int, ServerDataPtr> DatabaseMap;
  static DatabaseMap s_localDatabases;

  MYSQL *m_conn;
  ServerDataPtr m_server;
  unsigned int m_connectTimeout;
  unsigned int m_readTimeout;
  int m_maxRetryOpenOnFail;

  static int parallelExecute(DBConnQueryJobPtrVec &jobs, ErrorInfoMap &errors,
                             int maxThread);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_DB_CONN_H_
