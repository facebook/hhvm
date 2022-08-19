/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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
#include "sql/auth/dynamic_privilege_table.h"

#include <string.h>
#include <memory>
#include <string>

#include "lex_string.h"
#include "m_ctype.h"
#include "my_alloc.h"
#include "my_base.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql/components/my_service.h"
#include "mysql/components/service.h"
#include "mysql/components/services/dynamic_privilege.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/components/services/registry.h"
#include "mysql/mysql_lex_string.h"
#include "mysql/psi/psi_base.h"
#include "mysql/service_plugin_registry.h"
#include "mysqld_error.h"
#include "sql/auth/auth_common.h"
#include "sql/auth/auth_internal.h"
#include "sql/auth/sql_auth_cache.h"
#include "sql/auth/sql_user_table.h"
#include "sql/current_thd.h"
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/key.h"
#include "sql/records.h"
#include "sql/row_iterator.h"
#include "sql/sql_const.h"
#include "sql/table.h"

class THD;

#define MYSQL_DYNAMIC_PRIV_FIELD_USER 0
#define MYSQL_DYNAMIC_PRIV_FIELD_HOST 1
#define MYSQL_DYNAMIC_PRIV_FIELD_PRIV 2
#define MYSQL_DYNAMIC_PRIV_FIELD_GRANT 3

Dynamic_privilege_register g_dynamic_privilege_register;

/**
  This function returns a pointer to a global variable allocated on the heap.
  @return A pointer to the dynamic privilege register.
*/

Dynamic_privilege_register *get_dynamic_privilege_register(void) {
  return &g_dynamic_privilege_register;
}

/**
  Given an open table handler this function refresh the list of dynamic
  privilege grants by reading the dynamic_privilege table.

  If an error is raised, this function will set the DA.

  @param thd The thread handle
  @param tablelst An handle to an open table
  @return Error state
    @retval true An error occurred.
    @retval false Success
*/

bool populate_dynamic_privilege_caches(THD *thd, TABLE_LIST *tablelst) {
  DBUG_TRACE;
  bool error = false;
  DBUG_ASSERT(assert_acl_cache_write_lock(thd));
  Acl_table_intact table_intact(thd);

  if (table_intact.check(tablelst[0].table, ACL_TABLES::TABLE_DYNAMIC_PRIV))
    return true;

  TABLE *table = tablelst[0].table;
  table->use_all_columns();
  unique_ptr_destroy_only<RowIterator> iterator =
      init_table_iterator(thd, table, nullptr, false,
                          /*ignore_not_found_rows=*/false);
  if (iterator == nullptr) {
    my_error(ER_TABLE_CORRUPT, MYF(0), table->s->db.str,
             table->s->table_name.str);
    return true;
  }
  int read_rec_errcode;
  MEM_ROOT tmp_mem;
  char percentile_character[2] = {'%', '\0'};
  char empty_str = '\0';
  /*
    We need the the dynamic privilege register in order to register any unknown
    privilege identifiers.
  */
  SERVICE_TYPE(registry) *r = mysql_plugin_registry_acquire();
  {
    my_service<SERVICE_TYPE(dynamic_privilege_register)> service(
        "dynamic_privilege_register.mysql_server", r);
    if (!service.is_valid()) {
      return true;
    }
    init_alloc_root(PSI_NOT_INSTRUMENTED, &tmp_mem, 256, 0);
    while (!error && !(read_rec_errcode = iterator->Read())) {
      char *host =
          get_field(&tmp_mem, table->field[MYSQL_DYNAMIC_PRIV_FIELD_HOST]);
      if (host == nullptr) host = &percentile_character[0];
      char *user =
          get_field(&tmp_mem, table->field[MYSQL_DYNAMIC_PRIV_FIELD_USER]);
      if (user == nullptr) user = &empty_str;
      char *priv =
          get_field(&tmp_mem, table->field[MYSQL_DYNAMIC_PRIV_FIELD_PRIV]);
      char *with_grant_option = get_field(
          &tmp_mem, table->field[MYSQL_DYNAMIC_PRIV_FIELD_WITH_GRANT_OPTION]);
      if (priv == nullptr) priv = &empty_str;
      my_caseup_str(system_charset_info, priv);
      LEX_CSTRING str_priv = {priv, strlen(priv)};
      LEX_CSTRING str_user = {user, strlen(user)};
      LEX_CSTRING str_host = {host, strlen(host)};
      Update_dynamic_privilege_table no_update;
      if (grant_dynamic_privilege(str_priv, str_user, str_host,
                                  (*with_grant_option == 'Y' ? true : false),
                                  no_update)) {
        /*
          This privilege ID hasn't been registered yet. It can happen when a
          previously grant has been given but the plugin or component which owns
          the privilege ID isn't loaded yet.
          The policy is that any privilege ID that exist in mysql.global_grants
          is a valid privilege ID.
        */
        if (service->register_privilege(str_priv.str, str_priv.length) ||
            grant_dynamic_privilege(str_priv, str_user, str_host,
                                    (*with_grant_option == 'Y' ? true : false),
                                    no_update)) {
          /*
            Only if we fail a second time we assume that the error was critical
            and operation have to be aborted.
            We don't return immediately here because we need to release the
            registry first.
          */
          error = true;
        }
      }
    }
    /*
      To avoid any issues with inconsistencies we unconditionally increase
      acl cache version here.
    */
    get_global_acl_cache()->increase_version();
  }  // exit scope
  mysql_plugin_registry_release(r);
  return error;
}

