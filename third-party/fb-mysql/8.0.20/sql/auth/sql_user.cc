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

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <algorithm>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "map_helpers.h"
#include "my_alloc.h"
#include "my_base.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_sqlcommand.h"
#include "my_sys.h"
#include "my_time.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/mysql_lex_string.h"
#include "mysql/plugin.h"
#include "mysql/plugin_audit.h"
#include "mysql/plugin_auth.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/psi_base.h"
#include "mysql_com.h"
#include "mysql_time.h"
#include "mysqld_error.h"
#include "password.h" /* my_make_scrambled_password */
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"
#include "sql/auth/dynamic_privilege_table.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/debug_sync.h"  // DEBUG_SYNC
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/key.h"
#include "sql/log_event.h" /* append_query_string */
#include "sql/protocol.h"
#include "sql/sql_audit.h"
#include "sql/sql_class.h"
#include "sql/sql_connect.h"
#include "sql/sql_const.h"
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_parse.h"  /* check_access */
#include "sql/sql_plugin.h" /* lock_plugin_data etc. */
#include "sql/sql_plugin_ref.h"
#include "sql/strfunc.h"
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/thd_raii.h"
#include "sql_string.h"
#include "violite.h"
/* key_restore */

#include "prealloced_array.h"
#include "sql/auth/auth_internal.h"
#include "sql/auth/sql_auth_cache.h"
#include "sql/auth/sql_authentication.h"
#include "sql/auth/sql_user_table.h"
#include "sql/current_thd.h"
#include "sql/derror.h" /* ER_THD */
#include "sql/log.h"
#include "sql/mysqld.h"
#include "sql/sql_rewrite.h"

#include <openssl/rand.h>  // RAND_bytes
/**
  Auxiliary function for constructing a  user list string.
  This function is used for error reporting and logging.

  @param thd     Thread context
  @param str     A String to store the user list.
  @param user    A LEX_USER which will be appended into user list.
  @param comma   If true, append a ',' before the the user.
 */
void log_user(THD *thd, String *str, LEX_USER *user, bool comma = true) {
  String from_user(user->user.str, user->user.length, system_charset_info);
  String from_plugin(user->plugin.str, user->plugin.length,
                     system_charset_info);
  String from_auth(user->auth.str, user->auth.length, system_charset_info);
  String from_host(user->host.str, user->host.length, system_charset_info);

  if (comma) str->append(',');
  append_query_string(thd, system_charset_info, &from_user, str);
  str->append(STRING_WITH_LEN("@"));
  append_query_string(thd, system_charset_info, &from_host, str);
}

extern bool initialized;

/*
 Enumeration of various ACL's and Hashes used in handle_grant_struct()
*/
enum enum_acl_lists {
  USER_ACL = 0,
  DB_ACL,
  COLUMN_PRIVILEGES_HASH,
  PROC_PRIVILEGES_HASH,
  FUNC_PRIVILEGES_HASH,
  PROXY_USERS_ACL
};

bool check_change_password(THD *thd, const char *host, const char *user,
                           bool retain_current_password) {
  Security_context *sctx;
  DBUG_ASSERT(initialized);
  sctx = thd->security_context();
  if (!thd->slave_thread &&
      (strcmp(sctx->user().str, user) ||
       my_strcasecmp(system_charset_info, host, sctx->priv_host().str))) {
    if (sctx->password_expired()) {
      my_error(ER_MUST_CHANGE_PASSWORD, MYF(0));
      return true;
    }
    if (check_access(thd, UPDATE_ACL, consts::mysql.c_str(), nullptr, nullptr,
                     true, false))
      return (true);

    if (sctx->can_operate_with({user, host}, consts::system_user))
      return (true);
  }

  if (retain_current_password) {
    if (check_access(thd, UPDATE_ACL, consts::mysql.c_str(), nullptr, nullptr,
                     true, true) &&
        !(sctx->check_access(CREATE_USER_ACL, consts::mysql)) &&
        !(sctx->has_global_grant(STRING_WITH_LEN("APPLICATION_PASSWORD_ADMIN"))
              .first)) {
      my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0),
               "CREATE USER or APPLICATION_PASSWORD_ADMIN");
      return true;
    }
  }

  if (!thd->slave_thread && likely((get_server_state() == SERVER_OPERATING)) &&
      !strcmp(thd->security_context()->priv_user().str, "")) {
    my_error(ER_PASSWORD_ANONYMOUS_USER, MYF(0));
    return true;
  }

  return false;
}

/**
  Auxiliary function for constructing CREATE USER sql for a given user.

  @param thd                    Thread context
  @param user_name              user for which the sql should be constructed.
  @param are_both_users_same    If the command is issued for self or not.

  @retval
    0         OK.
    1         Error.
 */

bool mysql_show_create_user(THD *thd, LEX_USER *user_name,
                            bool are_both_users_same) {
  int error = 0;
  ACL_USER *acl_user;
  LEX *lex = thd->lex;
  Protocol *protocol = thd->get_protocol();
  USER_RESOURCES tmp_user_resource;
  enum SSL_type ssl_type;
  const char *ssl_cipher, *x509_issuer, *x509_subject;
  static const int COMMAND_BUFFER_LENGTH = 2048;
  char buff[COMMAND_BUFFER_LENGTH];
  Item_string *field = nullptr;
  List<Item> field_list;
  String sql_text(buff, sizeof(buff), system_charset_info);
  LEX_ALTER alter_info;
  List_of_auth_id_refs default_roles;
  List<LEX_USER> *old_default_roles = lex->default_roles;
  bool hide_password_hash = false;

  DBUG_TRACE;
  if (are_both_users_same) {
    TABLE_LIST t1("mysql", "user", TL_READ);
    hide_password_hash =
        check_table_access(thd, SELECT_ACL, &t1, false, UINT_MAX, true);
  }

  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
  if (!acl_cache_lock.lock()) return true;

  if (!(acl_user =
            find_acl_user(user_name->host.str, user_name->user.str, true))) {
    String wrong_users;
    log_user(thd, &wrong_users, user_name, wrong_users.length() > 0);
    my_error(ER_CANNOT_USER, MYF(0), "SHOW CREATE USER",
             wrong_users.c_ptr_safe());
    return true;
  }
  /* fill in plugin, auth_str from acl_user */
  user_name->auth.str = acl_user->credentials[PRIMARY_CRED].m_auth_string.str;
  user_name->auth.length =
      acl_user->credentials[PRIMARY_CRED].m_auth_string.length;
  user_name->plugin = acl_user->plugin;
  user_name->uses_identified_by_clause = true;
  user_name->uses_identified_with_clause = false;
  user_name->uses_authentication_string_clause = false;
  user_name->retain_current_password = false;
  user_name->discard_old_password = false;

  /* make a copy of user resources, ssl and password expire attributes */
  tmp_user_resource = lex->mqh;
  lex->mqh = acl_user->user_resource;

  /* Set specified_limits flags so user resources are shown properly. */
  if (lex->mqh.user_conn)
    lex->mqh.specified_limits |= USER_RESOURCES::USER_CONNECTIONS;
  if (lex->mqh.questions)
    lex->mqh.specified_limits |= USER_RESOURCES::QUERIES_PER_HOUR;
  if (lex->mqh.updates)
    lex->mqh.specified_limits |= USER_RESOURCES::UPDATES_PER_HOUR;
  if (lex->mqh.conn_per_hour)
    lex->mqh.specified_limits |= USER_RESOURCES::CONNECTIONS_PER_HOUR;

  ssl_type = lex->ssl_type;
  ssl_cipher = lex->ssl_cipher;
  x509_issuer = lex->x509_issuer;
  x509_subject = lex->x509_subject;

  lex->ssl_type = acl_user->ssl_type;
  lex->ssl_cipher = acl_user->ssl_cipher;
  lex->x509_issuer = acl_user->x509_issuer;
  lex->x509_subject = acl_user->x509_subject;

  alter_info = lex->alter_password;

  lex->alter_password.update_password_expired_column =
      acl_user->password_expired;
  lex->alter_password.use_default_password_lifetime =
      acl_user->use_default_password_lifetime;
  lex->alter_password.expire_after_days = acl_user->password_lifetime;
  lex->alter_password.update_account_locked_column = true;
  lex->alter_password.account_locked = acl_user->account_locked;
  lex->alter_password.update_password_expired_fields = true;

  lex->alter_password.password_history_length =
      acl_user->password_history_length;
  lex->alter_password.use_default_password_history =
      acl_user->use_default_password_history;
  lex->alter_password.update_password_history =
      !acl_user->use_default_password_history;

  lex->alter_password.password_reuse_interval =
      acl_user->password_reuse_interval;
  lex->alter_password.use_default_password_reuse_interval =
      acl_user->use_default_password_reuse_interval;
  lex->alter_password.update_password_reuse_interval =
      !acl_user->use_default_password_reuse_interval;
  lex->alter_password.update_password_require_current =
      acl_user->password_require_current;

  lex->alter_password.failed_login_attempts =
      acl_user->password_locked_state.get_failed_login_attempts();
  lex->alter_password.password_lock_time =
      acl_user->password_locked_state.get_password_lock_time_days();

  lex->alter_password.update_failed_login_attempts =
      lex->alter_password.failed_login_attempts != 0;
  lex->alter_password.update_password_lock_time =
      lex->alter_password.password_lock_time != 0;

  /* send the metadata to client */
  field = new Item_string("", 0, &my_charset_latin1);
  field->max_length = 256;
  strxmov(buff, "CREATE USER for ", user_name->user.str, "@",
          user_name->host.str, NullS);
  field->item_name.set(buff);
  field_list.push_back(field);
  if (thd->send_result_metadata(&field_list,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF)) {
    error = 1;
    goto err;
  }
  sql_text.length(0);
  if (lex->sql_command == SQLCOM_SHOW_CREATE_USER ||
      lex->sql_command == SQLCOM_CREATE_USER) {
    /*
      Recreate LEX for default roles given an ACL_USER. This will later be used
      by rewrite_default_roles() called by Rewriter_show_create_user::rewrite()
    */
    get_default_roles(create_authid_from(acl_user), default_roles);
    if (default_roles.size() > 0) {
      LEX_STRING *tmp_user = nullptr;
      LEX_STRING *tmp_host = nullptr;
      /*
        Make sure we reallocate the default_roles list when using it outside of
        parser code so it has the same mem root as its items.
      */
      lex->default_roles = new (thd->mem_root) List<LEX_USER>;
      for (auto &&role : default_roles) {
        if (!(tmp_user = make_lex_string_root(thd->mem_root, role.first.str,
                                              role.first.length)) ||
            !(tmp_host = make_lex_string_root(thd->mem_root, role.second.str,
                                              role.second.length))) {
          error = 1;
          goto err;
        }
        LEX_USER *lex_role = LEX_USER::alloc(thd, tmp_user, tmp_host);
        if (lex_role == nullptr) {
          error = 1;
          goto err;
        }
        lex->default_roles->push_back(lex_role);
      }
    }
  }
  lex->users_list.push_back(user_name);
  {
    Show_user_params show_user_params(
        hide_password_hash, thd->variables.print_identified_with_as_hex);
    /*
      By disabling instrumentation, we're requesting a rewrite to our
      local buffer, sql_text. The value on the THD and those seen in
      instrumentation remain unchanged.
    */
    mysql_rewrite_acl_query(thd, sql_text, Consumer_type::STDOUT,
                            &show_user_params, false);
  }

  /* send the result row to client */
  protocol->start_row();
  protocol->store_string(sql_text.ptr(), sql_text.length(), sql_text.charset());
  if (protocol->end_row()) {
    error = 1;
    goto err;
  }

err:
  lex->default_roles = old_default_roles;
  /* restore user resources, ssl and password expire attributes */
  lex->mqh = tmp_user_resource;
  lex->ssl_type = ssl_type;
  lex->ssl_cipher = ssl_cipher;
  lex->x509_issuer = x509_issuer;
  lex->x509_subject = x509_subject;

  lex->alter_password = alter_info;
  my_eof(thd);
  return error;
}

#include "sql/query_result.h"  // Time_zone
#include "sql/tztime.h"

