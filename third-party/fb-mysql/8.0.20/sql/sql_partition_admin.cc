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

#include "sql/sql_partition_admin.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <memory>

#include "lex_string.h"
#include "m_ctype.h"
#include "my_base.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_sys.h"
#include "my_thread_local.h"
#include "mysql/plugin.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"            // check_access
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client
#include "sql/dd/properties.h"               // dd::Properties
#include "sql/dd/types/table.h"              // dd::Table
#include "sql/dd_table_share.h"              // open_table_def
#include "sql/debug_sync.h"                  // DEBUG_SYNC
#include "sql/handler.h"
#include "sql/log.h"
#include "sql/mdl.h"
#include "sql/mysqld.h"          // opt_log_slow_admin_statements
#include "sql/partition_info.h"  // class partition_info etc.
#include "sql/partitioning/partition_handler.h"  // Partition_handler
#include "sql/sql_base.h"                        // open_and_lock_tables, etc
#include "sql/sql_class.h"                       // THD
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_partition.h"
#include "sql/sql_table.h"  // mysql_alter_table, etc.
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/transaction.h"  // trans_commit_stmt
#include "sql_string.h"
#include "thr_lock.h"

class partition_element;

namespace dd {
class Table;
}  // namespace dd

bool Sql_cmd_alter_table_exchange_partition::execute(THD *thd) {
  /* Moved from mysql_execute_command */
  LEX *lex = thd->lex;
  /* first SELECT_LEX (have special meaning for many of non-SELECTcommands) */
  SELECT_LEX *select_lex = lex->select_lex;
  /* first table of first SELECT_LEX */
  TABLE_LIST *first_table = select_lex->table_list.first;
  /*
    Code in mysql_alter_table() may modify its HA_CREATE_INFO argument,
    so we have to use a copy of this structure to make execution
    prepared statement- safe. A shallow copy is enough as no memory
    referenced from this structure will be modified.
    @todo move these into constructor...
  */
  HA_CREATE_INFO create_info(*lex->create_info);
  Alter_info alter_info(*m_alter_info, thd->mem_root);
  ulong priv_needed = ALTER_ACL | DROP_ACL | INSERT_ACL | CREATE_ACL;

  DBUG_TRACE;

  if (thd->is_fatal_error()) /* out of memory creating a copy of alter_info */
    return true;

  /* also check the table to be exchanged with the partition */
  DBUG_ASSERT(alter_info.flags & Alter_info::ALTER_EXCHANGE_PARTITION);

  if (check_access(thd, priv_needed, first_table->db,
                   &first_table->grant.privilege,
                   &first_table->grant.m_internal, false, false) ||
      check_access(thd, priv_needed, first_table->next_local->db,
                   &first_table->next_local->grant.privilege,
                   &first_table->next_local->grant.m_internal, false, false))
    return true;

  if (check_grant(thd, priv_needed, first_table, false, UINT_MAX, false))
    return true;

  /* Not allowed with EXCHANGE PARTITION */
  DBUG_ASSERT(!create_info.data_file_name && !create_info.index_file_name);

  thd->enable_slow_log = opt_log_slow_admin_statements;
  return exchange_partition(thd, first_table, &alter_info);
}

