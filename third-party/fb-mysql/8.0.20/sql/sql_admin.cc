/* Copyright (c) 2010, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/sql_admin.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <string>
#include <utility>

#include <sql/ssl_acceptor_context.h>
#include "keycache.h"
#include "m_string.h"
#include "my_base.h"
#include "my_dbug.h"
#include "my_dir.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_macros.h"
#include "my_sys.h"
#include "myisam.h"  // TT_USEFRM
#include "mysql/components/services/log_builtins.h"
#include "mysql/psi/mysql_file.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"  // *_ACL
#include "sql/auth/sql_security_ctx.h"
#include "sql/clone_handler.h"
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client
#include "sql/dd/dd_table.h"                 // dd::recreate_table
#include "sql/dd/impl/sdi_utils.h"           // mdl_lock
#include "sql/dd/info_schema/table_stats.h"  // dd::info_schema::update_*
#include "sql/dd/string_type.h"              // dd::String_type
#include "sql/dd/types/abstract_table.h"     // dd::enum_table_type
#include "sql/dd/types/table.h"              // dd::Table
#include "sql/debug_sync.h"                  // DEBUG_SYNC
#include "sql/derror.h"                      // ER_THD
#include "sql/handler.h"
#include "sql/histograms/histogram.h"
#include "sql/item.h"
#include "sql/key.h"
#include "sql/keycaches.h"  // get_key_cache
#include "sql/lock.h"       // acquire_shared_global_read_lock()
#include "sql/log.h"
#include "sql/log_event.h"
#include "sql/mdl.h"
#include "sql/mysqld.h"             // key_file_misc
#include "sql/partition_element.h"  // PART_ADMIN
#include "sql/protocol.h"
#include "sql/protocol_classic.h"
#include "sql/rpl_group_replication.h"  // is_group_replication_running
#include "sql/rpl_gtid.h"
#include "sql/rpl_slave_commit_order_manager.h"  // Commit_order_manager
#include "sql/sp.h"                              // Sroutine_hash_entry
#include "sql/sp_rcontext.h"                     // sp_rcontext
#include "sql/sql_alter.h"
#include "sql/sql_alter_instance.h"  // Alter_instance
#include "sql/sql_backup_lock.h"     // acquire_shared_backup_lock
#include "sql/sql_base.h"            // Open_table_context
#include "sql/sql_class.h"           // THD
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_parse.h"      // check_table_access
#include "sql/sql_partition.h"  // set_part_state
#include "sql/sql_prepare.h"    // mysql_test_show
#include "sql/sql_table.h"      // mysql_recreate_table
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/table_trigger_dispatcher.h"  // Table_trigger_dispatcher
#include "sql/thd_raii.h"
#include "sql/transaction.h"  // trans_rollback_stmt
#include "sql_string.h"
#include "thr_lock.h"
#include "violite.h"

bool Column_name_comparator::operator()(const String *lhs,
                                        const String *rhs) const {
  DBUG_ASSERT(lhs->charset()->number == rhs->charset()->number);
  return sortcmp(lhs, rhs, lhs->charset()) < 0;
}

static int send_check_errmsg(THD *thd, TABLE_LIST *table,
                             const char *operator_name, const char *errmsg)

{
  Protocol *protocol = thd->get_protocol();
  protocol->start_row();
  protocol->store(table->alias, system_charset_info);
  protocol->store(operator_name, system_charset_info);
  protocol->store_string(STRING_WITH_LEN("error"), system_charset_info);
  protocol->store(errmsg, system_charset_info);
  thd->clear_error();
  if (protocol->end_row()) return -1;
  return 1;
}

static int prepare_for_repair(THD *thd, TABLE_LIST *table_list,
                              HA_CHECK_OPT *check_opt) {
  int error = 0;
  TABLE tmp_table, *table;
  TABLE_SHARE *share;
  bool has_mdl_lock = false;
  char from[FN_REFLEN], tmp[FN_REFLEN + 32];
  const char **ext;
  MY_STAT stat_info;
  Open_table_context ot_ctx(thd,
                            (MYSQL_OPEN_IGNORE_FLUSH | MYSQL_OPEN_HAS_MDL_LOCK |
                             MYSQL_LOCK_IGNORE_TIMEOUT));
  DBUG_TRACE;

  if (!(check_opt->sql_flags & TT_USEFRM)) return 0;

  if (!(table = table_list->table)) {
    const char *key;
    size_t key_length;
    /*
      If the table didn't exist, we have a shared metadata lock
      on it that is left from mysql_admin_table()'s attempt to
      open it. Release the shared metadata lock before trying to
      acquire the exclusive lock to satisfy MDL asserts and avoid
      deadlocks.
    */
    thd->mdl_context.release_transactional_locks();
    /*
      Attempt to do full-blown table open in mysql_admin_table() has failed.
      Let us try to open at least a .FRM for this table.
    */
    MDL_REQUEST_INIT(&table_list->mdl_request, MDL_key::TABLE, table_list->db,
                     table_list->table_name, MDL_EXCLUSIVE, MDL_TRANSACTION);

    if (lock_table_names_nsec(thd, table_list, table_list->next_global,
                              thd->variables.lock_wait_timeout_nsec, 0))
      return 0;
    has_mdl_lock = true;

    key_length = get_table_def_key(table_list, &key);

    mysql_mutex_lock(&LOCK_open);
    share = get_table_share(thd, table_list->db, table_list->table_name, key,
                            key_length, false);
    mysql_mutex_unlock(&LOCK_open);
    if (share == nullptr) return 0;  // Can't open frm file

    if (open_table_from_share(thd, share, "", 0, 0, 0, &tmp_table, false,
                              nullptr)) {
      mysql_mutex_lock(&LOCK_open);
      release_table_share(share);
      mysql_mutex_unlock(&LOCK_open);
      return 0;  // Out of memory
    }
    table = &tmp_table;
  }

  /*
    REPAIR TABLE ... USE_FRM for temporary tables makes little sense.
  */
  if (table->s->tmp_table) {
    error = send_check_errmsg(thd, table_list, "repair",
                              "Cannot repair temporary table from .frm file");
    goto end;
  }

  /*
    Check if this is a table type that stores index and data separately,
    like ISAM or MyISAM. We assume fixed order of engine file name
    extentions array. First element of engine file name extentions array
    is meta/index file extention. Second element - data file extention.
  */
  ext = table->file->ht->file_extensions;
  if (!ext || !ext[0] || !ext[1]) goto end;  // No data file

  /* A MERGE table must not come here. */
  DBUG_ASSERT(table->file->ht->db_type != DB_TYPE_MRG_MYISAM);

  /*
    Storage engines supporting atomic DDL do not come here either.

    If we are to have storage engine which supports atomic DDL on one
    hand and REPAIR ... USE_FRM on another then the below code related
    to table re-creation in SE needs to be adjusted to at least
    commit the transaction.
  */
  DBUG_ASSERT(!(table->file->ht->flags & HTON_SUPPORTS_ATOMIC_DDL));

  // Name of data file
  strxmov(from, table->s->normalized_path.str, ext[1], NullS);
  if (!mysql_file_stat(key_file_misc, from, &stat_info, MYF(0)))
    goto end;  // Can't use USE_FRM flag

  snprintf(tmp, sizeof(tmp), "%s-%lx_%x", from, current_pid, thd->thread_id());

  if (table_list->table) {
    /*
      Table was successfully open in mysql_admin_table(). Now we need
      to close it, but leave it protected by exclusive metadata lock.
    */
    if (wait_while_table_is_used(thd, table, HA_EXTRA_FORCE_REOPEN)) goto end;
    close_all_tables_for_name(thd, table_list->table->s, false, nullptr);
    table_list->table = nullptr;
  }
  /*
    After this point we have an exclusive metadata lock on our table
    in both cases when table was successfully open in mysql_admin_table()
    and when it was open in prepare_for_repair().
  */

  if (my_rename(from, tmp, MYF(MY_WME))) {
    error = send_check_errmsg(thd, table_list, "repair",
                              "Failed renaming data file");
    goto end;
  }
  if (dd::recreate_table(thd, table_list->db, table_list->table_name)) {
    error = send_check_errmsg(thd, table_list, "repair",
                              "Failed generating table from .frm file");
    goto end;
  }
  if (mysql_file_rename(key_file_misc, tmp, from, MYF(MY_WME))) {
    error = send_check_errmsg(thd, table_list, "repair",
                              "Failed restoring .MYD file");
    goto end;
  }

  if (thd->locked_tables_list.reopen_tables(thd)) goto end;

  /*
    Now we should be able to open the partially repaired table
    to finish the repair in the handler later on.
  */
  if (open_table(thd, table_list, &ot_ctx)) {
    error = send_check_errmsg(thd, table_list, "repair",
                              "Failed to open partially repaired table");
    goto end;
  }

end:
  thd->locked_tables_list.unlink_all_closed_tables(thd, nullptr, 0);
  if (table == &tmp_table) {
    mysql_mutex_lock(&LOCK_open);
    closefrm(table, true);  // Free allocated memory
    mysql_mutex_unlock(&LOCK_open);
  }
  /* In case of a temporary table there will be no metadata lock. */
  if (error && has_mdl_lock) thd->mdl_context.release_transactional_locks();

  return error;
}

/**
  Check if a given error is something that could occur during
  open_and_lock_tables() that does not indicate table corruption.

  @param  sql_errno  Error number to check.

  @retval true       Error does not indicate table corruption.
  @retval false      Error could indicate table corruption.
*/

static inline bool table_not_corrupt_error(uint sql_errno) {
  return (sql_errno == ER_NO_SUCH_TABLE || sql_errno == ER_FILE_NOT_FOUND ||
          sql_errno == ER_LOCK_WAIT_TIMEOUT || sql_errno == ER_LOCK_DEADLOCK ||
          sql_errno == ER_CANT_LOCK_LOG_TABLE ||
          sql_errno == ER_OPEN_AS_READONLY || sql_errno == ER_WRONG_OBJECT);
}

