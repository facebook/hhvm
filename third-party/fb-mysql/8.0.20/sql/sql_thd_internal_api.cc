/* Copyright (c) 2015, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/sql_thd_internal_api.h"

#include <algorithm>

#include "my_config.h"

#include <fcntl.h>
#include <string.h>
#include <list>
#include <string>
#include <utility>

#include "m_string.h"
#include "mysql/components/services/psi_stage_bits.h"
#include "pfs_thread_provider.h"
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "my_macros.h"
#include "my_psi_config.h"
#include "my_sys.h"
#include "mysql/psi/mysql_file.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/mysql_socket.h"
#include "mysql/thread_type.h"
#include "sql/auth/auth_acls.h"  // SUPER_ACL
#include "sql/binlog.h"       // mysql_bin_log
#include "sql/current_thd.h"  // current_thd
#include "sql/mysqld.h"
#include "sql/mysqld_thd_manager.h"  // Global_THD_manager
#include "sql/protocol_classic.h"
#include "sql/query_options.h"
#include "sql/rpl_filter.h"  // binlog_filter
#include "sql/sql_class.h"   // THD
#include "sql/sql_lex.h"
#include "sql/sql_parse.h"  // sqlcom_can_generate_row_events
#include "sql/system_variables.h"
#include "sql/transaction_info.h"
#include "violite.h"

struct mysql_cond_t;
struct mysql_mutex_t;

void thd_init(THD *thd, char *stack_start, bool bound MY_ATTRIBUTE((unused)),
              PSI_thread_key psi_key MY_ATTRIBUTE((unused))) {
  DBUG_TRACE;
  // TODO: Purge threads currently terminate too late for them to be added.
  // Note that P_S interprets all threads with thread_id != 0 as
  // foreground threads. And THDs need thread_id != 0 to be added
  // to the global THD list.
  if (thd->system_thread != SYSTEM_THREAD_BACKGROUND) {
    thd->set_new_thread_id();
    Global_THD_manager *thd_manager = Global_THD_manager::get_instance();
    thd_manager->add_thd(thd);
  }
#ifdef HAVE_PSI_THREAD_INTERFACE
  PSI_thread *psi;
  psi = PSI_THREAD_CALL(new_thread)(psi_key, thd, thd->thread_id());
  if (bound) {
    PSI_THREAD_CALL(set_thread_os_id)(psi);
  }
  PSI_THREAD_CALL(set_thread_THD)(psi, thd);
  thd->set_psi(psi);
#endif /* HAVE_PSI_THREAD_INTERFACE */

  if (!thd->system_thread) {
    DBUG_PRINT("info",
               ("init new connection. thd: %p fd: %d", thd,
                mysql_socket_getfd(
                    thd->get_protocol_classic()->get_vio()->mysql_socket)));
  }
  thd_set_thread_stack(thd, stack_start);

  thd->store_globals();
}

THD *create_thd(bool enable_plugins, bool background_thread, bool bound,
                PSI_thread_key psi_key) {
  THD *thd = new THD(enable_plugins);
  if (background_thread) {
    thd->system_thread = SYSTEM_THREAD_BACKGROUND;
    // Skip grants and set the system_user flag in THD.
    thd->security_context()->skip_grants();
  }
  (void)thd_init(thd, reinterpret_cast<char *>(&thd), bound, psi_key);
  return thd;
}

void destroy_thd(THD *thd) {
  thd->release_resources();
#ifdef HAVE_PSI_THREAD_INTERFACE
  PSI_THREAD_CALL(delete_thread)(thd->get_psi());
  thd->set_psi(nullptr);
#endif /* HAVE_PSI_THREAD_INTERFACE */

  // TODO: Purge threads currently terminate too late for them to be added.
  if (thd->system_thread != SYSTEM_THREAD_BACKGROUND) {
    Global_THD_manager *thd_manager = Global_THD_manager::get_instance();
    thd_manager->remove_thd(thd);
  }
  delete thd;
}

void thd_set_thread_stack(THD *thd, const char *stack_start) {
  thd->thread_stack = stack_start;
}

extern "C" void thd_enter_cond(void *opaque_thd, mysql_cond_t *cond,
                               mysql_mutex_t *mutex,
                               const PSI_stage_info *stage,
                               PSI_stage_info *old_stage,
                               const char *src_function, const char *src_file,
                               int src_line) {
  THD *thd = static_cast<THD *>(opaque_thd);
  if (!thd) thd = current_thd;

  return thd->enter_cond(cond, mutex, stage, old_stage, src_function, src_file,
                         src_line);
}

extern "C" void thd_exit_cond(void *opaque_thd, const PSI_stage_info *stage,
                              const char *src_function, const char *src_file,
                              int src_line) {
  THD *thd = static_cast<THD *>(opaque_thd);
  if (!thd) thd = current_thd;

  thd->exit_cond(stage, src_function, src_file, src_line);
}

