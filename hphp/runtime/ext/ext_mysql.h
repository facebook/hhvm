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

#ifndef incl_HPHP_EXT_MYSQL_H_
#define incl_HPHP_EXT_MYSQL_H_

#include "folly/Optional.h"

#include "hphp/runtime/base/base_includes.h"
#include "mysql.h"
#include "hphp/runtime/base/smart_containers.h"

#ifdef PHP_MYSQL_UNIX_SOCK_ADDR
#ifdef MYSQL_UNIX_ADDR
#undef MYSQL_UNIX_ADDR
#endif
#define MYSQL_UNIX_ADDR PHP_MYSQL_UNIX_SOCK_ADDR
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class MySQL : public SweepableResourceData {
public:
  /**
   * Operations on a resource object.
   */
  static MYSQL *GetConn(CVarRef link_identifier, MySQL **rconn = NULL);
  static MySQL *Get(CVarRef link_identifier);
  static bool CloseConn(CVarRef link_identifier);

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
  static MySQL *GetPersistent(CStrRef host, int port, CStrRef socket,
                              CStrRef username, CStrRef password,
                              int client_flags) {
    return GetCachedImpl("mysql::persistent_conns", host, port, socket,
                         username, password, client_flags);
  }

  static void SetPersistent(CStrRef host, int port, CStrRef socket,
                            CStrRef username, CStrRef password,
                            int client_flags, MySQL *conn) {
    SetCachedImpl("mysql::persistent_conns", host, port, socket,
                  username, password, client_flags, conn);
  }

  /**
   * If connection object is not provided, a default connection will be used.
   */
  static MySQL *GetDefaultConn();
  static void SetDefaultConn(MySQL *conn);

private:
  static int s_default_port;

  static String GetHash(CStrRef host, int port, CStrRef socket,
                        CStrRef username, CStrRef password, int client_flags);

  static MySQL *GetCachedImpl(const char *name, CStrRef host, int port,
                              CStrRef socket, CStrRef username,
                              CStrRef password, int client_flags);

  static void SetCachedImpl(const char *name, CStrRef host, int port,
                            CStrRef socket, CStrRef username, CStrRef password,
                            int client_flags, MySQL *conn);

public:
  MySQL(const char *host, int port, const char *username,
        const char *password, const char *database,
        MYSQL* raw_connection = nullptr);
  ~MySQL();
  void setLastError(const char *func);
  void close();

  static StaticString s_class_name;
  // overriding ResourceData
  virtual CStrRef o_getClassNameHook() const { return s_class_name; }
  virtual bool isInvalid() const { return m_conn == nullptr; }

  bool connect(CStrRef host, int port, CStrRef socket, CStrRef username,
               CStrRef password, CStrRef database, int client_flags,
               int connect_timeout);
#ifdef FACEBOOK
  bool async_connect(CStrRef host, int port, CStrRef socket, CStrRef username,
                     CStrRef password, CStrRef database);
#endif
  bool reconnect(CStrRef host, int port, CStrRef socket, CStrRef username,
                 CStrRef password, CStrRef database, int client_flags,
                 int connect_timeout);

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
  bool m_last_error_set;
  int m_last_errno;
  std::string m_last_error;
  int m_xaction_count;
  bool m_multi_query;
};

///////////////////////////////////////////////////////////////////////////////

class MySQLFieldInfo {
public:
  MySQLFieldInfo()
    : max_length(0), length(0), type(0), flags(0)
  {}

  String name;
  String table;
  String def;
  int64_t max_length;
  int64_t length;
  int type;
  unsigned int flags;
};

class MySQLResult : public SweepableResourceData {
public:
  DECLARE_OBJECT_ALLOCATION(MySQLResult);

  explicit MySQLResult(MYSQL_RES *res, bool localized = false);
  virtual ~MySQLResult();