Sql_cmd_analyze_table::Sql_cmd_analyze_table(
    THD *thd, Alter_info *alter_info, Histogram_command histogram_command,
    int histogram_buckets)
    : Sql_cmd_ddl_table(alter_info),
      m_histogram_command(histogram_command),
      m_histogram_fields(Column_name_comparator(),
                         Mem_root_allocator<String>(thd->mem_root)),
      m_histogram_buckets(histogram_buckets) {}

bool Sql_cmd_analyze_table::drop_histogram(THD *thd, TABLE_LIST *table,
                                           histograms::results_map &results) {
  histograms::columns_set fields;

  for (const auto column : get_histogram_fields())
    fields.emplace(column->ptr(), column->length());
  return histograms::drop_histograms(thd, *table, fields, results);
}

/**
  Send any errors from the ANALYZE TABLE statement to the client.

  This function sends any errors stored in the diagnostics area as a result set
  to the client instead of a "normal" error. It will also clear the diagnostics
  area before returning.

  @param thd The thread handler.
  @param operator_name The name of the ANALYZE TABLE operation that will be
         printed in the column "Op" of the result set. This is usually either
         "analyze" or "histogram".
  @param table_name The name of the table that ANALYZE TABLE operated on.

  @retval true An error occurred while sending the result set to the client.
  @retval false The result set was sent to the client.
*/
static bool send_analyze_table_errors(THD *thd, const char *operator_name,
                                      const char *table_name) {
  Diagnostics_area::Sql_condition_iterator it =
      thd->get_stmt_da()->sql_conditions();
  const Sql_condition *err;
  Protocol *protocol = thd->get_protocol();
  while ((err = it++)) {
    protocol->start_row();
    protocol->store(table_name, system_charset_info);
    protocol->store(operator_name, system_charset_info);
    protocol->store_string(warning_level_names[err->severity()].str,
                           warning_level_names[err->severity()].length,
                           system_charset_info);
    protocol->store(err->message_text(), system_charset_info);
    if (protocol->end_row()) return true;
  }
  thd->get_stmt_da()->reset_condition_info(thd);
  return false;
}

bool Sql_cmd_analyze_table::send_histogram_results(
    THD *thd, const histograms::results_map &results, const TABLE_LIST *table) {
  Item *item;
  List<Item> field_list;

  field_list.push_back(item =
                           new Item_empty_string("Table", NAME_CHAR_LEN * 2));
  item->maybe_null = true;
  field_list.push_back(item = new Item_empty_string("Op", 10));
  item->maybe_null = true;
  field_list.push_back(item = new Item_empty_string("Msg_type", 10));
  item->maybe_null = true;
  field_list.push_back(
      item = new Item_empty_string("Msg_text", SQL_ADMIN_MSG_TEXT_SIZE));
  item->maybe_null = true;
  if (thd->send_result_metadata(&field_list,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF)) {
    return true; /* purecov: deadcode */
  }

  std::string combined_name(table->db, table->db_length);
  combined_name.append(".");
  combined_name.append(table->table_name, table->table_name_length);
  if (send_analyze_table_errors(thd, "histogram", combined_name.c_str()))
    return true;

  Protocol *protocol = thd->get_protocol();
  for (const auto &pair : results) {
    const char *table_name = combined_name.c_str();

    std::string message;
    std::string message_type;
    switch (pair.second) {
      // Status messages
      case histograms::Message::HISTOGRAM_CREATED:
        message_type.assign("status");
        message.assign("Histogram statistics created for column '");
        message.append(pair.first);
        message.append("'.");
        break;
      case histograms::Message::HISTOGRAM_DELETED:
        message_type.assign("status");
        message.assign("Histogram statistics removed for column '");
        message.append(pair.first);
        message.append("'.");
        break;
      // Errror messages
      case histograms::Message::FIELD_NOT_FOUND:
        message_type.assign("Error");
        message.assign("The column '");
        message.append(pair.first);
        message.append("' does not exist.");
        break;
      case histograms::Message::UNSUPPORTED_DATA_TYPE:
        message_type.assign("Error");
        message.assign("The column '");
        message.append(pair.first);
        message.append("' has an unsupported data type.");
        break;
      case histograms::Message::TEMPORARY_TABLE:
        message_type.assign("Error");
        message.assign(
            "Cannot create histogram statistics for a temporary table.");
        break;
      case histograms::Message::ENCRYPTED_TABLE:
        message_type.assign("Error");
        message.assign(
            "Cannot create histogram statistics for an encrypted table.");
        break;
      case histograms::Message::VIEW:
        message_type.assign("Error");
        message.assign("Cannot create histogram statistics for a view.");
        break;
      case histograms::Message::MULTIPLE_TABLES_SPECIFIED:
        message_type.assign("Error");
        message.assign(
            "Only one table can be specified while modifying histogram "
            "statistics.");
        table_name = "";
        break;
      case histograms::Message::COVERED_BY_SINGLE_PART_UNIQUE_INDEX:
        message_type.assign("Error");
        message.assign("The column '");
        message.append(pair.first);
        message.append("' is covered by a single-part unique index.");
        break;
      case histograms::Message::NO_HISTOGRAM_FOUND:
        message_type.assign("Error");
        message.assign("No histogram statistics found for column '");
        message.append(pair.first);
        message.append("'.");
        break;
      case histograms::Message::SERVER_READ_ONLY:
        message_type.assign("Error");
        message.assign("The server is in read-only mode.");
        table_name = "";
        break;
    }

    protocol->start_row();
    if (protocol->store(table_name, system_charset_info) ||
        protocol->store_string(STRING_WITH_LEN("histogram"),
                               system_charset_info) ||
        protocol->store_string(message_type.c_str(), message_type.length(),
                               system_charset_info) ||
        protocol->store_string(message.c_str(), message.size(),
                               system_charset_info) ||
        protocol->end_row()) {
      return true; /* purecov: deadcode */
    }
  }

  return false;
}

bool Sql_cmd_analyze_table::update_histogram(THD *thd, TABLE_LIST *table,
                                             histograms::results_map &results) {
  histograms::columns_set fields;

  for (const auto column : get_histogram_fields())
    fields.emplace(column->ptr(), column->length());

  return histograms::update_histogram(thd, table, fields,
                                      get_histogram_buckets(), results);
}

using Check_result = std::pair<bool, int>;
template <typename CHECK_FUNC>
static Check_result check_for_upgrade(THD *thd, dd::String_type &sname,
                                      dd::String_type &tname, CHECK_FUNC &&cf) {
  dd::cache::Dictionary_client *dc = thd->dd_client();

  const dd::Table *t = nullptr;
  if (dc->acquire(sname, tname, &t)) {
    return {true, HA_ADMIN_FAILED};
  }
  DBUG_ASSERT(t != nullptr);

  if (is_checked_for_upgrade(*t)) {
    DBUG_PRINT("admin", ("Table %s (%llu) already checked for upgrade, "
                         "skipping",
                         t->name().c_str(), t->id()));
    return {false, HA_ADMIN_ALREADY_DONE};
  }
  DBUG_PRINT("admin",
             ("Table %s (%llu) needs checking", t->name().c_str(), t->id()));
  int result_code = cf();

  if (result_code != HA_ADMIN_OK && result_code != HA_ADMIN_ALREADY_DONE) {
    DBUG_PRINT("admin", ("result_code: %d", result_code));
    return {false, result_code};
  }
  Check_result error{true, result_code};

  // Ok we have successfully checked table for upgrade. Record
  // this fact in the DD.

  if (acquire_shared_global_read_lock_nsec(
          thd, thd->variables.lock_wait_timeout_nsec)) {
    return error;
  }

  // Need IX on schema for acquire_for_modification()
  if (dd::sdi_utils::mdl_lock(thd, MDL_key::SCHEMA, sname, "",
                              MDL_INTENTION_EXCLUSIVE)) {
    return error;
  }

  // Need X on table so that the last_checked version can be updated
  if (dd::sdi_utils::mdl_lock(thd, MDL_key::TABLE, sname, tname)) {
    return error;
  }

  dd::Table *c = nullptr;
  if (dc->acquire_for_modification(t->id(), &c)) {
    return error;
  }
  c->mark_as_checked_for_upgrade();
  if (dc->update(c)) {
    return error;
  }
  DBUG_PRINT("admin",
             ("dd::Table %s marked as checked for upgrade", c->name().c_str()));

  return {false, result_code};
}