/**
  Perform credentials history check and update the password history table

  Note that the data for the checks are extracted from LEX_USER. So these
  need to be up to date in all cases.

  How credential history checks are performed:
  ~~~
  count= 0;
  FOR SELECT * FROM mysql.password_history ORDER BY USER,HOST,TS DESC
    WHERE USER=::current_user AND HOST=::current_host
  {
     if (count >= ::password_history && (NOW() - ts) > ::password_reuse_time)
     {
       delete row;
       continue;
     }

     if (CRED was produced by ::password)
       signal("wrong password");

     count = count + 1;
  }

  INSERT INTO mysql.password_history (USER,HOST,TS,CRED)
    VALUES (::current_user, ::current_host, NOW(), ::hashed_password);
  ~~~

  @param thd      The current thread
  @param user     The user account user to operate on
  @param host     The user acount host to operate on
  @param password_history The effective password history value
  @param password_reuse_interval The effective password reuse interval value
  @param auth     auth plugin to use for verification
  @param cleartext  the clear text password supplied
  @param cleartext_length length of cleartext password
  @param cred_hash hash of the credential to be inserted into the history
  @param cred_hash_length length of cred_hash
  @param history_table  The opened history table
  @param what_to_set   The mask of what to set
  @retval false   Password is OK
  @retval true    Password is not OK
*/
static bool auth_verify_password_history(
    THD *thd, LEX_CSTRING *user, LEX_CSTRING *host, uint32 password_history,
    long password_reuse_interval, st_mysql_auth *auth, const char *cleartext,
    unsigned int cleartext_length, const char *cred_hash,
    unsigned int cred_hash_length, TABLE_LIST *history_table,
    ulong what_to_set) {
  TABLE *table = history_table->table;
  uchar user_key[MAX_KEY_LENGTH];
  uint key_prefix_length;
  int error;
  Field *user_field, *host_field, *ts_field, *cred_field;
  bool result = false;

  Acl_table_intact intact(thd);

  if (!table) {
    if ((password_history || password_reuse_interval) && cred_hash_length) {
      /* fail if there's no history table and we need to update it */
      my_error(ER_NO_SUCH_TABLE, MYF(0), "mysql", "password_history");
      return true;
    }
    /* threat missing table as absent and empty otherwise */
    return false;
  }

  /* all good: we don't handle empty passwords in history */
  if (!cleartext_length && !cred_hash_length) return false;

  /* invalid table causes verification to fail */
  if (intact.check(history_table->table, ACL_TABLES::TABLE_PASSWORD_HISTORY))
    return true;

  user_field = table->field[MYSQL_PASSWORD_HISTORY_FIELD_USER];
  host_field = table->field[MYSQL_PASSWORD_HISTORY_FIELD_HOST];
  ts_field = table->field[MYSQL_PASSWORD_HISTORY_FIELD_PASSWORD_TIMESTAMP];
  cred_field = table->field[MYSQL_PASSWORD_HISTORY_FIELD_PASSWORD];

  table->use_all_columns();

  /* create the search key on user and host */
  user_field->store(user->str, user->length, system_charset_info);
  host_field->store(host->str, host->length, system_charset_info);
  key_prefix_length = (table->key_info->key_part[0].store_length +
                       table->key_info->key_part[1].store_length);
  key_copy(user_key, table->record[0], table->key_info, key_prefix_length);

  uint32 count = 0;

  int rc = table->file->ha_index_init(0, true);

  if (rc) {
    table->file->print_error(rc, MYF(0));
    result = true;
    goto end;
  }

  /* find the first matching record by the first 2 fields of a key */
  error = table->file->ha_index_read_idx_map(
      table->record[0], 0, user_key, (key_part_map)((1L << 0) | (1L << 1)),
      HA_READ_KEY_EXACT);

  /* fetch the current day */
  MYSQL_TIME tm_now;
  long now_day;
  thd->time_zone()->gmt_sec_to_TIME(&tm_now, thd->query_start_timeval_trunc(6));
  now_day = calc_daynr(tm_now.year, tm_now.month, tm_now.day);

  /* iterate over the password history rows for the user */
  while (!error) {
    MYSQL_TIME ts_val;
    char outbuf[MAX_FIELD_WIDTH] = {0};
    long ts_day, date_diff;
    String cred_val(&outbuf[0], sizeof(outbuf), &my_charset_bin);
    int is_error = 0;

    /* fetch the recorded time */
    if (ts_field->get_date(&ts_val, 0)) goto get_next_row;

    /* convert to a day number */
    ts_day = calc_daynr(ts_val.year, ts_val.month, ts_val.day);

    /* get the difference in days */
    date_diff = now_day - ts_day;

    count++;

    /*
       We check everything that's in any range, including the last row(s)
    */
    if (count <= password_history || date_diff < password_reuse_interval) {
      /* fetch the cred field */
      cred_field->val_str(&cred_val);

      /*
        Check if the password matches the stored hash.
        There can't possibly be a match when we're altering the plugin
        used. So we check for that and just delete the rows in this case.
        But we still check the validity of the hash in case someone has
        tampered with the history table manually.
      */
      if (cleartext_length && cleartext &&
          0 == (what_to_set & DIFFERENT_PLUGIN_ATTR) &&
          (auth->authentication_flags & AUTH_FLAG_USES_INTERNAL_STORAGE) &&
          auth->validate_authentication_string &&
          !auth->validate_authentication_string(cred_val.c_ptr_safe(),
                                                (unsigned)cred_val.length()) &&
          auth->compare_password_with_hash &&
          !auth->compare_password_with_hash(
              cred_val.c_ptr_safe(), (unsigned long)cred_val.length(),
              cleartext, (unsigned long)cleartext_length, &is_error) &&
          !is_error) {
        my_error(ER_CREDENTIALS_CONTRADICT_TO_HISTORY, MYF(0), user->length,
                 user->str, host->length, host->str);
        /* password found in history */
        result = true;
        goto end;
      }
    }

    /*
      Delete all rows outside all check ranges, including the last row in
      history range count, since we're to add another one.
    */
    if ((count >= password_history &&
         (!password_reuse_interval || date_diff > password_reuse_interval)) ||
        0L != (what_to_set & DIFFERENT_PLUGIN_ATTR)) {
      int ignore_error;
      /* delete and go on, even if there's an error deleting */
      if (0 != (ignore_error = table->file->ha_delete_row(table->record[0])))
        table->file->print_error(ignore_error, MYF(0));
      goto get_next_row;
    }

  get_next_row:
    error = table->file->ha_index_next_same(table->record[0], user_key,
                                            key_prefix_length);
  }

  /* something went wrong reading */
  if (error != HA_ERR_KEY_NOT_FOUND && error != HA_ERR_END_OF_FILE) {
    table->file->print_error(error, MYF(0));
    result = true;
    goto end;
  }

  /* Update the history if a hash is supplied and the plugin supports it */
  if ((password_history || password_reuse_interval) && cred_hash_length &&
      (auth->authentication_flags & AUTH_FLAG_USES_INTERNAL_STORAGE)) {
    /* add history if needed */
    restore_record(table, s->default_values);
    table->field[MYSQL_PASSWORD_HISTORY_FIELD_USER]->store(
        user->str, user->length, system_charset_info);
    table->field[MYSQL_PASSWORD_HISTORY_FIELD_HOST]->store(
        host->str, host->length, system_charset_info);
    table->field[MYSQL_PASSWORD_HISTORY_FIELD_PASSWORD_TIMESTAMP]->store_time(
        &tm_now);
    table->field[MYSQL_PASSWORD_HISTORY_FIELD_PASSWORD]->store(
        cred_hash, cred_hash_length, &my_charset_utf8_bin);
    table->field[MYSQL_PASSWORD_HISTORY_FIELD_PASSWORD]->set_notnull();

    if (0 != (error = table->file->ha_write_row(table->record[0]))) {
      table->file->print_error(error, MYF(0));
      result = true;
    }
  }
end:
  if (table->file->inited != handler::NONE) {
    int rc_end = table->file->ha_index_end();

    if (rc_end) {
      /* purecov: begin inspected */
      table->file->print_error(rc_end, MYF(ME_ERRORLOG));
      DBUG_ASSERT(false);
      /* purecov: end */
    }
  }
  return result;
}

/**
  Updates the password history table for cases of deleting or renaming users

  This function, unline the other "update" functions does not handle the
  addition of new data. That's done by auth_verify_password_history().
  The function only handles renames and deletes of user accounts.
  It does not go via the normal non-mysql.user handle_grant_data() route
  since there is a (partial) key on user/host and hence no need to do a
  full table scan.

  @param thd the execution context
  @param tables the list of opened ACL tables
  @param drop true if it's a drop operation
  @param user_from the user to rename from or the user to drop
  @param user_to the user to rename to or the user to add
  @param[out] row_existed set to true if row matching user_from existed
  @retval true operation failed
  @retval false success
*/

static bool handle_password_history_table(THD *thd, TABLE_LIST *tables,
                                          bool drop, LEX_USER *user_from,
                                          LEX_USER *user_to,
                                          bool *row_existed) {
  bool result = false;
  TABLE *table = tables[ACL_TABLES::TABLE_PASSWORD_HISTORY].table;
  uchar user_key[MAX_KEY_LENGTH];
  uint key_prefix_length;
  int error;
  Field *user_field, *host_field;

  Acl_table_intact table_intact(thd);

  *row_existed = false;

  if (!table) {
    /* table not preset is considered empty if not adding to it */
    return false;
  }

  if (table_intact.check(table, ACL_TABLES::TABLE_PASSWORD_HISTORY)) {
    result = true;
    goto end;
  }

  user_field = table->field[MYSQL_PASSWORD_HISTORY_FIELD_USER];
  host_field = table->field[MYSQL_PASSWORD_HISTORY_FIELD_HOST];

  table->use_all_columns();

  /* create the search key on user and host */
  user_field->store(user_from->user.str, user_from->user.length,
                    system_charset_info);
  host_field->store(user_from->host.str, user_from->host.length,
                    system_charset_info);
  key_prefix_length = (table->key_info->key_part[0].store_length +
                       table->key_info->key_part[1].store_length);
  key_copy(user_key, table->record[0], table->key_info, key_prefix_length);

  int rc;

  rc = table->file->ha_index_init(0, true);

  if (rc) {
    table->file->print_error(rc, MYF(0));
    result = true;
    goto end;
  }

  /* find the first matching record by host/user key prefix */
  error = table->file->ha_index_read_idx_map(
      table->record[0], 0, user_key, (key_part_map)((1L << 0) | (1L << 1)),
      HA_READ_KEY_EXACT);

  /* iterate over the password history rows for the user */
  while (!error) {
    /* found at least 1 row */
    if (!*row_existed) {
      *row_existed = true;
      /* no need to look for more rows if not updating */
      if (!drop && !user_to) {
        /* mark the cursor as being at end */
        error = HA_ERR_KEY_NOT_FOUND;
        break;
      }
    }

    if (drop) {
      /* if we're dropping, delete the row */
      if (0 != (error = table->file->ha_delete_row(table->record[0]))) break;
    } else if (user_to) {
      /* we're renaming, set the new user/host values */
      store_record(table, record[1]);
      table->field[MYSQL_PASSWORD_HISTORY_FIELD_USER]->store(
          user_to->user.str, user_to->user.length, system_charset_info);
      table->field[MYSQL_PASSWORD_HISTORY_FIELD_HOST]->store(
          user_to->host.str, user_to->host.length, system_charset_info);
      error = table->file->ha_update_row(table->record[1], table->record[0]);
      if (error) break;
    }
    error = table->file->ha_index_next_same(table->record[0], user_key,
                                            key_prefix_length);
  }
  /* something went wrong reading */
  if (error != HA_ERR_KEY_NOT_FOUND && error != HA_ERR_END_OF_FILE) {
    table->file->print_error(error, MYF(0));
    result = true;
  }

end:
  if (table->file->inited != handler::NONE) {
    int rc_end = table->file->ha_index_end();

    if (rc_end) {
      /* purecov: begin inspected */
      table->file->print_error(rc_end, MYF(ME_ERRORLOG));
      DBUG_ASSERT(false);
      /* purecov: end */
    }
  }
  return result;
}

