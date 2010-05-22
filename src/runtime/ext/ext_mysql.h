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

#ifndef __EXT_MYSQL_H__
#define __EXT_MYSQL_H__

#include <runtime/base/base_includes.h>
#include <mysql/mysql.h>

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
        const char *password);
  ~MySQL();
  void setLastError(const char *func);
  void close();

  // overriding ResourceData
  virtual const char *o_getClassName() const { return "MySQL";}
  virtual bool isResource() const { return m_conn != NULL;}

  bool connect(CStrRef host, int port, CStrRef socket, CStrRef username,
               CStrRef password, int client_flags, int connect_timeout);
  bool reconnect(CStrRef host, int port, CStrRef socket, CStrRef username,
                 CStrRef password, int client_flags, int connect_timeout);

  MYSQL *get() { return m_conn;}

private:
  MYSQL *m_conn;

public:
  std::string m_host;
  int m_port;
  std::string m_username;
  std::string m_password;
  bool m_last_error_set;
  int m_last_errno;
  std::string m_last_error;
};

///////////////////////////////////////////////////////////////////////////////

class MySQLFieldInfo {
public:
  MySQLFieldInfo()
    : name(NULL), table(NULL), def(NULL),
      max_length(0), length(0), type(0), flags(0) {}

  Variant *name;
  Variant *table;
  Variant *def;
  int64 max_length;
  int64 length;
  int type;
  unsigned int flags;
};

class MySQLResult : public SweepableResourceData {
public:
  DECLARE_OBJECT_ALLOCATION(MySQLResult);

  MySQLResult(MYSQL_RES *res, bool localized = false);
  virtual ~MySQLResult();

  // overriding ResourceData
  const char *o_getClassName() const { return "MySQLResult";}

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

  void addField(Variant *value);

  void setFieldCount(int64 fields);
  void setFieldInfo(int64 f, MYSQL_FIELD *field);
  MySQLFieldInfo *getFieldInfo(int64 field);

  /**
   * Gets the field content. Only for localized result.
   */
  Variant getField(int64 field) const;

  int64 getFieldCount() const;

  int64 getRowCount() const;

  bool seekRow(int64 row);

  bool fetchRow();

  bool isRowReady() const {
    return m_row_ready;
  }

  bool seekField(int64 field);

  MySQLFieldInfo *fetchFieldInfo();

protected:
  MYSQL_RES *m_res;
  bool m_localized; // whether all the rows have been localized
  MySQLFieldInfo *m_fields;
  std::list<std::vector<Variant *> > *m_rows;
  std::list<std::vector<Variant *> >::const_iterator m_current_row;
  int64 m_current_field;
  bool m_row_ready; // set to false after seekRow, true after fetchRow
  int64 m_field_count;
  int64 m_row_count;
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

String f_mysql_escape_string(CStrRef unescaped_string);

Variant f_mysql_real_escape_string(CStrRef unescaped_string,
                                   CVarRef link_identifier = null);

inline String f_mysql_get_client_info() {
  return String(mysql_get_client_info(), CopyString);
}
inline Variant f_mysql_set_charset(CStrRef charset,
                                   CVarRef link_identifier = null) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return null;
  return !mysql_set_character_set(conn, charset.data());
}
inline Variant f_mysql_ping(CVarRef link_identifier = null) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return null;
  return !mysql_ping(conn);
}
inline Variant f_mysql_client_encoding(CVarRef link_identifier = null) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return String(mysql_character_set_name(conn), CopyString);
}
inline Variant f_mysql_close(CVarRef link_identifier = null) {
  return MySQL::CloseConn(link_identifier);
}

Variant f_mysql_errno(CVarRef link_identifier = null);

Variant f_mysql_error(CVarRef link_identifier = null);

inline Variant f_mysql_get_host_info(CVarRef link_identifier = null) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return String(mysql_get_host_info(conn), CopyString);
}
inline Variant f_mysql_get_proto_info(CVarRef link_identifier = null) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return (int64)mysql_get_proto_info(conn);
}
inline Variant f_mysql_get_server_info(CVarRef link_identifier = null) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return String(mysql_get_server_info(conn), CopyString);
}
inline Variant f_mysql_info(CVarRef link_identifier = null) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return String(mysql_info(conn), CopyString);
}
inline Variant f_mysql_insert_id(CVarRef link_identifier = null) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return mysql_insert_id(conn);
}
inline Variant f_mysql_stat(CVarRef link_identifier = null) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return String(mysql_stat(conn), CopyString);
}
inline Variant f_mysql_thread_id(CVarRef link_identifier = null) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return (int64)mysql_thread_id(conn);
}
inline Variant f_mysql_create_db(CStrRef db,
                                 CVarRef link_identifier = null) {
  throw NotSupportedException
    (__func__, "Deprecated. Use mysql_query(CREATE DATABASE) instead.");
}
inline Variant f_mysql_select_db(CStrRef db,
                                 CVarRef link_identifier = null) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return mysql_select_db(conn, db.data()) == 0;
}
inline Variant f_mysql_drop_db(CStrRef db,
                               CVarRef link_identifier = null) {
  throw NotSupportedException
    (__func__, "Deprecated. Use mysql_query(DROP DATABASE) instead.");
}
inline Variant f_mysql_affected_rows(CVarRef link_identifier = null) {
  MYSQL *conn = MySQL::GetConn(link_identifier);
  if (!conn) return false;
  return mysql_affected_rows(conn);
}

///////////////////////////////////////////////////////////////////////////////
// query functions

bool f_mysql_set_timeout(int query_timeout_ms = -1,
                         CVarRef link_identifier = null);

Variant f_mysql_query(CStrRef query, CVarRef link_identifier = null);

Variant f_mysql_unbuffered_query(CStrRef query,
                                 CVarRef link_identifier = null);
inline Variant f_mysql_db_query(CStrRef database, CStrRef query,
                                CVarRef link_identifier = null) {
  throw NotSupportedException
    (__func__, "Deprecated. Use mysql_query() instead.");
}
Variant f_mysql_list_dbs(CVarRef link_identifier = null);

Variant f_mysql_list_tables(CStrRef database,
                            CVarRef link_identifier = null);
inline Variant f_mysql_list_fields(CStrRef database_name, CStrRef table_name,
                                   CVarRef link_identifier = null) {
  throw NotSupportedException
    (__func__, "Deprecated. Use mysql_query(SHOW COLUMNS FROM table "
     "[LIKE 'name']) instead.");
}
Variant f_mysql_list_processes(CVarRef link_identifier = null);

///////////////////////////////////////////////////////////////////////////////
// row operations

bool f_mysql_data_seek(CVarRef result, int row);

Variant f_mysql_fetch_row(CVarRef result);

Variant f_mysql_fetch_assoc(CVarRef result);

Variant f_mysql_fetch_array(CVarRef result, int result_type = 3);

Variant f_mysql_fetch_lengths(CVarRef result);

Variant f_mysql_fetch_object(CVarRef result, CStrRef class_name = "stdClass",
                             CArrRef params = null);

Variant f_mysql_result(CVarRef result, int row, CVarRef field = null_variant);

///////////////////////////////////////////////////////////////////////////////
// result functions

inline Variant f_mysql_db_name(CVarRef result, int row,
                               CVarRef field = null_variant) {
  return f_mysql_result(result, row, field);
}
inline Variant f_mysql_tablename(CVarRef result, int i) {
  return f_mysql_result(result, i);
}

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
}

#endif // __EXT_MYSQL_H__