/*
  RETURN VALUES
    false Message sent to net (admin operation went ok)
    true  Message should be sent by caller
          (admin operation or network communication failed)
*/
static bool mysql_admin_table(
    THD *thd, TABLE_LIST *tables, HA_CHECK_OPT *check_opt,
    const char *operator_name, thr_lock_type lock_type, bool open_for_modify,
    bool repair_table_use_frm, uint extra_open_options,
    int (*prepare_func)(THD *, TABLE_LIST *, HA_CHECK_OPT *),
    int (handler::*operator_func)(THD *, HA_CHECK_OPT *), int check_view,
    Alter_info *alter_info, bool need_to_acquire_shared_backup_lock) {
  /*
    Prevent InnoDB from automatically committing InnoDB
    transaction each time data-dictionary tables are closed after
    being updated.
  */
  Disable_autocommit_guard autocommit_guard(thd);

  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  TABLE_LIST *table;
  SELECT_LEX *select = thd->lex->select_lex;
  List<Item> field_list;
  Item *item;
  Protocol *protocol = thd->get_protocol();
  LEX *lex = thd->lex;
  int result_code;
  bool gtid_rollback_must_be_skipped =
      ((thd->variables.gtid_next.type == ASSIGNED_GTID ||
        thd->variables.gtid_next.type == ANONYMOUS_GTID) &&
       (!thd->skip_gtid_rollback));
  bool ignore_grl_on_analyze = operator_func == &handler::ha_analyze;
  DBUG_TRACE;

  field_list.push_back(item =
                           new Item_empty_string("Table", NAME_CHAR_LEN * 2));
  item->maybe_null = true;
  field_list.push_back(item = new Item_empty_string("Op", 10));
  item->maybe_null = true;
  field_list.push_back(item = new Item_empty_string("Msg_type", 10));
  item->maybe_null = true;
  field_list.push_back(
      item = new Item_empty_string("Msg_text", SQL_ADMIN_MSG_TEXT_SIZE));
  item->maybe_null = true;
  if (thd->send_result_metadata(&field_list,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF))
    return true;

  /*
    Close all temporary tables which were pre-open to simplify
    privilege checking. Clear all references to closed tables.
  */
  close_thread_tables(thd);
  for (table = tables; table; table = table->next_local) table->table = nullptr;

  /*
    This statement will be written to the binary log even if it fails.
    But a failing statement calls trans_rollback_stmt which calls
    gtid_state->update_on_rollback, which releases GTID ownership.
    And GTID ownership must be held when the statement is being
    written to the binary log.  Therefore, we set this flag before
    executing the statement. The flag tells
    gtid_state->update_on_rollback to skip releasing ownership.
  */
  if (gtid_rollback_must_be_skipped) thd->skip_gtid_rollback = true;

  for (table = tables; table; table = table->next_local) {
    char table_name[NAME_LEN * 2 + 2];
    const char *db = table->db;
    bool fatal_error = false;
    bool open_error;

    DBUG_PRINT("admin", ("table: '%s'.'%s'", table->db, table->table_name));
    DBUG_PRINT("admin", ("extra_open_options: %u", extra_open_options));
    strxmov(table_name, db, ".", table->table_name, NullS);
    thd->open_options |= extra_open_options;
    table->set_lock({lock_type, THR_DEFAULT});
    /*
      To make code safe for re-execution we need to reset type of MDL
      request as code below may change it.
      To allow concurrent execution of read-only operations we acquire
      weak metadata lock for them.
    */
    table->mdl_request.set_type((lock_type >= TL_WRITE_ALLOW_WRITE)
                                    ? MDL_SHARED_NO_READ_WRITE
                                    : MDL_SHARED_READ);
    /* open only one table from local list of command */
    {
      TABLE_LIST *save_next_global, *save_next_local;
      save_next_global = table->next_global;
      table->next_global = nullptr;
      save_next_local = table->next_local;
      table->next_local = nullptr;
      select->table_list.first = table;
      /*
        Time zone tables and SP tables can be add to lex->query_tables list,
        so it have to be prepared.
        TODO: Investigate if we can put extra tables into argument instead of
        using lex->query_tables
      */
      lex->query_tables = table;
      lex->query_tables_last = &table->next_global;
      lex->query_tables_own_last = nullptr;
      /*
        CHECK TABLE command is allowed for views as well. Check on alter flags
        to differentiate from ALTER TABLE...CHECK PARTITION on which view is not
        allowed.
      */
      if (alter_info->flags & Alter_info::ALTER_ADMIN_PARTITION ||
          check_view != 1)
        table->required_type = dd::enum_table_type::BASE_TABLE;

      if (!thd->locked_tables_mode && repair_table_use_frm) {
        /*
          If we're not under LOCK TABLES and we're executing REPAIR TABLE
          USE_FRM, we need to ignore errors from open_and_lock_tables().
          REPAIR TABLE USE_FRM is a heavy weapon used when a table is
          critically damaged, so open_and_lock_tables() will most likely
          report errors. Those errors are not interesting for the user
          because it's already known that the table is badly damaged.
        */

        Diagnostics_area tmp_da(false);
        thd->push_diagnostics_area(&tmp_da);

        open_error = open_temporary_tables(thd, table);

        if (!open_error) {
          open_error = open_and_lock_tables(thd, table, 0);

          if (!open_error && need_to_acquire_shared_backup_lock &&
              /*
                Acquire backup lock explicitly since lock types used by
                admin statements won't cause its automatic acquisition
                in open_and_lock_tables().
              */
              acquire_shared_backup_lock_nsec(
                  thd, thd->variables.lock_wait_timeout_nsec)) {
            result_code = HA_ADMIN_FAILED;
            goto send_result;
          }
        }

        thd->pop_diagnostics_area();
        if (tmp_da.is_error()) {
          // Copy the exception condition information.
          thd->get_stmt_da()->set_error_status(tmp_da.mysql_errno(),
                                               tmp_da.message_text(),
                                               tmp_da.returned_sqlstate());
        }
      } else {
        /*
          It's assumed that even if it is REPAIR TABLE USE_FRM, the table
          can be opened if we're under LOCK TABLES (otherwise LOCK TABLES
          would fail). Thus, the only errors we could have from
          open_and_lock_tables() are logical ones, like incorrect locking
          mode. It does make sense for the user to see such errors.
        */

        open_error = open_temporary_tables(thd, table);

        if (!open_error) {
          open_error = open_and_lock_tables(thd, table, 0);

          if (!open_error && need_to_acquire_shared_backup_lock &&
              /*
                Acquire backup lock explicitly since lock types used by
                admin statements won't cause its automatic acquisition
                in open_and_lock_tables().
              */
              acquire_shared_backup_lock_nsec(
                  thd, thd->variables.lock_wait_timeout_nsec)) {
            result_code = HA_ADMIN_FAILED;
            goto send_result;
          }
        }
      }

      /*
        Views are always treated as materialized views, including creation
        of temporary table descriptor.
      */
      if (!open_error && table->is_view()) {
        open_error = table->resolve_derived(thd, false);
        if (!open_error) open_error = table->setup_materialized_derived(thd);
      }
      table->next_global = save_next_global;
      table->next_local = save_next_local;
      thd->open_options &= ~extra_open_options;

      /*
        If open_and_lock_tables() failed, close_thread_tables() will close
        the table and table->table can therefore be invalid.
      */
      if (open_error) table->table = nullptr;

      /*
        Under locked tables, we know that the table can be opened,
        so any errors opening the table are logical errors.
        In these cases it does not make sense to try to repair.
      */
      if (open_error && thd->locked_tables_mode) {
        result_code = HA_ADMIN_FAILED;
        goto send_result;
      }
      if (table->table) {
        /*
          Set up which partitions that should be processed
          if ALTER TABLE t ANALYZE/CHECK/OPTIMIZE/REPAIR PARTITION ..
          CACHE INDEX/LOAD INDEX for specified partitions
        */
        if (alter_info->flags & Alter_info::ALTER_ADMIN_PARTITION) {
          if (!table->table->part_info) {
            my_error(ER_PARTITION_MGMT_ON_NONPARTITIONED, MYF(0));
            result_code = HA_ADMIN_FAILED;
            goto send_result;
          }

          if (set_part_state(alter_info, table->table->part_info, PART_ADMIN,
                             true)) {
            my_error(ER_DROP_PARTITION_NON_EXISTENT, MYF(0), table_name);
            result_code = HA_ADMIN_FAILED;
            goto send_result;
          }
        }
      }
    }
    DBUG_PRINT("admin", ("table: %p", table->table));

    if (prepare_func) {
      DBUG_PRINT("admin", ("calling prepare_func"));
      switch ((*prepare_func)(thd, table, check_opt)) {
        case 1:  // error, message written to net
          trans_rollback_stmt(thd);
          trans_rollback(thd);
          /* Make sure this table instance is not reused after the operation. */
          if (table->table) table->table->m_needs_reopen = true;
          close_thread_tables(thd);
          thd->mdl_context.release_transactional_locks();
          DBUG_PRINT("admin", ("simple error, admin next table"));
          continue;
        case -1:  // error, message could be written to net
          /* purecov: begin inspected */
          DBUG_PRINT("admin", ("severe error, stop"));
          goto err;
          /* purecov: end */
        default:  // should be 0 otherwise
          DBUG_PRINT("admin", ("prepare_func succeeded"));
          ;
      }
    }

    /*
      CHECK TABLE command is only command where VIEW allowed here and this
      command use only temporary teble method for VIEWs resolving => there
      can't be VIEW tree substitition of join view => if opening table
      succeed then table->table will have real TABLE pointer as value (in
      case of join view substitution table->table can be 0, but here it is
      impossible)
    */
    if (!table->table) {
      DBUG_PRINT("admin", ("open table failed"));
      if (thd->get_stmt_da()->cond_count() == 0)
        push_warning(thd, Sql_condition::SL_WARNING, ER_CHECK_NO_SUCH_TABLE,
                     ER_THD(thd, ER_CHECK_NO_SUCH_TABLE));
      if (thd->get_stmt_da()->is_error() &&
          table_not_corrupt_error(thd->get_stmt_da()->mysql_errno()))
        result_code = HA_ADMIN_FAILED;
      else
        /* Default failure code is corrupt table */
        result_code = HA_ADMIN_CORRUPT;
      goto send_result;
    }

    if (table->is_view()) {
      result_code = HA_ADMIN_OK;
      goto send_result;
    }

    if (table->schema_table) {
      result_code = HA_ADMIN_NOT_IMPLEMENTED;
      goto send_result;
    }

    if ((table->table->db_stat & HA_READ_ONLY) && open_for_modify) {
      /* purecov: begin inspected */
      char buff[FN_REFLEN + MYSQL_ERRMSG_SIZE];
      size_t length;
      enum_sql_command save_sql_command = lex->sql_command;
      DBUG_PRINT("admin", ("sending error message"));
      protocol->start_row();
      protocol->store(table_name, system_charset_info);
      protocol->store(operator_name, system_charset_info);
      protocol->store_string(STRING_WITH_LEN("error"), system_charset_info);
      length = snprintf(buff, sizeof(buff), ER_THD(thd, ER_OPEN_AS_READONLY),
                        table_name);
      protocol->store_string(buff, length, system_charset_info);
      {
        /* Prevent intermediate commits to invoke commit order */
        Implicit_substatement_state_guard substatement_guard(
            thd, enum_implicit_substatement_guard_mode ::
                     DISABLE_GTID_AND_SPCO_IF_SPCO_ACTIVE);
        trans_commit_stmt(thd, ignore_grl_on_analyze);
        trans_commit(thd, ignore_grl_on_analyze);
      }
      /* Make sure this table instance is not reused after the operation. */
      if (table->table) table->table->m_needs_reopen = true;
      close_thread_tables(thd);
      thd->mdl_context.release_transactional_locks();
      lex->reset_query_tables_list(false);
      /*
        Restore Query_tables_list::sql_command value to make statement
        safe for re-execution.
      */
      lex->sql_command = save_sql_command;
      if (protocol->end_row()) goto err;
      thd->get_stmt_da()->reset_diagnostics_area();
      continue;
      /* purecov: end */
    }

    /*
      Close all instances of the table to allow MyISAM "repair"
      to rename files.
      @todo: This code does not close all instances of the table.
      It only closes instances in other connections, but if this
      connection has LOCK TABLE t1 a READ, t1 b WRITE,
      both t1 instances will be kept open.
      There is no need to execute this branch for InnoDB, which does
      repair by recreate. There is no need to do it for OPTIMIZE,
      which doesn't move files around.
      Hence, this code should be moved to prepare_for_repair(),
      and executed only for MyISAM engine.
    */
    if (lock_type == TL_WRITE && !table->table->s->tmp_table) {
      if (wait_while_table_is_used(thd, table->table,
                                   HA_EXTRA_PREPARE_FOR_RENAME))
        goto err;
      DEBUG_SYNC(thd, "after_admin_flush");
      /*
        XXX: hack: switch off open_for_modify to skip the
        flush that is made later in the execution flow.
      */
      open_for_modify = false;
    }

    if (table->table->s->crashed && operator_func == &handler::ha_check) {
      /* purecov: begin inspected */
      DBUG_PRINT("admin", ("sending crashed warning"));
      protocol->start_row();
      protocol->store(table_name, system_charset_info);
      protocol->store(operator_name, system_charset_info);
      protocol->store_string(STRING_WITH_LEN("warning"), system_charset_info);
      protocol->store_string(STRING_WITH_LEN("Table is marked as crashed"),
                             system_charset_info);
      if (protocol->end_row()) goto err;
      /* purecov: end */
    }

    if (operator_func == &handler::ha_repair &&
        !(check_opt->sql_flags & TT_USEFRM)) {
      // Check for old temporal format if avoid_temporal_upgrade is disabled.
      mysql_mutex_lock(&LOCK_global_system_variables);
      const bool check_temporal_upgrade = !avoid_temporal_upgrade;
      mysql_mutex_unlock(&LOCK_global_system_variables);

      if ((check_table_for_old_types(table->table, check_temporal_upgrade) ==
           HA_ADMIN_NEEDS_ALTER) ||
          (table->table->file->ha_check_for_upgrade(check_opt) ==
           HA_ADMIN_NEEDS_ALTER)) {
        DBUG_PRINT("admin", ("recreating table"));
        /*
          Temporary table are always created by current server so they never
          require upgrade. So we don't need to pre-open them before calling
          mysql_recreate_table().
        */
        DBUG_ASSERT(!table->table->s->tmp_table);

        trans_rollback_stmt(thd);
        trans_rollback(thd);
        /* Make sure this table instance is not reused after the operation. */
        if (table->table) table->table->m_needs_reopen = true;
        close_thread_tables(thd);
        thd->mdl_context.release_transactional_locks();

        /*
          table_list->table has been closed and freed. Do not reference
          uninitialized data. open_tables() could fail.
        */
        table->table = nullptr;
        /* Same applies to MDL ticket. */
        table->mdl_request.ticket = nullptr;

        {
          // binlogging is done by caller if wanted
          Disable_binlog_guard binlog_guard(thd);
          result_code = mysql_recreate_table(thd, table, false);
        }
        /*
          mysql_recreate_table() can push OK or ERROR.
          Clear 'OK' status. If there is an error, keep it:
          we will store the error message in a result set row
          and then clear.
        */
        if (thd->get_stmt_da()->is_ok())
          thd->get_stmt_da()->reset_diagnostics_area();
        table->table = nullptr;
        result_code = result_code ? HA_ADMIN_FAILED : HA_ADMIN_OK;
        goto send_result;
      }
    }

    if (check_opt && (check_opt->sql_flags & TT_FOR_UPGRADE) != 0) {
      if (table->table->s->tmp_table) {
        result_code = HA_ADMIN_OK;
      } else {
        dd::String_type snam = dd::make_string_type(table->table->s->db);
        dd::String_type tnam =
            dd::make_string_type(table->table->s->table_name);

        Check_result cr = check_for_upgrade(thd, snam, tnam, [&]() {
          DBUG_PRINT("admin", ("calling operator_func '%s'", operator_name));
          return (table->table->file->*operator_func)(thd, check_opt);
        });

        result_code = cr.second;
        if (cr.first) {
          goto err;
        }
      }
    }
    // Some other admin COMMAND
    else {
      DBUG_PRINT("admin", ("calling operator_func '%s'", operator_name));
      result_code = (table->table->file->*operator_func)(thd, check_opt);
    }
    DBUG_PRINT("admin", ("operator_func returned: %d", result_code));

    /*
      ANALYZE statement calculates values for dynamic fields of
      I_S.TABLES and I_S.STATISTICS table in table_stats and index_stats
      table. This table is joined with new dd table to provide results
      when I_S table is queried.
      To get latest statistics of table or index, user should use analyze
      table statement before querying I_S.TABLES or I_S.STATISTICS
    */

    if (!read_only && ignore_grl_on_analyze) {
      // Acquire the lock
      if (dd::info_schema::update_table_stats(thd, table) ||
          dd::info_schema::update_index_stats(thd, table)) {
        // Play safe, rollback possible changes to the data-dictionary.
        trans_rollback_stmt(thd);
        trans_rollback_implicit(thd);
        result_code = HA_ADMIN_STATS_UPD_ERR;
        goto send_result;
      }
    }

    /*
      push_warning() if the table version is lesser than current
      server version and there are triggers for this table.
    */
    if (operator_func == &handler::ha_check &&
        (check_opt->sql_flags & TT_FOR_UPGRADE) && table->table->triggers) {
      table->table->triggers->print_upgrade_warnings(thd);
    }

  send_result:

    lex->cleanup_after_one_table_open();
    thd->clear_error();  // these errors shouldn't get client
    if (send_analyze_table_errors(thd, operator_name, table_name)) goto err;
    protocol->start_row();
    protocol->store(table_name, system_charset_info);
    protocol->store(operator_name, system_charset_info);

  send_result_message:

    DBUG_PRINT("info", ("result_code: %d", result_code));
    switch (result_code) {
      case HA_ADMIN_NOT_IMPLEMENTED: {
        char buf[MYSQL_ERRMSG_SIZE];
        size_t length =
            snprintf(buf, sizeof(buf), ER_THD(thd, ER_CHECK_NOT_IMPLEMENTED),
                     operator_name);
        protocol->store_string(STRING_WITH_LEN("note"), system_charset_info);
        protocol->store_string(buf, length, system_charset_info);
      } break;

      case HA_ADMIN_NOT_BASE_TABLE: {
        char buf[MYSQL_ERRMSG_SIZE];

        String tbl_name;
        tbl_name.append(String(db, system_charset_info));
        tbl_name.append('.');
        tbl_name.append(String(table_name, system_charset_info));

        size_t length =
            snprintf(buf, sizeof(buf), ER_THD(thd, ER_BAD_TABLE_ERROR),
                     tbl_name.c_ptr());
        protocol->store_string(STRING_WITH_LEN("note"), system_charset_info);
        protocol->store_string(buf, length, system_charset_info);
      } break;

      case HA_ADMIN_OK:
        protocol->store_string(STRING_WITH_LEN("status"), system_charset_info);
        protocol->store_string(STRING_WITH_LEN("OK"), system_charset_info);
        break;

      case HA_ADMIN_FAILED:
        protocol->store_string(STRING_WITH_LEN("status"), system_charset_info);
        protocol->store_string(STRING_WITH_LEN("Operation failed"),
                               system_charset_info);
        break;

      case HA_ADMIN_REJECT:
        protocol->store_string(STRING_WITH_LEN("status"), system_charset_info);
        protocol->store_string(
            STRING_WITH_LEN("Operation need committed state"),
            system_charset_info);
        open_for_modify = false;
        break;

      case HA_ADMIN_ALREADY_DONE:
        protocol->store_string(STRING_WITH_LEN("status"), system_charset_info);
        protocol->store_string(STRING_WITH_LEN("Table is already up to date"),
                               system_charset_info);
        break;

      case HA_ADMIN_CORRUPT:
        protocol->store_string(STRING_WITH_LEN("error"), system_charset_info);
        protocol->store_string(STRING_WITH_LEN("Corrupt"), system_charset_info);
        fatal_error = true;
        break;

      case HA_ADMIN_INVALID:
        protocol->store_string(STRING_WITH_LEN("error"), system_charset_info);
        protocol->store_string(STRING_WITH_LEN("Invalid argument"),
                               system_charset_info);
        break;

      case HA_ADMIN_TRY_ALTER: {
        uint save_flags;

        /* Store the original value of alter_info->flags */
        save_flags = alter_info->flags;
        {
          /* Prevent intermediate commits to invoke commit order */
          Implicit_substatement_state_guard substatement_guard(
              thd, enum_implicit_substatement_guard_mode ::
                       DISABLE_GTID_AND_SPCO_IF_SPCO_ACTIVE);
          /*
            This is currently used only by InnoDB. ha_innobase::optimize()
            answers "try with alter", so here we close the table, do an ALTER
            TABLE, reopen the table and do ha_innobase::analyze() on it. We have
            to end the row, so analyze could return more rows.
          */
          trans_commit_stmt(thd, ignore_grl_on_analyze);
          trans_commit(thd, ignore_grl_on_analyze);
        }
        close_thread_tables(thd);
        thd->mdl_context.release_transactional_locks();

        /*
           table_list->table has been closed and freed. Do not reference
           uninitialized data. open_tables() could fail.
         */
        table->table = nullptr;
        /* Same applies to MDL ticket. */
        table->mdl_request.ticket = nullptr;

        DEBUG_SYNC(thd, "ha_admin_try_alter");
        protocol->store_string(STRING_WITH_LEN("note"), system_charset_info);
        if (alter_info->flags & Alter_info::ALTER_ADMIN_PARTITION) {
          protocol->store_string(
              STRING_WITH_LEN("Table does not support optimize on "
                              "partitions. All partitions "
                              "will be rebuilt and analyzed."),
              system_charset_info);
        } else {
          protocol->store_string(
              STRING_WITH_LEN("Table does not support optimize, "
                              "doing recreate + analyze instead"),
              system_charset_info);
        }
        if (protocol->end_row()) goto err;
        DBUG_PRINT("info", ("HA_ADMIN_TRY_ALTER, trying analyze..."));
        TABLE_LIST *save_next_local = table->next_local,
                   *save_next_global = table->next_global;
        table->next_local = table->next_global = nullptr;
        {
          // binlogging is done by caller if wanted
          Disable_binlog_guard binlog_guard(thd);
          /* Don't forget to pre-open temporary tables. */
          result_code = (open_temporary_tables(thd, table) ||
                         mysql_recreate_table(thd, table, false));
        }
        /*
          mysql_recreate_table() can push OK or ERROR.
          Clear 'OK' status. If there is an error, keep it:
          we will store the error message in a result set row
          and then clear.
        */
        if (thd->get_stmt_da()->is_ok())
          thd->get_stmt_da()->reset_diagnostics_area();
        {
          /* Prevent intermediate commits to invoke commit order */
          Implicit_substatement_state_guard substatement_guard(
              thd, enum_implicit_substatement_guard_mode ::
                       DISABLE_GTID_AND_SPCO_IF_SPCO_ACTIVE);
          trans_commit_stmt(thd, ignore_grl_on_analyze);
          trans_commit(thd, ignore_grl_on_analyze);
        }
        close_thread_tables(thd);
        thd->mdl_context.release_transactional_locks();
        /* Clear references to TABLE and MDL_ticket after releasing them. */
        table->table = nullptr;
        table->mdl_request.ticket = nullptr;
        if (!result_code)  // recreation went ok
        {
          DEBUG_SYNC(thd, "ha_admin_open_ltable");
          if (acquire_shared_backup_lock_nsec(
                  thd, thd->variables.lock_wait_timeout_nsec)) {
            result_code = HA_ADMIN_FAILED;
          } else {
            table->mdl_request.set_type(MDL_SHARED_READ);
            if (!open_temporary_tables(thd, table) &&
                (table->table = open_n_lock_single_table(
                     thd, table, TL_READ_NO_INSERT, 0))) {
              /*
             Reset the ALTER_ADMIN_PARTITION bit in alter_info->flags
             to force analyze on all partitions.
               */
              alter_info->flags &= ~(Alter_info::ALTER_ADMIN_PARTITION);
              result_code = table->table->file->ha_analyze(thd, check_opt);
              if (result_code == HA_ADMIN_ALREADY_DONE)
                result_code = HA_ADMIN_OK;
              else if (result_code)  // analyze failed
                table->table->file->print_error(result_code, MYF(0));
              alter_info->flags = save_flags;
            } else
              result_code = -1;  // open failed
          }
        }
        /* Start a new row for the final status row */
        protocol->start_row();
        protocol->store(table_name, system_charset_info);
        protocol->store(operator_name, system_charset_info);
        if (result_code)  // either mysql_recreate_table or analyze failed
        {
          DBUG_ASSERT(thd->is_error() || thd->killed);
          if (thd->is_error()) {
            Diagnostics_area *da = thd->get_stmt_da();
            if (!thd->get_protocol()->connection_alive()) {
              LogEvent()
                  .type(LOG_TYPE_ERROR)
                  .subsys(LOG_SUBSYSTEM_TAG)
                  .prio(ERROR_LEVEL)
                  .source_file(MY_BASENAME)
                  .lookup(ER_ERROR_INFO_FROM_DA, da->mysql_errno(),
                          da->message_text())
                  .sqlstate(da->returned_sqlstate());
            } else {
              /* Hijack the row already in-progress. */
              protocol->store_string(STRING_WITH_LEN("error"),
                                     system_charset_info);
              protocol->store(da->message_text(), system_charset_info);
              if (protocol->end_row()) goto err;
              /* Start off another row for HA_ADMIN_FAILED */
              protocol->start_row();
              protocol->store(table_name, system_charset_info);
              protocol->store(operator_name, system_charset_info);
            }
            thd->clear_error();
          }
          /* Make sure this table instance is not reused after the operation. */
          if (table->table) table->table->m_needs_reopen = true;
        }
        result_code = result_code ? HA_ADMIN_FAILED : HA_ADMIN_OK;
        table->next_local = save_next_local;
        table->next_global = save_next_global;
        goto send_result_message;
      }
      case HA_ADMIN_WRONG_CHECKSUM: {
        protocol->store_string(STRING_WITH_LEN("note"), system_charset_info);
        protocol->store_string(ER_THD(thd, ER_VIEW_CHECKSUM),
                               strlen(ER_THD(thd, ER_VIEW_CHECKSUM)),
                               system_charset_info);
        break;
      }

      case HA_ADMIN_NEEDS_UPGRADE:
      case HA_ADMIN_NEEDS_ALTER: {
        char buf[MYSQL_ERRMSG_SIZE];
        size_t length;

        protocol->store_string(STRING_WITH_LEN("error"), system_charset_info);
        if (table->table->file->ha_table_flags() & HA_CAN_REPAIR)
          length =
              snprintf(buf, sizeof(buf), ER_THD(thd, ER_TABLE_NEEDS_UPGRADE),
                       table->table_name);
        else
          length =
              snprintf(buf, sizeof(buf), ER_THD(thd, ER_TABLE_NEEDS_REBUILD),
                       table->table_name);
        protocol->store_string(buf, length, system_charset_info);
        fatal_error = true;
        break;
      }

      case HA_ADMIN_STATS_UPD_ERR:
        protocol->store_string(STRING_WITH_LEN("status"), system_charset_info);
        protocol->store_string(
            STRING_WITH_LEN("Unable to write table statistics to DD tables"),
            system_charset_info);
        break;

      case HA_ADMIN_NEEDS_DUMP_UPGRADE: {
        /*
          In-place upgrade does not allow pre 5.0 decimal to 8.0. Recreation of
          tables will not create pre 5.0 decimal types. Hence, control should
          never reach here.
        */
        DBUG_ASSERT(false);

        char buf[MYSQL_ERRMSG_SIZE];
        size_t length;

        protocol->store_string(STRING_WITH_LEN("error"), system_charset_info);
        length = snprintf(buf, sizeof(buf),
                          "Table upgrade required for "
                          "`%-.64s`.`%-.64s`. Please dump/reload table to "
                          "fix it!",
                          table->db, table->table_name);
        protocol->store_string(buf, length, system_charset_info);
        fatal_error = true;
        break;
      }

      default:  // Probably HA_ADMIN_INTERNAL_ERROR
      {
        char buf[MYSQL_ERRMSG_SIZE];
        size_t length = snprintf(buf, sizeof(buf),
                                 "Unknown - internal error %d during operation",
                                 result_code);
        protocol->store_string(STRING_WITH_LEN("error"), system_charset_info);
        protocol->store_string(buf, length, system_charset_info);
        fatal_error = true;
        break;
      }
    }
    if (table->table) {
      const bool skip_flush =
          (operator_func == &handler::ha_analyze) &&
          (table->table->file->ha_table_flags() & HA_ONLINE_ANALYZE);
      if (table->table->s->tmp_table) {
        /*
          If the table was not opened successfully, do not try to get
          status information. (Bug#47633)
        */
        if (open_for_modify && !open_error)
          table->table->file->info(HA_STATUS_CONST);
      } else if ((!skip_flush && open_for_modify) || fatal_error) {
        tdc_remove_table(thd, TDC_RT_REMOVE_UNUSED, table->db,
                         table->table_name, false);
      } else {
        /*
          Reset which partitions that should be processed
          if ALTER TABLE t ANALYZE/CHECK/.. PARTITION ..
          CACHE INDEX/LOAD INDEX for specified partitions
        */
        if (table->table->part_info &&
            alter_info->flags & Alter_info::ALTER_ADMIN_PARTITION) {
          set_all_part_state(table->table->part_info, PART_NORMAL);
        }
      }
    }
    /* Error path, a admin command failed. */
    if (thd->transaction_rollback_request) {
      /*
        Unlikely, but transaction rollback was requested by one of storage
        engines (e.g. due to deadlock). Perform it.
      */
      DBUG_PRINT("admin", ("rollback"));

      if (trans_rollback_stmt(thd) || trans_rollback_implicit(thd)) goto err;
    } else {
      enum_implicit_substatement_guard_mode mode =
          enum_implicit_substatement_guard_mode ::
              DISABLE_GTID_AND_SPCO_IF_SPCO_ACTIVE;

      if (strcmp(operator_name, "optimize") == 0 ||
          strcmp(operator_name, "analyze") == 0 ||
          strcmp(operator_name, "repair") == 0) {
        mode = enum_implicit_substatement_guard_mode ::
            ENABLE_GTID_AND_SPCO_IF_SPCO_ACTIVE;
      }

      /*
        It allows saving GTID and invoking commit order i.e. set
        thd->is_operating_substatement_implicitly = false, when
        slave-preserve-commit-order is enabled and any of OPTIMIZE TABLE,
        ANALYZE TABLE and REPAIR TABLE command is getting executed,
        otherwise saving GTID and invoking commit order is disabled.
      */
      Implicit_substatement_state_guard guard(thd, mode);

      if (trans_commit_stmt(thd, ignore_grl_on_analyze) ||
          trans_commit_implicit(thd, ignore_grl_on_analyze))
        goto err;
      DBUG_PRINT("admin", ("commit"));
    }
    close_thread_tables(thd);
    thd->mdl_context.release_transactional_locks();

    if (protocol->end_row()) goto err;
  }

  my_eof(thd);

  if (gtid_rollback_must_be_skipped) thd->skip_gtid_rollback = false;

  return false;

err:
  DBUG_PRINT("admin", ("err:"));
  if (gtid_rollback_must_be_skipped) thd->skip_gtid_rollback = false;

  trans_rollback_stmt(thd);
  trans_rollback(thd);

  if (thd->sp_runtime_ctx) thd->sp_runtime_ctx->end_partial_result_set = true;

  /* Make sure this table instance is not reused after the operation. */
  if (table->table) table->table->m_needs_reopen = true;
  close_thread_tables(thd);  // Shouldn't be needed
  thd->mdl_context.release_transactional_locks();
  return true;
}