/**
  Checks, if the REPLACE clause is required, optional or not required.
  It throws error:
  If REPLACE clause is required but not specified.
  If REPLACE clause is not required but specified.
  If current password specified in the REPLACE clause does not match with
  authentication string of the user.

  The plaintext current password is erased from LEX_USER, iff its length > 0 .

  @param thd      The execution context
  @param Str      LEX user
  @param acl_user The associated user which carries the ACL
  @param auth     Auth plugin to use for verification
  @param is_privileged_user     Whether caller has CREATE_USER_ACL
                                or UPDATE_ACL over mysql.*
  @param user_exists  Whether user already exists

  @retval true operation failed
  @retval false success
*/
static bool validate_password_require_current(THD *thd, LEX_USER *Str,
                                              ACL_USER *acl_user,
                                              st_mysql_auth *auth,
                                              bool is_privileged_user,
                                              bool user_exists) {
  if (user_exists) {
    if (Str->uses_replace_clause) {
      int is_error = 0;
      Security_context *sctx = thd->security_context();
      DBUG_ASSERT(sctx);
      // If trying to set password for other user
      if (strcmp(sctx->user().str, Str->user.str) ||
          my_strcasecmp(system_charset_info, sctx->priv_host().str,
                        Str->host.str)) {
        my_error(ER_CURRENT_PASSWORD_NOT_REQUIRED, MYF(0));
        return (true);
      }

      /*
        Handle the validation of empty current password first as some of
        authenication plugins do not like to check the empty passwords.
      */
      if (acl_user->credentials[PRIMARY_CRED].m_auth_string.length == 0) {
        if (Str->current_auth.length > 0) {
          my_error(ER_INCORRECT_CURRENT_PASSWORD, MYF(0));
          return (true);
        } else {
          return (false);
        }
      }
      /*
        Compare the specified plain text current password with the
        current auth string.
      */
      else if ((auth->authentication_flags & AUTH_FLAG_USES_INTERNAL_STORAGE) &&
               auth->compare_password_with_hash &&
               auth->compare_password_with_hash(
                   acl_user->credentials[PRIMARY_CRED].m_auth_string.str,
                   (unsigned long)acl_user->credentials[PRIMARY_CRED]
                       .m_auth_string.length,
                   Str->current_auth.str,
                   (unsigned long)Str->current_auth.length, &is_error) &&
               !is_error) {
        my_error(ER_INCORRECT_CURRENT_PASSWORD, MYF(0));
        return (true);
      }

      /*
        Current password is valid plain text password with len > 0.
        Erase that in memory. We don't need it any further
     */
      memset(const_cast<char *>(Str->current_auth.str), 0,
             Str->current_auth.length);
    } else if (!is_privileged_user) {
      /*
        If the field value is set or field value is NULL and global sys
        variable flag is ON then REPLACE clause must be specified.
      */
      if ((acl_user->password_require_current == Lex_acl_attrib_udyn::YES) ||
          (acl_user->password_require_current == Lex_acl_attrib_udyn::DEFAULT &&
           password_require_current)) {
        my_error(ER_MISSING_CURRENT_PASSWORD, MYF(0));
        return (true);
      }
    }
  }
  return (false);
}

char translate_byte_to_password_char(unsigned char c) {
  static const std::string translation = std::string(
      "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXY"
      "Z,.-;:_+*!%&/(){}[]<>@");
  int index = round(((float)c * ((float)(translation.length() - 1) / 255.0)));
  return translation[index];
}

/**
  Generates a random password of the length decided by the system variable
  generated_random_password_length.

  @param[out] password The generated password.
  @param length The length of the generated password.

*/

void generate_random_password(std::string *password, uint32_t length) {
  unsigned char buffer[256];
  if (length > 255) length = 255;
  RAND_bytes((unsigned char *)&buffer[0], length);
  password->reserve(length + 1);
  for (uint32_t i = 0; i < length; ++i) {
    password->append(1, translate_byte_to_password_char(buffer[i]));
  }
}

/**
  Sends the result set of generated passwords to the client.
  @param thd The thread handler
  @param generated_passwords A list of 3-tuple strings containing user, host
    and plaintext password.
  @return success state
    @retval true An error occurred (DA is set)
    @retval false Success (my_eof)
*/

bool send_password_result_set(
    THD *thd, const Userhostpassword_list &generated_passwords) {
  List<Item> meta_data;
  meta_data.push_back(new Item_string("user", 4, system_charset_info));
  meta_data.push_back(new Item_string("host", 4, system_charset_info));
  meta_data.push_back(
      new Item_string("generated password", 18, system_charset_info));
  List<Item> item_list;
  Query_result_send output;
  if (output.send_result_set_metadata(
          thd, meta_data, Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF))
    return true;
  for (auto password : generated_passwords) {
    Item *item = new Item_string(password[0].c_str(), password[0].length(),
                                 system_charset_info);
    item_list.push_back(item);
    item = new Item_string(password[1].c_str(), password[1].length(),
                           system_charset_info);
    item_list.push_back(item);
    item = new Item_string(password[2].c_str(), password[2].length(),
                           system_charset_info);
    item_list.push_back(item);
    if (output.send_data(thd, item_list)) {
      item_list.empty();
      return true;
    }
    // items clean themselves up when THD dies.
    item_list.empty();
  }
  my_eof(thd);
  return false;
}

/**
  This function does following:
   1. Convert plain text password to hash and update the same in
      user definition.
   2. Validate hash string if specified in user definition.
   3. Identify what all fields needs to be updated in mysql.user
      table based on user definition.

  If the is_role flag is set, the password validation is not used.

  The function perform some semantic parsing of the original statement
  by investigating the syntactic elements found in the LEX_USER object
  not-so-appropriately named Str.

  The code fits the purpose as a helper function to mysql_create_user()
  but it is used from mysql_alter_user(), mysql_grant(), change_password() and
  mysql_routine_grant() with a slightly varying semantic meaning.

  @param thd          Thread context
  @param Str          user on which attributes has to be applied
  @param what_to_set  User attributes
  @param is_privileged_user     Whether caller has CREATE_USER_ACL
                                or UPDATE_ACL over mysql.*
  @param is_role      CREATE ROLE was used to create the authid.
  @param history_table          The table to verify history against.
  @param[out] history_check_done  Set to on if the history table is updated
  @param cmd          Command information
  @param[out] generated_passwords A list of generated random passwords. Depends
  on LEX_USER.

  @retval 0 ok
  @retval 1 ERROR;
*/