extern "C" void thd_enter_stage(void *opaque_thd,
                                const PSI_stage_info *new_stage,
                                PSI_stage_info *old_stage,
                                const char *src_function, const char *src_file,
                                int src_line) {
  THD *thd = static_cast<THD *>(opaque_thd);
  if (!thd) thd = current_thd;

  if (thd) {
    thd->enter_stage(new_stage, old_stage, src_function, src_file, src_line);
  }
}

extern "C" void thd_set_waiting_for_disk_space(void *opaque_thd,
                                               const bool waiting) {
  THD *thd = static_cast<THD *>(opaque_thd);
  if (!thd) thd = current_thd;

  if (thd != nullptr) {
    thd->set_waiting_for_disk_space(waiting);
  }
}

void thd_increment_bytes_sent(size_t length) {
  THD *thd = current_thd;
  if (likely(thd != nullptr)) { /* current_thd==NULL when close_connection()
                                calls net_send_error() */
    thd->status_var.bytes_sent += length;
  }
}

void thd_increment_bytes_received(size_t length) {
  THD *thd = current_thd;
  if (likely(thd != nullptr)) thd->status_var.bytes_received += length;
}

void thd_increment_tmp_table_bytes_written(size_t length) {
  THD *thd = current_thd;
  if (likely(thd != NULL)) {
    thd->status_var.tmp_table_bytes_written += length;
    /* store the number of bytes written into tmp table space
     * into statement metrics tables
     */
    MYSQL_INC_STATEMENT_TMP_TABLE_BYTES_WRITTEN(thd->m_statement_psi,
                                                (ulonglong)length);
  }
}

partition_info *thd_get_work_part_info(THD *thd) { return thd->work_part_info; }

enum_tx_isolation thd_get_trx_isolation(const THD *thd) {
  return thd->tx_isolation;
}

const CHARSET_INFO *thd_charset(THD *thd) { return (thd->charset()); }

LEX_CSTRING thd_query_unsafe(THD *thd) {
  DBUG_ASSERT(current_thd == thd);
  return thd->query();
}

size_t thd_query_safe(THD *thd, char *buf, size_t buflen) {
  mysql_mutex_lock(&thd->LOCK_thd_query);
  LEX_CSTRING query_string = thd->query();
  size_t len = std::min(buflen - 1, query_string.length);
  if (len > 0) strncpy(buf, query_string.str, len);
  buf[len] = '\0';
  mysql_mutex_unlock(&thd->LOCK_thd_query);
  return len;
}

void thd_query_safe(THD *thd, std::string *query) {
  mysql_mutex_lock(&thd->LOCK_thd_query);
  LEX_CSTRING query_string = thd->query();
  query->assign(query_string.str, query_string.length);
  mysql_mutex_unlock(&thd->LOCK_thd_query);
}

int thd_slave_thread(const THD *thd) { return (thd->slave_thread); }

int thd_non_transactional_update(const THD *thd) {
  return thd->get_transaction()->has_modified_non_trans_table(
      Transaction_ctx::SESSION);
}

int thd_binlog_format(const THD *thd) {
  if (mysql_bin_log.is_open() && (thd->variables.option_bits & OPTION_BIN_LOG))
    return (int)thd->variables.binlog_format;
  else
    return BINLOG_FORMAT_UNSPEC;
}

bool thd_binlog_filter_ok(const THD *thd) {
  return binlog_filter->db_ok(thd->db().str);
}

bool thd_sqlcom_can_generate_row_events(const THD *thd) {
  return sqlcom_can_generate_row_events(thd->lex->sql_command);
}

enum durability_properties thd_get_durability_property(const THD *thd) {
  enum durability_properties ret = HA_REGULAR_DURABILITY;

  if (thd != nullptr) ret = thd->durability_property;

  return ret;
}

void thd_get_autoinc(const THD *thd, ulong *off, ulong *inc) {
  *off = thd->variables.auto_increment_offset;
  *inc = thd->variables.auto_increment_increment;
}

bool thd_is_strict_mode(const THD *thd) { return thd->is_strict_sql_mode(); }

bool thd_is_error(const THD *thd) { return thd->is_error(); }

bool thd_is_super(const THD *thd) {
  return thd->security_context()->check_access(SUPER_ACL);
}

LEX_CSTRING thd_host_or_ip(THD *thd) {
  return thd->security_context()->host_or_ip();
}

