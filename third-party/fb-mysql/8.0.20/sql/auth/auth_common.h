/* Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef AUTH_COMMON_INCLUDED
#define AUTH_COMMON_INCLUDED

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "lex_string.h"
#include "my_command.h"
#include "my_dbug.h"
#include "my_hostname.h"  // HOSTNAME_LENGTH
#include "my_inttypes.h"
#include "mysql_com.h"  // USERNAME_LENGTH
#include "template_utils.h"

/* Forward Declarations */
class Alter_info;
class Field_iterator_table_ref;
class Item;
class LEX_COLUMN;
class String;
class THD;
struct CHARSET_INFO;
struct GRANT_INFO;
struct GRANT_INTERNAL_INFO;
struct HA_CREATE_INFO;
struct LEX_USER;
template <class T>
class List;
typedef struct user_conn USER_CONN;
class Security_context;
class ACL_USER;
struct TABLE;
struct MEM_ROOT;
struct TABLE_LIST;
enum class role_enum;
enum class Consumer_type;
class LEX_GRANT_AS;

namespace consts {
extern const std::string mysql;
extern const std::string system_user;
}  // namespace consts

/** user, host tuple which reference either acl_cache or g_default_roles */
typedef std::pair<LEX_CSTRING, LEX_CSTRING> Auth_id_ref;
typedef std::vector<Auth_id_ref> List_of_auth_id_refs;

bool operator<(const Auth_id_ref &a, const Auth_id_ref &b);

enum ACL_internal_access_result {
  /**
    Access granted for all the requested privileges,
    do not use the grant tables.
    This flag is used only for the INFORMATION_SCHEMA privileges,
    for compatibility reasons.
  */
  ACL_INTERNAL_ACCESS_GRANTED,
  /** Access denied, do not use the grant tables. */
  ACL_INTERNAL_ACCESS_DENIED,
  /** No decision yet, use the grant tables. */
  ACL_INTERNAL_ACCESS_CHECK_GRANT
};

/* Classes */

/**
  Per internal table ACL access rules.
  This class is an interface.
  Per table(s) specific access rule should be implemented in a subclass.
  @sa ACL_internal_schema_access
*/
class ACL_internal_table_access {
 public:
  ACL_internal_table_access() {}

  virtual ~ACL_internal_table_access() {}

  /**
    Check access to an internal table.
    When a privilege is granted, this method add the requested privilege
    to save_priv.
    @param want_access the privileges requested
    @param [in, out] save_priv the privileges granted
    @retval ACL_INTERNAL_ACCESS_GRANTED All the requested privileges
      are granted, and saved in save_priv.
    @retval ACL_INTERNAL_ACCESS_DENIED At least one of the requested
      privileges was denied.
    @retval ACL_INTERNAL_ACCESS_CHECK_GRANT No requested privilege
      was denied, and grant should be checked for at least one
      privilege. Requested privileges that are granted, if any, are saved
      in save_priv.
  */
  virtual ACL_internal_access_result check(ulong want_access,
                                           ulong *save_priv) const = 0;
};

/**
  Per internal schema ACL access rules.
  This class is an interface.
  Each per schema specific access rule should be implemented
  in a different subclass, and registered.
  Per schema access rules can control:
  - every schema privileges on schema.*
  - every table privileges on schema.table
  @sa ACL_internal_schema_registry
*/
class ACL_internal_schema_access {
 public:
  ACL_internal_schema_access() {}

  virtual ~ACL_internal_schema_access() {}

  /**
    Check access to an internal schema.
    @param want_access the privileges requested
    @param [in, out] save_priv the privileges granted
    @retval ACL_INTERNAL_ACCESS_GRANTED All the requested privileges
      are granted, and saved in save_priv.
    @retval ACL_INTERNAL_ACCESS_DENIED At least one of the requested
      privileges was denied.
    @retval ACL_INTERNAL_ACCESS_CHECK_GRANT No requested privilege
      was denied, and grant should be checked for at least one
      privilege. Requested privileges that are granted, if any, are saved
      in save_priv.
  */
  virtual ACL_internal_access_result check(ulong want_access,
                                           ulong *save_priv) const = 0;

  /**
    Search for per table ACL access rules by table name.
    @param name the table name
    @return per table access rules, or NULL
  */
  virtual const ACL_internal_table_access *lookup(const char *name) const = 0;
};

/**
  A registry for per internal schema ACL.
  An 'internal schema' is a database schema maintained by the
  server implementation, such as 'performance_schema' and 'INFORMATION_SCHEMA'.
*/
class ACL_internal_schema_registry {
 public:
  static void register_schema(const LEX_CSTRING &name,
                              const ACL_internal_schema_access *access);
  static const ACL_internal_schema_access *lookup(const char *name);
};

/**
  Extension of ACL_internal_schema_access for Information Schema
*/
class IS_internal_schema_access : public ACL_internal_schema_access {
 public:
  IS_internal_schema_access() {}

  ~IS_internal_schema_access() {}

  ACL_internal_access_result check(ulong want_access, ulong *save_priv) const;

  const ACL_internal_table_access *lookup(const char *name) const;
};

/* Data Structures */

extern const std::vector<std::string> global_acls_vector;

enum mysql_db_table_field {
  MYSQL_DB_FIELD_HOST = 0,
  MYSQL_DB_FIELD_DB,
  MYSQL_DB_FIELD_USER,
  MYSQL_DB_FIELD_SELECT_PRIV,
  MYSQL_DB_FIELD_INSERT_PRIV,
  MYSQL_DB_FIELD_UPDATE_PRIV,
  MYSQL_DB_FIELD_DELETE_PRIV,
  MYSQL_DB_FIELD_CREATE_PRIV,
  MYSQL_DB_FIELD_DROP_PRIV,
  MYSQL_DB_FIELD_GRANT_PRIV,
  MYSQL_DB_FIELD_REFERENCES_PRIV,
  MYSQL_DB_FIELD_INDEX_PRIV,
  MYSQL_DB_FIELD_ALTER_PRIV,
  MYSQL_DB_FIELD_CREATE_TMP_TABLE_PRIV,
  MYSQL_DB_FIELD_LOCK_TABLES_PRIV,
  MYSQL_DB_FIELD_CREATE_VIEW_PRIV,
  MYSQL_DB_FIELD_SHOW_VIEW_PRIV,
  MYSQL_DB_FIELD_CREATE_ROUTINE_PRIV,
  MYSQL_DB_FIELD_ALTER_ROUTINE_PRIV,
  MYSQL_DB_FIELD_EXECUTE_PRIV,
  MYSQL_DB_FIELD_EVENT_PRIV,
  MYSQL_DB_FIELD_TRIGGER_PRIV,
  MYSQL_DB_FIELD_COUNT
};

