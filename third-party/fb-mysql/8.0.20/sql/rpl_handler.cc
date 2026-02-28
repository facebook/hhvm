/* Copyright (c) 2008, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/rpl_handler.h"

#include <string.h>
#include <map>
#include <memory>
#include <new>
#include <unordered_map>
#include <utility>
#include <vector>

#include "lex_string.h"
#include "map_helpers.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_io.h"
#include "my_loglevel.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/plugin.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/mysql_thread.h"
#include "mysql/psi/psi_base.h"
#include "mysql/raft_listener_queue_if.h"
#include "mysql/service_mysql_alloc.h"
#include "mysqld_error.h"
#include "prealloced_array.h"
#include "sql/current_thd.h"
#include "sql/debug_sync.h"  // DEBUG_SYNC
#include "sql/handler.h"
#include "sql/item_func.h"  // user_var_entry
#include "sql/key.h"
#include "sql/log.h"
#include "sql/mysqld.h"              // server_uuid
#include "sql/mysqld_thd_manager.h"  // Global_THD_manager
#include "sql/psi_memory_key.h"
#include "sql/replication.h"  // Trans_param
#include "sql/rpl_gtid.h"
#include "sql/rpl_mi.h"     // Master_info
#include "sql/set_var.h"    // OPT_PERSIST
#include "sql/sql_class.h"  // THD
#include "sql/sql_const.h"
#include "sql/sql_lex.h"
#include "sql/sql_parse.h"   // sql_command_flags
#include "sql/sql_plugin.h"  // plugin_int_to_ref
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/thr_malloc.h"
#include "sql/transaction_info.h"
#include "sql_string.h"

/** start of raft related extern funtion declarations  **/

extern int raft_reset_slave(THD *thd);
extern int raft_change_master(
    THD *thd, const std::pair<const std::string, uint> &master_instance,
    const std::string &master_uuid);
extern int rotate_binlog_file(THD *thd);
extern int raft_stop_sql_thread(THD *thd);
extern int raft_stop_io_thread(THD *thd);
extern int raft_start_sql_thread(THD *thd);
extern int rli_relay_log_raft_reset(
    std::pair<std::string, uint64_t> raft_log_applied_upto_pos, THD *thd);
extern int trim_logged_gtid(const std::vector<std::string> &trimmed_gtids);
/** end of raft related extern funtion declarations  **/

Trans_delegate *transaction_delegate;
Binlog_storage_delegate *binlog_storage_delegate;
Server_state_delegate *server_state_delegate;

Binlog_transmit_delegate *binlog_transmit_delegate;
Binlog_relay_IO_delegate *binlog_relay_io_delegate;

/* Raft plugin related variables */
Raft_replication_delegate *raft_replication_delegate;
RaftListenerQueue raft_listener_queue;

Observer_info::Observer_info(void *ob, st_plugin_int *p)
    : observer(ob), plugin_int(p) {
  plugin = plugin_int_to_ref(plugin_int);
}

Delegate::Delegate(
#ifdef HAVE_PSI_RWLOCK_INTERFACE
    PSI_rwlock_key key
#endif
) {
  inited = false;
#ifdef HAVE_PSI_RWLOCK_INTERFACE
  if (mysql_rwlock_init(key, &lock)) return;
#else
  if (mysql_rwlock_init(0, &lock)) return;
#endif
  init_sql_alloc(key_memory_delegate, &memroot, 1024, 0);
  inited = true;
}

/*
  structure to save transaction log filename and position
*/
typedef struct Trans_binlog_info {
  my_off_t log_pos;
  char log_file[FN_REFLEN];
} Trans_binlog_info;

int get_user_var_int(const char *name, long long int *value, int *null_value) {
  bool null_val;
  THD *thd = current_thd;

  /* Protects thd->user_vars. */
  mysql_mutex_lock(&thd->LOCK_thd_data);

  const auto it = thd->user_vars.find(name);
  if (it == thd->user_vars.end()) {
    mysql_mutex_unlock(&thd->LOCK_thd_data);
    return 1;
  }
  *value = it->second->val_int(&null_val);
  if (null_value) *null_value = null_val;
  mysql_mutex_unlock(&thd->LOCK_thd_data);
  return 0;
}

int get_user_var_real(const char *name, double *value, int *null_value) {
  bool null_val;
  THD *thd = current_thd;

  /* Protects thd->user_vars. */
  mysql_mutex_lock(&thd->LOCK_thd_data);

  const auto it = thd->user_vars.find(name);
  if (it == thd->user_vars.end()) {
    mysql_mutex_unlock(&thd->LOCK_thd_data);
    return 1;
  }
  *value = it->second->val_real(&null_val);
  if (null_value) *null_value = null_val;
  mysql_mutex_unlock(&thd->LOCK_thd_data);
  return 0;
}

int get_user_var_str(const char *name, char *value, size_t len,
                     unsigned int precision, int *null_value) {
  String str;
  bool null_val;
  THD *thd = current_thd;

  /* Protects thd->user_vars. */
  mysql_mutex_lock(&thd->LOCK_thd_data);

  const auto it = thd->user_vars.find(name);
  if (it == thd->user_vars.end()) {
    mysql_mutex_unlock(&thd->LOCK_thd_data);
    return 1;
  }
  it->second->val_str(&null_val, &str, precision);
  strncpy(value, str.c_ptr(), len);
  if (null_value) *null_value = null_val;
  mysql_mutex_unlock(&thd->LOCK_thd_data);
  return 0;
}

int delegates_init() {
  alignas(Trans_delegate) static char place_trans_mem[sizeof(Trans_delegate)];
  alignas(Binlog_storage_delegate) static char
      place_storage_mem[sizeof(Binlog_storage_delegate)];
  alignas(Server_state_delegate) static char
      place_state_mem[sizeof(Server_state_delegate)];
  alignas(Binlog_transmit_delegate) static char
      place_transmit_mem[sizeof(Binlog_transmit_delegate)];
  alignas(Binlog_relay_IO_delegate) static char
      place_relay_io_mem[sizeof(Binlog_relay_IO_delegate)];
  alignas(Raft_replication_delegate) static char
      place_raft_mem[sizeof(Raft_replication_delegate)];

  transaction_delegate = new (place_trans_mem) Trans_delegate;
  if (!transaction_delegate->is_inited()) {
    LogErr(ERROR_LEVEL, ER_RPL_TRX_DELEGATES_INIT_FAILED);
    return 1;
  }

  binlog_storage_delegate = new (place_storage_mem) Binlog_storage_delegate;
  if (!binlog_storage_delegate->is_inited()) {
    LogErr(ERROR_LEVEL, ER_RPL_BINLOG_STORAGE_DELEGATES_INIT_FAILED);
    return 1;
  }

  server_state_delegate = new (place_state_mem) Server_state_delegate;
  binlog_transmit_delegate = new (place_transmit_mem) Binlog_transmit_delegate;
  if (!binlog_transmit_delegate->is_inited()) {
    LogErr(ERROR_LEVEL, ER_RPL_BINLOG_TRANSMIT_DELEGATES_INIT_FAILED);
    return 1;
  }

  binlog_relay_io_delegate = new (place_relay_io_mem) Binlog_relay_IO_delegate;
  if (!binlog_relay_io_delegate->is_inited()) {
    LogErr(ERROR_LEVEL, ER_RPL_BINLOG_RELAY_DELEGATES_INIT_FAILED);
    return 1;
  }

  raft_replication_delegate = new (place_raft_mem) Raft_replication_delegate;
  if (!raft_replication_delegate->is_inited()) {
    // TODO: use raft specific error codes later
    LogErr(ERROR_LEVEL, ER_RPL_BINLOG_RELAY_DELEGATES_INIT_FAILED);
    return 1;
  }

  return 0;
}

