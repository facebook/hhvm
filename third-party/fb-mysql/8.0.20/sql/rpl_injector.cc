/* Copyright (c) 2006, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/rpl_injector.h"

#include "my_compiler.h"
#include "mysql/service_mysql_alloc.h"
#include "sql/binlog.h"     // mysql_bin_log
#include "sql/log_event.h"  // Incident_log_event
#include "sql/mdl.h"
#include "sql/psi_memory_key.h"
#include "sql/sql_base.h"     // close_thread_tables
#include "sql/sql_class.h"    // THD
#include "sql/transaction.h"  // trans_begin

/*
  injector::transaction - member definitions
*/

/* inline since it's called below */
inline injector::transaction::transaction(MYSQL_BIN_LOG *log, THD *thd)
    : m_state(START_STATE), m_thd(thd) {
  /*
     Default initialization of m_start_pos (which initializes it to garbage).
     We need to fill it in using the code below.
  */
  LOG_INFO log_info;
  log->get_current_log(&log_info);
  /* !!! binlog_pos does not follow RAII !!! */
  m_start_pos.m_file_name =
      my_strdup(key_memory_binlog_pos, log_info.log_file_name, MYF(0));
  m_start_pos.m_file_pos = log_info.pos;

  if (unlikely(m_start_pos.m_file_name == nullptr)) {
    m_thd = nullptr;
    return;
  }

  /*
     Next pos is unknown until after commit of the Binlog transaction
  */
  m_next_pos.m_file_name = nullptr;
  m_next_pos.m_file_pos = 0;

  /*
    Ensure we don't pick up this thd's last written Binlog pos in
    empty-transaction-commit cases.
    This is not ideal, as it zaps this information for any other
    usage (e.g. WL4047)
    Potential improvement : save the 'old' next pos prior to
    commit, and restore on error.
  */
  m_thd->clear_next_event_pos();

  trans_begin(m_thd);
}

injector::transaction::~transaction() {
  if (!good()) return;

  /* Needed since my_free expects a 'char*' (instead of 'void*'). */
  char *const start_pos_memory = const_cast<char *>(m_start_pos.m_file_name);

  if (start_pos_memory) {
    my_free(start_pos_memory);
  }

  char *const next_pos_memory = const_cast<char *>(m_next_pos.m_file_name);
  if (next_pos_memory) {
    my_free(next_pos_memory);
  }
}

/**
   @retval 0 transaction committed
   @retval 1 transaction rolled back
 */
int injector::transaction::commit() {
  DBUG_TRACE;
  int error = m_thd->binlog_flush_pending_rows_event(true);
  /*
    Cluster replication does not preserve statement or
    transaction boundaries of the master.  Instead, a new
    transaction on replication slave is started when a new GCI
    (global checkpoint identifier) is issued, and is committed
    when the last event of the check point has been received and
    processed. This ensures consistency of each cluster in
    cluster replication, and there is no requirement for stronger
    consistency: MySQL replication is asynchronous with other
    engines as well.

    A practical consequence of that is that row level replication
    stream passed through the injector thread never contains
    COMMIT events.
    Here we should preserve the server invariant that there is no
    outstanding statement transaction when the normal transaction
    is committed by committing the statement transaction
    explicitly.
  */
  trans_commit_stmt(m_thd);
  if (!trans_commit(m_thd)) {
    close_thread_tables(m_thd);
    m_thd->mdl_context.release_transactional_locks();
  }

  /* Copy next position out into our next pos member */
  if ((error == 0) && (m_thd->binlog_next_event_pos.file_name != nullptr) &&
      ((m_next_pos.m_file_name = my_strdup(
            key_memory_binlog_pos, m_thd->binlog_next_event_pos.file_name,
            MYF(0))) != nullptr)) {
    m_next_pos.m_file_pos = m_thd->binlog_next_event_pos.pos;
  } else {
    /* Error, problem copying etc. */
    m_next_pos.m_file_name = nullptr;
    m_next_pos.m_file_pos = 0;
  }

  return error;
}

int injector::transaction::rollback() {
  DBUG_TRACE;
  trans_rollback_stmt(m_thd);
  if (!trans_rollback(m_thd)) {
    close_thread_tables(m_thd);
    if (!m_thd->locked_tables_mode)
      m_thd->mdl_context.release_transactional_locks();
  }
  return 0;
}

