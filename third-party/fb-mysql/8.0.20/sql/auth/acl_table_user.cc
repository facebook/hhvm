/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/auth/acl_table_user.h" /* For user table data */

#include <stdlib.h>  /* atoi */
#include <string.h>  /* strlen, strcmp, NULL, memcmp, memcpy */
#include <algorithm> /* sort */
#include <map>       /* map */

#include "field_types.h"   /* MYSQL_TYPE_ENUM, MYSQL_TYPE_JSON */
#include "lex_string.h"    /* LEX_CSTRING */
#include "m_ctype.h"       /* my_charset_* */
#include "m_string.h"      /* STRING_WITH_LEN */
#include "my_base.h"       /* HA_ERR_* */
#include "my_dbug.h"       /* DBUG macros */
#include "my_inttypes.h"   /* MYF, uchar, longlong, ulonglong */
#include "my_loglevel.h"   /* WARNING_LEVEL, loglevel, ERROR_LEVEL */
#include "my_sqlcommand.h" /* SQLCOM_ALTER_USER, SQLCOM_GRANT */
#include "my_sys.h"        /* my_error */
#include "mysql/components/services/log_builtins.h" /* for LogEvent, LogErr */
#include "mysql/plugin.h" /* st_mysql_plugin, MYSQL_AUTHENTICATION_PLUGIN */
#include "mysql/plugin_auth.h"      /* st_mysql_auth */
#include "mysql/psi/psi_base.h"     /* PSI_NOT_INSTRUMENTED */
#include "mysql_time.h"             /* MYSQL_TIME, MYSQL_TIMESTAMP_ERROR */
#include "mysqld_error.h"           /* ER_* */
#include "prealloced_array.h"       /* Prealloced_array */
#include "sql/auth/auth_acls.h"     /* ACLs */
#include "sql/auth/auth_common.h"   /* User_table_schema, ... */
#include "sql/auth/auth_internal.h" /* acl_print_ha_error */
#include "sql/auth/partial_revokes.h"
#include "sql/auth/sql_auth_cache.h"     /* global_acl_memory */
#include "sql/auth/sql_authentication.h" /* Cached_authentication_plugins */
#include "sql/auth/sql_user_table.h"     /* Acl_table_intact */
#include "sql/auth/user_table.h"         /* replace_user_table */
#include "sql/field.h"     /* Field, Field_json, Field_enum, TYPE_OK */
#include "sql/handler.h"   /* handler, DB_TYPE_NDBCLUSTER, handlerton */
#include "sql/item_func.h" /* mqh_used */
#include "sql/json_dom.h"
#include "sql/key.h"    /* key_copy, KEY */
#include "sql/mysqld.h" /* specialflag */
#include "sql/records.h"
#include "sql/row_iterator.h"     /* RowIterator */
#include "sql/sql_class.h"        /* THD */
#include "sql/sql_const.h"        /* ACL_ALLOC_BLOCK_SIZE, MAX_KEY_LENGTH */
#include "sql/sql_lex.h"          /* LEX */
#include "sql/sql_plugin.h"       /* plugin_unlock, my_plugin_lock_by_name */
#include "sql/sql_plugin_ref.h"   /* plugin_decl, plugin_ref */
#include "sql/sql_time.h"         /* str_to_time_with_warn */
#include "sql/sql_update.h"       /* compare_records */
#include "sql/system_variables.h" /* System_variables */
#include "sql/table.h"            /* TABLE, TABLE_SHARE, ... */
#include "sql/thr_malloc.h"       /* init_sql_alloc */
#include "sql/tztime.h"           /* Time_zone */
#include "sql_string.h"           /* String */
#include "template_utils.h"       /* down_cast */
#include "typelib.h"              /* TYPELIB */
#include "violite.h"              /* SSL_* */

#define INVALID_DATE "0000-00-00 00:00:00"

namespace consts {
/** Initial timestamp */
const struct timeval BEGIN_TIMESTAMP = {0, 0};

/** Error indicating table operation error */
const int CRITICAL_ERROR = -1;

/** Empty string */
const std::string empty_string("");

/* Name of the fields in mysql.user.user_attributes */

/** For secondary password */
const std::string additional_password("additional_password");

/** For partial revokes */
const std::string Restrictions("Restrictions");

/** for password locking */
const std::string Password_locking("Password_locking");

/** underkeys of password locking */
const std::string failed_login_attempts("failed_login_attempts");

/** underkeys of password locking */
const std::string password_lock_time_days("password_lock_time_days");
}  // namespace consts

namespace acl_table {

/** Keys used in mysql.user.user_attributes */
static std::map<const User_attribute_type, const std::string>
    attribute_type_to_str = {
        {User_attribute_type::ADDITIONAL_PASSWORD, consts::additional_password},
        {User_attribute_type::RESTRICTIONS, consts::Restrictions},
        {User_attribute_type::PASSWORD_LOCKING, consts::Password_locking}};

namespace {
/**
  Class to handle information stored in mysql.user.user_attributes
*/
class Acl_user_attributes {
 public:
  /**
    Default constructor.
  */
  Acl_user_attributes(MEM_ROOT *mem_root, bool read_restrictions,
                      Auth_id &auth_id, ulong global_privs);

  Acl_user_attributes(MEM_ROOT *mem_root, bool read_restrictions,
                      Auth_id &auth_id, Restrictions *m_restrictions);

  ~Acl_user_attributes();

 public:
  /**
    Obtain info from JSON representation of user attributes

    @param [in] json_object JSON object that holds user attributes

    @returns status of parsing json_object
      @retval false Success
      @retval true  Error parsing the JSON object
  */
  bool deserialize(const Json_object &json_object);

  /**
    Create JSON object from user attributes

    @param [out] json_object Object to store serialized user attributes

    @returns status of serialization
      @retval false Success
      @retval true  Error serializing user attributes
  */
  bool serialize(Json_object &json_object) const;

  /**
    Update second password for user. We replace existing one if any.

    @param [in] credential Second password

    @returns status of password update
      @retval false Success
      @retval true  Error. Second password is empty
  */
  bool update_additional_password(std::string &credential);

  /**
    Discard second password.
  */
  void discard_additional_password();

  /**
    Get second password

    @returns second password
  */
  const std::string get_additional_password() const;

  /**
    Get the restriction list for the user

    @returns Restriction list
  */
  Restrictions get_restrictions() const;

  void update_restrictions(const Restrictions &restricitions);

  auto get_failed_login_attempts() const {
    return m_password_lock.failed_login_attempts;
  }
  auto get_password_lock_time_days() const {
    return m_password_lock.password_lock_time_days;
  }
  auto get_password_lock() const { return m_password_lock; }
  void set_password_lock(Password_lock password_lock) {
    m_password_lock = password_lock;
  }

 private:
  void report_and_remove_invalid_db_restrictions(
      DB_restrictions &db_restrictions, ulong mask, enum loglevel level,
      ulonglong errcode);
  bool deserialize_password_lock(const Json_object &json_object);