/*
  Assigned specified indexes for a table into key cache

  SYNOPSIS
    assign_to_keycache()
    thd		Thread object
    tables	Table list (one table only)

  RETURN VALUES
   false ok
   true  error
*/

bool Sql_cmd_cache_index::assign_to_keycache(THD *thd, TABLE_LIST *tables) {
  HA_CHECK_OPT check_opt;
  KEY_CACHE *key_cache;
  DBUG_TRACE;

  mysql_mutex_lock(&LOCK_global_system_variables);
  if (!(key_cache = get_key_cache(&m_key_cache_name))) {
    mysql_mutex_unlock(&LOCK_global_system_variables);
    my_error(ER_UNKNOWN_KEY_CACHE, MYF(0), m_key_cache_name.str);
    return true;
  }
  mysql_mutex_unlock(&LOCK_global_system_variables);
  if (!key_cache->key_cache_inited) {
    my_error(ER_UNKNOWN_KEY_CACHE, MYF(0), m_key_cache_name.str);
    return true;
  }
  check_opt.key_cache = key_cache;
  // ret is needed since DBUG_RETURN isn't friendly to function call parameters:
  const bool ret = mysql_admin_table(
      thd, tables, &check_opt, "assign_to_keycache", TL_READ_NO_INSERT, false,
      false, 0, nullptr, &handler::assign_to_keycache, 0, m_alter_info, false);
  return ret;
}