/**
  @brief Checks that the tables will be able to be used for EXCHANGE PARTITION.
  @param table      Non partitioned table.
  @param part_table Partitioned table.

  @retval false if OK, otherwise error is reported and true is returned.
*/
static bool check_exchange_partition(TABLE *table, TABLE *part_table) {
  DBUG_TRACE;

  /* Both tables must exist */
  if (!part_table || !table) {
    my_error(ER_CHECK_NO_SUCH_TABLE, MYF(0));
    return true;
  }

  /* The first table must be partitioned, and the second must not */
  if (!part_table->part_info) {
    my_error(ER_PARTITION_MGMT_ON_NONPARTITIONED, MYF(0));
    return true;
  }
  if (table->part_info) {
    my_error(ER_PARTITION_EXCHANGE_PART_TABLE, MYF(0),
             table->s->table_name.str);
    return true;
  }

  if (!part_table->file->ht->partition_flags ||
      !(part_table->file->ht->partition_flags() & HA_CAN_EXCHANGE_PARTITION)) {
    my_error(ER_PARTITION_MGMT_ON_NONPARTITIONED, MYF(0));
    return true;
  }

  if (table->file->ht != part_table->part_info->default_engine_type) {
    my_error(ER_MIX_HANDLER_ERROR, MYF(0));
    return true;
  }

  /* Verify that table is not tmp table, partitioned tables cannot be tmp. */
  if (table->s->tmp_table != NO_TMP_TABLE) {
    my_error(ER_PARTITION_EXCHANGE_TEMP_TABLE, MYF(0),
             table->s->table_name.str);
    return true;
  }

  /* The table cannot have foreign keys constraints or be referenced */
  if (table->s->foreign_keys || table->s->foreign_key_parents) {
    my_error(ER_PARTITION_EXCHANGE_FOREIGN_KEY, MYF(0),
             table->s->table_name.str);
    return true;
  }
  return false;
}

/**
  @brief Compare table structure/options between a non partitioned table
  and a specific partition of a partitioned table.

  @param thd        Thread object.
  @param table      Non partitioned table.
  @param part_table Partitioned table.
  @param part_elem  Partition element to use for partition specific compare.
  @param part_id    Id of specific partition
*/
static bool compare_table_with_partition(THD *thd, TABLE *table,
                                         TABLE *part_table,
                                         partition_element *part_elem,
                                         uint part_id) {
  HA_CREATE_INFO table_create_info, part_create_info;
  Alter_info part_alter_info(thd->mem_root);
  Alter_table_ctx part_alter_ctx;  // Not used
  DBUG_TRACE;

  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  const dd::Table *part_table_def = nullptr;
  if (!part_table->s->tmp_table) {
    if (thd->dd_client()->acquire(part_table->s->db.str,
                                  part_table->s->table_name.str,
                                  &part_table_def)) {
      return true;
    }
    // Should not happen, we know the table exists and can be opened.
    DBUG_ASSERT(part_table_def != nullptr);
  }

  bool metadata_equal = false;

  update_create_info_from_table(&table_create_info, table);
  /* get the current auto_increment value */
  table->file->update_create_info(&table_create_info);
  /* mark all columns used, since they are used when preparing the new table */
  part_table->use_all_columns();
  table->use_all_columns();

  /* db_type is not set in prepare_alter_table */
  part_create_info.db_type = part_table->part_info->default_engine_type;

  if (mysql_prepare_alter_table(thd, part_table_def, part_table,
                                &part_create_info, &part_alter_info,
                                &part_alter_ctx)) {
    my_error(ER_TABLES_DIFFERENT_METADATA, MYF(0));
    return true;
  }

  /*
    Since we exchange the partition with the table, allow exchanging
    auto_increment value as well.
  */
  part_create_info.auto_increment_value =
      table_create_info.auto_increment_value;

  /* Check compatible row_types and set create_info accordingly. */
  Partition_handler *part_handler;
  part_handler = part_table->file->get_partition_handler();
  auto part_row_type =
      part_handler->get_partition_row_type(part_table_def, part_id);

  if (part_row_type != table->s->real_row_type) {
    my_error(ER_PARTITION_EXCHANGE_DIFFERENT_OPTION, MYF(0), "ROW_FORMAT");
    return true;
  }
  part_create_info.row_type = table->s->row_type;

  /*
    NOTE: ha_blackhole does not support check_if_compatible_data,
    so this always fail for blackhole tables.
    ha_myisam compares pointers to verify that DATA/INDEX DIRECTORY is
    the same, so any table using data/index_file_name will fail.
  */
  if (mysql_compare_tables(table, &part_alter_info, &part_create_info,
                           &metadata_equal)) {
    my_error(ER_TABLES_DIFFERENT_METADATA, MYF(0));
    return true;
  }

  DEBUG_SYNC(thd, "swap_partition_after_compare_tables");
  if (!metadata_equal) {
    my_error(ER_TABLES_DIFFERENT_METADATA, MYF(0));
    return true;
  }
  DBUG_ASSERT(table->s->db_create_options == part_table->s->db_create_options);
  DBUG_ASSERT(table->s->db_options_in_use == part_table->s->db_options_in_use);

  if (table_create_info.avg_row_length != part_create_info.avg_row_length) {
    my_error(ER_PARTITION_EXCHANGE_DIFFERENT_OPTION, MYF(0), "AVG_ROW_LENGTH");
    return true;
  }

  if (table_create_info.table_options != part_create_info.table_options) {
    my_error(ER_PARTITION_EXCHANGE_DIFFERENT_OPTION, MYF(0), "TABLE OPTION");
    return true;
  }

  if (table->s->table_charset != part_table->s->table_charset) {
    my_error(ER_PARTITION_EXCHANGE_DIFFERENT_OPTION, MYF(0), "CHARACTER SET");
    return true;
  }

  /*
    NOTE: We do not support update of frm-file, i.e. change
    max/min_rows, data/index_file_name etc.
    The workaround is to use REORGANIZE PARTITION to rewrite
    the frm file and then use EXCHANGE PARTITION when they are the same.
  */
  if (compare_partition_options(&table_create_info, part_elem)) return true;

  return false;
}