 private:
  /** Mem root */
  MEM_ROOT *m_mem_root;
  /** Operation for restrictions */
  bool m_read_restrictions;
  /** Auth ID */
  Auth_id m_auth_id;
  /** Second password for user */
  std::string m_additional_password;
  /** Restrictions_list on certain databases for user */
  Restrictions m_restrictions;
  /** Global static privileges */
  ulong m_global_privs;
  /** password locking */
  Password_lock m_password_lock;
};

Acl_user_attributes::Acl_user_attributes(MEM_ROOT *mem_root,
                                         bool read_restrictions,
                                         Auth_id &auth_id, ulong global_privs)
    : m_mem_root(mem_root),
      m_read_restrictions(read_restrictions),
      m_auth_id(auth_id),
      m_additional_password(),
      m_restrictions(mem_root),
      m_global_privs(global_privs),
      m_password_lock() {}

Acl_user_attributes::Acl_user_attributes(MEM_ROOT *mem_root,
                                         bool read_restrictions,
                                         Auth_id &auth_id,
                                         Restrictions *restrictions)
    : Acl_user_attributes(mem_root, read_restrictions, auth_id, ~NO_ACCESS) {
  if (restrictions) m_restrictions = *restrictions;
}

Acl_user_attributes::~Acl_user_attributes() { m_restrictions.clear_db(); }

void Acl_user_attributes::report_and_remove_invalid_db_restrictions(
    DB_restrictions &db_restrictions, ulong mask, enum loglevel level,
    ulonglong errcode) {
  for (auto &itr : db_restrictions()) {
    ulong privs = itr.second;
    if (privs != (privs & mask)) {
      std::string invalid_privs;
      std::string separator(", ");
      bool second = false;
      ulong filtered_privs = privs & ~mask;
      if (filtered_privs)
        db_restrictions.remove(itr.first.c_str(), filtered_privs);
      while (filtered_privs != 0) {
        std::string one_priv = get_one_priv(filtered_privs);
        if (one_priv.length()) {
          if (second) invalid_privs.append(separator);
          invalid_privs.append(one_priv);
          if (!second) second = true;
        }
      }
      if (!invalid_privs.length()) invalid_privs.append("<unknown_privileges>");
      std::string auth_id;
      m_auth_id.auth_str(&auth_id);

      LogErr(level, errcode, auth_id.c_str(), invalid_privs.c_str(),
             itr.first.length() ? itr.first.c_str() : "<invalid_database>");
    }
  }
  /*
    Now, remove the databases with no restrictions without invalidating
    the internal container of DB_restrictions
  */
  db_restrictions.remove(0);
}

bool Acl_user_attributes::deserialize_password_lock(
    const Json_object &json_object) {
  /* password locking */
  m_password_lock.password_lock_time_days = 0;
  m_password_lock.failed_login_attempts = 0;

  const Json_dom *password_locking_dom = json_object.get(
      attribute_type_to_str[User_attribute_type::PASSWORD_LOCKING]);
  if (password_locking_dom) {
    if (password_locking_dom->json_type() != enum_json_type::J_OBJECT)
      return true;
    const Json_object *password_locking =
        down_cast<const Json_object *>(password_locking_dom);

    const Json_dom *password_lock_time_days_dom =
        password_locking->get(consts::password_lock_time_days);
    if (password_lock_time_days_dom) {
      if (password_lock_time_days_dom->json_type() != enum_json_type::J_INT)
        return true;
      const Json_int *password_lock_time_days =
          down_cast<const Json_int *>(password_lock_time_days_dom);
      longlong val = password_lock_time_days->value();
      if (val < -1 || val > INT_MAX) return true;
      m_password_lock.password_lock_time_days = val;
    }

    const Json_dom *failed_login_attempts_dom =
        password_locking->get(consts::failed_login_attempts);
    if (failed_login_attempts_dom) {
      if (failed_login_attempts_dom->json_type() != enum_json_type::J_INT) {
        m_password_lock.password_lock_time_days = 0;
        return true;
      }
      const Json_int *failed_login_attempts =
          down_cast<const Json_int *>(failed_login_attempts_dom);
      longlong val = failed_login_attempts->value();
      if (val < 0 || val > UINT_MAX) {
        m_password_lock.password_lock_time_days = 0;
        return true;
      }
      m_password_lock.failed_login_attempts = val;
    }
  }
  return false;
}

bool Acl_user_attributes::deserialize(const Json_object &json_object) {
  {
    /** Second password */
    const Json_dom *additional_password_dom = json_object.get(
        attribute_type_to_str[User_attribute_type::ADDITIONAL_PASSWORD]);
    if (additional_password_dom) {
      if (additional_password_dom->json_type() != enum_json_type::J_STRING)
        return true;

      const Json_string *additional_password =
          down_cast<const Json_string *>(additional_password_dom);
      m_additional_password = additional_password->value();
    }
  }

  /* In case of writes, DB restrictions are always overwritten */
  if (m_read_restrictions) {
    DB_restrictions db_restrictions(nullptr);
    if (db_restrictions.add(json_object)) return true;
    /* Filtering & warnings */
    report_and_remove_invalid_db_restrictions(
        db_restrictions, DB_OP_ACLS, WARNING_LEVEL,
        ER_WARN_INCORRECT_PRIVILEGE_FOR_DB_RESTRICTIONS);
    report_and_remove_invalid_db_restrictions(db_restrictions, m_global_privs,
                                              WARNING_LEVEL,
                                              ER_WARN_INVALID_DB_RESTRICTIONS);
    m_restrictions.set_db(db_restrictions);
  }

  return deserialize_password_lock(json_object);
}

bool Acl_user_attributes::serialize(Json_object &json_object) const {
  if (m_additional_password.length()) {
    Json_string additional_password(m_additional_password);
    if (json_object.add_clone(
            attribute_type_to_str[User_attribute_type::ADDITIONAL_PASSWORD],
            &additional_password))
      return true;
  }

  if (m_restrictions.db().is_not_empty()) {
    Json_array restrictions_array;
    m_restrictions.db().get_as_json(restrictions_array);
    if (json_object.add_clone(
            attribute_type_to_str[User_attribute_type::RESTRICTIONS],
            &restrictions_array))
      return true;
  }

  if (m_password_lock.password_lock_time_days ||
      m_password_lock.failed_login_attempts) {
    Json_object password_lock;
    Json_int password_lock_time_days(m_password_lock.password_lock_time_days);
    Json_int failed_login_attempts(m_password_lock.failed_login_attempts);
    password_lock.add_clone(consts::password_lock_time_days,
                            &password_lock_time_days);
    password_lock.add_clone(consts::failed_login_attempts,
                            &failed_login_attempts);
    json_object.add_clone(
        attribute_type_to_str[User_attribute_type::PASSWORD_LOCKING],
        &password_lock);
  }

  return false;
}

bool Acl_user_attributes::update_additional_password(std::string &credential) {
  if (credential.length()) {
    m_additional_password = credential;
  } else {
    return true;
  }
  return false;
}

void Acl_user_attributes::discard_additional_password() {
  m_additional_password.clear();
}

const std::string Acl_user_attributes::get_additional_password() const {
  return m_additional_password;
}

Restrictions Acl_user_attributes::get_restrictions() const {
  return m_restrictions;
}

void Acl_user_attributes::update_restrictions(
    const Restrictions &restricitions) {
  m_restrictions = restricitions;
}

/**
  Helper function to parse mysql.user.user_attributes column

  @param [in]  thd             Thread handle
  @param [in]  table           Handle to mysql.user table
  @param [in]  table_schema    mysql.user schema version
  @param [out] user_attributes Deserialized user attributes

  @returns status of parsing user_attributes column
    @retval false Success
    @retval true  Problem parsing the column
*/
bool parse_user_attributes(THD *thd, TABLE *table,
                           User_table_schema *table_schema,
                           Acl_user_attributes &user_attributes) {
  // Read only if the column of type JSON and it is not null.
  if (table->field[table_schema->user_attributes_idx()]->type() ==
          MYSQL_TYPE_JSON &&
      !table->field[table_schema->user_attributes_idx()]->is_null()) {
    Json_wrapper json_wrapper;
    if ((down_cast<Field_json *>(
             table->field[table_schema->user_attributes_idx()])
             ->val_json(&json_wrapper)))
      return true;

    Json_dom *json_dom = json_wrapper.to_dom(thd);
    if (!json_dom || json_dom->json_type() != enum_json_type::J_OBJECT)
      return true;

    const Json_object *json_object = down_cast<const Json_object *>(json_dom);
    if (user_attributes.deserialize(*json_object)) return true;
  }
  return false;
}
}  // namespace

Acl_table_user_writer_status::Acl_table_user_writer_status(MEM_ROOT *mem_root)
    : skip_cache_update(true),
      updated_rights(NO_ACCESS),
      error(consts::CRITICAL_ERROR),
      password_change_timestamp(consts::BEGIN_TIMESTAMP),
      second_cred(consts::empty_string),
      restrictions(mem_root),
      password_lock() {}

/**
  mysql.user table writer constructor

  Note: Table handle must be non-null.

  @param [in] thd              Thread handle
  @param [in] table            Handle to mysql.user table
  @param [in] combo            User information
  @param [in] rights           Updated global privileges
  @param [in] revoke_grant     If its REVOKE statement
  @param [in] can_create_user  Whether user has ability to create new user
  @param [in] what_to_update   Things to be updated
  @param [in] restrictions     Restrictions of the user, if there is any
*/
Acl_table_user_writer::Acl_table_user_writer(
    THD *thd, TABLE *table, LEX_USER *combo, ulong rights, bool revoke_grant,
    bool can_create_user, Pod_user_what_to_update what_to_update,
    Restrictions *restrictions)
    : Acl_table(thd, table, acl_table::Acl_table_operation::OP_INSERT),
      m_combo(combo),
      m_rights(rights),
      m_revoke_grant(revoke_grant),
      m_can_create_user(can_create_user),
      m_what_to_update(what_to_update),
      m_table_schema(nullptr),
      m_restrictions(restrictions) {
  if (table) {
    User_table_schema_factory user_table_schema_factory;
    m_table_schema = user_table_schema_factory.get_user_table_schema(table);
  }
}

/** Cleanup */
Acl_table_user_writer::~Acl_table_user_writer() {
  if (m_table_schema) delete m_table_schema;
}

/**
  Perform add/update to mysql.user table

  @returns status of add/update operation. In case of success it contains
           information that's useful for cache update.
*/
Acl_table_user_writer_status Acl_table_user_writer::driver() {
  bool builtin_plugin = false;
  bool update_password = (m_what_to_update.m_what & PLUGIN_ATTR);
  Table_op_error_code error;
  LEX *lex = m_thd->lex;
  Acl_table_user_writer_status return_value(m_thd->mem_root);
  Acl_table_user_writer_status err_return_value(m_thd->mem_root);

  DBUG_TRACE;
  DBUG_ASSERT(assert_acl_cache_write_lock(m_thd));

  /* Setup the table for writing */
  if (setup_table(error, builtin_plugin)) {
    return_value.error = error;
    return return_value;
  }

  if (m_operation == Acl_table_operation::OP_UPDATE) {
    if ((lex->sql_command != SQLCOM_ALTER_USER) && !m_rights &&
        lex->ssl_type == SSL_TYPE_NOT_SPECIFIED && !lex->mqh.specified_limits &&
        !m_revoke_grant && (!builtin_plugin || !update_password) &&
        !m_restrictions) {
      DBUG_PRINT("info", ("Dynamic privileges exit path"));
      /*
        At this point, even though there is no error,
        we want to skip updates to cache because it's a no-op.
      */
      return_value.error = 0;
      return return_value;
    }
  }

  std::string current_password;
  if ((m_what_to_update.m_what & USER_ATTRIBUTES) &&
      (m_what_to_update.m_user_attributes & USER_ATTRIBUTE_RETAIN_PASSWORD))
    current_password = get_current_credentials();

  if (update_authentication_info(return_value) ||
      update_privileges(return_value) || update_ssl_properties() ||
      update_user_attributes(current_password, return_value) ||
      update_user_resources() || update_password_expiry() ||
      update_password_history() || update_password_reuse() ||
      update_password_require_current() || update_account_locking()) {
    return err_return_value;
  }

  (void)finish_operation(error);

  if (!error) {
    return_value.error = 0;
    return_value.skip_cache_update = false;
  }

  return return_value;
}

/**
  Position user table.

  Try to find a row matching with given account information. If one is
  found, set record pointer to it and set operation type as UPDATE. If no
  record is found, then set record pointer to empty record.

  Raises error in DA in various cases where sanity of table and
  intention of operation is checked.

  @param [out] error Table operation error
  @param [out] builtin_plugin  For existing record, if authentication plugin
                               is one of the builtins or not.

  @returns Operation status
    @retval false Table is positioned. In case of insert, it means no record
                  is found for given (user,host). In case of update, table
                  is set to point to existing record.
    @retval true  Error positioning table.
*/
bool Acl_table_user_writer::setup_table(int &error, bool &builtin_plugin) {
  bool update_password = (m_what_to_update.m_what & PLUGIN_ATTR);
  switch (m_operation) {
    case Acl_table_operation::OP_INSERT:
    case Acl_table_operation::OP_UPDATE: {
      uchar user_key[MAX_KEY_LENGTH];
      Acl_table_intact table_intact(m_thd);
      LEX_CSTRING old_plugin;
      error = consts::CRITICAL_ERROR;
      builtin_plugin = false;
      if (table_intact.check(m_table, ACL_TABLES::TABLE_USER)) return true;

      m_table->use_all_columns();
      DBUG_ASSERT(m_combo->host.str != nullptr);
      m_table->field[m_table_schema->host_idx()]->store(
          m_combo->host.str, m_combo->host.length, system_charset_info);
      m_table->field[m_table_schema->user_idx()]->store(
          m_combo->user.str, m_combo->user.length, system_charset_info);
      key_copy(user_key, m_table->record[0], m_table->key_info,
               m_table->key_info->key_length);

      error = m_table->file->ha_index_read_idx_map(
          m_table->record[0], 0, user_key, HA_WHOLE_KEY, HA_READ_KEY_EXACT);
      DBUG_ASSERT(m_table->file->ht->db_type == DB_TYPE_NDBCLUSTER ||
                  error != HA_ERR_LOCK_DEADLOCK);
      DBUG_ASSERT(m_table->file->ht->db_type == DB_TYPE_NDBCLUSTER ||
                  error != HA_ERR_LOCK_WAIT_TIMEOUT);
      DBUG_EXECUTE_IF("wl7158_replace_user_table_1",
                      error = HA_ERR_LOCK_DEADLOCK;);
      if (error) {
        if (error != HA_ERR_KEY_NOT_FOUND && error != HA_ERR_END_OF_FILE) {
          acl_print_ha_error(error);
          return true;
        }
        m_operation = Acl_table_operation::OP_INSERT;

        /*
          The user record wasn't found; if the intention was to revoke
          privileges (indicated by what == 'N') then execution must fail
          now.
        */
        if (m_revoke_grant) {
          my_error(ER_NONEXISTING_GRANT, MYF(0), m_combo->user.str,
                   m_combo->host.str);
          /*
            Return 1 as an indication that expected error occurred during
            handling of REVOKE statement for an unknown user.
          */
          error = 1;
          return true;
        }

        if (m_thd->lex->sql_command == SQLCOM_ALTER_USER) {
          /* Entry should have existsed since this is ALTER USER */
          error = 1;
          return true;
        }

        optimize_plugin_compare_by_pointer(&m_combo->plugin);
        builtin_plugin = auth_plugin_is_built_in(m_combo->plugin.str);

        /* The user record was neither present nor the intention was to
         * create it */
        if (!m_can_create_user) {
          if (!update_password) {
            /* Have come here to GRANT privilege to the non-existing user */
            my_error(ER_CANT_CREATE_USER_WITH_GRANT, MYF(0));
          } else {
            /* Have come here to update the password of the non-existing
             * user
             */
            my_error(ER_PASSWORD_NO_MATCH, MYF(0), m_combo->user.str,
                     m_combo->host.str);
          }
          error = 1;
          return true;
        }
        if (m_thd->lex->sql_command == SQLCOM_GRANT) {
          my_error(ER_PASSWORD_NO_MATCH, MYF(0), m_combo->user.str,
                   m_combo->host.str);
          error = 1;
          return true;
        }
        restore_record(m_table, s->default_values);
        DBUG_ASSERT(m_combo->host.str != nullptr);
        m_table->field[m_table_schema->host_idx()]->store(
            m_combo->host.str, m_combo->host.length, system_charset_info);
        m_table->field[m_table_schema->user_idx()]->store(
            m_combo->user.str, m_combo->user.length, system_charset_info);
      } else {
        /* There is a matching user record */
        m_operation = Acl_table_operation::OP_UPDATE;

        /* Check if there is such a user in user table in memory? */

        if (!find_acl_user(m_combo->host.str, m_combo->user.str, false)) {
          my_error(ER_PASSWORD_NO_MATCH, MYF(0));
          error = consts::CRITICAL_ERROR;
          return true;
        }

        store_record(m_table, record[1]);  // Save copy for update

        /* 1. resolve plugins in the LEX_USER struct if needed */
        /* Get old plugin value from storage. */
        old_plugin.str = get_field(
            m_thd->mem_root, m_table->field[m_table_schema->plugin_idx()]);

        if (old_plugin.str == nullptr || *old_plugin.str == '\0') {
          my_error(ER_PASSWORD_NO_MATCH, MYF(0));
          error = 1;
          return true;
        }

        /*
          It is important not to include the trailing '\0' in the string
          length because otherwise the plugin hash search will fail.
        */
        old_plugin.length = strlen(old_plugin.str);

        /* Optimize for pointer comparision of built-in plugin name */
        optimize_plugin_compare_by_pointer(&old_plugin);
        builtin_plugin = auth_plugin_is_built_in(old_plugin.str);
      }
      break;
    }
    default:
      return false;
  }
  return false;
}

/**
  Finish the operation

  Depending on type of operation (INSERT/UPDATE), either insert a new row
  in mysql.user table or update an existing row using SE APIs.

  @param [out] out_error Table operation error, if any

  @returns status of write operation
*/
Acl_table_op_status Acl_table_user_writer::finish_operation(
    Table_op_error_code &out_error) {
  switch (m_operation) {
    case Acl_table_operation::OP_INSERT: {
      out_error = m_table->file->ha_write_row(m_table->record[0]);  // insert
      DBUG_ASSERT(out_error != HA_ERR_FOUND_DUPP_KEY);
      DBUG_ASSERT(m_table->file->ht->db_type == DB_TYPE_NDBCLUSTER ||
                  out_error != HA_ERR_LOCK_DEADLOCK);
      DBUG_ASSERT(m_table->file->ht->db_type == DB_TYPE_NDBCLUSTER ||
                  out_error != HA_ERR_LOCK_WAIT_TIMEOUT);
      DBUG_EXECUTE_IF("wl7158_replace_user_table_3",
                      out_error = HA_ERR_LOCK_DEADLOCK;);
      if (out_error) {
        if (!m_table->file->is_ignorable_error(out_error)) {
          acl_print_ha_error(out_error);
          out_error = consts::CRITICAL_ERROR;
          return Acl_table_op_status::OP_ERROR_CRITICAL;
        }
      }
      break;
    }
    case Acl_table_operation::OP_UPDATE: {
      /*
        We should NEVER delete from the user table, as a uses can still
        use mysqld even if he doesn't have any privileges in the user table!
      */
      if (compare_records(m_table)) {
        out_error = m_table->file->ha_update_row(m_table->record[1],
                                                 m_table->record[0]);
        DBUG_ASSERT(out_error != HA_ERR_FOUND_DUPP_KEY);
        DBUG_ASSERT(m_table->file->ht->db_type == DB_TYPE_NDBCLUSTER ||
                    out_error != HA_ERR_LOCK_DEADLOCK);
        DBUG_ASSERT(m_table->file->ht->db_type == DB_TYPE_NDBCLUSTER ||
                    out_error != HA_ERR_LOCK_WAIT_TIMEOUT);
        DBUG_EXECUTE_IF("wl7158_replace_user_table_2",
                        out_error = HA_ERR_LOCK_DEADLOCK;);
        if (out_error && out_error != HA_ERR_RECORD_IS_THE_SAME) {
          acl_print_ha_error(out_error);
          out_error = consts::CRITICAL_ERROR;
          return Acl_table_op_status::OP_ERROR_CRITICAL;
        } else
          out_error = 0;
      }
      break;
    }
    default:
      out_error = 0;
  }
  return Acl_table_op_status::OP_OK;
}

/**
  Update user's authentication information

  Raises error in DA if mysql.user table does not have following columns:
  - plugin
  - password_last_changed
  - password_expired

  @param [out] return_value To update password change timestamp

  @returns update operation status
    @retval false Success
    @retval true  Error storing authentication info or table is not in
                  expected format
*/
bool Acl_table_user_writer::update_authentication_info(
    Acl_table_user_writer_status &return_value) {
  if (m_what_to_update.m_what & PLUGIN_ATTR ||
      (m_what_to_update.m_what & DEFAULT_AUTH_ATTR &&
       m_operation == Acl_table_operation::OP_INSERT)) {
    bool builtin_plugin;
    if (m_table->s->fields >= m_table_schema->plugin_idx()) {
      m_table->field[m_table_schema->plugin_idx()]->store(
          m_combo->plugin.str, m_combo->plugin.length, system_charset_info);
      m_table->field[m_table_schema->plugin_idx()]->set_notnull();
      m_table->field[m_table_schema->authentication_string_idx()]->store(
          m_combo->auth.str, m_combo->auth.length, &my_charset_utf8_bin);
      m_table->field[m_table_schema->authentication_string_idx()]
          ->set_notnull();
    } else {
      my_error(ER_BAD_FIELD_ERROR, MYF(0), "plugin", "mysql.user");
      return true;
    }
    /* If we change user plugin then check if it is builtin plugin */
    optimize_plugin_compare_by_pointer(&m_combo->plugin);
    builtin_plugin = auth_plugin_is_built_in(m_combo->plugin.str);
    /*
      we update the password last changed field whenever there is change
      in auth str and plugin is built in
    */
    if (m_table->s->fields > m_table_schema->password_last_changed_idx()) {
      if (builtin_plugin) {
        /*
          Calculate time stamp up to seconds elapsed from 1 Jan 1970
          00:00:00.
        */
        return_value.password_change_timestamp =
            m_thd->query_start_timeval_trunc(0);
        m_table->field[m_table_schema->password_last_changed_idx()]
            ->store_timestamp(&return_value.password_change_timestamp);
        m_table->field[m_table_schema->password_last_changed_idx()]
            ->set_notnull();
      }
    } else {
      my_error(ER_BAD_FIELD_ERROR, MYF(0), "password_last_changed",
               "mysql.user");
      return true;
    }
    /* if we have a password supplied we update the expiration field */
    if (m_table->s->fields > m_table_schema->password_expired_idx()) {
      m_table->field[m_table_schema->password_expired_idx()]->store(
          "N", 1, system_charset_info);
    } else {
      my_error(ER_BAD_FIELD_ERROR, MYF(0), "password_expired", "mysql.user");
      return true;
    }
  }
  return false;
}

/**
  Update global privileges for user
  @param [out] return_value To store updated global privileges

  @returns Update status for global privileges
*/
bool Acl_table_user_writer::update_privileges(
    Acl_table_user_writer_status &return_value) {
  if (m_what_to_update.m_what & ACCESS_RIGHTS_ATTR) {
    /* Update table columns with new privileges */
    char what = m_revoke_grant ? 'N' : 'Y';
    Field **tmp_field;
    ulong priv;
    for (tmp_field = m_table->field + 2, priv = SELECT_ACL;
         *tmp_field && (*tmp_field)->real_type() == MYSQL_TYPE_ENUM &&
         ((Field_enum *)(*tmp_field))->typelib->count == 2;
         tmp_field++, priv <<= 1) {
      if (priv & m_rights) {
        // set requested privileges
        (*tmp_field)->store(&what, 1, &my_charset_latin1);
        DBUG_PRINT("info",
                   ("Updating field %lu with privilege %c",
                    (ulong)(m_table->field + 2 - tmp_field), (char)what));
      }
    }
    if (m_table->s->fields > m_table_schema->create_role_priv_idx()) {
      if (CREATE_ROLE_ACL & m_rights) {
        m_table->field[m_table_schema->create_role_priv_idx()]->store(
            &what, 1, &my_charset_latin1);
      }

      if (DROP_ROLE_ACL & m_rights) {
        m_table->field[m_table_schema->drop_role_priv_idx()]->store(
            &what, 1, &my_charset_latin1);
      }
    }
  }

  return_value.updated_rights = get_user_privileges();
  DBUG_PRINT("info",
             ("Privileges on disk are now %lu", return_value.updated_rights));
  DBUG_PRINT("info", ("table fields: %d", m_table->s->fields));

  return false;
}

/**
  Update SSL properties

  @returns Update status
    @retval false Success
    @retval true  Table is not in expected format
*/
bool Acl_table_user_writer::update_ssl_properties() {
  if (m_what_to_update.m_what & SSL_ATTR) {
    LEX *lex = m_thd->lex;
    if (m_table->s->fields >= m_table_schema->x509_subject_idx()) {
      switch (lex->ssl_type) {
        case SSL_TYPE_ANY: {
          m_table->field[m_table_schema->ssl_type_idx()]->store(
              STRING_WITH_LEN("ANY"), &my_charset_latin1);
          m_table->field[m_table_schema->ssl_cipher_idx()]->store(
              "", 0, &my_charset_latin1);
          m_table->field[m_table_schema->x509_issuer_idx()]->store(
              "", 0, &my_charset_latin1);
          m_table->field[m_table_schema->x509_subject_idx()]->store(
              "", 0, &my_charset_latin1);
          break;
        }
        case SSL_TYPE_X509: {
          m_table->field[m_table_schema->ssl_type_idx()]->store(
              STRING_WITH_LEN("X509"), &my_charset_latin1);
          m_table->field[m_table_schema->ssl_cipher_idx()]->store(
              "", 0, &my_charset_latin1);
          m_table->field[m_table_schema->x509_issuer_idx()]->store(
              "", 0, &my_charset_latin1);
          m_table->field[m_table_schema->x509_subject_idx()]->store(
              "", 0, &my_charset_latin1);
          break;
        }
        case SSL_TYPE_SPECIFIED: {
          m_table->field[m_table_schema->ssl_type_idx()]->store(
              STRING_WITH_LEN("SPECIFIED"), &my_charset_latin1);
          m_table->field[m_table_schema->ssl_cipher_idx()]->store(
              "", 0, &my_charset_latin1);
          m_table->field[m_table_schema->x509_issuer_idx()]->store(
              "", 0, &my_charset_latin1);
          m_table->field[m_table_schema->x509_subject_idx()]->store(
              "", 0, &my_charset_latin1);
          if (lex->ssl_cipher)
            m_table->field[m_table_schema->ssl_cipher_idx()]->store(
                lex->ssl_cipher, strlen(lex->ssl_cipher), system_charset_info);
          if (lex->x509_issuer)
            m_table->field[m_table_schema->x509_issuer_idx()]->store(
                lex->x509_issuer, strlen(lex->x509_issuer),
                system_charset_info);
          if (lex->x509_subject)
            m_table->field[m_table_schema->x509_subject_idx()]->store(
                lex->x509_subject, strlen(lex->x509_subject),
                system_charset_info);
          break;
        }
        case SSL_TYPE_NOT_SPECIFIED:
          break;
        case SSL_TYPE_NONE: {
          m_table->field[m_table_schema->ssl_type_idx()]->store(
              "", 0, &my_charset_latin1);
          m_table->field[m_table_schema->ssl_cipher_idx()]->store(
              "", 0, &my_charset_latin1);
          m_table->field[m_table_schema->x509_issuer_idx()]->store(
              "", 0, &my_charset_latin1);
          m_table->field[m_table_schema->x509_subject_idx()]->store(
              "", 0, &my_charset_latin1);
          break;
          default:
            return true;
        }
      }
    } else {
      return true;
    }
  }
  return false;
}

/**
  Update user resource restrictions

  @returns status of the operation
*/
bool Acl_table_user_writer::update_user_resources() {
  if (m_what_to_update.m_what & RESOURCE_ATTR) {
    USER_RESOURCES mqh = m_thd->lex->mqh;
    if (mqh.specified_limits & USER_RESOURCES::QUERIES_PER_HOUR)
      m_table->field[m_table_schema->max_questions_idx()]->store(
          (longlong)mqh.questions, true);
    if (mqh.specified_limits & USER_RESOURCES::UPDATES_PER_HOUR)
      m_table->field[m_table_schema->max_updates_idx()]->store(
          (longlong)mqh.updates, true);
    if (mqh.specified_limits & USER_RESOURCES::CONNECTIONS_PER_HOUR)
      m_table->field[m_table_schema->max_connections_idx()]->store(
          (longlong)mqh.conn_per_hour, true);
    if (m_table->s->fields >= 36 &&
        (mqh.specified_limits & USER_RESOURCES::USER_CONNECTIONS))
      m_table->field[m_table_schema->max_user_connections_idx()]->store(
          (longlong)mqh.user_conn, true);
  }
  mqh_used = mqh_used || m_thd->lex->mqh.questions || m_thd->lex->mqh.updates ||
             m_thd->lex->mqh.conn_per_hour;
  return false;
}

/**
  Update password expiration info

  Raises error in DA if mysql.user table does not have password_expired
  column.

  @returns status of operation
    @retval false Success
    @retval true  Table is not in expected format
*/
bool Acl_table_user_writer::update_password_expiry() {
  if (m_what_to_update.m_what & PASSWORD_EXPIRE_ATTR) {
    /*
      ALTER/CREATE USER <user> PASSWORD EXPIRE (or)
      ALTER USER <user> IDENTIFIED WITH plugin
    */
    if (m_combo->alter_status.update_password_expired_column) {
      if (m_table->s->fields > m_table_schema->password_expired_idx()) {
        m_table->field[m_table_schema->password_expired_idx()]->store(
            "Y", 1, system_charset_info);
      } else {
        my_error(ER_BAD_FIELD_ERROR, MYF(0), "password_expired", "mysql.user");
        return true;
      }
    }
    /*
      If password_expired column is not to be updated and only
      password_lifetime is to be updated
    */
    if (m_table->s->fields > m_table_schema->password_lifetime_idx() &&
        !m_combo->alter_status.update_password_expired_column) {
      if (!m_combo->alter_status.use_default_password_lifetime) {
        m_table->field[m_table_schema->password_lifetime_idx()]->store(
            (longlong)m_combo->alter_status.expire_after_days, true);
        m_table->field[m_table_schema->password_lifetime_idx()]->set_notnull();
      } else
        m_table->field[m_table_schema->password_lifetime_idx()]->set_null();
    }
  }
  return false;
}

/**
  Update account locking info

  Raises error in DA if mysql.user table does not have account_locked
  column.
  @returns status of the operation
    @retval false Success
    @retval true  Table is not in expected format
*/
bool Acl_table_user_writer::update_account_locking() {
  if (m_what_to_update.m_what & ACCOUNT_LOCK_ATTR) {
    if (m_operation == Acl_table_operation::OP_INSERT ||
        (m_operation == Acl_table_operation::OP_UPDATE &&
         m_combo->alter_status.update_account_locked_column)) {
      if (m_table->s->fields > m_table_schema->account_locked_idx()) {
        /*
          Update the field for a new row and for the row that exists and the
          update was enforced (ACCOUNT [UNLOCK|LOCK]).
        */
        m_table->field[m_table_schema->account_locked_idx()]->store(
            m_combo->alter_status.account_locked ? "Y" : "N", 1,
            system_charset_info);
      } else {
        my_error(ER_BAD_FIELD_ERROR, MYF(0), "account_locked", "mysql.user");
        return true;
      }
    }
  }
  return false;
}

/**
  Password history updates

  Raises error in DA if mysql.user table does not have
  password_reuse_history column.

  @returns status of the operation
    @retval false Success
    @retval true  Table is not in expected format
*/
bool Acl_table_user_writer::update_password_history() {
  if (m_combo->alter_status.update_password_history) {
    /* ALTER USER .. PASSWORD HISTORY */
    if (m_table->s->fields > m_table_schema->password_reuse_history_idx()) {
      Field *fld_history =
          m_table->field[m_table_schema->password_reuse_history_idx()];
      if (m_combo->alter_status.use_default_password_history)
        fld_history->set_null();
      else {
        fld_history->store(m_combo->alter_status.password_history_length);
        fld_history->set_notnull();
      }
    } else {
      my_error(ER_BAD_FIELD_ERROR, MYF(0), "password_reuse_history",
               "mysql.user");
      return true;
    }
  }
  return false;
}

/**
  Password reuse time updates

  Raises error in DA if mysql.user table does not have
  password_reuse_time column.

  @returns status of the operation
    @retval false Success
    @retval true  Table is not in expected format
*/
bool Acl_table_user_writer::update_password_reuse() {
  if (m_combo->alter_status.update_password_reuse_interval) {
    /* ALTER USER .. PASSWORD REUSE INTERVAL */
    if (m_table->s->fields > m_table_schema->password_reuse_time_idx()) {
      Field *fld = m_table->field[m_table_schema->password_reuse_time_idx()];
      if (m_combo->alter_status.use_default_password_reuse_interval)
        fld->set_null();
      else {
        fld->store(m_combo->alter_status.password_reuse_interval);
        fld->set_notnull();
      }
    } else {
      my_error(ER_BAD_FIELD_ERROR, MYF(0), "password_reuse_time", "mysql.user");
      return true;
    }
  }
  return false;
}

/**
  Whether current password is required to update exisitng one

  Raises error in DA if mysql.user table does not have
  password_require_current column.

  @returns status of the operation
    @retval false Success
    @retval true  Table is not in expected format
*/
bool Acl_table_user_writer::update_password_require_current() {
  /* ALTER USER .. PASSWORD REQUIRE CURRENT */
  if (m_table->s->fields > m_table_schema->password_require_current_idx()) {
    Field *fld = m_table->field[m_table_schema->password_require_current_idx()];
    switch (m_combo->alter_status.update_password_require_current) {
      case Lex_acl_attrib_udyn::DEFAULT:
        fld->set_null();
        break;
      case Lex_acl_attrib_udyn::NO:
        fld->store("N", 1, system_charset_info);
        fld->set_notnull();
        break;
      case Lex_acl_attrib_udyn::YES:
        fld->store("Y", 1, system_charset_info);
        fld->set_notnull();
        break;
      case Lex_acl_attrib_udyn::UNCHANGED:
        if (m_operation == Acl_table_operation::OP_INSERT) fld->set_null();
        break;
      default:
        DBUG_ASSERT(false);
    }
  } else {
    my_error(ER_BAD_FIELD_ERROR, MYF(0), "password_require_current",
             "mysql.user");
    return true;
  }
  return false;
}

/**
  User_attributes updates

  Raises error in DA if mysql.user table does not have
  user_attributes column.

  @returns status of the operation
    @retval false Success
    @retval true  Table/Column is not in expected format
*/
bool Acl_table_user_writer::update_user_attributes(
    std::string &current_password, Acl_table_user_writer_status &return_value) {
  if (m_what_to_update.m_what & USER_ATTRIBUTES) {
    if (m_table->s->fields >= m_table_schema->user_attributes_idx()) {
      Auth_id auth_id(m_combo->user, m_combo->host);
      Acl_user_attributes user_attributes(
          m_thd->mem_root,
          !(m_what_to_update.m_user_attributes & USER_ATTRIBUTE_RESTRICTIONS),
          auth_id, m_restrictions);
      if (m_operation == Acl_table_operation::OP_UPDATE &&
          parse_user_attributes(m_thd, m_table, m_table_schema,
                                user_attributes))
        return true;

      /* Update additional password */
      if (m_what_to_update.m_user_attributes & USER_ATTRIBUTE_RETAIN_PASSWORD) {
        if (user_attributes.update_additional_password(current_password))
          return true;
        else {
          return_value.second_cred = current_password;
        }
      }

      /* Remove additional password */
      if (m_what_to_update.m_user_attributes &
          USER_ATTRIBUTE_DISCARD_PASSWORD) {
        /* We don't care if element was present or not */
        user_attributes.discard_additional_password();
        return_value.second_cred = consts::empty_string;
      }

      /* Update restrictions */
      if (m_what_to_update.m_user_attributes & USER_ATTRIBUTE_RESTRICTIONS) {
        user_attributes.update_restrictions(*m_restrictions);
      }

      /*
        Update password lock or default to
        the current values if not specified
      */
      return_value.password_lock = user_attributes.get_password_lock();
      if (m_what_to_update.m_user_attributes &
          USER_ATTRIBUTE_FAILED_LOGIN_ATTEMPTS)
        return_value.password_lock.failed_login_attempts =
            this->m_combo->alter_status.failed_login_attempts;
      if (m_what_to_update.m_user_attributes &
          USER_ATTRIBUTE_PASSWORD_LOCK_TIME)
        return_value.password_lock.password_lock_time_days =
            m_combo->alter_status.password_lock_time;
      user_attributes.set_password_lock(return_value.password_lock);

      /* Update the column */
      {
        Json_object out_json_object;
        user_attributes.serialize(out_json_object);
        if (out_json_object.cardinality()) {
          Json_wrapper json_wrapper(&out_json_object);
          json_wrapper.set_alias();
          Field_json *json_field = down_cast<Field_json *>(
              m_table->field[m_table_schema->user_attributes_idx()]);
          if (json_field->store_json(&json_wrapper) != TYPE_OK) return true;
          m_table->field[m_table_schema->user_attributes_idx()]->set_notnull();
        } else {
          /* Remove the object because it's not needed anymore. */
          m_table->field[m_table_schema->user_attributes_idx()]->set_null();
        }
      }
      return_value.restrictions = user_attributes.get_restrictions();
    } else {
      my_error(ER_BAD_FIELD_ERROR, MYF(0), "user_attributes", "mysql.user");
      return true;
    }
  } else {
    if (m_operation == Acl_table_operation::OP_INSERT) {
      m_table->field[m_table_schema->user_attributes_idx()]->set_null();
    }
  }
  return false;
}

/**
  Helper function to get global privileges from mysql.user table

  @returns Bitmask representing global privileges granted to given account
*/
ulong Acl_table_user_writer::get_user_privileges() {
  uint next_field;
  char *priv_str;
  ulong rights =
      get_access(m_table, m_table_schema->select_priv_idx(), &next_field);
  if (m_table->s->fields > m_table_schema->drop_role_priv_idx()) {
    priv_str =
        get_field(&global_acl_memory,
                  m_table->field[m_table_schema->create_role_priv_idx()]);
    if (priv_str && (*priv_str == 'Y' || *priv_str == 'y')) {
      rights |= CREATE_ROLE_ACL;
    }
    priv_str = get_field(&global_acl_memory,
                         m_table->field[m_table_schema->drop_role_priv_idx()]);
    if (priv_str && (*priv_str == 'Y' || *priv_str == 'y')) {
      rights |= DROP_ROLE_ACL;
    }
  }
  return rights;
}

/**
  Get current password from mysql.user.authentication_string

  @returns value from mysql.user.authentication_string
*/
std::string Acl_table_user_writer::get_current_credentials() {
  const char *current_password =
      get_field(m_thd->mem_root,
                m_table->field[m_table_schema->authentication_string_idx()]);
  std::string retval(current_password ? current_password : "",
                     current_password ? strlen(current_password) : 0);
  return retval;
}

/**
  mysql.user table reader constructor.

  @param [in] thd    Handle to THD object. Must be non-null
  @param [in] table  mysql.user table handle. Must be non-null
*/
Acl_table_user_reader::Acl_table_user_reader(THD *thd, TABLE *table)
    : Acl_table(thd, table, acl_table::Acl_table_operation::OP_READ) {
  init_sql_alloc(PSI_NOT_INSTRUMENTED, &m_mem_root, ACL_ALLOC_BLOCK_SIZE, 0);
  m_restrictions = new Restrictions(&m_mem_root);
}

/**
  Free resources before we destroy.
*/
Acl_table_user_reader::~Acl_table_user_reader() {
  if (m_table_schema) delete m_table_schema;
  if (m_restrictions) delete m_restrictions;
  free_root(&m_mem_root, MYF(0));
}

/**
  Finish mysql.user table read operation

  @param [in] error Table operation error.

  @returns OK status, always.
*/
Acl_table_op_status Acl_table_user_reader::finish_operation(
    Table_op_error_code &error) {
  // not used
  error = 0;
  return Acl_table_op_status::OP_OK;
}

/**
  Make table ready to read

  @param [out] is_old_db_layout To see if this is a case of running new
                                binary with old data directory without
                                running mysql_upgrade.

  @returns status of initialization
    @retval false Success
    @retval true  Error initializing table
*/
bool Acl_table_user_reader::setup_table(bool &is_old_db_layout) {
  DBUG_TRACE;
  m_iterator = init_table_iterator(m_thd, m_table, nullptr, false,
                                   /*ignore_not_found_rows=*/false);
  if (m_iterator == nullptr) return true;
  m_table->use_all_columns();
  clean_user_cache();

  User_table_schema_factory user_table_schema_factory;
  /*
    We need to check whether we are working with old database layout. This
    might be the case for instance when we are running mysql_upgrade.
  */
  m_table_schema = user_table_schema_factory.get_user_table_schema(m_table);
  is_old_db_layout =
      user_table_schema_factory.is_old_user_table_schema(m_table);

  return false;
}

/**
  Scrub ACL_USER.

  @param [out] user ACL_USER to be updated
*/
void Acl_table_user_reader::reset_acl_user(ACL_USER &user) {
  /*
    All accounts can authenticate per default. This will change when
    we add a new field to the user table.

    Currently this flag is only set to false when authentication is
    attempted using an unknown user name.
  */
  user.can_authenticate = true;

  /* Account is unlocked by default. */
  user.account_locked = false;

  /*
    The authorization id isn't a part of the role-graph per default.
    This is true even if CREATE ROLE is used.
  */
  user.is_role = false;
}

/**
  Get user and host information for the account.

  @param [out] user ACL_USER structure
*/
void Acl_table_user_reader::read_account_name(ACL_USER &user) {
  bool check_no_resolve = specialflag & SPECIAL_NO_RESOLVE;
  user.host.update_hostname(
      get_field(&m_mem_root, m_table->field[m_table_schema->host_idx()]));
  user.user =
      get_field(&m_mem_root, m_table->field[m_table_schema->user_idx()]);
  if (check_no_resolve && hostname_requires_resolving(user.host.get_host()) &&
      strcmp(user.host.get_host(), "localhost") != 0) {
    LogErr(WARNING_LEVEL, ER_AUTHCACHE_USER_SKIPPED_NEEDS_RESOLVE,
           user.user ? user.user : "",
           user.host.get_host() ? user.host.get_host() : "");
  }
  user.sort = get_sort(2, user.host.get_host(), user.user);
}

/**
  Read authentication string for the account. We do verification later.

  @param [out] user ACL_USER structure

  @returns Status of reading authentication data.
    @retval false Success
    @retval true  Error reading the field. Skip user.
*/
bool Acl_table_user_reader::read_authentication_string(ACL_USER &user) {
  /* Read password from authentication_string field */
  if (m_table->s->fields > m_table_schema->authentication_string_idx()) {
    user.credentials[PRIMARY_CRED].m_auth_string.str =
        get_field(&m_mem_root,
                  m_table->field[m_table_schema->authentication_string_idx()]);
  } else {
    LogErr(ERROR_LEVEL, ER_AUTHCACHE_USER_TABLE_DODGY);
    return true;
  }
  if (user.credentials[PRIMARY_CRED].m_auth_string.str) {
    user.credentials[PRIMARY_CRED].m_auth_string.length =
        strlen(user.credentials[PRIMARY_CRED].m_auth_string.str);
  } else {
    user.credentials[PRIMARY_CRED].m_auth_string = EMPTY_CSTR;
  }

  return false;
}

/**
  Get global privilege information

  @param [out] user ACL_USER structure
*/
void Acl_table_user_reader::read_privileges(ACL_USER &user) {
  uint next_field;
  user.access =
      get_access(m_table, m_table_schema->select_priv_idx(), &next_field) &
      GLOBAL_ACLS;
  /*
    if it is pre 5.0.1 privilege table then map CREATE privilege on
    CREATE VIEW & SHOW VIEW privileges
  */
  if (m_table->s->fields <= m_table_schema->show_view_priv_idx() &&
      (user.access & CREATE_ACL))
    user.access |= (CREATE_VIEW_ACL | SHOW_VIEW_ACL);

  /*
    if it is pre 5.0.2 privilege table then map CREATE/ALTER privilege on
    CREATE PROCEDURE & ALTER PROCEDURE privileges
  */
  if (m_table->s->fields <= m_table_schema->create_routine_priv_idx() &&
      (user.access & CREATE_ACL))
    user.access |= CREATE_PROC_ACL;
  if (m_table->s->fields <= m_table_schema->alter_routine_priv_idx() &&
      (user.access & ALTER_ACL))
    user.access |= ALTER_PROC_ACL;

  /* pre 5.0.3 did not have CREATE_USER_ACL */
  if (m_table->s->fields <= m_table_schema->create_user_priv_idx() &&
      (user.access & GRANT_ACL))
    user.access |= CREATE_USER_ACL;

  /*
    if it is pre 5.1.6 privilege table then map CREATE privilege on
    CREATE|ALTER|DROP|EXECUTE EVENT
  */
  if (m_table->s->fields <= m_table_schema->event_priv_idx() &&
      (user.access & SUPER_ACL))
    user.access |= EVENT_ACL;

  /* if it is pre 5.1.6 privilege then map TRIGGER privilege on CREATE. */
  if (m_table->s->fields <= m_table_schema->trigger_priv_idx() &&
      (user.access & SUPER_ACL))
    user.access |= TRIGGER_ACL;

  if (m_table->s->fields > m_table_schema->drop_role_priv_idx()) {
    char *priv = get_field(
        &m_mem_root, m_table->field[m_table_schema->create_role_priv_idx()]);

    if (priv && (*priv == 'Y' || *priv == 'y')) {
      user.access |= CREATE_ROLE_ACL;
    }

    priv = get_field(&m_mem_root,
                     m_table->field[m_table_schema->drop_role_priv_idx()]);

    if (priv && (*priv == 'Y' || *priv == 'y')) {
      user.access |= DROP_ROLE_ACL;
    }
  }

  if (m_table->s->fields <= m_table_schema->grant_priv_idx()) {
    // Without grant
    if (user.access & CREATE_ACL)
      user.access |= REFERENCES_ACL | INDEX_ACL | ALTER_ACL;
  }

  if (m_table->s->fields <= m_table_schema->ssl_type_idx()) {
    /* Convert old privileges */
    user.access |= LOCK_TABLES_ACL | CREATE_TMP_ACL | SHOW_DB_ACL;
    if (user.access & FILE_ACL) user.access |= REPL_CLIENT_ACL | REPL_SLAVE_ACL;
    if (user.access & PROCESS_ACL) user.access |= SUPER_ACL | EXECUTE_ACL;
  }
}

/**
  Read SSL restrictions

  @param [out] user ACL_USER structure
*/
void Acl_table_user_reader::read_ssl_fields(ACL_USER &user) {
  /* Starting from 4.0.2 we have more fields */
  if (m_table->s->fields >= m_table_schema->x509_subject_idx()) {
    char *ssl_type =
        get_field(&m_mem_root, m_table->field[m_table_schema->ssl_type_idx()]);
    if (!ssl_type)
      user.ssl_type = SSL_TYPE_NONE;
    else if (!strcmp(ssl_type, "ANY"))
      user.ssl_type = SSL_TYPE_ANY;
    else if (!strcmp(ssl_type, "X509"))
      user.ssl_type = SSL_TYPE_X509;
    else /* !strcmp(ssl_type, "SPECIFIED") */
      user.ssl_type = SSL_TYPE_SPECIFIED;

    user.ssl_cipher = get_field(
        &m_mem_root, m_table->field[m_table_schema->ssl_cipher_idx()]);
    user.x509_issuer = get_field(
        &m_mem_root, m_table->field[m_table_schema->x509_issuer_idx()]);
    user.x509_subject = get_field(
        &m_mem_root, m_table->field[m_table_schema->x509_subject_idx()]);
  } else {
    user.ssl_type = SSL_TYPE_NONE;
  }
}

/**
  Read user resource restrictions

  @param [out] user ACL_USER structure
*/
void Acl_table_user_reader::read_user_resources(ACL_USER &user) {
  if (m_table->s->fields >= m_table_schema->max_user_connections_idx()) {
    char *ptr = get_field(&m_mem_root,
                          m_table->field[m_table_schema->max_questions_idx()]);
    user.user_resource.questions = ptr ? atoi(ptr) : 0;
    ptr = get_field(&m_mem_root,
                    m_table->field[m_table_schema->max_updates_idx()]);
    user.user_resource.updates = ptr ? atoi(ptr) : 0;
    ptr = get_field(&m_mem_root,
                    m_table->field[m_table_schema->max_connections_idx()]);
    user.user_resource.conn_per_hour = ptr ? atoi(ptr) : 0;
    if (user.user_resource.questions || user.user_resource.updates ||
        user.user_resource.conn_per_hour)
      mqh_used = true;

    if (m_table->s->fields > m_table_schema->max_user_connections_idx()) {
      /* Starting from 5.0.3 we have max_user_connections field */
      ptr =
          get_field(&m_mem_root,
                    m_table->field[m_table_schema->max_user_connections_idx()]);
      user.user_resource.user_conn = ptr ? atoi(ptr) : 0;
    }
  }
}

/**
  Read plugin information

  If it is old layout read accordingly. Also, validate authentication string
  against expecte format for the plugin.

  @param [out] user                           ACL_USER structure
  @param [out] super_users_with_empty_plugin  User has SUPER privilege or
  not
  @param [in]  is_old_db_layout               We are reading from old table

  @returns status of reading plugin information
    @retval false Success
    @retval true  Error. Skip user.
*/
bool Acl_table_user_reader::read_plugin_info(
    ACL_USER &user, bool &super_users_with_empty_plugin,
    bool &is_old_db_layout) {
  if (m_table->s->fields >= m_table_schema->plugin_idx()) {
    /* We may have plugin & auth_String fields */
    const char *tmpstr =
        get_field(&m_mem_root, m_table->field[m_table_schema->plugin_idx()]);
    user.plugin.str = tmpstr ? tmpstr : "";
    user.plugin.length = strlen(user.plugin.str);

    /*
      In case we are working with 5.6 db layout we need to make server
      aware of Password field and that the plugin column can be null.
      In case when plugin column is null we use native password plugin
      if we can.
    */
    if (is_old_db_layout && (user.plugin.length == 0 ||
                             Cached_authentication_plugins::compare_plugin(
                                 PLUGIN_MYSQL_NATIVE_PASSWORD, user.plugin))) {
      char *password = get_field(
          &m_mem_root, m_table->field[m_table_schema->password_idx()]);

      // We do not support pre 4.1 hashes
      plugin_ref native_plugin =
          g_cached_authentication_plugins->get_cached_plugin_ref(
              PLUGIN_MYSQL_NATIVE_PASSWORD);
      if (native_plugin) {
        uint password_len = password ? strlen(password) : 0;
        st_mysql_auth *auth = (st_mysql_auth *)plugin_decl(native_plugin)->info;
        if (auth->validate_authentication_string(password, password_len) == 0) {
          // auth_string takes precedence over password
          if (user.credentials[PRIMARY_CRED].m_auth_string.length == 0) {
            user.credentials[PRIMARY_CRED].m_auth_string.str = password;
            user.credentials[PRIMARY_CRED].m_auth_string.length = password_len;
          }
          if (user.plugin.length == 0) {
            user.plugin.str = Cached_authentication_plugins::get_plugin_name(
                PLUGIN_MYSQL_NATIVE_PASSWORD);
            user.plugin.length = strlen(user.plugin.str);
          }
        } else {
          if ((user.access & SUPER_ACL) && !super_users_with_empty_plugin &&
              (user.plugin.length == 0))
            super_users_with_empty_plugin = true;

          LogErr(WARNING_LEVEL, ER_AUTHCACHE_USER_IGNORED_DEPRECATED_PASSWORD,
                 user.user ? user.user : "",
                 user.host.get_host() ? user.host.get_host() : "");
          return true;
        }
      }
    }

    /*
      Check if the plugin string is blank or null.
      If it is, the user will be skipped.
    */
    if (user.plugin.length == 0) {
      if ((user.access & SUPER_ACL) && !super_users_with_empty_plugin)
        super_users_with_empty_plugin = true;
      LogErr(WARNING_LEVEL, ER_AUTHCACHE_USER_IGNORED_NEEDS_PLUGIN,
             user.user ? user.user : "",
             user.host.get_host() ? user.host.get_host() : "");
      return true;
    }
    /*
      By comparing the plugin with the built in plugins it is possible
      to optimize the string allocation and comparision.
    */
    optimize_plugin_compare_by_pointer(&user.plugin);
  }
  /* Validate the hash string. */
  plugin_ref plugin = nullptr;
  plugin =
      my_plugin_lock_by_name(nullptr, user.plugin, MYSQL_AUTHENTICATION_PLUGIN);
  if (plugin) {
    st_mysql_auth *auth = (st_mysql_auth *)plugin_decl(plugin)->info;
    if (auth->validate_authentication_string(
            const_cast<char *>(
                user.credentials[PRIMARY_CRED].m_auth_string.str),
            user.credentials[PRIMARY_CRED].m_auth_string.length)) {
      LogErr(WARNING_LEVEL, ER_AUTHCACHE_USER_IGNORED_INVALID_PASSWORD,
             user.user ? user.user : "",
             user.host.get_host() ? user.host.get_host() : "");
      plugin_unlock(nullptr, plugin);
      return true;
    }
    plugin_unlock(nullptr, plugin);
  }
  return false;
}

/**
  Read password expiry field

  @param [out] user             ACL_USER structure
  @param [out] password_expired Whether password is expired or not

  @returns Status of reading password expiry value
    @retval false Success
    @retval true  Password expiry was set to TRUE for a plugin that does not
                  support password expiration. Skip user.
*/
bool Acl_table_user_reader::read_password_expiry(ACL_USER &user,
                                                 bool &password_expired) {
  if (m_table->s->fields > m_table_schema->password_expired_idx()) {
    char *tmpstr = get_field(
        &m_mem_root, m_table->field[m_table_schema->password_expired_idx()]);
    if (tmpstr && (*tmpstr == 'Y' || *tmpstr == 'y')) {
      user.password_expired = true;

      if (!auth_plugin_supports_expiration(user.plugin.str)) {
        LogErr(WARNING_LEVEL, ER_AUTHCACHE_EXPIRED_PASSWORD_UNSUPPORTED,
               user.user ? user.user : "",
               user.host.get_host() ? user.host.get_host() : "");
        return true;
      }
      password_expired = true;
    }
  }
  return false;
}

/**
  Determine if user account is locked

  @param [out] user ACL_USER structure
*/
void Acl_table_user_reader::read_password_locked(ACL_USER &user) {
  if (m_table->s->fields > m_table_schema->account_locked_idx()) {
    char *locked = get_field(
        &m_mem_root, m_table->field[m_table_schema->account_locked_idx()]);

    if (locked && (*locked == 'Y' || *locked == 'y')) {
      user.account_locked = true;
    }
  }
}

/**
  Get password change time

  @param [out] user ACL_USER structure
*/
void Acl_table_user_reader::read_password_last_changed(ACL_USER &user) {
  /*
  Initalize the values of timestamp and expire after day
  to error and true respectively.
  */
  user.password_last_changed.time_type = MYSQL_TIMESTAMP_ERROR;

  if (m_table->s->fields > m_table_schema->password_last_changed_idx()) {
    if (!m_table->field[m_table_schema->password_last_changed_idx()]
             ->is_null()) {
      char *password_last_changed = get_field(
          &m_mem_root,
          m_table->field[m_table_schema->password_last_changed_idx()]);

      if (password_last_changed &&
          memcmp(password_last_changed, INVALID_DATE, sizeof(INVALID_DATE))) {
        String str(password_last_changed, &my_charset_bin);
        str_to_time_with_warn(&str, &(user.password_last_changed));
      }
    }
  }
}

/**
  Get password expiry policy infomration

  @param [out] user ACL_USER structure
*/
void Acl_table_user_reader::read_password_lifetime(ACL_USER &user) {
  user.use_default_password_lifetime = true;
  user.password_lifetime = 0;
  if (m_table->s->fields > m_table_schema->password_lifetime_idx()) {
    if (!m_table->field[m_table_schema->password_lifetime_idx()]->is_null()) {
      char *ptr = get_field(
          &m_mem_root, m_table->field[m_table_schema->password_lifetime_idx()]);
      user.password_lifetime = ptr ? atoi(ptr) : 0;
      user.use_default_password_lifetime = false;
    }
  }
}

/**
  Get password history restriction

  @param [out] user ACL_USER structure
*/
void Acl_table_user_reader::read_password_history_fields(ACL_USER &user) {
  if (m_table->s->fields > m_table_schema->password_reuse_history_idx()) {
    if (m_table->field[m_table_schema->password_reuse_history_idx()]->is_null(
            0))
      user.use_default_password_history = true;
    else {
      char *ptr = get_field(
          &m_mem_root,
          m_table->field[m_table_schema->password_reuse_history_idx()]);
      /* ptr is NULL in case of DB NULL. Take the default in that case */
      user.password_history_length = ptr ? atoi(ptr) : 0;
      user.use_default_password_history = ptr == nullptr;
    }
  }
}

/**
  Get password reuse time restriction

  @param [out] user ACL_USER structure
*/
void Acl_table_user_reader::read_password_reuse_time_fields(ACL_USER &user) {
  if (m_table->s->fields > m_table_schema->password_reuse_time_idx()) {
    if (m_table->field[m_table_schema->password_reuse_time_idx()]->is_null(0))
      user.use_default_password_reuse_interval = true;
    else {
      char *ptr =
          get_field(&m_mem_root,
                    m_table->field[m_table_schema->password_reuse_time_idx()]);
      /* ptr is NULL in case of DB NULL. Take the default in that case */
      user.password_reuse_interval = ptr ? atoi(ptr) : 0;
      user.use_default_password_reuse_interval = ptr == nullptr;
    }
  }
}

/**
  Get information about requiring current password while changing password

  @param [out] user ACL_USER structure
*/
void Acl_table_user_reader::read_password_require_current(ACL_USER &user) {
  /* Read password_require_current field */
  if (m_table->s->fields > m_table_schema->password_require_current_idx()) {
    char *value = get_field(
        &m_mem_root,
        m_table->field[m_table_schema->password_require_current_idx()]);
    if (value == nullptr)
      user.password_require_current = Lex_acl_attrib_udyn::DEFAULT;
    else if (value[0] == 'Y')
      user.password_require_current = Lex_acl_attrib_udyn::YES;
    else if (value[0] == 'N')
      user.password_require_current = Lex_acl_attrib_udyn::NO;
  }
}

/**
  Read user attributes

  @param [out] user ACL_USER structure

  @returns status of reading user_attributes column
    @retval false Success
    @retval true  Error reading user_attributes column
*/
bool Acl_table_user_reader::read_user_attributes(ACL_USER &user) {
  /* Read user_attributes field */
  if (m_table->s->fields > m_table_schema->user_attributes_idx()) {
    Auth_id auth_id(user.user ? user.user : "",
                    user.user ? strlen(user.user) : 0,
                    user.host.get_host() ? user.host.get_host() : "",
                    user.host.get_host() ? strlen(user.host.get_host()) : 0);
    Acl_user_attributes user_attributes(&m_mem_root, true, auth_id,
                                        user.access);
    if (!m_table->field[m_table_schema->user_attributes_idx()]->is_null()) {
      if (parse_user_attributes(m_thd, m_table, m_table_schema,
                                user_attributes)) {
        LogErr(WARNING_LEVEL, ER_WARNING_AUTHCACHE_INVALID_USER_ATTRIBUTES,
               user.user ? user.user : "",
               user.host.get_host() ? user.host.get_host() : "");
        return true;
      }

      // 1. Read additional password
      std::string additional_password =
          user_attributes.get_additional_password();
      if (additional_password.length()) {
        user.credentials[SECOND_CRED].m_auth_string.length =
            additional_password.length();
        char *auth_string = static_cast<char *>(m_mem_root.Alloc(
            user.credentials[SECOND_CRED].m_auth_string.length + 1));
        memcpy(auth_string, additional_password.c_str(),
               user.credentials[SECOND_CRED].m_auth_string.length);
        auth_string[user.credentials[SECOND_CRED].m_auth_string.length] = 0;
        user.credentials[SECOND_CRED].m_auth_string.str = auth_string;
      } else {
        user.credentials[SECOND_CRED].m_auth_string = EMPTY_CSTR;
      }

      /* Validate the hash string. */
      plugin_ref plugin = nullptr;
      plugin = my_plugin_lock_by_name(nullptr, user.plugin,
                                      MYSQL_AUTHENTICATION_PLUGIN);
      if (plugin) {
        st_mysql_auth *auth = (st_mysql_auth *)plugin_decl(plugin)->info;
        if (auth->validate_authentication_string(
                const_cast<char *>(
                    user.credentials[SECOND_CRED].m_auth_string.str),
                user.credentials[SECOND_CRED].m_auth_string.length)) {
          LogErr(WARNING_LEVEL, ER_AUTHCACHE_USER_IGNORED_INVALID_PASSWORD,
                 user.user ? user.user : "",
                 user.host.get_host() ? user.host.get_host() : "");
          plugin_unlock(nullptr, plugin);
          return true;
        }
        plugin_unlock(nullptr, plugin);
      }
    } else {
      // user_attributes column is NULL. So use suitable defaults.
      user.credentials[SECOND_CRED].m_auth_string = EMPTY_CSTR;
    }
    *m_restrictions = user_attributes.get_restrictions();
    user.password_locked_state.set_parameters(
        user_attributes.get_password_lock_time_days(),
        user_attributes.get_failed_login_attempts());
  } else {
    user.credentials[SECOND_CRED].m_auth_string = EMPTY_CSTR;
  }
  return false;
}

/**
  Add a recently read row in acl_users

  @param [in] user ACL_USER structure
*/
void Acl_table_user_reader::add_row_to_acl_users(ACL_USER &user) {
  LEX_CSTRING auth = {user.credentials[PRIMARY_CRED].m_auth_string.str,
                      user.credentials[PRIMARY_CRED].m_auth_string.length};
  LEX_CSTRING second_auth = {
      user.credentials[SECOND_CRED].m_auth_string.str,
      user.credentials[SECOND_CRED].m_auth_string.length};
  LEX_ALTER password_life;
  password_life.update_password_expired_column = user.password_expired;
  password_life.expire_after_days = user.password_lifetime;
  password_life.use_default_password_lifetime =
      user.use_default_password_lifetime;
  password_life.account_locked = user.account_locked;
  password_life.use_default_password_history =
      user.password_history_length ? true : false;
  password_life.password_history_length =
      user.use_default_password_history ? 0 : user.password_history_length;
  password_life.use_default_password_history =
      user.use_default_password_history;
  password_life.use_default_password_reuse_interval =
      user.password_reuse_interval ? true : false;
  password_life.password_reuse_interval =
      user.use_default_password_reuse_interval ? 0
                                               : user.password_reuse_interval;
  password_life.use_default_password_reuse_interval =
      user.use_default_password_reuse_interval;
  password_life.update_password_require_current = user.password_require_current;
  password_life.failed_login_attempts =
      user.password_locked_state.get_failed_login_attempts();
  password_life.update_failed_login_attempts = true;
  password_life.password_lock_time =
      user.password_locked_state.get_password_lock_time_days();
  password_life.update_password_lock_time = true;
  acl_users_add_one(user.user, user.host.get_host(), user.ssl_type,
                    user.ssl_cipher, user.x509_issuer, user.x509_subject,
                    &user.user_resource, user.access, user.plugin, auth,
                    second_auth, user.password_last_changed, password_life,
                    false, *m_restrictions, password_life.failed_login_attempts,
                    password_life.password_lock_time, m_thd);
}

/**
  Read a row from mysql.user table and add it to in-memory structure

  @param [in] is_old_db_layout               mysql.user table is in old
  format
  @param [in] super_users_with_empty_plugin  User has SUPER privilege

  @returns Status of reading a row
    @retval false Success
    @retval true  Error reading the row. Unless critical, keep reading
  further.
*/
bool Acl_table_user_reader::read_row(bool &is_old_db_layout,
                                     bool &super_users_with_empty_plugin) {
  bool password_expired = false;
  DBUG_TRACE;
  /* Reading record from mysql.user */
  ACL_USER user;
  reset_acl_user(user);
  read_account_name(user);
  if (read_authentication_string(user)) return true;
  read_privileges(user);
  read_ssl_fields(user);
  read_user_resources(user);
  if (read_plugin_info(user, super_users_with_empty_plugin, is_old_db_layout))
    return false;
  read_password_expiry(user, password_expired);
  read_password_locked(user);
  read_password_last_changed(user);
  read_password_lifetime(user);
  read_password_history_fields(user);
  read_password_reuse_time_fields(user);
  read_password_require_current(user);
  if (read_user_attributes(user)) return false;

  set_user_salt(&user);
  user.password_expired = password_expired;

  add_row_to_acl_users(user);

  return false;
}

/**
  Driver function for mysql.user reader

  Reads rows from table. If a row is valid, adds corresponding information
  to in-memory structure.

  @returns status of reading mysql.user table
    @retval false Success
    @retval true  Error reading the table. Probably corrupt.
*/
bool Acl_table_user_reader::driver() {
  DBUG_TRACE;
  bool is_old_db_layout;
  bool super_users_with_empty_plugin = false;
  if (setup_table(is_old_db_layout)) return true;
  allow_all_hosts = false;
  int read_rec_errcode;
  while (!(read_rec_errcode = m_iterator->Read())) {
    if (read_row(is_old_db_layout, super_users_with_empty_plugin)) return true;
  }

  m_iterator.reset();
  if (read_rec_errcode > 0) return true;
  std::sort(acl_users->begin(), acl_users->end(), ACL_compare());
  acl_users->shrink_to_fit();
  rebuild_cached_acl_users_for_name();

  if (super_users_with_empty_plugin) {
    LogErr(WARNING_LEVEL, ER_NO_SUPER_WITHOUT_USER_PLUGIN);
  }

  return false;
}

Password_lock::Password_lock()
    : password_lock_time_days(0), failed_login_attempts(0) {}

Password_lock &Password_lock::operator=(const Password_lock &other) {
  if (this != &other) {
    password_lock_time_days = other.password_lock_time_days;
    failed_login_attempts = other.failed_login_attempts;
  }
  return *this;
}

Password_lock &Password_lock::operator=(Password_lock &&other) {
  if (this != &other) {
    std::swap(password_lock_time_days, other.password_lock_time_days);
    std::swap(failed_login_attempts, other.failed_login_attempts);
  }
  return *this;
}

Password_lock::Password_lock(const Password_lock &other) {
  password_lock_time_days = other.password_lock_time_days;
  failed_login_attempts = other.failed_login_attempts;
}

Password_lock::Password_lock(Password_lock &&other) {
  std::swap(password_lock_time_days, other.password_lock_time_days);
  std::swap(failed_login_attempts, other.failed_login_attempts);
}
}  // namespace acl_table