bool set_and_validate_user_attributes(
    THD *thd, LEX_USER *Str, acl_table::Pod_user_what_to_update &what_to_set,
    bool is_privileged_user, bool is_role, TABLE_LIST *history_table,
    bool *history_check_done, const char *cmd,
    Userhostpassword_list &generated_passwords) {
  bool user_exists = false;
  ACL_USER *acl_user;
  plugin_ref plugin = nullptr;
  char outbuf[MAX_FIELD_WIDTH] = {0};
  unsigned int buflen = MAX_FIELD_WIDTH, inbuflen;
  const char *inbuf;
  const char *password = nullptr;
  enum_sql_command command = thd->lex->sql_command;
  bool current_password_empty = false;
  bool new_password_empty = false;

  what_to_set.m_what = NONE_ATTR;
  what_to_set.m_user_attributes = acl_table::USER_ATTRIBUTE_NONE;
  DBUG_ASSERT(assert_acl_cache_read_lock(thd) ||
              assert_acl_cache_write_lock(thd));

  if (history_check_done) *history_check_done = false;
  /* update plugin,auth str attributes */
  if (Str->uses_identified_by_clause || Str->uses_identified_with_clause ||
      Str->uses_authentication_string_clause)
    what_to_set.m_what |= PLUGIN_ATTR;
  else
    what_to_set.m_what |= DEFAULT_AUTH_ATTR;

  /* update ssl attributes */
  if (thd->lex->ssl_type != SSL_TYPE_NOT_SPECIFIED)
    what_to_set.m_what |= SSL_ATTR;
  /* update connection attributes */
  if (thd->lex->mqh.specified_limits) what_to_set.m_what |= RESOURCE_ATTR;

  if ((acl_user = find_acl_user(Str->host.str, Str->user.str, true)))
    user_exists = true;

  /* copy password expire attributes to individual user */
  Str->alter_status = thd->lex->alter_password;

  mysql_mutex_lock(&LOCK_password_history);
  Str->alter_status.password_history_length =
      Str->alter_status.use_default_password_history
          ? global_password_history
          : Str->alter_status.password_history_length;
  mysql_mutex_unlock(&LOCK_password_history);
  mysql_mutex_lock(&LOCK_password_reuse_interval);
  Str->alter_status.password_reuse_interval =
      Str->alter_status.use_default_password_reuse_interval
          ? global_password_reuse_interval
          : Str->alter_status.password_reuse_interval;
  mysql_mutex_unlock(&LOCK_password_reuse_interval);

  /* update password expire attributes */
  if (Str->alter_status.update_password_expired_column ||
      !Str->alter_status.use_default_password_lifetime ||
      Str->alter_status.expire_after_days)
    what_to_set.m_what |= PASSWORD_EXPIRE_ATTR;

  /* update account lock attribute */
  if (Str->alter_status.update_account_locked_column)
    what_to_set.m_what |= ACCOUNT_LOCK_ATTR;

  if (Str->plugin.length) optimize_plugin_compare_by_pointer(&Str->plugin);

  if (user_exists) {
    switch (command) {
      case SQLCOM_CREATE_USER: {
        /*
          Since user exists, we are likely going to fail
          unless IF NOT EXISTS is specified. In that case
          we need to use default plugin to generate password
          so that binlog entry is correct.
        */
        if (!Str->uses_identified_with_clause)
          Str->plugin = default_auth_plugin_name;
        break;
      }
      case SQLCOM_ALTER_USER: {
        if (!Str->uses_identified_with_clause) {
          /* If no plugin is given, get existing plugin */
          Str->plugin = acl_user->plugin;
        } else if (!(Str->uses_identified_by_clause ||
                     Str->uses_authentication_string_clause) &&
                   auth_plugin_supports_expiration(Str->plugin.str)) {
          /*
            This is an attempt to change existing users authentication plugin
            without specifying any password. In such cases, expire user's
            password so we can force password change on next login
          */
          Str->alter_status.update_password_expired_column = true;
          what_to_set.m_what |= PASSWORD_EXPIRE_ATTR;
        }
        /*
          always check for password expire/interval attributes as there is no
          way to differentiate NEVER EXPIRE and EXPIRE DEFAULT scenario
        */
        if (Str->alter_status.update_password_expired_fields)
          what_to_set.m_what |= PASSWORD_EXPIRE_ATTR;

        /* detect changes in the plugin name */
        if (Str->plugin.str != acl_user->plugin.str) {
          what_to_set.m_what |= DIFFERENT_PLUGIN_ATTR;
          if (Str->retain_current_password) {
            my_error(ER_PASSWORD_CANNOT_BE_RETAINED_ON_PLUGIN_CHANGE, MYF(0),
                     Str->user.str, Str->host.str);
            what_to_set.m_what = NONE_ATTR;
            return true;
          }
          if (acl_user->credentials[SECOND_CRED].m_auth_string.length) {
            what_to_set.m_what |= USER_ATTRIBUTES;
            what_to_set.m_user_attributes |=
                acl_table::USER_ATTRIBUTE_DISCARD_PASSWORD;
          }
        }

        if (Str->retain_current_password || Str->discard_old_password) {
          DBUG_ASSERT(
              !(Str->retain_current_password && Str->discard_old_password));
          what_to_set.m_what |= USER_ATTRIBUTES;
          if (Str->retain_current_password)
            what_to_set.m_user_attributes |=
                acl_table::USER_ATTRIBUTE_RETAIN_PASSWORD;
          if (Str->discard_old_password)
            what_to_set.m_user_attributes |=
                acl_table::USER_ATTRIBUTE_DISCARD_PASSWORD;
          current_password_empty =
              acl_user->credentials[PRIMARY_CRED].m_auth_string.length ? false
                                                                       : true;
        }

        break;
      }
      case SQLCOM_SET_PASSWORD: {
        if (Str->retain_current_password) {
          what_to_set.m_what |= USER_ATTRIBUTES;
          what_to_set.m_user_attributes |=
              acl_table::USER_ATTRIBUTE_RETAIN_PASSWORD;
          current_password_empty =
              acl_user->credentials[PRIMARY_CRED].m_auth_string.length ? false
                                                                       : true;
        }
        break;
      }
      /*
        We need to fill in the elements of the LEX_USER structure even for GRANT
        and REVOKE.
      */
      case SQLCOM_GRANT:
        /* fall through */
      case SQLCOM_REVOKE:
        what_to_set.m_what = NONE_ATTR;
        Str->plugin = acl_user->plugin;
        Str->auth.str = acl_user->credentials[PRIMARY_CRED].m_auth_string.str;
        Str->auth.length =
            acl_user->credentials[PRIMARY_CRED].m_auth_string.length;
        break;

      default: {
        /*
          If we are here, authentication related information can be provided
          only if GRANT statement was used to change user's credentials.
        */

        if (!Str->uses_identified_with_clause) {
          /* if IDENTIFIED WITH is not specified set plugin from cache */
          Str->plugin = acl_user->plugin;
          /* set auth str from cache when not specified for existing user */
          if (!(Str->uses_identified_by_clause ||
                Str->uses_authentication_string_clause)) {
            Str->auth.str =
                acl_user->credentials[PRIMARY_CRED].m_auth_string.str;
            Str->auth.length =
                acl_user->credentials[PRIMARY_CRED].m_auth_string.length;
          }
        } else if (!(Str->uses_identified_by_clause ||
                     Str->uses_authentication_string_clause) &&
                   auth_plugin_supports_expiration(Str->plugin.str)) {
          /*
            This is an attempt to change existing users authentication plugin
            without specifying any password. In such cases, expire user's
            password so we can force password change on next login
          */
          Str->alter_status.update_password_expired_column = true;
          what_to_set.m_what |= PASSWORD_EXPIRE_ATTR;
        }
      }
    };

    /* if we don't update password history take the user's password history */
    if (!Str->alter_status.update_password_history) {
      if (acl_user->use_default_password_history) {
        mysql_mutex_lock(&LOCK_password_history);
        Str->alter_status.password_history_length = global_password_history;
        mysql_mutex_unlock(&LOCK_password_history);
      } else
        Str->alter_status.password_history_length =
            acl_user->password_history_length;
    }
    /* if we don't update password reuse interval take the user's interval */
    if (!Str->alter_status.update_password_reuse_interval) {
      if (acl_user->use_default_password_reuse_interval) {
        mysql_mutex_lock(&LOCK_password_reuse_interval);
        Str->alter_status.password_reuse_interval =
            global_password_reuse_interval;
        mysql_mutex_unlock(&LOCK_password_reuse_interval);
      } else
        Str->alter_status.password_reuse_interval =
            acl_user->password_reuse_interval;
    }
  } else {
    /* set default plugin for new users if not specified */
    if (!Str->uses_identified_with_clause)
      Str->plugin = default_auth_plugin_name;

    if (command == SQLCOM_GRANT) {
      my_error(ER_CANT_CREATE_USER_WITH_GRANT, MYF(0));
      return true;
    }
  }

  optimize_plugin_compare_by_pointer(&Str->plugin);

  /*
    Check if non-default password expiraition option
    is passed to a plugin that does not support it and raise
    and error if it is.
  */
  if (Str->alter_status.update_password_expired_fields &&
      !Str->alter_status.use_default_password_lifetime &&
      Str->alter_status.expire_after_days != 0 &&
      !auth_plugin_supports_expiration(Str->plugin.str)) {
    my_error(ER_PASSWORD_EXPIRATION_NOT_SUPPORTED_BY_AUTH_METHOD, MYF(0),
             Str->plugin.length, Str->plugin.str);
    return true;
  }

  plugin =
      my_plugin_lock_by_name(nullptr, Str->plugin, MYSQL_AUTHENTICATION_PLUGIN);

  /* check if plugin is loaded */
  if (!plugin) {
    what_to_set.m_what = NONE_ATTR;
    my_error(ER_PLUGIN_IS_NOT_LOADED, MYF(0), Str->plugin.str);
    return (true);
  }

  st_mysql_auth *auth = (st_mysql_auth *)plugin_decl(plugin)->info;

  if (user_exists && (what_to_set.m_what & PLUGIN_ATTR)) {
    if (auth->authentication_flags &
        AUTH_FLAG_PRIVILEGED_USER_FOR_PASSWORD_CHANGE) {
      if (!is_privileged_user &&
          (command == SQLCOM_ALTER_USER || command == SQLCOM_GRANT)) {
        /*
          An external plugin that prevents user
          to change authentication_string information
          unless user is privileged.
        */
        what_to_set.m_what = NONE_ATTR;
        my_error(ER_ACCESS_DENIED_ERROR, MYF(0),
                 thd->security_context()->priv_user().str,
                 thd->security_context()->priv_host().str,
                 thd->password ? ER_THD(thd, ER_YES) : ER_THD(thd, ER_NO));
        plugin_unlock(nullptr, plugin);
        return (true);
      }
    }

    if (!(auth->authentication_flags & AUTH_FLAG_USES_INTERNAL_STORAGE)) {
      if (command == SQLCOM_SET_PASSWORD) {
        /*
          A plugin that does not use internal storage and
          hence does not support SET PASSWORD
        */
        char warning_buffer[MYSQL_ERRMSG_SIZE];
        snprintf(warning_buffer, sizeof(warning_buffer),
                 "SET PASSWORD has no significance for user '%s'@'%s' as "
                 "authentication plugin does not support it.",
                 Str->user.str, Str->host.str);
        warning_buffer[MYSQL_ERRMSG_SIZE - 1] = '\0';
        push_warning(thd, Sql_condition::SL_NOTE, ER_SET_PASSWORD_AUTH_PLUGIN,
                     warning_buffer);
        plugin_unlock(nullptr, plugin);
        what_to_set.m_what = NONE_ATTR;
        return (false);
      }
    }
  }

  if (!(auth->authentication_flags & AUTH_FLAG_USES_INTERNAL_STORAGE)) {
    if (Str->alter_status.password_history_length ||
        Str->alter_status.password_reuse_interval) {
      /*
        A plugin that does not use internal storage and
        hence does not support password history is passed a password history
      */
      if (Str->alter_status.update_password_history ||
          Str->alter_status.update_password_reuse_interval)
        push_warning_printf(
            thd, Sql_condition::SL_WARNING,
            ER_WARNING_PASSWORD_HISTORY_CLAUSES_VOID,
            ER_THD(thd, ER_WARNING_PASSWORD_HISTORY_CLAUSES_VOID),
            Str->user.str, Str->host.str, plugin_decl(plugin)->name);
      /* reset back the password history clauses for that user */
      Str->alter_status.password_history_length = 0;
      Str->alter_status.password_reuse_interval = 0;
      Str->alter_status.update_password_history = true;
      Str->alter_status.update_password_reuse_interval = true;
      Str->alter_status.use_default_password_history = true;
      Str->alter_status.use_default_password_reuse_interval = true;
    }
  }
  /*
    If auth string is specified, change it to hash.
    Validate empty credentials for new user ex: CREATE USER u1;
    We skip authentication string generation if the issued statement was
    CREATE ROLE.
  */
  if (!is_role && (Str->uses_identified_by_clause ||
                   (Str->auth.length == 0 && !user_exists))) {
    inbuf = Str->auth.str;
    inbuflen = (unsigned)Str->auth.length;
    std::string gen_password;
    if (Str->has_password_generator) {
      thd->m_disable_password_validation = true;
      generate_random_password(&gen_password,
                               thd->variables.generated_random_password_length);
      inbuf = gen_password.c_str();
      inbuflen = gen_password.length();
      generated_passwords.push_back({std::string(Str->user.str),
                                     std::string(Str->host.str), gen_password});
    }
    if (auth->generate_authentication_string(outbuf, &buflen, inbuf,
                                             inbuflen) ||
        auth_verify_password_history(thd, &Str->user, &Str->host,
                                     Str->alter_status.password_history_length,
                                     Str->alter_status.password_reuse_interval,
                                     auth, inbuf, inbuflen, outbuf, buflen,
                                     history_table, what_to_set.m_what)) {
      plugin_unlock(nullptr, plugin);
      what_to_set.m_what = NONE_ATTR;
      /*
        generate_authentication_string may return error status
        without setting actual error.
      */
      if (!thd->is_error()) {
        String error_user;
        log_user(thd, &error_user, Str, false);
        my_error(ER_CANNOT_USER, MYF(0), cmd, error_user.c_ptr_safe());
      }
      return (true);
    }
    /* Allow for password validation in case it was disabled before */
    thd->m_disable_password_validation = false;
    if (history_check_done) *history_check_done = true;
    if (buflen) {
      password = strmake_root(thd->mem_root, outbuf, buflen);
    } else
      password = const_cast<char *>("");
    /*
       Erase in memory copy of plain text password, unless we need it
       later to send to client as a result set.
    */
    if (Str->auth.length > 0)
      memset(const_cast<char *>(Str->auth.str), 0, Str->auth.length);
    /* Use the authentication_string field as password */
    Str->auth.str = password;
    Str->auth.length = buflen;
    new_password_empty = Str->auth.length ? false : true;
  }

  /* Check iff the REPLACE clause is specified correctly for the user */
  if ((what_to_set.m_what & PLUGIN_ATTR) &&
      validate_password_require_current(thd, Str, acl_user, auth,
                                        is_privileged_user, user_exists)) {
    plugin_unlock(nullptr, plugin);
    what_to_set.m_what = NONE_ATTR;
    return (true);
  }

  /* Validate hash string */
  if (Str->uses_authentication_string_clause) {
    /*
      The statement CREATE ROLE calls mysql_create_user() with a set of
      lexicographic parameters: users_identified_by_password_caluse= false etc
      It also sets is_role= true. We don't have to check this parameter here
      since we're already know that the above parameters will be false
      but we place an extra assert here to remind us about the complex
      interdependencies if mysql_create_user() is refactored.
    */
    DBUG_ASSERT(!is_role);
    if (auth->validate_authentication_string(const_cast<char *>(Str->auth.str),
                                             (unsigned)Str->auth.length)) {
      my_error(ER_PASSWORD_FORMAT, MYF(0));
      plugin_unlock(nullptr, plugin);
      what_to_set.m_what = NONE_ATTR;
      return (true);
    }
    /*
      Call the password history validation so that it can store the incoming
      hash into the password history table.
      Here we can't check if the password was used since we don't have the
      cleartext password, but we still want to record it into the history table.
      Covers replication scenario too since the IDENTIFIED BY will get
      rewritten to IDENTIFIED ... WITH ... AS
    */
    if (auth_verify_password_history(thd, &Str->user, &Str->host,
                                     Str->alter_status.password_history_length,
                                     Str->alter_status.password_reuse_interval,
                                     auth, nullptr, 0, Str->auth.str,
                                     (unsigned)Str->auth.length, history_table,
                                     what_to_set.m_what)) {
      /* we should have an error generated here already */
      plugin_unlock(nullptr, plugin);
      what_to_set.m_what = NONE_ATTR;
      return (true);
    }
    if (history_check_done) *history_check_done = true;
  }

  if (user_exists && (what_to_set.m_user_attributes &
                      (acl_table::USER_ATTRIBUTE_RETAIN_PASSWORD |
                       acl_table::USER_ATTRIBUTE_DISCARD_PASSWORD))) {
    if (!(auth->authentication_flags & AUTH_FLAG_USES_INTERNAL_STORAGE)) {
      /* We do not support multiple passwords */
      if (Str->retain_current_password) {
        push_warning_printf(
            thd, Sql_condition::SL_WARNING,
            ER_WARNING_RETAIN_CURRENT_PASSWORD_CLAUSE_VOID,
            ER_THD(thd, ER_WARNING_RETAIN_CURRENT_PASSWORD_CLAUSE_VOID),
            Str->user.str, Str->host.str, plugin_decl(plugin)->name);
        what_to_set.m_user_attributes &=
            ~acl_table::USER_ATTRIBUTE_RETAIN_PASSWORD;
      }

      if (Str->discard_old_password) {
        push_warning_printf(
            thd, Sql_condition::SL_WARNING,
            ER_WARNING_DISCARD_OLD_PASSWORD_CLAUSE_VOID,
            ER_THD(thd, ER_WARNING_DISCARD_OLD_PASSWORD_CLAUSE_VOID),
            Str->user.str, Str->host.str, plugin_decl(plugin)->name);
        what_to_set.m_user_attributes &=
            ~acl_table::USER_ATTRIBUTE_DISCARD_PASSWORD;
      }
    } else if (what_to_set.m_user_attributes &
               acl_table::USER_ATTRIBUTE_RETAIN_PASSWORD) {
      if (current_password_empty) {
        my_error(ER_SECOND_PASSWORD_CANNOT_BE_EMPTY, MYF(0), Str->user.str,
                 Str->host.str);
        plugin_unlock(nullptr, plugin);
        what_to_set.m_what = NONE_ATTR;
        return true;
      }

      if (what_to_set.m_what & PLUGIN_ATTR && new_password_empty) {
        my_error(ER_CURRENT_PASSWORD_CANNOT_BE_RETAINED, MYF(0), Str->user.str,
                 Str->host.str);
        plugin_unlock(nullptr, plugin);
        what_to_set.m_what = NONE_ATTR;
        return true;
      }
    }
  }

  if (user_exists && (what_to_set.m_what & PLUGIN_ATTR) && !Str->auth.length &&
      (auth->authentication_flags & AUTH_FLAG_USES_INTERNAL_STORAGE)) {
    if (acl_user->credentials[SECOND_CRED].m_auth_string.length) {
      what_to_set.m_what |= USER_ATTRIBUTES;
      what_to_set.m_user_attributes |=
          acl_table::USER_ATTRIBUTE_DISCARD_PASSWORD;
    }
  }

  if (Str->alter_status.update_failed_login_attempts) {
    what_to_set.m_what |= USER_ATTRIBUTES;
    what_to_set.m_user_attributes |=
        acl_table::USER_ATTRIBUTE_FAILED_LOGIN_ATTEMPTS;
  }
  if (Str->alter_status.update_password_lock_time) {
    what_to_set.m_what |= USER_ATTRIBUTES;
    what_to_set.m_user_attributes |=
        acl_table::USER_ATTRIBUTE_PASSWORD_LOCK_TIME;
  }
  plugin_unlock(nullptr, plugin);
  return (false);
}