/**
  Delete or insert a row in the mysql.dynamic_privilege table.
  @param thd Thread handler
  @param table The opened table to be modified
  @param auth_id Target authorization ID
  @param privilege Privilege object ID
  @param with_grant_option Flag indicating if the grant option is set
  @param delete_option Flag indicating if this is an insert or delete

  If an error has occurred the DA is not set.

  @see Update_dynamic_privilege_table

  @return Error state
   @retval true An error occurred
   @retval false Success
*/

bool modify_dynamic_privileges_in_table(THD *thd, TABLE *table,
                                        const Auth_id_ref &auth_id,
                                        const LEX_CSTRING &privilege,
                                        bool with_grant_option,
                                        bool delete_option) {
  DBUG_TRACE;
  int ret = 0;
  uchar user_key[MAX_KEY_LENGTH];
  Acl_table_intact table_intact(thd);

  if (table_intact.check(table, ACL_TABLES::TABLE_DYNAMIC_PRIV)) return true;

  table->use_all_columns();
  table->field[MYSQL_DYNAMIC_PRIV_FIELD_HOST]->store(
      auth_id.second.str, auth_id.second.length, system_charset_info);
  table->field[MYSQL_DYNAMIC_PRIV_FIELD_USER]->store(
      auth_id.first.str, auth_id.first.length, system_charset_info);
  table->field[MYSQL_DYNAMIC_PRIV_FIELD_PRIV]->store(
      privilege.str, privilege.length, system_charset_info);
  key_copy(user_key, table->record[0], table->key_info,
           table->key_info->key_length);
  table->field[MYSQL_DYNAMIC_PRIV_FIELD_WITH_GRANT_OPTION]->store(
      (with_grant_option == true ? "Y" : "N"), 1, system_charset_info);
  ret = table->file->ha_index_read_idx_map(table->record[0], 0, user_key,
                                           HA_WHOLE_KEY, HA_READ_KEY_EXACT);
  if (delete_option) {
    if (ret == 0) {
      DBUG_PRINT("note",
                 ("Delete dynamic privilege %s for `%s`@`%s`", privilege.str,
                  auth_id.first.str, auth_id.second.str));
      ret = table->file->ha_delete_row(table->record[0]);
    } else if (ret == HA_ERR_KEY_NOT_FOUND) {
      /* If the key didn't exist the record is already gone and all is well. */
      return false;
    }
  } else if (ret == HA_ERR_KEY_NOT_FOUND && !delete_option) {
    /* Insert new edge into table */
    DBUG_PRINT("note",
               ("Insert dynamic privilege %s for `%s`@`%s` %s", privilege.str,
                auth_id.first.str, auth_id.second.str,
                (with_grant_option == true ? "WITH GRANT OPTION" : "")));
    ret = table->file->ha_write_row(table->record[0]);
  }
  return ret != 0;
}

bool iterate_all_dynamic_privileges(THD *thd,
                                    std::function<bool(const char *)> action) {
  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock()) return true;
  Dynamic_privilege_register::iterator it =
      get_dynamic_privilege_register()->begin();
  Dynamic_privilege_register::iterator end_it =
      get_dynamic_privilege_register()->end();
  while (it != end_it) {
    if (action(it->c_str())) return true;
    ++it;
  }
  return false;
}