enum mysql_user_table_field {
  MYSQL_USER_FIELD_HOST = 0,
  MYSQL_USER_FIELD_USER,
  MYSQL_USER_FIELD_SELECT_PRIV,
  MYSQL_USER_FIELD_INSERT_PRIV,
  MYSQL_USER_FIELD_UPDATE_PRIV,
  MYSQL_USER_FIELD_DELETE_PRIV,
  MYSQL_USER_FIELD_CREATE_PRIV,
  MYSQL_USER_FIELD_DROP_PRIV,
  MYSQL_USER_FIELD_RELOAD_PRIV,
  MYSQL_USER_FIELD_SHUTDOWN_PRIV,
  MYSQL_USER_FIELD_PROCESS_PRIV,
  MYSQL_USER_FIELD_FILE_PRIV,
  MYSQL_USER_FIELD_GRANT_PRIV,
  MYSQL_USER_FIELD_REFERENCES_PRIV,
  MYSQL_USER_FIELD_INDEX_PRIV,
  MYSQL_USER_FIELD_ALTER_PRIV,
  MYSQL_USER_FIELD_SHOW_DB_PRIV,
  MYSQL_USER_FIELD_SUPER_PRIV,
  MYSQL_USER_FIELD_CREATE_TMP_TABLE_PRIV,
  MYSQL_USER_FIELD_LOCK_TABLES_PRIV,
  MYSQL_USER_FIELD_EXECUTE_PRIV,
  MYSQL_USER_FIELD_REPL_SLAVE_PRIV,
  MYSQL_USER_FIELD_REPL_CLIENT_PRIV,
  MYSQL_USER_FIELD_CREATE_VIEW_PRIV,
  MYSQL_USER_FIELD_SHOW_VIEW_PRIV,
  MYSQL_USER_FIELD_CREATE_ROUTINE_PRIV,
  MYSQL_USER_FIELD_ALTER_ROUTINE_PRIV,
  MYSQL_USER_FIELD_CREATE_USER_PRIV,
  MYSQL_USER_FIELD_EVENT_PRIV,
  MYSQL_USER_FIELD_TRIGGER_PRIV,
  MYSQL_USER_FIELD_CREATE_TABLESPACE_PRIV,
  MYSQL_USER_FIELD_SSL_TYPE,
  MYSQL_USER_FIELD_SSL_CIPHER,
  MYSQL_USER_FIELD_X509_ISSUER,
  MYSQL_USER_FIELD_X509_SUBJECT,
  MYSQL_USER_FIELD_MAX_QUESTIONS,
  MYSQL_USER_FIELD_MAX_UPDATES,
  MYSQL_USER_FIELD_MAX_CONNECTIONS,
  MYSQL_USER_FIELD_MAX_USER_CONNECTIONS,
  MYSQL_USER_FIELD_PLUGIN,
  MYSQL_USER_FIELD_AUTHENTICATION_STRING,
  MYSQL_USER_FIELD_PASSWORD_EXPIRED,
  MYSQL_USER_FIELD_PASSWORD_LAST_CHANGED,
  MYSQL_USER_FIELD_PASSWORD_LIFETIME,
  MYSQL_USER_FIELD_ACCOUNT_LOCKED,
  MYSQL_USER_FIELD_CREATE_ROLE_PRIV,
  MYSQL_USER_FIELD_DROP_ROLE_PRIV,
  MYSQL_USER_FIELD_PASSWORD_REUSE_HISTORY,
  MYSQL_USER_FIELD_PASSWORD_REUSE_TIME,
  MYSQL_USER_FIELD_PASSWORD_REQUIRE_CURRENT,
  MYSQL_USER_FIELD_USER_ATTRIBUTES,
  MYSQL_USER_FIELD_COUNT
};

enum mysql_proxies_priv_table_feild {
  MYSQL_PROXIES_PRIV_FIELD_HOST = 0,
  MYSQL_PROXIES_PRIV_FIELD_USER,
  MYSQL_PROXIES_PRIV_FIELD_PROXIED_HOST,
  MYSQL_PROXIES_PRIV_FIELD_PROXIED_USER,
  MYSQL_PROXIES_PRIV_FIELD_WITH_GRANT,
  MYSQL_PROXIES_PRIV_FIELD_GRANTOR,
  MYSQL_PROXIES_PRIV_FIELD_TIMESTAMP,
  MYSQL_PROXIES_PRIV_FIELD_COUNT
};

enum mysql_procs_priv_table_field {
  MYSQL_PROCS_PRIV_FIELD_HOST = 0,
  MYSQL_PROCS_PRIV_FIELD_DB,
  MYSQL_PROCS_PRIV_FIELD_USER,
  MYSQL_PROCS_PRIV_FIELD_ROUTINE_NAME,
  MYSQL_PROCS_PRIV_FIELD_ROUTINE_TYPE,
  MYSQL_PROCS_PRIV_FIELD_GRANTOR,
  MYSQL_PROCS_PRIV_FIELD_PROC_PRIV,
  MYSQL_PROCS_PRIV_FIELD_TIMESTAMP,
  MYSQL_PROCS_PRIV_FIELD_COUNT
};

enum mysql_columns_priv_table_field {
  MYSQL_COLUMNS_PRIV_FIELD_HOST = 0,
  MYSQL_COLUMNS_PRIV_FIELD_DB,
  MYSQL_COLUMNS_PRIV_FIELD_USER,
  MYSQL_COLUMNS_PRIV_FIELD_TABLE_NAME,
  MYSQL_COLUMNS_PRIV_FIELD_COLUMN_NAME,
  MYSQL_COLUMNS_PRIV_FIELD_TIMESTAMP,
  MYSQL_COLUMNS_PRIV_FIELD_COLUMN_PRIV,
  MYSQL_COLUMNS_PRIV_FIELD_COUNT
};