/**
  @brief Swap places between a partition and a table.

  @details Verify that the tables are compatible (same engine, definition etc),
  verify that all rows in the table will fit in the partition,
  if all OK, rename table to tmp name, rename partition to table
  and finally rename tmp name to partition.

  1) Take upgradable mdl, open tables and then lock them (inited in parse)
  2) Verify that metadata matches
  3) verify data
  4) Upgrade to exclusive mdl for both tables
  5) Rename table <-> partition
  6) Rely on close_thread_tables to release mdl and table locks

  @param thd            Thread handle
  @param table_list     Table where the partition exists as first table,
                        Table to swap with the partition as second table
  @param alter_info     Contains partition name to swap

  @note This is a DDL operation so triggers will not be used.
*/
bool Sql_cmd_alter_table_exchange_partition::exchange_partition(
    THD *thd, TABLE_LIST *table_list, Alter_info *alter_info) {
  TABLE *part_table, *swap_table;
  TABLE_LIST *swap_table_list;
  partition_element *part_elem;
  String *partition_name;
  uint swap_part_id;
  Alter_table_prelocking_strategy alter_prelocking_strategy;
  uint table_counter;
  DBUG_TRACE;
  DBUG_ASSERT(alter_info->flags & Alter_info::ALTER_EXCHANGE_PARTITION);

  /* Don't allow to exchange with log table */
  swap_table_list = table_list->next_local;
  if (query_logger.check_if_log_table(swap_table_list, false)) {
    my_error(ER_WRONG_USAGE, MYF(0), "PARTITION", "log table");
    return true;
  }

  /*
    Currently no MDL lock that allows both read and write and is upgradeable
    to exclusive, so leave the lock type to TL_WRITE_ALLOW_READ also on the
    partitioned table.

    TODO: add MDL lock that allows both read and write and is upgradable to
    exclusive lock. This would allow to continue using the partitioned table
    also with update/insert/delete while the verification of the swap table
    is running.
  */

  /*
    NOTE: It is not possible to exchange a crashed partition/table since
    we need some info from the engine, which we can only access after open,
    to be able to verify the structure/metadata.
  */
  table_list->mdl_request.set_type(MDL_SHARED_NO_WRITE);
  if (open_tables(thd, &table_list, &table_counter, 0,
                  &alter_prelocking_strategy))
    return true;

  part_table = table_list->table;
  swap_table = swap_table_list->table;

  if (check_exchange_partition(swap_table, part_table)) return true;

  /* set lock pruning on first table */
  partition_name = alter_info->partition_names.head();
  if (table_list->table->part_info->set_named_partition_bitmap(
          partition_name->c_ptr(), partition_name->length()))
    return true;

  if (lock_tables(thd, table_list, table_counter, 0)) return true;

  THD_STAGE_INFO(thd, stage_verifying_table);

  if (!(part_elem = part_table->part_info->get_part_elem(
            partition_name->c_ptr(), &swap_part_id))) {
    my_error(ER_UNKNOWN_PARTITION, MYF(0), partition_name->c_ptr(),
             part_table->alias);
    return true;
  }

  if (swap_part_id == NOT_A_PARTITION_ID) {
    DBUG_ASSERT(part_table->part_info->is_sub_partitioned());
    my_error(ER_PARTITION_INSTEAD_OF_SUBPARTITION, MYF(0));
    return true;
  }

  if (compare_table_with_partition(thd, swap_table, part_table, part_elem,
                                   swap_part_id))
    return true;

  /* Table and partition has same structure/options */

  if (alter_info->with_validation != Alter_info::ALTER_WITHOUT_VALIDATION) {
    thd_proc_info(thd, "verifying data with partition");

    if (verify_data_with_partition(swap_table, part_table, swap_part_id)) {
      return true;
    }
  }

  /* OK to exchange */

  /*
    Get exclusive mdl lock on both tables, alway the non partitioned table
    first. Remember the tickets for downgrading locks later.
  */
  auto downgrade_mdl_lambda = [thd](MDL_ticket *ticket) {
    if (thd->locked_tables_mode)
      ticket->downgrade_lock(MDL_SHARED_NO_READ_WRITE);
  };
  std::unique_ptr<MDL_ticket, decltype(downgrade_mdl_lambda)>
      swap_tab_downgrade_mdl_guard(swap_table->mdl_ticket,
                                   downgrade_mdl_lambda);
  std::unique_ptr<MDL_ticket, decltype(downgrade_mdl_lambda)>
      part_tab_downgrade_mdl_guard(part_table->mdl_ticket,
                                   downgrade_mdl_lambda);

  /*
    No need to set used_partitions to only propagate
    HA_EXTRA_PREPARE_FOR_RENAME to one part since no built in engine uses
    that flag. And the action would probably be to force close all other
    instances which is what we are doing any way.
  */
  if (wait_while_table_is_used(thd, swap_table, HA_EXTRA_PREPARE_FOR_RENAME) ||
      wait_while_table_is_used(thd, part_table, HA_EXTRA_PREPARE_FOR_RENAME))
    return true;

  DEBUG_SYNC(thd, "swap_partition_after_wait");

  Partition_handler *part_handler;

  if (!(part_handler = part_table->file->get_partition_handler())) {
    my_error(ER_PARTITION_MGMT_ON_NONPARTITIONED, MYF(0));
    return true;
  }

  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  dd::Table *part_table_def = nullptr;
  dd::Table *swap_table_def = nullptr;

  if (thd->dd_client()->acquire_for_modification<dd::Table>(
          table_list->db, table_list->table_name, &part_table_def) ||
      thd->dd_client()->acquire_for_modification<dd::Table>(
          swap_table_list->db, swap_table_list->table_name, &swap_table_def))
    return true;

  /* Tables were successfully opened above. */
  DBUG_ASSERT(part_table_def != nullptr && swap_table_def != nullptr);

  if (part_table_def->options().exists("secondary_engine") ||
      swap_table_def->options().exists("secondary_engine")) {
    /* Exchange operation is not allowed for tables with secondary engine
     * since it's not currently supported by change propagation
     */
    my_error(ER_SECONDARY_ENGINE_DDL, MYF(0));
    return true;
  }

  DEBUG_SYNC(thd, "swap_partition_before_exchange");

  int ha_error = part_handler->exchange_partition(swap_part_id, part_table_def,
                                                  swap_table_def);

  if (ha_error) {
    handlerton *hton = part_table->file->ht;
    part_table->file->print_error(ha_error, MYF(0));
    // Close TABLE instances which marked as old earlier.
    close_all_tables_for_name(thd, swap_table->s, false, nullptr);
    close_all_tables_for_name(thd, part_table->s, false, nullptr);
    /*
      Rollback all possible changes to data-dictionary and SE which
      Partition_handler::exchange_partitions() might have done before
      reporting an error.
      Do this before we downgrade metadata locks.
    */
    (void)trans_rollback_stmt(thd);
    /*
      Full rollback in case we have THD::transaction_rollback_request
      and to synchronize DD state in cache and on disk (as statement
      rollback doesn't clear DD cache of modified uncommitted objects).
    */
    (void)trans_rollback(thd);
    if ((hton->flags & HTON_SUPPORTS_ATOMIC_DDL) && hton->post_ddl)
      hton->post_ddl(thd);
    (void)thd->locked_tables_list.reopen_tables(thd);
    return true;
  } else {
    if (part_table->file->ht->flags & HTON_SUPPORTS_ATOMIC_DDL) {
      handlerton *hton = part_table->file->ht;

      // Close TABLE instances which marked as old earlier.
      close_all_tables_for_name(thd, swap_table->s, false, nullptr);
      close_all_tables_for_name(thd, part_table->s, false, nullptr);

      /*
        Ensure that we call post-DDL hook and re-open tables even
        in case of error.
      */
      auto rollback_post_ddl_reopen_lambda = [hton](THD *thd_arg) {
        /*
          Rollback all possible changes to data-dictionary and SE which
          Partition_handler::exchange_partitions() might have done before
          reporting an error. Do this before we downgrade metadata locks.
        */
        (void)trans_rollback_stmt(thd_arg);
        /*
          Full rollback in case we have THD::transaction_rollback_request
          and to synchronize DD state in cache and on disk (as statement
          rollback doesn't clear DD cache of modified uncommitted objects).
        */
        (void)trans_rollback(thd_arg);
        /*
          Call SE post DDL hook. This handles both rollback and commit cases.
        */
        if (hton->post_ddl) hton->post_ddl(thd_arg);
        (void)thd_arg->locked_tables_list.reopen_tables(thd_arg);
      };

      std::unique_ptr<THD, decltype(rollback_post_ddl_reopen_lambda)>
          rollback_post_ddl_reopen_guard(thd, rollback_post_ddl_reopen_lambda);

      if (thd->dd_client()->update(part_table_def) ||
          thd->dd_client()->update(swap_table_def) ||
          write_bin_log(thd, true, thd->query().str, thd->query().length,
                        true)) {
        return true;
      }

      if (trans_commit_stmt(thd) || trans_commit_implicit(thd)) return true;
    } else {
      /*
        Close TABLE instances which were marked as old earlier and reopen
        tables. Ignore the fact that the statement might fail due to binlog
        write failure.
      */
      close_all_tables_for_name(thd, swap_table->s, false, nullptr);
      close_all_tables_for_name(thd, part_table->s, false, nullptr);
      (void)thd->locked_tables_list.reopen_tables(thd);

      if (write_bin_log(thd, true, thd->query().str, thd->query().length))
        return true;
    }
  }

  my_ok(thd);

  return false;
}