  static StaticString s_class_name;
  // overriding ResourceData
  virtual CStrRef o_getClassNameHook() const { return s_class_name; }

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
// connection functions

Variant f_mysql_connect(CStrRef server = null_string,
                        CStrRef username = null_string,
                        CStrRef password = null_string,
                        bool new_link = false,
                        int client_flags = 0,
                        int connect_timeout_ms = -1,
                        int query_timeout_ms = -1);
Variant f_mysql_pconnect(CStrRef server = null_string,
                         CStrRef username = null_string,
                         CStrRef password = null_string,
                         int client_flags = 0,
                         int connect_timeout_ms = -1,
                         int query_timeout_ms = -1);

Variant f_mysql_connect_with_db(CStrRef server = null_string,
                        CStrRef username = null_string,
                        CStrRef password = null_string,
                        CStrRef database = null_string,
                        bool new_link = false,
                        int client_flags = 0,
                        int connect_timeout_ms = -1,
                        int query_timeout_ms = -1);
Variant f_mysql_pconnect_with_db(CStrRef server = null_string,
                         CStrRef username = null_string,
                         CStrRef password = null_string,
                         CStrRef database = null_string,
                         int client_flags = 0,
                         int connect_timeout_ms = -1,
                         int query_timeout_ms = -1);

Variant f_mysql_async_connect_start(CStrRef server = null_string,
                                    CStrRef username = null_string,
                                    CStrRef password = null_string,
                                    CStrRef database = null_string);
bool f_mysql_async_connect_completed(CVarRef link_identifier);
bool f_mysql_async_query_start(CStrRef query, CVarRef link_identifier);
Variant f_mysql_async_query_result(CVarRef link_identifier);
bool f_mysql_async_query_completed(CVarRef result);
Variant f_mysql_async_fetch_array(CVarRef result, int result_type = 1);
Variant f_mysql_async_wait_actionable(CVarRef items, double timeout);
int64_t f_mysql_async_status(CVarRef link_identifier);

String f_mysql_escape_string(CStrRef unescaped_string);

Variant f_mysql_real_escape_string(CStrRef unescaped_string,
                                   CVarRef link_identifier = uninit_null());

String f_mysql_get_client_info();
Variant f_mysql_set_charset(CStrRef charset,
                                   CVarRef link_identifier = uninit_null());
Variant f_mysql_ping(CVarRef link_identifier = uninit_null());
Variant f_mysql_client_encoding(CVarRef link_identifier = uninit_null());
Variant f_mysql_close(CVarRef link_identifier = uninit_null());

Variant f_mysql_errno(CVarRef link_identifier = uninit_null());

Variant f_mysql_error(CVarRef link_identifier = uninit_null());

Variant f_mysql_warning_count(CVarRef link_identifier = uninit_null());

Variant f_mysql_get_host_info(CVarRef link_identifier = uninit_null());
Variant f_mysql_get_proto_info(CVarRef link_identifier = uninit_null());
Variant f_mysql_get_server_info(CVarRef link_identifier = uninit_null());
Variant f_mysql_info(CVarRef link_identifier = uninit_null());
Variant f_mysql_insert_id(CVarRef link_identifier = uninit_null());
Variant f_mysql_stat(CVarRef link_identifier = uninit_null());
Variant f_mysql_thread_id(CVarRef link_identifier = uninit_null());
Variant f_mysql_create_db(CStrRef db,
                                 CVarRef link_identifier = uninit_null());
Variant f_mysql_select_db(CStrRef db,
                                 CVarRef link_identifier = uninit_null());
Variant f_mysql_drop_db(CStrRef db,
                               CVarRef link_identifier = uninit_null());
Variant f_mysql_affected_rows(CVarRef link_identifier = uninit_null());

///////////////////////////////////////////////////////////////////////////////
// query functions

Variant mysql_makevalue(CStrRef data, MYSQL_FIELD *mysql_field);
Variant mysql_makevalue(CStrRef data, enum_field_types field_type);

bool f_mysql_set_timeout(int query_timeout_ms = -1,
                         CVarRef link_identifier = uninit_null());

Variant f_mysql_query(CStrRef query, CVarRef link_identifier = uninit_null());
Variant f_mysql_multi_query(CStrRef query, CVarRef link_identifier = uninit_null());

bool f_mysql_next_result(CVarRef link_identifier = uninit_null());

bool f_mysql_more_results(CVarRef link_identifier = uninit_null());

Variant f_mysql_fetch_result(CVarRef link_identifier = uninit_null());

Variant f_mysql_unbuffered_query(CStrRef query,
                                 CVarRef link_identifier = uninit_null());
Variant f_mysql_db_query(CStrRef database, CStrRef query,
                         CVarRef link_identifier = uninit_null());
Variant f_mysql_list_dbs(CVarRef link_identifier = uninit_null());

Variant f_mysql_list_tables(CStrRef database,
                            CVarRef link_identifier = uninit_null());
Variant f_mysql_list_fields(CStrRef database_name, CStrRef table_name,
                            CVarRef link_identifier = uninit_null());
Variant f_mysql_list_processes(CVarRef link_identifier = uninit_null());

///////////////////////////////////////////////////////////////////////////////
// row operations

bool f_mysql_data_seek(CVarRef result, int row);

Variant f_mysql_fetch_row(CVarRef result);

Variant f_mysql_fetch_assoc(CVarRef result);

Variant f_mysql_fetch_array(CVarRef result, int result_type = 3);

Variant f_mysql_fetch_lengths(CVarRef result);

Variant f_mysql_fetch_object(CVarRef result, CStrRef class_name = "stdClass",
                             CArrRef params = uninit_null().toArray());

Variant f_mysql_result(CVarRef result, int row, CVarRef field = null_variant);

///////////////////////////////////////////////////////////////////////////////
// result functions

Variant f_mysql_db_name(CVarRef result, int row,
                               CVarRef field = null_variant);
Variant f_mysql_tablename(CVarRef result, int i);

Variant f_mysql_num_fields(CVarRef result);
Variant f_mysql_num_rows(CVarRef result);
Variant f_mysql_free_result(CVarRef result);

///////////////////////////////////////////////////////////////////////////////
// field info

Variant f_mysql_fetch_field(CVarRef result, int field = -1);
bool f_mysql_field_seek(CVarRef result, int field = 0);
Variant f_mysql_field_name(CVarRef result, int field = 0);
Variant f_mysql_field_table(CVarRef result, int field = 0);
Variant f_mysql_field_len(CVarRef result, int field = 0);
Variant f_mysql_field_type(CVarRef result, int field = 0);
Variant f_mysql_field_flags(CVarRef result, int field = 0);

///////////////////////////////////////////////////////////////////////////////
extern const int64_t k_ASYNC_OP_INVALID;
extern const int64_t k_ASYNC_OP_UNSET;
extern const int64_t k_ASYNC_OP_CONNECT;
extern const int64_t k_ASYNC_OP_QUERY;
extern const int64_t k_ASYNC_OP_FETCH_ROW;

}

#endif // incl_HPHP_EXT_MYSQL_H_
