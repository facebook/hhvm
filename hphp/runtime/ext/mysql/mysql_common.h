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

#ifndef incl_HPHP_MYSQL_COMMON_H_
#define incl_HPHP_MYSQL_COMMON_H_

#include "folly/Optional.h"
#include <vector>

#include "hphp/runtime/base/base-includes.h"
#include "mysql.h"
#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/base/request-event-handler.h"

#ifdef PHP_MYSQL_UNIX_SOCK_ADDR
#ifdef MYSQL_UNIX_ADDR
#undef MYSQL_UNIX_ADDR
#endif
#define MYSQL_UNIX_ADDR PHP_MYSQL_UNIX_SOCK_ADDR
#endif

namespace HPHP {

enum MySQLState { CLOSED = 0, INITED = 1, CONNECTED = 2 };

class MySQL : public SweepableResourceData {
public:
  /**
   * Operations on a resource object.
   */
  static MYSQL *GetConn(const Variant& link_identifier, MySQL **rconn = NULL);
  static MySQL *Get(const Variant& link_identifier);
  static bool CloseConn(const Variant& link_identifier);

  /**
   * Default settings.
   */
  static String GetDefaultServer()   { return String();}
  static int GetDefaultPort();
  static String GetDefaultSocket();
  static String GetDefaultUsername() { return String();}
  static String GetDefaultPassword() { return String();}
  static String GetDefaultDatabase() { return String();}

  /**
   * A connection may be persistent across multiple HTTP requests.
   */
  static MySQL *GetPersistent(const String& host, int port, const String& socket,
                              const String& username, const String& password,
                              int client_flags) {
    return GetCachedImpl("mysql::persistent_conns", host, port, socket,
                         username, password, client_flags);
  }

  static void SetPersistent(const String& host, int port, const String& socket,
                            const String& username, const String& password,
                            int client_flags, MySQL *conn) {
    SetCachedImpl("mysql::persistent_conns", host, port, socket,
                  username, password, client_flags, conn);
  }

  /**
   * If connection object is not provided, a default connection will be used.
   */
  static MySQL *GetDefaultConn();
  static void SetDefaultConn(MySQL *conn);

  static int GetDefaultReadTimeout();
  static void SetDefaultReadTimeout(int timeout_ms);

private:
  static int s_default_port;

  static String GetHash(const String& host, int port, const String& socket,
                        const String& username, const String& password, int client_flags);

  static MySQL *GetCachedImpl(const char *name, const String& host, int port,
                              const String& socket, const String& username,
                              const String& password, int client_flags);

  static void SetCachedImpl(const char *name, const String& host, int port,
                            const String& socket, const String& username, const String& password,
                            int client_flags, MySQL *conn);

public:
  MySQL(const char *host, int port, const char *username,
        const char *password, const char *database,
        MYSQL* raw_connection = nullptr);
  ~MySQL();
  void sweep() FOLLY_OVERRIDE;
  void setLastError(const char *func);
  void close();

  CLASSNAME_IS("mysql link")
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }
  virtual bool isInvalid() const { return m_conn == nullptr; }

  bool connect(const String& host, int port, const String& socket, const String& username,
               const String& password, const String& database, int client_flags,
               int connect_timeout);
#ifdef FACEBOOK
  bool async_connect(const String& host, int port, const String& socket, const String& username,
                     const String& password, const String& database);
#endif
  bool reconnect(const String& host, int port, const String& socket, const String& username,
                 const String& password, const String& database, int client_flags,
                 int connect_timeout);

  MySQLState getState() { return m_state; }

  MYSQL *get() { return m_conn;}
  MYSQL *eject_mysql() {
    auto ret = m_conn;
    m_conn = nullptr;
    return ret;
  }

private:
  MYSQL *m_conn;

public:
  std::string m_host;
  int m_port;
  std::string m_username;
  std::string m_password;
  std::string m_database;
  std::string m_socket;
  bool m_last_error_set;
  int m_last_errno;
  std::string m_last_error;
  int m_xaction_count;
  bool m_multi_query;
  String m_async_query;
  MySQLState m_state;
};

///////////////////////////////////////////////////////////////////////////////

struct MySQLRequestData final : RequestEventHandler {
  void requestInit() override;
  void requestShutdown() override {
    defaultConn.reset();
    totalRowCount = 0;
  }

  Resource defaultConn;
  int readTimeout;
  int totalRowCount; // from all queries in current request

  static MySQLRequestData s_mysql_data;
};

///////////////////////////////////////////////////////////////////////////////

class MySQLFieldInfo {
public:
  MySQLFieldInfo()
    : max_length(0), length(0), type(0), flags(0), decimals(0), charsetnr(0)
  {}

  String name;
  String org_name;
  String table;
  String org_table;
  String def;
  String db;
  int64_t max_length;
  int64_t length;
  int type;
  unsigned int flags;
  unsigned int decimals;
  unsigned int charsetnr;
};

///////////////////////////////////////////////////////////////////////////////