/**
  Search and create/update a record for the user requested.

  @param [in] thd              The current thread.
  @param [in] table            Pointer to a TABLE object of mysql.user table
  @param [in] combo            User information
  @param [in] rights           Rights requested
  @param [in] revoke_grant     Set to true if a REVOKE command is executed
  @param [in] can_create_user  Set true if it's allowed to create user
  @param [in] what_to_update   Bitmap indicating which attributes need to be
                               updated.
  @param [in] restrictions     Restrictions handle if there is any

  @return  Operation result
  @retval  0    OK.
    @retval  < 0  System error or storage engine error happen
    @retval  > 0  Error in handling current user entry but still can continue
                  processing subsequent user specified in the ACL statement.
*/

int replace_user_table(THD *thd, TABLE *table, LEX_USER *combo, ulong rights,
                       bool revoke_grant, bool can_create_user,
                       acl_table::Pod_user_what_to_update &what_to_update,
                       Restrictions *restrictions /*= nullptr*/) {
  acl_table::Acl_table_user_writer user_table(thd, table, combo, rights,
                                              revoke_grant, can_create_user,
                                              what_to_update, restrictions);
  acl_table::Acl_table_user_writer_status return_value(thd->mem_root);

  DBUG_TRACE;
  DBUG_ASSERT(assert_acl_cache_write_lock(thd));

  return_value = user_table.driver();

  if (!(return_value.error || return_value.skip_cache_update)) {
    bool old_row_exists = (user_table.get_operation_mode() ==
                           acl_table::Acl_table_operation::OP_UPDATE);
    bool builtin_plugin = auth_plugin_is_built_in(combo->plugin.str);
    bool update_password = (what_to_update.m_what & PLUGIN_ATTR);
    LEX *lex = thd->lex;
    /*
      Convert the time when the password was changed from timeval
      structure to MYSQL_TIME format, to store it in cache.
    */
    MYSQL_TIME password_change_time;

    if (builtin_plugin && (update_password || !old_row_exists))
      thd->variables.time_zone->gmt_sec_to_TIME(
          &password_change_time,
          (my_time_t)return_value.password_change_timestamp.tv_sec);
    else
      password_change_time.time_type = MYSQL_TIMESTAMP_ERROR;
    clear_and_init_db_cache(); /* Clear privilege cache */
    if (old_row_exists)
      acl_update_user(combo->user.str, combo->host.str, lex->ssl_type,
                      lex->ssl_cipher, lex->x509_issuer, lex->x509_subject,
                      &lex->mqh, return_value.updated_rights, combo->plugin,
                      combo->auth, return_value.second_cred,
                      password_change_time, combo->alter_status,
                      return_value.restrictions, what_to_update,
                      return_value.password_lock.failed_login_attempts,
                      return_value.password_lock.password_lock_time_days);
    else
      acl_insert_user(thd, combo->user.str, combo->host.str, lex->ssl_type,
                      lex->ssl_cipher, lex->x509_issuer, lex->x509_subject,
                      &lex->mqh, return_value.updated_rights, combo->plugin,
                      combo->auth, password_change_time, combo->alter_status,
                      return_value.restrictions,
                      return_value.password_lock.failed_login_attempts,
                      return_value.password_lock.password_lock_time_days);
  }
  return return_value.error;
}

/**
  Read data from user table and fill in-memory caches

  @param [in] thd       THD handle
  @param [in] m_table   mysql.user table handle

  @returns status of reading data from table
    @retval true  Error reading data. Don't trust it.
    @retval false All well.
*/
bool read_user_table(THD *thd, TABLE *m_table) {
  acl_table::Acl_table_user_reader acl_table_user_reader(thd, m_table);
  DBUG_TRACE;

  if (acl_table_user_reader.driver()) return true;

  return false;
}