/*
  Preload specified indexes for a table into key cache

  SYNOPSIS
    preload_keys()
    thd		Thread object
    tables	Table list (one table only)

  RETURN VALUES
    false ok
    true  error
*/

bool Sql_cmd_load_index::preload_keys(THD *thd, TABLE_LIST *tables) {
  DBUG_TRACE;
  /*
    We cannot allow concurrent inserts. The storage engine reads
    directly from the index file, bypassing the cache. It could read
    outdated information if parallel inserts into cache blocks happen.
  */
  // ret is needed since DBUG_RETURN isn't friendly to function call parameters:
  const bool ret = mysql_admin_table(
      thd, tables, nullptr, "preload_keys", TL_READ_NO_INSERT, false, false, 0,
      nullptr, &handler::preload_keys, 0, m_alter_info, false);
  return ret;
}

bool Sql_cmd_analyze_table::set_histogram_fields(List<String> *fields) {
  DBUG_ASSERT(m_histogram_fields.empty());

  List_iterator<String> it(*fields);
  String *field;
  while ((field = it++)) {
    if (!m_histogram_fields.emplace(field).second) {
      my_error(ER_DUP_FIELDNAME, MYF(0), field->ptr());
      return true;
    }
  }

  return false;
}

bool Sql_cmd_analyze_table::handle_histogram_command(THD *thd,
                                                     TABLE_LIST *table) {
  // This should not be empty here.
  DBUG_ASSERT(!get_histogram_fields().empty());

  histograms::results_map results;
  bool res = false;
  if (table->next_local != nullptr) {
    /*
      Only one table can be specified for
      ANALYZE TABLE ... UPDATE/DROP HISTOGRAM
    */
    results.emplace("", histograms::Message::MULTIPLE_TABLES_SPECIFIED);
    res = true;
  } else {
    if (read_only || thd->tx_read_only) {
      // Do not try to update histograms when in read_only mode.
      results.emplace("", histograms::Message::SERVER_READ_ONLY);
      res = false;
    } else {
      Disable_autocommit_guard autocommit_guard(thd);

      /* Prevent intermediate commits to invoke commit order */
      Implicit_substatement_state_guard substatement_guard(
          thd, enum_implicit_substatement_guard_mode ::
                   DISABLE_GTID_AND_SPCO_IF_SPCO_ACTIVE);

      dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
      switch (get_histogram_command()) {
        case Histogram_command::UPDATE_HISTOGRAM:
          res = acquire_shared_backup_lock_nsec(
                    thd, thd->variables.lock_wait_timeout_nsec) ||
                update_histogram(thd, table, results);
          break;
        case Histogram_command::DROP_HISTOGRAM:
          res = acquire_shared_backup_lock_nsec(
                    thd, thd->variables.lock_wait_timeout_nsec) ||
                drop_histogram(thd, table, results);

          if (res) {
            /*
              Do a rollback. We can end up here if query was interrupted
              during drop_histogram.
            */
            trans_rollback_stmt(thd);
            trans_rollback(thd);
          } else {
            res = trans_commit_stmt(thd) || trans_commit(thd);
          }
          break;
        case Histogram_command::NONE:
          DBUG_ASSERT(false); /* purecov: deadcode */
          break;
      }

      if (!res) {
        /*
          If a histogram was added, updated or removed, we will request the old
          TABLE_SHARE to go away from the table definition cache. This is
          beacuse histogram data is cached in the TABLE_SHARE, so we want new
          transactions to fetch the updated data into the TABLE_SHARE before
          using it again.
        */
        tdc_remove_table(thd, TDC_RT_REMOVE_UNUSED, table->db,
                         table->table_name, false);
      }
    }
  }

  thd->clear_error();
  send_histogram_results(thd, results, table);
  thd->get_stmt_da()->reset_condition_info(thd);
  my_eof(thd);
  return res;
}