void delegates_destroy() {
  if (transaction_delegate) transaction_delegate->~Trans_delegate();
  if (binlog_storage_delegate)
    binlog_storage_delegate->~Binlog_storage_delegate();
  if (server_state_delegate) server_state_delegate->~Server_state_delegate();
  if (binlog_transmit_delegate)
    binlog_transmit_delegate->~Binlog_transmit_delegate();
  if (binlog_relay_io_delegate)
    binlog_relay_io_delegate->~Binlog_relay_IO_delegate();
}

/*
  This macro is used by raft Delegate methods to call into raft plugin
  The only difference is that this is a 'stricter' version which will return
  failure if the plugin hooks were not called
 */
#define FOREACH_OBSERVER_STRICT(r, f, args)                            \
  Prealloced_array<plugin_ref, 8> plugins(PSI_NOT_INSTRUMENTED);       \
  read_lock();                                                         \
  Observer_info_iterator iter = observer_info_iter();                  \
  Observer_info *info = iter++;                                        \
  for (; info; info = iter++) {                                        \
    plugin_ref plugin = my_plugin_lock(0, &info->plugin);              \
    if (!plugin) {                                                     \
      /* plugin is not intialized or deleted, this is not an error */  \
      enable_raft_plugin ? (r) = 1 : (r) = 0;                          \
      break;                                                           \
    }                                                                  \
    plugins.push_back(plugin);                                         \
    if (((Observer *)info->observer)->f &&                             \
        ((Observer *)info->observer)->f args) {                        \
      r = 1;                                                           \
      LogEvent()                                                       \
          .prio(ERROR_LEVEL)                                           \
          .errcode(ER_RPL_PLUGIN_FUNCTION_FAILED)                      \
          .subsys(LOG_SUBSYSTEM_TAG)                                   \
          .function(#f)                                                \
          .message("Run function '" #f "' in plugin '%s' failed",      \
                   info->plugin_int->name.str);                        \
      break;                                                           \
    }                                                                  \
    /* Plugin is successfully called, set return status to 0           \
     * indicating success */                                           \
    (r) = 0;                                                           \
  }                                                                    \
  unlock();                                                            \
  /*                                                                   \
     Unlock plugins should be done after we released the Delegate lock \
     to avoid possible deadlock when this is the last user of the      \
     plugin, and when we unlock the plugin, it will try to             \
     deinitialize the plugin, which will try to lock the Delegate in   \
     order to remove the observers.                                    \
  */                                                                   \
  if (!plugins.empty()) plugin_unlock_list(0, &plugins[0], plugins.size());

/*
  This macro is used by almost all the Delegate methods to iterate
  over all the observers running given callback function of the
  delegate .

  Add observer plugins to the thd->lex list, after each statement, all
  plugins add to thd->lex will be automatically unlocked.
 */
#define FOREACH_OBSERVER(r, f, args)                                   \
  Prealloced_array<plugin_ref, 8> plugins(PSI_NOT_INSTRUMENTED);       \
  read_lock();                                                         \
  Observer_info_iterator iter = observer_info_iter();                  \
  Observer_info *info = iter++;                                        \
  for (; info; info = iter++) {                                        \
    plugin_ref plugin = my_plugin_lock(0, &info->plugin);              \
    if (!plugin) {                                                     \
      /* plugin is not intialized or deleted, this is not an error */  \
      r = 0;                                                           \
      break;                                                           \
    }                                                                  \
    plugins.push_back(plugin);                                         \
    if (((Observer *)info->observer)->f &&                             \
        ((Observer *)info->observer)->f args) {                        \
      r = 1;                                                           \
      LogEvent()                                                       \
          .prio(ERROR_LEVEL)                                           \
          .errcode(ER_RPL_PLUGIN_FUNCTION_FAILED)                      \
          .subsys(LOG_SUBSYSTEM_TAG)                                   \
          .function(#f)                                                \
          .message("Run function '" #f "' in plugin '%s' failed",      \
                   info->plugin_int->name.str);                        \
      break;                                                           \
    }                                                                  \
  }                                                                    \
  unlock();                                                            \
  /*                                                                   \
     Unlock plugins should be done after we released the Delegate lock \
     to avoid possible deadlock when this is the last user of the      \
     plugin, and when we unlock the plugin, it will try to             \
     deinitialize the plugin, which will try to lock the Delegate in   \
     order to remove the observers.                                    \
  */                                                                   \
  if (!plugins.empty()) plugin_unlock_list(0, &plugins[0], plugins.size());

#define FOREACH_OBSERVER_ERROR_OUT(r, f, args, out)                    \
  Prealloced_array<plugin_ref, 8> plugins(PSI_NOT_INSTRUMENTED);       \
  read_lock();                                                         \
  Observer_info_iterator iter = observer_info_iter();                  \
  Observer_info *info = iter++;                                        \
                                                                       \
  int error_out = 0;                                                   \
  for (; info; info = iter++) {                                        \
    plugin_ref plugin = my_plugin_lock(0, &info->plugin);              \
    if (!plugin) {                                                     \
      /* plugin is not intialized or deleted, this is not an error */  \
      r = 0;                                                           \
      break;                                                           \
    }                                                                  \
    plugins.push_back(plugin);                                         \
                                                                       \
    bool hook_error = false;                                           \
    hook_error = ((Observer *)info->observer)->f(args, error_out);     \
                                                                       \
    out += error_out;                                                  \
    if (hook_error) {                                                  \
      r = 1;                                                           \
      LogEvent()                                                       \
          .prio(ERROR_LEVEL)                                           \
          .errcode(ER_RPL_PLUGIN_FUNCTION_FAILED)                      \
          .subsys(LOG_SUBSYSTEM_TAG)                                   \
          .function(#f)                                                \
          .message("Run function '" #f "' in plugin '%s' failed",      \
                   info->plugin_int->name.str);                        \
      break;                                                           \
    }                                                                  \
  }                                                                    \
  unlock();                                                            \
  /*                                                                   \
     Unlock plugins should be done after we released the Delegate lock \
     to avoid possible deadlock when this is the last user of the      \
     plugin, and when we unlock the plugin, it will try to             \
     deinitialize the plugin, which will try to lock the Delegate in   \
     order to remove the observers.                                    \
  */                                                                   \
  if (!plugins.empty()) plugin_unlock_list(0, &plugins[0], plugins.size());

static bool se_before_commit(THD *, plugin_ref plugin, void *arg) {
  handlerton *hton = plugin_data<handlerton *>(plugin);
  if (hton->se_before_commit) hton->se_before_commit(arg);
  return false;
}

int Trans_delegate::before_commit(THD *thd, bool all,
                                  Binlog_cache_storage *trx_cache_log,
                                  Binlog_cache_storage *stmt_cache_log,
                                  ulonglong cache_log_max_size,
                                  bool is_atomic_ddl_arg) {
  DBUG_TRACE;
  Trans_param param;
  TRANS_PARAM_ZERO(param);
  param.server_id = thd->server_id;
  param.server_uuid = server_uuid;
  param.thread_id = thd->thread_id();
  param.gtid_info.type = thd->variables.gtid_next.type;
  param.gtid_info.sidno = thd->variables.gtid_next.gtid.sidno;
  param.gtid_info.gno = thd->variables.gtid_next.gtid.gno;
  param.trx_cache_log = trx_cache_log;
  param.stmt_cache_log = stmt_cache_log;
  param.cache_log_max_size = cache_log_max_size;
  param.original_commit_timestamp = &thd->variables.original_commit_timestamp;
  param.is_atomic_ddl = is_atomic_ddl_arg;
  param.rpl_channel_type = thd->rpl_thd_ctx.get_rpl_channel_type();
  param.group_replication_consistency =
      thd->variables.group_replication_consistency;
  param.original_server_version = &(thd->variables.original_server_version);
  param.immediate_server_version = &(thd->variables.immediate_server_version);

  bool is_real_trans =
      (all || !thd->get_transaction()->is_active(Transaction_ctx::SESSION));
  if (is_real_trans) param.flags |= TRANS_IS_REAL_TRANS;

  if (mysql_bin_log.is_apply_log)
    thd->get_trans_relay_log_pos(&param.log_file, &param.log_pos);
  else
    thd->get_trans_fixed_pos(&param.log_file, &param.log_pos);

  DBUG_PRINT("enter",
             ("log_file: %s, log_pos: %llu", param.log_file, param.log_pos));
  int ret = 0;
  FOREACH_OBSERVER(ret, before_commit, (&param));
  plugin_foreach(thd, se_before_commit, MYSQL_STORAGE_ENGINE_PLUGIN, &param);
  return ret;
}

/**
 Helper method to check if the given table has 'CASCADE' foreign key or not.

 @param[in]   table     Table object that needs to be verified.

 @return bool true      If the table has 'CASCADE' foreign key.
              false     If the table does not have 'CASCADE' foreign key.
*/
bool has_cascade_foreign_key(TABLE *table) {
  DBUG_TRACE;

  TABLE_SHARE_FOREIGN_KEY_INFO *fk = table->s->foreign_key;

  for (uint i = 0; i < table->s->foreign_keys; i++) {
    /*
      The supported values of update/delete_rule are: CASCADE, SET NULL,
      NO ACTION, RESTRICT and SET DEFAULT.
    */
    if (dd::Foreign_key::RULE_CASCADE == fk[i].update_rule ||
        dd::Foreign_key::RULE_CASCADE == fk[i].delete_rule ||
        dd::Foreign_key::RULE_SET_NULL == fk[i].update_rule ||
        dd::Foreign_key::RULE_SET_NULL == fk[i].delete_rule ||
        dd::Foreign_key::RULE_SET_DEFAULT == fk[i].update_rule ||
        dd::Foreign_key::RULE_SET_DEFAULT == fk[i].delete_rule) {
      return true;
    }
  }
  return false;
}

/**
 Helper method to create table information for the hook call
 */
void prepare_table_info(THD *thd, Trans_table_info *&table_info_list,
                        uint &number_of_tables) {
  DBUG_TRACE;

  TABLE *open_tables = thd->open_tables;

  // Fail if tables are not open
  if (open_tables == nullptr) {
    return;
  }

  // Gather table information
  std::vector<Trans_table_info> table_info_holder;
  for (; open_tables != nullptr; open_tables = open_tables->next) {
    Trans_table_info table_info = {nullptr, 0, 0, false};

    if (open_tables->no_replicate) {
      continue;
    }

    table_info.table_name = open_tables->s->table_name.str;

    uint primary_keys = 0;
    if (open_tables->key_info != nullptr &&
        (open_tables->s->primary_key < MAX_KEY)) {
      primary_keys = open_tables->s->primary_key;

      // if primary keys is still 0, lets double check on another var
      if (primary_keys == 0) {
        primary_keys = open_tables->key_info->user_defined_key_parts;
      }
    }

    table_info.number_of_primary_keys = primary_keys;

    table_info.db_type = open_tables->s->db_type()->db_type;

    /*
      Find out if the table has foreign key with ON UPDATE/DELETE CASCADE
      clause.
    */
    table_info.has_cascade_foreign_key = has_cascade_foreign_key(open_tables);

    table_info_holder.push_back(table_info);
  }

  // Now that one has all the information, one should build the
  // data that will be delivered to the plugin
  if (table_info_holder.size() > 0) {
    number_of_tables = table_info_holder.size();

    table_info_list = (Trans_table_info *)my_malloc(
        PSI_NOT_INSTRUMENTED, number_of_tables * sizeof(Trans_table_info),
        MYF(0));

    std::vector<Trans_table_info>::iterator table_info_holder_it =
        table_info_holder.begin();
    for (int table = 0; table_info_holder_it != table_info_holder.end();
         table_info_holder_it++, table++) {
      table_info_list[table].number_of_primary_keys =
          (*table_info_holder_it).number_of_primary_keys;
      table_info_list[table].table_name = (*table_info_holder_it).table_name;
      table_info_list[table].db_type = (*table_info_holder_it).db_type;
      table_info_list[table].has_cascade_foreign_key =
          (*table_info_holder_it).has_cascade_foreign_key;
    }
  }
}

/**
  Helper that gathers all table runtime information

  @param[in]   thd       the current execution thread
  @param[out]  ctx_info  Trans_context_info in which the result is stored.
 */
static void prepare_transaction_context(THD *thd,
                                        Trans_context_info &ctx_info) {
  // Extracting the session value of SQL binlogging
  ctx_info.binlog_enabled = thd->variables.sql_log_bin;

  // Extracting the session value of binlog format
  ctx_info.binlog_format = thd->variables.binlog_format;

  // Extracting the global mutable value of binlog checksum
  ctx_info.binlog_checksum_options = binlog_checksum_options;

  // Extracting the session value of transaction_write_set_extraction
  ctx_info.transaction_write_set_extraction =
      thd->variables.transaction_write_set_extraction;

  // Extracting transaction isolation level
  ctx_info.tx_isolation = thd->tx_isolation;
}

int Trans_delegate::before_dml(THD *thd, int &result) {
  DBUG_TRACE;
  Trans_param param;
  TRANS_PARAM_ZERO(param);

  param.server_id = thd->server_id;
  param.server_uuid = server_uuid;
  param.thread_id = thd->thread_id();

  prepare_table_info(thd, param.tables_info, param.number_of_tables);
  prepare_transaction_context(thd, param.trans_ctx_info);

  int ret = 0;
  FOREACH_OBSERVER_ERROR_OUT(ret, before_dml, &param, result);

  my_free(param.tables_info);

  return ret;
}

static bool se_before_rollback(THD *, plugin_ref plugin, void *arg) {
  handlerton *hton = plugin_data<handlerton *>(plugin);
  if (hton->se_before_rollback) hton->se_before_rollback(arg);
  return false;
}

int Trans_delegate::before_rollback(THD *thd, bool all) {
  DBUG_TRACE;
  Trans_param param;
  TRANS_PARAM_ZERO(param);
  param.server_id = thd->server_id;
  param.server_uuid = server_uuid;
  param.thread_id = thd->thread_id();
  param.rpl_channel_type = thd->rpl_thd_ctx.get_rpl_channel_type();

  bool is_real_trans =
      (all || !thd->get_transaction()->is_active(Transaction_ctx::SESSION));
  if (is_real_trans) param.flags |= TRANS_IS_REAL_TRANS;

  int ret = 0;
  FOREACH_OBSERVER(ret, before_rollback, (&param));
  plugin_foreach(thd, se_before_rollback, MYSQL_STORAGE_ENGINE_PLUGIN, &param);
  return ret;
}

static bool se_after_commit(THD *, plugin_ref plugin, void *arg) {
  handlerton *hton = plugin_data<handlerton *>(plugin);
  if (hton->se_after_commit) hton->se_after_commit(arg);
  return false;
}

int Trans_delegate::after_commit(THD *thd, bool all) {
  DBUG_TRACE;
  Trans_param param;
  TRANS_PARAM_ZERO(param);
  param.server_uuid = server_uuid;
  param.thread_id = thd->thread_id();

  Gtid gtid;
  thd->rpl_thd_ctx.last_used_gtid_tracker_ctx().get_last_used_gtid(gtid);
  param.gtid_info.sidno = gtid.sidno;
  param.gtid_info.gno = gtid.gno;

  bool is_real_trans =
      (all || !thd->get_transaction()->is_active(Transaction_ctx::SESSION));
  if (is_real_trans) param.flags |= TRANS_IS_REAL_TRANS;
  thd->get_trans_fixed_pos(&param.log_file, &param.log_pos);
  param.server_id = thd->server_id;
  param.rpl_channel_type = thd->rpl_thd_ctx.get_rpl_channel_type();

  DBUG_PRINT("enter",
             ("log_file: %s, log_pos: %llu", param.log_file, param.log_pos));
  DEBUG_SYNC(thd, "before_call_after_commit_observer");

  int ret = 0;
  FOREACH_OBSERVER(ret, after_commit, (&param));
  plugin_foreach(thd, se_after_commit, MYSQL_STORAGE_ENGINE_PLUGIN, &param);
  return ret;
}

int Trans_delegate::after_rollback(THD *thd, bool all) {
  DBUG_TRACE;
  Trans_param param;
  TRANS_PARAM_ZERO(param);
  param.server_uuid = server_uuid;
  param.thread_id = thd->thread_id();

  bool is_real_trans =
      (all || !thd->get_transaction()->is_active(Transaction_ctx::SESSION));
  if (is_real_trans) param.flags |= TRANS_IS_REAL_TRANS;
  if (mysql_bin_log.is_apply_log)
    thd->get_trans_relay_log_pos(&param.log_file, &param.log_pos);
  else
    thd->get_trans_fixed_pos(&param.log_file, &param.log_pos);
  param.server_id = thd->server_id;
  param.rpl_channel_type = thd->rpl_thd_ctx.get_rpl_channel_type();

  int ret = 0;
  FOREACH_OBSERVER(ret, after_rollback, (&param));
  return ret;
}

int Trans_delegate::trans_begin(THD *thd, int &out) {
  DBUG_TRACE;
  Trans_param param;
  TRANS_PARAM_ZERO(param);
  param.server_uuid = server_uuid;
  param.thread_id = thd->thread_id();
  param.group_replication_consistency =
      thd->variables.group_replication_consistency;
  param.hold_timeout = thd->variables.net_wait_timeout;
  param.server_id = thd->server_id;
  param.rpl_channel_type = thd->rpl_thd_ctx.get_rpl_channel_type();

  int ret = 0;
  FOREACH_OBSERVER_ERROR_OUT(ret, begin, &param, out);
  return ret;
}

int Binlog_storage_delegate::after_flush(THD *thd, const char *log_file,
                                         my_off_t log_pos) {
  DBUG_TRACE;
  DBUG_PRINT("enter",
             ("log_file: %s, log_pos: %llu", log_file, (ulonglong)log_pos));
  Binlog_storage_param param;
  param.server_id = thd->server_id;

  int ret = 0;
  FOREACH_OBSERVER(ret, after_flush, (&param, log_file, log_pos));
  return ret;
}

/**
 * This hook MUST be invoked after ALL recovery operations are performed
 * and the server is ready to serve clients.
 *
 * @return 0 on success, >0 otherwise.
 */
int Server_state_delegate::before_handle_connection(THD *) {
  DBUG_TRACE;
  Server_state_param param;

  int ret = 0;
  FOREACH_OBSERVER(ret, before_handle_connection, (&param));
  return ret;
}

/**
 * This hook MUST be invoked before ANY recovery action is started.
 *
 * @return 0 on success, >0 otherwise.
 */
int Server_state_delegate::before_recovery(THD *) {
  DBUG_TRACE;
  Server_state_param param;

  int ret = 0;
  FOREACH_OBSERVER(ret, before_recovery, (&param));
  return ret;
}

/**
 * This hook MUST be invoked after the recovery from the engine
 * is complete.
 *
 * @return 0 on success, >0 otherwise.
 */
int Server_state_delegate::after_engine_recovery(THD *) {
  DBUG_TRACE;
  Server_state_param param;

  int ret = 0;
  FOREACH_OBSERVER(ret, after_engine_recovery, (&param));
  return ret;
}

/**
 * This hook MUST be invoked after the server has completed the
 * local recovery. The server can proceed with the further operations
 * like engaging in distributed recovery etc.
 *
 * @return 0 on success, >0 otherwise.
 */
int Server_state_delegate::after_recovery(THD *) {
  DBUG_TRACE;
  Server_state_param param;

  int ret = 0;
  FOREACH_OBSERVER(ret, after_recovery, (&param));
  return ret;
}

/**
 * This hook MUST be invoked before server shutdown action is
 * initiated.
 *
 * @return 0 on success, >0 otherwise.
 */
int Server_state_delegate::before_server_shutdown(THD *) {
  DBUG_TRACE;
  Server_state_param param;

  int ret = 0;
  FOREACH_OBSERVER(ret, before_server_shutdown, (&param));
  return ret;
}

/**
 * This hook MUST be invoked after server shutdown operation is
 * complete.
 *
 * @return 0 on success, >0 otherwise.
 */
int Server_state_delegate::after_server_shutdown(THD *) {
  DBUG_TRACE;
  Server_state_param param;

  int ret = 0;
  FOREACH_OBSERVER(ret, after_server_shutdown, (&param));
  return ret;
}

/**
 * This hook MUST be invoked after upgrade from .frm to data dictionary
 *
 * @return 0 on success, >0 otherwise.
 */
int Server_state_delegate::after_dd_upgrade_from_57(THD *) {
  DBUG_TRACE;
  Server_state_param param;

  int ret = 0;
  FOREACH_OBSERVER(ret, after_dd_upgrade_from_57, (&param));
  return ret;
}

int Binlog_storage_delegate::after_sync(THD *thd, const char *log_file,
                                        my_off_t log_pos) {
  DBUG_TRACE;
  DBUG_PRINT("enter",
             ("log_file: %s, log_pos: %llu", log_file, (ulonglong)log_pos));
  Binlog_storage_param param;
  param.server_id = thd->server_id;

  DBUG_ASSERT(log_pos != 0);
  int ret = 0;
  FOREACH_OBSERVER(ret, after_sync, (&param, log_file, log_pos));

  DEBUG_SYNC(thd, "after_call_after_sync_observer");
  return ret;
}

int Binlog_transmit_delegate::transmit_start(THD *thd, ushort flags,
                                             const char *log_file,
                                             my_off_t log_pos,
                                             bool *observe_transmission) {
  Binlog_transmit_param param;
  param.flags = flags;
  param.server_id = thd->server_id;
  param.host_or_ip = thd->security_context()->host_or_ip().str;

  int ret = 0;
  FOREACH_OBSERVER(ret, transmit_start, (&param, log_file, log_pos));
  *observe_transmission = param.should_observe();
  return ret;
}

int Binlog_transmit_delegate::transmit_stop(THD *thd, ushort flags) {
  Binlog_transmit_param param;
  param.flags = flags;
  param.server_id = thd->server_id;
  param.host_or_ip = thd->security_context()->host_or_ip().str;

  DBUG_EXECUTE_IF("crash_binlog_transmit_hook", DBUG_SUICIDE(););

  int ret = 0;
  FOREACH_OBSERVER(ret, transmit_stop, (&param));
  return ret;
}

int Binlog_transmit_delegate::reserve_header(THD *thd, ushort flags,
                                             String *packet) {
/* NOTE2ME: Maximum extra header size for each observer, I hope 32
   bytes should be enough for each Observer to reserve their extra
   header. If later found this is not enough, we can increase this
   /HEZX
*/
#define RESERVE_HEADER_SIZE 32
  unsigned char header[RESERVE_HEADER_SIZE];
  ulong hlen;
  Binlog_transmit_param param;
  param.flags = flags;
  param.server_id = thd->server_id;
  param.host_or_ip = thd->security_context()->host_or_ip().str;

  DBUG_EXECUTE_IF("crash_binlog_transmit_hook", DBUG_SUICIDE(););

  int ret = 0;
  read_lock();
  Observer_info_iterator iter = observer_info_iter();
  Observer_info *info = iter++;
  for (; info; info = iter++) {
    plugin_ref plugin = my_plugin_lock(thd, &info->plugin);
    if (!plugin) {
      ret = 1;
      break;
    }
    hlen = 0;
    if (((Observer *)info->observer)->reserve_header &&
        ((Observer *)info->observer)
            ->reserve_header(&param, header, RESERVE_HEADER_SIZE, &hlen)) {
      ret = 1;
      plugin_unlock(thd, plugin);
      break;
    }
    plugin_unlock(thd, plugin);
    if (hlen == 0) continue;
    if (hlen > RESERVE_HEADER_SIZE || packet->append((char *)header, hlen)) {
      ret = 1;
      break;
    }
  }
  unlock();
  return ret;
}

int Binlog_transmit_delegate::before_send_event(THD *thd, ushort flags,
                                                String *packet,
                                                const char *log_file,
                                                my_off_t log_pos) {
  Binlog_transmit_param param;
  param.flags = flags;
  param.server_id = thd->server_id;
  param.host_or_ip = thd->security_context()->host_or_ip().str;

  DBUG_EXECUTE_IF("crash_binlog_transmit_hook", DBUG_SUICIDE(););

  int ret = 0;
  FOREACH_OBSERVER(
      ret, before_send_event,
      (&param, pointer_cast<uchar *>(packet->ptr()), packet->length(),
       log_file + dirname_length(log_file), log_pos));
  return ret;
}

int Binlog_transmit_delegate::after_send_event(THD *thd, ushort flags,
                                               String *packet,
                                               const char *skipped_log_file,
                                               my_off_t skipped_log_pos) {
  Binlog_transmit_param param;
  param.flags = flags;
  param.server_id = thd->server_id;
  param.host_or_ip = thd->security_context()->host_or_ip().str;

  DBUG_EXECUTE_IF("crash_binlog_transmit_hook", DBUG_SUICIDE(););

  int ret = 0;
  FOREACH_OBSERVER(
      ret, after_send_event,
      (&param, packet->ptr(), packet->length(),
       skipped_log_file + dirname_length(skipped_log_file), skipped_log_pos));
  return ret;
}

int Binlog_transmit_delegate::after_reset_master(THD *thd, ushort flags)

{
  Binlog_transmit_param param;
  param.flags = flags;
  param.server_id = thd->server_id;
  param.host_or_ip = thd->security_context()->host_or_ip().str;

  int ret = 0;
  FOREACH_OBSERVER(ret, after_reset_master, (&param));
  return ret;
}

void Binlog_relay_IO_delegate::init_param(Binlog_relay_IO_param *param,
                                          Master_info *mi) {
  param->mysql = mi->mysql;
  param->channel_name = mi->get_channel();
  param->user = const_cast<char *>(mi->get_user());
  param->host = mi->host;
  param->port = mi->port;
  param->master_log_name = const_cast<char *>(mi->get_master_log_name());
  param->master_log_pos = mi->get_master_log_pos();
}

int Binlog_relay_IO_delegate::thread_start(THD *thd, Master_info *mi) {
  Binlog_relay_IO_param param;
  init_param(&param, mi);
  param.server_id = thd->server_id;
  param.thread_id = thd->thread_id();

  int ret = 0;
  FOREACH_OBSERVER(ret, thread_start, (&param));
  return ret;
}

int Binlog_relay_IO_delegate::thread_stop(THD *thd, Master_info *mi) {
  Binlog_relay_IO_param param;
  init_param(&param, mi);
  param.server_id = thd->server_id;
  param.thread_id = thd->thread_id();

  int ret = 0;
  FOREACH_OBSERVER(ret, thread_stop, (&param));
  return ret;
}

int Binlog_relay_IO_delegate::applier_start(THD *thd, Master_info *mi) {
  Binlog_relay_IO_param param;
  init_param(&param, mi);
  param.server_id = thd->server_id;
  param.thread_id = thd->thread_id();

  int ret = 0;
  FOREACH_OBSERVER(ret, applier_start, (&param));
  return ret;
}

int Binlog_relay_IO_delegate::applier_stop(THD *thd, Master_info *mi,
                                           bool aborted) {
  Binlog_relay_IO_param param;
  init_param(&param, mi);
  param.server_id = thd->server_id;
  param.thread_id = thd->thread_id();

  int ret = 0;
  FOREACH_OBSERVER(ret, applier_stop, (&param, aborted));
  return ret;
}

int Binlog_relay_IO_delegate::before_request_transmit(THD *thd, Master_info *mi,
                                                      ushort flags) {
  Binlog_relay_IO_param param;
  init_param(&param, mi);
  param.server_id = thd->server_id;
  param.thread_id = thd->thread_id();

  int ret = 0;
  FOREACH_OBSERVER(ret, before_request_transmit, (&param, (uint32)flags));
  return ret;
}

int Binlog_relay_IO_delegate::after_read_event(THD *thd, Master_info *mi,
                                               const char *packet, ulong len,
                                               const char **event_buf,
                                               ulong *event_len) {
  Binlog_relay_IO_param param;
  init_param(&param, mi);
  param.server_id = thd->server_id;
  param.thread_id = thd->thread_id();

  int ret = 0;
  FOREACH_OBSERVER(ret, after_read_event,
                   (&param, packet, len, event_buf, event_len));
  return ret;
}

int Binlog_relay_IO_delegate::after_queue_event(THD *thd, Master_info *mi,
                                                const char *event_buf,
                                                ulong event_len, bool synced) {
  Binlog_relay_IO_param param;
  init_param(&param, mi);
  param.server_id = thd->server_id;
  param.thread_id = thd->thread_id();

  uint32 flags = 0;
  if (synced) flags |= BINLOG_STORAGE_IS_SYNCED;

  int ret = 0;
  FOREACH_OBSERVER(ret, after_queue_event,
                   (&param, event_buf, event_len, flags));
  return ret;
}

int Binlog_relay_IO_delegate::after_reset_slave(THD *thd, Master_info *mi)

{
  Binlog_relay_IO_param param;
  init_param(&param, mi);
  param.server_id = thd->server_id;
  param.thread_id = thd->thread_id();

  int ret = 0;
  FOREACH_OBSERVER(ret, after_reset_slave, (&param));
  return ret;
}

int Binlog_relay_IO_delegate::applier_log_event(THD *thd, int &out) {
  DBUG_TRACE;
  Trans_param trans_param;
  TRANS_PARAM_ZERO(trans_param);
  Binlog_relay_IO_param param;

  param.server_id = thd->server_id;
  param.thread_id = thd->thread_id();

  prepare_table_info(thd, trans_param.tables_info,
                     trans_param.number_of_tables);

  int ret = 0;
  FOREACH_OBSERVER(ret, applier_log_event, (&param, &trans_param, out));

  my_free(trans_param.tables_info);

  return ret;
}

int Raft_replication_delegate::before_flush(THD *thd, IO_CACHE *io_cache,
                                            RaftReplicateMsgOpType op_type) {
  DBUG_ENTER("Raft_replication_delegate::before_flush");
  Raft_replication_param param;

  int ret = 0;

  FOREACH_OBSERVER_STRICT(ret, before_flush, (&param, io_cache, op_type));

  DBUG_PRINT("return", ("term: %ld, index: %ld", param.term, param.index));

  /* (term, index) will be used later in before_commit hook of trans
   * observer */
  thd->set_trans_marker(param.term, param.index);

  DBUG_RETURN(ret);
}

int Raft_replication_delegate::before_commit(THD *thd) {
  DBUG_ENTER("Raft_replications_delegate::before_commit");
  Raft_replication_param param;

  thd->get_trans_marker(&param.term, &param.index);

  DBUG_PRINT("enter", ("term: %ld, index: %ld", param.term, param.index));

  int ret = 0;
  FOREACH_OBSERVER_STRICT(ret, before_commit, (&param));

  DEBUG_SYNC(thd, "after_call_after_sync_observer");
  DBUG_RETURN(ret);
}

int Raft_replication_delegate::setup_flush(
    THD * /* thd */, Raft_replication_observer::st_setup_flush_arg *arg) {
  DBUG_ENTER("Raft_replication_delegate::setup_flush");
  int ret = 0;

  FOREACH_OBSERVER_STRICT(ret, setup_flush, (arg));

  DBUG_RETURN(ret);
}

int Raft_replication_delegate::before_shutdown(THD * /* thd */) {
  DBUG_ENTER("Raft_replication_delegate::before_shutdown");
  int ret = 0;

  FOREACH_OBSERVER_STRICT(ret, before_shutdown, ());

  DBUG_RETURN(ret);
}

int Raft_replication_delegate::register_paths(
    THD * /* thd */, const std::string &s_uuid, uint32_t server_id,
    const std::string &wal_dir_parent, const std::string &log_dir_parent,
    const std::string &raft_log_path_prefix, const std::string &s_hostname,
    uint64_t port) {
  DBUG_ENTER("Raft_replication_delegate::register_paths");
  int ret = 0;

  FOREACH_OBSERVER_STRICT(
      ret, register_paths,
      (&raft_listener_queue, s_uuid, server_id, wal_dir_parent, log_dir_parent,
       raft_log_path_prefix, s_hostname, port));

  DBUG_RETURN(ret);
}

int Raft_replication_delegate::after_commit(THD *thd) {
  DBUG_ENTER("Raft_replication_delegate::after_commit");
  Raft_replication_param param;

  thd->get_trans_marker(&param.term, &param.index);

  const char *file = nullptr;
  my_off_t pos = 0;
  if (mysql_bin_log.is_apply_log)
    thd->get_trans_relay_log_pos(&file, &pos);
  else
    thd->get_trans_fixed_pos(&file, &pos);

  int ret = 0;
  FOREACH_OBSERVER_STRICT(ret, after_commit, (&param));
  DBUG_RETURN(ret);
}

int Raft_replication_delegate::purge_logs(THD *thd, uint64_t file_ext) {
  DBUG_ENTER("Raft_replication_delegate::purge_logs");
  Raft_replication_param param;
  param.purge_file_ext = file_ext;
  int ret = 0;
  FOREACH_OBSERVER_STRICT(ret, purge_logs, (&param));

  // Set the safe purge file that was sent back by the plugin
  thd->set_safe_purge_file(param.purge_file);

  DBUG_RETURN(ret);
}

int Raft_replication_delegate::show_raft_status(
    THD * /* thd */,
    std::vector<std::pair<std::string, std::string>> *var_value_pairs) {
  DBUG_ENTER("Raft_replication_delegate::show_raft_status");
  Raft_replication_param param;
  int ret = 0;
  FOREACH_OBSERVER_STRICT(ret, show_raft_status, (var_value_pairs));
  DBUG_RETURN(ret);
}

int register_trans_observer(Trans_observer *observer, void *p) {
  return transaction_delegate->add_observer(observer, (st_plugin_int *)p);
}

int unregister_trans_observer(Trans_observer *observer, void *) {
  return transaction_delegate->remove_observer(observer);
}

int register_binlog_storage_observer(Binlog_storage_observer *observer,
                                     void *p) {
  DBUG_TRACE;
  int result =
      binlog_storage_delegate->add_observer(observer, (st_plugin_int *)p);
  return result;
}

int unregister_binlog_storage_observer(Binlog_storage_observer *observer,
                                       void *) {
  return binlog_storage_delegate->remove_observer(observer);
}

int register_server_state_observer(Server_state_observer *observer,
                                   void *plugin_var) {
  DBUG_TRACE;
  int result = server_state_delegate->add_observer(observer,
                                                   (st_plugin_int *)plugin_var);
  return result;
}

int unregister_server_state_observer(Server_state_observer *observer, void *) {
  DBUG_TRACE;
  int result = server_state_delegate->remove_observer(observer);
  return result;
}

int register_binlog_transmit_observer(Binlog_transmit_observer *observer,
                                      void *p) {
  return binlog_transmit_delegate->add_observer(observer, (st_plugin_int *)p);
}

int unregister_binlog_transmit_observer(Binlog_transmit_observer *observer,
                                        void *) {
  return binlog_transmit_delegate->remove_observer(observer);
}

int register_binlog_relay_io_observer(Binlog_relay_IO_observer *observer,
                                      void *p) {
  return binlog_relay_io_delegate->add_observer(observer, (st_plugin_int *)p);
}

int unregister_binlog_relay_io_observer(Binlog_relay_IO_observer *observer,
                                        void *) {
  return binlog_relay_io_delegate->remove_observer(observer);
}

int register_raft_replication_observer(Raft_replication_observer *observer,
                                       void *p) {
  DBUG_ENTER("register_raft_replication_observer");
  raft_listener_queue.init();
  int result =
      raft_replication_delegate->add_observer(observer, (st_plugin_int *)p);
  DBUG_RETURN(result);
}

int unregister_raft_replication_observer(Raft_replication_observer *observer,
                                         void *) {
  raft_listener_queue.deinit();
  return raft_replication_delegate->remove_observer(observer);
}

int launch_hook_trans_begin(THD *thd, TABLE_LIST *all_tables) {
  DBUG_TRACE;
  LEX *lex = thd->lex;
  enum_sql_command sql_command = lex->sql_command;
  // by default commands are put on hold
  bool hold_command = true;
  int ret = 0;

  // if command belong to a transaction that already pass by hook, it can
  // continue
  if (thd->get_transaction()->was_trans_begin_hook_invoked()) {
    return 0;
  }

  bool is_show = ((sql_command_flags[sql_command] & CF_STATUS_COMMAND) &&
                  (sql_command != SQLCOM_BINLOG_BASE64_EVENT)) ||
                 (sql_command == SQLCOM_SHOW_RELAYLOG_EVENTS);
  bool is_set = (sql_command == SQLCOM_SET_OPTION);
  bool is_select = (sql_command == SQLCOM_SELECT);
  bool is_do = (sql_command == SQLCOM_DO);
  bool is_empty = (sql_command == SQLCOM_EMPTY_QUERY);
  bool is_use = (sql_command == SQLCOM_CHANGE_DB);
  bool is_stop_gr = (sql_command == SQLCOM_STOP_GROUP_REPLICATION);
  bool is_shutdown = (sql_command == SQLCOM_SHUTDOWN);
  bool is_reset_persist =
      (sql_command == SQLCOM_RESET && lex->option_type == OPT_PERSIST);

  if ((is_set || is_do || is_show || is_empty || is_use || is_stop_gr ||
       is_shutdown || is_reset_persist) &&
      !lex->uses_stored_routines()) {
    return 0;
  }

  if (is_select) {
    bool is_udf = false;

    // if select is an udf function
    SELECT_LEX *select_lex_elem = lex->unit->first_select();
    while (select_lex_elem != nullptr) {
      Item *item;
      List_iterator_fast<Item> it(select_lex_elem->fields_list);
      while ((item = it++)) {
        if (item->type() == Item::FUNC_ITEM) {
          Item_func *func_item = down_cast<Item_func *>(item);
          Item_func::Functype functype = func_item->functype();
          if (functype == Item_func::FUNC_SP || functype == Item_func::UDF_FUNC)
            is_udf = true;
        }
      }
      select_lex_elem = select_lex_elem->next_select();
    }

    if (!is_udf && all_tables == nullptr) {
      // SELECT that don't use tables and isn't a UDF
      hold_command = false;
    }

    if (hold_command && all_tables != nullptr) {
      // SELECT that use tables
      bool is_perf_schema_table = false;
      bool is_process_list = false;
      bool is_sys_db = false;
      bool stop_db_check = false;

      for (TABLE_LIST *table = all_tables; table && !stop_db_check;
           table = table->next_global) {
        DBUG_ASSERT(table->db && table->table_name);

        if (is_perfschema_db(table->db, table->db_length))
          is_perf_schema_table = true;
        else if (is_infoschema_db(table->db, table->db_length) &&
                 !my_strcasecmp(system_charset_info, "PROCESSLIST",
                                table->table_name)) {
          is_process_list = true;
        } else if (table->db_length == 3 &&
                   !my_strcasecmp(system_charset_info, "sys", table->db)) {
          is_sys_db = true;
        } else {
          is_perf_schema_table = false;
          is_process_list = false;
          is_sys_db = false;
          stop_db_check = true;
        }
      }

      if (is_process_list || is_perf_schema_table || is_sys_db) {
        hold_command = false;
      }
    }
  }

  if (hold_command) {
    DBUG_EXECUTE_IF("launch_hook_trans_begin_assert_if_hold",
                    { DBUG_ASSERT(0); };);

    PSI_stage_info old_stage;
    thd->enter_stage(&stage_hook_begin_trans, &old_stage, __func__, __FILE__,
                     __LINE__);

    if (opt_group_replication_plugin_hooks) {
      RUN_HOOK(transaction, trans_begin, (thd, ret));
    }

    THD_STAGE_INFO(thd, old_stage);
    if (!ret && (sql_command == SQLCOM_BEGIN ||
                 thd->in_active_multi_stmt_transaction())) {
      thd->get_transaction()->set_trans_begin_hook_invoked();
    }
  }

  return ret;
}

static int update_sys_var(const char *var_name, uint name_len,
                          Item &update_item) {
  // find_sys_var will take a read lock on LOCK_system_variables_hash
  sys_var *sys_var_ptr = find_sys_var(current_thd, var_name, name_len);
  if (sys_var_ptr) {
    LEX_CSTRING tmp;
    set_var set_v(OPT_GLOBAL, sys_var_ptr, tmp, &update_item);
    return !set_v.check(current_thd) && set_v.update(current_thd);
  }

  return 1;
}

static int handle_read_only(
    const std::map<std::string, unsigned int> &sys_var_map) {
  int error = 0;
  auto read_only_it = sys_var_map.find("read_only");
  auto super_read_only_it = sys_var_map.find("super_read_only");
  if (read_only_it == sys_var_map.end() &&
      super_read_only_it == sys_var_map.end())
    return 1;

  if (super_read_only_it != sys_var_map.end() &&
      super_read_only_it->second == 1) {
    // Case 1: set super_read_only=1. This will implicitly set read_only.
    Item_uint super_read_only_item(super_read_only_it->second);
    error = update_sys_var(STRING_WITH_LEN("super_read_only"),
                           super_read_only_item);
  } else if (read_only_it != sys_var_map.end() && read_only_it->second == 0) {
    // Case 2: set read_only=0. This will implicitly unset super_read_only.
    Item_uint read_only_item(read_only_it->second);
    error = update_sys_var(STRING_WITH_LEN("read_only"), read_only_item);
  } else {
    // Case 3: Need to set read_only=1 OR/AND set super_read_only=0
    if (read_only_it != sys_var_map.end()) {
      Item_uint read_only_item(read_only_it->second);
      error = update_sys_var(STRING_WITH_LEN("read_only"), read_only_item);
    }

    if (!error && super_read_only_it != sys_var_map.end()) {
      Item_uint super_read_only_item(super_read_only_it->second);
      error = update_sys_var(STRING_WITH_LEN("super_read_only"),
                             super_read_only_item);
    }
  }

  return error;
}

static int set_durability(
    const std::map<std::string, unsigned int> &durability) {
  // sync_binlog
  const auto sync_binlog_it = durability.find("sync_binlog");
  if (sync_binlog_it == durability.end()) {
    return 1;
  }
  Item_uint sync_binlog_item(sync_binlog_it->second);
  int sync_binlog_update =
      update_sys_var(STRING_WITH_LEN("sync_binlog"), sync_binlog_item);
  if (sync_binlog_update)  // failed
    return sync_binlog_update;

  // innodb_flush_log_at_trx_commit might not always update since innodb
  // might not be enabled
  const auto flush_log_it = durability.find("innodb_flush_log_at_trx_commit");
  if (flush_log_it != durability.end()) {
    Item_uint flush_log_item(flush_log_it->second);
    int innodb_flush_log_at_trx_commit_update = update_sys_var(
        STRING_WITH_LEN("innodb_flush_log_at_trx_commit"), flush_log_item);
    if (innodb_flush_log_at_trx_commit_update)  // failed
      return innodb_flush_log_at_trx_commit_update;
  }

  // innodb_doublewrite not always update since innodb might not be enabled
  const auto doublewrite_it = durability.find("innodb_doublewrite");
  if (doublewrite_it != durability.end()) {
    Item_uint doublewrite_item(doublewrite_it->second);
    int innodb_doublewrite_update =
        update_sys_var(STRING_WITH_LEN("innodb_doublewrite"), doublewrite_item);
    if (innodb_doublewrite_update)  // failed
      return innodb_doublewrite_update;
  }

  return 0;
}

extern "C" void *process_raft_queue(void *) {
  bool thd_added = false;

  Global_THD_manager *thd_manager = Global_THD_manager::get_instance();
  /* Setup this thread */
  my_thread_init();
  THD *thd = new THD;
  thd->thread_stack = (char *)&thd;
  thd->store_globals();
  thd->security_context()->skip_grants();
  thd->set_new_thread_id();
  thd_manager->add_thd(thd);
  thd_added = true;

  /* Start listening for new events in the queue */
  bool exit = false;
  // The exit is triggered by the raft plugin gracefully
  // enqueing an exit function for this thread.
  // if we listen to abort_loop or thd->killed
  // then we might not give the plugin the opportunity to
  // do some critical tasks before exit
  while (!exit) {
    thd->get_stmt_da()->reset_diagnostics_area();
    RaftListenerQueue::QueueElement element = raft_listener_queue.get();
    RaftListenerCallbackResult result;
    DBUG_PRINT("info",
               ("process_raft_queue: %d\n", static_cast<int>(element.type)));
    switch (element.type) {
      case RaftListenerCallbackType::SET_READ_ONLY: {
        handle_read_only(element.arg.val_sys_var_uint);
        break;
      }
      case RaftListenerCallbackType::TRIM_LOGGED_GTIDS: {
        trim_logged_gtid(element.arg.trim_gtids);
        break;
      }
      case RaftListenerCallbackType::ROTATE_BINLOG: {
        result.error = rotate_binlog_file(current_thd);
        break;
      }
      case RaftListenerCallbackType::ROTATE_RELAYLOG: {
        RaftRotateInfo raft_rotate_info;
        raft_rotate_info.new_log_ident = element.arg.log_file_pos.first;
        raft_rotate_info.pos = element.arg.log_file_pos.second;
        myf flags = MYF(element.arg.val_uint);
        raft_rotate_info.noop = flags & RaftListenerQueue::RAFT_FLAGS_NOOP;
        raft_rotate_info.post_append =
            flags & RaftListenerQueue::RAFT_FLAGS_POSTAPPEND;
        raft_rotate_info.rotate_opid = element.arg.val_opid;
        result.error = rotate_relay_log_for_raft(&raft_rotate_info);
        break;
      }
      case RaftListenerCallbackType::RAFT_LISTENER_THREADS_EXIT:
        exit = true;
        result.error = 0;
        break;
      case RaftListenerCallbackType::RLI_RELAY_LOG_RESET: {
        result.error =
            rli_relay_log_raft_reset(element.arg.log_file_pos, current_thd);
        break;
      }
      case RaftListenerCallbackType::RESET_SLAVE: {
        result.error = raft_reset_slave(current_thd);
        // When resetting a slave we also want to clear the read-only message
        // since we can't make assumptions on the master instance anymore
        if (!result.error) {
          Item_string item("", 0, current_thd->charset());
          result.error = update_sys_var(
              STRING_WITH_LEN("read_only_error_msg_extra"), item);
        }
        break;
      }

      case RaftListenerCallbackType::BINLOG_CHANGE_TO_APPLY: {
        result.error = binlog_change_to_apply();
        break;
      }
      case RaftListenerCallbackType::BINLOG_CHANGE_TO_BINLOG: {
        result.error = binlog_change_to_binlog(current_thd);
        break;
      }
      case RaftListenerCallbackType::STOP_SQL_THREAD: {
        result.error = raft_stop_sql_thread(current_thd);
        break;
      }
      case RaftListenerCallbackType::START_SQL_THREAD: {
        result.error = raft_start_sql_thread(current_thd);
        break;
      }
      case RaftListenerCallbackType::STOP_IO_THREAD: {
        result.error = raft_stop_io_thread(current_thd);
        break;
      }
      case RaftListenerCallbackType::CHANGE_MASTER: {
        result.error = raft_change_master(
            current_thd, element.arg.master_instance, element.arg.master_uuid);
        if (!result.error && !element.arg.val_str.empty()) {
          Item_string item(element.arg.val_str.c_str(),
                           element.arg.val_str.length(),
                           current_thd->charset());
          result.error = update_sys_var(
              STRING_WITH_LEN("read_only_error_msg_extra"), item);
        }
        break;
      }

      case RaftListenerCallbackType::GET_COMMITTED_GTIDS: {
        result.error =
            get_committed_gtids(element.arg.trim_gtids, &result.gtids);
        break;
      }

      case RaftListenerCallbackType::GET_EXECUTED_GTIDS: {
        char *buffer;
        global_sid_lock->wrlock();
        gtid_state->get_executed_gtids()->to_string(&buffer);
        global_sid_lock->unlock();
        result.val_str = std::string(buffer);
        result.error = 0;
        my_free(buffer);
        break;
      }
      case RaftListenerCallbackType::SET_BINLOG_DURABILITY: {
        result.error = set_durability(element.arg.val_sys_var_uint);
        break;
      }
      case RaftListenerCallbackType::RAFT_CONFIG_CHANGE: {
        result.error = raft_config_change(std::move(element.arg.val_str));
        break;
      }
      case RaftListenerCallbackType::RAFT_UPDATE_FOLLOWER_INFO: {
        result.error = raft_update_follower_info(element.arg.val_str_map,
                                                 element.arg.val_bool,
                                                 element.arg.is_shutdown);
        break;
      }
      case RaftListenerCallbackType::HANDLE_DUMP_THREADS: {
        result.error = handle_dump_threads(element.arg.val_bool);
        break;
      }
      default:
        // placate the compiler
        result.error = 0;
    }

    // Fulfill the promise (if requested)
    if (element.result) element.result->set_value(std::move(result));
  }

  // Cleanup and exit
  thd->release_resources();
  if (thd_added) thd_manager->remove_thd(thd);
  delete thd;
  my_thread_end();
  // NO_LINT_DEBUG
  sql_print_information("Raft listener queue aborted");
  pthread_exit(0);
  return 0;
}

int start_raft_listener_thread() {
  my_thread_handle th;
  if ((mysql_thread_create(0, &th, &connection_attrib, process_raft_queue,
                           (void *)0))) {
    // NO_LINT_DEBUG
    sql_print_error("Could not create raft_listener_thread");
    return 1;
  }

  return 0;
}

RaftListenerQueue::~RaftListenerQueue() { deinit(); }

int RaftListenerQueue::add(QueueElement element) {
  std::unique_lock<std::mutex> lock(queue_mutex_);
  if (!inited_) {
    // NO_LINT_DEBUG
    sql_print_error("Raft listener queue and thread is not inited");
    return 1;
  }

  queue_.emplace(std::move(element));
  lock.unlock();
  queue_cv_.notify_all();

  return 0;
}

RaftListenerQueue::QueueElement RaftListenerQueue::get() {
  // Wait for something to be put into the event queue
  std::unique_lock<std::mutex> lock(queue_mutex_);
  while (queue_.empty()) queue_cv_.wait(lock);

  QueueElement element = queue_.front();
  queue_.pop();

  return element;
}

int RaftListenerQueue::init() {
  // NO_LINT_DEBUG
  sql_print_information("Initializing Raft listener queue");
  std::unique_lock<std::mutex> lock(init_mutex_);
  if (inited_) return 0;  // Already inited

  if (start_raft_listener_thread()) return 1;  // Fails to initialize

  inited_ = true;
  return 0;  // Initialization success
}

void RaftListenerQueue::deinit() {
  // NO_LINT_DEBUG
  std::unique_lock<std::mutex> lock(init_mutex_);
  if (!inited_) return;

  fprintf(stderr, "Shutting down Raft listener queue");
  // Queue an exit event in the queue. The listener thread will eventually pick
  // this up and exit
  std::promise<RaftListenerCallbackResult> prms;
  auto fut = prms.get_future();
  QueueElement element;
  element.type = RaftListenerCallbackType::RAFT_LISTENER_THREADS_EXIT;
  element.result = &prms;
  add(element);
  fut.get();
  inited_ = false;
  return;
}