enum mysql_tables_priv_table_field {
  MYSQL_TABLES_PRIV_FIELD_HOST = 0,
  MYSQL_TABLES_PRIV_FIELD_DB,
  MYSQL_TABLES_PRIV_FIELD_USER,
  MYSQL_TABLES_PRIV_FIELD_TABLE_NAME,
  MYSQL_TABLES_PRIV_FIELD_GRANTOR,
  MYSQL_TABLES_PRIV_FIELD_TIMESTAMP,
  MYSQL_TABLES_PRIV_FIELD_TABLE_PRIV,
  MYSQL_TABLES_PRIV_FIELD_COLUMN_PRIV,
  MYSQL_TABLES_PRIV_FIELD_COUNT
};

enum mysql_role_edges_table_field {
  MYSQL_ROLE_EDGES_FIELD_FROM_HOST = 0,
  MYSQL_ROLE_EDGES_FIELD_FROM_USER,
  MYSQL_ROLE_EDGES_FIELD_TO_HOST,
  MYSQL_ROLE_EDGES_FIELD_TO_USER,
  MYSQL_ROLE_EDGES_FIELD_WITH_ADMIN_OPTION,
  MYSQL_ROLE_EDGES_FIELD_COUNT
};

enum mysql_default_roles_table_field {
  MYSQL_DEFAULT_ROLES_FIELD_HOST = 0,
  MYSQL_DEFAULT_ROLES_FIELD_USER,
  MYSQL_DEFAULT_ROLES_FIELD_DEFAULT_ROLE_HOST,
  MYSQL_DEFAULT_ROLES_FIELD_DEFAULT_ROLE_USER,
  MYSQL_DEFAULT_ROLES_FIELD_COUNT
};

enum mysql_password_history_table_field {
  MYSQL_PASSWORD_HISTORY_FIELD_HOST = 0,
  MYSQL_PASSWORD_HISTORY_FIELD_USER,
  MYSQL_PASSWORD_HISTORY_FIELD_PASSWORD_TIMESTAMP,
  MYSQL_PASSWORD_HISTORY_FIELD_PASSWORD,
  MYSQL_PASSWORD_HISTORY_FIELD_COUNT
};

enum mysql_dynamic_priv_table_field {
  MYSQL_DYNAMIC_PRIV_FIELD_USER = 0,
  MYSQL_DYNAMIC_PRIV_FIELD_HOST,
  MYSQL_DYNAMIC_PRIV_FIELD_PRIV,
  MYSQL_DYNAMIC_PRIV_FIELD_WITH_GRANT_OPTION,
  MYSQL_DYNAMIC_PRIV_FIELD_COUNT
};

/* When we run mysql_upgrade we must make sure that the server can be run
   using previous mysql.user table schema during acl_load.

   User_table_schema is a common interface for the current and the
                              previous mysql.user table schema.
 */
class User_table_schema {
 public:
  virtual uint host_idx() = 0;
  virtual uint user_idx() = 0;
  virtual uint password_idx() = 0;
  virtual uint select_priv_idx() = 0;
  virtual uint insert_priv_idx() = 0;
  virtual uint update_priv_idx() = 0;
  virtual uint delete_priv_idx() = 0;
  virtual uint create_priv_idx() = 0;
  virtual uint drop_priv_idx() = 0;
  virtual uint reload_priv_idx() = 0;
  virtual uint shutdown_priv_idx() = 0;
  virtual uint process_priv_idx() = 0;
  virtual uint file_priv_idx() = 0;
  virtual uint grant_priv_idx() = 0;
  virtual uint references_priv_idx() = 0;
  virtual uint index_priv_idx() = 0;
  virtual uint alter_priv_idx() = 0;
  virtual uint show_db_priv_idx() = 0;
  virtual uint super_priv_idx() = 0;
  virtual uint create_tmp_table_priv_idx() = 0;
  virtual uint lock_tables_priv_idx() = 0;
  virtual uint execute_priv_idx() = 0;
  virtual uint repl_slave_priv_idx() = 0;
  virtual uint repl_client_priv_idx() = 0;
  virtual uint create_view_priv_idx() = 0;
  virtual uint show_view_priv_idx() = 0;
  virtual uint create_routine_priv_idx() = 0;
  virtual uint alter_routine_priv_idx() = 0;
  virtual uint create_user_priv_idx() = 0;
  virtual uint event_priv_idx() = 0;
  virtual uint trigger_priv_idx() = 0;
  virtual uint create_tablespace_priv_idx() = 0;
  virtual uint create_role_priv_idx() = 0;
  virtual uint drop_role_priv_idx() = 0;
  virtual uint ssl_type_idx() = 0;
  virtual uint ssl_cipher_idx() = 0;
  virtual uint x509_issuer_idx() = 0;
  virtual uint x509_subject_idx() = 0;
  virtual uint max_questions_idx() = 0;
  virtual uint max_updates_idx() = 0;
  virtual uint max_connections_idx() = 0;
  virtual uint max_user_connections_idx() = 0;
  virtual uint plugin_idx() = 0;
  virtual uint authentication_string_idx() = 0;
  virtual uint password_expired_idx() = 0;
  virtual uint password_last_changed_idx() = 0;
  virtual uint password_lifetime_idx() = 0;
  virtual uint account_locked_idx() = 0;
  virtual uint password_reuse_history_idx() = 0;
  virtual uint password_reuse_time_idx() = 0;
  // Added in 8.0.13
  virtual uint password_require_current_idx() = 0;
  // Added in 8.0.14
  virtual uint user_attributes_idx() = 0;

  virtual ~User_table_schema() {}
};

/*
  This class describes indices for the current mysql.user table schema.
 */