bool Sql_cmd_analyze_table::execute(THD *thd) {
  TABLE_LIST *first_table = thd->lex->select_lex->get_table_list();
  bool res = true;
  thr_lock_type lock_type = TL_READ_NO_INSERT;
  DBUG_TRACE;

  if (check_table_access(thd, SELECT_ACL | INSERT_ACL, first_table, false,
                         UINT_MAX, false))
    goto error;

  DBUG_EXECUTE_IF("simulate_analyze_table_lock_wait_timeout_error", {
    my_error(ER_LOCK_WAIT_TIMEOUT, MYF(0),
             "simulate_analyze_table_lock_wait_timeout_error");
    return true;
  });

  thd->enable_slow_log = opt_log_slow_admin_statements;

  if (get_histogram_command() != Histogram_command::NONE) {
    res = handle_histogram_command(thd, first_table);
  } else {
    res = mysql_admin_table(thd, first_table, &thd->lex->check_opt, "analyze",
                            lock_type, true, false, 0, nullptr,
                            &handler::ha_analyze, 0, m_alter_info, true);
  }

  /* ! we write after unlocking the table */
  if (!res && !thd->lex->no_write_to_binlog) {
    /*
      Presumably, ANALYZE and binlog writing doesn't require synchronization
    */
    res = write_bin_log(thd, true, thd->query().str, thd->query().length);
  }
  thd->lex->select_lex->table_list.first = first_table;
  thd->lex->query_tables = first_table;

error:
  return res;
}

bool Sql_cmd_check_table::execute(THD *thd) {
  TABLE_LIST *first_table = thd->lex->select_lex->get_table_list();
  thr_lock_type lock_type = TL_READ_NO_INSERT;
  bool res = true;
  DBUG_TRACE;

  if (check_table_access(thd, SELECT_ACL, first_table, true, UINT_MAX, false))
    goto error; /* purecov: inspected */
  thd->enable_slow_log = opt_log_slow_admin_statements;

  res = mysql_admin_table(thd, first_table, &thd->lex->check_opt, "check",
                          lock_type, false, false, HA_OPEN_FOR_REPAIR, nullptr,
                          &handler::ha_check, 1, m_alter_info, true);

  thd->lex->select_lex->table_list.first = first_table;
  thd->lex->query_tables = first_table;

error:
  return res;
}

bool Sql_cmd_optimize_table::execute(THD *thd) {
  TABLE_LIST *first_table = thd->lex->select_lex->get_table_list();
  bool res = true;
  DBUG_TRACE;

  if (check_table_access(thd, SELECT_ACL | INSERT_ACL, first_table, false,
                         UINT_MAX, false))
    goto error; /* purecov: inspected */
  thd->enable_slow_log = opt_log_slow_admin_statements;
  res = (specialflag & SPECIAL_NO_NEW_FUNC)
            ? mysql_recreate_table(thd, first_table, true)
            : mysql_admin_table(thd, first_table, &thd->lex->check_opt,
                                "optimize", TL_WRITE, true, false, 0, nullptr,
                                &handler::ha_optimize, 0, m_alter_info, true);
  /* ! we write after unlocking the table */
  if (!res && !thd->lex->no_write_to_binlog) {
    /*
      Presumably, OPTIMIZE and binlog writing doesn't require synchronization
    */
    res = write_bin_log(thd, true, thd->query().str, thd->query().length);
  }
  thd->lex->select_lex->table_list.first = first_table;
  thd->lex->query_tables = first_table;

error:
  return res;
}