/**
  Change a password hash for a user.

  @param thd Thread handle
  @param lex_user LEX_USER
  @param new_password New password hash for host\@user
  @param current_password Current password for host\@user
  @param retain_current_password Preference to retain current password

  Note : it will also reset the change_password flag.
  This is safe to do unconditionally since the simple userless form
  SET PASSWORD = 'text' will be the only allowed form when
  this flag is on. So we don't need to check user names here.

  @see set_var_password::update(THD *thd)

  @return Error code
   @retval 0 ok
   @retval 1 ERROR; In this case the error is sent to the client.
*/

bool change_password(THD *thd, LEX_USER *lex_user, const char *new_password,
                     const char *current_password,
                     bool retain_current_password) {
  TABLE_LIST tables[ACL_TABLES::LAST_ENTRY];
  TABLE *table;
  LEX_USER *combo = nullptr;
  std::set<LEX_USER *> users;
  acl_table::Pod_user_what_to_update what_to_set;
  size_t new_password_len = strlen(new_password);
  bool transactional_tables;
  bool result = false;
  bool commit_result = false;
  std::string authentication_plugin;
  bool is_role;
  int ret;
  sql_mode_t old_sql_mode = thd->variables.sql_mode;

  DBUG_TRACE;
  DBUG_ASSERT(lex_user && lex_user->host.str);
  DBUG_PRINT("enter", ("host: '%s'  user: '%s' current_password: '%s' \
                       new_password: '%s'",
                       lex_user->host.str, lex_user->user.str, current_password,
                       new_password));

  if (check_change_password(thd, lex_user->host.str, lex_user->user.str,
                            retain_current_password))
    return true;

  Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::WRITE_MODE);
  /*
    This statement will be replicated as a statement, even when using
    row-based replication.  The binlog state will be cleared here to
    statement based replication and will be reset to the originals
    values when we are out of this function scope
  */
  Save_and_Restore_binlog_format_state binlog_format_state(thd);

  if ((ret = open_grant_tables(thd, tables, &transactional_tables)))
    return ret != 1;

  { /* Critical section */

    if (!acl_cache_lock.lock()) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    table = tables[ACL_TABLES::TABLE_USER].table;

    ACL_USER *acl_user =
        find_acl_user(lex_user->host.str, lex_user->user.str, true);
    if (acl_user == nullptr) {
      my_error(ER_PASSWORD_NO_MATCH, MYF(0));
      commit_and_close_mysql_tables(thd);
      return true;
    }

    DBUG_ASSERT(acl_user->plugin.length != 0);
    is_role = acl_user->is_role;

    if (!(combo = (LEX_USER *)thd->alloc(sizeof(LEX_USER)))) return true;

    combo->user.str = lex_user->user.str;
    combo->host.str = lex_user->host.str;
    combo->user.length = lex_user->user.length;
    combo->host.length = lex_user->host.length;
    combo->plugin.str = acl_user->plugin.str;
    combo->plugin.length = acl_user->plugin.length;

    lex_string_strmake(thd->mem_root, &combo->user, combo->user.str,
                       strlen(combo->user.str));
    lex_string_strmake(thd->mem_root, &combo->host, combo->host.str,
                       strlen(combo->host.str));
    lex_string_strmake(thd->mem_root, &combo->plugin, combo->plugin.str,
                       strlen(combo->plugin.str));
    optimize_plugin_compare_by_pointer(&combo->plugin);
    combo->auth.str = new_password;
    combo->auth.length = new_password_len;
    combo->current_auth.str = current_password;
    combo->current_auth.length =
        (current_password) ? strlen(current_password) : 0;
    combo->uses_identified_by_clause = true;
    combo->uses_identified_with_clause = false;
    combo->uses_authentication_string_clause = false;
    combo->uses_replace_clause = (current_password) ? true : false;
    combo->retain_current_password = retain_current_password;
    combo->discard_old_password = false;
    combo->has_password_generator = false;

    /* set default values */
    thd->lex->ssl_type = SSL_TYPE_NOT_SPECIFIED;
    memset(&(thd->lex->mqh), 0, sizeof(thd->lex->mqh));
    thd->lex->alter_password.cleanup();

    bool is_privileged_user = is_privileged_user_for_credential_change(thd);
    /*
      Change_password() only sets the password for one user at a time and
      it does not support the generation of random passwords. Instead it's
      called from set_var_password which might have generated the password.
      Since we're falling back on code used by mysql_create_user() we still
      supply a list for storing generated password, although password
      generation never will happen at this stage.
      Calling this function has the side effect that combo->auth is rewritten
      into a hash.
    */
    Userhostpassword_list dummy;
    if (set_and_validate_user_attributes(
            thd, combo, what_to_set, is_privileged_user, false,
            &tables[ACL_TABLES::TABLE_PASSWORD_HISTORY], nullptr,
            "SET PASSWORD", dummy)) {
      authentication_plugin.assign(combo->plugin.str);
      result = true;
      goto end;
    }

    // We must not have user with plain text password at this point
    // unless the password was randomly generated in which case the
    // plain text password will live on in the calling function for the
    // purpose of returning it to the client.
    thd->lex->contains_plaintext_password = false;
    authentication_plugin.assign(combo->plugin.str);
    thd->variables.sql_mode &= ~MODE_PAD_CHAR_TO_FULL_LENGTH;
    ret = replace_user_table(thd, table, combo, 0, false, false, what_to_set);
    thd->variables.sql_mode = old_sql_mode;

    if (ret) {
      result = true;
      goto end;
    }
    if (!update_sctx_cache(thd->security_context(), acl_user, false) &&
        thd->security_context()->password_expired()) {
      /* the current user is not the same as the user we operate on */
      my_error(ER_MUST_CHANGE_PASSWORD, MYF(0));
      result = true;
      goto end;
    }

    result = false;
    users.insert(combo);

  end:

    User_params user_params(&users);
    commit_result = log_and_commit_acl_ddl(thd, transactional_tables, &users,
                                           &user_params, false, !result);

    mysql_audit_notify(
        thd, AUDIT_EVENT(MYSQL_AUDIT_AUTHENTICATION_CREDENTIAL_CHANGE),
        thd->is_error() || result, lex_user->user.str, lex_user->host.str,
        authentication_plugin.c_str(), is_role, nullptr, nullptr);
  } /* Critical section */

  /* Notify storage engines (including rewrite list) */
  if (!(result || commit_result)) {
    List<LEX_USER> user_list;
    user_list.push_back(lex_user);
    acl_notify_htons(thd, SQLCOM_SET_PASSWORD, &user_list, &users);
  }

  return result || commit_result;
}

namespace {

// No need for a return value; the matcher sets its own state.
template <class T, class Matcher>
void search_for_matching_grant(const T *hash, Matcher &matches) {
  for (auto it = hash->begin(); it != hash->end(); ++it) {
    const GRANT_NAME *grant_name = it->second.get();
    if (matches(grant_name->user, grant_name->host.get_host())) return;
  }
}

template <class T, class Matcher>
void remove_matching_grants(T *hash, Matcher &matches) {
  for (auto it = hash->begin(); it != hash->end();) {
    const GRANT_NAME *grant_name = it->second.get();
    if (matches(grant_name->user, grant_name->host.get_host()))
      it = hash->erase(it);
    else
      ++it;
  }
}

template <class T, class Matcher>
bool rename_matching_grants(T *hash, Matcher &matches, LEX_USER *user_to) {
  DBUG_TRACE;

  using Elem = typename T::value_type::second_type::element_type;

  /*
    Inserting while traversing a hash table is not valid procedure and
    hence we save pointers to GRANT_NAME objects for later processing.

    Prealloced_array can't hold unique_ptr, so we'll need to take them
    in and out here.
  */
  Prealloced_array<Elem *, 16> acl_grant_name(PSI_INSTRUMENT_ME);
  for (auto it = hash->begin(); it != hash->end();) {
    Elem *grant_name = it->second.get();
    if (matches(grant_name->user, grant_name->host.get_host())) {
      if (acl_grant_name.push_back(it->second.release())) return true;
      it = hash->erase(it);
    } else
      ++it;
  }

  /*
    Update the grant structures with the new user name and host name,
    then insert them back.
  */
  for (Elem *grant_name : acl_grant_name) {
    grant_name->set_user_details(user_to->host.str, grant_name->db,
                                 user_to->user.str, grant_name->tname, true);
    hash->emplace(grant_name->hash_key,
                  unique_ptr_destroy_only<Elem>(grant_name));
  }
  return false;
}

}  // namespace

/**
  Handle an in-memory privilege structure.

  @param struct_no  The number of the structure to handle (0..5).
  @param drop       If user_from is to be dropped.
  @param user_from  The the user to be searched/dropped/renamed.
  @param user_to    The new name for the user if to be renamed, NULL otherwise.
  @param on_drop_role_priv  true enabled by the DROP ROLE privilege

  @note
    Scan through all elements in an in-memory grant structure and apply
    the requested operation.
    Delete from grant structure if drop is true.
    Update in grant structure if drop is false and user_to is not NULL.
    Search in grant structure if drop is false and user_to is NULL.
    Structures are enumerated as follows:
    0 ACL_USER
    1 ACL_DB
    2 COLUMN_PRIVILIGES_HASH
    3 PROC_PRIVILEGES_HASH
    4 FUNC_PRIVILEGES_HASH
    5 ACL_PROXY_USERS

  @retval > 0  At least one element matched.
  @retval 0    OK, but no element matched.
  @retval -1   Wrong arguments to function or Out of Memory.
*/