class User_table_current_schema : public User_table_schema {
 public:
  uint host_idx() { return MYSQL_USER_FIELD_HOST; }
  uint user_idx() { return MYSQL_USER_FIELD_USER; }
  // not available
  uint password_idx() {
    DBUG_ASSERT(0);
    return MYSQL_USER_FIELD_COUNT;
  }
  uint select_priv_idx() { return MYSQL_USER_FIELD_SELECT_PRIV; }
  uint insert_priv_idx() { return MYSQL_USER_FIELD_INSERT_PRIV; }
  uint update_priv_idx() { return MYSQL_USER_FIELD_UPDATE_PRIV; }
  uint delete_priv_idx() { return MYSQL_USER_FIELD_DELETE_PRIV; }
  uint create_priv_idx() { return MYSQL_USER_FIELD_CREATE_PRIV; }
  uint drop_priv_idx() { return MYSQL_USER_FIELD_DROP_PRIV; }
  uint reload_priv_idx() { return MYSQL_USER_FIELD_RELOAD_PRIV; }
  uint shutdown_priv_idx() { return MYSQL_USER_FIELD_SHUTDOWN_PRIV; }
  uint process_priv_idx() { return MYSQL_USER_FIELD_PROCESS_PRIV; }
  uint file_priv_idx() { return MYSQL_USER_FIELD_FILE_PRIV; }
  uint grant_priv_idx() { return MYSQL_USER_FIELD_GRANT_PRIV; }
  uint references_priv_idx() { return MYSQL_USER_FIELD_REFERENCES_PRIV; }
  uint index_priv_idx() { return MYSQL_USER_FIELD_INDEX_PRIV; }
  uint alter_priv_idx() { return MYSQL_USER_FIELD_ALTER_PRIV; }
  uint show_db_priv_idx() { return MYSQL_USER_FIELD_SHOW_DB_PRIV; }
  uint super_priv_idx() { return MYSQL_USER_FIELD_SUPER_PRIV; }
  uint create_role_priv_idx() { return MYSQL_USER_FIELD_CREATE_ROLE_PRIV; }
  uint drop_role_priv_idx() { return MYSQL_USER_FIELD_DROP_ROLE_PRIV; }
  uint create_tmp_table_priv_idx() {
    return MYSQL_USER_FIELD_CREATE_TMP_TABLE_PRIV;
  }
  uint lock_tables_priv_idx() { return MYSQL_USER_FIELD_LOCK_TABLES_PRIV; }
  uint execute_priv_idx() { return MYSQL_USER_FIELD_EXECUTE_PRIV; }
  uint repl_slave_priv_idx() { return MYSQL_USER_FIELD_REPL_SLAVE_PRIV; }
  uint repl_client_priv_idx() { return MYSQL_USER_FIELD_REPL_CLIENT_PRIV; }
  uint create_view_priv_idx() { return MYSQL_USER_FIELD_CREATE_VIEW_PRIV; }
  uint show_view_priv_idx() { return MYSQL_USER_FIELD_SHOW_VIEW_PRIV; }
  uint create_routine_priv_idx() {
    return MYSQL_USER_FIELD_CREATE_ROUTINE_PRIV;
  }
  uint alter_routine_priv_idx() { return MYSQL_USER_FIELD_ALTER_ROUTINE_PRIV; }
  uint create_user_priv_idx() { return MYSQL_USER_FIELD_CREATE_USER_PRIV; }
  uint event_priv_idx() { return MYSQL_USER_FIELD_EVENT_PRIV; }
  uint trigger_priv_idx() { return MYSQL_USER_FIELD_TRIGGER_PRIV; }
  uint create_tablespace_priv_idx() {
    return MYSQL_USER_FIELD_CREATE_TABLESPACE_PRIV;
  }
  uint ssl_type_idx() { return MYSQL_USER_FIELD_SSL_TYPE; }
  uint ssl_cipher_idx() { return MYSQL_USER_FIELD_SSL_CIPHER; }
  uint x509_issuer_idx() { return MYSQL_USER_FIELD_X509_ISSUER; }
  uint x509_subject_idx() { return MYSQL_USER_FIELD_X509_SUBJECT; }
  uint max_questions_idx() { return MYSQL_USER_FIELD_MAX_QUESTIONS; }
  uint max_updates_idx() { return MYSQL_USER_FIELD_MAX_UPDATES; }
  uint max_connections_idx() { return MYSQL_USER_FIELD_MAX_CONNECTIONS; }
  uint max_user_connections_idx() {
    return MYSQL_USER_FIELD_MAX_USER_CONNECTIONS;
  }
  uint plugin_idx() { return MYSQL_USER_FIELD_PLUGIN; }
  uint authentication_string_idx() {
    return MYSQL_USER_FIELD_AUTHENTICATION_STRING;
  }
  uint password_expired_idx() { return MYSQL_USER_FIELD_PASSWORD_EXPIRED; }
  uint password_last_changed_idx() {
    return MYSQL_USER_FIELD_PASSWORD_LAST_CHANGED;
  }
  uint password_lifetime_idx() { return MYSQL_USER_FIELD_PASSWORD_LIFETIME; }
  uint account_locked_idx() { return MYSQL_USER_FIELD_ACCOUNT_LOCKED; }
  uint password_reuse_history_idx() {
    return MYSQL_USER_FIELD_PASSWORD_REUSE_HISTORY;
  }
  uint password_reuse_time_idx() {
    return MYSQL_USER_FIELD_PASSWORD_REUSE_TIME;
  }
  uint password_require_current_idx() {
    return MYSQL_USER_FIELD_PASSWORD_REQUIRE_CURRENT;
  }
  uint user_attributes_idx() { return MYSQL_USER_FIELD_USER_ATTRIBUTES; }
};

/*
  This class describes indices for the old mysql.user table schema.
 */