class MySQLResult : public SweepableResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION(MySQLResult);

  explicit MySQLResult(MYSQL_RES *res, bool localized = false);
  virtual ~MySQLResult();

  CLASSNAME_IS("mysql result")
  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }

  void close() {
    if (m_res) {
      mysql_free_result(m_res);
      m_res = NULL;
    }
  }

  MYSQL_RES *get() {
    return m_res;
  }

  bool isLocalized() {
    return m_localized;
  }

  void addRow();

  void addField(Variant&& value);

  void setFieldCount(int64_t fields);
  void setFieldInfo(int64_t f, MYSQL_FIELD *field);
  MySQLFieldInfo *getFieldInfo(int64_t field);

  /**
   * Gets the field content. Only for localized result.
   */
  Variant getField(int64_t field) const;

  int64_t getFieldCount() const;

  int64_t getRowCount() const;

  bool seekRow(int64_t row);

  bool fetchRow();

  bool isRowReady() const {
    return m_row_ready;
  }

  bool seekField(int64_t field);
  int64_t tellField();

  MySQLFieldInfo *fetchFieldInfo();

  void setAsyncConnection(MySQL* conn) {
    m_conn = conn;
    m_conn->incRefCount();
  }

protected:
  MYSQL_RES *m_res;
  MYSQL_ROW m_current_async_row;
  bool m_localized; // whether all the rows have been localized
  MySQLFieldInfo *m_fields;
  folly::Optional<smart::list<smart::vector<Variant>>> m_rows;
  smart::list<smart::vector<Variant>>::const_iterator m_current_row;
  int64_t m_current_field;
  bool m_row_ready; // set to false after seekRow, true after fetchRow
  int64_t m_field_count;
  int64_t m_row_count;
  MySQL* m_conn;  // only set for async for refcounting underlying buffers
};

///////////////////////////////////////////////////////////////////////////////

class MySQLStmtVariables {
public:
  explicit MySQLStmtVariables(std::vector<Variant*> arr);
  ~MySQLStmtVariables();

  bool init_params(MYSQL_STMT *stmt, const String& types);
  bool bind_result(MYSQL_STMT *stmt);
  bool bind_params(MYSQL_STMT *stmt);
  void update_result();

private:
  std::vector<Variant*>  m_arr;
  std::vector<Variant>   m_value_arr;
  MYSQL_BIND            *m_vars;
  my_bool               *m_null;
  unsigned long         *m_length;
};

///////////////////////////////////////////////////////////////////////////////

class MySQLStmt : public SweepableResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION(MySQLStmt);

  explicit MySQLStmt(MYSQL *mysql);
  virtual ~MySQLStmt();

  CLASSNAME_IS("mysql stmt")

  // overriding ResourceData
  virtual const String& o_getClassNameHook() const { return classnameof(); }

  Variant close();

  MYSQL_STMT *get() { return m_stmt; }

  Variant affected_rows();
  Variant attr_get(int64_t attr);
  Variant attr_set(int64_t attr, int64_t value);
  Variant bind_param(const String& types, std::vector<Variant*> vars);
  Variant bind_result(std::vector<Variant*> vars);
  Variant data_seek(int64_t offset);
  Variant get_errno();
  Variant get_error();
  Variant execute();
  Variant fetch();
  Variant field_count();
  Variant free_result();
  Variant insert_id();
  Variant num_rows();
  Variant param_count();
  Variant prepare(const String& query);
  Variant reset();
  Variant result_metadata();
  Variant send_long_data(int64_t param_idx, const String& data);
  Variant store_result();

protected:
  MYSQL_STMT *m_stmt;
  bool m_prepared;
  MySQLStmtVariables *m_param_vars;
  MySQLStmtVariables *m_result_vars;
};

///////////////////////////////////////////////////////////////////////////////
// helper

MySQLResult *php_mysql_extract_result(const Resource& result);


enum MySQLFieldEntryType { NAME, TABLE, LEN, TYPE, FLAGS };
#define PHP_MYSQL_FIELD_NAME  1
#define PHP_MYSQL_FIELD_TABLE 2
#define PHP_MYSQL_FIELD_LEN   3
#define PHP_MYSQL_FIELD_TYPE  4
#define PHP_MYSQL_FIELD_FLAGS 5

Variant php_mysql_field_info(const Resource& result, int field, int entry_type);
Variant php_mysql_do_connect_on_link(MySQL* mySQL, String server,
                                     String username, String password,
                                     String database, int client_flags,
                                     bool persistent, bool async,
                                     int connect_timeout_ms,
                                     int query_timeout_ms);
Variant php_mysql_do_connect(const String& server, const String& username,
                             const String& password, const String& database,
                             int client_flags, bool persistent,
                             bool async,
                             int connect_timeout_ms,
                             int query_timeout_ms);

enum MySQLQueryReturn { FAIL = 0, OK = 1, OK_FETCH_RESULT = 2 };
MySQLQueryReturn php_mysql_do_query(const String& query, const Variant& link_id,
                                    bool async_mode);
Variant php_mysql_get_result(const Variant& link_id, bool use_store);
Variant php_mysql_do_query_and_get_result(const String& query, const Variant& link_id,
                                          bool use_store, bool async_mode);

#define PHP_MYSQL_ASSOC  1 << 0
#define PHP_MYSQL_NUM    1 << 1
#define PHP_MYSQL_BOTH   (PHP_MYSQL_ASSOC|PHP_MYSQL_NUM)

Variant php_mysql_fetch_hash(const Resource& result, int result_type);

Variant mysql_makevalue(const String& data, MYSQL_FIELD *mysql_field);
Variant mysql_makevalue(const String& data, enum_field_types field_type);
const char *php_mysql_get_field_name(int field_type);

///////////////////////////////////////////////////////////////////////////////

extern const int64_t k_ASYNC_OP_INVALID;
extern const int64_t k_ASYNC_OP_UNSET;
extern const int64_t k_ASYNC_OP_CONNECT;
extern const int64_t k_ASYNC_OP_QUERY;
extern const int64_t k_ASYNC_OP_FETCH_ROW;

}

#endif // incl_HPHP_MYSQL_COMMON_H_