bool is_mysql_datadir_path(const char *path) {
  if (path == nullptr || strlen(path) >= FN_REFLEN) return false;

  char mysql_data_dir[FN_REFLEN], path_dir[FN_REFLEN];
  convert_dirname(path_dir, path, NullS);
  convert_dirname(mysql_data_dir, mysql_unpacked_real_data_home, NullS);
  size_t mysql_data_home_len = dirname_length(mysql_data_dir);
  size_t path_len = dirname_length(path_dir);

  if (path_len < mysql_data_home_len) return true;

  if (!lower_case_file_system)
    return memcmp(mysql_data_dir, path_dir, mysql_data_home_len);

  return files_charset_info->coll->strnncoll(
      files_charset_info, reinterpret_cast<uchar *>(path_dir), path_len,
      reinterpret_cast<uchar *>(mysql_data_dir), mysql_data_home_len, true);
}

int mysql_tmpfile_path(const char *path, const char *prefix) {
  DBUG_ASSERT(path != nullptr);
  DBUG_ASSERT((strlen(path) + strlen(prefix)) <= FN_REFLEN);

  char filename[FN_REFLEN];
  int mode = O_CREAT | O_EXCL | O_RDWR;
#ifdef _WIN32
  mode |= O_TRUNC | O_SEQUENTIAL;
#endif
  File fd = mysql_file_create_temp(PSI_NOT_INSTRUMENTED, filename, path, prefix,
                                   mode, UNLINK_FILE, MYF(MY_WME));
  return fd;
}

bool thd_is_bootstrap_thread(THD *thd) {
  DBUG_ASSERT(thd);
  return (thd->is_bootstrap_system_thread() &&
          !thd->is_init_file_system_thread());
}

bool thd_is_dd_update_stmt(const THD *thd) {
  DBUG_ASSERT(thd != nullptr);

  /*
    OPTION_DD_UPDATE_CONTEXT flag is set when thread switches context to
    update data dictionary tables for the
      * DDL statements.
      * Administration statements as ANALYZE TABLE.
      * Event threads for next activation time of a event and to update status.
      * SDI import.
      ...
    So verifying OPTION_DD_UPDATE_CONTEXT flag value to check if thread is
    updating the data dictionary tables.
  */
  return (thd->variables.option_bits & OPTION_DD_UPDATE_CONTEXT);
}

my_thread_id thd_thread_id(const THD *thd) { return (thd->thread_id()); }

/**
  Get tables in the query. The tables are returned as a list of pairs
  where the first value is the dbname and the second value is the table name.

  @param thd       The MySQL internal thread pointer

  @return List of pairs: dbname, table name
 */
std::list<std::pair<const char *, const char *>> thd_get_query_tables(
    THD *thd) {
  DBUG_ASSERT(current_thd == thd);
  return thd->get_query_tables();
}

/**
  Get the value of the query attribute

  @param thd       The MySQL internal thread pointer
  @param qattr_key Name of the query attribute

  @return Value of the query attribute 'qattr_key'
*/
const std::string &thd_get_query_attr(THD *thd, const std::string &qattr_key) {
  return thd->get_query_attr(qattr_key);
}

/**
  Get the value of the connection attribute

  @param thd       The MySQL internal thread pointer
  @param cattr_key Name of the connection attribute

  @return Value of the query attribute 'cattr_key'
*/
const std::string &thd_get_connection_attr(THD *thd,
                                           const std::string &cattr_key) {
  return thd->get_connection_attr(cattr_key);
}

/**
  Get the query SQL ID

  @param thd       The MySQL internal thread pointer

  @return the SQL ID of the query
*/
const std::string thd_get_sql_id(THD *thd) {
  char sql_id_string[DIGEST_HASH_TO_STRING_LENGTH + 1];

  if (thd->mt_key_is_set(THD::SQL_ID)) {
    DIGEST_HASH_TO_STRING(thd->mt_key_value(THD::SQL_ID).data(), sql_id_string);
    sql_id_string[DIGEST_HASH_TO_STRING_LENGTH] = '\0';
  } else {
    sql_id_string[0] = '\0';
  }
  return std::string(sql_id_string);
}

/**
  Invoke yield_cond.

  @return true if should yield, false otherwise.
*/
bool thd_yield_cond(THD *thd) { return !thd->yield_cond || thd->yield_cond(); }

/**
  Get currently held mutex (for cond wait).

  @return current mutex if held, nullptr otherwise.
*/
mysql_mutex_t *thd_current_mutex(THD *thd) { return thd->current_mutex; }

/**
  Set thread priority.
*/
void thd_set_priority(THD *thd) { thd->set_thread_priority(); }

void thd_add_response_attr(THD *thd, const std::string &rattr_key,
                           const std::string &rattr_val) {
  auto tracker = thd->session_tracker.get_tracker(SESSION_RESP_ATTR_TRACKER);

  if (tracker->is_enabled()) {
    LEX_CSTRING key = {rattr_key.c_str(), rattr_key.length()};
    LEX_CSTRING value = {rattr_val.c_str(), rattr_val.length()};
    tracker->mark_as_changed(thd, &key, &value);
  }
}