class User_table_old_schema : public User_table_schema {
 public:
  enum mysql_user_table_field_56 {
    MYSQL_USER_FIELD_HOST_56 = 0,
    MYSQL_USER_FIELD_USER_56,
    MYSQL_USER_FIELD_PASSWORD_56,
    MYSQL_USER_FIELD_SELECT_PRIV_56,
    MYSQL_USER_FIELD_INSERT_PRIV_56,
    MYSQL_USER_FIELD_UPDATE_PRIV_56,
    MYSQL_USER_FIELD_DELETE_PRIV_56,
    MYSQL_USER_FIELD_CREATE_PRIV_56,
    MYSQL_USER_FIELD_DROP_PRIV_56,
    MYSQL_USER_FIELD_RELOAD_PRIV_56,
    MYSQL_USER_FIELD_SHUTDOWN_PRIV_56,
    MYSQL_USER_FIELD_PROCESS_PRIV_56,
    MYSQL_USER_FIELD_FILE_PRIV_56,
    MYSQL_USER_FIELD_GRANT_PRIV_56,
    MYSQL_USER_FIELD_REFERENCES_PRIV_56,
    MYSQL_USER_FIELD_INDEX_PRIV_56,
    MYSQL_USER_FIELD_ALTER_PRIV_56,
    MYSQL_USER_FIELD_SHOW_DB_PRIV_56,
    MYSQL_USER_FIELD_SUPER_PRIV_56,
    MYSQL_USER_FIELD_CREATE_TMP_TABLE_PRIV_56,
    MYSQL_USER_FIELD_LOCK_TABLES_PRIV_56,
    MYSQL_USER_FIELD_EXECUTE_PRIV_56,
    MYSQL_USER_FIELD_REPL_SLAVE_PRIV_56,
    MYSQL_USER_FIELD_REPL_CLIENT_PRIV_56,
    MYSQL_USER_FIELD_CREATE_VIEW_PRIV_56,
    MYSQL_USER_FIELD_SHOW_VIEW_PRIV_56,
    MYSQL_USER_FIELD_CREATE_ROUTINE_PRIV_56,
    MYSQL_USER_FIELD_ALTER_ROUTINE_PRIV_56,
    MYSQL_USER_FIELD_CREATE_USER_PRIV_56,
    MYSQL_USER_FIELD_EVENT_PRIV_56,
    MYSQL_USER_FIELD_TRIGGER_PRIV_56,
    MYSQL_USER_FIELD_CREATE_TABLESPACE_PRIV_56,
    MYSQL_USER_FIELD_SSL_TYPE_56,
    MYSQL_USER_FIELD_SSL_CIPHER_56,
    MYSQL_USER_FIELD_X509_ISSUER_56,
    MYSQL_USER_FIELD_X509_SUBJECT_56,
    MYSQL_USER_FIELD_MAX_QUESTIONS_56,
    MYSQL_USER_FIELD_MAX_UPDATES_56,
    MYSQL_USER_FIELD_MAX_CONNECTIONS_56,
    MYSQL_USER_FIELD_MAX_USER_CONNECTIONS_56,
    MYSQL_USER_FIELD_PLUGIN_56,
    MYSQL_USER_FIELD_AUTHENTICATION_STRING_56,
    MYSQL_USER_FIELD_PASSWORD_EXPIRED_56,
    MYSQL_USER_FIELD_COUNT_56
  };

  uint host_idx() { return MYSQL_USER_FIELD_HOST_56; }
  uint user_idx() { return MYSQL_USER_FIELD_USER_56; }
  uint password_idx() { return MYSQL_USER_FIELD_PASSWORD_56; }
  uint select_priv_idx() { return MYSQL_USER_FIELD_SELECT_PRIV_56; }
  uint insert_priv_idx() { return MYSQL_USER_FIELD_INSERT_PRIV_56; }
  uint update_priv_idx() { return MYSQL_USER_FIELD_UPDATE_PRIV_56; }
  uint delete_priv_idx() { return MYSQL_USER_FIELD_DELETE_PRIV_56; }
  uint create_priv_idx() { return MYSQL_USER_FIELD_CREATE_PRIV_56; }
  uint drop_priv_idx() { return MYSQL_USER_FIELD_DROP_PRIV_56; }
  uint reload_priv_idx() { return MYSQL_USER_FIELD_RELOAD_PRIV_56; }
  uint shutdown_priv_idx() { return MYSQL_USER_FIELD_SHUTDOWN_PRIV_56; }
  uint process_priv_idx() { return MYSQL_USER_FIELD_PROCESS_PRIV_56; }
  uint file_priv_idx() { return MYSQL_USER_FIELD_FILE_PRIV_56; }
  uint grant_priv_idx() { return MYSQL_USER_FIELD_GRANT_PRIV_56; }
  uint references_priv_idx() { return MYSQL_USER_FIELD_REFERENCES_PRIV_56; }
  uint index_priv_idx() { return MYSQL_USER_FIELD_INDEX_PRIV_56; }
  uint alter_priv_idx() { return MYSQL_USER_FIELD_ALTER_PRIV_56; }
  uint show_db_priv_idx() { return MYSQL_USER_FIELD_SHOW_DB_PRIV_56; }
  uint super_priv_idx() { return MYSQL_USER_FIELD_SUPER_PRIV_56; }
  uint create_tmp_table_priv_idx() {
    return MYSQL_USER_FIELD_CREATE_TMP_TABLE_PRIV_56;
  }
  uint lock_tables_priv_idx() { return MYSQL_USER_FIELD_LOCK_TABLES_PRIV_56; }
  uint execute_priv_idx() { return MYSQL_USER_FIELD_EXECUTE_PRIV_56; }
  uint repl_slave_priv_idx() { return MYSQL_USER_FIELD_REPL_SLAVE_PRIV_56; }
  uint repl_client_priv_idx() { return MYSQL_USER_FIELD_REPL_CLIENT_PRIV_56; }
  uint create_view_priv_idx() { return MYSQL_USER_FIELD_CREATE_VIEW_PRIV_56; }
  uint show_view_priv_idx() { return MYSQL_USER_FIELD_SHOW_VIEW_PRIV_56; }
  uint create_routine_priv_idx() {
    return MYSQL_USER_FIELD_CREATE_ROUTINE_PRIV_56;
  }
  uint alter_routine_priv_idx() {
    return MYSQL_USER_FIELD_ALTER_ROUTINE_PRIV_56;
  }
  uint create_user_priv_idx() { return MYSQL_USER_FIELD_CREATE_USER_PRIV_56; }
  uint event_priv_idx() { return MYSQL_USER_FIELD_EVENT_PRIV_56; }
  uint trigger_priv_idx() { return MYSQL_USER_FIELD_TRIGGER_PRIV_56; }
  uint create_tablespace_priv_idx() {
    return MYSQL_USER_FIELD_CREATE_TABLESPACE_PRIV_56;
  }
  uint ssl_type_idx() { return MYSQL_USER_FIELD_SSL_TYPE_56; }
  uint ssl_cipher_idx() { return MYSQL_USER_FIELD_SSL_CIPHER_56; }
  uint x509_issuer_idx() { return MYSQL_USER_FIELD_X509_ISSUER_56; }
  uint x509_subject_idx() { return MYSQL_USER_FIELD_X509_SUBJECT_56; }
  uint max_questions_idx() { return MYSQL_USER_FIELD_MAX_QUESTIONS_56; }
  uint max_updates_idx() { return MYSQL_USER_FIELD_MAX_UPDATES_56; }
  uint max_connections_idx() { return MYSQL_USER_FIELD_MAX_CONNECTIONS_56; }
  uint max_user_connections_idx() {
    return MYSQL_USER_FIELD_MAX_USER_CONNECTIONS_56;
  }
  uint plugin_idx() { return MYSQL_USER_FIELD_PLUGIN_56; }
  uint authentication_string_idx() {
    return MYSQL_USER_FIELD_AUTHENTICATION_STRING_56;
  }
  uint password_expired_idx() { return MYSQL_USER_FIELD_PASSWORD_EXPIRED_56; }

