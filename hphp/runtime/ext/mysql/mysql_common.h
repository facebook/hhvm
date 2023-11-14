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

#pragma once

#include "hphp/runtime/base/req-list.h"
#include "hphp/runtime/base/req-optional.h"
#include "hphp/runtime/base/req-vector.h"
#include "hphp/runtime/base/request-event-handler.h"
#include "hphp/runtime/ext/extension.h"

#include "mysql.h"

#include "squangle/mysql_client/SSLOptionsProviderBase.h"

#ifdef PHP_MYSQL_UNIX_SOCK_ADDR
#ifdef MYSQL_UNIX_ADDR
#undef MYSQL_UNIX_ADDR
#endif
#define MYSQL_UNIX_ADDR PHP_MYSQL_UNIX_SOCK_ADDR
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#if MYSQL_VERSION_ID >= 80004
using my_bool = bool;
#endif

struct MySQLUtil {
  enum TimeoutType { ConnectTimeout, ReadTimeout, WriteTimeout };

  static int
  set_mysql_timeout(MYSQL* mysql, MySQLUtil::TimeoutType type, int ms);
};

enum class MySQLState : int8_t {
  CLOSED = 0,
  INITED = 1,
  CONNECTED = 2
};

struct MySQL {

  /////////////////////////////////////////////////////////////////////////////

  MySQL(const char *host, int port, const char *username,
        const char *password, const char *database,
        MYSQL* raw_connection = nullptr);

  ~MySQL() { close(); }

  void setLastError(const char *func);
  void close();

  bool connect(const String& host, int port,
               const String& socket,
               const String& username,
               const String& password,
               const String& database,
               int client_flags,
               int connect_timeout);

  bool reconnect(const String& host, int port,
                 const String& socket,
                 const String& username,
                 const String& password,
                 const String& database,
                 int client_flags,
                 int connect_timeout);

  /////////////////////////////////////////////////////////////////////////////

  bool isPersistent() { return m_persistent; }
  void setPersistent() { m_persistent = true; }

  MySQLState getState() { return m_state; }
  MYSQL* get() { return m_conn;}

  MYSQL* eject_mysql() {
    auto ret = m_conn;
    m_conn = nullptr;
    return ret;
  }

  /////////////////////////////////////////////////////////////////////////////

  /**
   * Operations on a resource object.
   */
  static std::shared_ptr<MySQL> Get(const Variant& link_identifier);

  static MYSQL* GetConn(const Variant& link_identifier,
                        std::shared_ptr<MySQL>* rconn = nullptr);

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
  static std::shared_ptr<MySQL> GetPersistent(const String& host, int port,
                                              const String& socket,
                                              const String& username,
                                              const String& password,
                                              int client_flags) {
    return GetCachedImpl(host, port, socket, username, password, client_flags);
  }

  static void SetPersistent(const String& host, int port,
                            const String& socket,
                            const String& username,
                            const String& password,
                            int client_flags,
                            const std::shared_ptr<MySQL>& conn) {
    SetCachedImpl(host, port, socket, username, password, client_flags, conn);
  }

  /**
   * If connection object is not provided, a default connection will be used.
   */
  static std::shared_ptr<MySQL> GetDefaultConn();
  static void SetDefaultConn(std::shared_ptr<MySQL> conn);

  static int GetDefaultReadTimeout();
  static void SetDefaultReadTimeout(int timeout_ms);

  static size_t NumCachedConnections();
  /////////////////////////////////////////////////////////////////////////////

private:
  static int s_default_port;
  static bool s_allow_reconnect;
  static bool s_allow_persistent;
  static int s_cur_num_persistent;
  static int s_max_num_persistent;
  static const std::string s_persistent_type;

  static std::string GetHash(const String& host, int port,
                             const String& socket,
                             const String& username,
                             const String& password,
                             int client_flags);

  static std::shared_ptr<MySQL> GetCachedImpl(const String& host, int port,
                                              const String& socket,
                                              const String& username,
                                              const String& password,
                                              int client_flags);

  static void SetCachedImpl(const String& host, int port,
                            const String& socket,
                            const String& username,
                            const String& password,
                            int client_flags,
                            std::shared_ptr<MySQL> conn);

  /////////////////////////////////////////////////////////////////////////////
public:
  // Global MySQL settings
  static bool IsAllowReconnect() { return s_allow_reconnect; }
  static void SetAllowReconnect(bool allow_reconnect) {
    s_allow_reconnect = allow_reconnect;
  }
  static bool IsAllowPersistent() { return s_allow_persistent; }
  static void SetAllowPersistent(bool allow_persistent) {
    s_allow_persistent = allow_persistent;
  }
  static int GetMaxNumPersistent() { return s_max_num_persistent; }
  static void SetMaxNumPersistent(int max_num_persistent) {
    s_max_num_persistent = max_num_persistent;
  }
  // Ongoing settings
  static int GetCurrentNumPersistent() { return s_cur_num_persistent; }
  static void SetCurrentNumPersistent(int num) {
    s_cur_num_persistent = num;
  }
private:
  MYSQL* m_conn;
  bool m_persistent{false};

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
  MySQLState m_state;
  std::string m_async_query;
};

///////////////////////////////////////////////////////////////////////////////

struct MySQLResource : SweepableResourceData {
  explicit MySQLResource(std::shared_ptr<MySQL> mysql) : m_mysql(mysql) {
    assertx(mysql);
  }

  CLASSNAME_IS("mysql link")
  DECLARE_RESOURCE_ALLOCATION(MySQLResource);

  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }
  bool isInvalid() const override { return m_mysql->get() == nullptr; }

  std::shared_ptr<MySQL> mysql() const { return m_mysql; }