bool Sql_cmd_alter_table_analyze_partition::execute(THD *thd) {
  bool res;
  DBUG_TRACE;
  DBUG_ASSERT(m_alter_info->flags & Alter_info::ALTER_ADMIN_PARTITION);

  res = Sql_cmd_analyze_table::execute(thd);

  return res;
}

bool Sql_cmd_alter_table_check_partition::execute(THD *thd) {
  bool res;
  DBUG_TRACE;
  DBUG_ASSERT(m_alter_info->flags & Alter_info::ALTER_ADMIN_PARTITION);

  res = Sql_cmd_check_table::execute(thd);

  return res;
}

bool Sql_cmd_alter_table_optimize_partition::execute(THD *thd) {
  bool res;
  DBUG_TRACE;
  DBUG_ASSERT(m_alter_info->flags & Alter_info::ALTER_ADMIN_PARTITION);

  res = Sql_cmd_optimize_table::execute(thd);

  return res;
}

bool Sql_cmd_alter_table_repair_partition::execute(THD *thd) {
  bool res;
  DBUG_TRACE;
  DBUG_ASSERT(m_alter_info->flags & Alter_info::ALTER_ADMIN_PARTITION);

  res = Sql_cmd_repair_table::execute(thd);

  return res;
}

bool Sql_cmd_alter_table_truncate_partition::execute(THD *thd) {
  int error;
  ulonglong timeout_nsec = thd->variables.lock_wait_timeout_nsec;
  TABLE_LIST *first_table = thd->lex->select_lex->table_list.first;
  uint table_counter;
  Partition_handler *part_handler = nullptr;
  handlerton *hton;
  DBUG_TRACE;
  DBUG_ASSERT((m_alter_info->flags & (Alter_info::ALTER_ADMIN_PARTITION |
                                      Alter_info::ALTER_TRUNCATE_PARTITION)) ==
              (Alter_info::ALTER_ADMIN_PARTITION |
               Alter_info::ALTER_TRUNCATE_PARTITION));

  /* Fix the lock types (not the same as ordinary ALTER TABLE). */
  first_table->set_lock({TL_WRITE, THR_DEFAULT});
  first_table->mdl_request.set_type(MDL_EXCLUSIVE);

  /*
    Check table permissions and open it with a exclusive lock.
    Ensure it is a partitioned table and finally, upcast the
    handler and invoke the partition truncate method. Lastly,
    write the statement to the binary log if necessary.
  */

  if (check_one_table_access(thd, DROP_ACL, first_table)) return true;

  if (open_tables(thd, &first_table, &table_counter, 0)) return true;

  if (!first_table->table || first_table->is_view() ||
      !first_table->table->file->ht->partition_flags ||
      !(part_handler = first_table->table->file->get_partition_handler())) {
    my_error(ER_PARTITION_MGMT_ON_NONPARTITIONED, MYF(0));
    return true;
  }

  hton = first_table->table->file->ht;

  /*
    Prune all, but named partitions. SE can use partitions bitmap
    to understand what partitions need to be truncated. This also
    allows to avoid excessive calls to external_lock().
  */
  first_table->partition_names = &m_alter_info->partition_names;
  if (first_table->table->part_info->set_partition_bitmaps(first_table))
    return true;

  /*
    Under locked table modes we still don't have an exclusive lock.
    Hence, upgrade the lock since the handler truncate method mandates
    an exclusive metadata lock. Don't forget to downgrade the lock
    before leaving this method.
  */
  auto downgrade_mdl_lambda = [](MDL_ticket *ticket) {
    ticket->downgrade_lock(MDL_SHARED_NO_READ_WRITE);
  };
  std::unique_ptr<MDL_ticket, decltype(downgrade_mdl_lambda)>
      downgrade_mdl_guard(nullptr, downgrade_mdl_lambda);

  if (thd->locked_tables_mode) {
    MDL_ticket *ticket = first_table->table->mdl_ticket;
    if (thd->mdl_context.upgrade_shared_lock_nsec(ticket, MDL_EXCLUSIVE,
                                                  timeout_nsec))
      return true;
    downgrade_mdl_guard.reset(ticket);
  }

  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  dd::Table *table_def = nullptr;

  if (thd->dd_client()->acquire_for_modification<dd::Table>(
          first_table->db, first_table->table_name, &table_def))
    return true;

  /* Table was successfully opened above. */
  DBUG_ASSERT(table_def != nullptr);

  if (table_def->options().exists("secondary_engine")) {
    /* Truncate operation is not allowed for tables with secondary engine
     * since it's not currently supported by change propagation
     */
    my_error(ER_SECONDARY_ENGINE_DDL, MYF(0));
    return true;
  }

  if (hton->partition_flags() & HA_TRUNCATE_PARTITION_PRECLOSE) {
    /*
      Storage engine requires closing all open table instances before
      calling Partition_handler::truncate_partition().
    */
    char path[FN_REFLEN + 1];
    TABLE_SHARE share;
    TABLE table;

    /*
      Save information about which partitions need to be truncated
      before destroying partitions bitmaps along with open table
      instance.
    */
    MY_BITMAP saved_parts_map;
    if (first_table->table->part_info->init_partition_bitmap(&saved_parts_map,
                                                             thd->mem_root))
      return true;
    bitmap_copy(&saved_parts_map,
                &first_table->table->part_info->read_partitions);

    close_all_tables_for_name(thd, first_table->table->s, false, nullptr);

    /*
      Construct artificial TABLE/TABLE_SHARE and handler instances for
      calling Partition_handler::truncate_partition().
    */
    build_table_filename(path, sizeof(path) - 1, first_table->db,
                         first_table->table_name, "", 0);

    init_tmp_table_share(thd, &share, first_table->db, 0,
                         first_table->table_name, path, nullptr);

    auto free_share_lambda = [](TABLE_SHARE *share_arg) {
      free_table_share(share_arg);
    };
    std::unique_ptr<TABLE_SHARE, decltype(free_share_lambda)> free_share_guard(
        &share, free_share_lambda);

    error = open_table_def(thd, &share, *table_def);

    if (!error) {
      /*
        When db_stat is 0, we can pass nullptr as dd::Table since it
        won't be used.
      */
      destroy(&table);
      error = open_table_from_share(thd, &share, "", 0, (uint)READ_ALL, 0,
                                    &table, true, nullptr);

      if (!error) {
        auto closefrm_lambda = [](TABLE *t) { (void)closefrm(t, false); };
        std::unique_ptr<TABLE, decltype(closefrm_lambda)> closefrm_guard(
            &table, closefrm_lambda);

        bitmap_copy(&table.part_info->read_partitions, &saved_parts_map);

        part_handler = table.file->get_partition_handler();
        // We succeeded obtaining Partition_hanlder earlier.
        DBUG_ASSERT(part_handler != nullptr);

        if (!error && (error = part_handler->truncate_partition(table_def))) {
          table.file->print_error(error, MYF(0));
        }
      }
    }
  } else {
    if (lock_tables(thd, first_table, table_counter, 0)) return true;

    tdc_remove_table(thd, TDC_RT_REMOVE_NOT_OWN, first_table->db,
                     first_table->table_name, false);

    /* Invoke the handler method responsible for truncating the partition. */
    if ((error = part_handler->truncate_partition(table_def))) {
      first_table->table->file->print_error(error, MYF(0));
    }

    /*
      Since we about to update table definition in the data-dictionary below
      we need to remove its TABLE/TABLE_SHARE from TDC now.
    */
    close_all_tables_for_name(thd, first_table->table->s, false, nullptr);
  }

  if (hton->flags & HTON_SUPPORTS_ATOMIC_DDL) {
    /*
      Storage engine supporting atomic DDL can fully rollback truncate
      if any problem occurs. This will happen during statement rollback.

      In case of success we need to save dd::Table object which might
      have been updated by SE. If this step or subsequent write to binary
      log fail then statement rollback will also restore status quo ante.

      Note that Table Definition and Table Caches were invalidated above.
    */
    if (!error) {
      if (thd->dd_client()->update<dd::Table>(table_def) ||
          write_bin_log(thd, true, thd->query().str, thd->query().length, true))
        error = 1;
    }
  } else {
    /*
      For engines which don't support atomic DDL all effects of a
      truncate operation are committed even if the operation fails.
      Thus, the query must be written to the binary log.
      The exception is a unimplemented truncate method or failure
      before any call to handler::truncate() is done.
      Also, it is logged in statement format, regardless of the binlog format.
    */
    if (error != HA_ERR_WRONG_COMMAND) {
      error |=
          write_bin_log(thd, !error, thd->query().str, thd->query().length);
    }
  }

  if (!error) error = (trans_commit_stmt(thd) || trans_commit_implicit(thd));

  if (error) {
    trans_rollback_stmt(thd);
    /*
      Full rollback in case we have THD::transaction_rollback_request
      and to synchronize DD state in cache and on disk (as statement
      rollback doesn't clear DD cache of modified uncommitted objects).
    */
    trans_rollback(thd);
  }

  if ((hton->flags & HTON_SUPPORTS_ATOMIC_DDL) && hton->post_ddl)
    hton->post_ddl(thd);

  (void)thd->locked_tables_list.reopen_tables(thd);

  if (!error) my_ok(thd);

  return error;
}