  // those fields are not available in 5.6 db schema
  uint password_last_changed_idx() { return MYSQL_USER_FIELD_COUNT_56; }
  uint password_lifetime_idx() { return MYSQL_USER_FIELD_COUNT_56; }
  uint account_locked_idx() { return MYSQL_USER_FIELD_COUNT_56; }
  uint create_role_priv_idx() { return MYSQL_USER_FIELD_COUNT_56; }
  uint drop_role_priv_idx() { return MYSQL_USER_FIELD_COUNT_56; }
  uint password_reuse_history_idx() { return MYSQL_USER_FIELD_COUNT_56; }
  uint password_reuse_time_idx() { return MYSQL_USER_FIELD_COUNT_56; }
  uint password_require_current_idx() { return MYSQL_USER_FIELD_COUNT_56; }
  uint user_attributes_idx() { return MYSQL_USER_FIELD_COUNT_56; }
};

class User_table_schema_factory {
 public:
  virtual User_table_schema *get_user_table_schema(TABLE *table) {
    return is_old_user_table_schema(table)
               ? implicit_cast<User_table_schema *>(new User_table_old_schema())
               : implicit_cast<User_table_schema *>(
                     new User_table_current_schema());
  }

  virtual bool is_old_user_table_schema(TABLE *table);
  virtual ~User_table_schema_factory() {}
};

extern bool mysql_user_table_is_in_short_password_format;
extern bool disconnect_on_expired_password;
extern const char *any_db;  // Special symbol for check_access
/** controls the extra checks on plugin availability for mysql.user records */

extern bool validate_user_plugins;

/* Function Declarations */

/* sql_authentication */

int set_default_auth_plugin(char *plugin_name, size_t plugin_name_length);
std::string get_default_autnetication_plugin_name();

void acl_log_connect(const char *user, const char *host, const char *auth_as,
                     const char *db, THD *thd,
                     enum enum_server_command command);
int acl_authenticate(THD *thd, enum_server_command command);
bool acl_check_host(THD *thd, const char *host, const char *ip);

/* sql_auth_cache */
bool assert_acl_cache_read_lock(THD *thd);
bool assert_acl_cache_write_lock(THD *thd);

/*
  User Attributes are the once which are defined during CREATE/ALTER/GRANT
  statement. These attributes are divided into following catagories.
*/

#define NONE_ATTR 0L
#define DEFAULT_AUTH_ATTR (1L << 0)    /* update defaults auth */
#define PLUGIN_ATTR (1L << 1)          /* update plugin */
                                       /* authentication_string */
#define SSL_ATTR (1L << 2)             /* ex: SUBJECT,CIPHER.. */
#define RESOURCE_ATTR (1L << 3)        /* ex: MAX_QUERIES_PER_HOUR.. */
#define PASSWORD_EXPIRE_ATTR (1L << 4) /* update password expire col */
#define ACCESS_RIGHTS_ATTR (1L << 5)   /* update privileges */
#define ACCOUNT_LOCK_ATTR (1L << 6)    /* update account lock status */
#define DIFFERENT_PLUGIN_ATTR \
  (1L << 7)                       /* updated plugin with a different value */
#define USER_ATTRIBUTES (1L << 8) /* Request to update user attributes */

/* sql_user */
void log_user(THD *thd, String *str, LEX_USER *user, bool comma);
bool check_change_password(THD *thd, const char *host, const char *user,
                           bool retain_current_password);
bool change_password(THD *thd, LEX_USER *user, const char *password,
                     const char *current_password,
                     bool retain_current_password);
bool mysql_create_user(THD *thd, List<LEX_USER> &list, bool if_not_exists,
                       bool is_role);
bool mysql_alter_user(THD *thd, List<LEX_USER> &list, bool if_exists);
bool mysql_drop_user(THD *thd, List<LEX_USER> &list, bool if_exists,
                     bool drop_role);
bool mysql_rename_user(THD *thd, List<LEX_USER> &list);

/* sql_auth_cache */
void init_acl_memory();
int wild_case_compare(CHARSET_INFO *cs, const char *str, const char *wildstr);
int wild_case_compare(CHARSET_INFO *cs, const char *str, size_t str_len,
                      const char *wildstr, size_t wildstr_len);
bool hostname_requires_resolving(const char *hostname);
bool acl_init(bool dont_read_acl_tables);
bool is_acl_inited();
void acl_free(bool end = false);
bool check_engine_type_for_acl_table(THD *thd, bool mdl_locked);
bool grant_init(bool skip_grant_tables);
void grant_free(void);
bool reload_acl_caches(THD *thd, bool mdl_locked);
ulong acl_get(THD *thd, const char *host, const char *ip, const char *user,
              const char *db, bool db_is_pattern);
bool is_acl_user(THD *thd, const char *host, const char *user);
bool acl_getroot(THD *thd, Security_context *sctx, const char *user,
                 const char *host, const char *ip, const char *db);
bool check_acl_tables_intact(THD *thd, bool mdl_locked);
bool check_acl_tables_intact(THD *thd, TABLE_LIST *tables);
void notify_flush_event(THD *thd);
bool wildcard_db_grant_exists();

/* sql_authorization */
bool skip_grant_tables();
bool mysql_set_active_role_none(THD *thd);
bool mysql_set_role_default(THD *thd);
bool mysql_set_active_role_all(THD *thd, const List<LEX_USER> *except_users);
bool mysql_set_active_role(THD *thd, const List<LEX_USER> *role_list);
bool mysql_grant(THD *thd, const char *db, List<LEX_USER> &list, ulong rights,
                 bool revoke_grant, bool is_proxy,
                 const List<LEX_CSTRING> &dynamic_privilege,
                 bool grant_all_current_privileges, LEX_GRANT_AS *grant_as);
bool mysql_routine_grant(THD *thd, TABLE_LIST *table, bool is_proc,
                         List<LEX_USER> &user_list, ulong rights, bool revoke,
                         bool write_to_binlog);
int mysql_table_grant(THD *thd, TABLE_LIST *table, List<LEX_USER> &user_list,
                      List<LEX_COLUMN> &column_list, ulong rights, bool revoke);
bool check_grant(THD *thd, ulong want_access, TABLE_LIST *tables,
                 bool any_combination_will_do, uint number, bool no_errors);