bool Sql_cmd_repair_table::execute(THD *thd) {
  TABLE_LIST *first_table = thd->lex->select_lex->get_table_list();
  bool res = true;
  DBUG_TRACE;

  if (check_table_access(thd, SELECT_ACL | INSERT_ACL, first_table, false,
                         UINT_MAX, false))
    goto error; /* purecov: inspected */
  thd->enable_slow_log = opt_log_slow_admin_statements;
  res = mysql_admin_table(
      thd, first_table, &thd->lex->check_opt, "repair", TL_WRITE, true,
      thd->lex->check_opt.sql_flags & TT_USEFRM, HA_OPEN_FOR_REPAIR,
      &prepare_for_repair, &handler::ha_repair, 0, m_alter_info, true);

  /* ! we write after unlocking the table */
  if (!res && !thd->lex->no_write_to_binlog) {
    /*
      Presumably, REPAIR and binlog writing doesn't require synchronization
    */
    res = write_bin_log(thd, true, thd->query().str, thd->query().length);
  }
  thd->lex->select_lex->table_list.first = first_table;
  thd->lex->query_tables = first_table;

error:
  return res;
}

bool Sql_cmd_shutdown::execute(THD *thd) {
  DBUG_TRACE;
  bool res = true;
  res = !shutdown(thd, SHUTDOWN_DEFAULT, thd->lex->shutdown_exit_code,
                  thd->lex->shutdown_ro_instance_only);

  return res;
}

class Alter_instance_reload_tls : public Alter_instance {
 public:
  explicit Alter_instance_reload_tls(THD *thd, bool force = false)
      : Alter_instance(thd), force_(force) {}

  bool execute() {
    Security_context *sctx = m_thd->security_context();
    if (!sctx->has_global_grant(STRING_WITH_LEN("CONNECTION_ADMIN")).first) {
      my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0), "CONNECTION_ADMIN");
      return true;
    }

    bool res = false;
    enum enum_ssl_init_error error = SSL_INITERR_NOERROR;
    SslAcceptorContext::singleton_flush(&error, force_);
    if (error != SSL_INITERR_NOERROR) {
      const char *error_text = sslGetErrString(error);
      if (force_) {
        push_warning_printf(m_thd, Sql_condition::SL_WARNING,
                            ER_DA_SSL_LIBRARY_ERROR,
                            ER_THD(m_thd, ER_DA_SSL_LIBRARY_ERROR), error_text);
        LogErr(WARNING_LEVEL, ER_SSL_LIBRARY_ERROR, sslGetErrString(error));
      } else {
        my_error(ER_DA_SSL_LIBRARY_ERROR, MYF(0), error_text);
        res = true;
      }
    }

    if (!res) my_ok(m_thd);
    return res;
  }
  ~Alter_instance_reload_tls() {}

 protected:
  bool force_;
};

bool Sql_cmd_alter_instance::execute(THD *thd) {
  bool res = true;
  DBUG_TRACE;
  switch (alter_instance_action) {
    case ROTATE_INNODB_MASTER_KEY:
      alter_instance = new Rotate_innodb_master_key(thd);
      break;
    case ALTER_INSTANCE_RELOAD_TLS:
      alter_instance = new Alter_instance_reload_tls(thd, true);
      break;
    case ALTER_INSTANCE_RELOAD_TLS_ROLLBACK_ON_ERROR:
      alter_instance = new Alter_instance_reload_tls(thd);
      break;
    case ROTATE_BINLOG_MASTER_KEY:
      alter_instance = new Rotate_binlog_master_key(thd);
      break;
    default:
      DBUG_ASSERT(false);
      my_error(ER_NOT_SUPPORTED_YET, MYF(0), "ALTER INSTANCE");
      return true;
  }

  /*
    If we reach here, the only case when alter_instance
    is NULL is if we got out of memory error.
    In case of unsupported option, we should have returned
    from default case in switch() statement above.
  */
  if (!alter_instance) {
    my_error(ER_OUT_OF_RESOURCES, MYF(0));
  } else {
    res = alter_instance->execute();
    delete alter_instance;
    alter_instance = nullptr;
  }

  return res;
}

Sql_cmd_clone::Sql_cmd_clone(LEX_USER *user_info, ulong port,
                             LEX_CSTRING data_dir)
    : m_port(port), m_data_dir(data_dir), m_clone(), m_is_local(false) {
  m_host = user_info->host;
  m_user = user_info->user;
  m_passwd = user_info->auth;
}

bool Sql_cmd_clone::execute(THD *thd) {
  DBUG_TRACE;

  bool is_replace = (m_data_dir.str == nullptr);

  if (is_local()) {
    DBUG_PRINT("admin", ("CLONE type = local, DIR = %s", m_data_dir.str));

  } else {
    DBUG_PRINT("admin", ("CLONE type = remote, DIR = %s",
                         is_replace ? "" : m_data_dir.str));
  }

  auto sctx = thd->security_context();

  /* For replacing current data directory, needs clone_admin privilege. */
  if (is_replace) {
    if (!(sctx->has_global_grant(STRING_WITH_LEN("CLONE_ADMIN")).first)) {
      my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0), "CLONE_ADMIN");
      return true;
    }
  } else if (!(sctx->has_global_grant(STRING_WITH_LEN("BACKUP_ADMIN")).first)) {
    my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0), "BACKUP_ADMIN");
    return true;
  }

  /* A user session cannot run clone that replaces data on a group member. */
  if (is_replace && is_group_replication_running() &&
      strcmp(thd->security_context()->priv_user().str, "mysql.session")) {
    my_error(ER_CLONE_DISALLOWED, MYF(0), "Group Replication is running");
    return true;
  }

  DBUG_ASSERT(m_clone == nullptr);
  m_clone = clone_plugin_lock(thd, &m_plugin);

  if (m_clone == nullptr) {
    my_error(ER_PLUGIN_IS_NOT_LOADED, MYF(0), "clone");
    return true;
  }

  if (is_local()) {
    DBUG_ASSERT(!is_replace);
    auto err = m_clone->clone_local(thd, m_data_dir.str);
    clone_plugin_unlock(thd, m_plugin);

    if (err != 0) {
      return true;
    }

    my_ok(thd);
    return false;
  }

  DBUG_ASSERT(!is_local());

  enum mysql_ssl_mode ssl_mode = SSL_MODE_DISABLED;

  if (thd->lex->ssl_type == SSL_TYPE_NONE) {
    ssl_mode = SSL_MODE_DISABLED;
  } else if (thd->lex->ssl_type == SSL_TYPE_SPECIFIED) {
    ssl_mode = SSL_MODE_REQUIRED;
  } else {
    DBUG_ASSERT(thd->lex->ssl_type == SSL_TYPE_NOT_SPECIFIED);
    ssl_mode = SSL_MODE_PREFERRED;
  }

  auto err = m_clone->clone_remote_client(
      thd, m_host.str, static_cast<uint>(m_port), m_user.str, m_passwd.str,
      m_data_dir.str, ssl_mode);
  clone_plugin_unlock(thd, m_plugin);
  m_clone = nullptr;

  /* Set active VIO as clone plugin might have reset it */
  if (thd->is_classic_protocol()) {
    NET *net = thd->get_protocol_classic()->get_net();
    thd->set_active_vio(net->vio);
  }

  if (err != 0) {
    /* Log donor error number and message. */
    if (err == ER_CLONE_DONOR) {
      const char *donor_mesg = nullptr;
      int donor_error = 0;
      bool success =
          Clone_handler::get_donor_error(nullptr, donor_error, donor_mesg);
      if (success && donor_error != 0 && donor_mesg != nullptr) {
        char info_mesg[128];
        snprintf(info_mesg, 128, "Clone Donor error : %d : %s", donor_error,
                 donor_mesg);
        LogErr(INFORMATION_LEVEL, ER_CLONE_CLIENT_TRACE, info_mesg);
      }
    }
    return true;
  }

  /* Check for KILL after setting active VIO */
  if (!is_replace && thd->killed != THD::NOT_KILLED) {
    my_error(ER_QUERY_INTERRUPTED, MYF(0));
    return true;
  }

  /* Restart server after successfully cloning to current data directory. */
  if (is_replace && signal_restart_server()) {
    /* Shutdown server if restart failed. */
    LogErr(ERROR_LEVEL, ER_CLONE_SHUTDOWN_TRACE);
    Diagnostics_area shutdown_da(false);
    thd->push_diagnostics_area(&shutdown_da);
    /* CLONE_ADMIN privilege allows us to shutdown/restart at end. */
    kill_mysql();
    thd->pop_diagnostics_area();
    return true;
  }

  my_ok(thd);
  return false;
}

bool Sql_cmd_clone::load(THD *thd) {
  DBUG_TRACE;
  DBUG_ASSERT(m_clone == nullptr);
  DBUG_ASSERT(!is_local());

  auto sctx = thd->security_context();

  if (!(sctx->has_global_grant(STRING_WITH_LEN("BACKUP_ADMIN")).first)) {
    my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0), "BACKUP_ADMIN");
    return true;
  }

  m_clone = clone_plugin_lock(thd, &m_plugin);

  if (m_clone == nullptr) {
    my_error(ER_PLUGIN_IS_NOT_LOADED, MYF(0), "clone");
    return true;
  }

  my_ok(thd);
  return false;
}