private:
  std::shared_ptr<MySQL> m_mysql;
};

///////////////////////////////////////////////////////////////////////////////

struct MySQLRequestData final : RequestEventHandler {
  void requestInit() override;
  void requestShutdown() override {
    defaultConn.reset();
    totalRowCount = 0;
  }

  req::ptr<MySQLResource> defaultConn;
  int readTimeout;
  int totalRowCount; // from all queries in current request

  static MySQLRequestData s_mysql_data;
};

///////////////////////////////////////////////////////////////////////////////

struct MySQLFieldInfo {
  String name;
  String org_name;
  String table;
  String org_table;
  String def;
  String db;
  int64_t max_length{0};
  int64_t length{0};
  int type{0};
  unsigned int flags{0};
  unsigned int decimals{0};
  unsigned int charsetnr{0};
};

///////////////////////////////////////////////////////////////////////////////

struct MySQLResult : SweepableResourceData {
  DECLARE_RESOURCE_ALLOCATION(MySQLResult);

  explicit MySQLResult(MYSQL_RES *res, bool localized = false);
  ~MySQLResult() override;

  CLASSNAME_IS("mysql result")
  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  void close() {
    sweep();
    if (isLocalized()) {
      m_rows.reset();
    }
  }

  bool isInvalid() const override {
    if (isLocalized()) {
      return !m_rows.has_value();
    }
    return m_res == nullptr;
  }

  MYSQL_RES *get() {
    return m_res;
  }

  bool isLocalized() const {
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

  void setAsyncConnection(const std::shared_ptr<MySQL>& conn) {
    m_conn = conn;
  }

protected:
  MYSQL_RES *m_res;
  MYSQL_ROW m_current_async_row;
  bool m_localized; // whether all the rows have been localized
  req::vector<MySQLFieldInfo> m_fields;
  req::Optional<req::list<req::vector<Variant>>> m_rows;
  req::list<req::vector<Variant>>::const_iterator m_current_row;
  int64_t m_current_field;
  bool m_row_ready; // set to false after seekRow, true after fetchRow
  int64_t m_row_count;
  std::shared_ptr<MySQL> m_conn;  // only set for async for
                                  // refcounting underlying buffers
};

///////////////////////////////////////////////////////////////////////////////

struct MySQLStmt : public SweepableResourceData {
  DECLARE_RESOURCE_ALLOCATION(MySQLStmt);

  explicit MySQLStmt(MYSQL *mysql);
  ~MySQLStmt() override;

  CLASSNAME_IS("mysql stmt")

  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  Variant close();

  MYSQL_STMT *get() { return m_stmt; }

  Variant affected_rows();
  Variant attr_get(int64_t attr);
  Variant attr_set(int64_t attr, int64_t value);
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
  Variant sqlstate();
  Variant store_result();

protected:
  MYSQL_STMT *m_stmt;
  bool m_prepared;
};

///////////////////////////////////////////////////////////////////////////////
// helper

req::ptr<MySQLResult> php_mysql_extract_result(const OptResource& result);
req::ptr<MySQLResult> php_mysql_extract_result(const Variant& result);


enum MySQLFieldEntryType { NAME, TABLE, LEN, TYPE, FLAGS };
#define PHP_MYSQL_FIELD_NAME  1
#define PHP_MYSQL_FIELD_TABLE 2
#define PHP_MYSQL_FIELD_LEN   3
#define PHP_MYSQL_FIELD_TYPE  4
#define PHP_MYSQL_FIELD_FLAGS 5

Variant php_mysql_field_info(const OptResource& result, int field, int entry_type);
Variant php_mysql_do_connect_on_link(
    std::shared_ptr<MySQL> mySQL,
    String server,
    String username,
    String password,
    String database,
    int client_flags,
    bool persistent,
    int connect_timeout_ms,
    int query_timeout_ms,
    const Array* conn_attrs = nullptr,
    std::shared_ptr<facebook::common::mysql_client::SSLOptionsProviderBase>
        ssl_opts = nullptr
);

Variant php_mysql_do_connect(
    const String& server,
    const String& username,
    const String& password,
    const String& database,
    int client_flags,
    bool persistent,
    int connect_timeout_ms,
    int query_timeout_ms,
    const Array* conn_attrs = nullptr);

Variant php_mysql_do_connect_with_ssl(
    const String& server,
    const String& username,
    const String& password,
    const String& database,
    int client_flags,
    int connect_timeout_ms,
    int query_timeout_ms,
    const Array* conn_attrs,
    const Variant& sslContextProvider /* = null */);

enum MySQLQueryReturn { FAIL = 0, OK = 1, OK_FETCH_RESULT = 2 };
MySQLQueryReturn php_mysql_do_query(
    const String& query,
    const Variant& link_id);
Variant php_mysql_get_result(const Variant& link_id, bool use_store);
Variant php_mysql_do_query_and_get_result(
    const String& query,
    const Variant& link_id,
    bool use_store);

#define PHP_MYSQL_ASSOC  1 << 0
#define PHP_MYSQL_NUM    1 << 1
#define PHP_MYSQL_BOTH   (PHP_MYSQL_ASSOC|PHP_MYSQL_NUM)

Variant php_mysql_fetch_hash(const OptResource& result, int result_type);

Variant mysql_makevalue(const String& data, MYSQL_FIELD *mysql_field);
Variant mysql_makevalue(const String& data, enum_field_types field_type);
const char *php_mysql_get_field_name(int field_type);

///////////////////////////////////////////////////////////////////////////////
}

