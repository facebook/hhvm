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

#include "sql/dd/impl/transaction_impl.h"

#include <stddef.h>
#include <new>
#include <utility>

#include "my_base.h"
#include "my_dbug.h"
#include "sql/dd/impl/raw/raw_table.h"  // dd::Raw_table
#include "sql/query_options.h"
#include "sql/sql_base.h"  // MYSQL_LOCK_IGNORE_TIMEOUT
#include "sql/sql_lex.h"
#include "sql/system_variables.h"
#include "sql/table.h"

namespace dd {

///////////////////////////////////////////////////////////////////////////

Open_dictionary_tables_ctx::~Open_dictionary_tables_ctx() {
  // Delete Raw_table instances.
  for (Object_table_map::iterator it = m_tables.begin(); it != m_tables.end();
       ++it) {
    delete it->second;
  }
}

void Open_dictionary_tables_ctx::add_table(const String_type &name) {
  if (!m_tables[name])
    m_tables[name] = new (std::nothrow) Raw_table(m_lock_type, name);
}

bool Open_dictionary_tables_ctx::open_tables() {
  DBUG_TRACE;

  DBUG_ASSERT(!m_tables.empty());

  Object_table_map::iterator it = m_tables.begin();
  Object_table_map::iterator it_next = m_tables.begin();

  TABLE_LIST *table_list = it_next->second->get_table_list();

  ++it_next;

  // fprintf(stderr, "--> open_tables():\n");
  while (it_next != m_tables.end()) {
    // fprintf(stderr, "  - '%s'\n", it->first.c_str());

    it->second->get_table_list()->next_global =
        it_next->second->get_table_list();

    ++it;
    ++it_next;
  }

  // TODO: KILL_BAD_DATA has now been removed
  //
  // We need MYSQL_OPEN_IGNORE_KILLED so that we can operate on
  // new DD even when thd->killed is set. E.g., if ALTER TABLE
  // fails due to some reason (could be in strict mode)
  // THD::raise_condition() sets thd->killed= THD::KILL_BAD_DATA.
  // As part of ALTER TABLE cleanup procedure, there are needs
  // to update DD tables, e.g., removing temporary table created
  // for ALTER TABLE. To enable open_table() to work even when
  // thd->killed is set, we need to this flag to be set.
  //
  // TODO-WL7743: Double check if this is ok.
  // IMO, operations on new DD should not be affected by thd->killed
  // flag, and should be safe to ignore.
  //
  // This flag should be removed once we implement WL#7743, where
  // we try to commit/rollback transaction only at the end of
  // statement. This should avoid invoking DD calls once
  // thd->killed flag is set. Note sure completely.
  //
  // FLUSH TABLES is ignored for the DD tables. Hence the setting
  // the MYSQL_OPEN_IGNORE_FLUSH flag.

  const uint flags =
      (MYSQL_LOCK_IGNORE_TIMEOUT | MYSQL_OPEN_IGNORE_KILLED |
       MYSQL_OPEN_IGNORE_FLUSH |
       (m_ignore_global_read_lock ? MYSQL_OPEN_IGNORE_GLOBAL_READ_LOCK : 0));
  uint counter;

  if (::open_tables(m_thd, &table_list, &counter, flags)) return true;

  /*
    Data-dictionary tables must use storage engine supporting attachable
    transactions.

    We also disable auto-increment locking for data-dictionary tables.
    It leads to increased chances of deadlocks during atomic DDL and
    is not really necessary for replicating data-dictionary changes
    (as we do not aim to replicate exact IDs in the data-dictionary).
  */
  for (TABLE_LIST *t = table_list; t; t = t->next_global) {
    DBUG_ASSERT(t->table->file->ha_table_flags() &
                HA_ATTACHABLE_TRX_COMPATIBLE);
    if (t->table->file->ha_extra(HA_EXTRA_NO_AUTOINC_LOCKING)) return true;
  }

  // Lock the tables.
  if (lock_tables(m_thd, table_list, counter, flags)) return true;

  return false;
}

Raw_table *Open_dictionary_tables_ctx::get_table(
    const String_type &name) const {
  Object_table_map::const_iterator it = m_tables.find(name);

  return it == m_tables.end() ? NULL : it->second;
}

///////////////////////////////////////////////////////////////////////////

Update_dictionary_tables_ctx::Update_dictionary_tables_ctx(THD *thd)
    : otx(thd, TL_WRITE),
      m_thd(thd),
      m_kill_immunizer(thd),
      m_lex_saved(nullptr),
      m_saved_in_sub_stmt(thd->in_sub_stmt),
      m_saved_time_zone_used(thd->time_zone_used),
      m_saved_auto_increment_increment(
          thd->variables.auto_increment_increment) {
  m_saved_check_for_truncated_fields = m_thd->check_for_truncated_fields;

  m_saved_mode = m_thd->variables.sql_mode;
  m_thd->variables.sql_mode = 0;  // Reset during DD operations

  // Save old lex
  m_lex_saved = m_thd->lex;
  m_thd->lex = new (m_thd->mem_root) st_lex_local;
  lex_start(m_thd);

  m_thd->reset_n_backup_open_tables_state(&m_open_tables_state_backup,
                                          Open_tables_state::SYSTEM_TABLES);

  if ((m_saved_binlog_row_based = m_thd->is_current_stmt_binlog_format_row()))
    m_thd->clear_current_stmt_binlog_format_row();

  // Disable bin logging
  m_saved_options = m_thd->variables.option_bits;
  m_thd->variables.option_bits &= ~OPTION_BIN_LOG;

  // Set bit to indicate that the thread is updating the data dictionary tables.
  m_thd->variables.option_bits |= OPTION_DD_UPDATE_CONTEXT;

  /*
    In @@autocommit=1 mode InnoDB automatically commits its transaction when
    all InnoDB tables in the statement are closed. Particularly, this can
    happen when ~Update_dictionary_tables_ctx() closes data-dictionary tables
    and there are no other InnoDB tables open by the statement.
    Since normally we decide whether we want to commit or rollback changes to
    data-dictionary sometime after this point we need to avoid this happening.
    So we disallow usage of Update_dictionary_tables_ctx in @@autocommit=1
    mode. This means that all DDL statements using Update_dictionary_tables_ctx
    to update data-dictionary need to turn off @@autocommit for its duration.
  */
  DBUG_ASSERT((m_thd->variables.option_bits & OPTION_NOT_AUTOCOMMIT) &&
              !(m_thd->variables.option_bits & OPTION_AUTOCOMMIT));

  // Store current intervals.
  m_thd->auto_inc_intervals_in_cur_stmt_for_binlog.swap(
      &m_auto_inc_intervals_in_cur_stmt_for_binlog_saved);

  // Store current interval.
  m_thd->auto_inc_intervals_forced.swap(&m_auto_inc_intervals_forced_saved);

  m_thd->variables.auto_increment_increment = 1;

  m_thd->in_sub_stmt = 0;

  m_thd->time_zone_used = false;
}

Update_dictionary_tables_ctx::~Update_dictionary_tables_ctx() {
  // Close all the tables that are open till now.
  close_thread_tables(m_thd);

  m_thd->check_for_truncated_fields = m_saved_check_for_truncated_fields;
  m_thd->variables.sql_mode = m_saved_mode;

  m_thd->variables.option_bits = m_saved_options;

  if (m_saved_binlog_row_based) m_thd->set_current_stmt_binlog_format_row();
  m_saved_binlog_row_based = false;

  m_thd->restore_backup_open_tables_state(&m_open_tables_state_backup);

  // Restore the lex
  lex_end(m_thd->lex);
  delete (st_lex_local *)m_thd->lex;
  m_thd->lex = m_lex_saved;

  // Restore auto_inc_intervals_in_cur_stmt_for_binlog
  m_auto_inc_intervals_in_cur_stmt_for_binlog_saved.empty();  // XXX: remove?
  m_auto_inc_intervals_in_cur_stmt_for_binlog_saved.swap(
      &m_thd->auto_inc_intervals_in_cur_stmt_for_binlog);

  // Restore forced auto-inc interva.
  m_auto_inc_intervals_forced_saved.swap(&m_thd->auto_inc_intervals_forced);

  m_thd->variables.auto_increment_increment = m_saved_auto_increment_increment;

  m_thd->in_sub_stmt = m_saved_in_sub_stmt;

  m_thd->time_zone_used = m_saved_time_zone_used;
}

}  // namespace dd