static int handle_grant_struct(enum enum_acl_lists struct_no, bool drop,
                               LEX_USER *user_from, LEX_USER *user_to,
                               bool on_drop_role_priv) {
  DBUG_TRACE;
  DBUG_PRINT("info", ("scan struct: %u  search: '%s'@'%s'", struct_no,
                      user_from->user.str, user_from->host.str));

  int result = 0;
  auto matches = [user_from, &result](const char *user, const char *host) {
    if (!user) user = "";
    if (!host) host = "";
    bool match =
        strcmp(user_from->user.str, user) == 0 &&
        my_strcasecmp(system_charset_info, user_from->host.str, host) == 0;
    if (match) result = 1;
    return match;
  };

  DBUG_ASSERT(assert_acl_cache_write_lock(current_thd));

  switch (struct_no) {
    case USER_ACL:
      for (uint idx = 0; idx < acl_users->size(); idx++) {
        ACL_USER *acl_user = &acl_users->at(idx);
        if (!matches(acl_user->user, acl_user->host.get_host())) continue;

        if (drop) {
          // if we're dropping roles and the account is not locked (not a role)
          // bail off
          if (on_drop_role_priv && !acl_user->account_locked) {
            char command[128];
            get_privilege_desc(command, sizeof(command), CREATE_USER_ACL);
            my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0), command);
            return -1;
          }
          acl_restrictions->remove_restrictions(acl_user);
          acl_users->erase(idx);
          rebuild_cached_acl_users_for_name();
          /*
             - If we are iterating through an array then we just have moved all
             elements after the current element one position closer to its head.
             This means that we have to take another look at the element at
             current position as it is a new element from the array's tail.
             - This is valid for case USER_ACL, DB_ACL and PROXY_USERS_ACL.
           */
          idx--;
        } else if (user_to) {
          acl_user->user = strdup_root(&global_acl_memory, user_to->user.str);
          acl_user->host.update_hostname(
              strdup_root(&global_acl_memory, user_to->host.str));
          rebuild_cached_acl_users_for_name();
        } else {
          /* If search is requested, we do not need to search further. */
          break;
        }
      }
      break;

    case DB_ACL:
      for (ACL_DB *acl_db = acl_dbs->begin(); acl_db < acl_dbs->end();) {
        if (!matches(acl_db->user, acl_db->host.get_host())) {
          acl_db++;
          continue;
        }

        if (drop) {
          acl_db = acl_remove_db(acl_db);
        } else if (user_to) {
          acl_db->user = strdup_root(&global_acl_memory, user_to->user.str);
          acl_db->host.update_hostname(
              strdup_root(&global_acl_memory, user_to->host.str));
          acl_db++;
        } else {
          /* If search is requested, we do not need to search further. */
          break;
        }
      }
      break;

    case COLUMN_PRIVILEGES_HASH:
      if (drop)
        remove_matching_grants(column_priv_hash.get(), matches);
      else if (user_to) {
        if (rename_matching_grants(column_priv_hash.get(), matches, user_to))
          return -1;
      } else
        search_for_matching_grant(column_priv_hash.get(), matches);
      break;

    case PROC_PRIVILEGES_HASH:
      if (drop)
        remove_matching_grants(proc_priv_hash.get(), matches);
      else if (user_to) {
        if (rename_matching_grants(proc_priv_hash.get(), matches, user_to))
          return -1;
      } else
        search_for_matching_grant(proc_priv_hash.get(), matches);
      break;

    case FUNC_PRIVILEGES_HASH:
      if (drop)
        remove_matching_grants(func_priv_hash.get(), matches);
      else if (user_to) {
        if (rename_matching_grants(func_priv_hash.get(), matches, user_to))
          return -1;
      } else
        search_for_matching_grant(func_priv_hash.get(), matches);
      break;

    case PROXY_USERS_ACL:
      for (uint idx = 0; idx < acl_proxy_users->size(); idx++) {
        ACL_PROXY_USER *acl_proxy_user = &acl_proxy_users->at(idx);
        if (!matches(acl_proxy_user->get_user(),
                     acl_proxy_user->host.get_host()))
          continue;

        if (drop) {
          acl_proxy_users->erase(idx);
          idx--;
        } else if (user_to) {
          acl_proxy_user->set_user(&global_acl_memory, user_to->user.str);
          acl_proxy_user->host.update_hostname(
              (user_to->host.str && *user_to->host.str)
                  ? strdup_root(&global_acl_memory, user_to->host.str)
                  : nullptr);
        } else {
          /* If search is requested, we do not need to search further. */
          break;
        }
      }
      break;

    default:
      return -1;
  }

#ifdef EXTRA_DEBUG
  DBUG_PRINT("loop", ("scan struct: %u  result %d", struct_no, result));
#endif

  return result;
}

/**
  Handle all privilege tables and in-memory privilege structures.
    @param  thd                 Thread handle
    @param  tables              The array with the seven open tables.
    @param  drop                If user_from is to be dropped.
    @param  user_from           The the user to be searched/dropped/renamed.
    @param  user_to             The new name for the user if to be renamed,
                                NULL otherwise.
    @param  on_drop_role_priv   Enabled via the DROP ROLE privilege

  @note
    Go through all grant tables and in-memory grant structures and apply
    the requested operation.
    Delete from grant data if drop is true.
    Update in grant data if drop is false and user_to is not NULL.
    Search in grant data if drop is false and user_to is NULL.

  @return  operation result
    @retval > 0  At least one element matched.
    @retval 0  OK, but no element matched.
    @retval < 0  System error (OOM, error from storage engine).
*/

static int handle_grant_data(THD *thd, TABLE_LIST *tables, bool drop,
                             LEX_USER *user_from, LEX_USER *user_to,
                             bool on_drop_role_priv) {
  int result = 0;
  int found;
  int ret;
  Acl_table_intact table_intact(thd);
  DBUG_TRACE;

  if (drop) {
    /*
      Tables are defined by open_grant_tables()
      index 6 := mysql.role_edges
      index 7 := mysql.default_roles
    */
    if (revoke_all_roles_from_user(
            thd, tables[ACL_TABLES::TABLE_ROLE_EDGES].table,
            tables[ACL_TABLES::TABLE_DEFAULT_ROLES].table, user_from)) {
      return -1;
    }
    /* Remove all associated dynamic privileges on a best effort basis */
    Update_dynamic_privilege_table update_table(
        thd, tables[ACL_TABLES::TABLE_DYNAMIC_PRIV].table);
    result = revoke_all_dynamic_privileges(user_from->user, user_from->host,
                                           update_table);
  }

  /* Handle user table. */
  if ((found = handle_grant_table(thd, tables, ACL_TABLES::TABLE_USER, drop,
                                  user_from, user_to)) < 0) {
    /* Handle of table failed, don't touch the in-memory array. */
    return -1;
  } else {
    /* Handle user array. */
    if ((ret = handle_grant_struct(USER_ACL, drop, user_from, user_to,
                                   on_drop_role_priv)) > 0 ||
        found) {
      result = 1; /* At least one record/element found. */
      /* If search is requested, we do not need to search further. */
      if (!drop && !user_to) goto end;
    } else if (ret < 0) {
      result = -1;
      goto end;
    }
  }

  /* Handle db table. */
  if ((found = handle_grant_table(thd, tables, ACL_TABLES::TABLE_DB, drop,
                                  user_from, user_to)) < 0) {
    /* Handle of table failed, don't touch the in-memory array. */
    return -1;
  } else {
    /* Handle db array. */
    if ((((ret = handle_grant_struct(DB_ACL, drop, user_from, user_to,
                                     on_drop_role_priv)) > 0 &&
          !result) ||
         found) &&
        !result) {
      result = 1; /* At least one record/element found. */
      /* If search is requested, we do not need to search further. */
      if (!drop && !user_to) goto end;
    } else if (ret < 0) {
      result = -1;
      goto end;
    }
  }

  DBUG_EXECUTE_IF("mysql_handle_grant_data_fail_on_routine_table",
                  DBUG_SET("+d,wl7158_handle_grant_table_2"););

  /* Handle stored routines table. */
  if ((found = handle_grant_table(thd, tables, ACL_TABLES::TABLE_PROCS_PRIV,
                                  drop, user_from, user_to)) < 0) {
    /* Handle of table failed, don't touch in-memory array. */
    DBUG_EXECUTE_IF("mysql_handle_grant_data_fail_on_routine_table",
                    DBUG_SET("-d,wl7158_handle_grant_table_2"););
    return -1;
  } else {
    /* Handle procs array. */
    if ((((ret = handle_grant_struct(PROC_PRIVILEGES_HASH, drop, user_from,
                                     user_to, on_drop_role_priv)) > 0 &&
          !result) ||
         found) &&
        !result) {
      result = 1; /* At least one record/element found. */
      /* If search is requested, we do not need to search further. */
      if (!drop && !user_to) goto end;
    } else if (ret < 0) {
      result = -1;
      goto end;
    }
    /* Handle funcs array. */
    if ((((ret = handle_grant_struct(FUNC_PRIVILEGES_HASH, drop, user_from,
                                     user_to, on_drop_role_priv)) > 0 &&
          !result) ||
         found) &&
        !result) {
      result = 1; /* At least one record/element found. */
      /* If search is requested, we do not need to search further. */
      if (!drop && !user_to) goto end;
    } else if (ret < 0) {
      result = -1;
      goto end;
    }
  }

  DBUG_EXECUTE_IF("mysql_handle_grant_data_fail_on_tables_table",
                  DBUG_SET("+d,wl7158_handle_grant_table_2"););

  /* Handle tables table. */
  if ((found = handle_grant_table(thd, tables, ACL_TABLES::TABLE_TABLES_PRIV,
                                  drop, user_from, user_to)) < 0) {
    DBUG_EXECUTE_IF("mysql_handle_grant_data_fail_on_tables_table",
                    DBUG_SET("-d,wl7158_handle_grant_table_2"););
    /* Handle of table failed, don't touch columns and in-memory array. */
    return -1;
  } else {
    if (found && !result) {
      result = 1; /* At least one record found. */
      /* If search is requested, we do not need to search further. */
      if (!drop && !user_to) goto end;
    }

    DBUG_EXECUTE_IF("mysql_handle_grant_data_fail_on_columns_table",
                    DBUG_SET("+d,wl7158_handle_grant_table_2"););

    /* Handle columns table. */
    if ((found = handle_grant_table(thd, tables, ACL_TABLES::TABLE_COLUMNS_PRIV,
                                    drop, user_from, user_to)) < 0) {
      DBUG_EXECUTE_IF("mysql_handle_grant_data_fail_on_columns_table",
                      DBUG_SET("-d,wl7158_handle_grant_table_2"););
      /* Handle of table failed, don't touch the in-memory array. */
      return -1;
    } else {
      /* Handle columns hash. */
      if ((((ret = handle_grant_struct(COLUMN_PRIVILEGES_HASH, drop, user_from,
                                       user_to, on_drop_role_priv)) > 0 &&
            !result) ||
           found) &&
          !result)
        result = 1; /* At least one record/element found. */
      else if (ret < 0)
        result = -1;
    }
  }

  /* Handle proxies_priv table. */
  if (tables[ACL_TABLES::TABLE_PROXIES_PRIV].table) {
    DBUG_EXECUTE_IF("mysql_handle_grant_data_fail_on_proxies_priv_table",
                    DBUG_SET("+d,wl7158_handle_grant_table_2"););

    if (table_intact.check(tables[ACL_TABLES::TABLE_PROXIES_PRIV].table,
                           ACL_TABLES::TABLE_PROXIES_PRIV)) {
      result = -1;
      goto end;
    }

    if ((found = handle_grant_table(thd, tables, ACL_TABLES::TABLE_PROXIES_PRIV,
                                    drop, user_from, user_to)) < 0) {
      DBUG_EXECUTE_IF("mysql_handle_grant_data_fail_on_proxies_priv_table",
                      DBUG_SET("-d,wl7158_handle_grant_table_2"););
      /* Handle of table failed, don't touch the in-memory array. */
      return -1;
    } else {
      /* Handle proxies_priv array. */
      if (((ret = handle_grant_struct(PROXY_USERS_ACL, drop, user_from, user_to,
                                      on_drop_role_priv)) > 0 &&
           !result) ||
          found)
        result = 1; /* At least one record/element found. */
      else if (ret < 0)
        result = -1;
    }
  }

  {
    bool row_existed;
    if (handle_password_history_table(thd, tables, drop, user_from, user_to,
                                      &row_existed))
      return -1;
    else if (row_existed)
      return 1;
  }

end:
  return result;
}

/*
  Create a list of users.

  On successful completion the function emits my_ok() or my_eof().

  SYNOPSIS
    mysql_create_user()
    thd                         The current thread.
    list                        The users to create.

  RETURN
    false       OK.
    true        Error.
*/