bool check_grant_column(THD *thd, GRANT_INFO *grant, const char *db_name,
                        const char *table_name, const char *name, size_t length,
                        Security_context *sctx, ulong want_privilege);
bool check_column_grant_in_table_ref(THD *thd, TABLE_LIST *table_ref,
                                     const char *name, size_t length,
                                     ulong want_privilege);
bool check_grant_all_columns(THD *thd, ulong want_access,
                             Field_iterator_table_ref *fields);
bool check_grant_routine(THD *thd, ulong want_access, TABLE_LIST *procs,
                         bool is_proc, bool no_error);
bool check_grant_db(THD *thd, const char *db);
bool acl_check_proxy_grant_access(THD *thd, const char *host, const char *user,
                                  bool with_grant);
void get_privilege_desc(char *to, uint max_length, ulong access);
void get_mqh(THD *thd, const char *user, const char *host, USER_CONN *uc);
ulong get_table_grant(THD *thd, TABLE_LIST *table);
ulong get_column_grant(THD *thd, GRANT_INFO *grant, const char *db_name,
                       const char *table_name, const char *field_name);
bool mysql_show_grants(THD *, LEX_USER *, const List_of_auth_id_refs &, bool,
                       bool);
bool mysql_show_create_user(THD *thd, LEX_USER *user, bool are_both_users_same);
bool mysql_revoke_all(THD *thd, List<LEX_USER> &list);
bool sp_revoke_privileges(THD *thd, const char *sp_db, const char *sp_name,
                          bool is_proc);
bool sp_grant_privileges(THD *thd, const char *sp_db, const char *sp_name,
                         bool is_proc);
void fill_effective_table_privileges(THD *thd, GRANT_INFO *grant,
                                     const char *db, const char *table);
int fill_schema_user_privileges(THD *thd, TABLE_LIST *tables, Item *cond);
int fill_schema_schema_privileges(THD *thd, TABLE_LIST *tables, Item *cond);
int fill_schema_table_privileges(THD *thd, TABLE_LIST *tables, Item *cond);
int fill_schema_column_privileges(THD *thd, TABLE_LIST *tables, Item *cond);
const ACL_internal_schema_access *get_cached_schema_access(
    GRANT_INTERNAL_INFO *grant_internal_info, const char *schema_name);

bool lock_tables_precheck(THD *thd, TABLE_LIST *tables);
bool create_table_precheck(THD *thd, TABLE_LIST *tables,
                           TABLE_LIST *create_table);
bool check_fk_parent_table_access(THD *thd, HA_CREATE_INFO *create_info,
                                  Alter_info *alter_info);
bool check_readonly(THD *thd, bool err_if_readonly);
void err_readonly(THD *thd);

bool is_secure_transport(int vio_type);

bool check_one_table_access(THD *thd, ulong privilege, TABLE_LIST *tables);
bool check_single_table_access(THD *thd, ulong privilege, TABLE_LIST *tables,
                               bool no_errors);
bool check_routine_access(THD *thd, ulong want_access, const char *db,
                          char *name, bool is_proc, bool no_errors);
bool check_some_access(THD *thd, ulong want_access, TABLE_LIST *table);
bool has_full_view_routine_access(THD *thd, const char *db,
                                  const char *definer_user,
                                  const char *definer_host);
bool has_partial_view_routine_access(THD *thd, const char *db,
                                     const char *routine_name, bool is_proc);
bool check_access(THD *thd, ulong want_access, const char *db, ulong *save_priv,
                  GRANT_INTERNAL_INFO *grant_internal_info,
                  bool dont_check_global_grants, bool no_errors);
bool check_table_access(THD *thd, ulong requirements, TABLE_LIST *tables,
                        bool any_combination_of_privileges_will_do, uint number,
                        bool no_errors);
bool check_table_encryption_admin_access(THD *thd);
bool mysql_grant_role(THD *thd, const List<LEX_USER> *users,
                      const List<LEX_USER> *roles, bool with_admin_opt);
bool mysql_revoke_role(THD *thd, const List<LEX_USER> *users,
                       const List<LEX_USER> *roles);
void get_default_roles(const Auth_id_ref &user, List_of_auth_id_refs &list);

bool is_granted_table_access(THD *thd, ulong required_acl, TABLE_LIST *table);

bool mysql_alter_or_clear_default_roles(THD *thd, role_enum role_type,
                                        const List<LEX_USER> *users,
                                        const List<LEX_USER> *roles);
void roles_graphml(THD *thd, String *);
bool has_grant_role_privilege(THD *thd, const LEX_CSTRING &role_name,
                              const LEX_CSTRING &role_host);
Auth_id_ref create_authid_from(const LEX_USER *user);
std::string create_authid_str_from(const LEX_USER *user);
std::pair<std::string, std::string> get_authid_from_quoted_string(
    std::string str);
void append_identifier(String *packet, const char *name, size_t length);
bool is_role_id(LEX_USER *authid);
void shutdown_acl_cache();
bool is_granted_role(LEX_CSTRING user, LEX_CSTRING host, LEX_CSTRING role,
                     LEX_CSTRING role_host);
bool is_mandatory_role(LEX_CSTRING role, LEX_CSTRING role_host,
                       bool *is_mandatory);
bool check_show_access(THD *thd, TABLE_LIST *table);
bool check_global_access(THD *thd, ulong want_access);

/* sql_user_table */
void commit_and_close_mysql_tables(THD *thd);

typedef enum ssl_artifacts_status {
  SSL_ARTIFACTS_NOT_FOUND = 0,
  SSL_ARTIFACTS_VIA_OPTIONS,
  SSL_ARTIFACT_TRACES_FOUND,
  SSL_ARTIFACTS_AUTO_DETECTED
} ssl_artifacts_status;

ulong get_global_acl_cache_size();
extern bool opt_auto_generate_certs;
bool do_auto_cert_generation(ssl_artifacts_status auto_detection_status,
                             const char **ssl_ca, const char **ssl_key,
                             const char **ssl_cert);

#define DEFAULT_SSL_CA_CERT "ca.pem"
#define DEFAULT_SSL_CA_KEY "ca-key.pem"
#define DEFAULT_SSL_SERVER_CERT "server-cert.pem"
#define DEFAULT_SSL_SERVER_KEY "server-key.pem"

void update_mandatory_roles(void);
bool check_authorization_id_string(THD *thd, LEX_STRING &mandatory_roles);
void func_current_role(const THD *thd, String *active_role);

