/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/rpl_table_access.h"

#include <stddef.h>

#include "lex_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysqld_error.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/current_thd.h"  // my_thread_set_THR_THD
#include "sql/handler.h"      // ha_rollback_trans
#include "sql/log.h"
#include "sql/sql_base.h"   // close_thread_tables
#include "sql/sql_class.h"  // THD
#include "sql/sql_lex.h"    // Query_tables_list
#include "sql/table.h"      // TABLE_LIST

bool System_table_access::open_table(THD *thd, const LEX_CSTRING dbstr,
                                     const LEX_CSTRING tbstr,
                                     uint max_num_field,
                                     enum thr_lock_type lock_type,
                                     TABLE **table,
                                     Open_tables_backup *backup) {
  Query_tables_list query_tables_list_backup;

  DBUG_TRACE;
  before_open(thd);

  /*
    We need to use new Open_tables_state in order not to be affected
    by LOCK TABLES/prelocked mode.
    Also in order not to break execution of current statement we also
    have to backup/reset/restore Query_tables_list part of LEX, which
    is accessed and updated in the process of opening and locking
    tables.
  */
  thd->lex->reset_n_backup_query_tables_list(&query_tables_list_backup);
  thd->reset_n_backup_open_tables_state(backup,
                                        Open_tables_state::SYSTEM_TABLES);

  TABLE_LIST tables(dbstr.str, dbstr.length, tbstr.str, tbstr.length, tbstr.str,
                    lock_type);

  tables.open_strategy = TABLE_LIST::OPEN_IF_EXISTS;

  if (!open_n_lock_single_table(thd, &tables, tables.lock_descriptor().type,
                                m_flags)) {
    close_thread_tables(thd);
    thd->restore_backup_open_tables_state(backup);
    thd->lex->restore_backup_query_tables_list(&query_tables_list_backup);
    if (thd->is_operating_gtid_table_implicitly)
      LogErr(WARNING_LEVEL, ER_RPL_GTID_TABLE_CANNOT_OPEN, dbstr.str,
             tbstr.str);
    else
      my_error(ER_NO_SUCH_TABLE, MYF(0), dbstr.str, tbstr.str);
    return true;
  }

  if (tables.table->s->fields < max_num_field) {
    /*
      Safety: this can only happen if someone started the server and then
      altered the table.
    */
    ha_rollback_trans(thd, false);
    close_thread_tables(thd);
    thd->restore_backup_open_tables_state(backup);
    thd->lex->restore_backup_query_tables_list(&query_tables_list_backup);
    my_error(ER_COL_COUNT_DOESNT_MATCH_CORRUPTED_V2, MYF(0),
             tables.table->s->db.str, tables.table->s->table_name.str,
             max_num_field, tables.table->s->fields);
    return true;
  }

  thd->lex->restore_backup_query_tables_list(&query_tables_list_backup);

  *table = tables.table;
  tables.table->use_all_columns();
  return false;
}

bool System_table_access::close_table(THD *thd, TABLE *table,
                                      Open_tables_backup *backup, bool error,
                                      bool need_commit) {
  Query_tables_list query_tables_list_backup;
  bool res = false;

  DBUG_TRACE;

  if (table) {
    if (error)
      res = ha_rollback_trans(thd, false);
    else {
      /*
        To make the commit not to block with global read lock set
        "ignore_global_read_lock" flag to true.
       */
      res = ha_commit_trans(thd, false, true);
    }
    if (need_commit) {
      if (error)
        res = ha_rollback_trans(thd, true) || res;
      else {
        /*
          To make the commit not to block with global read lock set
          "ignore_global_read_lock" flag to true.
         */
        res = ha_commit_trans(thd, true, true) || res;
      }
    }
    /*
      In order not to break execution of current statement we have to
      backup/reset/restore Query_tables_list part of LEX, which is
      accessed and updated in the process of closing tables.
    */
    thd->lex->reset_n_backup_query_tables_list(&query_tables_list_backup);
    close_thread_tables(thd);
    thd->lex->restore_backup_query_tables_list(&query_tables_list_backup);
    thd->restore_backup_open_tables_state(backup);
  }

  DBUG_EXECUTE_IF("simulate_flush_commit_error", {
    my_printf_error(ER_ERROR_DURING_FLUSH_LOGS,
                    ER_THD(thd, ER_ERROR_DURING_FLUSH_LOGS), MYF(ME_FATALERROR),
                    true);
    LogErr(ERROR_LEVEL, ER_ERROR_DURING_FLUSH_LOG_COMMIT_PHASE, true);
    return true;
  });

  return res;
}

THD *System_table_access::create_thd() {
  THD *thd = nullptr;
  thd = new THD;
  thd->thread_stack = (char *)&thd;
  thd->store_globals();
  thd->security_context()->skip_grants();

  return (thd);
}

void System_table_access::drop_thd(THD *thd) {
  DBUG_TRACE;

  delete thd;
  current_thd = nullptr;
}