bool mysql_create_user(THD *thd, List<LEX_USER> &list, bool if_not_exists,
                       bool is_role) {
  int result = 0;
  String wrong_users;
  LEX_USER *user_name, *tmp_user_name;
  List_iterator<LEX_USER> user_list(list);
  TABLE_LIST tables[ACL_TABLES::LAST_ENTRY];
  bool transactional_tables;
  acl_table::Pod_user_what_to_update what_to_update;
  bool is_anonymous_user = false;
  std::set<LEX_USER *> extra_users;
  std::set<LEX_USER *> reset_users;
  Userhostpassword_list generated_passwords;
  DBUG_TRACE;

  /*
    This statement will be replicated as a statement, even when using
    row-based replication.  The binlog state will be cleared here to
    statement based replication and will be reset to the originals
    values when we are out of this function scope
  */
  Save_and_Restore_binlog_format_state binlog_format_state(thd);

  /* CREATE USER may be skipped on replication client. */
  if ((result = open_grant_tables(thd, tables, &transactional_tables)))
    return result != 1;

  { /* Critical section */
    Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::WRITE_MODE);

    if (!acl_cache_lock.lock()) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    if (check_system_user_privilege(thd, list)) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    while ((tmp_user_name = user_list++)) {
      bool history_check_done = false;
      /*
        Ignore the current user as it already exists.
      */
      if (!(user_name = get_current_user(thd, tmp_user_name))) {
        result = 1;
        log_user(thd, &wrong_users, user_name, wrong_users.length() > 0);
        continue;
      }
      if (set_and_validate_user_attributes(
              thd, user_name, what_to_update, true, is_role,
              &tables[ACL_TABLES::TABLE_PASSWORD_HISTORY], &history_check_done,
              "CREATE USER", generated_passwords)) {
        result = 1;
        log_user(thd, &wrong_users, user_name, wrong_users.length() > 0);
        continue;
      }
      if (!strcmp(user_name->user.str, "") &&
          (what_to_update.m_what & PASSWORD_EXPIRE_ATTR)) {
        is_anonymous_user = true;
        result = 1;
        continue;
      }

      /*
        Search all in-memory structures and grant tables
        for a mention of the new user name.
        We may not need to process the password history here since it may have
        already been processed in set_and_validate_user_attributes().
        Hence we check the flag and temporarily set the history table
        to unopened state and then restore it back.
      */
      int ret;
      TABLE *history_tbl = tables[ACL_TABLES::TABLE_PASSWORD_HISTORY].table;
      if (history_check_done)
        tables[ACL_TABLES::TABLE_PASSWORD_HISTORY].table = nullptr;
      ret = handle_grant_data(thd, tables, false, user_name, nullptr, false);
      if (history_check_done)
        tables[ACL_TABLES::TABLE_PASSWORD_HISTORY].table = history_tbl;
      if (ret) {
        if (ret < 0) {
          result = 1;
          break;
        }
        if (if_not_exists) {
          String warn_user;
          log_user(thd, &warn_user, user_name, false);
          push_warning_printf(
              thd, Sql_condition::SL_NOTE, ER_USER_ALREADY_EXISTS,
              ER_THD(thd, ER_USER_ALREADY_EXISTS), warn_user.c_ptr_safe());
          try {
            extra_users.insert(user_name);
          } catch (...) {
            LogErr(WARNING_LEVEL,
                   ER_USER_NOT_IN_EXTRA_USERS_BINLOG_POSSIBLY_INCOMPLETE,
                   warn_user.c_ptr_safe());
          }
          continue;
        } else {
          log_user(thd, &wrong_users, user_name, wrong_users.length() > 0);
          result = 1;
        }
        continue;
      }

      ret = replace_user_table(thd, tables[ACL_TABLES::TABLE_USER].table,
                               user_name, 0, false, true, what_to_update);

      if (ret) {
        result = 1;
        if (ret < 0) break;

        log_user(thd, &wrong_users, user_name, wrong_users.length() > 0);

        continue;
      }

      /*
        Update default roles if any were specified. If roles doesn't exist we
        fail the statement. If the role exists but isn't granted, this statement
        performs an implicit GRANT.
      */
      if (thd->lex->default_roles != nullptr &&
          thd->lex->sql_command == SQLCOM_CREATE_USER) {
        List_of_auth_id_refs default_roles;
        List_iterator<LEX_USER> role_it(*(thd->lex->default_roles));
        LEX_USER *role;
        while ((role = role_it++) && result == 0) {
          if (!is_granted_role(tmp_user_name->user, tmp_user_name->host,
                               role->user, role->host)) {
            ACL_USER *acl_role =
                find_acl_user(role->host.str, role->user.str, true);
            const ACL_USER *acl_user = find_acl_user(
                tmp_user_name->host.str, tmp_user_name->user.str, true);
            if (acl_role == nullptr) {
              std::string authid = create_authid_str_from(role);
              my_error(ER_USER_DOES_NOT_EXIST, MYF(0), authid.c_str());
              result = 1;
            } else if (acl_user == nullptr) {
              std::string authid = create_authid_str_from(tmp_user_name);
              my_error(ER_USER_DOES_NOT_EXIST, MYF(0), authid.c_str());
              result = 1;
            } else if (!has_grant_role_privilege(thd, role->user, role->host)) {
              my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0),
                       "WITH ADMIN, ROLE_ADMIN, SUPER");
              result = 1;
            } else {
              DBUG_ASSERT(result == 0);
              grant_role(acl_role, acl_user, false);
              Auth_id_ref from_user = create_authid_from(role);
              Auth_id_ref to_user = create_authid_from(tmp_user_name);
              result = modify_role_edges_in_table(
                  thd, tables[ACL_TABLES::TABLE_ROLE_EDGES].table, from_user,
                  to_user, false, false);
            }
          }  // end if !is_granted_role()

          default_roles.push_back(create_authid_from(role));
        }

        if (result == 0)
          result = alter_user_set_default_roles(
              thd, tables[ACL_TABLES::TABLE_DEFAULT_ROLES].table, tmp_user_name,
              default_roles);
      }

      /* For connection resources */
      reset_users.insert(tmp_user_name);

    }  // END while tmp_user_name= user_lists++

    // We must not have plain text password for any user at this point.
    thd->lex->contains_plaintext_password = false;

    /* In case of SE error, we would have raised error before reaching here. */
    if (result && !thd->is_error()) {
      my_error(ER_CANNOT_USER, MYF(0),
               (is_role ? "CREATE ROLE" : "CREATE USER"),
               is_anonymous_user ? "anonymous user" : wrong_users.c_ptr_safe());
    }

    User_params user_params(&extra_users);
    result = log_and_commit_acl_ddl(thd, transactional_tables, &extra_users,
                                    &user_params);
  } /* Critical section */

  if (!result) {
    LEX_USER *reset_user;
    DEBUG_SYNC(thd, "before_reset_mqh_in_create_user");
    for (LEX_USER *one_user : reset_users) {
      if ((reset_user = get_current_user(thd, one_user))) {
        reset_mqh(thd, reset_user, false);
      }
    }
    /* Notify storage engines */
    acl_notify_htons(thd, SQLCOM_CREATE_USER, &list);
  }

  if (result == 0) {
    if (generated_passwords.size() == 0) {
      my_ok(thd);
    } else if (send_password_result_set(thd, generated_passwords)) {
      result = 1;
    }
  }  // end if

  /*
    If this is a slave thread we should never have generated random passwords
  */
  DBUG_ASSERT(!thd->slave_thread ||
              (thd->slave_thread && generated_passwords.size() == 0));
  return result;
}

/**
  Drop a list of users and all their privileges.

  @param thd                         The current thread.
  @param list                        The users to drop.
  @param if_exists                   The if exists flag
  @param on_drop_role_priv           enabled by the DROP ROLE privilege

  @retval false      OK
  @retval true       Error
*/

bool mysql_drop_user(THD *thd, List<LEX_USER> &list, bool if_exists,
                     bool on_drop_role_priv) {
  int result = 0;
  String wrong_users;
  LEX_USER *user_name, *tmp_user_name;
  List_iterator<LEX_USER> user_list(list);
  TABLE_LIST tables[ACL_TABLES::LAST_ENTRY];
  sql_mode_t old_sql_mode = thd->variables.sql_mode;
  bool transactional_tables;
  std::set<LEX_USER *> audit_users;
  DBUG_TRACE;

  /*
    Make sure that none of the authids we're about to drop is used as a
    mandatory role. Mandatory roles needs to be disabled first before the
    authid can be dropped.
  */
  LEX_USER *user;
  std::vector<Role_id> mandatory_roles;

  /*
    This statement will be replicated as a statement, even when using
    row-based replication.  The binlog state will be cleared here to
    statement based replication and will be reset to the originals
    values when we are out of this function scope
  */
  Save_and_Restore_binlog_format_state binlog_format_state(thd);

  /* DROP USER may be skipped on replication client. */
  if ((result = open_grant_tables(thd, tables, &transactional_tables)))
    return result != 1;

  { /* Critical section */
    Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::WRITE_MODE);

    if (!acl_cache_lock.lock()) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    if (check_system_user_privilege(thd, list)) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    get_mandatory_roles(&mandatory_roles);
    while ((user = user_list++) != nullptr) {
      if (std::find_if(mandatory_roles.begin(), mandatory_roles.end(),
                       [&](Role_id &id) -> bool {
                         Role_id id2(user->user, user->host);
                         return id == id2;
                       }) != mandatory_roles.end()) {
        Role_id authid(user->user, user->host);
        std::string out;
        authid.auth_str(&out);
        my_error(ER_MANDATORY_ROLE, MYF(0), out.c_str());
        return true;
      }
    }
    user_list.rewind();
    thd->variables.sql_mode &= ~MODE_PAD_CHAR_TO_FULL_LENGTH;

    while ((tmp_user_name = user_list++)) {
      if (!(user_name = get_current_user(thd, tmp_user_name))) {
        result = 1;
        continue;
      }

      audit_users.insert(tmp_user_name);

      int ret = handle_grant_data(thd, tables, true, user_name, nullptr,
                                  on_drop_role_priv);
      if (ret <= 0) {
        if (ret < 0) {
          result = 1;
          break;
        }
        if (if_exists) {
          String warn_user;
          log_user(thd, &warn_user, user_name, false);
          push_warning_printf(
              thd, Sql_condition::SL_NOTE, ER_USER_DOES_NOT_EXIST,
              ER_THD(thd, ER_USER_DOES_NOT_EXIST), warn_user.c_ptr_safe());
        } else {
          log_user(thd, &wrong_users, user_name, wrong_users.length() > 0);
          result = 1;
        }
        continue;
      }
    }

    /* Rebuild 'acl_check_hosts' since 'acl_users' has been modified */
    rebuild_check_host();
    rebuild_cached_acl_users_for_name();

    if (result && !thd->is_error()) {
      String operation_str;
      if (thd->query_plan.get_command() == SQLCOM_DROP_ROLE) {
        operation_str.append("DROP ROLE");
      } else {
        operation_str.append("DROP USER");
      }
      my_error(ER_CANNOT_USER, MYF(0), operation_str.c_ptr_quick(),
               wrong_users.c_ptr_safe());
    }

    if (!thd->is_error())
      result =
          populate_roles_caches(thd, (tables + ACL_TABLES::TABLE_ROLE_EDGES));

    result = log_and_commit_acl_ddl(thd, transactional_tables);

    {
      /* Notify audit plugin. We will ignore the return value. */
      LEX_USER *audit_user;
      for (LEX_USER *one_user : audit_users) {
        if ((audit_user = get_current_user(thd, one_user)))
          mysql_audit_notify(
              thd, AUDIT_EVENT(MYSQL_AUDIT_AUTHENTICATION_AUTHID_DROP),
              thd->is_error(), audit_user->user.str, audit_user->host.str,
              audit_user->plugin.str, is_role_id(audit_user), nullptr, nullptr);
      }
    }
  } /* Critical section */

  /* Notify storage engines */
  if (!result) {
    acl_notify_htons(thd, SQLCOM_DROP_USER, &list);
  }

  thd->variables.sql_mode = old_sql_mode;
  return result;
}

/*
  Rename a user.

  SYNOPSIS
    mysql_rename_user()
    thd                         The current thread.
    list                        The user name pairs: (from, to).

  RETURN
    false       OK.
    true        Error.
*/