extern uint32 global_password_history, global_password_reuse_interval;

struct Security_context_policy {
  enum Operation { Precheck, Execute };
  Security_context_policy() = default;
  virtual ~Security_context_policy() = default;
  Security_context_policy(const Security_context_policy &) = default;
  virtual bool operator()(Security_context *, Operation) = 0;
};

typedef std::function<bool(Security_context *,
                           Security_context_policy::Operation)>
    Security_context_functor;

template <class Derived>
class Create_authid : public Security_context_policy {
 public:
  bool operator()(Security_context *sctx, Operation op) {
    if (op == Precheck && static_cast<Derived *>(this)->precheck(sctx))
      return true;
    if (op == Execute && static_cast<Derived *>(this)->create(sctx))
      return true;
    return false;
  }
};

template <class Derived>
class Grant_privileges : public Security_context_policy {
 public:
  bool operator()(Security_context *sctx, Operation op) {
    if (op == Precheck && static_cast<Derived *>(this)->precheck(sctx))
      return true;
    if (op == Execute && static_cast<Derived *>(this)->grant_privileges(sctx))
      return true;
    return false;
  }
};

template <typename T>
using Sctx_ptr = std::unique_ptr<T, std::function<void(T *)>>;

/**
  Factory for creating any Security_context given a pre-constructed policy.
*/
class Security_context_factory {
 public:
  /**
    Default Security_context factory implementation. Given two policies and
    a authid this class returns a Security_context.
    @param thd                        The thread handle
    @param user                       User name associated with auth id
    @param host                       Host name associated with auth id
    @param extend_user_profile        The policy for creating the user profile
    @param priv                       The policy for authorizing the authid to
                                      use the server.
    @param static_priv                Static privileges for authid.
    @param drop_policy                The policy for deleting the authid and
                                      revoke privileges
  */
  Security_context_factory(THD *thd, std::string user, std::string host,
                           Security_context_functor extend_user_profile,
                           Security_context_functor priv,
                           Security_context_functor static_priv,
                           std::function<void(Security_context *)> drop_policy)
      : m_thd(thd),
        m_user(std::move(user)),
        m_host(std::move(host)),
        m_user_profile(std::move(extend_user_profile)),
        m_privileges(std::move(priv)),
        m_static_privileges(std::move(static_priv)),
        m_drop_policy(std::move(drop_policy)) {}

  Sctx_ptr<Security_context> create(MEM_ROOT *mem_root);

 private:
  bool apply_pre_constructed_policies(Security_context *sctx);

  THD *m_thd;
  std::string m_user;
  std::string m_host;
  Security_context_functor m_user_profile;
  Security_context_functor m_privileges;
  Security_context_functor m_static_privileges;
  const std::function<void(Security_context *)> m_drop_policy;
};

class Default_local_authid : public Create_authid<Default_local_authid> {
 public:
  Default_local_authid(const THD *thd);
  bool precheck(Security_context *sctx);
  bool create(Security_context *sctx);

 private:
  const THD *m_thd;
};

/**
  Grant the privilege temporarily to the in-memory global privleges map.
  This class is not thread safe.
 */
class Grant_temporary_dynamic_privileges
    : public Grant_privileges<Grant_temporary_dynamic_privileges> {
 public:
  Grant_temporary_dynamic_privileges(const THD *thd,
                                     std::vector<std::string> privs);
  bool precheck(Security_context *sctx);
  bool grant_privileges(Security_context *sctx);

 private:
  const THD *m_thd;
  const std::vector<std::string> m_privs;
};

class Drop_temporary_dynamic_privileges {
 public:
  explicit Drop_temporary_dynamic_privileges(std::vector<std::string> privs)
      : m_privs(std::move(privs)) {}
  void operator()(Security_context *sctx);

 private:
  std::vector<std::string> m_privs;
};

class Grant_temporary_static_privileges
    : public Grant_privileges<Grant_temporary_static_privileges> {
 public:
  Grant_temporary_static_privileges(const THD *thd, const ulong privs);
  bool precheck(Security_context *sctx);
  bool grant_privileges(Security_context *sctx);

 private:
  /** THD handle */
  const THD *m_thd;

  /** Privileges */
  const ulong m_privs;
};

bool operator==(const LEX_CSTRING &a, const LEX_CSTRING &b);
bool is_partial_revoke_exists(THD *thd);
void set_system_user_flag(THD *thd, bool check_for_main_security_ctx = false);

/**
  Storage container for default auth ids. Default roles are only weakly
  depending on ACL_USERs. You can retain a default role even if the
  corresponding ACL_USER is missing in the acl_cache.
*/
class Auth_id {
 public:
  Auth_id();
  Auth_id(const char *user, size_t user_len, const char *host, size_t host_len);
  Auth_id(const Auth_id_ref &id);
  Auth_id(const LEX_CSTRING &user, const LEX_CSTRING &host);
  Auth_id(const std::string &user, const std::string &host);
  Auth_id(const LEX_USER *lex_user);
  Auth_id(const ACL_USER *acl_user);

  ~Auth_id();
  Auth_id(const Auth_id &id);
  Auth_id &operator=(const Auth_id &) = default;

  bool operator<(const Auth_id &id) const;
  void auth_str(std::string *out) const;
  std::string auth_str() const;
  const std::string &user() const;
  const std::string &host() const;

 private:
  void create_key();
  /** User part */
  std::string m_user;
  /** Host part */
  std::string m_host;
  /**
    Key: Internal representation mainly to facilitate use of
         Auth_id class in standard container.
         Format: 'user\0host\0'
  */
  std::string m_key;
};

/*
  As of now Role_id is an alias of Auth_id.
  We may extend the Auth_id as Role_id once
  more substances are added to latter.
*/
using Role_id = Auth_id;

/**
  Length of string buffer, that is enough to contain
  username and hostname parts of the user identifier with trailing zero in
  MySQL standard format:
  user_name_part\@host_name_part\\0
*/
static constexpr int USER_HOST_BUFF_SIZE =
    HOSTNAME_LENGTH + USERNAME_LENGTH + 2;

void generate_random_password(std::string *password, uint32_t);
typedef std::list<std::vector<std::string>> Userhostpassword_list;
bool send_password_result_set(THD *thd,
                              const Userhostpassword_list &generated_passwords);
bool lock_and_get_mandatory_roles(std::vector<Role_id> *mandatory_roles);
#endif /* AUTH_COMMON_INCLUDED */