int injector::transaction::use_table(server_id_type sid, table tbl) {
  DBUG_TRACE;

  int error;

  if ((error = check_state(TABLE_STATE))) return error;

  server_id_type save_id = m_thd->server_id;
  m_thd->set_server_id(sid);
  error = m_thd->binlog_write_table_map(tbl.get_table(), tbl.is_transactional(),
                                        false);
  m_thd->set_server_id(save_id);
  return error;
}

int injector::transaction::write_row(server_id_type sid, table tbl,
                                     MY_BITMAP const *cols, record_type record,
                                     const unsigned char *extra_row_info) {
  DBUG_TRACE;

  int error = check_state(ROW_STATE);
  if (error) return error;

  server_id_type save_id = m_thd->server_id;
  m_thd->set_server_id(sid);
  table::save_sets saveset(tbl, cols, cols);

  error = m_thd->binlog_write_row(tbl.get_table(), tbl.is_transactional(),
                                  record, extra_row_info);
  m_thd->set_server_id(save_id);
  return error;
}

int injector::transaction::write_row(server_id_type sid, table tbl,
                                     MY_BITMAP const *cols,
                                     record_type record) {
  return write_row(sid, tbl, cols, record, nullptr);
}

int injector::transaction::delete_row(server_id_type sid, table tbl,
                                      MY_BITMAP const *cols, record_type record,
                                      const unsigned char *extra_row_info) {
  DBUG_TRACE;

  int error = check_state(ROW_STATE);
  if (error) return error;

  server_id_type save_id = m_thd->server_id;
  m_thd->set_server_id(sid);
  table::save_sets saveset(tbl, cols, cols);
  error = m_thd->binlog_delete_row(tbl.get_table(), tbl.is_transactional(),
                                   record, extra_row_info);
  m_thd->set_server_id(save_id);
  return error;
}

int injector::transaction::delete_row(server_id_type sid, table tbl,
                                      MY_BITMAP const *cols,
                                      record_type record) {
  return delete_row(sid, tbl, cols, record, nullptr);
}

int injector::transaction::update_row(server_id_type sid, table tbl,
                                      MY_BITMAP const *before_cols,
                                      MY_BITMAP const *after_cols,
                                      record_type before, record_type after,
                                      const unsigned char *extra_row_info) {
  DBUG_TRACE;

  int error = check_state(ROW_STATE);
  if (error) return error;

  server_id_type save_id = m_thd->server_id;
  m_thd->set_server_id(sid);
  // The read- and write sets with autorestore (in the destructor)
  table::save_sets saveset(tbl, before_cols, after_cols);

  error = m_thd->binlog_update_row(tbl.get_table(), tbl.is_transactional(),
                                   before, after, extra_row_info);
  m_thd->set_server_id(save_id);
  return error;
}

int injector::transaction::update_row(server_id_type sid, table tbl,
                                      MY_BITMAP const *cols, record_type before,
                                      record_type after) {
  return update_row(sid, tbl, cols, cols, before, after, nullptr);
}

injector::transaction::binlog_pos injector::transaction::start_pos() const {
  return m_start_pos;
}

injector::transaction::binlog_pos injector::transaction::next_pos() const {
  return m_next_pos;
}

/*
  injector - member definitions
*/

/* This constructor is called below */
inline injector::injector() {}

static injector *s_injector = nullptr;
injector *injector::instance() {
  if (s_injector == nullptr) s_injector = new injector;
  /* "There can be only one [instance]" */
  return s_injector;
}

void injector::free_instance() {
  injector *inj = s_injector;

  if (inj != nullptr) {
    s_injector = nullptr;
    delete inj;
  }
}

void injector::new_trans(THD *thd, injector::transaction *ptr) {
  DBUG_TRACE;
  /*
    Currently, there is no alternative to using 'mysql_bin_log' since that
    is hardcoded into the way the handler is using the binary log.
  */
  transaction trans(&mysql_bin_log, thd);
  ptr->swap(trans);
}

int injector::record_incident(
    THD *thd, binary_log::Incident_event::enum_incident incident,
    LEX_CSTRING const message) {
  Incident_log_event ev(thd, incident, message);
  return mysql_bin_log.write_incident(&ev, thd, true /*need_lock_log=true*/,
                                      message.str);
}