bool mysql_rename_user(THD *thd, List<LEX_USER> &list) {
  int result = 0;
  String wrong_users;
  LEX_USER *tmp_user_from;
  LEX_USER *tmp_user_to;
  List_iterator<LEX_USER> user_list(list);
  TABLE_LIST tables[ACL_TABLES::LAST_ENTRY];
  std::unique_ptr<Security_context> orig_sctx = nullptr;
  bool transactional_tables;
  DBUG_TRACE;

  { /* Is this auth id a role id? */
    Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::READ_MODE);
    List_iterator<LEX_USER> authid_list_iterator(list);
    LEX_USER *authid;

    if (!acl_cache_lock.lock()) return true;

    if (check_system_user_privilege(thd, list)) return true;

    while ((authid = authid_list_iterator++)) {
      if (!(authid = get_current_user(thd, authid))) {
        /*
          This user is not a role.
        */
        continue;
      }
      if (is_role_id(authid)) {
        my_error(ER_RENAME_ROLE, MYF(0));
        return true;
      }
    }
  }

  /*
    This statement will be replicated as a statement, even when using
    row-based replication.  The binlog state will be cleared here to
    statement based replication and will be reset to the originals
    values when we are out of this function scope
  */
  Save_and_Restore_binlog_format_state binlog_format_state(thd);

  /* RENAME USER may be skipped on replication client. */
  if ((result = open_grant_tables(thd, tables, &transactional_tables)))
    return result != 1;

  { /* Critical section */
    Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::WRITE_MODE);
    if (!acl_cache_lock.lock()) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    while ((tmp_user_from = user_list++)) {
      LEX_USER *user_from;
      LEX_USER *user_to;
      if (!(user_from = get_current_user(thd, tmp_user_from))) {
        result = 1;
        continue;
      }
      tmp_user_to = user_list++;
      if (!(user_to = get_current_user(thd, tmp_user_to))) {
        result = 1;
        continue;
      }
      DBUG_ASSERT(user_to != nullptr); /* Syntax enforces pairs of users. */

      /*
        Search all in-memory structures and grant tables
        for a mention of the new user name.
      */
      int ret = handle_grant_data(thd, tables, false, user_to, nullptr, false);

      if (ret != 0) {
        if (ret < 0) {
          result = 1;
          break;
        }

        log_user(thd, &wrong_users, user_from, wrong_users.length() > 0);
        result = 1;
        continue;
      }

      ret = handle_grant_data(thd, tables, false, user_from, user_to, false);

      if (ret <= 0) {
        if (ret < 0) {
          result = 1;
          break;
        }

        log_user(thd, &wrong_users, user_from, wrong_users.length() > 0);
        result = 1;
        continue;
      }

      Update_dynamic_privilege_table update_table(
          thd, tables[ACL_TABLES::TABLE_DYNAMIC_PRIV].table);
      if (rename_dynamic_grant(user_from->user, user_from->host, user_to->user,
                               user_to->host, update_table)) {
        result = 1;
        break;
      }
      roles_rename_authid(thd, tables[ACL_TABLES::TABLE_ROLE_EDGES].table,
                          tables[ACL_TABLES::TABLE_DEFAULT_ROLES].table,
                          user_from, user_to);
      /* Update the security context if user renames self */
      if (do_update_sctx(thd->security_context(), user_from)) {
        /* Keep a copy of original security context if not done already */
        if (orig_sctx == nullptr) {
          orig_sctx = std::make_unique<Security_context>();
          *(orig_sctx.get()) = *(thd->security_context());
        }
        update_sctx(thd->security_context(), tmp_user_to);
      }
    }

    /* Rebuild 'acl_check_hosts' since 'acl_users' has been modified */
    rebuild_check_host();
    rebuild_cached_acl_users_for_name();

    if (result && !thd->is_error())
      my_error(ER_CANNOT_USER, MYF(0), "RENAME USER", wrong_users.c_ptr_safe());

    if (!thd->is_error())
      result =
          populate_roles_caches(thd, (tables + ACL_TABLES::TABLE_ROLE_EDGES));

    /*
      Restore the orignal security context temporarily because binlog must
      write the original definer/invoker in the binlog in order for slave
      to work
    */
    Security_context *current_sctx = thd->security_context();
    current_sctx->restore_security_context(thd, orig_sctx.get());

    result = log_and_commit_acl_ddl(thd, transactional_tables);

    /* Restore the updated security context */
    current_sctx->restore_security_context(thd, current_sctx);

    /* Notify audit plugin. We will ignore the return value. */
    LEX_USER *user_from, *user_to;
    List_iterator<LEX_USER> audit_user_list(list);
    LEX_USER *audit_user_from, *audit_user_to;
    while ((audit_user_from = audit_user_list++)) {
      audit_user_to = audit_user_list++;

      if ((((user_from = get_current_user(thd, audit_user_from)) &&
            ((user_to = get_current_user(thd, audit_user_to))))))
        mysql_audit_notify(
            thd, AUDIT_EVENT(MYSQL_AUDIT_AUTHENTICATION_AUTHID_RENAME),
            thd->is_error(), user_from->user.str, user_from->host.str,
            user_from->plugin.str, is_role_id(user_from), user_to->user.str,
            user_to->user.str);
    }
  } /* Critical section */

  /* Notify storage engines */
  if (!result) {
    acl_notify_htons(thd, SQLCOM_RENAME_USER, &list);
  }

  return result;
}

/*
  Alter user list.

  SYNOPSIS
    mysql_alter_user()
    thd                         The current thread.
    list                        The user names.

  RETURN
    false       OK.
    true        Error.
*/

bool mysql_alter_user(THD *thd, List<LEX_USER> &list, bool if_exists) {
  int result = 0;
  bool is_anonymous_user = false;
  String wrong_users;
  LEX_USER *user_from, *tmp_user_from;
  List_iterator<LEX_USER> user_list(list);
  TABLE_LIST tables[ACL_TABLES::LAST_ENTRY];
  bool transactional_tables;
  bool is_privileged_user = false;
  std::set<LEX_USER *> extra_users;
  ACL_USER *self = nullptr;
  bool password_expire_undo = false;
  std::set<LEX_USER *> audit_users;
  std::set<LEX_USER *> reset_users;
  Userhostpassword_list generated_passwords;
  DBUG_TRACE;

  /*
    This statement will be replicated as a statement, even when using
    row-based replication.  The binlog state will be cleared here to
    statement based replication and will be reset to the originals
    values when we are out of this function scope
  */
  Save_and_Restore_binlog_format_state binlog_format_state(thd);

  if ((result = open_grant_tables(thd, tables, &transactional_tables)))
    return result != 1;

  { /* Critical section */
    Acl_cache_lock_guard acl_cache_lock(thd, Acl_cache_lock_mode::WRITE_MODE);

    if (!acl_cache_lock.lock()) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    if (check_system_user_privilege(thd, list)) {
      commit_and_close_mysql_tables(thd);
      return true;
    }

    is_privileged_user = is_privileged_user_for_credential_change(thd);

    while ((tmp_user_from = user_list++)) {
      int ret;
      ACL_USER *acl_user;
      acl_table::Pod_user_what_to_update what_to_alter;
      bool history_check_done = false;
      TABLE *history_tbl = nullptr;
      bool dummy_row_existed = false;

      /* add the defaults where needed */
      if (!(user_from = get_current_user(thd, tmp_user_from))) {
        log_user(thd, &wrong_users, tmp_user_from, wrong_users.length() > 0);
        result = 1;
        continue;
      }

      /* copy password expire attributes to individual lex user */
      user_from->alter_status = thd->lex->alter_password;

      if (set_and_validate_user_attributes(
              thd, user_from, what_to_alter, is_privileged_user, false,
              &tables[ACL_TABLES::TABLE_PASSWORD_HISTORY], &history_check_done,
              "ALTER USER", generated_passwords)) {
        result = 1;
        continue;
      }

      /*
        Check if the user's authentication method supports expiration only
        if PASSWORD EXPIRE attribute is specified
      */
      if (user_from->alter_status.update_password_expired_column &&
          !auth_plugin_supports_expiration(user_from->plugin.str)) {
        result = 1;
        log_user(thd, &wrong_users, user_from, wrong_users.length() > 0);
        continue;
      }

      if (!strcmp(user_from->user.str, "") &&
          (what_to_alter.m_what & PASSWORD_EXPIRE_ATTR) &&
          user_from->alter_status.update_password_expired_column) {
        result = 1;
        is_anonymous_user = true;
        continue;
      }

      acl_user = find_acl_user(user_from->host.str, user_from->user.str, true);

      if (history_check_done) {
        /*
          If the history check is already done we pretend there's no
          history table so we can turn off the eventual check here.
        */
        history_tbl = tables[ACL_TABLES::TABLE_PASSWORD_HISTORY].table;
        tables[ACL_TABLES::TABLE_PASSWORD_HISTORY].table = nullptr;
      }
      ret = handle_grant_data(thd, tables, false, user_from, nullptr, false);

      /* purge the password history if plugin is different */
      if ((what_to_alter.m_what & DIFFERENT_PLUGIN_ATTR) &&
          handle_password_history_table(thd, tables, true, user_from, nullptr,
                                        &dummy_row_existed)) {
        /* can't delete stuff from password history */
        result = 1;
        break;
      }

      if (history_check_done) {
        tables[ACL_TABLES::TABLE_PASSWORD_HISTORY].table = history_tbl;
      }

      if (!acl_user || ret <= 0) {
        if (ret < 0) {
          result = 1;
          break;
        }

        if (if_exists) {
          String warn_user;
          log_user(thd, &warn_user, user_from, false);
          push_warning_printf(
              thd, Sql_condition::SL_NOTE, ER_USER_DOES_NOT_EXIST,
              ER_THD(thd, ER_USER_DOES_NOT_EXIST), warn_user.c_ptr_safe());
          try {
            extra_users.insert(tmp_user_from);
          } catch (...) {
            LogErr(WARNING_LEVEL,
                   ER_USER_NOT_IN_EXTRA_USERS_BINLOG_POSSIBLY_INCOMPLETE,
                   warn_user.c_ptr_safe());
          }
        } else {
          log_user(thd, &wrong_users, user_from, wrong_users.length() > 0);
          result = 1;
        }
        continue;
      }

      /* update the mysql.user table */
      ret = replace_user_table(thd, tables[ACL_TABLES::TABLE_USER].table,
                               user_from, 0, false, false, what_to_alter);
      if (ret) {
        if (ret < 0) {
          result = 1;
          break;
        }
        if (if_exists) {
          String warn_user;
          log_user(thd, &warn_user, user_from, false);
          push_warning_printf(
              thd, Sql_condition::SL_NOTE, ER_USER_DOES_NOT_EXIST,
              ER_THD(thd, ER_USER_DOES_NOT_EXIST), warn_user.c_ptr_safe());
          try {
            extra_users.insert(user_from);
          } catch (...) {
            LogErr(WARNING_LEVEL,
                   ER_USER_NOT_IN_EXTRA_USERS_BINLOG_POSSIBLY_INCOMPLETE,
                   warn_user.c_ptr_safe());
          }
        } else {
          log_user(thd, &wrong_users, user_from, wrong_users.length() > 0);
          result = 1;
        }
        continue;
      }
      if (update_sctx_cache(
              thd->security_context(), acl_user,
              user_from->alter_status.update_password_expired_column)) {
        self = acl_user;
        password_expire_undo =
            !user_from->alter_status.update_password_expired_column;
      }

      /*
        If there is change related to authentication plugin,
        we would like to notify interested audit plugins.
      */
      if (what_to_alter.m_what & PLUGIN_ATTR) audit_users.insert(tmp_user_from);

      if (what_to_alter.m_what & RESOURCE_ATTR)
        reset_users.insert(tmp_user_from);
    }

    // We must not have plain text password for any user at this point.
    thd->lex->contains_plaintext_password = false;

    clear_and_init_db_cache();  // Clear locked hostname cache

    if (result && self)
      update_sctx_cache(thd->security_context(), self, password_expire_undo);

    if (result && !thd->is_error()) {
      if (is_anonymous_user)
        my_error(ER_PASSWORD_EXPIRE_ANONYMOUS_USER, MYF(0));
      else
        my_error(ER_CANNOT_USER, MYF(0), "ALTER USER",
                 wrong_users.c_ptr_safe());
    }

    User_params user_params(&extra_users);
    result = log_and_commit_acl_ddl(thd, transactional_tables, &extra_users,
                                    &user_params);
    /* Notify audit plugin. We will ignore the return value. */
    LEX_USER *audit_user;
    for (LEX_USER *one_user : audit_users) {
      if ((audit_user = get_current_user(thd, one_user)))
        mysql_audit_notify(
            thd, AUDIT_EVENT(MYSQL_AUDIT_AUTHENTICATION_CREDENTIAL_CHANGE),
            thd->is_error(), audit_user->user.str, audit_user->host.str,
            audit_user->plugin.str, is_role_id(audit_user), nullptr, nullptr);
    }
  } /* Critical section */

  if (!result) {
    LEX_USER *extra_user;
    DEBUG_SYNC(thd, "before_reset_mqh_in_alter_user");
    for (LEX_USER *one_user : reset_users) {
      if ((extra_user = get_current_user(thd, one_user))) {
        reset_mqh(thd, extra_user, false);
      }
    }
    /* Notify storage engines (including rewrite list) */
    acl_notify_htons(thd, SQLCOM_ALTER_USER, &list, &extra_users);
  }

  if (result == 0) {
    if (generated_passwords.size() == 0) {
      my_ok(thd);
    } else if (send_password_result_set(thd, generated_passwords)) {
      result = 1;
    }
  }  // end if
  return result;
}