bool Sql_cmd_clone::execute_server(THD *thd) {
  DBUG_TRACE;
  DBUG_ASSERT(!is_local());

  bool ret = false;
  auto net = thd->get_protocol_classic()->get_net();
  auto sock = net->vio->mysql_socket;

  Diagnostics_area clone_da(false);

  thd->push_diagnostics_area(&clone_da);

  auto err = m_clone->clone_remote_server(thd, sock);

  if (err == 0) {
    my_ok(thd);
  }

  thd->pop_diagnostics_area();

  if (err != 0) {
    auto da = thd->get_stmt_da();

    da->set_overwrite_status(true);

    da->set_error_status(clone_da.mysql_errno(), clone_da.message_text(),
                         clone_da.returned_sqlstate());
    da->push_warning(thd, clone_da.mysql_errno(), clone_da.returned_sqlstate(),
                     Sql_condition::SL_ERROR, clone_da.message_text());
    ret = true;
  }

  clone_plugin_unlock(thd, m_plugin);
  m_clone = nullptr;

  return ret;
}

bool Sql_cmd_clone::rewrite(THD *thd, String &rlb) {
  /* No password for local clone. */
  if (is_local()) {
    return false;
  }

  rlb.append(STRING_WITH_LEN("CLONE INSTANCE FROM "));

  /* Append user name. */
  String user(m_user.str, m_user.length, system_charset_info);
  append_query_string(thd, system_charset_info, &user, &rlb);

  /* Append host name. */
  rlb.append(STRING_WITH_LEN("@"));
  String host(m_host.str, m_host.length, system_charset_info);
  append_query_string(thd, system_charset_info, &host, &rlb);

  /* Append port number. */
  rlb.append(STRING_WITH_LEN(":"));
  String num_buffer(42);
  num_buffer.set((longlong)m_port, &my_charset_bin);
  rlb.append(num_buffer);

  /* Append password clause. */
  rlb.append(STRING_WITH_LEN(" IDENTIFIED BY <secret>"));

  /* Append data directory clause. */
  if (m_data_dir.str != nullptr) {
    rlb.append(STRING_WITH_LEN(" DATA DIRECTORY = "));
    String dir(m_data_dir.str, m_data_dir.length, system_charset_info);
    append_query_string(thd, system_charset_info, &dir, &rlb);
  }

  /* Append SSL information. */
  if (thd->lex->ssl_type == SSL_TYPE_NONE) {
    rlb.append(STRING_WITH_LEN(" REQUIRE NO SSL"));

  } else if (thd->lex->ssl_type == SSL_TYPE_SPECIFIED) {
    rlb.append(STRING_WITH_LEN(" REQUIRE SSL"));
  }
  return true;
}

bool Sql_cmd_create_role::execute(THD *thd) {
  DBUG_TRACE;
  // TODO: Execution-time processing of the CREATE ROLE statement
  if (check_global_access(thd, CREATE_ROLE_ACL | CREATE_USER_ACL)) return true;
  /* Conditionally writes to binlog */
  HA_CREATE_INFO create_info;
  /*
    Roles must be locked for authentication by default.
    The below is a hack to make mysql_create_user() behave
    correctly.
  */
  thd->lex->ssl_cipher = nullptr;
  thd->lex->x509_issuer = nullptr;
  thd->lex->x509_subject = nullptr;
  thd->lex->ssl_type = SSL_TYPE_NOT_SPECIFIED;
  thd->lex->alter_password.account_locked = true;
  thd->lex->alter_password.update_account_locked_column = true;
  thd->lex->alter_password.expire_after_days = 0;
  thd->lex->alter_password.update_password_expired_column = true;
  thd->lex->alter_password.use_default_password_lifetime = true;
  thd->lex->alter_password.update_password_expired_fields = true;
  thd->lex->alter_password.update_password_require_current =
      Lex_acl_attrib_udyn::UNCHANGED;
  thd->lex->alter_password.failed_login_attempts = 0;
  thd->lex->alter_password.password_lock_time = 0;

  List_iterator<LEX_USER> it(*const_cast<List<LEX_USER> *>(roles));
  LEX_USER *role;
  while ((role = it++)) {
    role->uses_identified_by_clause = false;
    role->uses_identified_with_clause = false;
    role->uses_authentication_string_clause = false;
    role->alter_status.expire_after_days = 0;
    role->alter_status.account_locked = true;
    role->alter_status.update_account_locked_column = true;
    role->alter_status.update_password_expired_fields = true;
    role->alter_status.use_default_password_lifetime = true;
    role->alter_status.update_password_expired_column = true;
    role->auth.str = nullptr;
    role->auth.length = 0;
    role->has_password_generator = false;
  }
  if (!(mysql_create_user(thd, *const_cast<List<LEX_USER> *>(roles),
                          if_not_exists, true))) {
    // Either my_ok() or my_eof() was called in mysql_create_user()
    return false;
  }
  // my_error() was called.
  return true;
}

bool Sql_cmd_drop_role::execute(THD *thd) {
  DBUG_TRACE;
  /*
    We want to do extra checks (if user login is disabled) when golding a
    using DROP_ROLE privilege.
    To do that we record if CREATE USER was granted.
    Then if one of DROP ROLE or CREATE USER was granted (the original
    requirement) and CREATE USER was not granted we know that it was DROP ROLE
    that caused the check to pass.

    Thus we raise the flag (drop_role) in this case.
  */
  bool on_create_user_priv =
      thd->security_context()->check_access(CREATE_USER_ACL, "", true);
  if (check_global_access(thd, DROP_ROLE_ACL | CREATE_USER_ACL)) return true;
  if (mysql_drop_user(thd, const_cast<List<LEX_USER> &>(*roles), ignore_errors,
                      !on_create_user_priv))
    return true;
  my_ok(thd);
  return false;
}

bool Sql_cmd_set_role::execute(THD *thd) {
  DBUG_TRACE;
  bool ret = false;
  switch (role_type) {
    case role_enum::ROLE_NONE:
      ret = mysql_set_active_role_none(thd);
      break;
    case role_enum::ROLE_DEFAULT:
      ret = mysql_set_role_default(thd);
      break;
    case role_enum::ROLE_ALL:
      ret = mysql_set_active_role_all(thd, except_roles);
      break;
    case role_enum::ROLE_NAME:
      ret = mysql_set_active_role(thd, role_list);
      break;
  }

  /*
    1. In case of role_enum::ROLE_NONE -
       User might have SYSTEM_USER privilege granted explicitly using GRANT
       statement.
    2. For other cases -
       User may have got SYSTEM_USER privilege either through one of the roles
       OR, privilege may have been granted explicitly using GRANT statement.
    Therefore, update the THD accordingly.

    Update the flag in THD if invoker has SYSTEM_USER privilege not if the
    definer user has that privilege.
  */
  if (!ret) set_system_user_flag(thd, true);

  return ret;
}

bool Sql_cmd_grant_roles::execute(THD *thd) {
  DBUG_TRACE;
  for (const LEX_USER &role : *roles) {
    if (!has_grant_role_privilege(thd, role.user, role.host)) {
      my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0),
               "WITH ADMIN, ROLE_ADMIN, SUPER");
      return true;
    }
  }
  return mysql_grant_role(thd, users, roles, this->with_admin_option);
}

bool Sql_cmd_revoke_roles::execute(THD *thd) {
  DBUG_TRACE;
  for (const LEX_USER &role : *roles) {
    if (!has_grant_role_privilege(thd, role.user, role.host)) {
      my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0),
               "WITH ADMIN, ROLE_ADMIN, SUPER");
      return true;
    }
  }
  return mysql_revoke_role(thd, users, roles);
}

bool Sql_cmd_alter_user_default_role::execute(THD *thd) {
  DBUG_TRACE;

  bool ret = mysql_alter_or_clear_default_roles(thd, role_type, users, roles);
  if (!ret) my_ok(thd);

  return ret;
}

bool Sql_cmd_show_grants::execute(THD *thd) {
  DBUG_TRACE;
  bool show_mandatory_roles = (for_user == nullptr);
  bool have_using_clause =
      (using_users != nullptr && using_users->elements > 0);

  if (for_user == nullptr || for_user->user.str == nullptr) {
    /* SHOW PRIVILEGE FOR CURRENT_USER */
    LEX_USER current_user;
    get_default_definer(thd, &current_user);
    if (!have_using_clause) {
      const List_of_auth_id_refs *active_list =
          thd->security_context()->get_active_roles();
      return mysql_show_grants(thd, &current_user, *active_list,
                               show_mandatory_roles, have_using_clause);
    }
  } else if (strcmp(thd->security_context()->priv_user().str,
                    for_user->user.str) != 0) {
    TABLE_LIST table("mysql", "user", nullptr, TL_READ);
    if (!is_granted_table_access(thd, SELECT_ACL, &table)) {
      char command[128];
      get_privilege_desc(command, sizeof(command), SELECT_ACL);
      my_error(ER_TABLEACCESS_DENIED_ERROR, MYF(0), command,
               thd->security_context()->priv_user().str,
               thd->security_context()->host_or_ip().str, "user");
      return false;
    }
  }
  List_of_auth_id_refs authid_list;
  if (have_using_clause) {
    for (const LEX_USER &user : *using_users) {
      authid_list.emplace_back(user.user, user.host);
    }
  }

  LEX_USER *tmp_user = const_cast<LEX_USER *>(for_user);
  tmp_user = get_current_user(thd, tmp_user);
  return mysql_show_grants(thd, tmp_user, authid_list, show_mandatory_roles,
                           have_using_clause);
}

bool Sql_cmd_show::execute(THD *thd) {
  DBUG_TRACE;

  thd->clear_current_query_costs();
  bool res = show_precheck(thd, thd->lex, true);
  if (!res) res = execute_show(thd, thd->lex->query_tables);
  thd->save_current_query_costs();

  return res;
}

bool Sql_cmd_show::prepare(THD *thd) {
  DBUG_TRACE;

  if (Sql_cmd::prepare(thd)) return true;

  bool rc = mysql_test_show(get_owner(), thd->lex->query_tables);
  return rc;
}
