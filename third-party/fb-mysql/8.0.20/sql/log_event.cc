/*
   Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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

#define LOG_SUBSYSTEM_TAG "Repl"

#include "sql/log_event.h"

#include "my_config.h"

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "my_rapidjson_size_t.h"
#include "rapidjson/document.h"

#include "base64.h"
#include "decimal.h"
#include "libbinlogevents/export/binary_log_funcs.h"  // my_timestamp_binary_length
#include "libbinlogevents/include/debug_vars.h"
#include "libbinlogevents/include/table_id.h"
#include "libbinlogevents/include/wrapper_functions.h"
#include "m_ctype.h"
#include "my_bitmap.h"
#include "my_byteorder.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_io.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "my_systime.h"
#include "my_table_map.h"
#include "my_time.h"  // MAX_DATE_STRING_REP_LENGTH
#include "mysql.h"    // MYSQL_OPT_MAX_ALLOWED_PACKET
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/components/services/psi_statement_bits.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/udf_registration_types.h"
#include "mysql_time.h"
#include "psi_memory_key.h"
#include "query_options.h"
#include "sql/auth/auth_acls.h"
#include "sql/binlog_reader.h"
#include "sql/my_decimal.h"   // my_decimal
#include "sql/rpl_handler.h"  // RUN_HOOK
#include "sql/rpl_tblmap.h"
#include "sql/system_variables.h"
#include "sql/tc_log.h"
#include "sql_const.h"
#include "sql_string.h"
#include "template_utils.h"

#ifndef MYSQL_SERVER
#include "client/mysqlbinlog.h"
#include "sql/json_binary.h"
#include "sql/json_diff.h"  // enum_json_diff_operation
#include "sql/json_dom.h"   // Json_wrapper
#endif

#ifdef MYSQL_SERVER

#include <errno.h>
#include <fcntl.h>
#include <cstdint>
#include <new>

#include "libbinlogevents/include/binary_log.h"  // binary_log
#include "my_base.h"
#include "my_command.h"
#include "my_dir.h"  // my_dir
#include "my_sqlcommand.h"
#include "mysql/plugin.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_file.h"
#include "mysql/psi/mysql_stage.h"
#include "mysql/psi/mysql_statement.h"
#include "mysql/psi/mysql_transaction.h"
#include "mysql/psi/psi_statement.h"
#include "mysqld_error.h"
#include "prealloced_array.h"
#include "sql/auth/auth_common.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/basic_ostream.h"
#include "sql/binlog.h"
#include "sql/current_thd.h"
#include "sql/dd/types/abstract_table.h"  // dd::enum_table_type
#include "sql/debug_sync.h"               // debug_sync_set_action
#include "sql/derror.h"                   // ER_THD
#include "sql/enum_query_type.h"
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/item.h"
#include "sql/item_func.h"  // Item_func_set_user_var
#include "sql/key.h"
#include "sql/log.h"  // Log_throttle
#include "sql/mdl.h"
#include "sql/mysqld.h"  // lower_case_table_names server_uuid ...
#include "sql/protocol.h"
#include "sql/rpl_msr.h"          // channel_map
#include "sql/rpl_mts_submode.h"  // Mts_submode
#include "sql/rpl_reporting.h"
#include "sql/rpl_rli.h"      // Relay_log_info
#include "sql/rpl_rli_pdb.h"  // Slave_job_group
#include "sql/rpl_slave.h"    // use_slave_mask
#include "sql/sp_head.h"      // sp_name
#include "sql/sql_base.h"     // close_thread_tables
#include "sql/sql_bitmap.h"
#include "sql/sql_class.h"
#include "sql/sql_cmd.h"
#include "sql/sql_data_change.h"
#include "sql/sql_db.h"  // load_db_opt_by_name
#include "sql/sql_digest_stream.h"
#include "sql/sql_error.h"
#include "sql/sql_exchange.h"  // sql_exchange
#include "sql/sql_lex.h"
#include "sql/sql_list.h"        // I_List
#include "sql/sql_load.h"        // Sql_cmd_load_table
#include "sql/sql_locale.h"      // my_locale_by_number
#include "sql/sql_parse.h"       // mysql_test_parse_for_slave
#include "sql/sql_plugin.h"      // plugin_foreach
#include "sql/sql_show.h"        // append_identifier
#include "sql/sql_tablespace.h"  // Sql_cmd_tablespace
#include "sql/table.h"
#include "sql/table_trigger_dispatcher.h"
#include "sql/thd_raii.h"     // Disable_binlog_guard
#include "sql/transaction.h"  // trans_rollback_stmt
#include "sql/transaction_info.h"
#include "sql/tztime.h"  // Time_zone
#include "thr_lock.h"
#define window_size Log_throttle::LOG_THROTTLE_WINDOW_SIZE
Error_log_throttle slave_ignored_err_throttle(
    window_size, INFORMATION_LEVEL, ER_SERVER_SLAVE_IGNORED_TABLE, "Repl",
    "Error log throttle: %lu time(s) Error_code: 1237"
    " \"Slave SQL thread ignored the query because of"
    " replicate-*-table rules\" got suppressed.");
#endif /* MYSQL_SERVER */

#include "libbinlogevents/include/codecs/binary.h"
#include "libbinlogevents/include/codecs/factory.h"
#include "libbinlogevents/include/compression/iterator.h"
#include "sql/rpl_gtid.h"
#include "sql/rpl_record.h"  // enum_row_image_type, Bit_reader
#include "sql/rpl_utility.h"
#include "sql/xa_aux.h"

struct mysql_mutex_t;

PSI_memory_key key_memory_log_event;
PSI_memory_key key_memory_Incident_log_event_message;
PSI_memory_key key_memory_Rows_query_log_event_rows_query;

using std::max;
using std::min;

/* Time executing SQL for replication */
ulonglong command_slave_seconds = 0;

/**
  BINLOG_CHECKSUM variable.
*/
const char *binlog_checksum_type_names[] = {"NONE", "CRC32", NullS};

unsigned int binlog_checksum_type_length[] = {sizeof("NONE") - 1,
                                              sizeof("CRC32") - 1, 0};

TYPELIB binlog_checksum_typelib = {
    array_elements(binlog_checksum_type_names) - 1, "",
    binlog_checksum_type_names, binlog_checksum_type_length};

#define log_cs &my_charset_latin1

/*
  Size of buffer for printing a double in format %.<PREC>g

  optional '-' + optional zero + '.'  + PREC digits + 'e' + sign +
  exponent digits + '\0'
*/
#define FMT_G_BUFSIZE(PREC) (3 + (PREC) + 5 + 1)

#if defined(MYSQL_SERVER)
static int rows_event_stmt_cleanup(Relay_log_info const *rli, THD *thd);

static const char *HA_ERR(int i) {
  /*
    This function should only be called in case of an error
    was detected
   */
  DBUG_ASSERT(i != 0);
  switch (i) {
    case HA_ERR_KEY_NOT_FOUND:
      return "HA_ERR_KEY_NOT_FOUND";
    case HA_ERR_FOUND_DUPP_KEY:
      return "HA_ERR_FOUND_DUPP_KEY";
    case HA_ERR_RECORD_CHANGED:
      return "HA_ERR_RECORD_CHANGED";
    case HA_ERR_WRONG_INDEX:
      return "HA_ERR_WRONG_INDEX";
    case HA_ERR_CRASHED:
      return "HA_ERR_CRASHED";
    case HA_ERR_WRONG_IN_RECORD:
      return "HA_ERR_WRONG_IN_RECORD";
    case HA_ERR_OUT_OF_MEM:
      return "HA_ERR_OUT_OF_MEM";
    case HA_ERR_NOT_A_TABLE:
      return "HA_ERR_NOT_A_TABLE";
    case HA_ERR_WRONG_COMMAND:
      return "HA_ERR_WRONG_COMMAND";
    case HA_ERR_OLD_FILE:
      return "HA_ERR_OLD_FILE";
    case HA_ERR_NO_ACTIVE_RECORD:
      return "HA_ERR_NO_ACTIVE_RECORD";
    case HA_ERR_RECORD_DELETED:
      return "HA_ERR_RECORD_DELETED";
    case HA_ERR_RECORD_FILE_FULL:
      return "HA_ERR_RECORD_FILE_FULL";
    case HA_ERR_INDEX_FILE_FULL:
      return "HA_ERR_INDEX_FILE_FULL";
    case HA_ERR_END_OF_FILE:
      return "HA_ERR_END_OF_FILE";
    case HA_ERR_UNSUPPORTED:
      return "HA_ERR_UNSUPPORTED";
    case HA_ERR_TOO_BIG_ROW:
      return "HA_ERR_TOO_BIG_ROW";
    case HA_WRONG_CREATE_OPTION:
      return "HA_WRONG_CREATE_OPTION";
    case HA_ERR_FOUND_DUPP_UNIQUE:
      return "HA_ERR_FOUND_DUPP_UNIQUE";
    case HA_ERR_UNKNOWN_CHARSET:
      return "HA_ERR_UNKNOWN_CHARSET";
    case HA_ERR_WRONG_MRG_TABLE_DEF:
      return "HA_ERR_WRONG_MRG_TABLE_DEF";
    case HA_ERR_CRASHED_ON_REPAIR:
      return "HA_ERR_CRASHED_ON_REPAIR";
    case HA_ERR_CRASHED_ON_USAGE:
      return "HA_ERR_CRASHED_ON_USAGE";
    case HA_ERR_LOCK_WAIT_TIMEOUT:
      return "HA_ERR_LOCK_WAIT_TIMEOUT";
    case HA_ERR_LOCK_TABLE_FULL:
      return "HA_ERR_LOCK_TABLE_FULL";
    case HA_ERR_READ_ONLY_TRANSACTION:
      return "HA_ERR_READ_ONLY_TRANSACTION";
    case HA_ERR_LOCK_DEADLOCK:
      return "HA_ERR_LOCK_DEADLOCK";
    case HA_ERR_CANNOT_ADD_FOREIGN:
      return "HA_ERR_CANNOT_ADD_FOREIGN";
    case HA_ERR_NO_REFERENCED_ROW:
      return "HA_ERR_NO_REFERENCED_ROW";
    case HA_ERR_ROW_IS_REFERENCED:
      return "HA_ERR_ROW_IS_REFERENCED";
    case HA_ERR_NO_SAVEPOINT:
      return "HA_ERR_NO_SAVEPOINT";
    case HA_ERR_NON_UNIQUE_BLOCK_SIZE:
      return "HA_ERR_NON_UNIQUE_BLOCK_SIZE";
    case HA_ERR_NO_SUCH_TABLE:
      return "HA_ERR_NO_SUCH_TABLE";
    case HA_ERR_TABLE_EXIST:
      return "HA_ERR_TABLE_EXIST";
    case HA_ERR_NO_CONNECTION:
      return "HA_ERR_NO_CONNECTION";
    case HA_ERR_NULL_IN_SPATIAL:
      return "HA_ERR_NULL_IN_SPATIAL";
    case HA_ERR_TABLE_DEF_CHANGED:
      return "HA_ERR_TABLE_DEF_CHANGED";
    case HA_ERR_NO_PARTITION_FOUND:
      return "HA_ERR_NO_PARTITION_FOUND";
    case HA_ERR_RBR_LOGGING_FAILED:
      return "HA_ERR_RBR_LOGGING_FAILED";
    case HA_ERR_DROP_INDEX_FK:
      return "HA_ERR_DROP_INDEX_FK";
    case HA_ERR_FOREIGN_DUPLICATE_KEY:
      return "HA_ERR_FOREIGN_DUPLICATE_KEY";
    case HA_ERR_TABLE_NEEDS_UPGRADE:
      return "HA_ERR_TABLE_NEEDS_UPGRADE";
    case HA_ERR_TABLE_READONLY:
      return "HA_ERR_TABLE_READONLY";
    case HA_ERR_AUTOINC_READ_FAILED:
      return "HA_ERR_AUTOINC_READ_FAILED";
    case HA_ERR_AUTOINC_ERANGE:
      return "HA_ERR_AUTOINC_ERANGE";
    case HA_ERR_GENERIC:
      return "HA_ERR_GENERIC";
    case HA_ERR_RECORD_IS_THE_SAME:
      return "HA_ERR_RECORD_IS_THE_SAME";
    case HA_ERR_LOGGING_IMPOSSIBLE:
      return "HA_ERR_LOGGING_IMPOSSIBLE";
    case HA_ERR_CORRUPT_EVENT:
      return "HA_ERR_CORRUPT_EVENT";
    case HA_ERR_ROWS_EVENT_APPLY:
      return "HA_ERR_ROWS_EVENT_APPLY";
    case HA_ERR_FK_DEPTH_EXCEEDED:
      return "HA_ERR_FK_DEPTH_EXCEEDED";
    case HA_ERR_INNODB_READ_ONLY:
      return "HA_ERR_INNODB_READ_ONLY";
    case HA_ERR_COMPUTE_FAILED:
      return "HA_ERR_COMPUTE_FAILED";
    case HA_ERR_NO_WAIT_LOCK:
      return "HA_ERR_NO_WAIT_LOCK";
  }
  return "No Error!";
}

/**
   Error reporting facility for Rows_log_event::do_apply_event

   @param level     error, warning or info
   @param ha_error  HA_ERR_ code
   @param rli       pointer to the active Relay_log_info instance
   @param thd       pointer to the slave thread's thd
   @param table     pointer to the event's table object
   @param type      the type of the event
   @param log_name  the master binlog file name
   @param pos       the master binlog file pos (the next after the event)

*/
static void inline slave_rows_error_report(enum loglevel level, int ha_error,
                                           Relay_log_info const *rli, THD *thd,
                                           TABLE *table, const char *type,
                                           const char *log_name, ulong pos) {
  const char *handler_error = (ha_error ? HA_ERR(ha_error) : nullptr);
  bool is_group_replication_applier_channel =
      channel_map.is_group_replication_channel_name(
          (const_cast<Relay_log_info *>(rli))->get_channel(), true);
  char buff[MAX_SLAVE_ERRMSG], *slider;
  const char *buff_end = buff + sizeof(buff);
  size_t len;
  Diagnostics_area::Sql_condition_iterator it =
      thd->get_stmt_da()->sql_conditions();
  const Sql_condition *err;
  buff[0] = 0;

  for (err = it++, slider = buff; err && slider < buff_end - 1;
       slider += len, err = it++) {
    len = snprintf(slider, buff_end - slider, " %s, Error_code: %d;",
                   err->message_text(), err->mysql_errno());
  }
  if (is_group_replication_applier_channel) {
    if (ha_error != 0) {
      rli->report(level,
                  thd->is_error() ? thd->get_stmt_da()->mysql_errno()
                                  : ER_UNKNOWN_ERROR,
                  "Could not execute %s event on table %s.%s;"
                  "%s handler error %s",
                  type, table->s->db.str, table->s->table_name.str, buff,
                  handler_error == nullptr ? "<unknown>" : handler_error);
    } else {
      rli->report(level,
                  thd->is_error() ? thd->get_stmt_da()->mysql_errno()
                                  : ER_UNKNOWN_ERROR,
                  "Could not execute %s event on table %s.%s;"
                  "%s",
                  type, table->s->db.str, table->s->table_name.str, buff);
    }
  } else {
    if (ha_error != 0) {
      rli->report(level,
                  thd->is_error() ? thd->get_stmt_da()->mysql_errno()
                                  : ER_UNKNOWN_ERROR,
                  "Could not execute %s event on table %s.%s;"
                  "%s handler error %s; "
                  "the event's master log %s, end_log_pos %lu",
                  type, table->s->db.str, table->s->table_name.str, buff,
                  handler_error == nullptr ? "<unknown>" : handler_error,
                  log_name, pos);
    } else {
      rli->report(level,
                  thd->is_error() ? thd->get_stmt_da()->mysql_errno()
                                  : ER_UNKNOWN_ERROR,
                  "Could not execute %s event on table %s.%s;"
                  "%s the event's master log %s, end_log_pos %lu",
                  type, table->s->db.str, table->s->table_name.str, buff,
                  log_name, pos);
    }
  }
}

/**
  Set the rewritten database, or current database if it should not be
  rewritten, into THD.

  @param thd THD handle
  @param db database name
  @param db_len the length of database name

  @retval true if the passed db is rewritten.
  @retval false if the passed db is not rewritten.
*/
static bool set_thd_db(THD *thd, const char *db, size_t db_len) {
  bool need_increase_counter = false;
  char lcase_db_buf[NAME_LEN + 1];
  LEX_CSTRING new_db;
  new_db.length = db_len;
  if (lower_case_table_names) {
    my_stpcpy(lcase_db_buf, db);
    my_casedn_str(system_charset_info, lcase_db_buf);
    new_db.str = lcase_db_buf;
  } else
    new_db.str = db;

  /* This function is called by a slave thread. */
  DBUG_ASSERT(thd->rli_slave);

  Rpl_filter *rpl_filter = thd->rli_slave->rpl_filter;
  new_db.str = rpl_filter->get_rewrite_db(new_db.str, &new_db.length);

  if (lower_case_table_names) {
    /* lcase_db_buf != new_db.str means that lcase_db_buf is rewritten. */
    if (strcmp(lcase_db_buf, new_db.str)) need_increase_counter = true;
  } else {
    /* db != new_db.str means that db is rewritten. */
    if (strcmp(db, new_db.str)) need_increase_counter = true;
  }

  thd->set_db(new_db);

  return need_increase_counter;
}

#endif

/*
  pretty_print_str()
*/

#ifndef MYSQL_SERVER
static inline void pretty_print_str(IO_CACHE *cache, const char *str,
                                    size_t len, bool identifier) {
  const char *end = str + len;
  my_b_printf(cache, identifier ? "`" : "\'");
  while (str < end) {
    char c;
    switch ((c = *str++)) {
      case '\n':
        my_b_printf(cache, "\\n");
        break;
      case '\r':
        my_b_printf(cache, "\\r");
        break;
      case '\\':
        my_b_printf(cache, "\\\\");
        break;
      case '\b':
        my_b_printf(cache, "\\b");
        break;
      case '\t':
        my_b_printf(cache, "\\t");
        break;
      case '\'':
        my_b_printf(cache, "\\'");
        break;
      case 0:
        my_b_printf(cache, "\\0");
        break;
      case '`':
        if (identifier)
          my_b_printf(cache, "``");
        else
          my_b_printf(cache, "`");
        break;
      default:
        my_b_printf(cache, "%c", c);
        break;
    }
  }
  my_b_printf(cache, identifier ? "`" : "\'");
}

/**
  Print src as an string enclosed with "'"

  @param[out] cache  IO_CACHE where the string will be printed.
  @param[in] str  the string will be printed.
  @param[in] len  length of the string.
*/
static inline void pretty_print_str(IO_CACHE *cache, const char *str,
                                    size_t len) {
  pretty_print_str(cache, str, len, false);
}

/**
  Print src as an identifier enclosed with "`"

  @param[out] cache  IO_CACHE where the identifier will be printed.
  @param[in] str  the string will be printed.
  @param[in] len  length of the string.
 */
static inline void pretty_print_identifier(IO_CACHE *cache, const char *str,
                                           size_t len) {
  pretty_print_str(cache, str, len, true);
}

#endif /* !MYSQL_SERVER */

#if defined(MYSQL_SERVER)

static void clear_all_errors(THD *thd, Relay_log_info *rli) {
  thd->is_slave_error = false;
  thd->clear_error();
  rli->clear_error();
  if (rli->workers_array_initialized) {
    for (size_t i = 0; i < rli->get_worker_count(); i++) {
      rli->get_worker(i)->clear_error();
    }
  }
}

inline int idempotent_error_code(int err_code) {
  int ret = 0;

  switch (err_code) {
    case 0:
      ret = 1;
      break;
    /*
      The following list of "idempotent" errors
      means that an error from the list might happen
      because of idempotent (more than once)
      applying of a binlog file.
      Notice, that binlog has a  ddl operation its
      second applying may cause

      case HA_ERR_TABLE_DEF_CHANGED:
      case HA_ERR_CANNOT_ADD_FOREIGN:

      which are not included into to the list.

      Note that HA_ERR_RECORD_DELETED is not in the list since
      do_exec_row() should not return that error code.
    */
    case HA_ERR_RECORD_CHANGED:
    case HA_ERR_KEY_NOT_FOUND:
    case HA_ERR_END_OF_FILE:
    case HA_ERR_FOUND_DUPP_KEY:
    case HA_ERR_FOUND_DUPP_UNIQUE:
    case HA_ERR_FOREIGN_DUPLICATE_KEY:
    case HA_ERR_NO_REFERENCED_ROW:
    case HA_ERR_ROW_IS_REFERENCED:
      ret = 1;
      break;
    default:
      ret = 0;
      break;
  }
  return (ret);
}

/**
  Ignore error code specified on command line.
*/

int ignored_error_code(int err_code) {
  return ((err_code == ER_SLAVE_IGNORED_TABLE) ||
          (use_slave_mask && bitmap_is_set(&slave_error_mask, err_code)));
}

/*
  This function converts an engine's error to a server error.

  If the thread does not have an error already reported, it tries to
  define it by calling the engine's method print_error. However, if a
  mapping is not found, it uses the ER_UNKNOWN_ERROR and prints out a
  warning message.
*/
static int convert_handler_error(int error, THD *thd, TABLE *table) {
  uint actual_error = (thd->is_error() ? thd->get_stmt_da()->mysql_errno() : 0);

  if (actual_error == 0) {
    table->file->print_error(error, MYF(0));
    actual_error = (thd->is_error() ? thd->get_stmt_da()->mysql_errno()
                                    : ER_UNKNOWN_ERROR);
    if (actual_error == ER_UNKNOWN_ERROR)
      LogErr(WARNING_LEVEL, ER_UNKNOWN_ERROR_DETECTED_IN_SE, error);
  }

  return (actual_error);
}

inline bool concurrency_error_code(int error) {
  switch (error) {
    case ER_LOCK_WAIT_TIMEOUT:
    case ER_LOCK_DEADLOCK:
    case ER_XA_RBDEADLOCK:
      return true;
    default:
      return (false);
  }
}

inline bool unexpected_error_code(int unexpected_error) {
  switch (unexpected_error) {
    case ER_NET_READ_ERROR:
    case ER_NET_ERROR_ON_WRITE:
    case ER_QUERY_INTERRUPTED:
    case ER_SERVER_SHUTDOWN:
    case ER_NEW_ABORTING_CONNECTION:
      return (true);
    default:
      return (false);
  }
}

/*
  pretty_print_str()
*/
static void pretty_print_str(String *packet, const char *str, size_t len) {
  packet->append('\'');

  for (size_t i = 0; i < len; i++) {
    switch (str[i]) {
      case '\n':
        packet->append("\\n");
        break;
      case '\r':
        packet->append("\\r");
        break;
      case '\\':
        packet->append("\\\\");
        break;
      case '\b':
        packet->append("\\b");
        break;
      case '\t':
        packet->append("\\t");
        break;
      case '\'':
        packet->append("\\'");
        break;
      case 0:
        packet->append("\\0");
        break;
      default:
        packet->append(str[i]);
        break;
    }
  }
  packet->append('\'');
}

static inline void pretty_print_str(String *packet, const String *str) {
  pretty_print_str(packet, str->ptr(), str->length());
}

/**
  Creates a temporary name for load data infile:.

  @param buf		      Store new filename here
  @param file_id	      File_id (part of file name)
  @param event_server_id     Event_id (part of file name)
  @param ext		      Extension for file name

  @return
    Pointer to start of extension
*/

static char *slave_load_file_stem(char *buf, uint file_id, int event_server_id,
                                  const char *ext) {
  char *res;
  fn_format(buf, PREFIX_SQL_LOAD, slave_load_tmpdir, "", MY_UNPACK_FILENAME);
  to_unix_path(buf);

  buf = strend(buf);
  int appended_length = sprintf(buf, "%s-%d-", server_uuid, event_server_id);
  buf += appended_length;
  res = longlong10_to_str(file_id, buf, 10);
  my_stpcpy(res, ext);  // Add extension last
  return res;           // Pointer to extension
}

/**
  Delete all temporary files used for SQL_LOAD.
*/

static void cleanup_load_tmpdir() {
  MY_DIR *dirp;
  FILEINFO *file;
  uint i;
  char fname[FN_REFLEN], prefbuf[TEMP_FILE_MAX_LEN], *p;

  if (!(dirp = my_dir(slave_load_tmpdir, MYF(0)))) return;

  /*
     When we are deleting temporary files, we should only remove
     the files associated with the server id of our server.
     We don't use event_server_id here because since we've disabled
     direct binlogging of Create_file/Append_file/Exec_load events
     we cannot meet Start_log event in the middle of events from one
     LOAD DATA.
  */
  p = strmake(prefbuf, STRING_WITH_LEN(PREFIX_SQL_LOAD));
  sprintf(p, "%s-", server_uuid);

  for (i = 0; i < dirp->number_off_files; i++) {
    file = dirp->dir_entry + i;
    if (is_prefix(file->name, prefbuf)) {
      fn_format(fname, file->name, slave_load_tmpdir, "", MY_UNPACK_FILENAME);
      mysql_file_delete(key_file_misc, fname, MYF(0));
    }
  }

  my_dirend(dirp);
}
#endif

template <typename T>
bool net_field_length_checked(const uchar **packet, size_t *max_length,
                              T *out) {
  if (*max_length < 1) return true;
  const uchar *pos = *packet;
  if (*pos < 251) {
    (*packet)++;
    (*max_length)--;
    *out = (T)*pos;
  } else if (*pos == 251) {
    (*packet)++;
    (*max_length)--;
    *out = (T)NULL_LENGTH;
  } else if (*pos == 252) {
    if (*max_length < 3) return true;
    (*packet) += 3;
    (*max_length) -= 3;
    *out = (T)uint2korr(pos + 1);
  } else if (*pos == 253) {
    if (*max_length < 4) return true;
    (*packet) += 4;
    (*max_length) -= 4;
    *out = (T)uint3korr(pos + 1);
  } else {
    if (*max_length < 9) return true;
    (*packet) += 9;
    (*max_length) -= 9;
    *out = (T)uint8korr(pos + 1);
  }
  return false;
}
template bool net_field_length_checked<size_t>(const uchar **packet,
                                               size_t *max_length, size_t *out);
template bool net_field_length_checked<ulonglong>(const uchar **packet,
                                                  size_t *max_length,
                                                  ulonglong *out);

/**
  Transforms a string into "" or its expression in 0x... form.
*/

char *str_to_hex(char *to, const char *from, size_t len) {
  if (len) {
    *to++ = '0';
    *to++ = 'x';
    to = octet2hex(to, from, len);
  } else
    to = my_stpcpy(to, "\"\"");
  return to;  // pointer to end 0 of 'to'
}

#ifdef MYSQL_SERVER

/**
  Append a version of the 'from' string suitable for use in a query to
  the 'to' string.  To generate a correct escaping, the character set
  information in 'csinfo' is used.
*/

int append_query_string(const THD *thd, const CHARSET_INFO *csinfo,
                        String const *from, String *to) {
  char *beg, *ptr;
  size_t const orig_len = to->length();
  if (to->reserve(orig_len + from->length() * 2 + 3)) return 1;

  beg = to->c_ptr_quick() + to->length();
  ptr = beg;
  if (csinfo->escape_with_backslash_is_dangerous)
    ptr = str_to_hex(ptr, from->ptr(), from->length());
  else {
    *ptr++ = '\'';
    if (!(thd->variables.sql_mode & MODE_NO_BACKSLASH_ESCAPES)) {
      ptr +=
          escape_string_for_mysql(csinfo, ptr, 0, from->ptr(), from->length());
    } else {
      const char *frm_str = from->ptr();

      for (; frm_str < (from->ptr() + from->length()); frm_str++) {
        /* Using '' way to represent "'" */
        if (*frm_str == '\'') *ptr++ = *frm_str;

        *ptr++ = *frm_str;
      }
    }

    *ptr++ = '\'';
  }
  to->length(orig_len + ptr - beg);
  return 0;
}
#endif

/**
  Prints a "session_var=value" string. Used by mysqlbinlog to print some SET
  commands just before it prints a query.
*/

#ifndef MYSQL_SERVER

static void print_set_option(IO_CACHE *file, uint32 bits_changed, uint32 option,
                             uint32 flags, const char *name, bool *need_comma) {
  if (bits_changed & option) {
    if (*need_comma) my_b_printf(file, ", ");
    my_b_printf(file, "%s=%d", name, static_cast<bool>(flags & option));
    *need_comma = true;
  }
}
#endif

#ifdef MYSQL_SERVER
Replicated_columns_view::Replicated_columns_view(
    Replicated_columns_view::enum_replication_flow direction, THD const *thd)
    : Replicated_columns_view{nullptr, direction, thd} {}

Replicated_columns_view::Replicated_columns_view(
    TABLE const *target,
    Replicated_columns_view::enum_replication_flow direction, THD const *thd)
    : Table_columns_view{} {
  filter_fn_type filter{nullptr};

  if (direction == Replicated_columns_view::OUTBOUND)
    filter = [this](TABLE const *table, size_t column_index) -> bool {
      return this->outbound_filtering(table, column_index);
    };
  else
    filter = [this](TABLE const *table, size_t column_index) -> bool {
      return this->inbound_filtering(table, column_index);
    };

  this->set_thd(thd)       //
      .set_filter(filter)  //
      .set_table(target);
}

Replicated_columns_view &Replicated_columns_view::set_thd(THD const *thd) {
  this->m_thd = thd;
  this->init_fields_bitmaps();
  return (*this);
}

bool Replicated_columns_view::is_inbound_filtering_enabled() {
  return (this->m_thd == nullptr ||
          this->m_thd->variables.immediate_server_version ==
              UNDEFINED_SERVER_VERSION ||
          this->m_thd->variables.immediate_server_version >= 80018);
}

bool Replicated_columns_view::inbound_filtering(TABLE const *table,
                                                size_t column_index) {
  if (!this->is_inbound_filtering_enabled()) return false;
  // If the set of filtered columns is changed, we need to replicate the change
  // in other blocks that reproduce the behavior - Rapid binlog parser, for
  // instance.
  return bitmap_is_set(&table->fields_for_functional_indexes, column_index);
}

bool Replicated_columns_view::outbound_filtering(TABLE const *table,
                                                 size_t column_index) {
  // If the set of filtered columns is changed, we need to replicate the change
  // in other blocks that reproduce the behavior - Rapid binlog parser, for
  // instance.
  return bitmap_is_set(&table->fields_for_functional_indexes, column_index);
}
#endif

/**************************************************************************
        Log_event methods (= the parent class of all events)
**************************************************************************/

#ifdef MYSQL_SERVER

time_t Log_event::get_time() {
  /* Not previously initialized */
  if (!common_header->when.tv_sec && !common_header->when.tv_usec) {
    THD *tmp_thd = thd ? thd : current_thd;
    if (tmp_thd)
      common_header->when = tmp_thd->start_time;
    else
      my_micro_time_to_timeval(my_micro_time(), &(common_header->when));
  }
  return (time_t)common_header->when.tv_sec;
}

#endif

/**
  @return
  returns the human readable name of the event's type
*/

const char *Log_event::get_type_str(Log_event_type type) {
  switch (type) {
    case binary_log::STOP_EVENT:
      return "Stop";
    case binary_log::QUERY_EVENT:
      return "Query";
    case binary_log::ROTATE_EVENT:
      return "Rotate";
    case binary_log::INTVAR_EVENT:
      return "Intvar";
    case binary_log::APPEND_BLOCK_EVENT:
      return "Append_block";
    case binary_log::DELETE_FILE_EVENT:
      return "Delete_file";
    case binary_log::RAND_EVENT:
      return "RAND";
    case binary_log::XID_EVENT:
      return "Xid";
    case binary_log::USER_VAR_EVENT:
      return "User var";
    case binary_log::FORMAT_DESCRIPTION_EVENT:
      return "Format_desc";
    case binary_log::TABLE_MAP_EVENT:
      return "Table_map";
    case binary_log::WRITE_ROWS_EVENT_V1:
      return "Write_rows_v1";
    case binary_log::UPDATE_ROWS_EVENT_V1:
      return "Update_rows_v1";
    case binary_log::DELETE_ROWS_EVENT_V1:
      return "Delete_rows_v1";
    case binary_log::BEGIN_LOAD_QUERY_EVENT:
      return "Begin_load_query";
    case binary_log::EXECUTE_LOAD_QUERY_EVENT:
      return "Execute_load_query";
    case binary_log::INCIDENT_EVENT:
      return "Incident";
    case binary_log::IGNORABLE_LOG_EVENT:
      return "Ignorable";
    case binary_log::ROWS_QUERY_LOG_EVENT:
      return "Rows_query";
    case binary_log::WRITE_ROWS_EVENT:
      return "Write_rows";
    case binary_log::UPDATE_ROWS_EVENT:
      return "Update_rows";
    case binary_log::DELETE_ROWS_EVENT:
      return "Delete_rows";
    case binary_log::GTID_LOG_EVENT:
      return "Gtid";
    case binary_log::ANONYMOUS_GTID_LOG_EVENT:
      return "Anonymous_Gtid";
    case binary_log::PREVIOUS_GTIDS_LOG_EVENT:
      return "Previous_gtids";
    case binary_log::HEARTBEAT_LOG_EVENT:
      return "Heartbeat";
    case binary_log::TRANSACTION_CONTEXT_EVENT:
      return "Transaction_context";
    case binary_log::VIEW_CHANGE_EVENT:
      return "View_change";
    case binary_log::XA_PREPARE_LOG_EVENT:
      return "XA_prepare";
    case binary_log::PARTIAL_UPDATE_ROWS_EVENT:
      return "Update_rows_partial";
    case binary_log::TRANSACTION_PAYLOAD_EVENT:
      return "Transaction_payload";
    case binary_log::METADATA_EVENT:
      return "Metadata";
    default:
      return "Unknown"; /* impossible */
  }
}

const char *Log_event::get_type_str() const {
  return get_type_str(get_type_code());
}

/*
  Log_event::Log_event()
*/

#ifdef MYSQL_SERVER
Log_event::Log_event(THD *thd_arg, uint16 flags_arg,
                     enum_event_cache_type cache_type_arg,
                     enum_event_logging_type logging_type_arg,
                     Log_event_header *header, Log_event_footer *footer)
    : temp_buf(nullptr),
      m_free_temp_buf_in_destructor(true),
      exec_time(0),
      event_cache_type(cache_type_arg),
      event_logging_type(logging_type_arg),
      crc(0),
      common_header(header),
      common_footer(footer),
      thd(thd_arg) {
  server_id = thd->server_id;
  common_header->unmasked_server_id = server_id;
  common_header->when = thd->start_time;
  common_header->log_pos = 0;
  common_header->flags = flags_arg;
}

/**
  This minimal constructor is for when you are not even sure that there
  is a valid THD. For example in the server when we are shutting down or
  flushing logs after receiving a SIGHUP (then we must write a Rotate to
  the binlog but we have no THD, so we need this minimal constructor).
*/

Log_event::Log_event(Log_event_header *header, Log_event_footer *footer,
                     enum_event_cache_type cache_type_arg,
                     enum_event_logging_type logging_type_arg)
    : temp_buf(nullptr),
      m_free_temp_buf_in_destructor(true),
      exec_time(0),
      event_cache_type(cache_type_arg),
      event_logging_type(logging_type_arg),
      crc(0),
      common_header(header),
      common_footer(footer),
      thd(nullptr) {
  server_id = ::server_id;
  common_header->unmasked_server_id = server_id;
}
#endif /* MYSQL_SERVER */

/*
  Log_event::Log_event()
*/

Log_event::Log_event(Log_event_header *header, Log_event_footer *footer)
    : temp_buf(nullptr),
      m_free_temp_buf_in_destructor(true),
      exec_time(0),
      event_cache_type(EVENT_INVALID_CACHE),
      event_logging_type(EVENT_INVALID_LOGGING),
      crc(0),
      common_header(header),
      common_footer(footer) {
#ifdef MYSQL_SERVER
  thd = nullptr;
#endif
  /*
     Mask out any irrelevant parts of the server_id
  */
  server_id = common_header->unmasked_server_id & opt_server_id_mask;
}

/*
  This method is not on header file to avoid using key_memory_log_event
  outside log_event.cc, allowing header file to be included on plugins.
*/
void *Log_event::operator new(size_t size) {
  return my_malloc(key_memory_log_event, size, MYF(MY_WME | MY_FAE));
}

#ifdef MYSQL_SERVER
inline int Log_event::do_apply_event_worker(Slave_worker *w) {
  DBUG_EXECUTE_IF("crash_in_a_worker", {
    /* we will crash a worker after waiting for
    2 seconds to make sure that other transactions are
    scheduled and completed */
    if (w->id == 2) {
      DBUG_SET("-d,crash_in_a_worker");
      my_sleep(2000000);
      DBUG_SUICIDE();
    }
  });
  return do_apply_event(w);
}

int Log_event::do_update_pos(Relay_log_info *rli) {
  int error = 0;
  DBUG_ASSERT(!rli->belongs_to_client());

  if (rli) error = rli->stmt_done(common_header->log_pos);
  return error;
}

Log_event::enum_skip_reason Log_event::do_shall_skip(Relay_log_info *rli) {
  /*
    The logic for slave_skip_counter is as follows:

    - Events that are skipped because they have the same server_id as
      the slave do not decrease slave_skip_counter.

    - Other events (that pass the server_id test) will decrease
      slave_skip_counter.

    - Except in one case: if slave_skip_counter==1, it will only
      decrease to 0 if we are at a so-called group boundary. Here, a
      group is defined as the range of events that represent a single
      transaction in the relay log: see comment for is_in_group in
      rpl_rli.h for a definition.

    The difficult part to implement is the logic to avoid decreasing
    the counter to 0.  Given that groups have the form described in
    is_in_group in rpl_rli.h, we implement the logic as follows:

    - Gtid, Rand, User_var, Int_var will never decrease the counter to
      0.

    - BEGIN will set thd->variables.option_bits & OPTION_BEGIN and
      COMMIT/Xid will clear it.  This happens regardless of whether
      the BEGIN/COMMIT/Xid is skipped itself.

    - Other events will decrease the counter unless OPTION_BEGIN is
      set.
  */
  DBUG_PRINT("info", ("ev->server_id=%lu, ::server_id=%lu,"
                      " rli->replicate_same_server_id=%d,"
                      " rli->slave_skip_counter=%d",
                      (ulong)server_id, (ulong)::server_id,
                      rli->replicate_same_server_id, rli->slave_skip_counter));
  if ((server_id == ::server_id && !rli->replicate_same_server_id) ||
      (rli->slave_skip_counter == 1 && rli->is_in_group()))
    return EVENT_SKIP_IGNORE;
  else if (rli->slave_skip_counter > 0)
    return EVENT_SKIP_COUNT;
  else
    return EVENT_SKIP_NOT;
}

/*
  Log_event::pack_info()
*/

int Log_event::pack_info(Protocol *protocol) {
  protocol->store("", &my_charset_bin);
  return 0;
}

const char *Log_event::get_db() { return thd ? thd->db().str : nullptr; }

/**
  Only called by SHOW BINLOG EVENTS
*/
int Log_event::net_send(Protocol *protocol, const char *log_name,
                        my_off_t pos) {
  const char *p = strrchr(log_name, FN_LIBCHAR);
  const char *event_type;
  if (p) log_name = p + 1;

  protocol->start_row();
  protocol->store(log_name, &my_charset_bin);
  protocol->store((ulonglong)pos);
  event_type = get_type_str();
  protocol->store_string(event_type, strlen(event_type), &my_charset_bin);
  protocol->store((uint32)server_id);
  protocol->store((ulonglong)common_header->log_pos);
  if (pack_info(protocol)) return 1;
  return protocol->end_row();
}

/**
  init_show_field_list() prepares the column names and types for the
  output of SHOW BINLOG EVENTS; it is used only by SHOW BINLOG
  EVENTS.
*/

void Log_event::init_show_field_list(List<Item> *field_list) {
  field_list->push_back(new Item_empty_string("Log_name", 20));
  field_list->push_back(new Item_return_int("Pos", MY_INT32_NUM_DECIMAL_DIGITS,
                                            MYSQL_TYPE_LONGLONG));
  field_list->push_back(new Item_empty_string("Event_type", 20));
  field_list->push_back(new Item_return_int("Server_id", 10, MYSQL_TYPE_LONG));
  field_list->push_back(new Item_return_int(
      "End_log_pos", MY_INT32_NUM_DECIMAL_DIGITS, MYSQL_TYPE_LONGLONG));
  field_list->push_back(new Item_empty_string("Info", 20));
}

/**
   A decider of whether to trigger checksum computation or not.
   To be invoked in Log_event::write() stack.
   The decision is positive

    S,M) if it's been marked for checksumming with @c checksum_alg

    M) otherwise, if @@global.binlog_checksum is not NONE and the event is
       directly written to the binlog file.
       The to-be-cached event decides at @c write_cache() time.

   Otherwise the decision is negative.

   @note   A side effect of the method is altering Log_event::checksum_alg
           it the latter was undefined at calling.

   @return true (positive) or false (negative)
*/
bool Log_event::need_checksum() {
  DBUG_TRACE;
  bool ret = false;
  /*
     few callers of Log_event::write
     (incl FD::write, FD constructing code on the slave side, Rotate relay log
     and Stop event)
     provides their checksum alg preference through Log_event::checksum_alg.
  */
  if (common_footer->checksum_alg != binary_log::BINLOG_CHECKSUM_ALG_UNDEF)
    ret = (common_footer->checksum_alg != binary_log::BINLOG_CHECKSUM_ALG_OFF);
  else if (binlog_checksum_options != binary_log::BINLOG_CHECKSUM_ALG_OFF &&
           event_cache_type == Log_event::EVENT_NO_CACHE)
    ret = (binlog_checksum_options != 0);
  else
    ret = false;

  /*
    FD calls the methods before data_written has been calculated.
    The following invariant claims if the current is not the first
    call (and therefore data_written is not zero) then `ret' must be
    true. It may not be null because FD is always checksummed.
  */

  DBUG_ASSERT(get_type_code() != binary_log::FORMAT_DESCRIPTION_EVENT || ret ||
              common_header->data_written == 0);

  if (common_footer->checksum_alg == binary_log::BINLOG_CHECKSUM_ALG_UNDEF)
    common_footer->checksum_alg =
        ret ?  // calculated value stored
            static_cast<enum_binlog_checksum_alg>(binlog_checksum_options)
            : binary_log::BINLOG_CHECKSUM_ALG_OFF;

  DBUG_ASSERT(
      !ret ||
      ((common_footer->checksum_alg ==
            static_cast<enum_binlog_checksum_alg>(binlog_checksum_options) ||
        /*
           Stop event closes the relay-log and its checksum alg
           preference is set by the caller can be different
           from the server's binlog_checksum_options.
        */
        get_type_code() == binary_log::STOP_EVENT ||
        /*
           Rotate:s can be checksummed regardless of the server's
           binlog_checksum_options. That applies to both
           the local RL's Rotate and the master's Rotate
           which IO thread instantiates via queue_binlog_ver_3_event.
        */
        get_type_code() == binary_log::ROTATE_EVENT ||
        /*
           The previous event has its checksum option defined
           according to the format description event.
        */
        get_type_code() == binary_log::PREVIOUS_GTIDS_LOG_EVENT ||
        /* FD is always checksummed */
        get_type_code() == binary_log::FORMAT_DESCRIPTION_EVENT) &&
       common_footer->checksum_alg != binary_log::BINLOG_CHECKSUM_ALG_OFF));

  DBUG_ASSERT(common_footer->checksum_alg !=
              binary_log::BINLOG_CHECKSUM_ALG_UNDEF);
  DBUG_ASSERT(((get_type_code() != binary_log::ROTATE_EVENT &&
                get_type_code() != binary_log::STOP_EVENT) ||
               get_type_code() != binary_log::FORMAT_DESCRIPTION_EVENT) ||
              event_cache_type == Log_event::EVENT_NO_CACHE);

  return ret;
}

bool Log_event::wrapper_my_b_safe_write(Basic_ostream *ostream,
                                        const uchar *buf, size_t size) {
  if (size == 0) return false;

  if (need_checksum() && size != 0) crc = checksum_crc32(crc, buf, size);

  return ostream->write(buf, size);
}

bool Log_event::write_footer(Basic_ostream *ostream) {
  /*
     footer contains the checksum-algorithm descriptor
     followed by the checksum value
  */
  if (need_checksum()) {
    uchar buf[BINLOG_CHECKSUM_LEN];
    int4store(buf, crc);
    return ostream->write((uchar *)buf, sizeof(buf));
  }
  return false;
}

uint32 Log_event::write_header_to_memory(uchar *buf) {
  // Query start time
  ulong timestamp = (ulong)get_time();

#ifndef DBUG_OFF
  if (DBUG_EVALUATE_IF("inc_event_time_by_1_hour", 1, 0) &&
      DBUG_EVALUATE_IF("dec_event_time_by_1_hour", 1, 0)) {
    /**
      This assertion guarantees that these debug flags are not
      used at the same time (they would cancel each other).
    */
    DBUG_ASSERT(0);
  } else {
    DBUG_EXECUTE_IF("inc_event_time_by_1_hour", timestamp = timestamp + 3600;);
    DBUG_EXECUTE_IF("dec_event_time_by_1_hour", timestamp = timestamp - 3600;);
  }
#endif

  /*
    Header will be of size LOG_EVENT_HEADER_LEN for all events, except for
    FORMAT_DESCRIPTION_EVENT and ROTATE_EVENT, where it will be
    LOG_EVENT_MINIMAL_HEADER_LEN (remember these 2 have a frozen header,
    because we read them before knowing the format).
  */

  int4store(buf, timestamp);
  buf[EVENT_TYPE_OFFSET] = get_type_code();
  int4store(buf + SERVER_ID_OFFSET, server_id);
  int4store(buf + EVENT_LEN_OFFSET,
            static_cast<uint32>(common_header->data_written));
  int4store(buf + LOG_POS_OFFSET, static_cast<uint32>(common_header->log_pos));
  int2store(buf + FLAGS_OFFSET, common_header->flags);

  return LOG_EVENT_HEADER_LEN;
}

bool Log_event::write_header(Basic_ostream *ostream, size_t event_data_length) {
  uchar header[LOG_EVENT_HEADER_LEN];
  bool ret;
  DBUG_TRACE;

  /* Store number of bytes that will be written by this event */
  common_header->data_written = event_data_length + sizeof(header);

  if (need_checksum()) {
    crc = checksum_crc32(0L, nullptr, 0);
    common_header->data_written += BINLOG_CHECKSUM_LEN;
  }

  /*
    Usually events are written into binlog cache first. And later, they are
    flushed into binlog file. When events are being written into binlog cache,
    log_pos(a.k.a. end_log_pos) field is meaningless. So it is set to 0. the
    log_pos field will be updated later when the events are being flushed into
    binlog file.

    In a few cases(e.g. rotation(FD, Rotate events)), events are written into
    binlog file directly through event->write(). In these cases, log_pos is
    updated to the begin position of the event before calling event->write().
    Then log_pos is updated to the end position of the event here.
  */
  if (common_header->log_pos != 0) {
    common_header->log_pos += common_header->data_written;
  }

  write_header_to_memory(header);

  ret = ostream->write(header, LOG_EVENT_HEADER_LEN);

  /*
    Update the checksum.

    In case this is a Format_description_log_event, we need to clear
    the LOG_EVENT_BINLOG_IN_USE_F flag before computing the checksum,
    since the flag will be cleared when the binlog is closed.  On
    verification, the flag is dropped before computing the checksum
    too.
  */
  if (need_checksum() &&
      (common_header->flags & LOG_EVENT_BINLOG_IN_USE_F) != 0) {
    common_header->flags &= ~LOG_EVENT_BINLOG_IN_USE_F;
    int2store(header + FLAGS_OFFSET, common_header->flags);
  }
  crc = my_checksum(crc, header, LOG_EVENT_HEADER_LEN);

  // For RBR events, capture the binlog bytes written
  if (!ret && is_row_log_event()) {
    thd->inc_row_binlog_bytes_written(common_header->data_written);
  }
  return ret;
}
#endif /* MYSQL_SERVER */

bool Log_event::is_valid() {
  return common_header != nullptr && common_header->get_is_valid();
}

#ifndef MYSQL_SERVER

/*
  Log_event::print_header()
*/

void Log_event::print_header(IO_CACHE *file, PRINT_EVENT_INFO *print_event_info,
                             bool is_more MY_ATTRIBUTE((unused))) const {
  char llbuff[22];
  my_off_t hexdump_from = print_event_info->hexdump_from;
  DBUG_TRACE;

  my_b_printf(file, "#");
  print_timestamp(file, nullptr);
  my_b_printf(file, " server id %lu  end_log_pos %s ", (ulong)server_id,
              llstr(common_header->log_pos, llbuff));

  /* print the checksum */

  if (common_footer->checksum_alg != binary_log::BINLOG_CHECKSUM_ALG_OFF &&
      common_footer->checksum_alg != binary_log::BINLOG_CHECKSUM_ALG_UNDEF) {
    char checksum_buf[BINLOG_CHECKSUM_LEN * 2 + 4];  // to fit to "0x%lx "
    size_t const bytes_written =
        snprintf(checksum_buf, sizeof(checksum_buf), "0x%08lx ", (ulong)crc);
    my_b_printf(
        file, "%s ",
        get_type(&binlog_checksum_typelib, common_footer->checksum_alg));
    my_b_printf(file, checksum_buf, bytes_written);
  }

  /* mysqlbinlog --hexdump */
  if (print_event_info->hexdump_from) {
    my_b_printf(file, "\n");
    uchar *ptr = (uchar *)temp_buf;
    my_off_t size =
        uint4korr(ptr + EVENT_LEN_OFFSET) - LOG_EVENT_MINIMAL_HEADER_LEN;
    my_off_t i;

    /* Header len * 4 >= header len * (2 chars + space + extra space) */
    char *h, hex_string[49] = {0};
    char *c, char_string[16 + 1] = {0};

    /* Pretty-print event common header if header is exactly 19 bytes */
    if (print_event_info->common_header_len == LOG_EVENT_MINIMAL_HEADER_LEN) {
      char emit_buf[256];  // Enough for storing one line
      my_b_printf(file,
                  "# Position  Timestamp   Type   Master ID        "
                  "Size      Master Pos    Flags \n");
      size_t const bytes_written = snprintf(
          emit_buf, sizeof(emit_buf),
          "# %8.8lx %02x %02x %02x %02x   %02x   "
          "%02x %02x %02x %02x   %02x %02x %02x %02x   "
          "%02x %02x %02x %02x   %02x %02x\n",
          (unsigned long)hexdump_from, ptr[0], ptr[1], ptr[2], ptr[3], ptr[4],
          ptr[5], ptr[6], ptr[7], ptr[8], ptr[9], ptr[10], ptr[11], ptr[12],
          ptr[13], ptr[14], ptr[15], ptr[16], ptr[17], ptr[18]);
      DBUG_ASSERT(static_cast<size_t>(bytes_written) < sizeof(emit_buf));
      my_b_write(file, (uchar *)emit_buf, bytes_written);
      ptr += LOG_EVENT_MINIMAL_HEADER_LEN;
      hexdump_from += LOG_EVENT_MINIMAL_HEADER_LEN;
    }

    /* Rest of event (without common header) */
    for (i = 0, c = char_string, h = hex_string; i < size; i++, ptr++) {
      snprintf(h, 4, (i % 16 <= 7) ? "%02x " : " %02x", *ptr);
      h += 3;

      *c++ = my_isalnum(&my_charset_bin, *ptr) ? *ptr : '.';

      if (i % 16 == 15) {
        /*
          my_b_printf() does not support full printf() formats, so we
          have to do it this way.

          TODO: Rewrite my_b_printf() to support full printf() syntax.
         */
        char emit_buf[256];
        size_t const bytes_written =
            snprintf(emit_buf, sizeof(emit_buf), "# %8.8lx %-48.48s |%16s|\n",
                     (unsigned long)(hexdump_from + (i & 0xfffffff0)),
                     hex_string, char_string);
        DBUG_ASSERT(static_cast<size_t>(bytes_written) < sizeof(emit_buf));
        my_b_write(file, (uchar *)emit_buf, bytes_written);
        hex_string[0] = 0;
        char_string[0] = 0;
        c = char_string;
        h = hex_string;
      }
    }
    *c = '\0';
    DBUG_ASSERT(hex_string[48] == 0);

    if (hex_string[0]) {
      char emit_buf[256];
      // Right-pad hex_string with spaces, up to 48 characters.
      memset(h, ' ', (sizeof(hex_string) - 1) - (h - hex_string));
      size_t const bytes_written =
          snprintf(emit_buf, sizeof(emit_buf), "# %8.8lx %-48.48s |%s|\n",
                   (unsigned long)(hexdump_from + (i & 0xfffffff0)), hex_string,
                   char_string);
      DBUG_ASSERT(static_cast<size_t>(bytes_written) < sizeof(emit_buf));
      my_b_write(file, (uchar *)emit_buf, bytes_written);
    }
    /*
      need a # to prefix the rest of printouts for example those of
      Rows_log_event::print_helper().
    */
    my_b_write(file, reinterpret_cast<const uchar *>("# "), 2);
  }
}

/**
  Auxiliary function that sets up a conversion table for m_b_write_quoted.

  The table has 256 elements.  The i'th element is 5 characters, the
  first being the length (1..4) and the remaining containing character
  #i quoted and not null-terminated.  If character #i does not need
  quoting (it is >= 32 and not backslash or single-quote), the table
  only contains the character itself.  A quoted character needs at
  most 4 bytes ("\xXX"), plus the length byte, so each element is 5
  bytes.

  This function is called exactly once even in a multi-threaded
  environment, because it is only called in the initializer of a
  static variable.

  @return Pointer to the table, a 256*5 character array where
  character i quoted .
*/
static const uchar *get_quote_table() {
  static uchar buf[256][5];
  for (int i = 0; i < 256; i++) {
    char str[6];
    switch (i) {
      case '\b':
        strcpy(str, "\\b");
        break;
      case '\f':
        strcpy(str, "\\f");
        break;
      case '\n':
        strcpy(str, "\\n");
        break;
      case '\r':
        strcpy(str, "\\r");
        break;
      case '\t':
        strcpy(str, "\\t");
        break;
      case '\\':
        strcpy(str, "\\\\");
        break;
      case '\'':
        strcpy(str, "\\'");
        break;
      default:
        if (i < 32)
          sprintf(str, "\\x%02x", i);
        else {
          str[0] = i;
          str[1] = '\0';
        }
        break;
    }
    buf[i][0] = strlen(str);
    memcpy(buf[i] + 1, str, strlen(str));
  }
  return (const uchar *)(buf);
}

/**
  Auxiliary function that sets up a conversion table for m_b_write_quoted.

  This conversion table is for SQL compatible string literals with
  printable characters and control characters.

  See get_quote_table for more details on structure of the table.

  @return Pointer to the table, a 256*5 character array where
  character i quoted .
 */
static const uchar *get_sql_str_table() {
  static uchar buf[256][5];
  for (int i = 0; i < 256; i++) {
    char str[6];
    switch (i) {
      case 0:
        strcpy(str, "\\0");
        break;
      case '\'':
        strcpy(str, "\\'");
        break;
      case '\"':
        strcpy(str, "\\\"");
        break;
      case '\b':
        strcpy(str, "\b");
        break;
      case '\n':
        strcpy(str, "\\n");
        break;
      case '\r':
        strcpy(str, "\\r");
        break;
      case '\t':
        strcpy(str, "\\t");
        break;
      case 26:
        strcpy(str, "\\Z");
        break;
      case '\\':
        strcpy(str, "\\\\");
        break;
      default:
        if (isprint(i)) {
          str[0] = i;
          str[1] = '\0';
        } else {
          str[0] = '\0';
        }
        break;
    }
    buf[i][0] = strlen(str);
    memcpy(buf[i] + 1, str, strlen(str));
  }
  return (const uchar *)(buf);
}

/**
  Auxiliary function that sets up a conversion table for m_b_write_quoted.

  This conversion table is for SQL compatible hexadecimal literals for any
  binary data.

  See get_quote_table for more details on structure of the table.

  @return Pointer to the table, a 256*5 character array where
  character i quoted .
 */
static const uchar *get_sql_bin_table() {
  static uchar buf[256][5];
  for (int i = 0; i < 256; i++) {
    char *str = (char *)&buf[i][1];
    sprintf(str, "%02X", i);
    buf[i][0] = strlen(str);
  }
  return (const uchar *)(buf);
}

/**
  Print using sql-compatible string format
 */
static bool print_sql_string;

/**
  Set the global print_sql_string option
 */
void my_b_set_print_sql_string(bool option) { print_sql_string = option; }

/**
  Prints a quoted string to io cache.

  If print_sql_string is off, print the string in quoted format - see
  get_quote_table for more details.

  If print_sql_string is on, print the string as string literal if the string
  only has printable characters and control characters. Otherwise print as
  hexadecimal literals. See get_sql_str_table and get_sql_bin_table for more
  details.

  @param[in] file              IO cache
  @param[in] ptr               Pointer to string
  @param[in] length            String length

  @retval false Success
  @retval true Failure
*/
static bool my_b_write_quoted(IO_CACHE *file, const uchar *ptr, uint length) {
  const uchar *s;
  const uchar *table;
  static const uchar *quote_table = get_quote_table();
  static const uchar *sql_str_table = get_sql_str_table();
  static const uchar *sql_bin_table = get_sql_bin_table();

  if (print_sql_string) {
    bool is_printable = true;
    uint len = length;
    for (s = ptr; len > 0; s++, len--) {
      const uchar *len_and_str = sql_str_table + *s * 5;

      /* 0 length means non-printable */
      if (len_and_str[0] == 0) {
        is_printable = false;
        break;
      }
    }

    if (is_printable) {
      /* Use string literal for printable chars */
      table = sql_str_table;
      my_b_printf(file, "'");
    } else {
      /* Use hexadecimal literal for binary */
      table = sql_bin_table;
      my_b_printf(file, "X'");
    }
  } else {
    table = quote_table;
    my_b_printf(file, "'");
  }

  for (s = ptr; length > 0; s++, length--) {
    const uchar *len_and_str = table + *s * 5;
    my_b_write(file, len_and_str + 1, len_and_str[0]);
  }
  if (my_b_printf(file, "'") == (size_t)-1) return true;
  return false;
}

/**
  Prints a bit string to io cache in format  b'1010'.

  @param[in] file              IO cache
  @param[in] ptr               Pointer to string
  @param[in] nbits             Number of bits
*/
static void my_b_write_bit(IO_CACHE *file, const uchar *ptr, uint nbits) {
  uint bitnum, nbits8 = ((nbits + 7) / 8) * 8, skip_bits = nbits8 - nbits;
  my_b_printf(file, "b'");
  for (bitnum = skip_bits; bitnum < nbits8; bitnum++) {
    int is_set = (ptr[(bitnum) / 8] >> (7 - bitnum % 8)) & 0x01;
    my_b_write(file, (const uchar *)(is_set ? "1" : "0"), 1);
  }
  my_b_printf(file, "'");
}

/**
  Prints a packed string to io cache.
  The string consists of length packed to 1 or 2 bytes,
  followed by string data itself.

  @param[in] file              IO cache
  @param[in] ptr               Pointer to string
  @param[in] length            String size

  @retval   - number of bytes scanned.
*/
static size_t my_b_write_quoted_with_length(IO_CACHE *file, const uchar *ptr,
                                            uint length) {
  if (length < 256) {
    length = *ptr;
    my_b_write_quoted(file, ptr + 1, length);
    return length + 1;
  } else {
    length = uint2korr(ptr);
    my_b_write_quoted(file, ptr + 2, length);
    return length + 2;
  }
}

/**
  Prints a 32-bit number in signed or unsigned representation

  @param[in] file              IO cache
  @param[in] si                Signed number
  @param[in] ui                Unsigned number
  @param[in] is_unsigned       Print unsigned representation
*/
static void my_b_write_sint32_and_uint32(IO_CACHE *file, int32 si, uint32 ui,
                                         bool is_unsigned) {
  if (!is_unsigned)
    my_b_printf(file, "%d", si);
  else
    my_b_printf(file, "%u", ui);
}

#ifndef MYSQL_SERVER
static const char *json_diff_operation_name(enum_json_diff_operation op,
                                            int last_path_char) {
  switch (op) {
    case enum_json_diff_operation::REPLACE:
      return "JSON_REPLACE";
    case enum_json_diff_operation::INSERT:
      if (last_path_char == ']')
        return "JSON_ARRAY_INSERT";
      else
        return "JSON_INSERT";
    case enum_json_diff_operation::REMOVE:
      return "JSON_REMOVE";
  }
  /* NOTREACHED */
  /* purecov: begin deadcode */
  DBUG_ASSERT(0);
  return nullptr;
  /* purecov: end */
}

static bool json_wrapper_to_string(IO_CACHE *out, String *buf,
                                   Json_wrapper *wrapper, bool json_type) {
  if (wrapper->to_string(buf, false, "json_wrapper_to_string"))
    return true; /* purecov: inspected */  // OOM
  if (json_type)
    return my_b_write_quoted(out, (uchar *)buf->ptr(), buf->length());
  switch (wrapper->type()) {
    case enum_json_type::J_NULL:
    case enum_json_type::J_DECIMAL:
    case enum_json_type::J_INT:
    case enum_json_type::J_UINT:
    case enum_json_type::J_DOUBLE:
    case enum_json_type::J_BOOLEAN:
      my_b_write(out, (uchar *)buf->ptr(), buf->length());
      break;
    case enum_json_type::J_STRING:
    case enum_json_type::J_DATE:
    case enum_json_type::J_TIME:
    case enum_json_type::J_DATETIME:
    case enum_json_type::J_TIMESTAMP:
    case enum_json_type::J_OPAQUE:
    case enum_json_type::J_ERROR:
      my_b_write_quoted(out, (uchar *)buf->ptr(), buf->length());
      break;
    case enum_json_type::J_OBJECT:
    case enum_json_type::J_ARRAY:
      my_b_printf(out, "CAST(");
      my_b_write_quoted(out, (uchar *)buf->ptr(), buf->length());
      my_b_printf(out, " AS JSON)");
      break;
    default:
      DBUG_ASSERT(0); /* purecov: deadcode */
  }
  return false;
}

static const char *print_json_diff(IO_CACHE *out, const uchar *data,
                                   size_t length, const char *col_name) {
  DBUG_TRACE;

  static const char *line_separator = "\n###      ";

  // read length
  const uchar *p = data;

  const uchar *start_p = p;
  size_t start_length = length;

  // Read the list of operations.
  std::vector<const char *> operation_names;
  while (length) {
    // read operation
    int operation_int = *p;
    if (operation_int >= JSON_DIFF_OPERATION_COUNT)
      return "reading operation type (invalid operation code)";
    enum_json_diff_operation operation =
        static_cast<enum_json_diff_operation>(operation_int);
    p++;
    length--;

    // skip path
    size_t path_length;
    if (net_field_length_checked<size_t>(&p, &length, &path_length))
      return "reading path length to skip";
    if (path_length > length) return "skipping path";
    p += path_length;
    length -= path_length;

    // compute operation name
    const char *operation_name = json_diff_operation_name(operation, p[-1]);
    operation_names.push_back(operation_name);

    // skip value
    if (operation != enum_json_diff_operation::REMOVE) {
      size_t value_length;
      if (net_field_length_checked<size_t>(&p, &length, &value_length))
        return "reading value length to skip";
      if (value_length > length) return "skipping value";
      p += value_length;
      length -= value_length;
    }
  }

  // Print function names in reverse order.
  bool printed = false;
  for (int i = operation_names.size() - 1; i >= 0; i--) {
    if (i == 0 || operation_names[i - 1] != operation_names[i]) {
      if (printed)
        if (my_b_printf(out, "%s", line_separator) == (size_t)-1)
          return "printing line separator";
      /* purecov: inspected */  // error writing to output
      if (my_b_printf(out, "%s(", operation_names[i]) == (size_t)-1)
        return "printing function name";
      /* purecov: inspected */  // error writing to output
      printed = true;
    }
  }

  // Print column id
  if (my_b_printf(out, "%s", col_name) == (size_t)-1)
    return "printing column id";
  /* purecov: inspected */  // error writing to output

  // In case this vector is empty (a no-op), make an early return
  // after printing only the column name
  if (operation_names.size() == 0) return nullptr;

  // Print comma between column name and next function argument
  if (my_b_printf(out, ", ") == (size_t)-1) return "printing comma";
  /* purecov: inspected */  // error writing to output

  // Print paths and values.
  p = start_p;
  length = start_length;
  StringBuffer<STRING_BUFFER_USUAL_SIZE> buf;
  int diff_i = 0;
  while (length) {
    // Read operation
    enum_json_diff_operation operation = (enum_json_diff_operation)*p;
    p++;
    length--;

    // Read path length
    size_t path_length;
    if (net_field_length_checked<size_t>(&p, &length, &path_length))
      return "reading path length";
    /* purecov: deadcode */  // already checked in loop above

    // Print path
    if (my_b_write_quoted(out, p, path_length)) return "printing path";
    /* purecov: inspected */  // error writing to output
    p += path_length;
    length -= path_length;

    if (operation != enum_json_diff_operation::REMOVE) {
      // Print comma between path and value
      if (my_b_printf(out, ", ") == (size_t)-1) return "printing comma";
      /* purecov: inspected */  // error writing to output

      // Read value length
      size_t value_length;
      if (net_field_length_checked<size_t>(&p, &length, &value_length))
        return "reading value length";
      /* purecov: deadcode */  // already checked in loop above

      // Read value
      json_binary::Value value =
          json_binary::parse_binary((const char *)p, value_length);
      p += value_length;
      length -= value_length;
      if (value.type() == json_binary::Value::ERROR)
        return "parsing json value";
      Json_wrapper wrapper(value);

      // Print value
      buf.length(0);
      if (json_wrapper_to_string(out, &buf, &wrapper, false))
        return "converting json to string";
      /* purecov: inspected */  // OOM
      buf.length(0);
    }

    // Print closing parenthesis
    if (length == 0 || operation_names[diff_i + 1] != operation_names[diff_i])
      if (my_b_printf(out, ")") == (size_t)-1)
        return "printing closing parenthesis";
    /* purecov: inspected */  // error writing to output

    // Print ending comma
    if (length != 0)
      if (my_b_printf(out, ",%s", line_separator) == (size_t)-1)
        return "printing comma";
    /* purecov: inspected */  // error writing to output

    diff_i++;
  }

  return nullptr;
}
#endif  // ifndef MYSQL_SERVER

/**
  Print a packed value of the given SQL type into IO cache

  @param[in] file              IO cache
  @param[in] ptr               Pointer to string
  @param[in] type              Column type
  @param[in] meta              Column meta information
  @param[out] typestr          SQL type string buffer (for verbose output)
  @param[in] typestr_length    Size of typestr
  @param[in] col_name          Column name
  @param[in] is_partial        True if this is a JSON column that will be
                               read in partial format, false otherwise.
  @param[in] is_unsigned       Unsigned type

  @retval 0 on error
  @retval number of bytes scanned from ptr for non-NULL fields, or
  another positive number for NULL fields
*/
#ifndef MYSQL_SERVER
static size_t log_event_print_value(IO_CACHE *file, const uchar *ptr, uint type,
                                    uint meta, char *typestr,
                                    size_t typestr_length, char *col_name,
                                    bool is_partial, bool is_unsigned) {
  uint32 length = 0;

  if (type == MYSQL_TYPE_STRING) {
    if (meta >= 256) {
      uint byte0 = meta >> 8;
      uint byte1 = meta & 0xFF;

      if ((byte0 & 0x30) != 0x30) {
        /* a long CHAR() field: see #37426 */
        length = byte1 | (((byte0 & 0x30) ^ 0x30) << 4);
        type = byte0 | 0x30;
      } else
        length = meta & 0xFF;
    } else
      length = meta;
  }

  switch (type) {
    case MYSQL_TYPE_LONG: {
      snprintf(typestr, typestr_length, is_unsigned ? "INT UNSIGNED" : "INT");
      if (!ptr) return my_b_printf(file, "NULL");
      int32 si = sint4korr(ptr);
      uint32 ui = uint4korr(ptr);
      my_b_write_sint32_and_uint32(file, si, ui, is_unsigned);
      return 4;
    }

    case MYSQL_TYPE_TINY: {
      snprintf(typestr, typestr_length,
               is_unsigned ? "TINYINT UNSIGNED" : "TINYINT");
      if (!ptr) return my_b_printf(file, "NULL");
      my_b_write_sint32_and_uint32(file, (int)(signed char)*ptr,
                                   (uint)(unsigned char)*ptr, is_unsigned);
      return 1;
    }

    case MYSQL_TYPE_SHORT: {
      snprintf(typestr, typestr_length,
               is_unsigned ? "SHORTINT UNSIGNED" : "SHORTINT");
      if (!ptr) return my_b_printf(file, "NULL");
      int32 si = (int32)sint2korr(ptr);
      uint32 ui = (uint32)uint2korr(ptr);
      my_b_write_sint32_and_uint32(file, si, ui, is_unsigned);
      return 2;
    }

    case MYSQL_TYPE_INT24: {
      snprintf(typestr, typestr_length,
               is_unsigned ? "MEDIUMINT UNSIGNED" : "MEDIUMINT");
      if (!ptr) return my_b_printf(file, "NULL");
      int32 si = sint3korr(ptr);
      uint32 ui = uint3korr(ptr);
      my_b_write_sint32_and_uint32(file, si, ui, is_unsigned);
      return 3;
    }

    case MYSQL_TYPE_LONGLONG: {
      snprintf(typestr, typestr_length,
               is_unsigned ? "LONGINT UNSIGNED" : "LONGINT");
      if (!ptr) return my_b_printf(file, "NULL");
      char tmp[64];
      if (!is_unsigned) {
        longlong si = sint8korr(ptr);
        longlong10_to_str(si, tmp, -10);
        my_b_printf(file, "%s", tmp);
      } else {
        ulonglong ui = uint8korr(ptr);
        longlong10_to_str((longlong)ui, tmp, 10);
        my_b_printf(file, "%s", tmp);
      }
      return 8;
    }

    case MYSQL_TYPE_NEWDECIMAL: {
      uint precision = meta >> 8;
      uint decimals = meta & 0xFF;
      snprintf(typestr, typestr_length, "DECIMAL(%d,%d)", precision, decimals);
      if (!ptr) return my_b_printf(file, "NULL");
      uint bin_size = my_decimal_get_binary_size(precision, decimals);
      my_decimal dec;
      binary2my_decimal(E_DEC_FATAL_ERROR, pointer_cast<const uchar *>(ptr),
                        &dec, precision, decimals);
      char buff[DECIMAL_MAX_STR_LENGTH + 1];
      int len = sizeof(buff);
      decimal2string(&dec, buff, &len);
      my_b_printf(file, "%s", buff);
      return bin_size;
    }

    case MYSQL_TYPE_FLOAT: {
      snprintf(typestr, typestr_length, "FLOAT");
      if (!ptr) return my_b_printf(file, "NULL");
      float fl = float4get(ptr);
      char tmp[320];
      sprintf(tmp, "%-20g", (double)fl);
      my_b_printf(file, "%s", tmp); /* my_b_printf doesn't support %-20g */
      return 4;
    }

    case MYSQL_TYPE_DOUBLE: {
      strcpy(typestr, "DOUBLE");
      if (!ptr) return my_b_printf(file, "NULL");
      double dbl = float8get(ptr);
      char tmp[320];
      sprintf(tmp, "%-.20g", dbl); /* my_b_printf doesn't support %-20g */
      my_b_printf(file, "%s", tmp);
      return 8;
    }

    case MYSQL_TYPE_BIT: {
      /* Meta-data: bit_len, bytes_in_rec, 2 bytes */
      uint nbits = ((meta >> 8) * 8) + (meta & 0xFF);
      snprintf(typestr, typestr_length, "BIT(%d)", nbits);
      if (!ptr) return my_b_printf(file, "NULL");
      length = (nbits + 7) / 8;
      my_b_write_bit(file, ptr, nbits);
      return length;
    }

    case MYSQL_TYPE_TIMESTAMP: {
      snprintf(typestr, typestr_length, "TIMESTAMP");
      if (!ptr) return my_b_printf(file, "NULL");
      uint32 i32 = uint4korr(ptr);
      my_b_printf(file, "%d", i32);
      return 4;
    }

    case MYSQL_TYPE_TIMESTAMP2: {
      snprintf(typestr, typestr_length, "TIMESTAMP(%d)", meta);
      if (!ptr) return my_b_printf(file, "NULL");
      char buf[MAX_DATE_STRING_REP_LENGTH];
      struct timeval tm;
      my_timestamp_from_binary(&tm, ptr, meta);
      int buflen = my_timeval_to_str(&tm, buf, meta);
      my_b_write(file, pointer_cast<uchar *>(buf), buflen);
      return my_timestamp_binary_length(meta);
    }

    case MYSQL_TYPE_DATETIME: {
      snprintf(typestr, typestr_length, "DATETIME");
      if (!ptr) return my_b_printf(file, "NULL");
      size_t d, t;
      uint64 i64 = uint8korr(ptr); /* YYYYMMDDhhmmss */
      d = static_cast<size_t>(i64 / 1000000);
      t = i64 % 1000000;
      my_b_printf(file, "%04d-%02d-%02d %02d:%02d:%02d",
                  static_cast<int>(d / 10000),
                  static_cast<int>(d % 10000) / 100, static_cast<int>(d % 100),
                  static_cast<int>(t / 10000),
                  static_cast<int>(t % 10000) / 100, static_cast<int>(t % 100));
      return 8;
    }

    case MYSQL_TYPE_DATETIME2: {
      snprintf(typestr, typestr_length, "DATETIME(%d)", meta);
      if (!ptr) return my_b_printf(file, "NULL");
      char buf[MAX_DATE_STRING_REP_LENGTH];
      MYSQL_TIME ltime;
      longlong packed = my_datetime_packed_from_binary(ptr, meta);
      TIME_from_longlong_datetime_packed(&ltime, packed);
      int buflen = my_datetime_to_str(ltime, buf, meta);
      my_b_write_quoted(file, (uchar *)buf, buflen);
      return my_datetime_binary_length(meta);
    }

    case MYSQL_TYPE_TIME: {
      snprintf(typestr, typestr_length, "TIME");
      if (!ptr) return my_b_printf(file, "NULL");
      uint32 i32 = uint3korr(ptr);
      my_b_printf(file, "'%02d:%02d:%02d'", i32 / 10000, (i32 % 10000) / 100,
                  i32 % 100);
      return 3;
    }

    case MYSQL_TYPE_TIME2: {
      snprintf(typestr, typestr_length, "TIME(%d)", meta);
      if (!ptr) return my_b_printf(file, "NULL");
      char buf[MAX_DATE_STRING_REP_LENGTH];
      MYSQL_TIME ltime;
      longlong packed = my_time_packed_from_binary(ptr, meta);
      TIME_from_longlong_time_packed(&ltime, packed);
      int buflen = my_time_to_str(ltime, buf, meta);
      my_b_write_quoted(file, (uchar *)buf, buflen);
      return my_time_binary_length(meta);
    }

    case MYSQL_TYPE_NEWDATE: {
      snprintf(typestr, typestr_length, "DATE");
      if (!ptr) return my_b_printf(file, "NULL");
      uint32 tmp = uint3korr(ptr);
      int part;
      char buf[11];
      char *pos = &buf[10];  // start from '\0' to the beginning

      /* Copied from field.cc */
      *pos-- = 0;  // End NULL
      part = (int)(tmp & 31);
      *pos-- = (char)('0' + part % 10);
      *pos-- = (char)('0' + part / 10);
      *pos-- = ':';
      part = (int)(tmp >> 5 & 15);
      *pos-- = (char)('0' + part % 10);
      *pos-- = (char)('0' + part / 10);
      *pos-- = ':';
      part = (int)(tmp >> 9);
      *pos-- = (char)('0' + part % 10);
      part /= 10;
      *pos-- = (char)('0' + part % 10);
      part /= 10;
      *pos-- = (char)('0' + part % 10);
      part /= 10;
      *pos = (char)('0' + part);
      my_b_printf(file, "'%s'", buf);
      return 3;
    }

    case MYSQL_TYPE_YEAR: {
      snprintf(typestr, typestr_length, "YEAR");
      if (!ptr) return my_b_printf(file, "NULL");
      uint32 i32 = *ptr;
      my_b_printf(file, "%04d", i32 + 1900);
      return 1;
    }

    case MYSQL_TYPE_ENUM:
      switch (meta & 0xFF) {
        case 1:
          snprintf(typestr, typestr_length, "ENUM(1 byte)");
          if (!ptr) return my_b_printf(file, "NULL");
          my_b_printf(file, "%d", (int)*ptr);
          return 1;
        case 2: {
          snprintf(typestr, typestr_length, "ENUM(2 bytes)");
          if (!ptr) return my_b_printf(file, "NULL");
          int32 i32 = uint2korr(ptr);
          my_b_printf(file, "%d", i32);
          return 2;
        }
        default:
          my_b_printf(file, "!! Unknown ENUM packlen=%d", meta & 0xFF);
          return 0;
      }
      break;

    case MYSQL_TYPE_SET:
      snprintf(typestr, typestr_length, "SET(%d bytes)", meta & 0xFF);
      if (!ptr) return my_b_printf(file, "NULL");
      my_b_write_bit(file, ptr, (meta & 0xFF) * 8);
      return meta & 0xFF;

    case MYSQL_TYPE_BLOB:
      switch (meta) {
        case 1:
          snprintf(typestr, typestr_length, "TINYBLOB/TINYTEXT");
          if (!ptr) return my_b_printf(file, "NULL");
          length = *ptr;
          my_b_write_quoted(file, ptr + 1, length);
          return length + 1;
        case 2:
          snprintf(typestr, typestr_length, "BLOB/TEXT");
          if (!ptr) return my_b_printf(file, "NULL");
          length = uint2korr(ptr);
          my_b_write_quoted(file, ptr + 2, length);
          return length + 2;
        case 3:
          snprintf(typestr, typestr_length, "MEDIUMBLOB/MEDIUMTEXT");
          if (!ptr) return my_b_printf(file, "NULL");
          length = uint3korr(ptr);
          my_b_write_quoted(file, ptr + 3, length);
          return length + 3;
        case 4:
          snprintf(typestr, typestr_length, "LONGBLOB/LONGTEXT");
          if (!ptr) return my_b_printf(file, "NULL");
          length = uint4korr(ptr);
          my_b_write_quoted(file, ptr + 4, length);
          return length + 4;
        default:
          my_b_printf(file, "!! Unknown BLOB packlen=%d", length);
          return 0;
      }

    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_VAR_STRING:
      length = meta;
      snprintf(typestr, typestr_length, "VARSTRING(%d)", length);
      if (!ptr) return my_b_printf(file, "NULL");
      return my_b_write_quoted_with_length(file, ptr, length);

    case MYSQL_TYPE_STRING:
      snprintf(typestr, typestr_length, "STRING(%d)", length);
      if (!ptr) return my_b_printf(file, "NULL");
      return my_b_write_quoted_with_length(file, ptr, length);

    case MYSQL_TYPE_JSON: {
      snprintf(typestr, typestr_length, "JSON");
      if (!ptr) return my_b_printf(file, "NULL");
      length = uint4korr(ptr);
      ptr += 4;
      if (is_partial) {
        const char *error = print_json_diff(file, ptr, length, col_name);
        if (error != nullptr)
          my_b_printf(file, "Error %s while printing JSON diff\n", error);
      } else {
        json_binary::Value value =
            json_binary::parse_binary((const char *)ptr, length);
        if (value.type() == json_binary::Value::ERROR) {
          if (my_b_printf(
                  file,
                  "Invalid JSON\n")) /* purecov: inspected */  // corrupted
                                                               // event
            return 0; /* purecov: inspected */  // error writing output
        } else {
          Json_wrapper wrapper(value);
          StringBuffer<STRING_BUFFER_USUAL_SIZE> s;
          if (json_wrapper_to_string(file, &s, &wrapper, true))
            my_b_printf(file, "Failed to format JSON object as string.\n");
          /* purecov: inspected */  // OOM
        }
      }
      return length + meta;
    }
    default: {
      char tmp[5];
      snprintf(tmp, sizeof(tmp), "%04x", meta);
      my_b_printf(file,
                  "!! Don't know how to handle column type=%d meta=%d (%s)\n",
                  type, meta, tmp);
    } break;
  }
  *typestr = 0;
  return 0;
}
#endif

/**
  Print a packed row into IO cache

  @param[in] file              IO cache
  @param[in] td                Table definition
  @param[in] print_event_info  Print parameters
  @param[in] cols_bitmap       Column bitmaps.
  @param[in] value             Pointer to packed row
  @param[in] prefix            Row's SQL clause ("SET", "WHERE", etc)

  @retval   - number of bytes scanned.
*/

size_t Rows_log_event::print_verbose_one_row(
    IO_CACHE *file, table_def *td, PRINT_EVENT_INFO *print_event_info,
    MY_BITMAP *cols_bitmap, const uchar *value, const uchar *prefix,
    enum_row_image_type row_image_type) {
  const uchar *value0 = value;
  char typestr[64] = "";

  // Read value_options if this is AI for PARTIAL_UPDATE_ROWS_EVENT
  ulonglong value_options = 0;
  Bit_reader partial_bits;
  if (get_type_code() == binary_log::PARTIAL_UPDATE_ROWS_EVENT &&
      row_image_type == enum_row_image_type::UPDATE_AI) {
    size_t length = m_rows_end - value;
    if (net_field_length_checked<ulonglong>(&value, &length, &value_options)) {
      my_b_printf(file,
                  "*** Error reading binlog_row_value_options from "
                  "Partial_update_rows_log_event\n");
      return 0;
    }
    if ((value_options & PARTIAL_JSON_UPDATES) != 0) {
      partial_bits.set_ptr(value);
      value += (td->json_column_count() + 7) / 8;
    }
  }

  /*
    Metadata bytes which gives the information about nullabity of
    master columns. Master writes one bit for each column in the
    image.
  */
  Bit_reader null_bits(value);
  value += (bitmap_bits_set(cols_bitmap) + 7) / 8;

  my_b_printf(file, "%s", prefix);

  for (size_t i = 0; i < td->size(); i++) {
    /*
      Note: need to read partial bit before reading cols_bitmap, since
      the partial_bits bitmap has a bit for every JSON column
      regardless of whether it is included in the bitmap or not.
    */
    bool is_partial = (value_options & PARTIAL_JSON_UPDATES) != 0 &&
                      row_image_type == enum_row_image_type::UPDATE_AI &&
                      td->type(i) == MYSQL_TYPE_JSON && partial_bits.get();

    if (bitmap_is_set(cols_bitmap, i) == 0) continue;

    bool is_null = null_bits.get();

    if (td->have_column_names())
      my_b_printf(file, "###   %s=", td->get_column_name(i));
    else
      my_b_printf(file, "###   @%d=", static_cast<int>(i + 1));
    if (!is_null) {
      size_t fsize =
          td->calc_field_size((uint)i, pointer_cast<const uchar *>(value));
      if (fsize > (size_t)(m_rows_end - value)) {
        my_b_printf(file,
                    "***Corrupted replication event was detected: "
                    "field size is set to %u, but there are only %u bytes "
                    "left of the event. Not printing the value***\n",
                    (uint)fsize, (uint)(m_rows_end - value));
        return 0;
      }
    }
    char col_name[256];
    sprintf(col_name, "@%lu", (unsigned long)i + 1);
    size_t size = log_event_print_value(
        file, is_null ? nullptr : value, td->type(i), td->field_metadata(i),
        typestr, sizeof(typestr), col_name, is_partial, td->is_unsigned(i));
    if (!size) return 0;

    if (!is_null) value += size;

    if (print_event_info->verbose > 1) {
      my_b_printf(file, " /* ");

      my_b_printf(file, "%s ", typestr);

      my_b_printf(file, "meta=%d nullable=%d is_null=%d ",
                  td->field_metadata(i), td->maybe_null(i), is_null);
      my_b_printf(file, "*/");
    }

    my_b_printf(file, "\n");
  }
  return value - value0;
}

/**
  Print a row event into IO cache in human readable form (in SQL format)

  @param[in] file              IO cache
  @param[in] print_event_info  Print parameters
*/
void Rows_log_event::print_verbose(IO_CACHE *file,
                                   PRINT_EVENT_INFO *print_event_info) {
  // Quoted length of the identifier can be twice the original length
  char quoted_db[1 + NAME_LEN * 2 + 2];
  char quoted_table[1 + NAME_LEN * 2 + 2];
  size_t quoted_db_len, quoted_table_len;
  Table_map_log_event *map;
  table_def *td;
  const char *sql_command, *sql_clause1, *sql_clause2;
  Log_event_type general_type_code = get_general_type_code();

  enum_row_image_type row_image_type =
      get_general_type_code() == binary_log::WRITE_ROWS_EVENT
          ? enum_row_image_type::WRITE_AI
          : get_general_type_code() == binary_log::DELETE_ROWS_EVENT
                ? enum_row_image_type::DELETE_BI
                : enum_row_image_type::UPDATE_BI;

  if (m_extra_row_info.have_ndb_info() ||
      DBUG_EVALUATE_IF("simulate_error_in_ndb_info_print", 1, 0)) {
    int extra_row_ndb_info_payload_len =
        m_extra_row_info.get_ndb_length() - EXTRA_ROW_INFO_HEADER_LENGTH;

    if (m_extra_row_info.get_ndb_length() < EXTRA_ROW_INFO_HEADER_LENGTH) {
      my_b_printf(file,
                  "***Error: The number of extra_row_ndb_info is smaller"
                  " than the minimum acceptable value.\n");
      return;
    }
    unsigned char *ndb_info = m_extra_row_info.get_ndb_info();
    my_b_printf(file, "### Extra row ndb info: data_format: %u, len: %u, ",
                ndb_info[EXTRA_ROW_INFO_FORMAT_OFFSET],
                extra_row_ndb_info_payload_len);
    /*
       Buffer for hex view of string, including '0x' prefix,
       2 hex chars / byte and trailing 0
    */
    const int buff_len = 2 + (256 * 2) + 1;
    char buff[buff_len];
    str_to_hex(buff, (const char *)&(ndb_info[EXTRA_ROW_INFO_HEADER_LENGTH]),
               extra_row_ndb_info_payload_len);
    my_b_printf(file, "data: %s\n", buff);
  }

  if (m_extra_row_info.have_part()) {
    if (general_type_code == binary_log::UPDATE_ROWS_EVENT) {
      my_b_printf(file,
                  "### Extra row info for partitioning: source_partition: %d"
                  " target_partition: %d",
                  m_extra_row_info.get_source_partition_id(),
                  m_extra_row_info.get_partition_id());
    } else
      my_b_printf(file, "### Extra row info for partitioning: partition: %u",
                  m_extra_row_info.get_partition_id());
    my_b_printf(file, "\n");
  }

  switch (general_type_code) {
    case binary_log::WRITE_ROWS_EVENT:
      sql_command = "INSERT INTO";
      sql_clause1 = "### SET\n";
      sql_clause2 = nullptr;
      break;
    case binary_log::DELETE_ROWS_EVENT:
      sql_command = "DELETE FROM";
      sql_clause1 = "### WHERE\n";
      sql_clause2 = nullptr;
      break;
    case binary_log::UPDATE_ROWS_EVENT:
    case binary_log::PARTIAL_UPDATE_ROWS_EVENT:
      sql_command = "UPDATE";
      sql_clause1 = "### WHERE\n";
      sql_clause2 = "### SET\n";
      break;
    default:
      sql_command = sql_clause1 = sql_clause2 = nullptr;
      DBUG_ASSERT(0); /* Not possible */
  }

  if (!(map = print_event_info->m_table_map.get_table(m_table_id)) ||
      !(td = map->create_table_def())) {
    char llbuff[22];
    my_b_printf(file, "### Row event for unknown table #%s",
                llstr(m_table_id, llbuff));
    return;
  }

  /* If the write rows event contained no values for the AI */
  if (((general_type_code == binary_log::WRITE_ROWS_EVENT) &&
       (m_rows_buf == m_rows_end))) {
    my_b_printf(file, "### INSERT INTO `%s`.`%s` VALUES ()\n",
                map->get_db_name(), map->get_table_name());
    goto end;
  }

  for (const uchar *value = m_rows_buf; value < m_rows_end;) {
    size_t length;
    quoted_db_len =
        my_strmov_quoted_identifier((char *)quoted_db, map->get_db_name());
    quoted_table_len = my_strmov_quoted_identifier((char *)quoted_table,
                                                   map->get_table_name());
    quoted_db[quoted_db_len] = '\0';
    quoted_table[quoted_table_len] = '\0';
    my_b_printf(file, "### %s %s.%s\n", sql_command, quoted_db, quoted_table);
    /* Print the first image */
    if (!(length = print_verbose_one_row(file, td, print_event_info, &m_cols,
                                         value, (const uchar *)sql_clause1,
                                         row_image_type)))
      goto end;
    value += length;

    /* Print the second image (for UPDATE only) */
    if (sql_clause2) {
      if (!(length = print_verbose_one_row(
                file, td, print_event_info, &m_cols_ai, value,
                (const uchar *)sql_clause2, enum_row_image_type::UPDATE_AI)))
        goto end;
      value += length;
    }
  }

end:
  delete td;
}

void Log_event::print_base64(IO_CACHE *file, PRINT_EVENT_INFO *print_event_info,
                             bool more) const {
  const uchar *ptr = (const uchar *)temp_buf;
  uint32 size = uint4korr(ptr + EVENT_LEN_OFFSET);
  DBUG_TRACE;

  uint64 const tmp_str_sz = base64_needed_encoded_length((uint64)size);
  char *const tmp_str =
      (char *)my_malloc(key_memory_log_event, tmp_str_sz, MYF(MY_WME));
  if (!tmp_str) {
    fprintf(stderr,
            "\nError: Out of memory. "
            "Could not print correct binlog event.\n");
    return;
  }

  if (base64_encode(ptr, (size_t)size, tmp_str)) {
    DBUG_ASSERT(0);
  }

  if (print_event_info->base64_output_mode != BASE64_OUTPUT_DECODE_ROWS) {
    if (my_b_tell(file) == 0) {
      my_b_printf(file, "\nBINLOG '\n");
      print_event_info->inside_binlog = true;
    }

    my_b_printf(file, "%s\n", tmp_str);

    if (!more) {
      my_b_printf(file, "'%s\n", print_event_info->delimiter);
      print_event_info->inside_binlog = false;
    }
  }

  if (print_event_info->verbose) {
    Rows_log_event *ev = nullptr;
    Log_event_type et = (Log_event_type)ptr[EVENT_TYPE_OFFSET];

    enum_binlog_checksum_alg ev_checksum_alg = common_footer->checksum_alg;
    Format_description_event fd_evt =
        Format_description_event(BINLOG_VERSION, server_version);
    fd_evt.footer()->checksum_alg = ev_checksum_alg;

    switch (et) {
      case binary_log::TABLE_MAP_EVENT: {
        Table_map_log_event *map;
        map = new Table_map_log_event((const char *)ptr, &fd_evt);
        print_event_info->m_table_map.set_table(map->get_table_id(), map);
        break;
      }
      case binary_log::WRITE_ROWS_EVENT:
      case binary_log::WRITE_ROWS_EVENT_V1: {
        ev = new Write_rows_log_event((const char *)ptr, &fd_evt);
        break;
      }
      case binary_log::DELETE_ROWS_EVENT:
      case binary_log::DELETE_ROWS_EVENT_V1: {
        ev = new Delete_rows_log_event((const char *)ptr, &fd_evt);
        break;
      }
      case binary_log::UPDATE_ROWS_EVENT:
      case binary_log::UPDATE_ROWS_EVENT_V1:
      case binary_log::PARTIAL_UPDATE_ROWS_EVENT: {
        ev = new Update_rows_log_event((const char *)ptr, &fd_evt);
        break;
      }
      default:
        break;
    }

    if (print_event_info->inside_binlog) {
      if (ev) print_event_info->verbose_events.push_back(ev);
    } else {
      std::vector<Rows_log_event *> &evs = print_event_info->verbose_events;
      for (size_t i = 0; i < evs.size(); ++i) {
        Rows_log_event *rle = evs.at(i);
        rle->print_verbose(file, print_event_info);
        delete rle;
      }
      evs.clear();
      if (ev) {
        ev->print_verbose(&print_event_info->footer_cache, print_event_info);
        delete ev;
      }
    }
  }

  my_free(tmp_str);
}

/*
  Log_event::print_timestamp()
*/

void Log_event::print_timestamp(IO_CACHE *file, time_t *ts) const {
  struct tm *res;
  /*
    In some Windows versions timeval.tv_sec is defined as "long",
    not as "time_t" and can be of a different size.
    Let's use a temporary time_t variable to execute localtime()
    with a correct argument type.
  */
  time_t ts_tmp = ts ? *ts : (ulong)common_header->when.tv_sec;
  DBUG_TRACE;
  struct tm tm_tmp;
  localtime_r(&ts_tmp, (res = &tm_tmp));
  my_b_printf(file, "%02d%02d%02d %2d:%02d:%02d", res->tm_year % 100,
              res->tm_mon + 1, res->tm_mday, res->tm_hour, res->tm_min,
              res->tm_sec);
}

#endif /* !MYSQL_SERVER */

#if defined(MYSQL_SERVER)
inline Log_event::enum_skip_reason Log_event::continue_group(
    Relay_log_info *rli) {
  if (rli->slave_skip_counter == 1) return Log_event::EVENT_SKIP_IGNORE;
  return Log_event::do_shall_skip(rli);
}

/**
   @param end_group_sets_max_dbs  when true the group terminal event
                          can carry partition info, see a note below.
   @return true  in cases the current event
                 carries partition data,
           false otherwise

   @note Some events combination may force to adjust partition info.
         In particular BEGIN, BEGIN_LOAD_QUERY_EVENT, COMMIT
         where none of the events holds partitioning data
         causes the sequential applying of the group through
         assigning OVER_MAX_DBS_IN_EVENT_MTS to mts_accessed_dbs
         of the group terminator (e.g COMMIT query) event.
*/
bool Log_event::contains_partition_info(bool end_group_sets_max_dbs) {
  bool res;

  switch (get_type_code()) {
    case binary_log::TABLE_MAP_EVENT:
    case binary_log::EXECUTE_LOAD_QUERY_EVENT:
    case binary_log::TRANSACTION_PAYLOAD_EVENT:
      res = true;

      break;

    case binary_log::QUERY_EVENT: {
      Query_log_event *qev = static_cast<Query_log_event *>(this);
      if ((ends_group() && end_group_sets_max_dbs) ||
          (qev->is_query_prefix_match(STRING_WITH_LEN("XA COMMIT")) ||
           qev->is_query_prefix_match(STRING_WITH_LEN("XA ROLLBACK")))) {
        res = true;
        qev->mts_accessed_dbs = OVER_MAX_DBS_IN_EVENT_MTS;
      } else
        res = (!ends_group() && !starts_group()) ? true : false;
      break;
    }
    default:
      res = false;
  }

  return res;
}
/*
  SYNOPSIS
    This function assigns a parent ID to the job group being scheduled in
  parallel. It also checks if we can schedule the new event in parallel with the
  previous ones being executed.

  @param        ev log event that has to be scheduled next.
  @param       rli Pointer to coordinator's relay log info.
  @return      true if error
               false otherwise
 */
static bool schedule_next_event(Log_event *ev, Relay_log_info *rli) {
  int error;
  // Check if we can schedule this event
  error = rli->current_mts_submode->schedule_next_event(rli, ev);
  switch (error) {
    char llbuff[22];
    case ER_MTS_CANT_PARALLEL:
      llstr(rli->get_event_relay_log_pos(), llbuff);
      my_error(ER_MTS_CANT_PARALLEL, MYF(0), ev->get_type_str(),
               rli->get_event_relay_log_name(), llbuff,
               "The master event is logically timestamped incorrectly.");
      return true;
    case ER_MTS_INCONSISTENT_DATA:
      llstr(rli->get_event_relay_log_pos(), llbuff);
      {
        char errfmt[] =
            "Coordinator experienced an error or was killed while scheduling "
            "an event at relay-log name %s position %s.";
        char errbuf[sizeof(errfmt) + FN_REFLEN + sizeof(llbuff)];
        sprintf(errbuf, errfmt, rli->get_event_relay_log_name(), llbuff);
        my_error(ER_MTS_INCONSISTENT_DATA, MYF(0), errbuf);
        return true;
      }
      /* Don't have to do anything. */
      return true;
    case -1:
      /* Unable to schedule: wait_for_last_committed_trx has failed */
      return true;
    default:
      return false;
  }
  /* Keep compiler happy */
  return false;
}

/**
   The method maps the event to a Worker and return a pointer to it.
   Sending the event to the Worker is done by the caller.

   Irrespective of the type of Group marking (DB partioned or BGC) the
   following holds true:

   - recognize the beginning of a group to allocate the group descriptor
     and queue it;
   - associate an event with a Worker (which also handles possible conflicts
     detection and waiting for their termination);
   - finalize the group assignement when the group closing event is met.

   When parallelization mode is BGC-based the partitioning info in the event
   is simply ignored. Thereby association with a Worker does not require
   Assigned Partition Hash of the partitioned method.
   This method is not interested in all the taxonomy of the event group
   property, what we care about is the boundaries of the group.

   As a part of the group, an event belongs to one of the following types:

   B - beginning of a group of events (BEGIN query_log_event)
   g - mini-group representative event containing the partition info
      (any Table_map, a Query_log_event)
   p - a mini-group internal event that *p*receeding its g-parent
      (int_, rand_, user_ var:s)
   r - a mini-group internal "regular" event that follows its g-parent
      (Delete, Update, Write -rows)
   T - terminator of the group (XID, COMMIT, ROLLBACK, auto-commit query)

   Only the first g-event computes the assigned Worker which once
   is determined remains to be for the rest of the group.
   That is the g-event solely carries partitioning info.
   For B-event the assigned Worker is NULL to indicate Coordinator
   has not yet decided. The same applies to p-event.

   Notice, these is a special group consisting of optionally multiple p-events
   terminating with a g-event.
   Such case is caused by old master binlog and a few corner-cases of
   the current master version (todo: to fix).

   In case of the event accesses more than OVER_MAX_DBS the method
   has to ensure sure previously assigned groups to all other workers are
   done.


   @note The function updates GAQ queue directly, updates APH hash
         plus relocates some temporary tables from Coordinator's list into
         involved entries of APH through @c map_db_to_worker.
         There's few memory allocations commented where to be freed.

   @return a pointer to the Worker struct or NULL.
*/

Slave_worker *Log_event::get_slave_worker(Relay_log_info *rli) {
  Slave_job_group group = Slave_job_group(), *ptr_group = nullptr;
  bool is_s_event;
  Slave_worker *ret_worker = nullptr;
  char llbuff[22];
  Slave_committed_queue *gaq = rli->gaq;
  DBUG_TRACE;

  /* checking partioning properties and perform corresponding actions */

  // Beginning of a group designated explicitly with BEGIN or GTID
  if ((is_s_event = starts_group()) || is_gtid_event(this) ||
      get_type_code() == binary_log::METADATA_EVENT ||
      // or DDL:s or autocommit queries possibly associated with own p-events
      (!rli->curr_group_seen_begin && !rli->curr_group_seen_gtid &&
       /*
         the following is a special case of B-free still multi-event group like
         { p_1,p_2,...,p_k, g }.
         In that case either GAQ is empty (the very first group is being
         assigned) or the last assigned group index points at one of
         mapped-to-a-worker.
       */
       (gaq->empty() ||
        gaq->get_job_group(rli->gaq->assigned_group_index)->worker_id !=
            MTS_WORKER_UNDEF))) {
    if (!rli->curr_group_seen_gtid && !rli->curr_group_seen_begin) {
      rli->mts_groups_assigned++;

      rli->curr_group_isolated = false;
      group.reset(common_header->log_pos, rli->mts_groups_assigned);
      // the last occupied GAQ's array index
      gaq->assigned_group_index = gaq->en_queue(&group);
      DBUG_PRINT("info", ("gaq_idx= %ld  gaq->size=%ld",
                          gaq->assigned_group_index, gaq->size));
      DBUG_ASSERT(gaq->assigned_group_index != MTS_WORKER_UNDEF);
      DBUG_ASSERT(gaq->assigned_group_index < gaq->size);
      DBUG_ASSERT(gaq->get_job_group(rli->gaq->assigned_group_index)
                      ->group_relay_log_name == nullptr);
      DBUG_ASSERT(rli->last_assigned_worker == nullptr ||
                  is_mts_parallel_type_logical_clock(rli));

      if (is_s_event || is_gtid_event(this)) {
        Slave_job_item job_item = {this, rli->get_event_relay_log_number(),
                                   rli->get_event_start_pos()};
        // B-event is appended to the Deferred Array associated with GCAP
        rli->curr_group_da.push_back(job_item);

        DBUG_ASSERT(rli->curr_group_da.size() == 1);

        if (starts_group()) {
          // mark the current group as started with explicit B-event
          rli->mts_end_group_sets_max_dbs = true;
          rli->curr_group_seen_begin = true;
        }

        if (is_gtid_event(this)) {
          // mark the current group as started with explicit Gtid-event
          rli->curr_group_seen_gtid = true;

          Gtid_log_event *gtid_log_ev = static_cast<Gtid_log_event *>(this);
          rli->started_processing(gtid_log_ev);
        }

        if (schedule_next_event(this, rli)) {
          rli->abort_slave = true;
          if (is_gtid_event(this)) {
            rli->clear_processing_trx();
          }
          return nullptr;
        }
        return ret_worker;
      }
    } else if (get_type_code() == binary_log::METADATA_EVENT) {
      // Metadata event allowed only when gtid is enabled
      DBUG_ASSERT(rli->curr_group_seen_gtid);

      rli->curr_group_seen_metadata = true;
      Slave_job_item job_item = {this, rli->get_event_relay_log_number(),
                                 rli->get_event_relay_log_pos()};

      rli->curr_group_da.push_back(job_item);

      // By the time we see Metadata event for the group, we should have already
      // seen gtid event. Hence number of elements in the array is 2
      DBUG_ASSERT(rli->curr_group_da.size() == 2);
      return ret_worker;
    } else {
      /*
       The block is a result of not making GTID event as group starter.
       TODO: Make GITD event as B-event that is starts_group() to
       return true.
      */
      Slave_job_item job_item = {this, rli->get_event_relay_log_number(),
                                 rli->get_event_relay_log_pos()};

      // B-event is appended to the Deferred Array associated with GCAP
      rli->curr_group_da.push_back(job_item);
      rli->curr_group_seen_begin = true;
      rli->mts_end_group_sets_max_dbs = true;
      if (!rli->curr_group_seen_gtid && schedule_next_event(this, rli)) {
        rli->abort_slave = true;
        return nullptr;
      }

      // If metadata event is seen, then there should be 3 elements in the array
      // (GTID event, metadata event and the query 'begin' event). Else, there
      // should only be two events in the array (GTID event and query 'begin'
      // event)
      DBUG_ASSERT(
          (!rli->curr_group_seen_metadata && rli->curr_group_da.size() == 2) ||
          (rli->curr_group_seen_metadata && rli->curr_group_da.size() == 3));

      DBUG_ASSERT(starts_group());
      return ret_worker;
    }
    if (schedule_next_event(this, rli)) {
      rli->abort_slave = true;
      return nullptr;
    }
  }

  ptr_group = gaq->get_job_group(rli->gaq->assigned_group_index);
  if (is_mts_parallel_type_logical_clock(rli)) {
    /* Get least occupied worker */
    ret_worker = rli->current_mts_submode->get_least_occupied_worker(
        rli, &rli->workers, this);
    if (ret_worker == nullptr) {
      /* get_least_occupied_worker may return NULL if the thread is killed */
      Slave_job_item job_item = {this, rli->get_event_relay_log_number(),
                                 rli->get_event_start_pos()};
      rli->curr_group_da.push_back(job_item);

      DBUG_ASSERT(thd->killed);
      return nullptr;
    }
    ptr_group->worker_id = ret_worker->id;
  } else if (contains_partition_info(rli->mts_end_group_sets_max_dbs)) {
    int i = 0;
    Mts_db_names mts_dbs;

    get_mts_dbs(&mts_dbs, rli->rpl_filter);
    /*
      Bug 12982188 - MTS: SBR ABORTS WITH ERROR 1742 ON LOAD DATA
      Logging on master can create a group with no events holding
      the partition info.
      The following assert proves there's the only reason
      for such group.
    */
#ifndef DBUG_OFF
    {
      bool empty_group_with_gtids = rli->curr_group_seen_begin &&
                                    rli->curr_group_seen_gtid && ends_group();

      bool begin_load_query_event =
          ((rli->curr_group_da.size() == 3 && rli->curr_group_seen_gtid) ||
           (rli->curr_group_da.size() == 2 && !rli->curr_group_seen_gtid)) &&
          (rli->curr_group_da.back().data->get_type_code() ==
           binary_log::BEGIN_LOAD_QUERY_EVENT);

      bool delete_file_event =
          ((rli->curr_group_da.size() == 4 && rli->curr_group_seen_gtid) ||
           (rli->curr_group_da.size() == 3 && !rli->curr_group_seen_gtid)) &&
          (rli->curr_group_da.back().data->get_type_code() ==
           binary_log::DELETE_FILE_EVENT);

      DBUG_ASSERT((!ends_group() ||
                   (get_type_code() == binary_log::TRANSACTION_PAYLOAD_EVENT) ||
                   (get_type_code() == binary_log::QUERY_EVENT &&
                    static_cast<Query_log_event *>(this)->is_query_prefix_match(
                        STRING_WITH_LEN("XA ROLLBACK")))) ||
                  empty_group_with_gtids ||
                  (rli->mts_end_group_sets_max_dbs &&
                   (begin_load_query_event || delete_file_event)));
    }
#endif

    // partioning info is found which drops the flag
    rli->mts_end_group_sets_max_dbs = false;
    ret_worker = rli->last_assigned_worker;
    if (mts_dbs.num == OVER_MAX_DBS_IN_EVENT_MTS) {
      // Worker with id 0 to handle serial execution
      if (!ret_worker) ret_worker = rli->workers.at(0);
      // No need to know a possible error out of synchronization call.
      (void)rli->current_mts_submode->wait_for_workers_to_finish(rli,
                                                                 ret_worker);
      /*
        this marking is transferred further into T-event of the current group.
      */
      rli->curr_group_isolated = true;
    }
#ifndef DBUG_OFF
    {
      std::ostringstream oss;
      for (i = 0;
           i < ((mts_dbs.num != OVER_MAX_DBS_IN_EVENT_MTS) ? mts_dbs.num : 1);
           i++) {
        if (mts_dbs.name[i] != 0) {
          oss << mts_dbs.name[i] << ", ";
        }
      }
      DBUG_PRINT("debug", ("ASSIGN %p %s", current_thd, oss.str().c_str()));
    }
#endif

    /* One run of the loop in the case of over-max-db:s */
    for (i = 0;
         i < ((mts_dbs.num != OVER_MAX_DBS_IN_EVENT_MTS) ? mts_dbs.num : 1);
         i++) {
      /*
        The over max db:s case handled through passing to map_db_to_worker
        such "all" db as encoded as  the "" empty string.
        Note, the empty string is allocated in a large buffer
        to satisfy hashcmp() implementation.
      */
      const char all_db[NAME_LEN] = {0};
      if (!(ret_worker = map_db_to_worker(
                mts_dbs.num == OVER_MAX_DBS_IN_EVENT_MTS ? all_db
                                                         : mts_dbs.name[i],
                rli, &mts_assigned_partitions[i],
                /*
                  todo: optimize it. Although pure
                  rows- event load in insensetive to the flag value
                */
                true, ret_worker))) {
        llstr(rli->get_event_relay_log_pos(), llbuff);
        my_error(ER_MTS_CANT_PARALLEL, MYF(0), get_type_str(),
                 rli->get_event_relay_log_name(), llbuff,
                 "could not distribute the event to a Worker");
        return ret_worker;
      }
      // all temporary tables are transferred from Coordinator in over-max case
      DBUG_ASSERT(mts_dbs.num != OVER_MAX_DBS_IN_EVENT_MTS ||
                  !thd->temporary_tables);
      DBUG_ASSERT(!strcmp(
          mts_assigned_partitions[i]->db,
          mts_dbs.num != OVER_MAX_DBS_IN_EVENT_MTS ? mts_dbs.name[i] : all_db));
      DBUG_ASSERT(ret_worker == mts_assigned_partitions[i]->worker);
      DBUG_ASSERT(mts_assigned_partitions[i]->usage >= 0);
    }

    if ((ptr_group = gaq->get_job_group(rli->gaq->assigned_group_index))
            ->worker_id == MTS_WORKER_UNDEF) {
      ptr_group->worker_id = ret_worker->id;

      DBUG_ASSERT(ptr_group->group_relay_log_name == nullptr);
    }

    DBUG_ASSERT(i == mts_dbs.num || mts_dbs.num == OVER_MAX_DBS_IN_EVENT_MTS);
  } else {
    // a mini-group internal "regular" event
    if (rli->last_assigned_worker) {
      ret_worker = rli->last_assigned_worker;

      DBUG_ASSERT(rli->curr_group_assigned_parts.size() > 0 ||
                  ret_worker->id == 0);
    } else  // int_, rand_, user_ var:s, load-data events
    {
      if (!(get_type_code() == binary_log::INTVAR_EVENT ||
            get_type_code() == binary_log::RAND_EVENT ||
            get_type_code() == binary_log::USER_VAR_EVENT ||
            get_type_code() == binary_log::BEGIN_LOAD_QUERY_EVENT ||
            get_type_code() == binary_log::APPEND_BLOCK_EVENT ||
            get_type_code() == binary_log::DELETE_FILE_EVENT ||
            is_ignorable_event())) {
        DBUG_ASSERT(!ret_worker);

        llstr(rli->get_event_relay_log_pos(), llbuff);
        my_error(ER_MTS_CANT_PARALLEL, MYF(0), get_type_str(),
                 rli->get_event_relay_log_name(), llbuff,
                 "the event is a part of a group that is unsupported in "
                 "the parallel execution mode");

        return ret_worker;
      }
      /*
        In the logical clock scheduler any internal gets scheduled directly.
        That is Int_var, @User_var and Rand bypass the deferred array.
        Their association with relay-log physical coordinates is provided
        by the same mechanism that applies to a regular event.
      */
      Slave_job_item job_item = {this, rli->get_event_relay_log_number(),
                                 rli->get_event_start_pos()};
      rli->curr_group_da.push_back(job_item);

      DBUG_ASSERT(!ret_worker);
      return ret_worker;
    }
  }

  DBUG_ASSERT(ret_worker);
  // T-event: Commit, Xid, a DDL query or dml query of B-less group.4

  /*
    Preparing event physical coordinates info for Worker before any
    event got scheduled so when Worker error-stopped at the first
    event it would be aware of where exactly in the event stream.
  */
  if (!ret_worker->master_log_change_notified) {
    if (!ptr_group)
      ptr_group = gaq->get_job_group(rli->gaq->assigned_group_index);
    ptr_group->group_master_log_name = my_strdup(
        key_memory_log_event, rli->get_group_master_log_name(), MYF(MY_WME));
    ret_worker->master_log_change_notified = true;

    DBUG_ASSERT(!ptr_group->notified);
#ifndef DBUG_OFF
    ptr_group->notified = true;
#endif
  }

  /* Notify the worker about new FD */
  if (!ret_worker->fd_change_notified) {
    if (!ptr_group)
      ptr_group = gaq->get_job_group(rli->gaq->assigned_group_index);
    /*
      Increment the usage counter on behalf of Worker.
      This avoids inadvertent FD deletion in a race case where Coordinator
      would install a next new FD before Worker has noticed the previous one.
    */
    ++rli->get_rli_description_event()->atomic_usage_counter;
    ptr_group->new_fd_event = rli->get_rli_description_event();
    ret_worker->fd_change_notified = true;
  }

  if (ends_group() ||
      (!rli->curr_group_seen_begin &&
       (get_type_code() == binary_log::QUERY_EVENT ||
        /*
          When applying an old binary log without Gtid_log_event and
          Anonymous_gtid_log_event, the logic of multi-threaded slave
          still need to require that an event (for example, Query_log_event,
          User_var_log_event, Intvar_log_event, and Rand_log_event) that
          appeared outside of BEGIN...COMMIT was treated as a transaction
          of its own. This was just a technicality in the code and did not
          cause a problem, since the event and the following Query_log_event
          would both be assigned to dedicated worker 0.
        */
        !rli->curr_group_seen_gtid))) {
    rli->mts_group_status = Relay_log_info::MTS_END_GROUP;
    if (rli->curr_group_isolated) set_mts_isolate_group();
    if (!ptr_group)
      ptr_group = gaq->get_job_group(rli->gaq->assigned_group_index);

    DBUG_ASSERT(ret_worker != nullptr);

    // coordinator has ended buffering this group, update monitoring info
    if (rli->is_processing_trx()) {
      DBUG_EXECUTE_IF("rpl_ps_tables", {
        const char act[] =
            "now SIGNAL signal.rpl_ps_tables_process_before "
            "WAIT_FOR signal.rpl_ps_tables_process_finish";
        DBUG_ASSERT(opt_debug_sync_timeout > 0);
        DBUG_ASSERT(!debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
      };);
      rli->finished_processing();
      DBUG_EXECUTE_IF("rpl_ps_tables", {
        const char act[] =
            "now SIGNAL signal.rpl_ps_tables_process_after_finish "
            "WAIT_FOR signal.rpl_ps_tables_process_continue";
        DBUG_ASSERT(opt_debug_sync_timeout > 0);
        DBUG_ASSERT(!debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
      };);
    }

    /*
      The following two blocks are executed if the worker has not been
      notified about new relay-log or a new checkpoints.
      Relay-log string is freed by Coordinator, Worker deallocates
      strings in the checkpoint block.
      However if the worker exits earlier reclaiming for both happens anyway at
      GAQ delete.
    */
    if (!ret_worker->relay_log_change_notified) {
      /*
        Prior this event, C rotated the relay log to drop each
        Worker's notified flag. Now group terminating event initiates
        the new relay-log (where the current event is from) name
        delivery to Worker that will receive it in commit_positions().
      */
      DBUG_ASSERT(ptr_group->group_relay_log_name == nullptr);

      ptr_group->group_relay_log_name = (char *)my_malloc(
          key_memory_log_event, strlen(rli->get_event_relay_log_name()) + 1,
          MYF(MY_WME));
      strcpy(ptr_group->group_relay_log_name, rli->get_event_relay_log_name());

      DBUG_ASSERT(ptr_group->group_relay_log_name != nullptr);

      ret_worker->relay_log_change_notified = true;
    }

    if (!ret_worker->checkpoint_notified) {
      if (!ptr_group)
        ptr_group = gaq->get_job_group(rli->gaq->assigned_group_index);
      ptr_group->checkpoint_log_name = my_strdup(
          key_memory_log_event, rli->get_group_master_log_name(), MYF(MY_WME));
      ptr_group->checkpoint_log_pos = rli->get_group_master_log_pos();
      ptr_group->checkpoint_relay_log_name = my_strdup(
          key_memory_log_event, rli->get_group_relay_log_name(), MYF(MY_WME));
      ptr_group->checkpoint_relay_log_pos = rli->get_group_relay_log_pos();
      ptr_group->shifted = ret_worker->bitmap_shifted;
      ret_worker->bitmap_shifted = 0;
      ret_worker->checkpoint_notified = true;
    }
    ptr_group->checkpoint_seqno = rli->rli_checkpoint_seqno;
    ptr_group->ts = common_header->when.tv_sec +
                    (time_t)exec_time;  // Seconds_behind_master related
    ptr_group->ts_millis =
        (rli->group_timestamp_millis ? rli->group_timestamp_millis
                                     : common_header->when.tv_sec * 1000) +
        exec_time * 1000;
    // reset for next group
    rli->group_timestamp_millis = 0;
    rli->rli_checkpoint_seqno++;
    /*
      Coordinator should not use the main memroot however its not
      reset elsewhere either, so let's do it safe way.
      The main mem root is also reset by the SQL thread in at the end
      of applying which Coordinator does not do in this case.
      That concludes the memroot reset can't harm anything in SQL thread roles
      after Coordinator has finished its current scheduling.
    */
    free_root(thd->mem_root, MYF(MY_KEEP_PREALLOC));

#ifndef DBUG_OFF
    w_rr++;
#endif
  }

  return ret_worker;
}

int Log_event::apply_gtid_event(Relay_log_info *rli) {
  DBUG_TRACE;

  int error = 0;
  if (rli->curr_group_da.size() < 1) return 1;

  Log_event *ev = rli->curr_group_da[0].data;
  DBUG_ASSERT(ev->get_type_code() == binary_log::GTID_LOG_EVENT ||
              ev->get_type_code() == binary_log::ANONYMOUS_GTID_LOG_EVENT);

  error = ev->do_apply_event(rli);
  /* Clean up */
  delete ev;
  rli->curr_group_da.clear();
  rli->curr_group_seen_gtid = false;
  /*
    Removes the job from the (G)lobal (A)ssigned (Q)ueue after
    applying it.
  */
  DBUG_ASSERT(rli->gaq->len > 0);
  Slave_job_group g = Slave_job_group();
  rli->gaq->de_tail(&g);
  /*
    The rli->mts_groups_assigned is increased when adding the slave job
    generated for the gtid into the (G)lobal (A)ssigned (Q)ueue. So we
    decrease it here.
  */
  rli->mts_groups_assigned--;

  return error;
}

bool Log_event::is_row_log_event() const noexcept {
  switch (get_type_code()) {
    case Log_event_type::WRITE_ROWS_EVENT:
    case Log_event_type::UPDATE_ROWS_EVENT:
    case Log_event_type::DELETE_ROWS_EVENT:
      return true;
    default:
      return false;
  }
  return false;
}

/**
 * Checks if the event needs to run in idempotent recovery mode and sets
 * things up if it's required
 */
void Log_event::check_and_set_idempotent_recovery(Relay_log_info *rli,
                                                  const char *gtid) {
  if (!is_row_log_event()) {
    return;
  }

  DBUG_EXECUTE_IF("dbg_enable_idempotent_recovery", {
    Gtid current_gtid;
    current_gtid.clear();
    rli->recovery_sid_lock.rdlock();
    DBUG_ASSERT(current_gtid.parse(&rli->recovery_sid_map, gtid) ==
                RETURN_STATUS_OK);
    rli->recovery_sid_lock.unlock();
    rli->recovery_max_engine_gtid = current_gtid;
  });

  // Check if idempotent mode is required (used after crash recovery)
  if (gtid[0] != '\0' && thd->is_enabled_idempotent_recovery() &&
      !rli->recovery_max_engine_gtid.is_empty()) {
    Gtid current_gtid;
    current_gtid.clear();
    rli->recovery_sid_lock.rdlock();
    current_gtid.parse(&rli->recovery_sid_map, gtid);
    rli->recovery_sid_lock.unlock();

    if (current_gtid.sidno == rli->recovery_max_engine_gtid.sidno &&
        current_gtid.gno <= rli->recovery_max_engine_gtid.gno) {
      sql_print_information("Enabling idempotent mode for %s", gtid);
      rbr_exec_mode = RBR_EXEC_MODE_IDEMPOTENT;
      auto rev = static_cast<Rows_log_event *>(this);
      rev->m_force_binlog_idempotent = true;
      thd->m_skip_row_logging_functions = true;
    }
  }
}

/**
   Scheduling event to execute in parallel or execute it directly.
   In MTS case the event gets associated with either Coordinator or a
   Worker.  A special case of the association is NULL when the Worker
   can't be decided yet.  In the single threaded sequential mode the
   event maps to SQL thread rli.

   @note in case of MTS failure Coordinator destroys all gathered
         deferred events.

   @return 0 as success, otherwise a failure.
*/
int Log_event::apply_event(Relay_log_info *rli) {
  DBUG_TRACE;
  DBUG_PRINT("info", ("event_type=%s", get_type_str()));
  bool parallel = false;
  enum enum_mts_event_exec_mode actual_exec_mode = EVENT_EXEC_PARALLEL;
  THD *rli_thd = rli->info_thd;

  worker = rli;
  m_mts_dep_allowed = false;

  if (rli->is_mts_recovery()) {
    bool skip = bitmap_is_set(&rli->recovery_groups, rli->mts_recovery_index) &&
                (get_mts_execution_mode(rli->mts_group_status ==
                                        Relay_log_info::MTS_IN_GROUP) ==
                 EVENT_EXEC_PARALLEL);
    if (skip) {
      return 0;
    } else {
      int error = do_apply_event(rli);
      if (rli->is_processing_trx()) {
        // needed to identify DDL's; uses the same logic as in
        // get_slave_worker()
        if (starts_group() && get_type_code() == binary_log::QUERY_EVENT) {
          rli->curr_group_seen_begin = true;
        }
        if (error == 0 &&
            (ends_group() || (get_type_code() == binary_log::QUERY_EVENT &&
                              !rli->curr_group_seen_begin))) {
          rli->finished_processing();
          rli->curr_group_seen_begin = false;
        }
      }
      return error;
    }
  }

  if (!(parallel = rli->is_parallel_exec()) ||
      ((actual_exec_mode = get_mts_execution_mode(
            rli->mts_group_status == Relay_log_info::MTS_IN_GROUP)) !=
       EVENT_EXEC_PARALLEL)) {
    if (parallel) {
      /*
         There are two classes of events that Coordinator executes
         itself. One e.g the master Rotate requires all Workers to finish up
         their assignments. The other async class, e.g the slave Rotate,
         can't have this such synchronization because Worker might be waiting
         for terminal events to finish.
      */

      if (actual_exec_mode != EVENT_EXEC_ASYNC) {
        /*
          this  event does not split the current group but is indeed
          a separator beetwen two master's binlog therefore requiring
          Workers to sync.
        */
        if (rli->curr_group_da.size() > 0 && is_mts_db_partitioned(rli) &&
            get_type_code() != binary_log::INCIDENT_EVENT) {
          char llbuff[22];
          /*
             Possible reason is a old version binlog sequential event
             wrappped with BEGIN/COMMIT or preceeded by User|Int|Random- var.
             MTS has to stop to suggest restart in the permanent sequential
             mode.
          */
          llstr(rli->get_event_relay_log_pos(), llbuff);
          my_error(ER_MTS_CANT_PARALLEL, MYF(0), get_type_str(),
                   rli->get_event_relay_log_name(), llbuff,
                   "possible malformed group of events from an old master");

          /* Coordinator cant continue, it marks MTS group status accordingly */
          rli->mts_group_status = Relay_log_info::MTS_KILLED_GROUP;

          goto err;
        }

        if (get_type_code() == binary_log::INCIDENT_EVENT &&
            rli->curr_group_da.size() > 0 &&
            rli->current_mts_submode->get_type() ==
                MTS_PARALLEL_TYPE_LOGICAL_CLOCK) {
#ifndef DBUG_OFF
          DBUG_ASSERT(rli->curr_group_da.size() == 1);
          Log_event *ev = rli->curr_group_da[0].data;
          DBUG_ASSERT(ev->get_type_code() == binary_log::GTID_LOG_EVENT ||
                      ev->get_type_code() ==
                          binary_log::ANONYMOUS_GTID_LOG_EVENT);
#endif
          /*
            With MTS logical clock mode, when coordinator is applying an
            incident event, it must withdraw delegated_job increased by
            the incident's GTID before waiting for workers to finish.
            So that it can exit from mts_checkpoint_routine.
          */
          ((Mts_submode_logical_clock *)rli->current_mts_submode)
              ->withdraw_delegated_job();
        }
        /*
          Marking sure the event will be executed in sequential mode.
        */
        if (rli->current_mts_submode->wait_for_workers_to_finish(rli) == -1) {
          // handle synchronization error
          rli->report(WARNING_LEVEL, 0,
                      "Slave worker thread has failed to apply an event. As a "
                      "consequence, the coordinator thread is stopping "
                      "execution.");
          return -1;
        }
        /*
          Given not in-group mark the event handler can invoke checkpoint
          update routine in the following course.
        */
        DBUG_ASSERT(rli->mts_group_status == Relay_log_info::MTS_NOT_IN_GROUP ||
                    !is_mts_db_partitioned(rli));

        if (get_type_code() == binary_log::INCIDENT_EVENT &&
            rli->curr_group_da.size() > 0) {
          DBUG_ASSERT(rli->curr_group_da.size() == 1);
          /*
            When MTS is enabled, the incident event must be applied by the
            coordinator. So the coordinator applies its GTID right before
            applying the incident event..
          */
          int error = apply_gtid_event(rli);
          if (error) return -1;
        }

#ifndef DBUG_OFF
        /* all Workers are idle as done through wait_for_workers_to_finish */
        for (uint k = 0; k < rli->curr_group_da.size(); k++) {
          DBUG_ASSERT(!(rli->workers[k]->usage_partition));
          DBUG_ASSERT(!(rli->workers[k]->jobs.len));
        }
#endif
      } else {
        DBUG_ASSERT(actual_exec_mode == EVENT_EXEC_ASYNC);
      }
    }

    check_and_set_idempotent_recovery(rli, rli->last_gtid);

    int error = do_apply_event(rli);

    if (is_gtid_event(this)) {
      auto gtid_ev = static_cast<Gtid_log_event *>(this);
      gtid_ev->extract_last_gtid_to(rli->last_gtid);
    }

    if (rli->is_processing_trx()) {
      // needed to identify DDL's; uses the same logic as in get_slave_worker()
      if (starts_group() && get_type_code() == binary_log::QUERY_EVENT) {
        rli->curr_group_seen_begin = true;
      }
      if (error == 0 &&
          (ends_group() || (get_type_code() == binary_log::QUERY_EVENT &&
                            !rli->curr_group_seen_begin))) {
        DBUG_EXECUTE_IF("rpl_ps_tables", {
          const char act[] =
              "now SIGNAL signal.rpl_ps_tables_apply_before "
              "WAIT_FOR signal.rpl_ps_tables_apply_finish";
          DBUG_ASSERT(opt_debug_sync_timeout > 0);
          DBUG_ASSERT(
              !debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
        };);
        rli->finished_processing();
        rli->curr_group_seen_begin = false;
        DBUG_EXECUTE_IF("rpl_ps_tables", {
          const char act[] =
              "now SIGNAL signal.rpl_ps_tables_apply_after_finish "
              "WAIT_FOR signal.rpl_ps_tables_apply_continue";
          DBUG_ASSERT(opt_debug_sync_timeout > 0);
          DBUG_ASSERT(
              !debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
        };);
      }
    }
    return error;
  }

  DBUG_ASSERT(actual_exec_mode == EVENT_EXEC_PARALLEL);
  DBUG_ASSERT(!(rli->curr_group_seen_begin && ends_group()) ||
              /*
                This is an empty group being processed due to gtids.
              */
              (rli->curr_group_seen_begin && rli->curr_group_seen_gtid &&
               ends_group()) ||
              is_mts_parallel_type_dependency(rli) ||
              is_mts_db_partitioned(rli) || rli->last_assigned_worker ||
              /*
                Begin_load_query can be logged w/o db info and within
                Begin/Commit. That's a pattern forcing sequential
                applying of LOAD-DATA.
              */
              (rli->curr_group_da.back().data->get_type_code() ==
               binary_log::BEGIN_LOAD_QUERY_EVENT) ||
              /*
                Delete_file can also be logged w/o db info and within
                Begin/Commit. That's a pattern forcing sequential
                applying of LOAD-DATA.
              */
              (rli->curr_group_da.back().data->get_type_code() ==
               binary_log::DELETE_FILE_EVENT));

  worker = nullptr;
  m_mts_dep_allowed = true;
  rli->mts_group_status = Relay_log_info::MTS_IN_GROUP;

  // milli-sec behind master related for MTS
  // case: this event contains trx metadata, so save the ts in msec for the grp
  if (opt_binlog_trx_meta_data &&
      get_type_code() == binary_log::ROWS_QUERY_LOG_EVENT &&
      has_trx_meta_data()) {
    rli->group_timestamp_millis = extract_last_timestamp();
  }

  if (!is_mts_parallel_type_dependency(rli))
    worker =
        (Relay_log_info *)(rli->last_assigned_worker = get_slave_worker(rli));

#ifndef DBUG_OFF
  if (rli->last_assigned_worker)
    DBUG_PRINT("mts",
               ("Assigning job to worker %lu", rli->last_assigned_worker->id));
#endif

err:
  if (rli_thd->is_error() || (!worker && rli->abort_slave)) {
    DBUG_ASSERT(!worker);

    /*
      Destroy all deferred buffered events but the current prior to exit.
      The current one will be deleted as an event never destined/assigned
      to any Worker in Coordinator's regular execution path.
    */
    for (uint k = 0; k < rli->curr_group_da.size(); k++) {
      Log_event *ev_buf = rli->curr_group_da[k].data;
      if (this != ev_buf) delete ev_buf;
    }
    rli->curr_group_da.clear();
  } else {
    DBUG_ASSERT(worker || rli->curr_group_assigned_parts.size() == 0);
  }

  return (!(rli_thd->is_error() || (!worker && rli->abort_slave)) ||
          DBUG_EVALUATE_IF("fault_injection_get_slave_worker", 1, 0))
             ? 0
             : -1;
}

/**************************************************************************
        Query_log_event methods
**************************************************************************/

/**
  This (which is used only for SHOW BINLOG EVENTS) could be updated to
  print SET @@session_var=. But this is not urgent, as SHOW BINLOG EVENTS is
  only an information, it does not produce suitable queries to replay (for
  example it does not print LOAD DATA INFILE).
  @todo
    show the catalog ??
*/

int Query_log_event::pack_info(Protocol *protocol) {
  // TODO: show the catalog ??
  String str_buf;
  // Add use `DB` to the string if required
  if (!(common_header->flags & LOG_EVENT_SUPPRESS_USE_F) && db && db_len) {
    str_buf.append("use ");
    append_identifier(this->thd, &str_buf, db, db_len);
    str_buf.append("; ");
  }
  // Add the query to the string
  if (query && q_len) {
    str_buf.append(query);
    if (ddl_xid != binary_log::INVALID_XID) {
      char xid_buf[64];
      str_buf.append(" /* xid=");
      longlong10_to_str(ddl_xid, xid_buf, 10);
      str_buf.append(xid_buf);
      str_buf.append(" */");
    }
  }
  // persist the buffer in protocol
  protocol->store_string(str_buf.ptr(), str_buf.length(), &my_charset_bin);
  return 0;
}

/**
  Utility function for the next method (Query_log_event::write()) .
*/
static void write_str_with_code_and_len(uchar **dst, const char *src,
                                        size_t len, uint code) {
  /*
    only 1 byte to store the length of catalog, so it should not
    surpass 255
  */
  DBUG_ASSERT(len <= 255);
  DBUG_ASSERT(src);
  *((*dst)++) = code;
  *((*dst)++) = (uchar)len;
  memmove(*dst, src, len);
  (*dst) += len;
}

/**
  Query_log_event::write().

  @note
    In this event we have to modify the header to have the correct
    EVENT_LEN_OFFSET as we don't yet know how many status variables we
    will print!
*/

bool Query_log_event::write(Basic_ostream *ostream) {
  uchar buf[Binary_log_event::QUERY_HEADER_LEN + MAX_SIZE_LOG_EVENT_STATUS];
  uchar *start, *start_of_status;
  size_t event_length;

  if (!query) return true;  // Something wrong with event

  /*
    We want to store the thread id:
    (- as an information for the user when he reads the binlog)
    - if the query uses temporary table: for the slave SQL thread to know to
    which master connection the temp table belongs.
    Now imagine we (write()) are called by the slave SQL thread (we are
    logging a query executed by this thread; the slave runs with
    --log-slave-updates). Then this query will be logged with
    thread_id=the_thread_id_of_the_SQL_thread. Imagine that 2 temp tables of
    the same name were created simultaneously on the master (in the masters
    binlog you have
    CREATE TEMPORARY TABLE t; (thread 1)
    CREATE TEMPORARY TABLE t; (thread 2)
    ...)
    then in the slave's binlog there will be
    CREATE TEMPORARY TABLE t; (thread_id_of_the_slave_SQL_thread)
    CREATE TEMPORARY TABLE t; (thread_id_of_the_slave_SQL_thread)
    which is bad (same thread id!).

    To avoid this, we log the thread's thread id EXCEPT for the SQL
    slave thread for which we log the original (master's) thread id.
    Now this moves the bug: what happens if the thread id on the
    master was 10 and when the slave replicates the query, a
    connection number 10 is opened by a normal client on the slave,
    and updates a temp table of the same name? We get a problem
    again. To avoid this, in the handling of temp tables (sql_base.cc)
    we use thread_id AND server_id.  TODO when this is merged into
    4.1: in 4.1, slave_proxy_id has been renamed to pseudo_thread_id
    and is a session variable: that's to make mysqlbinlog work with
    temp tables. We probably need to introduce

    SET PSEUDO_SERVER_ID
    for mysqlbinlog in 4.1. mysqlbinlog would print:
    SET PSEUDO_SERVER_ID=
    SET PSEUDO_THREAD_ID=
    for each query using temp tables.
  */
  int4store(buf + Q_THREAD_ID_OFFSET, slave_proxy_id);
  int4store(buf + Q_EXEC_TIME_OFFSET, exec_time);
  buf[Q_DB_LEN_OFFSET] = (char)db_len;
  int2store(buf + Q_ERR_CODE_OFFSET, error_code);

  /*
    You MUST always write status vars in increasing order of code. This
    guarantees that a slightly older slave will be able to parse those he
    knows.
  */
  start_of_status = start = buf + Binary_log_event::QUERY_HEADER_LEN;
  if (flags2_inited) {
    *start++ = Q_FLAGS2_CODE;
    int4store(start, flags2);
    start += 4;
  }
  if (sql_mode_inited) {
    *start++ = Q_SQL_MODE_CODE;
    int8store(start, sql_mode);
    start += 8;
  }
  if (catalog_len)  // i.e. this var is inited (false for 4.0 events)
  {
    write_str_with_code_and_len(&start, catalog, catalog_len,
                                Q_CATALOG_NZ_CODE);
    /*
      In 5.0.x where x<4 masters we used to store the end zero here. This was
      a waste of one byte so we don't do it in x>=4 masters. We change code to
      Q_CATALOG_NZ_CODE, because re-using the old code would make x<4 slaves
      of this x>=4 master segfault (expecting a zero when there is
      none). Remaining compatibility problems are: the older slave will not
      find the catalog; but it is will not crash, and it's not an issue
      that it does not find the catalog as catalogs were not used in these
      older MySQL versions (we store it in binlog and read it from relay log
      but do nothing useful with it). What is an issue is that the older slave
      will stop processing the Q_* blocks (and jumps to the db/query) as soon
      as it sees unknown Q_CATALOG_NZ_CODE; so it will not be able to read
      Q_AUTO_INCREMENT*, Q_CHARSET and so replication will fail silently in
      various ways. Documented that you should not mix alpha/beta versions if
      they are not exactly the same version, with example of 5.0.3->5.0.2 and
      5.0.4->5.0.3. If replication is from older to new, the new will
      recognize Q_CATALOG_CODE and have no problem.
    */
  }
  if (auto_increment_increment != 1 || auto_increment_offset != 1) {
    *start++ = Q_AUTO_INCREMENT;
    int2store(start, static_cast<uint16>(auto_increment_increment));
    int2store(start + 2, static_cast<uint16>(auto_increment_offset));
    start += 4;
  }
  if (charset_inited) {
    *start++ = Q_CHARSET_CODE;
    memcpy(start, charset, 6);
    start += 6;
  }
  if (time_zone_len) {
    /* In the TZ sys table, column Name is of length 64 so this should be ok */
    DBUG_ASSERT(time_zone_len <= MAX_TIME_ZONE_NAME_LENGTH);
    write_str_with_code_and_len(&start, time_zone_str, time_zone_len,
                                Q_TIME_ZONE_CODE);
  }
  if (lc_time_names_number) {
    DBUG_ASSERT(lc_time_names_number <= 0xFF);
    *start++ = Q_LC_TIME_NAMES_CODE;
    int2store(start, lc_time_names_number);
    start += 2;
  }
  if (charset_database_number) {
    *start++ = Q_CHARSET_DATABASE_CODE;
    int2store(start, charset_database_number);
    start += 2;
  }
  if (table_map_for_update) {
    *start++ = Q_TABLE_MAP_FOR_UPDATE_CODE;
    int8store(start, table_map_for_update);
    start += 8;
  }

  if (thd && thd->need_binlog_invoker()) {
    LEX_CSTRING invoker_user{nullptr, 0};
    LEX_CSTRING invoker_host{nullptr, 0};
    memset(&invoker_user, 0, sizeof(invoker_user));
    memset(&invoker_host, 0, sizeof(invoker_host));

    if (thd->slave_thread && thd->has_invoker()) {
      /* user will be null, if master is older than this patch */
      invoker_user = thd->get_invoker_user();
      invoker_host = thd->get_invoker_host();
    } else {
      Security_context *ctx = thd->security_context();
      LEX_CSTRING priv_user = ctx->priv_user();
      LEX_CSTRING priv_host = ctx->priv_host();

      invoker_user.length = priv_user.length;
      invoker_user.str = priv_user.str;
      if (priv_host.str[0] != '\0') {
        invoker_host.str = priv_host.str;
        invoker_host.length = priv_host.length;
      }
    }

    *start++ = Q_INVOKER;

    DBUG_EXECUTE_IF("wl12571_long_invoker_host", {
      invoker_host.str =
          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
          "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
      invoker_host.length = strlen(invoker_host.str);
    });

    /*
      Store user length and user. The max length of use is 80, so 1 byte is
      enough to store the user's length.
     */
    assert(invoker_user.length < UINT_MAX8);
    *start++ = (uchar)invoker_user.length;
    memcpy(start, invoker_user.str, invoker_user.length);
    start += invoker_user.length;

    /*
      Store host length and host. The max length of host is 255, so 1 byte
      is enough to store the host's length.
     */
    *start++ = (uchar)invoker_host.length;
    if (invoker_host.length > 0)
      memcpy(start, invoker_host.str, invoker_host.length);
    start += invoker_host.length;
  }

  if (thd && thd->get_binlog_accessed_db_names() != nullptr) {
    uchar dbs;
    *start++ = Q_UPDATED_DB_NAMES;

    static_assert(MAX_DBS_IN_EVENT_MTS <= OVER_MAX_DBS_IN_EVENT_MTS, "");

    /*
       In case of the number of db:s exceeds MAX_DBS_IN_EVENT_MTS
       no db:s is written and event will require the sequential applying on
       slave.
    */
    dbs =
        (thd->get_binlog_accessed_db_names()->elements <= MAX_DBS_IN_EVENT_MTS)
            ? thd->get_binlog_accessed_db_names()->elements
            : OVER_MAX_DBS_IN_EVENT_MTS;

    DBUG_ASSERT(dbs != 0);

    if (dbs <= MAX_DBS_IN_EVENT_MTS) {
      List_iterator_fast<char> it(*thd->get_binlog_accessed_db_names());
      char *db_name = it++;
      /*
         the single "" db in the acccessed db list corresponds to the same as
         exceeds MAX_DBS_IN_EVENT_MTS case, so dbs is set to the over-max.
      */
      if (dbs == 1 && !strcmp(db_name, "")) dbs = OVER_MAX_DBS_IN_EVENT_MTS;
      *start++ = dbs;
      if (dbs != OVER_MAX_DBS_IN_EVENT_MTS) do {
          size_t db_name_len = strlen(db_name);
          strcpy((char *)start, db_name);
          start += db_name_len + 1;
          if (enable_binlog_hlc && maintain_database_hlc)
            thd->databases.emplace(db_name, db_name_len);
        } while ((db_name = it++));
    } else {
      *start++ = dbs;
    }
  }

  if (thd && thd->query_start_usec_used) {
    *start++ = Q_MICROSECONDS;
    get_time();
    int3store(start, common_header->when.tv_usec);
    start += 3;
  }

  if (thd && thd->binlog_need_explicit_defaults_ts == true) {
    *start++ = Q_EXPLICIT_DEFAULTS_FOR_TIMESTAMP;
    *start++ = thd->variables.explicit_defaults_for_timestamp;
  }

  if (ddl_xid != binary_log::INVALID_XID) {
    *start++ = Q_DDL_LOGGED_WITH_XID;
    int8store(start, ddl_xid);
    start += 8;
  }

  if (default_collation_for_utf8mb4_number) {
    DBUG_ASSERT(default_collation_for_utf8mb4_number <= 0xFF);
    *start++ = Q_DEFAULT_COLLATION_FOR_UTF8MB4;
    int2store(start, default_collation_for_utf8mb4_number);
    start += 2;
  }

  if (thd && need_sql_require_primary_key) {
    *start++ = Q_SQL_REQUIRE_PRIMARY_KEY;
    *start++ = thd->variables.sql_require_primary_key;
  }

  if (thd && needs_default_table_encryption) {
    *start++ = Q_DEFAULT_TABLE_ENCRYPTION;
    *start++ = thd->variables.default_table_encryption;
  }

  /*
    NOTE: When adding new status vars, please don't forget to update
    the MAX_SIZE_LOG_EVENT_STATUS in log_event.h

    Here there could be code like
    if (command-line-option-which-says-"log_this_variable" && inited)
    {
    *start++= Q_THIS_VARIABLE_CODE;
    int4store(start, this_variable);
    start+= 4;
    }
  */

  /* Store length of status variables */
  status_vars_len = (uint)(start - start_of_status);
  DBUG_ASSERT(status_vars_len <= MAX_SIZE_LOG_EVENT_STATUS);
  int2store(buf + Q_STATUS_VARS_LEN_OFFSET, status_vars_len);

  /*
    Calculate length of whole event
    The "1" below is the \0 in the db's length
  */
  event_length = (uint)(start - buf) + get_post_header_size_for_derived() +
                 db_len + 1 + q_len;

  return (write_header(ostream, event_length) ||
          wrapper_my_b_safe_write(ostream, (uchar *)buf,
                                  Binary_log_event::QUERY_HEADER_LEN) ||
          write_post_header_for_derived(ostream) ||
          wrapper_my_b_safe_write(ostream, start_of_status,
                                  (uint)(start - start_of_status)) ||
          wrapper_my_b_safe_write(ostream,
                                  db ? pointer_cast<const uchar *>(db)
                                     : pointer_cast<const uchar *>(""),
                                  db_len + 1) ||
          wrapper_my_b_safe_write(ostream, pointer_cast<const uchar *>(query),
                                  q_len) ||
          write_footer(ostream))
             ? true
             : false;
}

/**
  The simplest constructor that could possibly work.  This is used for
  creating static objects that have a special meaning and are invisible
  to the log.
*/
Query_log_event::Query_log_event()
    : binary_log::Query_event(),
      Log_event(header(), footer()),
      data_buf(nullptr) {}

/**
  Returns true when the lex context determines an atomic DDL.
  The result is optimistic as there can be more properties to check out.

  @param lex  pointer to LEX object of being executed statement
*/
inline bool is_sql_command_atomic_ddl(const LEX *lex) {
  return ((sql_command_flags[lex->sql_command] & CF_POTENTIAL_ATOMIC_DDL) &&
          lex->sql_command != SQLCOM_OPTIMIZE &&
          lex->sql_command != SQLCOM_REPAIR &&
          lex->sql_command != SQLCOM_ANALYZE) ||
         (lex->sql_command == SQLCOM_CREATE_TABLE &&
          !(lex->create_info->options & HA_LEX_CREATE_TMP_TABLE)) ||
         (lex->sql_command == SQLCOM_DROP_TABLE && !lex->drop_temporary);
}

/**
  Returns whether or not the statement held by the `LEX` object parameter
  requires `Q_SQL_REQUIRE_PRIMARY_KEY` to be logged together with the statement.
 */
static bool is_sql_require_primary_key_needed(const LEX *lex) {
  enum enum_sql_command cmd = lex->sql_command;
  switch (cmd) {
    case SQLCOM_CREATE_TABLE:
    case SQLCOM_ALTER_TABLE:
      return true;
    default:
      break;
  }
  return false;
}

/**
  Returns whether or not the statement held by the `LEX` object parameter
  requires `Q_DEFAULT_TABLE_ENCRYPTION` to be logged together with the
  statement.
 */
static bool is_default_table_encryption_needed(const LEX *lex) {
  enum enum_sql_command cmd = lex->sql_command;
  switch (cmd) {
    case SQLCOM_CREATE_DB:
      // If it is CREATE DATABASE without an ENCRYPTION clause
      return !(lex->create_info->used_fields &
               HA_CREATE_USED_DEFAULT_ENCRYPTION);
    case SQLCOM_ALTER_TABLESPACE: {
      /*
        If it is CREATE TABLESPACE without an ENCRYPTION clause.  Note
        that CREATE TABLESPACE uses SQLCOM_ALTER_TABLESPACE, so to
        know if it is really a CREATE TABLESPACE we check that the
        dynamic_cast to Sql_cmd_create_tablespace works.
      */
      const Sql_cmd_tablespace *sct =
          dynamic_cast<const Sql_cmd_create_tablespace *>(lex->m_sql_cmd);
      return ((sct != nullptr) &&
              (sct->get_options().encryption.str == nullptr));
    }
    default:
      break;
  }
  return false;
}

bool is_atomic_ddl(THD *thd, bool using_trans_arg) {
  LEX *lex = thd->lex;

#ifndef DBUG_OFF
  enum enum_sql_command cmd = lex->sql_command;
  switch (cmd) {
    case SQLCOM_CREATE_USER:
    case SQLCOM_RENAME_USER:
    case SQLCOM_DROP_USER:
    case SQLCOM_ALTER_USER:
    case SQLCOM_ALTER_USER_DEFAULT_ROLE:
    case SQLCOM_GRANT:
    case SQLCOM_GRANT_ROLE:
    case SQLCOM_REVOKE:
    case SQLCOM_REVOKE_ALL:
    case SQLCOM_REVOKE_ROLE:
    case SQLCOM_DROP_ROLE:
    case SQLCOM_CREATE_ROLE:
    case SQLCOM_SET_PASSWORD:
    case SQLCOM_CREATE_TRIGGER:
    case SQLCOM_DROP_TRIGGER:
    case SQLCOM_ALTER_FUNCTION:
    case SQLCOM_CREATE_SPFUNCTION:
    case SQLCOM_DROP_FUNCTION:
    case SQLCOM_CREATE_FUNCTION:
    case SQLCOM_CREATE_PROCEDURE:
    case SQLCOM_DROP_PROCEDURE:
    case SQLCOM_ALTER_PROCEDURE:
    case SQLCOM_ALTER_EVENT:
    case SQLCOM_DROP_EVENT:
    case SQLCOM_CREATE_VIEW:
    case SQLCOM_DROP_VIEW:

      DBUG_ASSERT(using_trans_arg || thd->slave_thread || lex->drop_if_exists);

      break;

    case SQLCOM_CREATE_EVENT:
      /*
        trx cache is *not* used if event already exists and IF NOT EXISTS clause
        is used in the statement or if call is from the slave applier.
      */
      DBUG_ASSERT(using_trans_arg || thd->slave_thread ||
                  (lex->create_info->options & HA_LEX_CREATE_IF_NOT_EXISTS));
      break;

    default:
      break;
  }
#endif

  return using_trans_arg && is_sql_command_atomic_ddl(lex);
}

/**
  Creates a Query Log Event.

  @param thd_arg      Thread handle
  @param query_arg    Array of char representing the query
  @param query_length Size of the 'query_arg' array
  @param using_trans  Indicates that there are transactional changes.
  @param immediate    After being written to the binary log, the event
                      must be flushed immediately. This indirectly implies
                      the stmt-cache.
  @param suppress_use Suppress the generation of 'USE' statements
  @param errcode      The error code of the query
  @param ignore_cmd_internals       Ignore user's statement, i.e. lex
  information, while deciding which cache must be used.
*/
Query_log_event::Query_log_event(THD *thd_arg, const char *query_arg,
                                 size_t query_length, bool using_trans,
                                 bool immediate, bool suppress_use, int errcode,
                                 bool ignore_cmd_internals)

    : binary_log::Query_event(
          query_arg, thd_arg->catalog().str, thd_arg->db().str, query_length,
          thd_arg->thread_id(), thd_arg->variables.sql_mode,
          thd_arg->variables.auto_increment_increment,
          thd_arg->variables.auto_increment_offset,
          thd_arg->variables.lc_time_names->number,
          (ulonglong)thd_arg->table_map_for_update, errcode),
      Log_event(
          thd_arg,
          (thd_arg->thread_specific_used ? LOG_EVENT_THREAD_SPECIFIC_F : 0) |
              (suppress_use ? LOG_EVENT_SUPPRESS_USE_F : 0),
          using_trans ? Log_event::EVENT_TRANSACTIONAL_CACHE
                      : Log_event::EVENT_STMT_CACHE,
          Log_event::EVENT_NORMAL_LOGGING, header(), footer()),
      data_buf(nullptr) {
  /* save the original thread id; we already know the server id */
  slave_proxy_id = thd_arg->variables.pseudo_thread_id;
  common_header->set_is_valid(query != nullptr);

  /*
  exec_time calculation has changed to use the same method that is used
  to fill out "thd_arg->start_time"
  */

  struct timeval end_time;
  ulonglong micro_end_time = my_micro_time();
  my_micro_time_to_timeval(micro_end_time, &end_time);

  exec_time = end_time.tv_sec - thd_arg->query_start_in_secs();

  /**
    @todo this means that if we have no catalog, then it is replicated
    as an existing catalog of length zero. is that safe? /sven
  */
  catalog_len = (catalog) ? strlen(catalog) : 0;
  /* status_vars_len is set just before writing the event */
  db_len = (db) ? strlen(db) : 0;
  if (thd_arg->variables.collation_database != thd_arg->db_charset)
    charset_database_number = thd_arg->variables.collation_database->number;

  default_collation_for_utf8mb4_number =
      thd_arg->variables.default_collation_for_utf8mb4->number;

  /*
    We only replicate over the bits of flags2 that we need: the rest
    are masked out by "& OPTIONS_WRITTEN_TO_BINLOG".

    We also force AUTOCOMMIT=1.  Rationale (cf. BUG#29288): After
    fixing BUG#26395, we always write BEGIN and COMMIT around all
    transactions (even single statements in autocommit mode).  This is
    so that replication from non-transactional to transactional table
    and error recovery from XA to non-XA table should work as
    expected.  The BEGIN/COMMIT are added in log.cc. However, there is
    one exception: MyISAM bypasses log.cc and writes directly to the
    binlog.  So if autocommit is off, master has MyISAM, and slave has
    a transactional engine, then the slave will just see one long
    never-ending transaction.  The only way to bypass explicit
    BEGIN/COMMIT in the binlog is by using a non-transactional table.
    So setting AUTOCOMMIT=1 will make this work as expected.

    Note: explicitly replicate AUTOCOMMIT=1 from master. We do not
    assume AUTOCOMMIT=1 on slave; the slave still reads the state of
    the autocommit flag as written by the master to the binlog. This
    behavior may change after WL#4162 has been implemented.
  */
  flags2 = (uint32)(thd_arg->variables.option_bits &
                    (OPTIONS_WRITTEN_TO_BIN_LOG & ~OPTION_NOT_AUTOCOMMIT));
  DBUG_ASSERT(thd_arg->variables.character_set_client->number < 256 * 256);
  DBUG_ASSERT(thd_arg->variables.collation_connection->number < 256 * 256);
  DBUG_ASSERT(thd_arg->variables.collation_server->number < 256 * 256);
  DBUG_ASSERT(thd_arg->variables.character_set_client->mbminlen == 1);
  int2store(charset, thd_arg->variables.character_set_client->number);
  int2store(charset + 2, thd_arg->variables.collation_connection->number);
  int2store(charset + 4, thd_arg->variables.collation_server->number);
  if (thd_arg->time_zone_used) {
    /*
      Note that our event becomes dependent on the Time_zone object
      representing the time zone. Fortunately such objects are never deleted
      or changed during mysqld's lifetime.
    */
    time_zone_len = thd_arg->variables.time_zone->get_name()->length();
    time_zone_str = thd_arg->variables.time_zone->get_name()->ptr();
  } else
    time_zone_len = 0;

  /*
    In what follows, we define in which cache, trx-cache or stmt-cache,
    this Query Log Event will be written to.

    If ignore_cmd_internals is defined, we rely on the is_trans flag to
    choose the cache and this is done in the base class Log_event. False
    means that the stmt-cache will be used and upon statement commit/rollback
    the cache will be flushed to disk. True means that the trx-cache will
    be used and upon transaction commit/rollback the cache will be flushed
    to disk.

    If set immediate cache is defined, for convenience, we automatically
    use the stmt-cache. This mean that the statement will be written
    to the stmt-cache and immediately flushed to disk without waiting
    for a commit/rollback notification.

    For example, the cluster/ndb captures a request to execute a DDL
    statement and synchronously propagate it to all available MySQL
    servers. Unfortunately, the current protocol assumes that the
    generated events are immediately written to diks and does not check
    for commit/rollback.

    Upon dropping a connection, DDLs (i.e. DROP TEMPORARY TABLE) are
    generated and in this case the statements have the immediate flag
    set because there is no commit/rollback.

    If the immediate flag is not set, the decision on the cache is based
    on the current statement and the flag is_trans, which indicates if
    a transactional engine was updated.

    Statements are classifed as row producers (i.e. can_generate_row_events())
    or non-row producers. Non-row producers, DDL in general, are treated
    as the immediate flag was set and for convenience are written to the
    stmt-cache and immediately flushed to disk.

    Row producers are handled in general according to the is_trans flag.
    False means that the stmt-cache will be used and upon statement
    commit/rollback the cache will be flushed to disk. True means that the
    trx-cache will be used and upon transaction commit/rollback the cache
    will be flushed to disk.

    Unfortunately, there are exceptions to this non-row and row producer
    rules:

      . The SAVEPOINT, ROLLBACK TO SAVEPOINT, RELEASE SAVEPOINT does not
        have the flag is_trans set because there is no updated engine but
        must be written to the trx-cache.

      . SET If auto-commit is on, it must not go through a cache.

      . CREATE TABLE is classfied as non-row producer but CREATE TEMPORARY
        must be handled as row producer.

      . DROP TABLE is classfied as non-row producer but DROP TEMPORARY
        must be handled as row producer.

    Finally, some statements that does not have the flag is_trans set may
    be written to the trx-cache based on the following criteria:

      . updated both a transactional and a non-transactional engine (i.e.
        stmt_has_updated_trans_table()).

      . accessed both a transactional and a non-transactional engine and
        is classified as unsafe (i.e. is_mixed_stmt_unsafe()).

      . is executed within a transaction and previously a transactional
        engine was updated and the flag binlog_direct_non_trans_update
        is set.
  */
  if (ignore_cmd_internals) return;

  /*
    true defines that the trx-cache must be used.
  */
  bool cmd_can_generate_row_events = false;
  /*
    true defines that the trx-cache must be used.
  */
  bool cmd_must_go_to_trx_cache = false;

  LEX *lex = thd->lex;
  if (!immediate) {
    switch (lex->sql_command) {
      case SQLCOM_DROP_TABLE:
        cmd_can_generate_row_events =
            lex->drop_temporary && thd->in_multi_stmt_transaction_mode();
        break;
      case SQLCOM_CREATE_TABLE:
        cmd_must_go_to_trx_cache = lex->select_lex->item_list.elements &&
                                   thd->is_current_stmt_binlog_format_row();
        cmd_can_generate_row_events =
            ((lex->create_info->options & HA_LEX_CREATE_TMP_TABLE) &&
             thd->in_multi_stmt_transaction_mode()) ||
            cmd_must_go_to_trx_cache;
        break;
      case SQLCOM_SET_OPTION:
        if (lex->autocommit)
          cmd_can_generate_row_events = cmd_must_go_to_trx_cache = false;
        else
          cmd_can_generate_row_events = true;
        break;
      case SQLCOM_RELEASE_SAVEPOINT:
      case SQLCOM_ROLLBACK_TO_SAVEPOINT:
      case SQLCOM_SAVEPOINT:
      case SQLCOM_XA_PREPARE:
        cmd_can_generate_row_events = cmd_must_go_to_trx_cache = true;
        break;
      default:
        cmd_can_generate_row_events =
            sqlcom_can_generate_row_events(thd->lex->sql_command);
        break;
    }
  } else {
    DBUG_ASSERT(!using_trans);  // immediate is imcompatible with using_trans
  }

  /*
    Drop the flag as sort of reset right before the being logged query
    gets classified as possibly not atomic DDL.
  */
  if (thd->rli_slave) thd->rli_slave->ddl_not_atomic = false;

  if (cmd_can_generate_row_events) {
    cmd_must_go_to_trx_cache = cmd_must_go_to_trx_cache || using_trans;
    if (cmd_must_go_to_trx_cache ||
        stmt_has_updated_trans_table(
            thd->get_transaction()->ha_trx_info(Transaction_ctx::STMT)) ||
        thd->lex->is_mixed_stmt_unsafe(
            thd->in_multi_stmt_transaction_mode(),
            thd->variables.binlog_direct_non_trans_update,
            trans_has_updated_trans_table(thd), thd->tx_isolation) ||
        (!thd->variables.binlog_direct_non_trans_update &&
         trans_has_updated_trans_table(thd))) {
      event_logging_type = Log_event::EVENT_NORMAL_LOGGING;
      event_cache_type = Log_event::EVENT_TRANSACTIONAL_CACHE;
    } else {
      event_logging_type = Log_event::EVENT_NORMAL_LOGGING;
      event_cache_type = Log_event::EVENT_STMT_CACHE;
    }
  } else if (is_atomic_ddl(thd, using_trans)) {
    DBUG_ASSERT(stmt_causes_implicit_commit(thd, CF_IMPLICIT_COMMIT_END));
    /*
      Event creation is normally followed by its logging.
      Todo: add exceptions if any.
    */
    DBUG_ASSERT(!thd->is_operating_substatement_implicitly);

    Transaction_ctx *trn_ctx = thd->get_transaction();

    /* Transaction needs to be active for xid to be assigned, */
    DBUG_ASSERT(trn_ctx->is_active(Transaction_ctx::SESSION));
    /* and the transaction's xid has been already computed. */
    DBUG_ASSERT(!trn_ctx->xid_state()->get_xid()->is_null());

    my_xid xid = trn_ctx->xid_state()->get_xid()->get_my_xid();

    /*
      xid uniqueness: the last time used not equal to the current one
    */
    DBUG_ASSERT(thd->debug_binlog_xid_last.is_null() ||
                thd->debug_binlog_xid_last.get_my_xid() != xid);

    ddl_xid = xid;
#ifndef DBUG_OFF
    thd->debug_binlog_xid_last = *trn_ctx->xid_state()->get_xid();
#endif
    event_logging_type = Log_event::EVENT_NORMAL_LOGGING;
    event_cache_type = Log_event::EVENT_TRANSACTIONAL_CACHE;
  } else {
    /*
      Note SQLCOM_XA_COMMIT, SQLCOM_XA_ROLLBACK fall into this block.
      Even though CREATE-TABLE sub-statement of CREATE-TABLE-SELECT in
      RBR makes a turn here it is logged atomically with the SELECT
      Rows-log event part that determines the xid of the entire group.
    */
    event_logging_type = Log_event::EVENT_IMMEDIATE_LOGGING;
    event_cache_type = Log_event::EVENT_STMT_CACHE;

    DBUG_ASSERT(ddl_xid == binary_log::INVALID_XID);

    if (thd->rli_slave) thd->rli_slave->ddl_not_atomic = true;
  }

  need_sql_require_primary_key = is_sql_require_primary_key_needed(lex);

  needs_default_table_encryption = is_default_table_encryption_needed(lex);

  DBUG_ASSERT(event_cache_type != Log_event::EVENT_INVALID_CACHE);
  DBUG_ASSERT(event_logging_type != Log_event::EVENT_INVALID_LOGGING);
  DBUG_PRINT("info", ("Query_log_event has flags2: %lu  sql_mode: %llu",
                      (ulong)flags2, (ulonglong)sql_mode));
}
#endif /* MYSQL_SERVER */

/**
  This is used by the SQL slave thread to prepare the event before execution.
*/
Query_log_event::Query_log_event(
    const char *buf, const Format_description_event *description_event,
    Log_event_type event_type)
    : binary_log::Query_event(buf, description_event, event_type),
      Log_event(header(), footer()),
      has_ddl_committed(false) {
  DBUG_TRACE;
  data_buf = nullptr;
  if (!is_valid()) return;

  slave_proxy_id = thread_id;
  exec_time = query_exec_time;

  ulong buf_len = catalog_len + 1 + time_zone_len + 1 + user_len + 1 +
                  host_len + 1 + data_len + 1;

  if (!(data_buf = (Log_event_header::Byte *)my_malloc(key_memory_log_event,
                                                       buf_len, MYF(MY_WME)))) {
    common_header->set_is_valid(false);
    return;
  }
  /*
    The data buffer is used by the slave SQL thread while applying
    the event. The catalog, time_zone)str, user, host, db, query
    are pointers to this data_buf. The function call below, points these
    const pointers to the data buffer.
  */
  if (!(fill_data_buf(data_buf, buf_len))) {
    common_header->set_is_valid(false);
    return;
  }

  common_header->set_is_valid(query != nullptr && q_len > 0);
}

#ifndef MYSQL_SERVER
/**
  Given a timestamp (microseconds since epoch), generate a string
  of the form YYYY-MM-DD HH:MM:SS.UUUUUU and return the length.

  @param timestamp timestamp to convert to string.
  @param buf Buffer to which timestamp will be written as a string.
  @return The length of the string containing the converted timestamp
*/
inline size_t microsecond_timestamp_to_str(ulonglong timestamp, char *buf) {
  time_t seconds = (time_t)(timestamp / 1000000);
  int useconds = (int)(timestamp % 1000000);
  struct tm time_struct;
  localtime_r(&seconds, &time_struct);
  size_t length = strftime(buf, 255, "%F %T", &time_struct);
  length += sprintf(buf + length, ".%06d", useconds);
  length += strftime(buf + length, 255, " %Z", &time_struct);
  return length;
}

/**
  Query_log_event::print().

  @todo
    print the catalog ??
*/
void Query_log_event::print_query_header(
    IO_CACHE *file, PRINT_EVENT_INFO *print_event_info) const {
  // TODO: print the catalog ??
  char buff[48], *end;  // Enough for "SET TIMESTAMP=1305535348.123456"
  char quoted_id[1 + 2 * FN_REFLEN + 2];
  size_t quoted_len = 0;
  bool different_db = true;
  uint32 tmp;

  if (!print_event_info->short_form) {
    const char xid_assign[] = "\tXid = ";
    char xid_buf[64 + sizeof(xid_assign) - 1] = {0};
    if (ddl_xid != binary_log::INVALID_XID) {
      strcpy(xid_buf, xid_assign);
      longlong10_to_str(ddl_xid, xid_buf + strlen(xid_assign), 10);
    }
    print_header(file, print_event_info, false);
    my_b_printf(file, "\t%s\tthread_id=%lu\texec_time=%lu\terror_code=%d%s\n",
                get_type_str(), (ulong)thread_id, (ulong)exec_time, error_code,
                xid_buf);
  }

  if ((common_header->flags & LOG_EVENT_SUPPRESS_USE_F)) {
    if (!is_trans_keyword()) print_event_info->db[0] = '\0';
  } else if (db) {
    quoted_len = my_strmov_quoted_identifier((char *)quoted_id, db);
    quoted_id[quoted_len] = '\0';
    different_db = memcmp(print_event_info->db, db, db_len + 1);
    if (different_db) memcpy(print_event_info->db, db, db_len + 1);
    if (db[0] && different_db)
      my_b_printf(file, "use %s%s\n", quoted_id, print_event_info->delimiter);
  }

  end = longlong10_to_str(common_header->when.tv_sec,
                          my_stpcpy(buff, "SET TIMESTAMP="), 10);
  if (common_header->when.tv_usec)
    end += sprintf(end, ".%06d", (int)common_header->when.tv_usec);
  end = my_stpcpy(end, print_event_info->delimiter);
  *end++ = '\n';
  DBUG_ASSERT(end < buff + sizeof(buff));
  my_b_write(file, (uchar *)buff, (uint)(end - buff));
  if (!print_event_info->require_row_format &&
      (!print_event_info->thread_id_printed ||
       ((common_header->flags & LOG_EVENT_THREAD_SPECIFIC_F) &&
        thread_id != print_event_info->thread_id))) {
    // If --short-form, print deterministic value instead of pseudo_thread_id.
    my_b_printf(file, "SET @@session.pseudo_thread_id=%lu%s\n",
                short_form ? 999999999 : (ulong)thread_id,
                print_event_info->delimiter);
    print_event_info->thread_id = thread_id;
    print_event_info->thread_id_printed = true;
  }

  /*
    If flags2_inited==0, this is an event from 3.23 or 4.0; nothing to
    print (remember we don't produce mixed relay logs so there cannot be
    5.0 events before that one so there is nothing to reset).
  */
  if (likely(flags2_inited)) /* likely as this will mainly read 5.0 logs */
  {
    /* tmp is a bitmask of bits which have changed. */
    if (likely(print_event_info->flags2_inited))
      /* All bits which have changed */
      tmp = (print_event_info->flags2) ^ flags2;
    else /* that's the first Query event we read */
    {
      print_event_info->flags2_inited = true;
      tmp = ~((uint32)0); /* all bits have changed */
    }

    if (unlikely(tmp)) /* some bits have changed */
    {
      bool need_comma = false;
      my_b_printf(file, "SET ");
      print_set_option(file, tmp, OPTION_NO_FOREIGN_KEY_CHECKS, ~flags2,
                       "@@session.foreign_key_checks", &need_comma);
      print_set_option(file, tmp, OPTION_AUTO_IS_NULL, flags2,
                       "@@session.sql_auto_is_null", &need_comma);
      print_set_option(file, tmp, OPTION_RELAXED_UNIQUE_CHECKS, ~flags2,
                       "@@session.unique_checks", &need_comma);
      print_set_option(file, tmp, OPTION_NOT_AUTOCOMMIT, ~flags2,
                       "@@session.autocommit", &need_comma);
      my_b_printf(file, "%s\n", print_event_info->delimiter);
      print_event_info->flags2 = flags2;
    }
  }

  /*
    Now the session variables;
    it's more efficient to pass SQL_MODE as a number instead of a
    comma-separated list.
    FOREIGN_KEY_CHECKS, SQL_AUTO_IS_NULL, UNIQUE_CHECKS are session-only
    variables (they have no global version; they're not listed in
    sql_class.h), The tests below work for pure binlogs or pure relay
    logs. Won't work for mixed relay logs but we don't create mixed
    relay logs (that is, there is no relay log with a format change
    except within the 3 first events, which mysqlbinlog handles
    gracefully). So this code should always be good.
  */

  if (likely(sql_mode_inited) &&
      (unlikely(print_event_info->sql_mode != sql_mode ||
                !print_event_info->sql_mode_inited))) {
    /*
      All the SQL_MODEs included in 0x1003ff00 were removed in 8.0.5. The
      upgrade procedure clears these bits. So the bits can only be set on older
      binlogs. Therefore, we generate this version-conditioned expression that
      masks out the removed modes in case this is executed on 8.0.5 or later.
    */
    const char *mask = "";
    if (sql_mode & 0x1003ff00) mask = "/*!80005 &~0x1003ff00*/";
    my_b_printf(file, "SET @@session.sql_mode=%lu%s%s\n", (ulong)sql_mode, mask,
                print_event_info->delimiter);
    print_event_info->sql_mode = sql_mode;
    print_event_info->sql_mode_inited = true;
  }
  if (print_event_info->auto_increment_increment != auto_increment_increment ||
      print_event_info->auto_increment_offset != auto_increment_offset) {
    my_b_printf(file,
                "SET @@session.auto_increment_increment=%u, "
                "@@session.auto_increment_offset=%u%s\n",
                auto_increment_increment, auto_increment_offset,
                print_event_info->delimiter);
    print_event_info->auto_increment_increment = auto_increment_increment;
    print_event_info->auto_increment_offset = auto_increment_offset;
  }

  /* TODO: print the catalog when we feature SET CATALOG */

  if (likely(charset_inited) &&
      (unlikely(!print_event_info->charset_inited ||
                memcmp(print_event_info->charset, charset, 6)))) {
    const char *charset_p = charset;  // Avoid type-punning warning.
    CHARSET_INFO *cs_info = get_charset(uint2korr(charset_p), MYF(MY_WME));
    if (cs_info) {
      /* for mysql client */
      my_b_printf(file, "/*!\\C %s */%s\n", cs_info->csname,
                  print_event_info->delimiter);
    }
    my_b_printf(file,
                "SET "
                "@@session.character_set_client=%d,"
                "@@session.collation_connection=%d,"
                "@@session.collation_server=%d"
                "%s\n",
                uint2korr(charset_p), uint2korr(charset + 2),
                uint2korr(charset + 4), print_event_info->delimiter);
    memcpy(print_event_info->charset, charset, 6);
    print_event_info->charset_inited = true;
  }
  if (time_zone_len) {
    if (memcmp(print_event_info->time_zone_str, time_zone_str,
               time_zone_len + 1)) {
      my_b_printf(file, "SET @@session.time_zone='%s'%s\n", time_zone_str,
                  print_event_info->delimiter);
      memcpy(print_event_info->time_zone_str, time_zone_str, time_zone_len + 1);
    }
  }
  if (lc_time_names_number != print_event_info->lc_time_names_number) {
    my_b_printf(file, "SET @@session.lc_time_names=%d%s\n",
                lc_time_names_number, print_event_info->delimiter);
    print_event_info->lc_time_names_number = lc_time_names_number;
  }
  if (charset_database_number != print_event_info->charset_database_number) {
    if (charset_database_number)
      my_b_printf(file, "SET @@session.collation_database=%d%s\n",
                  charset_database_number, print_event_info->delimiter);
    else
      my_b_printf(file, "SET @@session.collation_database=DEFAULT%s\n",
                  print_event_info->delimiter);
    print_event_info->charset_database_number = charset_database_number;
  }
  if (explicit_defaults_ts != TERNARY_UNSET)
    my_b_printf(file, "SET @@session.explicit_defaults_for_timestamp=%d%s\n",
                explicit_defaults_ts == TERNARY_OFF ? 0 : 1,
                print_event_info->delimiter);
  if (default_collation_for_utf8mb4_number !=
      print_event_info->default_collation_for_utf8mb4_number) {
    if (default_collation_for_utf8mb4_number)
      my_b_printf(
          file, "/*!80011 SET @@session.default_collation_for_utf8mb4=%d*/%s\n",
          default_collation_for_utf8mb4_number, print_event_info->delimiter);
    print_event_info->default_collation_for_utf8mb4_number =
        default_collation_for_utf8mb4_number;
  }
  if (sql_require_primary_key != print_event_info->sql_require_primary_key) {
    my_b_printf(file, "/*!80013 SET @@session.sql_require_primary_key=%d*/%s\n",
                sql_require_primary_key, print_event_info->delimiter);
  }
  if (default_table_encryption != print_event_info->default_table_encryption) {
    my_b_printf(file,
                "/*!80016 SET @@session.default_table_encryption=%d*/%s\n",
                default_table_encryption, print_event_info->delimiter);
  }
}

void Query_log_event::print(FILE *, PRINT_EVENT_INFO *print_event_info) const {
  IO_CACHE *const head = &print_event_info->head_cache;

  /**
    reduce the size of io cache so that the write function is called
    for every call to my_b_write().
   */
  DBUG_EXECUTE_IF("simulate_file_write_error",
                  { head->write_pos = head->write_end - 500; });
  print_query_header(head, print_event_info);
  my_b_write(head, pointer_cast<const uchar *>(query), q_len);
  my_b_printf(head, "\n%s\n", print_event_info->delimiter);
}
#endif /* !MYSQL_SERVER */

#if defined(MYSQL_SERVER)

/**
   Associating slave Worker thread to a subset of temporary tables.

   @param thd_arg THD instance pointer
   @param rli     Relay_log_info of the worker
*/
void Query_log_event::attach_temp_tables_worker(THD *thd_arg,
                                                const Relay_log_info *rli) {
  if (!is_skip_temp_tables_handling_by_worker())
    rli->current_mts_submode->attach_temp_tables(thd_arg, rli, this);
}

/**
   Dissociating slave Worker thread from its thd->temporary_tables
   to possibly update the involved entries of db-to-worker hash
   with new values of temporary_tables.

   @param thd_arg THD instance pointer
   @param rli     relay log info of the worker thread
*/
void Query_log_event::detach_temp_tables_worker(THD *thd_arg,
                                                const Relay_log_info *rli) {
  if (!is_skip_temp_tables_handling_by_worker())
    rli->current_mts_submode->detach_temp_tables(thd_arg, rli, this);
}

/*
  Query_log_event::do_apply_event()
*/
int Query_log_event::do_apply_event(Relay_log_info const *rli) {
  // Note: We're using event's future_event_relay_log_pos instead of
  // rli->get_event_relay_log_pos() because rli is only updated in
  // do_update_pos() which is called after applying the event and we might need
  // to use this pos during application (e.g. during commit)
  Relay_log_info *rli_ptr = const_cast<Relay_log_info *>(rli);
  thd->set_trans_relay_log_pos(rli_ptr->get_event_relay_log_name(),
                               future_event_relay_log_pos);
  return do_apply_event(rli, query, q_len);
}

int Query_log_event::do_apply_event_worker(Slave_worker *w) {
  // Note: We're using event's future_event_relay_log_pos instead of
  // rli->get_event_relay_log_pos() because rli is only updated in
  // do_update_pos() which is called after applying the event and we might
  // need to use this pos during application (e.g. during commit)
  Slave_job_group *ptr_g = w->c_rli->gaq->get_job_group(mts_group_idx);
  thd->set_trans_relay_log_pos(ptr_g->group_relay_log_name
                                   ? ptr_g->group_relay_log_name
                                   : w->get_group_relay_log_name(),
                               future_event_relay_log_pos);
  return do_apply_event(w, query, q_len);
}

/*
  is_silent_error

  Return true if the thread has an error which should be
  handled silently
*/

static bool is_silent_error(THD *thd) {
  DBUG_TRACE;
  Diagnostics_area::Sql_condition_iterator it =
      thd->get_stmt_da()->sql_conditions();
  const Sql_condition *err;
  while ((err = it++)) {
    DBUG_PRINT("info", ("has condition %d %s", err->mysql_errno(),
                        err->message_text()));
    switch (err->mysql_errno()) {
      case ER_SLAVE_SILENT_RETRY_TRANSACTION: {
        return true;
      }
      default:
        break;
    }
  }
  return false;
}

/**
  @todo
  Compare the values of "affected rows" around here. Something
  like:
  @code
     if ((uint32) affected_in_event != (uint32) affected_on_slave)
     {
     sql_print_error("Slave: did not get the expected number of affected \
     rows running query from master - expected %d, got %d (this numbers \
     should have matched modulo 4294967296).", 0, ...);
     thd->query_error = 1;
     }
  @endcode
  We may also want an option to tell the slave to ignore "affected"
  mismatch. This mismatch could be implemented with a new ER_ code, and
  to ignore it you would use --slave-skip-errors...
*/
int Query_log_event::do_apply_event(Relay_log_info const *rli,
                                    const char *query_arg, size_t q_len_arg) {
  DBUG_TRACE;
  int expected_error, actual_error = 0;

  DBUG_PRINT("info", ("query=%s, q_len_arg=%lu", query,
                      static_cast<unsigned long>(q_len_arg)));

  // case: a rows query event containing trx metadata was encountered, we're
  // going to clean that up here
  if (rli->rows_query_ev && rli->rows_query_ev->has_trx_meta_data()) {
    delete const_cast<Relay_log_info *>(rli)->rows_query_ev;
    const_cast<Relay_log_info *>(rli)->rows_query_ev = NULL;
    thd->set_query(NULL, 0);
  }

  /*
    Colleagues: please never free(thd->catalog) in MySQL. This would
    lead to bugs as here thd->catalog is a part of an alloced block,
    not an entire alloced block (see
    Query_log_event::do_apply_event()). Same for thd->db().str.  Thank
    you.
  */

  if (catalog_len) {
    LEX_CSTRING catalog_lex_cstr = {catalog, catalog_len};
    thd->set_catalog(catalog_lex_cstr);
  } else
    thd->set_catalog(EMPTY_CSTR);

  bool need_inc_rewrite_db_filter_counter;
  size_t valid_len;
  bool len_error;
  bool is_invalid_db_name =
      validate_string(system_charset_info, db, db_len, &valid_len, &len_error);

  DBUG_PRINT("debug", ("is_invalid_db_name= %s, valid_len=%zu, len_error=%s",
                       is_invalid_db_name ? "true" : "false", valid_len,
                       len_error ? "true" : "false"));

  if (is_invalid_db_name || len_error) {
    rli->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                ER_THD(thd, ER_SLAVE_FATAL_ERROR),
                "Invalid database name in Query event.");
    thd->is_slave_error = true;
    goto end;
  }

  need_inc_rewrite_db_filter_counter = set_thd_db(thd, db, db_len);

  /*
    Setting the character set and collation of the current database thd->db.
   */
  if (get_default_db_collation(thd, thd->db().str, &thd->db_charset)) {
    DBUG_ASSERT(thd->is_error() || thd->killed);
    rli->report(ERROR_LEVEL, thd->get_stmt_da()->mysql_errno(),
                "Error in get_default_db_collation: %s",
                thd->get_stmt_da()->message_text());
    thd->is_slave_error = true;
    goto end;
  }

  thd->db_charset = thd->db_charset ? thd->db_charset : thd->collation();

  thd->variables.auto_increment_increment = auto_increment_increment;
  thd->variables.auto_increment_offset = auto_increment_offset;
  if (explicit_defaults_ts != TERNARY_UNSET)
    thd->variables.explicit_defaults_for_timestamp =
        explicit_defaults_ts == TERNARY_OFF ? false : true;

  /*
    todo: such cleanup should not be specific to Query event and therefore
          is preferable at a common with other event pre-execution point
  */
  clear_all_errors(thd, const_cast<Relay_log_info *>(rli));
  if (strcmp("COMMIT", query) == 0 && rli->tables_to_lock != nullptr) {
    /*
      Cleaning-up the last statement context:
      the terminal event of the current statement flagged with
      STMT_END_F got filtered out in ndb circular replication.
    */
    int error;
    char llbuff[22];
    if ((error =
             rows_event_stmt_cleanup(const_cast<Relay_log_info *>(rli), thd))) {
      const_cast<Relay_log_info *>(rli)->report(
          ERROR_LEVEL, error,
          "Error in cleaning up after an event preceding the commit; "
          "the group log file/position: %s %s",
          const_cast<Relay_log_info *>(rli)->get_group_master_log_name(),
          llstr(const_cast<Relay_log_info *>(rli)->get_group_master_log_pos(),
                llbuff));
    }
    /*
      Executing a part of rli->stmt_done() logics that does not deal
      with group position change. The part is redundant now but is
      future-change-proof addon, e.g if COMMIT handling will start checking
      invariants like IN_STMT flag must be off at committing the transaction.
    */
    const_cast<Relay_log_info *>(rli)->inc_event_relay_log_pos();
    const_cast<Relay_log_info *>(rli)->clear_flag(Relay_log_info::IN_STMT);
  } else {
    const_cast<Relay_log_info *>(rli)->slave_close_thread_tables(thd);
  }

  {
    Applier_security_context_guard security_context{rli, thd};
    if (!thd->variables.require_row_format) {
      if (!security_context.skip_priv_checks() &&
          !security_context.has_access({SUPER_ACL}) &&
          !security_context.has_access({"SYSTEM_VARIABLES_ADMIN"}) &&
          !security_context.has_access({"SESSION_VARIABLES_ADMIN"})) {
        rli->report(ERROR_LEVEL, ER_SPECIFIC_ACCESS_DENIED_ERROR,
                    ER_THD(thd, ER_SPECIFIC_ACCESS_DENIED_ERROR),
                    "SUPER, SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN");
        thd->is_slave_error = true;
        goto end;
      }
      thd->variables.pseudo_thread_id = thread_id;  // for temp tables
    }

    thd->set_time(&(common_header->when));
    thd->set_query(query_arg, q_len_arg);
    thd->set_query_for_display(query_arg, q_len_arg);
    thd->set_query_id(next_query_id());
    attach_temp_tables_worker(thd, rli);
    DBUG_PRINT("query", ("%s", thd->query().str));

    DBUG_EXECUTE_IF("simulate_error_in_ddl", error_code = 1051;);

    if (ignored_error_code((expected_error = error_code)) ||
        !unexpected_error_code(expected_error)) {
      if (flags2_inited)
        /*
          all bits of thd->variables.option_bits which are 1 in
          OPTIONS_WRITTEN_TO_BIN_LOG must take their value from flags2.
        */
        thd->variables.option_bits =
            flags2 | (thd->variables.option_bits & ~OPTIONS_WRITTEN_TO_BIN_LOG);
      /*
        else, we are in a 3.23/4.0 binlog; we previously received a
        Rotate_log_event which reset thd->variables.option_bits and sql_mode
        etc, so nothing to do.
      */
      /*
        We do not replicate MODE_NO_DIR_IN_CREATE. That is, if the master is a
        slave which runs with SQL_MODE=MODE_NO_DIR_IN_CREATE, this should not
        force us to ignore the dir too. Imagine you are a ring of machines, and
        one has a disk problem so that you temporarily need
        MODE_NO_DIR_IN_CREATE on this machine; you don't want it to propagate
        elsewhere (you don't want all slaves to start ignoring the dirs).
      */
      if (sql_mode_inited) {
        /*
          All the SQL_MODEs included in 0x1003ff00 were removed in 8.0.5.
          The upgrade procedure clears these bits. So the bits can only be set
          when replicating from an older server. We consider it safe to clear
          the bits, because:
          (1) all these bits except MAXDB has zero impact on replicated
          statements, and MAXDB has minimal impact only;
          (2) the upgrade-pre-check script warns when the bit is set, so we
          assume users have verified that it is safe to ignore the bit.
        */
        if (sql_mode & ~(MODE_ALLOWED_MASK | MODE_IGNORED_MASK)) {
          my_error(ER_UNSUPPORTED_SQL_MODE, MYF(0),
                   sql_mode & ~(MODE_ALLOWED_MASK | MODE_IGNORED_MASK));
          goto compare_errors;
        }
        sql_mode &= MODE_ALLOWED_MASK;
        thd->variables.sql_mode =
            (sql_mode_t)((thd->variables.sql_mode & MODE_NO_DIR_IN_CREATE) |
                         (sql_mode & ~(ulonglong)MODE_NO_DIR_IN_CREATE));
      }
      if (charset_inited) {
        if (rli->cached_charset_compare(charset)) {
          const char *charset_p = charset;  // Avoid type-punning warning.
          /* Verify that we support the charsets found in the event. */
          if (!(thd->variables.character_set_client =
                    get_charset(uint2korr(charset_p), MYF(MY_WME))) ||
              !(thd->variables.collation_connection =
                    get_charset(uint2korr(charset + 2), MYF(MY_WME))) ||
              !(thd->variables.collation_server =
                    get_charset(uint2korr(charset + 4), MYF(MY_WME)))) {
            /*
              We updated the thd->variables with nonsensical values (0). Let's
              set them to something safe (i.e. which avoids crash), and we'll
              stop with EE_UNKNOWN_CHARSET in compare_errors (unless set to
              ignore this error).
            */
            set_slave_thread_default_charset(thd, rli);
            goto compare_errors;
          }
          thd->update_charset();  // for the charset change to take effect
          /*
            We cannot ask for parsing a statement using a character set
            without state_maps (parser internal data).
          */
          if (!thd->variables.character_set_client->state_maps) {
            rli->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                        ER_THD(thd, ER_SLAVE_FATAL_ERROR),
                        "character_set cannot be parsed");
            thd->is_slave_error = true;
            goto end;
          }
          /*
            Reset thd->query_string.cs to the newly set value.
            Note, there is a small flaw here. For a very short time frame
            if the new charset is different from the old charset and
            if another thread executes "SHOW PROCESSLIST" after
            the above thd->set_query() and before this thd->set_query(),
            and if the current query has some non-ASCII characters,
            the another thread may see some '?' marks in the PROCESSLIST
            result. This should be acceptable now. This is a reminder
            to fix this if any refactoring happens here sometime.
          */
          thd->set_query(query_arg, q_len_arg);
          thd->reset_query_for_display();
        }
      }
      if (time_zone_len) {
        String tmp(time_zone_str, time_zone_len, &my_charset_bin);
        if (!(thd->variables.time_zone = my_tz_find(thd, &tmp))) {
          my_error(ER_UNKNOWN_TIME_ZONE, MYF(0), tmp.c_ptr());
          thd->variables.time_zone = global_system_variables.time_zone;
          goto compare_errors;
        }
      }
      if (lc_time_names_number) {
        if (!(thd->variables.lc_time_names =
                  my_locale_by_number(lc_time_names_number))) {
          my_printf_error(ER_UNKNOWN_ERROR, "Unknown locale: '%d'", MYF(0),
                          lc_time_names_number);
          thd->variables.lc_time_names = &my_locale_en_US;
          goto compare_errors;
        }
      } else
        thd->variables.lc_time_names = &my_locale_en_US;
      if (charset_database_number) {
        CHARSET_INFO *cs;
        if (!(cs = get_charset(charset_database_number, MYF(0)))) {
          char buf[20];
          longlong10_to_str(charset_database_number, buf, 10);
          my_error(ER_UNKNOWN_COLLATION, MYF(0), buf);
          goto compare_errors;
        }
        thd->variables.collation_database = cs;
      } else
        thd->variables.collation_database = thd->db_charset;
      if (default_collation_for_utf8mb4_number) {
        CHARSET_INFO *cs;
        if (!(cs = get_charset(default_collation_for_utf8mb4_number, MYF(0)))) {
          char buf[20];
          longlong10_to_str(default_collation_for_utf8mb4_number, buf, 10);
          my_error(ER_UNKNOWN_COLLATION, MYF(0), buf);
          goto compare_errors;
        }
        thd->variables.default_collation_for_utf8mb4 = cs;
      } else
        // The transaction was replicated from a server with utf8mb4_general_ci
        // as default collation for utf8mb4 (versions 5.7-)
        thd->variables.default_collation_for_utf8mb4 =
            &my_charset_utf8mb4_general_ci;

      if (sql_require_primary_key != 0xff &&
          Relay_log_info::PK_CHECK_STREAM ==
              rli->get_require_table_primary_key_check()) {
        DBUG_ASSERT(sql_require_primary_key == 0 ||
                    sql_require_primary_key == 1);
        if (!security_context.skip_priv_checks() &&
            !security_context.has_access({SUPER_ACL}) &&
            !security_context.has_access({"SYSTEM_VARIABLES_ADMIN"}) &&
            !security_context.has_access({"SESSION_VARIABLES_ADMIN"})) {
          rli->report(
              ERROR_LEVEL, ER_SPECIFIC_ACCESS_DENIED_ERROR,
              ER_THD(thd, ER_SPECIFIC_ACCESS_DENIED_ERROR),
              "SUPER, SYSTEM_VARIABLES_ADMIN or SESSION_VARIABLES_ADMIN");
          thd->is_slave_error = true;
          goto end;
        }
        thd->variables.sql_require_primary_key = sql_require_primary_key;
      }

      if (default_table_encryption != 0xff) {
        DBUG_ASSERT(default_table_encryption == 0 ||
                    default_table_encryption == 1);
        if (thd->variables.default_table_encryption !=
                default_table_encryption &&
            !security_context.skip_priv_checks() &&
            !security_context.has_access({SUPER_ACL}) &&
            !security_context.has_access(
                {"SYSTEM_VARIABLES_ADMIN", "TABLE_ENCRYPTION_ADMIN"})) {
          rli->report(
              ERROR_LEVEL, ER_SPECIFIC_ACCESS_DENIED_ERROR,
              ER_THD(thd, ER_SPECIFIC_ACCESS_DENIED_ERROR),
              "SUPER or SYSTEM_VARIABLES_ADMIN and TABLE_ENCRYPTION_ADMIN");
          thd->is_slave_error = true;
          goto end;
        }
        thd->variables.default_table_encryption = default_table_encryption;
      }

      thd->table_map_for_update = (table_map)table_map_for_update;

      LEX_STRING user_lex = LEX_STRING();
      LEX_STRING host_lex = LEX_STRING();
      if (user) {
        user_lex.str = const_cast<char *>(user);
        user_lex.length = strlen(user);
      }
      if (host) {
        host_lex.str = const_cast<char *>(host);
        host_lex.length = strlen(host);
      }
      thd->set_invoker(&user_lex, &host_lex);

      /*
        Flag if we need to rollback the statement transaction on
        slave if it by chance succeeds.
        If we expected a non-zero error code and get nothing and,
        it is a concurrency issue or ignorable issue, effects
        of the statement should be rolled back.
      */
      if (expected_error && (ignored_error_code(expected_error) ||
                             concurrency_error_code(expected_error))) {
        thd->variables.option_bits |= OPTION_MASTER_SQL_ERROR;
      }
      /* Execute the query (note that we bypass dispatch_command()) */
      Parser_state parser_state;
      if (!parser_state.init(thd, thd->query().str, thd->query().length)) {
        DBUG_ASSERT(thd->m_digest == nullptr);
        thd->m_digest = &thd->m_digest_state;
        DBUG_ASSERT(thd->m_statement_psi == nullptr);
        thd->m_statement_psi = MYSQL_START_STATEMENT(
            &thd->m_statement_state, stmt_info_rpl.m_key, thd->db().str,
            thd->db().length, thd->charset(), nullptr);
        THD_STAGE_INFO(thd, stage_starting);

        if (thd->m_digest != nullptr)
          thd->m_digest->reset(thd->m_token_array, max_digest_length);

        struct System_status_var query_start_status;
        struct System_status_var *query_start_status_ptr = nullptr;
        if (opt_log_slow_extra) {
          query_start_status_ptr = &query_start_status;
          query_start_status = thd->status_var;
        }

        /*
          Prevent "hanging" of previous rewritten query in SHOW PROCESSLIST.
        */
        thd->reset_rewritten_query();
        const ulonglong init_timer = my_timer_now();
        ulonglong last_timer = init_timer;
        mysql_parse(thd, &parser_state, &last_timer);

        if (sqlcom_can_generate_row_events(thd->lex->sql_command) &&
            thd->get_row_count_func() > 0) {
          /* At this point we know that the master's binlog_format is NOT ROW
             because we received a Query_log_event which generated rows.
             If master's binlog format was ROW all row generating events would
             be sent as Row_log_events */

          const char *option_name = nullptr;
          if (thd->is_enabled_idempotent_recovery()) {
            option_name = "slave_use_idempotent_for_recovery";
          } else if (is_mts_parallel_type_dependency(rli)) {
            option_name = "slave_parallel_type=DEPENDENCY";
          } else if (rpl_skip_tx_api) {
            option_name = "rpl_skip_tx_api";
          }

          if (option_name != nullptr) {
            rli->report(
                ERROR_LEVEL, ER_MTS_INCONSISTENT_DATA,
                "Master's binlog format is not ROW but %s is enabled on the "
                "slave, this should only be used when master's binlog format "
                "is ROW.",
                option_name);
            thd->is_slave_error = 1;
            goto end;
          }
        }

        enum_sql_command command = thd->lex->sql_command;

        /*
          Transaction isolation level of pure row based replicated transactions
          can be optimized to ISO_READ_COMMITTED by the applier when applying
          the Gtid_log_event.

          If we are applying a statement other than transaction control ones
          after having optimized the transactions isolation level, we must warn
          about the non-standard situation we have found.
        */
        if (is_sbr_logging_format() &&
            thd->variables.transaction_isolation > ISO_READ_COMMITTED &&
            thd->tx_isolation == ISO_READ_COMMITTED) {
          String message;
          message.append(
              "The isolation level for the current transaction "
              "was changed to READ_COMMITTED based on the "
              "assumption that it had only row events and was "
              "not mixed with statements. "
              "However, an unexpected statement was found in "
              "the middle of the transaction."
              "Query: '");
          message.append(thd->query().str);
          message.append("'");
          rli->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                      ER_THD(thd, ER_SLAVE_FATAL_ERROR), message.c_ptr());
          thd->is_slave_error = true;
          goto end;
        }

        /*
          Do not need to increase rewrite_db_filter counter for
          SQLCOM_CREATE_DB, SQLCOM_DROP_DB, SQLCOM_BEGIN and
          SQLCOM_COMMIT.
        */
        if (need_inc_rewrite_db_filter_counter && command != SQLCOM_CREATE_DB &&
            command != SQLCOM_DROP_DB && command != SQLCOM_BEGIN &&
            command != SQLCOM_COMMIT) {
          Rpl_filter *rpl_filter = thd->rli_slave->rpl_filter;
          if (rpl_filter)
            rpl_filter->get_rewrite_db_statistics()->increase_counter();
        }
        /* Finalize server status flags after executing a statement. */
        thd->update_slow_query_status();
        command_slave_seconds += my_timer_since(init_timer);
        log_slow_statement(thd, query_start_status_ptr);
      }

      thd->variables.option_bits &= ~OPTION_MASTER_SQL_ERROR;

      /*
        Resetting the enable_slow_log thd variable.

        We need to reset it back to the opt_log_slow_slave_statements
        value after the statement execution (and slow logging
        is done). It might have changed if the statement was an
        admin statement (in which case, down in mysql_parse execution
        thd->enable_slow_log is set to the value of
        opt_log_slow_admin_statements).
      */
      thd->enable_slow_log = opt_log_slow_slave_statements;
    } else {
      /*
        The query got a really bad error on the master (thread killed etc),
        which could be inconsistent. Parse it to test the table names: if the
        replicate-*-do|ignore-table rules say "this query must be ignored" then
        we exit gracefully; otherwise we warn about the bad error and tell DBA
        to check/fix it.
      */
      if (mysql_test_parse_for_slave(thd))
        clear_all_errors(
            thd, const_cast<Relay_log_info *>(rli)); /* Can ignore query */
      else {
        rli->report(ERROR_LEVEL, ER_ERROR_ON_MASTER,
                    ER_THD(thd, ER_ERROR_ON_MASTER), expected_error,
                    thd->query().str);
        thd->is_slave_error = true;
      }
      goto end;
    }
    /* If the query was not ignored, it is printed to the general log */
    if (!thd->is_error() ||
        thd->get_stmt_da()->mysql_errno() != ER_SLAVE_IGNORED_TABLE) {
      /*
        Log the rewritten query if the query was rewritten
        and the option to log raw was not set.

        There is an assumption here. We assume that query log
        events can never have multi-statement queries, thus the
        parsed statement is the same as the raw one.
      */
      if (opt_general_log_raw || thd->rewritten_query().length() == 0)
        query_logger.general_log_write(thd, COM_QUERY, thd->query().str,
                                       thd->query().length);
      else
        query_logger.general_log_write(thd, COM_QUERY,
                                       thd->rewritten_query().ptr(),
                                       thd->rewritten_query().length());
    }

  compare_errors:
    /* Parser errors shall be ignored when (GTID) skipping statements */
    if (thd->is_error() &&
        thd->get_stmt_da()->mysql_errno() == ER_PARSE_ERROR &&
        gtid_pre_statement_checks(thd) == GTID_STATEMENT_SKIP) {
      thd->get_stmt_da()->reset_diagnostics_area();
    }
    /*
      In the slave thread, we may sometimes execute some DROP / * 40005
      TEMPORARY * / TABLE that come from parts of binlogs (likely if we
      use RESET SLAVE or CHANGE MASTER TO), while the temporary table
      has already been dropped. To ignore such irrelevant "table does
      not exist errors", we silently clear the error if TEMPORARY was used.
    */
    if (thd->lex->sql_command == SQLCOM_DROP_TABLE &&
        thd->lex->drop_temporary && thd->is_error() &&
        thd->get_stmt_da()->mysql_errno() == ER_BAD_TABLE_ERROR &&
        !expected_error) {
      thd->get_stmt_da()->reset_diagnostics_area();
      // Flag drops for error-ignored DDL to advance execution coordinates.
      has_ddl_committed = false;
    }
    /*
      If we expected a non-zero error code, and we don't get the same error
      code, and it should be ignored or is related to a concurrency issue.
    */
    actual_error = thd->is_error() ? thd->get_stmt_da()->mysql_errno() : 0;
    DBUG_PRINT("info", ("expected_error: %d  sql_errno: %d", expected_error,
                        actual_error));

    if (actual_error != 0 && expected_error == actual_error) {
      if (!has_ddl_committed &&                  // Slave didn't commit a DDL
          ddl_xid == binary_log::INVALID_XID &&  // The event was not logged as
                                                 // atomic DDL on master
          !thd->rli_slave->ddl_not_atomic &&  // The DDL was considered atomic
                                              // by the slave
          is_atomic_ddl(thd, true))  // The DDL is atomic for the local server
      {
        thd->get_stmt_da()->reset_diagnostics_area();
        my_error(ER_SLAVE_POSSIBLY_DIVERGED_AFTER_DDL, MYF(0), 0);
        actual_error = ER_SLAVE_POSSIBLY_DIVERGED_AFTER_DDL;
      }
    }

    /*
      If a statement with expected error is received on slave and if the
      statement is not filtered on the slave, only then compare the expected
      error with the actual error that happened on slave.
    */
    if ((expected_error && rli->rpl_filter->db_ok(thd->db().str) &&
         expected_error != actual_error &&
         !concurrency_error_code(expected_error)) &&
        !ignored_error_code(actual_error) &&
        !ignored_error_code(expected_error)) {
      if (!ignored_error_code(ER_INCONSISTENT_ERROR)) {
        rli->report(
            ERROR_LEVEL, ER_INCONSISTENT_ERROR,
            ER_THD(thd, ER_INCONSISTENT_ERROR),
            ER_THD_NONCONST(thd, expected_error), expected_error,
            (actual_error ? thd->get_stmt_da()->message_text() : "no error"),
            actual_error, print_slave_db_safe(db), query_arg);
        thd->is_slave_error = true;
      } else {
        rli->report(INFORMATION_LEVEL, actual_error,
                    "The actual error and expected error on slave are"
                    " different that will result in ER_INCONSISTENT_ERROR but"
                    " that is passed as an argument to slave_skip_errors so no"
                    " error is thrown. "
                    "The expected error was %s with, Error_code: %d. "
                    "The actual error is %s with ",
                    ER_THD_NONCONST(thd, expected_error), expected_error,
                    thd->get_stmt_da()->message_text());
        clear_all_errors(thd, const_cast<Relay_log_info *>(rli));
      }
    }
    /*
      If we get the same error code as expected and it is not a concurrency
      issue, or should be ignored.
    */
    else if ((expected_error == actual_error &&
              !concurrency_error_code(expected_error)) ||
             ignored_error_code(actual_error)) {
      DBUG_PRINT("info", ("error ignored"));
      if (actual_error && ignored_error_code(actual_error)) {
        if (actual_error == ER_SLAVE_IGNORED_TABLE) {
          if (!slave_ignored_err_throttle.log())
            rli->report(INFORMATION_LEVEL, actual_error,
                        "Could not execute %s event. Detailed error: %s;"
                        " Error log throttle is enabled. This error will not be"
                        " displayed for next %lu secs. It will be suppressed",
                        get_type_str(), thd->get_stmt_da()->message_text(),
                        (window_size / 1000000));
        } else
          rli->report(INFORMATION_LEVEL, actual_error,
                      "Could not execute %s event. Detailed error: %s;",
                      get_type_str(), thd->get_stmt_da()->message_text());
      }
      has_ddl_committed = false;  // The same comments as above.
      clear_all_errors(thd, const_cast<Relay_log_info *>(rli));
      thd->killed = THD::NOT_KILLED;
    }
    /*
      Other cases: mostly we expected no error and get one.
    */
    else if (thd->is_slave_error || thd->is_fatal_error()) {
      if (!is_silent_error(thd)) {
        rli->report(ERROR_LEVEL, actual_error,
                    "Error '%s' on query. Default database: '%s'. Query: '%s'",
                    (actual_error ? thd->get_stmt_da()->message_text()
                                  : "unexpected success or fatal error"),
                    print_slave_db_safe(thd->db().str), query_arg);
      }
      thd->is_slave_error = true;
    }

    /*
      TODO: compare the values of "affected rows" around here. Something
      like:
      if ((uint32) affected_in_event != (uint32) affected_on_slave)
      {
      sql_print_error("Slave: did not get the expected number of affected \
      rows running query from master - expected %d, got %d (this numbers \
      should have matched modulo 4294967296).", 0, ...);
      thd->is_slave_error = 1;
      }
      We may also want an option to tell the slave to ignore "affected"
      mismatch. This mismatch could be implemented with a new ER_ code, and
      to ignore it you would use --slave-skip-errors...

      To do the comparison we need to know the value of "affected" which the
      above mysql_parse() computed. And we need to know the value of
      "affected" in the master's binlog. Both will be implemented later. The
      important thing is that we now have the format ready to log the values
      of "affected" in the binlog. So we can release 5.0.0 before effectively
      logging "affected" and effectively comparing it.
    */
  } /* End of if (db_ok(... */

  {
    /**
      The following failure injecion works in cooperation with tests
      setting @@global.debug= 'd,stop_slave_middle_group'.
      The sql thread receives the killed status and will proceed
      to shutdown trying to finish incomplete events group.
    */

    // TODO: address the middle-group killing in MTS case

    DBUG_EXECUTE_IF(
        "stop_slave_middle_group",
        if (strcmp("COMMIT", query) != 0 && strcmp("BEGIN", query) != 0) {
          if (thd->get_transaction()->cannot_safely_rollback(
                  Transaction_ctx::SESSION))
            const_cast<Relay_log_info *>(rli)->abort_slave = 1;
        };);
  }

end:

  if (thd->temporary_tables) detach_temp_tables_worker(thd, rli);
  /*
    Probably we have set thd->query, thd->db, thd->catalog to point to places
    in the data_buf of this event. Now the event is going to be deleted
    probably, so data_buf will be freed, so the thd->... listed above will be
    pointers to freed memory.
    So we must set them to 0, so that those bad pointers values are not later
    used. Note that "cleanup" queries like automatic DROP TEMPORARY TABLE
    don't suffer from these assignments to 0 as DROP TEMPORARY
    TABLE uses the db.table syntax.
  */
  thd->set_catalog(NULL_CSTR);
  thd->set_db(NULL_CSTR); /* will free the current database */
  thd->reset_query();
  thd->lex->sql_command = SQLCOM_END;
  DBUG_PRINT("info", ("end: query= 0"));

  /* Mark the statement completed. */
  MYSQL_END_STATEMENT(thd->m_statement_psi, thd->get_stmt_da());
  thd->m_statement_psi = nullptr;
  thd->m_digest = nullptr;

  /*
    As a disk space optimization, future masters will not log an event for
    LAST_INSERT_ID() if that function returned 0 (and thus they will be able
    to replace the THD::stmt_depends_on_first_successful_insert_id_in_prev_stmt
    variable by (THD->first_successful_insert_id_in_prev_stmt > 0) ; with the
    resetting below we are ready to support that.
  */
  thd->first_successful_insert_id_in_prev_stmt_for_binlog = 0;
  thd->first_successful_insert_id_in_prev_stmt = 0;
  thd->stmt_depends_on_first_successful_insert_id_in_prev_stmt = false;
  free_root(thd->mem_root, MYF(MY_KEEP_PREALLOC));
  return thd->is_slave_error;
}

int Query_log_event::do_update_pos(Relay_log_info *rli) {
  int ret = Log_event::do_update_pos(rli);

  DBUG_EXECUTE_IF(
      "crash_after_commit_and_update_pos", if (!strcmp("COMMIT", query)) {
        sql_print_information("Crashing crash_after_commit_and_update_pos.");
        rli->flush_info(true);
        ha_flush_logs(0);
        DBUG_SUICIDE();
      });

  return ret;
}

Log_event::enum_skip_reason Query_log_event::do_shall_skip(
    Relay_log_info *rli) {
  DBUG_TRACE;
  DBUG_PRINT("debug", ("query: %s; q_len: %d", query, static_cast<int>(q_len)));
  DBUG_ASSERT(query && q_len > 0);

  if (rli->slave_skip_counter > 0) {
    if (strcmp("BEGIN", query) == 0) {
      thd->variables.option_bits |= OPTION_BEGIN;
      return Log_event::continue_group(rli);
    }

    if (strcmp("COMMIT", query) == 0 || strcmp("ROLLBACK", query) == 0) {
      thd->variables.option_bits &= ~OPTION_BEGIN;
      return Log_event::EVENT_SKIP_COUNT;
    }
  }
  Log_event::enum_skip_reason ret = Log_event::do_shall_skip(rli);
  return ret;
}

#endif

/**
   Return the query string pointer (and its size) from a Query log event
   using only the event buffer (we don't instantiate a Query_log_event
   object for this).

   @param buf               Pointer to the event buffer.
   @param length            The size of the event buffer.
   @param fd_event          The description event of the master which logged
                            the event.
   @param[out] query_arg    The pointer to receive the query pointer.

   @return                  The size of the query.
*/
size_t Query_log_event::get_query(const char *buf, size_t length,
                                  const Format_description_event *fd_event,
                                  const char **query_arg) {
  DBUG_ASSERT((Log_event_type)buf[EVENT_TYPE_OFFSET] ==
              binary_log::QUERY_EVENT);

  char db_len;              /* size of db name */
  uint status_vars_len = 0; /* size of status_vars */
  size_t qlen;              /* size of the query */
  int checksum_size = 0;    /* size of trailing checksum */
  const char *end_of_query;

  uint common_header_len = fd_event->common_header_len;
  uint query_header_len =
      fd_event->post_header_len[binary_log::QUERY_EVENT - 1];

  /* Error if the event content is too small */
  if (length < (common_header_len + query_header_len)) goto err;

  /* Skip the header */
  buf += common_header_len;

  /* Check if there are status variables in the event */
  if ((query_header_len - QUERY_HEADER_MINIMAL_LEN) > 0) {
    status_vars_len = uint2korr(buf + Q_STATUS_VARS_LEN_OFFSET);
  }

  /* Check if the event has trailing checksum */
  if (fd_event->footer()->checksum_alg != binary_log::BINLOG_CHECKSUM_ALG_OFF)
    checksum_size = 4;

  db_len = (uchar)buf[Q_DB_LEN_OFFSET];

  /* Error if the event content is too small */
  if (length < (common_header_len + query_header_len + db_len + 1 +
                status_vars_len + checksum_size))
    goto err;

  *query_arg = buf + query_header_len + db_len + 1 + status_vars_len;

  /* Calculate the query length */
  end_of_query = buf +
                 (length - common_header_len) - /* we skipped the header */
                 checksum_size;
  qlen = end_of_query - *query_arg;
  return qlen;

err:
  *query_arg = nullptr;
  return 0;
}

/***************************************************************************
       Format_description_log_event methods
****************************************************************************/

/**
  Format_description_log_event 1st ctor.

    Ctor. Can be used to create the event to write to the binary log (when the
    server starts or when FLUSH LOGS).
*/
Format_description_log_event::Format_description_log_event()
    : Format_description_event(BINLOG_VERSION, ::server_version),
#ifdef MYSQL_SERVER
      Log_event(header(), footer(), Log_event::EVENT_INVALID_CACHE,
                Log_event::EVENT_INVALID_LOGGING)
#else
      Log_event(header(), footer())
#endif
{
  common_header->set_is_valid(true);
}

/**
  The problem with this constructor is that the fixed header may have a
  length different from this version, but we don't know this length as we
  have not read the Format_description_log_event which says it, yet. This
  length is in the post-header of the event, but we don't know where the
  post-header starts.

  So this type of event HAS to:
  - either have the header's length at the beginning (in the header, at a
  fixed position which will never be changed), not in the post-header. That
  would make the header be "shifted" compared to other events.
  - or have a header of size LOG_EVENT_MINIMAL_HEADER_LEN (19), in all future
  versions, so that we know for sure.

  I (Guilhem) chose the 2nd solution. Rotate has the same constraint (because
  it is sent before Format_description_log_event).
*/

Format_description_log_event::Format_description_log_event(
    const char *buf, const Format_description_event *description_event)
    : Format_description_event(buf, description_event),
      Log_event(header(), footer()) {
  DBUG_TRACE;
  if (!is_valid()) return;
  common_header->type_code = binary_log::FORMAT_DESCRIPTION_EVENT;
}

#ifndef MYSQL_SERVER
void Format_description_log_event::print(
    FILE *, PRINT_EVENT_INFO *print_event_info) const {
  DBUG_TRACE;

  IO_CACHE *const head = &print_event_info->head_cache;

  if (!print_event_info->short_form) {
    print_header(head, print_event_info, false);
    my_b_printf(head, "\tStart: binlog v %d, server v %s created ",
                binlog_version, server_version);
    print_timestamp(head, nullptr);
    if (created) my_b_printf(head, " at startup");
    my_b_printf(head, "\n");
    if (common_header->flags & LOG_EVENT_BINLOG_IN_USE_F)
      my_b_printf(head,
                  "# Warning: this binlog is either in use or was not "
                  "closed properly.\n");
  }

  if (is_raft_log_event()) {
    my_b_printf(
        head, "# This binlog was generated when this host was in raft mode.\n");
  }

  if (is_relay_log_event()) {
    my_b_printf(head,
                "# This Format_description_event appears in a relay log "
                "and was generated by the slave thread.\n");
  }

  if (!is_artificial_event() && created) {
#ifdef WHEN_WE_HAVE_THE_RESET_CONNECTION_SQL_COMMAND
    /*
      This is for mysqlbinlog: like in replication, we want to delete the stale
      tmp files left by an unclean shutdown of mysqld (temporary tables)
      and rollback unfinished transaction.
      Probably this can be done with RESET CONNECTION (syntax to be defined).
    */
    my_b_printf(head, "RESET CONNECTION%s\n", print_event_info->delimiter);
#else
    my_b_printf(head, "ROLLBACK%s\n", print_event_info->delimiter);
#endif
  }
  if (temp_buf && print_event_info->base64_output_mode != BASE64_OUTPUT_NEVER &&
      !print_event_info->short_form) {
    /*
      BUG#20980932 MYSQLBINLOG GENERATES 'ROLLBACK' AFTER FD EVENT CAUSING 1782
      ERROR
      Format_description_log_event of relay log is not output as a 'BINLOG'
    */
    if (!is_relay_log_event()) {
      if (print_event_info->base64_output_mode != BASE64_OUTPUT_DECODE_ROWS)
        my_b_printf(head, "BINLOG '\n");
      print_base64(head, print_event_info, false);
    }
    print_event_info->printed_fd_event = true;

    /*
      If --skip-gtids is given, the server when it replays the output
      should generate a new GTID if gtid_mode=ON.  However, when the
      server reads the base64-encoded Format_description_log_event, it
      will cleverly detect that this is a binlog to be replayed, and
      act a little bit like the replication thread, in the following
      sense: if the thread does not see any 'SET GTID_NEXT' statement,
      it will assume the binlog was created by an old server and try
      to preserve transactions as anonymous.  This is the opposite of
      what we want when passing the --skip-gtids flag, so therefore we
      output the following statement.

      The behavior where the client preserves transactions following a
      Format_description_log_event as anonymous was introduced in
      5.6.16.
    */
    if (print_event_info->skip_gtids)
      my_b_printf(head, "/*!50616 SET @@SESSION.GTID_NEXT='AUTOMATIC'*/%s\n",
                  print_event_info->delimiter);
  }
}
#endif /* !MYSQL_SERVER */

#ifdef MYSQL_SERVER
int Format_description_log_event::pack_info(Protocol *protocol) {
  char buf[12 + ST_SERVER_VER_LEN + 14 + 22], *pos;
  pos = my_stpcpy(buf, "Server ver: ");
  pos = my_stpcpy(pos, server_version);
  pos = my_stpcpy(pos, ", Binlog ver: ");
  pos = longlong10_to_str(binlog_version, pos, 10);
  protocol->store_string(buf, (uint)(pos - buf), &my_charset_bin);
  return 0;
}

bool Format_description_log_event::write(Basic_ostream *ostream) {
  bool ret;
  bool no_checksum;
  uchar buff[Binary_log_event::FORMAT_DESCRIPTION_HEADER_LEN +
             BINLOG_CHECKSUM_ALG_DESC_LEN];
  size_t rec_size = sizeof(buff);
  int2store(buff + ST_BINLOG_VER_OFFSET, binlog_version);
  memcpy((char *)buff + ST_SERVER_VER_OFFSET, server_version,
         ST_SERVER_VER_LEN);
  if (!dont_set_created) created = get_time();
  int4store(buff + ST_CREATED_OFFSET, static_cast<uint32>(created));
  buff[ST_COMMON_HEADER_LEN_OFFSET] = LOG_EVENT_HEADER_LEN;

  size_t number_of_events = 0;
  int post_header_len_size = static_cast<int>(post_header_len.size());

  if (post_header_len_size == Binary_log_event::LOG_EVENT_TYPES)
    // Replicating between master and slave with same version.
    // number_of_events will be same as Binary_log_event::LOG_EVENT_TYPES
    number_of_events = Binary_log_event::LOG_EVENT_TYPES;
  else if (post_header_len_size > Binary_log_event::LOG_EVENT_TYPES)
    /*
      Replicating between new master and old slave.
      In that case there won't be any memory issues, as there won't be
      any out of memory read.
    */
    number_of_events = Binary_log_event::LOG_EVENT_TYPES;
  else
    /*
      Replicating between old master and new slave.
      In that case it might lead to different number_of_events on master and
      slave. When the relay log is rotated, the FDE from master is used to
      create the FDE event on slave, which is being written here. In that case
      we might end up reading more bytes as
      post_header_len.size() < Binary_log_event::LOG_EVENT_TYPES;
      casuing memory issues.
    */
    number_of_events = post_header_len_size;

  memcpy((char *)buff + ST_COMMON_HEADER_LEN_OFFSET + 1,
         &post_header_len.front(), number_of_events);
  /*
    if checksum is requested
    record the checksum-algorithm descriptor next to
    post_header_len vector which will be followed by the checksum value.
    Master is supposed to trigger checksum computing by binlog_checksum_options,
    slave does it via marking the event according to
    FD_queue checksum_alg value.
  */
  static_assert(BINLOG_CHECKSUM_ALG_DESC_LEN == 1, "");
#ifndef DBUG_OFF
  common_header->data_written = 0;  // to prepare for need_checksum assert
#endif
  buff[Binary_log_event::FORMAT_DESCRIPTION_HEADER_LEN] =
      need_checksum() ? (uint8)common_footer->checksum_alg
                      : (uint8)binary_log::BINLOG_CHECKSUM_ALG_OFF;
  /*
     FD of checksum-aware server is always checksum-equipped, (V) is in,
     regardless of @@global.binlog_checksum policy.
     Thereby a combination of (A) == 0, (V) != 0 means
     it's the checksum-aware server's FD event that heads checksum-free binlog
     file.
     Here 0 stands for checksumming OFF to evaluate (V) as 0 is that case.
     A combination of (A) != 0, (V) != 0 denotes FD of the checksum-aware server
     heading the checksummed binlog.
     (A), (V) presence in FD of the checksum-aware server makes the event
     1 + 4 bytes bigger comparing to the former FD.
  */

  if ((no_checksum = (common_footer->checksum_alg ==
                      binary_log::BINLOG_CHECKSUM_ALG_OFF))) {
    // Forcing (V) room to fill anyway
    common_footer->checksum_alg = binary_log::BINLOG_CHECKSUM_ALG_CRC32;
  }
  ret = (write_header(ostream, rec_size) ||
         wrapper_my_b_safe_write(ostream, buff, rec_size) ||
         write_footer(ostream));
  if (no_checksum)
    common_footer->checksum_alg = binary_log::BINLOG_CHECKSUM_ALG_OFF;
  return ret;
}

int Format_description_log_event::do_apply_event(Relay_log_info const *rli) {
  int ret = 0;
  DBUG_TRACE;

  /*
    As a transaction NEVER spans on 2 or more binlogs:
    if we have an active transaction at this point, the master died
    while writing the transaction to the binary log, i.e. while
    flushing the binlog cache to the binlog. XA guarantees that master has
    rolled back. So we roll back.
    Note: this event could be sent by the master to inform us of the
    format of its binlog; in other words maybe it is not at its
    original place when it comes to us; we'll know this by checking
    log_pos ("artificial" events have log_pos == 0).
  */
  if (!thd->rli_fake && !is_artificial_event() && created &&
      thd->get_transaction()->is_active(Transaction_ctx::SESSION)) {
    /* This is not an error (XA is safe), just an information */
    rli->report(INFORMATION_LEVEL, 0,
                "Rolling back unfinished transaction (no COMMIT "
                "or ROLLBACK in relay log). A probable cause is that "
                "the master died while writing the transaction to "
                "its binary log, thus rolled back too.");
    const_cast<Relay_log_info *>(rli)->cleanup_context(thd, true);
  }

  /* If this event comes from ourself, there is no cleaning task to perform. */
  if (server_id != (uint32)::server_id) {
    if (created && !thd->variables.require_row_format) {
      ret = close_temporary_tables(thd);
      cleanup_load_tmpdir();
    } else {
      /*
        Set all temporary tables thread references to the current thread
        as they may point to the "old" SQL slave thread in case of its
        restart.
      */
      TABLE *table;
      for (table = thd->temporary_tables; table; table = table->next)
        table->in_use = thd;
    }
  }

  if (!ret) {
    /* Save the information describing this binlog */
    ret = const_cast<Relay_log_info *>(rli)->set_rli_description_event(this);
  }

  return ret;
}

int Format_description_log_event::do_update_pos(Relay_log_info *rli) {
  if (server_id == (uint32)::server_id) {
    /*
      We only increase the relay log position if we are skipping
      events and do not touch any group_* variables, nor flush the
      relay log info.  If there is a crash, we will have to re-skip
      the events again, but that is a minor issue.

      If we do not skip stepping the group log position (and the
      server id was changed when restarting the server), it might well
      be that we start executing at a position that is invalid, e.g.,
      at a Rows_log_event or a Query_log_event preceeded by a
      Intvar_log_event instead of starting at a Table_map_log_event or
      the Intvar_log_event respectively.
     */
    rli->inc_event_relay_log_pos();
    return 0;
  } else {
    return Log_event::do_update_pos(rli);
  }
}

Log_event::enum_skip_reason Format_description_log_event::do_shall_skip(
    Relay_log_info *) {
  return Log_event::EVENT_SKIP_NOT;
}

/**************************************************************************
  Rotate_log_event methods
**************************************************************************/

/*
  Rotate_log_event::pack_info()
*/

int Rotate_log_event::pack_info(Protocol *protocol) {
  char buf1[256], buf[22];
  String tmp(buf1, sizeof(buf1), log_cs);
  tmp.length(0);
  tmp.append(new_log_ident, ident_len);
  tmp.append(STRING_WITH_LEN(";pos="));
  tmp.append(llstr(pos, buf));
  protocol->store_string(tmp.ptr(), tmp.length(), &my_charset_bin);
  return 0;
}
#endif  // MYSQL_SERVER

/*
  Rotate_log_event::print()
*/

#ifndef MYSQL_SERVER
void Rotate_log_event::print(FILE *, PRINT_EVENT_INFO *print_event_info) const {
  char buf[22];
  IO_CACHE *const head = &print_event_info->head_cache;

  if (print_event_info->short_form) return;
  print_header(head, print_event_info, false);
  my_b_printf(head, "\tRotate to ");
  if (new_log_ident)
    my_b_write(head, pointer_cast<const uchar *>(new_log_ident),
               (uint)ident_len);
  my_b_printf(head, "  pos: %s\n", llstr(pos, buf));
}
#endif /* !MYSQL_SERVER */

/*
  Rotate_log_event::Rotate_log_event() (2 constructors)
*/

#ifdef MYSQL_SERVER
Rotate_log_event::Rotate_log_event(const char *new_log_ident_arg,
                                   size_t ident_len_arg, ulonglong pos_arg,
                                   uint flags_arg)
    : binary_log::Rotate_event(new_log_ident_arg, ident_len_arg, flags_arg,
                               pos_arg),
      Log_event(header(), footer(), Log_event::EVENT_NO_CACHE,
                Log_event::EVENT_IMMEDIATE_LOGGING) {
#ifndef DBUG_OFF
  DBUG_TRACE;
#endif
  new_log_ident = new_log_ident_arg;
  pos = pos_arg;
  ident_len = ident_len_arg ? ident_len_arg : (uint)strlen(new_log_ident_arg);
  flags = flags_arg;

#ifndef DBUG_OFF
  char buff[22];
  DBUG_PRINT("enter", ("new_log_ident: %s  pos: %s  flags: %lu",
                       new_log_ident_arg, llstr(pos_arg, buff), (ulong)flags));
#endif
  if (flags & DUP_NAME)
    new_log_ident = my_strndup(key_memory_log_event, new_log_ident_arg,
                               ident_len, MYF(MY_WME));
  common_header->set_is_valid(new_log_ident != nullptr);
  if (flags & RELAY_LOG) set_relay_log_event();
}
#endif  // MYSQL_SERVER

Rotate_log_event::Rotate_log_event(
    const char *buf, const Format_description_event *description_event)
    : binary_log::Rotate_event(buf, description_event),
      Log_event(header(), footer()) {
  DBUG_TRACE;
  if (!is_valid()) return;
  DBUG_PRINT("debug", ("new_log_ident: '%s'", new_log_ident));
}

/*
  Rotate_log_event::write()
*/

#ifdef MYSQL_SERVER
bool Rotate_log_event::write(Basic_ostream *ostream) {
  char buf[Binary_log_event::ROTATE_HEADER_LEN];
  int8store(buf + R_POS_OFFSET, pos);
  return (
      write_header(ostream, Binary_log_event::ROTATE_HEADER_LEN + ident_len) ||
      wrapper_my_b_safe_write(ostream, (uchar *)buf,
                              Binary_log_event::ROTATE_HEADER_LEN) ||
      wrapper_my_b_safe_write(ostream,
                              pointer_cast<const uchar *>(new_log_ident),
                              (uint)ident_len) ||
      write_footer(ostream));
}

int Rotate_log_event::do_apply_event(Relay_log_info const *rli) {
  if (!enable_raft_plugin) return 0;

  // Note: We're using event's future_event_relay_log_pos instead of
  // rli->get_event_relay_log_pos() because rli is only updated in
  // do_update_pos() which is called after applying the event and we might need
  // to use this pos during application (e.g. during commit)
  Relay_log_info *rli_ptr = const_cast<Relay_log_info *>(rli);
  thd->set_trans_relay_log_pos(rli_ptr->get_event_relay_log_name(),
                               future_event_relay_log_pos);
  int64_t term, index;
  thd->get_trans_marker(&term, &index);
  return RUN_HOOK_STRICT(raft_replication, after_commit, (thd));
}

/*
  Got a rotate log event from the master.

  This is mainly used so that we can later figure out the logname and
  position for the master.

  We can't rotate the slave's BINlog as this will cause infinitive rotations
  in a A -> B -> A setup.
  The NOTES below is a wrong comment which will disappear when 4.1 is merged.

  This must only be called from the Slave SQL thread, since it calls
  flush_relay_log_info().

  @retval
    0	ok
*/
int Rotate_log_event::do_update_pos(Relay_log_info *rli) {
  int error = 0;
  DBUG_TRACE;
#ifndef DBUG_OFF
  char buf[32];
#endif

  DBUG_PRINT("info", ("server_id=%lu; ::server_id=%lu", (ulong)this->server_id,
                      (ulong)::server_id));
  DBUG_PRINT("info", ("new_log_ident: %s", this->new_log_ident));
  DBUG_PRINT("info", ("pos: %s", llstr(this->pos, buf)));

  /*
    If we are in a transaction or in a group: the only normal case is
    when the I/O thread was copying a big transaction, then it was
    stopped and restarted: we have this in the relay log:

    BEGIN
    ...
    ROTATE (a fake one)
    ...
    COMMIT or ROLLBACK

    In that case, we don't want to touch the coordinates which
    correspond to the beginning of the transaction.  Starting from
    5.0.0, there also are some rotates from the slave itself, in the
    relay log, which shall not change the group positions.
  */

  /*
    The way we check if SQL thread is currently in a group is different
    for STS and MTS.
  */
  bool in_group = rli->is_parallel_exec()
                      ? (rli->mts_group_status == Relay_log_info::MTS_IN_GROUP)
                      : rli->is_in_group();

  if ((server_id != ::server_id || rli->replicate_same_server_id) &&
      !is_relay_log_event() && !in_group) {
    if (is_mts_parallel_type_logical_clock(rli) && server_id != ::server_id) {
      // force the coordinator to start a new binlog segment.
      static_cast<Mts_submode_logical_clock *>(rli->current_mts_submode)
          ->start_new_group();
    }
    if (rli->is_parallel_exec()) {
      /*
        Rotate events are special events that are handled as a
        synchronization point. For that reason, the checkpoint
        routine is being called here.
      */
      if ((error = mts_checkpoint_routine(rli, false))) goto err;

      /*
        Fix for Bug #97694 at https://bugs.mysql.com/bug.php?id=97694
        It should be checked after calling mts_checkpoint_routine(), because
        that function could be interrupted by kill while 'force' is true.
      */
      if (sql_slave_killed(thd, rli)) {
        LogErr(INFORMATION_LEVEL, ER_RPL_SLAVE_ERROR_READING_RELAY_LOG_EVENTS,
               rli->get_for_channel_str(), "slave SQL thread was killed");
        goto err;
      }
    }

    mysql_mutex_lock(&rli->data_lock);
    DBUG_PRINT("info", ("old group_master_log_name: '%s'  "
                        "old group_master_log_pos: %lu",
                        rli->get_group_master_log_name(),
                        (ulong)rli->get_group_master_log_pos()));

    memcpy(const_cast<char *>(rli->get_group_master_log_name()), new_log_ident,
           ident_len + 1);
    rli->notify_group_master_log_name_update();
    /*
      Execution coordinate update by Rotate itself needs forced flush
      otherwise in crash case MTS won't be able to find the starting point
      for recovery.
      It is safe to update the last executed coordinates because all Worker
      assignments prior to Rotate has been already processed (as well as
      above call to @c mts_checkpoint_routine has harvested their
      contribution to the last executed coordinates).
    */
    if ((error = rli->inc_group_relay_log_pos(
             pos, false /* need_data_lock=false */, true /* force flush */))) {
      mysql_mutex_unlock(&rli->data_lock);
      goto err;
    }

    DBUG_PRINT("info", ("new group_master_log_name: '%s'  "
                        "new group_master_log_pos: %lu",
                        rli->get_group_master_log_name(),
                        (ulong)rli->get_group_master_log_pos()));
    mysql_mutex_unlock(&rli->data_lock);
    if (rli->is_parallel_exec()) {
      bool real_event = server_id && !is_artificial_event();
      rli->reset_notified_checkpoint(
          0, 0, real_event ? common_header->when.tv_sec + (time_t)exec_time : 0,
          real_event);
    }

    /*
      Reset thd->variables.option_bits and sql_mode etc, because this could be
      the signal of a master's downgrade from 5.0 to 4.0. However, no need to
      reset rli_description_event: indeed, if the next master is 5.0
      (even 5.0.1) we will soon get a Format_desc; if the next master is 4.0
      then the events are in the slave's format (conversion).
    */
    set_slave_thread_options(thd);
    set_slave_thread_default_charset(thd, rli);
    thd->variables.sql_mode = global_system_variables.sql_mode;
    thd->variables.auto_increment_increment =
        thd->variables.auto_increment_offset = 1;
    /*
      Rotate_log_events are generated on Slaves with server_id=0
      for all the ignored events, so that the positions in the repository
      is updated properly even for ignored events.

      This kind of Rotate_log_event is generated when

        1) the event is generated on the same host and reached due
           to circular replication (server_id == ::server_id)

        2) the event is from the host which is listed in ignore_server_ids

        3) IO thread is receiving HEARTBEAT event from the master

        4) IO thread is receiving PREVIOUS_GTID_LOG_EVENT from the master.

      We have to free thd's mem_root here after we update the positions
      in the repository table. Otherwise, imagine a situation where
      Slave is keep getting ignored events only and no other (non-ignored)
      events from the Master, Slave never executes free_root (that generally
      happens from Query_log_event::do_apply_event or
      Rows_log_event::do_apply_event when they find end of the group event).
    */
    if (server_id == 0) free_root(thd->mem_root, MYF(MY_KEEP_PREALLOC));
  } else
    rli->inc_event_relay_log_pos();

err:
  return error;
}

Log_event::enum_skip_reason Rotate_log_event::do_shall_skip(
    Relay_log_info *rli) {
  enum_skip_reason reason = Log_event::do_shall_skip(rli);

  switch (reason) {
    case Log_event::EVENT_SKIP_NOT:
    case Log_event::EVENT_SKIP_COUNT:
      return Log_event::EVENT_SKIP_NOT;

    case Log_event::EVENT_SKIP_IGNORE:
      return Log_event::EVENT_SKIP_IGNORE;
  }
  DBUG_ASSERT(0);
  return Log_event::EVENT_SKIP_NOT;  // To keep compiler happy
}

/**************************************************************************
        Intvar_log_event methods
**************************************************************************/

/*
  Intvar_log_event::pack_info()
*/

int Intvar_log_event::pack_info(Protocol *protocol) {
  char buf[256], *pos;
  pos = strmake(buf, (get_var_type_string()).c_str(), sizeof(buf) - 23);
  *pos++ = '=';
  pos = longlong10_to_str(val, pos, -10);
  protocol->store_string(buf, (uint)(pos - buf), &my_charset_bin);
  return 0;
}
#endif  // MYSQL_SERVER

/*
  Intvar_log_event::Intvar_log_event()
*/
Intvar_log_event::Intvar_log_event(
    const char *buf, const Format_description_event *description_event)
    : binary_log::Intvar_event(buf, description_event),
      Log_event(header(), footer()) {
  DBUG_TRACE;
}

/*
  Intvar_log_event::write()
*/

#ifdef MYSQL_SERVER
bool Intvar_log_event::write(Basic_ostream *ostream) {
  uchar buf[9];
  buf[I_TYPE_OFFSET] = (uchar)type;
  int8store(buf + I_VAL_OFFSET, val);
  return (write_header(ostream, sizeof(buf)) ||
          wrapper_my_b_safe_write(ostream, buf, sizeof(buf)) ||
          write_footer(ostream));
}
#endif

/*
  Intvar_log_event::print()
*/

#ifndef MYSQL_SERVER
void Intvar_log_event::print(FILE *, PRINT_EVENT_INFO *print_event_info) const {
  char llbuff[22] = {0};
  const char *msg = nullptr;
  IO_CACHE *const head = &print_event_info->head_cache;

  if (!print_event_info->short_form) {
    print_header(head, print_event_info, false);
    my_b_printf(head, "\tIntvar\n");
  }

  my_b_printf(head, "SET ");
  switch (type) {
    case LAST_INSERT_ID_EVENT:
      msg = "LAST_INSERT_ID";
      break;
    case INSERT_ID_EVENT:
      msg = "INSERT_ID";
      break;
    case INVALID_INT_EVENT:
    default:  // cannot happen
      msg = "INVALID_INT";
      break;
  }
  my_b_printf(head, "%s=%s%s\n", msg, llstr(val, llbuff),
              print_event_info->delimiter);
}
#endif

#if defined(MYSQL_SERVER)

/*
  Intvar_log_event::do_apply_event()
*/

int Intvar_log_event::do_apply_event(Relay_log_info const *rli) {
  /*
    We are now in a statement until the associated query log event has
    been processed.
   */
  const_cast<Relay_log_info *>(rli)->set_flag(Relay_log_info::IN_STMT);

  if (rli->deferred_events_collecting) return rli->deferred_events->add(this);

  switch (type) {
    case LAST_INSERT_ID_EVENT:
      thd->first_successful_insert_id_in_prev_stmt = val;
      break;
    case INSERT_ID_EVENT:
      thd->force_one_auto_inc_interval(val);
      break;
  }
  return 0;
}

int Intvar_log_event::do_update_pos(Relay_log_info *rli) {
  rli->inc_event_relay_log_pos();
  return 0;
}

Log_event::enum_skip_reason Intvar_log_event::do_shall_skip(
    Relay_log_info *rli) {
  /*
    It is a common error to set the slave skip counter to 1 instead of
    2 when recovering from an insert which used a auto increment,
    rand, or user var.  Therefore, if the slave skip counter is 1, we
    just say that this event should be skipped by ignoring it, meaning
    that we do not change the value of the slave skip counter since it
    will be decreased by the following insert event.
  */
  return continue_group(rli);
}

/**************************************************************************
  Rand_log_event methods
**************************************************************************/

int Rand_log_event::pack_info(Protocol *protocol) {
  char buf1[256], *pos;
  pos = my_stpcpy(buf1, "rand_seed1=");
  pos = longlong10_to_str(seed1, pos, 10);
  pos = my_stpcpy(pos, ",rand_seed2=");
  pos = longlong10_to_str(seed2, pos, 10);
  protocol->store_string(buf1, (uint)(pos - buf1), &my_charset_bin);
  return 0;
}
#endif  // MYSQL_SERVER

Rand_log_event::Rand_log_event(
    const char *buf, const Format_description_event *description_event)
    : binary_log::Rand_event(buf, description_event),
      Log_event(header(), footer()) {
  DBUG_TRACE;
}

#ifdef MYSQL_SERVER
bool Rand_log_event::write(Basic_ostream *ostream) {
  uchar buf[16];
  int8store(buf + RAND_SEED1_OFFSET, seed1);
  int8store(buf + RAND_SEED2_OFFSET, seed2);
  return (write_header(ostream, sizeof(buf)) ||
          wrapper_my_b_safe_write(ostream, buf, sizeof(buf)) ||
          write_footer(ostream));
}
#endif

#ifndef MYSQL_SERVER
void Rand_log_event::print(FILE *, PRINT_EVENT_INFO *print_event_info) const {
  IO_CACHE *const head = &print_event_info->head_cache;

  char llbuff[22], llbuff2[22];
  if (!print_event_info->short_form) {
    print_header(head, print_event_info, false);
    my_b_printf(head, "\tRand\n");
  }
  my_b_printf(head, "SET @@RAND_SEED1=%s, @@RAND_SEED2=%s%s\n",
              llstr(seed1, llbuff), llstr(seed2, llbuff2),
              print_event_info->delimiter);
}
#endif /* !MYSQL_SERVER */

#if defined(MYSQL_SERVER)
int Rand_log_event::do_apply_event(Relay_log_info const *rli) {
  /*
    We are now in a statement until the associated query log event has
    been processed.
   */
  const_cast<Relay_log_info *>(rli)->set_flag(Relay_log_info::IN_STMT);

  if (rli->deferred_events_collecting) return rli->deferred_events->add(this);

  thd->rand.seed1 = (ulong)seed1;
  thd->rand.seed2 = (ulong)seed2;
  return 0;
}

int Rand_log_event::do_update_pos(Relay_log_info *rli) {
  rli->inc_event_relay_log_pos();
  return 0;
}

Log_event::enum_skip_reason Rand_log_event::do_shall_skip(Relay_log_info *rli) {
  /*
    It is a common error to set the slave skip counter to 1 instead of
    2 when recovering from an insert which used a auto increment,
    rand, or user var.  Therefore, if the slave skip counter is 1, we
    just say that this event should be skipped by ignoring it, meaning
    that we do not change the value of the slave skip counter since it
    will be decreased by the following insert event.
  */
  return continue_group(rli);
}

/**
   Exec deferred Int-, Rand- and User- var events prefixing
   a Query-log-event event.

   @param thd THD handle

   @return false on success, true if a failure in an event applying occurred.
*/
bool slave_execute_deferred_events(THD *thd) {
  bool res = false;
  Relay_log_info *rli = thd->rli_slave;

  DBUG_ASSERT(rli &&
              (!rli->deferred_events_collecting || rli->deferred_events));

  if (!rli->deferred_events_collecting || rli->deferred_events->is_empty())
    return res;

  res = rli->deferred_events->execute(rli);
  rli->deferred_events->rewind();
  return res;
}

/**************************************************************************
  Xid_log_event methods
**************************************************************************/

int Xid_log_event::pack_info(Protocol *protocol) {
  char buf[128], *pos;
  pos = my_stpcpy(buf, "COMMIT /* xid=");
  pos = longlong10_to_str(xid, pos, 10);
  pos = my_stpcpy(pos, " */");
  protocol->store_string(buf, (uint)(pos - buf), &my_charset_bin);
  return 0;
}
#endif  // MYSQL_SERVER

Xid_log_event::Xid_log_event(const char *buf,
                             const Format_description_event *description_event)
    : binary_log::Xid_event(buf, description_event),
      Xid_apply_log_event(header(), footer()) {
  DBUG_TRACE;
}

#ifdef MYSQL_SERVER
bool Xid_log_event::write(Basic_ostream *ostream) {
  DBUG_EXECUTE_IF("do_not_write_xid", return 0;);
  return (write_header(ostream, sizeof(xid)) ||
          wrapper_my_b_safe_write(ostream, (uchar *)&xid, sizeof(xid)) ||
          write_footer(ostream));
}
#endif

#ifndef MYSQL_SERVER
void Xid_log_event::print(FILE *, PRINT_EVENT_INFO *print_event_info) const {
  IO_CACHE *const head = &print_event_info->head_cache;

  if (!print_event_info->short_form) {
    char buf[64];
    longlong10_to_str(xid, buf, 10);

    print_header(head, print_event_info, false);
    my_b_printf(head, "\tXid = %s\n", buf);
  }
  my_b_printf(head, "COMMIT%s\n", print_event_info->delimiter);
}
#endif /* !MYSQL_SERVER */

#if defined(MYSQL_SERVER)
/**
   The methods combines few commit actions to make it useable
   as in the single- so multi- threaded case.

   @param  thd_arg a pointer to THD handle
   @return false  as success and
           true   as an error
*/

bool Xid_log_event::do_commit(THD *thd_arg) {
  DBUG_EXECUTE_IF("dbug.reached_commit",
                  { DBUG_SET("+d,dbug.enabled_commit"); });
  bool error = trans_commit(thd_arg); /* Automatically rolls back on error. */
  DBUG_EXECUTE_IF("crash_after_apply",
                  sql_print_information("Crashing crash_after_apply.");
                  DBUG_SUICIDE(););
  thd_arg->mdl_context.release_transactional_locks();

  error |= mysql_bin_log.gtid_end_transaction(thd_arg);

  /*
    The parser executing a SQLCOM_COMMIT or SQLCOM_ROLLBACK will reset the
    tx isolation level and access mode when the statement is finishing a
    transaction.

    For replicated workload, when dealing with pure transactional workloads,
    there will be no QUERY(COMMIT) finishing a transaction, but a
    Xid_log_event instead.

    So, if the slave applier changed the current transaction isolation level,
    it needs to be restored to the session default value once the current
    transaction has been committed.
  */
  trans_reset_one_shot_chistics(thd);

  /*
    Increment the global status commit count variable
  */
  if (!error) thd_arg->status_var.com_stat[SQLCOM_COMMIT]++;

  return error;
}

/**
   Worker commits Xid transaction and in case of its transactional
   info table marks the current group as done in the Coordinator's
   Group Assigned Queue.

   @return zero as success or non-zero as an error
*/
int Xid_apply_log_event::do_apply_event_worker(Slave_worker *w) {
  DBUG_TRACE;
  int error = 0;
  bool skipped_commit_pos = true;

  lex_start(thd);
  mysql_reset_thd_for_next_command(thd);
  Slave_committed_queue *coordinator_gaq = w->c_rli->gaq;

  /* For a slave Xid_log_event is COMMIT */
  query_logger.general_log_print(thd, COM_QUERY,
                                 "COMMIT /* implicit, from Xid_log_event */");

  DBUG_PRINT(
      "mts",
      ("do_apply group master %s %llu  group relay %s %llu event %s %llu.",
       w->get_group_master_log_name(), w->get_group_master_log_pos(),
       w->get_group_relay_log_name(), w->get_group_relay_log_pos(),
       w->get_event_relay_log_name(), w->get_event_relay_log_pos()));

  DBUG_EXECUTE_IF("crash_before_update_pos",
                  sql_print_information("Crashing crash_before_update_pos.");
                  DBUG_SUICIDE(););

  ulong gaq_idx = mts_group_idx;
  Slave_job_group *ptr_group = coordinator_gaq->get_job_group(gaq_idx);

  if (!thd->get_transaction()->xid_state()->check_in_xa(false) &&
      w->is_transactional()) {
    /*
      Regular (not XA) transaction updates the transactional info table
      along with the main transaction. Otherwise, the local flag turned
      and given its value the info table is updated after do_commit.
      todo: the flag won't be need upon the full xa crash-safety bug76233
            gets fixed.
    */
    skipped_commit_pos = false;
    if ((error = w->commit_positions(this, ptr_group, w->is_transactional())))
      goto err;
  }

  thd->set_trans_relay_log_pos(w->get_group_relay_log_name(),
                               w->get_group_relay_log_pos());

  DBUG_PRINT(
      "mts",
      ("do_apply group master %s %llu  group relay %s %llu event %s %llu.",
       w->get_group_master_log_name(), w->get_group_master_log_pos(),
       w->get_group_relay_log_name(), w->get_group_relay_log_pos(),
       w->get_event_relay_log_name(), w->get_event_relay_log_pos()));

  DBUG_EXECUTE_IF(
      "crash_after_update_pos_before_apply",
      sql_print_information("Crashing crash_after_update_pos_before_apply.");
      DBUG_SUICIDE(););

  error = do_commit(thd);
  if (error) {
    if (!skipped_commit_pos) w->rollback_positions(ptr_group);
  } else if (skipped_commit_pos)
    error = w->commit_positions(this, ptr_group, w->is_transactional());
err:
  return error;
}

int Xid_apply_log_event::do_apply_event(Relay_log_info const *rli) {
  DBUG_TRACE;
  int error = 0;
  char saved_group_master_log_name[FN_REFLEN];
  char saved_group_relay_log_name[FN_REFLEN];
  volatile my_off_t saved_group_master_log_pos;
  volatile my_off_t saved_group_relay_log_pos;

  char new_group_master_log_name[FN_REFLEN];
  char new_group_relay_log_name[FN_REFLEN];
  volatile my_off_t new_group_master_log_pos;
  volatile my_off_t new_group_relay_log_pos;

  lex_start(thd);
  mysql_reset_thd_for_next_command(thd);
  /*
    Anonymous GTID ownership may be released here if the last
    statement before XID updated a non-transactional table and was
    written to the binary log as a separate transaction (either
    because binlog_format=row or because
    binlog_direct_non_transactional_updates=1).  So we need to
    re-acquire anonymous ownership.
  */
  gtid_reacquire_ownership_if_anonymous(thd);
  Relay_log_info *rli_ptr = const_cast<Relay_log_info *>(rli);

  /* For a slave Xid_log_event is COMMIT */
  query_logger.general_log_print(thd, COM_QUERY,
                                 "COMMIT /* implicit, from Xid_log_event */");

  mysql_mutex_lock(&rli_ptr->data_lock);

  /*
    Save the rli positions. We need them to temporarily reset the positions
    just before the commit.
   */
  strmake(saved_group_master_log_name, rli_ptr->get_group_master_log_name(),
          FN_REFLEN - 1);
  saved_group_master_log_pos = rli_ptr->get_group_master_log_pos();
  strmake(saved_group_relay_log_name, rli_ptr->get_group_relay_log_name(),
          FN_REFLEN - 1);
  saved_group_relay_log_pos = rli_ptr->get_group_relay_log_pos();

  DBUG_PRINT(
      "info",
      ("do_apply group master %s %llu  group relay %s %llu event %s %llu\n",
       rli_ptr->get_group_master_log_name(),
       rli_ptr->get_group_master_log_pos(), rli_ptr->get_group_relay_log_name(),
       rli_ptr->get_group_relay_log_pos(), rli_ptr->get_event_relay_log_name(),
       rli_ptr->get_event_relay_log_pos()));

  DBUG_EXECUTE_IF("crash_before_update_pos",
                  sql_print_information("Crashing crash_before_update_pos.");
                  DBUG_SUICIDE(););

  /*
    We need to update the positions in here to make it transactional.
  */
  rli_ptr->inc_event_relay_log_pos();
  rli_ptr->set_group_relay_log_pos(rli_ptr->get_event_relay_log_pos());
  rli_ptr->set_group_relay_log_name(rli_ptr->get_event_relay_log_name());

  if (common_header->log_pos)  // 3.23 binlogs don't have log_posx
    rli_ptr->set_group_master_log_pos(common_header->log_pos);

  /*
    rli repository being transactional means replication is crash safe.
    Positions are written into transactional tables ahead of commit and the
    changes are made permanent during commit.
    XA transactional does not actually commit so has to defer its flush_info().
   */
  if (!thd->get_transaction()->xid_state()->check_in_xa(false) &&
      rli_ptr->is_transactional()) {
    if ((error = rli_ptr->flush_info(true))) goto err;
  }

  DBUG_PRINT(
      "info",
      ("do_apply group master %s %llu  group relay %s %llu event %s %llu\n",
       rli_ptr->get_group_master_log_name(),
       rli_ptr->get_group_master_log_pos(), rli_ptr->get_group_relay_log_name(),
       rli_ptr->get_group_relay_log_pos(), rli_ptr->get_event_relay_log_name(),
       rli_ptr->get_event_relay_log_pos()));

  DBUG_EXECUTE_IF(
      "crash_after_update_pos_before_apply",
      sql_print_information("Crashing crash_after_update_pos_before_apply.");
      DBUG_SUICIDE(););

  /**
    Commit operation expects the global transaction state variable 'xa_state'to
    be set to 'XA_NOTR'. In order to simulate commit failure we set
    the 'xa_state' to 'XA_IDLE' so that the commit reports 'ER_XAER_RMFAIL'
    error.
   */
  DBUG_EXECUTE_IF("simulate_commit_failure", {
    thd->get_transaction()->xid_state()->set_state(XID_STATE::XA_IDLE);
  });

  /*
    Save the new rli positions. These positions will be set back to group*
    positions on successful completion of the commit operation.
   */
  strmake(new_group_master_log_name, rli_ptr->get_group_master_log_name(),
          FN_REFLEN - 1);
  new_group_master_log_pos = rli_ptr->get_group_master_log_pos();
  strmake(new_group_relay_log_name, rli_ptr->get_group_relay_log_name(),
          FN_REFLEN - 1);
  new_group_relay_log_pos = rli_ptr->get_group_relay_log_pos();
  thd->set_trans_relay_log_pos(rli_ptr->get_group_relay_log_name(),
                               rli_ptr->get_group_relay_log_pos());
  /*
    Rollback positions in memory just before commit. Position values will be
    reset to their new values only on successful commit operation.
   */
  rli_ptr->set_group_master_log_name(saved_group_master_log_name);
  rli_ptr->set_group_master_log_pos(saved_group_master_log_pos);
  rli_ptr->notify_group_master_log_name_update();
  rli_ptr->set_group_relay_log_name(saved_group_relay_log_name);
  rli_ptr->set_group_relay_log_pos(saved_group_relay_log_pos);

  DBUG_PRINT("info", ("Rolling back to group master %s %llu  group relay %s"
                      " %llu\n",
                      rli_ptr->get_group_master_log_name(),
                      rli_ptr->get_group_master_log_pos(),
                      rli_ptr->get_group_relay_log_name(),
                      rli_ptr->get_group_relay_log_pos()));
  mysql_mutex_unlock(&rli_ptr->data_lock);
  error = do_commit(thd);
  mysql_mutex_lock(&rli_ptr->data_lock);
  if (error) {
    rli->report(ERROR_LEVEL, thd->get_stmt_da()->mysql_errno(),
                "Error in Xid_log_event: Commit could not be completed, '%s'",
                thd->get_stmt_da()->message_text());
  } else {
    DBUG_EXECUTE_IF(
        "crash_after_commit_before_update_pos",
        sql_print_information("Crashing "
                              "crash_after_commit_before_update_pos.");
        DBUG_SUICIDE(););
    /* Update positions on successful commit */
    rli_ptr->set_group_master_log_name(new_group_master_log_name);
    rli_ptr->set_group_master_log_pos(new_group_master_log_pos);
    rli_ptr->notify_group_master_log_name_update();
    rli_ptr->set_group_relay_log_name(new_group_relay_log_name);
    rli_ptr->set_group_relay_log_pos(new_group_relay_log_pos);

    DBUG_PRINT("info", ("Updating positions on succesful commit to group master"
                        " %s %llu  group relay %s %llu\n",
                        rli_ptr->get_group_master_log_name(),
                        rli_ptr->get_group_master_log_pos(),
                        rli_ptr->get_group_relay_log_name(),
                        rli_ptr->get_group_relay_log_pos()));

    /*
      For transactional repository the positions are flushed ahead of commit.
      Where as for non transactional rli repository the positions are flushed
      only on succesful commit.
     */
    if (!rli_ptr->is_transactional()) rli_ptr->flush_info(false);
  }
err:
  // This is Bug#24588741 fix:
  if (rli_ptr->is_group_master_log_pos_invalid)
    rli_ptr->is_group_master_log_pos_invalid = false;
  mysql_cond_broadcast(&rli_ptr->data_cond);
  mysql_mutex_unlock(&rli_ptr->data_lock);

  return error;
}

Log_event::enum_skip_reason Xid_apply_log_event::do_shall_skip(
    Relay_log_info *rli) {
  DBUG_TRACE;
  if (rli->slave_skip_counter > 0) {
    thd->variables.option_bits &= ~OPTION_BEGIN;
    return Log_event::EVENT_SKIP_COUNT;
  }
  return Log_event::do_shall_skip(rli);
}

/**************************************************************************
  XA_prepare_log_event methods
**************************************************************************/

int XA_prepare_log_event::pack_info(Protocol *protocol) {
  char buf[ser_buf_size];
  char query[sizeof("XA COMMIT ONE PHASE") + 1 + sizeof(buf)];

  /* RHS of the following assert is unknown to client sources */
  static_assert(ser_buf_size == XID::ser_buf_size, "");
  serialize_xid(buf, my_xid.formatID, my_xid.gtrid_length, my_xid.bqual_length,
                my_xid.data);
  sprintf(query, (one_phase ? "XA COMMIT %s ONE PHASE" : "XA PREPARE %s"), buf);

  protocol->store_string(query, strlen(query), &my_charset_bin);
  return 0;
}

bool XA_prepare_log_event::write(Basic_ostream *ostream) {
  uint8 one_byte = one_phase;
  uchar buf_f[4];
  uchar buf_g[4];
  uchar buf_b[4];
  int4store(buf_f, static_cast<XID *>(xid)->get_format_id());
  int4store(buf_g, static_cast<XID *>(xid)->get_gtrid_length());
  int4store(buf_b, static_cast<XID *>(xid)->get_bqual_length());

  DBUG_ASSERT(xid_bufs_size == sizeof(buf_f) + sizeof(buf_g) + sizeof(buf_b));

  return write_header(ostream,
                      sizeof(one_byte) + xid_bufs_size +
                          static_cast<XID *>(xid)->get_gtrid_length() +
                          static_cast<XID *>(xid)->get_bqual_length()) ||
         wrapper_my_b_safe_write(ostream, &one_byte, sizeof(one_byte)) ||
         wrapper_my_b_safe_write(ostream, buf_f, sizeof(buf_f)) ||
         wrapper_my_b_safe_write(ostream, buf_g, sizeof(buf_g)) ||
         wrapper_my_b_safe_write(ostream, buf_b, sizeof(buf_b)) ||
         wrapper_my_b_safe_write(
             ostream,
             pointer_cast<const uchar *>(static_cast<XID *>(xid)->get_data()),
             static_cast<XID *>(xid)->get_gtrid_length() +
                 static_cast<XID *>(xid)->get_bqual_length()) ||
         write_footer(ostream);
}
#endif  // MYSQL_SERVER

#ifndef MYSQL_SERVER
void XA_prepare_log_event::print(FILE *,
                                 PRINT_EVENT_INFO *print_event_info) const {
  IO_CACHE *const head = &print_event_info->head_cache;
  char buf[ser_buf_size];

  print_header(head, print_event_info, false);
  serialize_xid(buf, my_xid.formatID, my_xid.gtrid_length, my_xid.bqual_length,
                my_xid.data);
  my_b_printf(head, "\tXA PREPARE %s\n", buf);
  my_b_printf(
      head, one_phase ? "XA COMMIT %s ONE PHASE\n%s\n" : "XA PREPARE %s\n%s\n",
      buf, print_event_info->delimiter);
}
#endif /* !MYSQL_SERVER */

#if defined(MYSQL_SERVER)

/**
  Differs from Xid_log_event::do_commit in that it carries out
  XA prepare (not the commit).
  It also can commit on one phase when the event's member @c one_phase
  set to true.

  @param  thd_arg  a pointer to THD handle
  @return false  as success and
          true   as an error
*/

bool XA_prepare_log_event::do_commit(THD *thd_arg) {
  enum_gtid_statement_status state = gtid_pre_statement_checks(thd_arg);
  if (state == GTID_STATEMENT_EXECUTE) {
    if (gtid_pre_statement_post_implicit_commit_checks(thd_arg))
      state = GTID_STATEMENT_CANCEL;
  }
  if (state == GTID_STATEMENT_CANCEL) {
    uint error = thd_arg->get_stmt_da()->mysql_errno();
    DBUG_ASSERT(error != 0);
    thd_arg->rli_slave->report(ERROR_LEVEL, error,
                               "Error executing XA PREPARE event: '%s'",
                               thd_arg->get_stmt_da()->message_text());
    thd_arg->is_slave_error = true;
    return true;
  } else if (state == GTID_STATEMENT_SKIP)
    return false;

  bool error = false;
  xid_t xid;
  xid.set(my_xid.formatID, my_xid.data, my_xid.gtrid_length,
          my_xid.data + my_xid.gtrid_length, my_xid.bqual_length);
  if (!one_phase) {
    /*
      This is XA-prepare branch.
    */
    thd_arg->lex->sql_command = SQLCOM_XA_PREPARE;
    thd_arg->lex->m_sql_cmd = new (thd_arg->mem_root) Sql_cmd_xa_prepare(&xid);
    error = thd_arg->lex->m_sql_cmd->execute(thd_arg);
  } else {
    thd_arg->lex->sql_command = SQLCOM_XA_COMMIT;
    thd_arg->lex->m_sql_cmd =
        new (thd_arg->mem_root) Sql_cmd_xa_commit(&xid, XA_ONE_PHASE);
    error = thd_arg->lex->m_sql_cmd->execute(thd_arg);
  }

  if (!error) error = mysql_bin_log.gtid_end_transaction(thd_arg);

  return error;
}

/**************************************************************************
  User_var_log_event methods
**************************************************************************/

int User_var_log_event::pack_info(Protocol *protocol) {
  char *buf = nullptr;
  char quoted_id[1 + FN_REFLEN * 2 + 2];  // quoted identifier
  size_t id_len =
      my_strmov_quoted_identifier(this->thd, quoted_id, name, name_len);
  quoted_id[id_len] = '\0';
  size_t val_offset = 2 + id_len;
  size_t event_len = val_offset;

  if (is_null) {
    if (!(buf = (char *)my_malloc(key_memory_log_event, val_offset + 5,
                                  MYF(MY_WME))))
      return 1;
    my_stpcpy(buf + val_offset, "NULL");
    event_len = val_offset + 4;
  } else {
    switch (type) {
      case REAL_RESULT:
        double real_val;
        real_val = float8get(val);
        if (!(buf = (char *)my_malloc(key_memory_log_event,
                                      val_offset + MY_GCVT_MAX_FIELD_WIDTH + 1,
                                      MYF(MY_WME))))
          return 1;
        event_len +=
            my_gcvt(real_val, MY_GCVT_ARG_DOUBLE, MY_GCVT_MAX_FIELD_WIDTH,
                    buf + val_offset, nullptr);
        break;
      case INT_RESULT:
        if (!(buf = (char *)my_malloc(key_memory_log_event, val_offset + 22,
                                      MYF(MY_WME))))
          return 1;
        event_len = longlong10_to_str(
                        uint8korr(val), buf + val_offset,
                        ((flags & User_var_log_event::UNSIGNED_F) ? 10 : -10)) -
                    buf;
        break;
      case DECIMAL_RESULT: {
        if (!(buf = (char *)my_malloc(key_memory_log_event,
                                      val_offset + DECIMAL_MAX_STR_LENGTH + 1,
                                      MYF(MY_WME))))
          return 1;
        String str(buf + val_offset, DECIMAL_MAX_STR_LENGTH + 1,
                   &my_charset_bin);
        my_decimal dec;
        binary2my_decimal(E_DEC_FATAL_ERROR, (uchar *)(val + 2), &dec, val[0],
                          val[1]);
        my_decimal2string(E_DEC_FATAL_ERROR, &dec, &str);
        event_len = str.length() + val_offset;
        break;
      }
      case STRING_RESULT:
        /* 15 is for 'COLLATE' and other chars */
        buf = (char *)my_malloc(
            key_memory_log_event,
            event_len + val_len * 2 + 1 + 2 * MY_CS_NAME_SIZE + 15,
            MYF(MY_WME));
        CHARSET_INFO *cs;
        if (!buf) return 1;
        if (!(cs = get_charset(charset_number, MYF(0)))) {
          my_stpcpy(buf + val_offset, "???");
          event_len += 3;
        } else {
          char *p = strxmov(buf + val_offset, "_", cs->csname, " ", NullS);
          p = str_to_hex(p, val, val_len);
          p = strxmov(p, " COLLATE ", cs->name, NullS);
          event_len = p - buf;
        }
        break;
      case ROW_RESULT:
      default:
        DBUG_ASSERT(false);
        return 1;
    }
  }
  buf[0] = '@';
  memcpy(buf + 1, quoted_id, id_len);
  buf[1 + id_len] = '=';
  protocol->store_string(buf, event_len, &my_charset_bin);
  my_free(buf);
  return 0;
}
#endif /* MYSQL_SERVER */

User_var_log_event::User_var_log_event(
    const char *buf, const Format_description_event *description_event)
    : binary_log::User_var_event(buf, description_event),
      Log_event(header(), footer())
#ifdef MYSQL_SERVER
      ,
      deferred(false),
      query_id(0)
#endif
{
  DBUG_TRACE;
}

#ifdef MYSQL_SERVER
bool User_var_log_event::write(Basic_ostream *ostream) {
  char buf[UV_NAME_LEN_SIZE];
  char buf1[UV_VAL_IS_NULL + UV_VAL_TYPE_SIZE + UV_CHARSET_NUMBER_SIZE +
            UV_VAL_LEN_SIZE];
  uchar buf2[std::max(8, DECIMAL_MAX_FIELD_SIZE + 2)], *pos = buf2;
  uint unsigned_len = 0;
  uint buf1_length;
  ulong event_length;

  int4store(buf, name_len);

  if ((buf1[0] = is_null)) {
    buf1_length = 1;
    val_len = 0;  // Length of 'pos'
  } else {
    buf1[1] = type;
    int4store(buf1 + 2, charset_number);

    switch (type) {
      case REAL_RESULT:
        float8store(buf2, *(double *)val);
        break;
      case INT_RESULT:
        int8store(buf2, *(longlong *)val);
        unsigned_len = 1;
        break;
      case DECIMAL_RESULT: {
        my_decimal *dec = (my_decimal *)val;
        dec->sanity_check();
        buf2[0] = (char)(dec->intg + dec->frac);
        buf2[1] = (char)dec->frac;
        decimal2bin(dec, buf2 + 2, buf2[0], buf2[1]);
        val_len = decimal_bin_size(buf2[0], buf2[1]) + 2;
        break;
      }
      case STRING_RESULT:
        pos = (uchar *)val;
        break;
      case ROW_RESULT:
      default:
        DBUG_ASSERT(false);
        return false;
    }
    int4store(buf1 + 2 + UV_CHARSET_NUMBER_SIZE, val_len);
    buf1_length = 10;
  }

  event_length = sizeof(buf) + name_len + buf1_length + val_len + unsigned_len;

  return (write_header(ostream, event_length) ||
          wrapper_my_b_safe_write(ostream, (uchar *)buf, sizeof(buf)) ||
          wrapper_my_b_safe_write(ostream, pointer_cast<const uchar *>(name),
                                  name_len) ||
          wrapper_my_b_safe_write(ostream, (uchar *)buf1, buf1_length) ||
          wrapper_my_b_safe_write(ostream, pos, val_len) ||
          wrapper_my_b_safe_write(ostream, &flags, unsigned_len) ||
          write_footer(ostream));
}
#endif

/*
  User_var_log_event::print()
*/

#ifndef MYSQL_SERVER
void User_var_log_event::print(FILE *,
                               PRINT_EVENT_INFO *print_event_info) const {
  IO_CACHE *const head = &print_event_info->head_cache;
  char quoted_id[1 + NAME_LEN * 2 + 2];  // quoted length of the identifier
  char name_id[NAME_LEN + 1];
  size_t quoted_len = 0;

  if (!print_event_info->short_form) {
    print_header(head, print_event_info, false);
    my_b_printf(head, "\tUser_var\n");
  }
  my_stpcpy(name_id, name);
  name_id[name_len] = '\0';
  my_b_printf(head, "SET @");
  quoted_len =
      my_strmov_quoted_identifier((char *)quoted_id, (const char *)name_id);
  quoted_id[quoted_len] = '\0';
  my_b_write(head, (uchar *)quoted_id, quoted_len);

  if (is_null) {
    my_b_printf(head, ":=NULL%s\n", print_event_info->delimiter);
  } else {
    switch (type) {
      case REAL_RESULT:
        double real_val;
        char real_buf[FMT_G_BUFSIZE(14)];
        real_val = float8get(val);
        sprintf(real_buf, "%.14g", real_val);
        my_b_printf(head, ":=%s%s\n", real_buf, print_event_info->delimiter);
        break;
      case INT_RESULT:
        char int_buf[22];
        longlong10_to_str(
            uint8korr(val), int_buf,
            ((flags & User_var_log_event::UNSIGNED_F) ? 10 : -10));
        my_b_printf(head, ":=%s%s\n", int_buf, print_event_info->delimiter);
        break;
      case DECIMAL_RESULT: {
        char str_buf[DECIMAL_MAX_STR_LENGTH + 1];
        int str_len = sizeof(str_buf);
        int precision = (int)val[0];
        int scale = (int)val[1];
        decimal_digit_t dec_buf[10];
        decimal_t dec;
        dec.len = 10;
        dec.buf = dec_buf;

        bin2decimal((uchar *)val + 2, &dec, precision, scale);
        decimal2string(&dec, str_buf, &str_len);
        my_b_printf(head, ":=%s%s\n", str_buf, print_event_info->delimiter);
        break;
      }
      case STRING_RESULT: {
        /*
          Let's express the string in hex. That's the most robust way. If we
          print it in character form instead, we need to escape it with
          character_set_client which we don't know (we will know it in 5.0, but
          in 4.1 we don't know it easily when we are printing
          User_var_log_event). Explanation why we would need to bother with
          character_set_client (quoting Bar):
          > Note, the parser doesn't switch to another unescaping mode after
          > it has met a character set introducer.
          > For example, if an SJIS client says something like:
          > SET @a= _ucs2 \0a\0b'
          > the string constant is still unescaped according to SJIS, not
          > according to UCS2.
        */
        char *hex_str;
        CHARSET_INFO *cs;

        hex_str = (char *)my_malloc(key_memory_log_event, 2 * val_len + 1 + 2,
                                    MYF(MY_WME));  // 2 hex digits / byte
        if (!hex_str) return;
        str_to_hex(hex_str, val, val_len);
        /*
          For proper behaviour when mysqlbinlog|mysql, we need to explicitely
          specify the variable's collation. It will however cause problems when
          people want to mysqlbinlog|mysql into another server not supporting
          the character set. But there's not much to do about this and it's
          unlikely.
        */
        if (!(cs = get_charset(charset_number, MYF(0))))
          /*
            Generate an unusable command (=> syntax error) is probably the best
            thing we can do here.
          */
          my_b_printf(head, ":=???%s\n", print_event_info->delimiter);
        else
          my_b_printf(head, ":=_%s %s COLLATE `%s`%s\n", cs->csname, hex_str,
                      cs->name, print_event_info->delimiter);
        my_free(hex_str);
      } break;
      case ROW_RESULT:
      default:
        DBUG_ASSERT(false);
        return;
    }
  }
}
#endif

/*
  User_var_log_event::do_apply_event()
*/

#if defined(MYSQL_SERVER)
int User_var_log_event::do_apply_event(Relay_log_info const *rli) {
  DBUG_TRACE;
  Item *it = nullptr;
  CHARSET_INFO *charset;
  query_id_t sav_query_id = 0; /* memorize orig id when deferred applying */

  if (rli->deferred_events_collecting) {
    set_deferred(current_thd->query_id);
    int ret = rli->deferred_events->add(this);
    return ret;
  } else if (is_deferred()) {
    sav_query_id = current_thd->query_id;
    current_thd->query_id = query_id; /* recreating original time context */
  }

  if (!(charset = get_charset(charset_number, MYF(MY_WME)))) {
    rli->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                ER_THD(thd, ER_SLAVE_FATAL_ERROR),
                "Invalid character set for User var event");
    return 1;
  }
  double real_val;
  longlong int_val;

  /*
    We are now in a statement until the associated query log event has
    been processed.
   */
  const_cast<Relay_log_info *>(rli)->set_flag(Relay_log_info::IN_STMT);

  if (is_null) {
    it = new Item_null();
  } else {
    switch (type) {
      case REAL_RESULT:
        if (val_len != 8) {
          rli->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                      ER_THD(thd, ER_SLAVE_FATAL_ERROR),
                      "Invalid variable length at User var event");
          return 1;
        }
        real_val = float8get(val);
        it = new Item_float(real_val, 0);
        val = (char *)&real_val;  // Pointer to value in native format
        val_len = 8;
        break;
      case INT_RESULT:
        if (val_len != 8) {
          rli->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                      ER_THD(thd, ER_SLAVE_FATAL_ERROR),
                      "Invalid variable length at User var event");
          return 1;
        }
        int_val = (longlong)uint8korr(val);
        it = new Item_int(int_val);
        val = (char *)&int_val;  // Pointer to value in native format
        val_len = 8;
        break;
      case DECIMAL_RESULT: {
        if (val_len < 3) {
          rli->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                      ER_THD(thd, ER_SLAVE_FATAL_ERROR),
                      "Invalid variable length at User var event");
          return 1;
        }
        Item_decimal *dec = new Item_decimal((uchar *)val + 2, val[0], val[1]);
        it = dec;
        val = (char *)dec->val_decimal(nullptr);
        val_len = sizeof(my_decimal);
        break;
      }
      case STRING_RESULT:
        it = new Item_string(val, val_len, charset);
        break;
      case ROW_RESULT:
      default:
        DBUG_ASSERT(false);
        return 0;
    }
  }
  Item_func_set_user_var *e =
      new Item_func_set_user_var(Name_string(name, name_len, false), it, false);
  /*
    Item_func_set_user_var can't substitute something else on its place =>
    0 can be passed as last argument (reference on item)

    Fix_fields() can fail, in which case a call of update_hash() might
    crash the server, so if fix fields fails, we just return with an
    error.
  */
  if (e->fix_fields(thd, nullptr)) return 1;

  /*
    A variable can just be considered as a table with
    a single record and with a single column. Thus, like
    a column value, it could always have IMPLICIT derivation.
   */
  e->update_hash(val, val_len, (Item_result)type, charset, DERIVATION_IMPLICIT,
                 (flags & binary_log::User_var_event::UNSIGNED_F));
  if (!is_deferred())
    free_root(thd->mem_root, 0);
  else
    current_thd->query_id = sav_query_id; /* restore current query's context */

  return 0;
}

int User_var_log_event::do_update_pos(Relay_log_info *rli) {
  rli->inc_event_relay_log_pos();
  return 0;
}

Log_event::enum_skip_reason User_var_log_event::do_shall_skip(
    Relay_log_info *rli) {
  /*
    It is a common error to set the slave skip counter to 1 instead
    of 2 when recovering from an insert which used a auto increment,
    rand, or user var.  Therefore, if the slave skip counter is 1, we
    just say that this event should be skipped by ignoring it, meaning
    that we do not change the value of the slave skip counter since it
    will be decreased by the following insert event.
  */
  return continue_group(rli);
}
#endif /* MYSQL_SERVER */

/**************************************************************************
  Unknown_log_event methods
**************************************************************************/

#ifndef MYSQL_SERVER
void Unknown_log_event::print(FILE *,
                              PRINT_EVENT_INFO *print_event_info) const {
  if (print_event_info->short_form) return;
  print_header(&print_event_info->head_cache, print_event_info, false);
  my_b_printf(&print_event_info->head_cache, "\n# %s", "Unknown event\n");
}

/**************************************************************************
        Stop_log_event methods
**************************************************************************/

/*
  Stop_log_event::print()
*/

void Stop_log_event::print(FILE *, PRINT_EVENT_INFO *print_event_info) const {
  if (print_event_info->short_form) return;

  print_header(&print_event_info->head_cache, print_event_info, false);
  my_b_printf(&print_event_info->head_cache, "\tStop\n");
}
#endif /* !MYSQL_SERVER */

#ifdef MYSQL_SERVER
/*
  The master stopped.  We used to clean up all temporary tables but
  this is useless as, as the master has shut down properly, it has
  written all DROP TEMPORARY TABLE (prepared statements' deletion is
  TODO only when we binlog prep stmts).  We used to clean up
  slave_load_tmpdir, but this is useless as it has been cleared at the
  end of LOAD DATA INFILE.  So we have nothing to do here.  The place
  were we must do this cleaning is in
  Start_log_event_v3::do_apply_event(), not here. Because if we come
  here, the master was sane.

  This must only be called from the Slave SQL thread, since it calls
  flush_relay_log_info().
*/
int Stop_log_event::do_update_pos(Relay_log_info *rli) {
  int error_inc = 0;
  int error_flush = 0;
  /*
    We do not want to update master_log pos because we get a rotate event
    before stop, so by now group_master_log_name is set to the next log.
    If we updated it, we will have incorrect master coordinates and this
    could give false triggers in MASTER_POS_WAIT() that we have reached
    the target position when in fact we have not.
    The group position is always unchanged in MTS mode because the event
    is never executed so can't be scheduled to a Worker.
  */
  if ((thd->variables.option_bits & OPTION_BEGIN) || rli->is_parallel_exec())
    rli->inc_event_relay_log_pos();
  else {
    error_inc = rli->inc_group_relay_log_pos(0, true /*need_data_lock=true*/);
    error_flush = rli->flush_info(true);
  }
  return (error_inc || error_flush);
}

/**************************************************************************
        Append_block_log_event methods
**************************************************************************/

/*
  Append_block_log_event ctor
*/

Append_block_log_event::Append_block_log_event(THD *thd_arg, const char *db_arg,
                                               uchar *block_arg,
                                               uint block_len_arg,
                                               bool using_trans)
    : binary_log::Append_block_event(db_arg, block_arg, block_len_arg,
                                     thd_arg->file_id),
      Log_event(thd_arg, 0,
                using_trans ? Log_event::EVENT_TRANSACTIONAL_CACHE
                            : Log_event::EVENT_STMT_CACHE,
                Log_event::EVENT_NORMAL_LOGGING, header(), footer()) {
  common_header->set_is_valid(block != nullptr);
}
#endif  // MYSQL_SERVER

/*
  Append_block_log_event ctor
*/

Append_block_log_event::Append_block_log_event(
    const char *buf, const Format_description_event *description_event)
    : binary_log::Append_block_event(buf, description_event),
      Log_event(header(), footer()) {
  DBUG_TRACE;
}

/*
  Append_block_log_event::write()
*/

#ifdef MYSQL_SERVER
bool Append_block_log_event::write(Basic_ostream *ostream) {
  uchar buf[Binary_log_event::APPEND_BLOCK_HEADER_LEN];
  int4store(buf + AB_FILE_ID_OFFSET, file_id);
  return (write_header(ostream,
                       Binary_log_event::APPEND_BLOCK_HEADER_LEN + block_len) ||
          wrapper_my_b_safe_write(ostream, buf,
                                  Binary_log_event::APPEND_BLOCK_HEADER_LEN) ||
          wrapper_my_b_safe_write(ostream, block, block_len) ||
          write_footer(ostream));
}
#endif

/*
  Append_block_log_event::print()
*/

#ifndef MYSQL_SERVER
void Append_block_log_event::print(FILE *,
                                   PRINT_EVENT_INFO *print_event_info) const {
  if (print_event_info->short_form) return;
  print_header(&print_event_info->head_cache, print_event_info, false);
  my_b_printf(&print_event_info->head_cache,
              "\n#%s: file_id: %d  block_len: %d\n", get_type_str(), file_id,
              block_len);
}
#endif /* !MYSQL_SERVER */

/*
  Append_block_log_event::pack_info()
*/

#if defined(MYSQL_SERVER)
int Append_block_log_event::pack_info(Protocol *protocol) {
  char buf[256];
  size_t length;
  length = snprintf(buf, sizeof(buf), ";file_id=%u;block_len=%u", file_id,
                    block_len);
  protocol->store_string(buf, length, &my_charset_bin);
  return 0;
}

/*
  Append_block_log_event::get_create_or_append()
*/

int Append_block_log_event::get_create_or_append() const {
  return 0; /* append to the file, fail if not exists */
}

/*
  Append_block_log_event::do_apply_event()
*/

int Append_block_log_event::do_apply_event(Relay_log_info const *rli) {
  char fname[FN_REFLEN + TEMP_FILE_MAX_LEN];
  int fd;
  int error = 1;
  DBUG_TRACE;

  Applier_security_context_guard security_context{rli, thd};
  if (DBUG_EVALUATE_IF("skip_the_priv_check_in_begin_load", false, true)) {
    if (!security_context.skip_priv_checks()) {
      if (!security_context.has_access({FILE_ACL})) {
        rli->report_privilege_check_error(
            ERROR_LEVEL,
            Relay_log_info::enum_priv_checks_status::
                LOAD_DATA_EVENT_NOT_ALLOWED,
            false /* to client */);
        return ER_FILE_PRIVILEGE_FOR_REPLICATION_CHECKS;
      }
    }
  }
#ifndef DBUG_OFF
  else {  // Let's ensure that we actually skipped the privilege check since the
          // error code caugth in test scripts would be the same as the no-skip
          // case. Test scripts should wait on the below signal, if
          // `skip_the_priv_check_in_begin_load` has been set.
    const char act[] = "now SIGNAL skipped_the_priv_check_in_begin_load";
    DBUG_ASSERT(!debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
  }
#endif

  THD_STAGE_INFO(thd, stage_making_temp_file_append_before_load_data);
  slave_load_file_stem(fname, file_id, server_id, ".data");
  if (get_create_or_append()) {
    /*
      Usually lex_start() is called by mysql_parse(), but we need it here
      as the present method does not call mysql_parse().
    */
    lex_start(thd);
    mysql_reset_thd_for_next_command(thd);
    /* old copy may exist already */
    mysql_file_delete(key_file_log_event_data, fname, MYF(0));
    DBUG_EXECUTE_IF("simulate_file_create_error_Append_block_event",
                    { strcat(fname, "/"); });
    if ((fd = mysql_file_create(key_file_log_event_data, fname, CREATE_MODE,
                                O_WRONLY | O_EXCL | O_NOFOLLOW, MYF(MY_WME))) <
        0) {
      rli->report(ERROR_LEVEL, thd->get_stmt_da()->mysql_errno(),
                  "Error in %s event: could not create file '%s', '%s'",
                  get_type_str(), fname, thd->get_stmt_da()->message_text());
      goto err;
    }
  } else if ((fd = mysql_file_open(key_file_log_event_data, fname,
                                   O_WRONLY | O_APPEND | O_NOFOLLOW,
                                   MYF(MY_WME))) < 0) {
    rli->report(ERROR_LEVEL, thd->get_stmt_da()->mysql_errno(),
                "Error in %s event: could not open file '%s', '%s'",
                get_type_str(), fname, thd->get_stmt_da()->message_text());
    goto err;
  }
  DBUG_EXECUTE_IF("remove_slave_load_file_before_write",
                  { my_delete_allow_opened(fname, MYF(0)); });

  DBUG_EXECUTE_IF("simulate_file_write_error_Append_block_event",
                  { mysql_file_close(fd, MYF(0)); });
  if (mysql_file_write(fd, block, block_len, MYF(MY_WME + MY_NABP))) {
    rli->report(ERROR_LEVEL, thd->get_stmt_da()->mysql_errno(),
                "Error in %s event: write to '%s' failed, '%s'", get_type_str(),
                fname, thd->get_stmt_da()->message_text());
    goto err;
  }
  error = 0;

err:
  if (fd >= 0) mysql_file_close(fd, MYF(0));
  return error;
}

/**************************************************************************
        Delete_file_log_event methods
**************************************************************************/

/*
  Delete_file_log_event ctor
*/

Delete_file_log_event::Delete_file_log_event(THD *thd_arg, const char *db_arg,
                                             bool using_trans)
    : binary_log::Delete_file_event(thd_arg->file_id, db_arg),
      Log_event(thd_arg, 0,
                using_trans ? Log_event::EVENT_TRANSACTIONAL_CACHE
                            : Log_event::EVENT_STMT_CACHE,
                Log_event::EVENT_NORMAL_LOGGING, header(), footer()) {
  common_header->set_is_valid(file_id != 0);
}
#endif  // MYSQL_SERVER

/*
  Delete_file_log_event ctor
*/

Delete_file_log_event::Delete_file_log_event(
    const char *buf, const Format_description_event *description_event)
    : binary_log::Delete_file_event(buf, description_event),
      Log_event(header(), footer()) {
  DBUG_TRACE;
}

/*
  Delete_file_log_event::write()
*/

#ifdef MYSQL_SERVER
bool Delete_file_log_event::write(Basic_ostream *ostream) {
  uchar buf[Binary_log_event::DELETE_FILE_HEADER_LEN];
  int4store(buf + DF_FILE_ID_OFFSET, file_id);
  return (write_header(ostream, sizeof(buf)) ||
          wrapper_my_b_safe_write(ostream, buf, sizeof(buf)) ||
          write_footer(ostream));
}
#endif

/*
  Delete_file_log_event::print()
*/

#ifndef MYSQL_SERVER
void Delete_file_log_event::print(FILE *,
                                  PRINT_EVENT_INFO *print_event_info) const {
  if (print_event_info->short_form) return;
  print_header(&print_event_info->head_cache, print_event_info, false);
  my_b_printf(&print_event_info->head_cache, "\n#Delete_file: file_id=%u\n",
              file_id);
}
#endif /* !MYSQL_SERVER */

/*
  Delete_file_log_event::pack_info()
*/

#if defined(MYSQL_SERVER)
int Delete_file_log_event::pack_info(Protocol *protocol) {
  char buf[64];
  size_t length;
  length = snprintf(buf, sizeof(buf), ";file_id=%u", (uint)file_id);
  protocol->store_string(buf, length, &my_charset_bin);
  return 0;
}

/*
  Delete_file_log_event::do_apply_event()
*/

int Delete_file_log_event::do_apply_event(Relay_log_info const *rli) {
  char fname[FN_REFLEN + TEMP_FILE_MAX_LEN];
  lex_start(thd);

  Applier_security_context_guard security_context{rli, thd};
  if (!security_context.skip_priv_checks()) {
    if (!security_context.has_access({FILE_ACL})) {
      rli->report_privilege_check_error(
          ERROR_LEVEL,
          Relay_log_info::enum_priv_checks_status::LOAD_DATA_EVENT_NOT_ALLOWED,
          false /* to client */);
      return ER_FILE_PRIVILEGE_FOR_REPLICATION_CHECKS;
    }
  }

  mysql_reset_thd_for_next_command(thd);
  char *ext = slave_load_file_stem(fname, file_id, server_id, ".data");
  mysql_file_delete(key_file_log_event_data, fname, MYF(MY_WME));
  my_stpcpy(ext, ".info");
  mysql_file_delete(key_file_log_event_info, fname, MYF(MY_WME));
  return 0;
}

/**************************************************************************
        Begin_load_query_log_event methods
**************************************************************************/

Begin_load_query_log_event::Begin_load_query_log_event(THD *thd_arg,
                                                       const char *db_arg,
                                                       uchar *block_arg,
                                                       uint block_len_arg,
                                                       bool using_trans)
    : binary_log::Append_block_event(db_arg, block_arg, block_len_arg,
                                     thd_arg->file_id),
      Append_block_log_event(thd_arg, db_arg, block_arg, block_len_arg,
                             using_trans),
      binary_log::Begin_load_query_event() {
  common_header->type_code = binary_log::BEGIN_LOAD_QUERY_EVENT;
  file_id = thd_arg->file_id = mysql_bin_log.next_file_id();
}
#endif  // MYSQL_SERVER

Begin_load_query_log_event::Begin_load_query_log_event(
    const char *buf, const Format_description_event *desc_event)
    : binary_log::Append_block_event(buf, desc_event),
      Append_block_log_event(buf, desc_event),
      binary_log::Begin_load_query_event(buf, desc_event) {
  DBUG_TRACE;
}

#if defined(MYSQL_SERVER)
int Begin_load_query_log_event::get_create_or_append() const {
  return 1; /* create the file */
}

Log_event::enum_skip_reason Begin_load_query_log_event::do_shall_skip(
    Relay_log_info *rli) {
  /*
    If the slave skip counter is 1, then we should not start executing
    on the next event.
  */
  return continue_group(rli);
}

/**************************************************************************
        Execute_load_query_log_event methods
**************************************************************************/

Execute_load_query_log_event::Execute_load_query_log_event(
    THD *thd_arg, const char *query_arg, ulong query_length_arg,
    uint fn_pos_start_arg, uint fn_pos_end_arg,
    binary_log::enum_load_dup_handling dup_handling_arg, bool using_trans,
    bool immediate, bool suppress_use, int errcode)
    : binary_log::Query_event(
          query_arg, thd_arg->catalog().str, thd_arg->db().str,
          query_length_arg, thd_arg->thread_id(), thd_arg->variables.sql_mode,
          thd_arg->variables.auto_increment_increment,
          thd_arg->variables.auto_increment_offset,
          thd_arg->variables.lc_time_names->number,
          (ulonglong)thd_arg->table_map_for_update, errcode),
      Query_log_event(thd_arg, query_arg, query_length_arg, using_trans,
                      immediate, suppress_use, errcode),
      binary_log::Execute_load_query_event(thd_arg->file_id, fn_pos_start_arg,
                                           fn_pos_end_arg, dup_handling_arg) {
  common_header->set_is_valid(Query_log_event::is_valid() && file_id != 0);
  common_header->type_code = binary_log::EXECUTE_LOAD_QUERY_EVENT;
}
#endif /* MYSQL_SERVER */

Execute_load_query_log_event::Execute_load_query_log_event(
    const char *buf, const Format_description_event *desc_event)
    : binary_log::Query_event(buf, desc_event,
                              binary_log::EXECUTE_LOAD_QUERY_EVENT),
      Query_log_event(buf, desc_event, binary_log::EXECUTE_LOAD_QUERY_EVENT),
      binary_log::Execute_load_query_event(buf, desc_event) {
  DBUG_TRACE;
  if (!is_valid()) return;
  if (!Query_log_event::is_valid()) {
    // clear all the variables set in execute_load_query_event
    file_id = 0;
    fn_pos_start = 0;
    fn_pos_end = 0;
    dup_handling = binary_log::LOAD_DUP_ERROR;
  }
  common_header->set_is_valid(Query_log_event::is_valid() && file_id != 0);
}

ulong Execute_load_query_log_event::get_post_header_size_for_derived() {
  return Binary_log_event::EXECUTE_LOAD_QUERY_EXTRA_HEADER_LEN;
}

#ifdef MYSQL_SERVER
bool Execute_load_query_log_event::write_post_header_for_derived(
    Basic_ostream *ostream) {
  uchar buf[Binary_log_event::EXECUTE_LOAD_QUERY_EXTRA_HEADER_LEN];
  int4store(buf, file_id);
  int4store(buf + 4, fn_pos_start);
  int4store(buf + 4 + 4, fn_pos_end);
  *(buf + 4 + 4 + 4) = (uchar)dup_handling;

  return wrapper_my_b_safe_write(
      ostream, buf, Binary_log_event::EXECUTE_LOAD_QUERY_EXTRA_HEADER_LEN);
}
#endif

#ifndef MYSQL_SERVER
void Execute_load_query_log_event::print(
    FILE *file, PRINT_EVENT_INFO *print_event_info) const {
  print(file, print_event_info, nullptr);
}

/**
  Prints the query as LOAD DATA LOCAL and with rewritten filename.
*/
void Execute_load_query_log_event::print(FILE *,
                                         PRINT_EVENT_INFO *print_event_info,
                                         const char *local_fname) const {
  IO_CACHE *const head = &print_event_info->head_cache;

  print_query_header(head, print_event_info);
  /**
    reduce the size of io cache so that the write function is called
    for every call to my_b_printf().
   */
  DBUG_EXECUTE_IF("simulate_execute_event_write_error", {
    head->write_pos = head->write_end;
    DBUG_SET("+d,simulate_file_write_error");
  });

  if (local_fname) {
    my_b_write(head, pointer_cast<const uchar *>(query), fn_pos_start);
    my_b_printf(head, " LOCAL INFILE ");
    pretty_print_str(head, local_fname, strlen(local_fname));

    if (dup_handling == binary_log::LOAD_DUP_REPLACE)
      my_b_printf(head, " REPLACE");
    my_b_printf(head, " INTO");
    my_b_write(head, pointer_cast<const uchar *>(query) + fn_pos_end,
               q_len - fn_pos_end);
    my_b_printf(head, "\n%s\n", print_event_info->delimiter);
  } else {
    my_b_write(head, pointer_cast<const uchar *>(query), q_len);
    my_b_printf(head, "\n%s\n", print_event_info->delimiter);
  }

  if (!print_event_info->short_form)
    my_b_printf(head, "# file_id: %d \n", file_id);
}
#endif

#if defined(MYSQL_SERVER)
int Execute_load_query_log_event::pack_info(Protocol *protocol) {
  char *buf, *pos;
  if (!(buf = (char *)my_malloc(key_memory_log_event,
                                9 + (db_len * 2) + 2 + q_len + 10 + 21,
                                MYF(MY_WME))))
    return 1;
  pos = buf;
  if (db && db_len) {
    /*
      Statically allocates room to store '\0' and an identifier
      that may have NAME_LEN * 2 due to quoting and there are
      two quoting characters that wrap them.
    */
    char quoted_db[1 + NAME_LEN * 2 + 2];  // quoted length of the identifier
    size_t size = 0;
    size = my_strmov_quoted_identifier(this->thd, quoted_db, db, 0);
    pos = my_stpcpy(buf, "use ");
    memcpy(pos, quoted_db, size);
    pos = my_stpcpy(pos + size, "; ");
  }
  if (query && q_len) {
    memcpy(pos, query, q_len);
    pos += q_len;
  }
  pos = my_stpcpy(pos, " ;file_id=");
  pos = longlong10_to_str(file_id, pos, 10);
  protocol->store_string(buf, pos - buf, &my_charset_bin);
  my_free(buf);
  return 0;
}

int Execute_load_query_log_event::do_apply_event(Relay_log_info const *rli) {
  char *p;
  char *buf;
  char *fname;
  char *fname_end;
  int error;

  Applier_security_context_guard security_context{rli, thd};
  if (!security_context.skip_priv_checks()) {
    if (!security_context.has_access({FILE_ACL})) {
      rli->report_privilege_check_error(
          ERROR_LEVEL,
          Relay_log_info::enum_priv_checks_status::LOAD_DATA_EVENT_NOT_ALLOWED,
          false /* to client */);
      return ER_FILE_PRIVILEGE_FOR_REPLICATION_CHECKS;
    }
  }

  buf = (char *)my_malloc(key_memory_log_event,
                          q_len + 1 - (fn_pos_end - fn_pos_start) +
                              (FN_REFLEN + TEMP_FILE_MAX_LEN) + 10 + 8 + 5,
                          MYF(MY_WME));

  DBUG_EXECUTE_IF("LOAD_DATA_INFILE_has_fatal_error", my_free(buf);
                  buf = nullptr;);

  /* Replace filename and LOCAL keyword in query before executing it */
  if (buf == nullptr) {
    rli->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                ER_THD(thd, ER_SLAVE_FATAL_ERROR), "Not enough memory");
    return 1;
  }

  p = buf;
  memcpy(p, query, fn_pos_start);
  p += fn_pos_start;
  fname = (p = strmake(p, STRING_WITH_LEN(" INFILE \'")));
  p = slave_load_file_stem(p, file_id, server_id, ".data");
  fname_end = p = strend(p);  // Safer than p=p+5
  *(p++) = '\'';
  switch (dup_handling) {
    case binary_log::LOAD_DUP_IGNORE:
      p = strmake(p, STRING_WITH_LEN(" IGNORE"));
      break;
    case binary_log::LOAD_DUP_REPLACE:
      p = strmake(p, STRING_WITH_LEN(" REPLACE"));
      break;
    default:
      /* Ordinary load data */
      break;
  }
  p = strmake(p, STRING_WITH_LEN(" INTO "));
  p = strmake(p, query + fn_pos_end, q_len - fn_pos_end);

  error = Query_log_event::do_apply_event(rli, buf, p - buf);

  /* Forging file name for deletion in same buffer */
  *fname_end = 0;

  /*
    If there was an error the slave is going to stop, leave the
    file so that we can re-execute this event at START SLAVE.
  */
  if (!error) mysql_file_delete(key_file_log_event_data, fname, MYF(MY_WME));

  my_free(buf);
  return error;
}

int Execute_load_query_log_event::do_apply_event_worker(Slave_worker *w) {
  return do_apply_event(w);
}

/*****************************************************************************
   Load_query_generator is used to generate the LOAD DATA statement for binlog
******************************************************************************/

Load_query_generator::Load_query_generator(THD *thd_arg, const sql_exchange *ex,
                                           const char *db_arg,
                                           const char *table_name_arg,
                                           bool is_concurrent_arg, bool replace,
                                           bool ignore, bool compressed_arg)
    : str((char *)buf, BUF_SIZE, &my_charset_bin),
      thd(thd_arg),
      sql_ex(ex),
      db(db_arg),
      table_name(table_name_arg ? table_name_arg : ""),
      fname(ex->file_name),
      is_concurrent(is_concurrent_arg),
      compressed(compressed_arg),
      has_replace(replace),
      has_ignore(ignore) {
  str.length(0);
}

const String *Load_query_generator::generate(size_t *fn_start, size_t *fn_end) {
  DBUG_ASSERT(thd->lex->sql_command == SQLCOM_LOAD);
  auto cmd = down_cast<Sql_cmd_load_table *>(thd->lex->m_sql_cmd);

  str.append("LOAD DATA ");

  if (is_concurrent) str.append("CONCURRENT ");

  if (fn_start) *fn_start = str.length() - 1;

  if (cmd->m_is_local_file) str.append("LOCAL ");
  str.append("INFILE ");
  pretty_print_str(&str, fname, strlen(fname));
  str.append(" ");

  if (has_replace)
    str.append("REPLACE ");
  else if (has_ignore)
    str.append("IGNORE ");

  str.append("INTO");
  if (fn_end) *fn_end = str.length();

  str.append(" TABLE ");
  str.append(table_name);

  if (compressed) str.append(" COMPRESSED ");

  if (sql_ex->cs != nullptr) {
    str.append(" CHARACTER SET ");
    str.append(sql_ex->cs->csname);
  }

  /* We have to create all optional fields as the default is not empty */
  str.append(" FIELDS TERMINATED BY ");
  pretty_print_str(&str, sql_ex->field.field_term);

  if (sql_ex->field.opt_enclosed) str.append(" OPTIONALLY ");
  str.append(" ENCLOSED BY ");
  pretty_print_str(&str, sql_ex->field.enclosed);

  str.append(" ESCAPED BY ");
  pretty_print_str(&str, sql_ex->field.escaped);

  str.append(" LINES TERMINATED BY ");
  pretty_print_str(&str, sql_ex->line.line_term);
  if (sql_ex->line.line_start->length() > 0) {
    str.append(" STARTING BY ");
    pretty_print_str(&str, sql_ex->line.line_start);
  }

  if (sql_ex->skip_lines > 0) {
    str.append(" IGNORE ");
    str.append_ulonglong(sql_ex->skip_lines);
    str.append(" LINES ");
  }

  /* prepare fields-list */
  if (!cmd->m_opt_fields_or_vars.is_empty()) {
    List_iterator<Item> li(cmd->m_opt_fields_or_vars);
    Item *item;
    str.append(" (");

    while ((item = li++)) {
      if (item->type() == Item::FIELD_ITEM || item->type() == Item::REF_ITEM)
        append_identifier(thd, &str, item->item_name.ptr(),
                          strlen(item->item_name.ptr()));
      else
        item->print(thd, &str, QT_ORDINARY);
      str.append(", ");
    }
    // remvoe the last ", "
    str.length(str.length() - 2);
    str.append(')');
  }

  if (!cmd->m_opt_set_fields.is_empty()) {
    List_iterator<Item> lu(cmd->m_opt_set_fields);
    List_iterator<String> ls(*cmd->m_opt_set_expr_strings);
    Item *item;

    str.append(" SET ");

    while ((item = lu++)) {
      String *s = ls++;

      append_identifier(thd, &str, item->item_name.ptr(),
                        strlen(item->item_name.ptr()));
      str.append(*s);
      str.append(", ");
    }
    // remvoe the last ", "
    str.length(str.length() - 2);
  }

  return &str;
}

#endif  // MYSQL_SERVER
#ifndef DBUG_OFF
#ifdef MYSQL_SERVER
static uchar dbug_extra_row_ndb_info_val = 0;

/**
   set_extra_data

   Called during self-test to generate various
   self-consistent binlog row event extra
   thread data structures which can be checked
   when reading the binlog.

   @param arr  Buffer to use
*/
static const uchar *set_extra_data(uchar *arr) {
  uchar val = (dbug_extra_row_ndb_info_val++) %
              (EXTRA_ROW_INFO_MAX_PAYLOAD + 1); /* 0 .. MAX_PAYLOAD + 1 */
  arr[EXTRA_ROW_INFO_LEN_OFFSET] = val + EXTRA_ROW_INFO_HEADER_LENGTH;
  arr[EXTRA_ROW_INFO_FORMAT_OFFSET] = val;
  for (uchar i = 0; i < val; i++) arr[EXTRA_ROW_INFO_HEADER_LENGTH + i] = val;

  return arr;
}

#endif  // #ifdef MYSQL_SERVER

/**
   check_extra_row_ndb_info

   Called during self-test to check that
   binlog row event extra data is self-
   consistent as defined by the set_extra_data
   function above.

   Will assert(false) if not.

   @param extra_row_ndb_info
*/
static void check_extra_row_ndb_info(uchar *extra_row_ndb_info) {
  assert(extra_row_ndb_info);
  size_t len = extra_row_ndb_info[EXTRA_ROW_INFO_LEN_OFFSET];
  size_t val = len - EXTRA_ROW_INFO_HEADER_LENGTH;
  assert(extra_row_ndb_info[EXTRA_ROW_INFO_FORMAT_OFFSET] == val);
  for (size_t i = 0; i < val; i++) {
    assert(extra_row_ndb_info[EXTRA_ROW_INFO_HEADER_LENGTH + i] == val);
  }
}

#endif  // #ifndef DBUG_OFF

int get_rpl_part_id(partition_info *part_info) {
  uint32_t part_id = binary_log::Rows_event::Extra_row_info::UNDEFINED;
  longlong func_value;
  if (part_info != nullptr) {
    part_info->get_partition_id(part_info, &part_id, &func_value);
  }
  return static_cast<int>(part_id);
}

/**************************************************************************
        Rows_log_event member functions
**************************************************************************/

#ifdef MYSQL_SERVER
Rows_log_event::Rows_log_event(THD *thd_arg, TABLE *tbl_arg,
                               const Table_id &tid, MY_BITMAP const *cols,
                               bool using_trans, Log_event_type event_type,
                               const unsigned char *extra_row_ndb_info)
    : binary_log::Rows_event(event_type),
      Log_event(thd_arg, 0,
                using_trans ? Log_event::EVENT_TRANSACTIONAL_CACHE
                            : Log_event::EVENT_STMT_CACHE,
                Log_event::EVENT_NORMAL_LOGGING, header(), footer()),
      m_curr_row(nullptr),
      m_curr_row_end(nullptr),
      m_key(nullptr),
      m_key_info(nullptr),
      m_distinct_keys(Key_compare(&m_key_info)),
      m_distinct_key_spare_buf(nullptr),
      m_fields{tbl_arg, Replicated_columns_view::OUTBOUND},
      master_had_triggers(false) {
  DBUG_TRACE;
  common_header->type_code = event_type;
  m_row_count = 0;
  m_table_id = tid;
  m_width = tbl_arg ? this->m_fields.filtered_size() : 1;
  m_rows_buf = nullptr;
  m_rows_cur = nullptr;
  m_rows_end = nullptr;
  m_flags = 0;
  m_type = event_type;

  DBUG_ASSERT(tbl_arg && tbl_arg->s && tid.is_valid());

  if (thd_arg->variables.option_bits & OPTION_NO_FOREIGN_KEY_CHECKS)
    set_flags(NO_FOREIGN_KEY_CHECKS_F);
  if (thd_arg->variables.option_bits & OPTION_RELAXED_UNIQUE_CHECKS)
    set_flags(RELAXED_UNIQUE_CHECKS_F);
#ifndef DBUG_OFF
  uchar extra_data[255];
  DBUG_EXECUTE_IF("extra_row_ndb_info_set",
                  /* Set extra row data to a known value */
                  extra_row_ndb_info = set_extra_data(extra_data););
#endif
  partition_info *part_info = tbl_arg->part_info;
  auto part_id = get_rpl_part_id(part_info);
  if (part_id != binary_log::Rows_event::Extra_row_info::UNDEFINED) {
    m_extra_row_info.set_partition_id(part_id);
  }
  /* Copy Extra ndb data from thd into new event */
  if (extra_row_ndb_info) {
    /* Copy Extra data from thd into new event */
    int extra_row_ndb_info_len = extra_row_ndb_info[EXTRA_ROW_INFO_LEN_OFFSET];
    DBUG_ASSERT(extra_row_ndb_info_len >= EXTRA_ROW_INFO_HEADER_LENGTH);

    m_extra_row_info.set_ndb_info(extra_row_ndb_info, extra_row_ndb_info_len);
  }

  /* if bitmap_init fails, caught in is_valid() */
  if (likely(!bitmap_init(&m_cols,
                          m_width <= sizeof(m_bitbuf) * 8 ? m_bitbuf : nullptr,
                          m_width))) {
    /* Cols can be zero if this is a dummy binrows event */
    if (likely(cols != nullptr)) {
      memcpy(m_cols.bitmap, cols->bitmap, no_bytes_in_map(cols));
      create_last_word_mask(&m_cols);
    }
  } else {
    // Needed because bitmap_init() does not set it to null on failure
    m_cols.bitmap = nullptr;
  }

  if (bitmap_init(&write_set_backup, nullptr, tbl_arg->s->fields)) {
    write_set_backup.bitmap = nullptr; /* purecov: deadcode */
  }

  /*
   -Check that malloc() succeeded in allocating memory for the rows
    buffer and the COLS vector.
   -Checking that an Update_rows_log_event
    is valid is done while setting the Update_rows_log_event::is_valid
  */
  common_header->set_is_valid(m_rows_buf && m_cols.bitmap &&
                              write_set_backup.bitmap);
}
#endif

Rows_log_event::Rows_log_event(
    const char *buf, const Format_description_event *description_event)
    : binary_log::Rows_event(buf, description_event),
      Log_event(header(), footer()),
      m_row_count(0),
#ifdef MYSQL_SERVER
      m_table(nullptr),
#endif
      m_rows_buf(nullptr),
      m_rows_cur(nullptr),
      m_rows_end(nullptr)
#if defined(MYSQL_SERVER)
      ,
      m_curr_row(nullptr),
      m_curr_row_end(nullptr),
      m_key(nullptr),
      m_key_info(nullptr),
      m_distinct_keys(Key_compare(&m_key_info)),
      m_distinct_key_spare_buf(nullptr),
      m_fields(Replicated_columns_view::INBOUND),
      master_had_triggers(false)
#endif
{
  DBUG_TRACE;
  if (!is_valid()) return;

  DBUG_ASSERT(header()->type_code == m_type);

  if (m_extra_row_info.have_ndb_info())
    DBUG_EXECUTE_IF("extra_row_ndb_info_check",
                    /* Check extra data has expected value */
                    check_extra_row_ndb_info(m_extra_row_info.get_ndb_info()););

  /*
     m_cols and m_cols_ai are of the type MY_BITMAP, which are members of
     class Rows_log_event, and are used while applying the row events on
     the slave.
     The bitmap integer is initialized by copying the contents of the
     vector column_before_image for m_cols.bitamp, and vector
     column_after_image for m_cols_ai.bitmap. m_cols_ai is only initialized
     for UPDATE_ROWS_EVENTS, else it is equal to the before image.
  */
  /* if bitmap_init fails, is_valid will be set to false */
  if (likely(!bitmap_init(&m_cols,
                          m_width <= sizeof(m_bitbuf) * 8 ? m_bitbuf : nullptr,
                          m_width))) {
    if (!columns_before_image.empty()) {
      memcpy(m_cols.bitmap, &columns_before_image[0], n_bits_len);
      create_last_word_mask(&m_cols);
      DBUG_DUMP("m_cols", (uchar *)m_cols.bitmap, no_bytes_in_map(&m_cols));
    }  // end if columns_before_image.empty()
    else {
      if (m_cols.bitmap != m_bitbuf) bitmap_free(&m_cols);
      m_cols.bitmap = nullptr;
    }
  } else {
    // Needed because bitmap_init() does not set it to null on failure
    m_cols.bitmap = nullptr;
    common_header->set_is_valid(false);
    return;
  }
  m_cols_ai.bitmap =
      m_cols.bitmap;  // See explanation below while setting is_valid.

  if (m_type == binary_log::UPDATE_ROWS_EVENT ||
      m_type == binary_log::UPDATE_ROWS_EVENT_V1 ||
      m_type == binary_log::PARTIAL_UPDATE_ROWS_EVENT) {
    /* if bitmap_init fails, is_valid will be set to false*/
    if (likely(!bitmap_init(
            &m_cols_ai,
            m_width <= sizeof(m_bitbuf_ai) * 8 ? m_bitbuf_ai : nullptr,
            m_width))) {
      if (!columns_after_image.empty()) {
        memcpy(m_cols_ai.bitmap, &columns_after_image[0], n_bits_len);
        create_last_word_mask(&m_cols_ai);
        DBUG_DUMP("m_cols_ai", (uchar *)m_cols_ai.bitmap,
                  no_bytes_in_map(&m_cols_ai));
      } else {
        if (m_cols_ai.bitmap != m_bitbuf_ai) bitmap_free(&m_cols_ai);
        m_cols_ai.bitmap = nullptr;
      }
    } else {
      // Needed because bitmap_init() does not set it to null on failure
      m_cols_ai.bitmap = nullptr;
      common_header->set_is_valid(false);
      return;
    }
  }

  /*
    m_rows_buf, m_curr_row and m_rows_end are pointers to the vector rows.
    m_rows_buf is the pointer to the first byte of first row in the event.
    m_curr_row points to current row being applied on the slave. Initially,
    this points to the same element as m_rows_buf in the vector.
    m_rows_end points to the last byte in the last row in the event.

    These pointers are used while applying the events on to the slave, and
    are not required for decoding.
  */
  if (likely(!row.empty())) {
    m_rows_buf = &row[0];
#if defined(MYSQL_SERVER)
    m_curr_row = m_rows_buf;
#endif
    m_rows_end = m_rows_buf + row.size() - 1;
    m_rows_cur = m_rows_end;
  }

  if (bitmap_init(&write_set_backup, nullptr, m_cols.n_bits)) {
    write_set_backup.bitmap = nullptr; /* purecov: deadcode */
  }

  /*
    -Check that malloc() succeeded in allocating memory for the row
     buffer and the COLS vector.
  */
  common_header->set_is_valid(m_rows_buf && m_cols.bitmap &&
                              write_set_backup.bitmap);
}

Rows_log_event::~Rows_log_event() {
  if (m_cols.bitmap) {
    if (m_cols.bitmap == m_bitbuf)  // no my_malloc happened
      m_cols.bitmap = nullptr;      // so no my_free in bitmap_free
    bitmap_free(&m_cols);           // To pair with bitmap_init().
  }
  if (this->m_local_cols_ai.bitmap != nullptr &&
      this->m_local_cols_ai.bitmap != this->m_local_cols.bitmap) {
    bitmap_free(&this->m_local_cols_ai);
  }
  if (this->m_local_cols.bitmap != nullptr) {
    bitmap_free(&this->m_local_cols);
  }

  if (write_set_backup.bitmap) {
    bitmap_free(&write_set_backup);
  }
}

#ifdef MYSQL_SERVER
bool Rows_log_event::process_triggers(enum_trigger_event_type event,
                                      enum_trigger_action_time_type time_type,
                                      bool old_row_is_record1) {
  bool result;
  DBUG_ENTER("Rows_log_event::process_triggers");
  if (slave_run_triggers_for_rbr == SLAVE_RUN_TRIGGERS_FOR_RBR_YES) {
    Disable_binlog_guard binlog_guard(thd);
    result = m_table->triggers->process_triggers(thd, event, time_type,
                                                 old_row_is_record1);
  } else
    result = m_table->triggers->process_triggers(thd, event, time_type,
                                                 old_row_is_record1);
  DBUG_RETURN(result);
}

int Rows_log_event::unpack_current_row(const Relay_log_info *const rli,
                                       MY_BITMAP const *cols,
                                       bool is_after_image, bool only_seek) {
  DBUG_ASSERT(m_table);

  mysql_mutex_lock(&thd->LOCK_thd_query);
  // For a update event we should not clear the query when unpacking after
  // image. This can be checked using the cols parameter.
  if (cols != &m_cols_ai) thd->row_query.clear();

  Log_event_type type = get_type_code();
  if (type == Log_event_type::WRITE_ROWS_EVENT ||
      type == Log_event_type::WRITE_ROWS_EVENT_V1)
    thd->row_query = "INSERT INTO ";
  else if (type == Log_event_type::DELETE_ROWS_EVENT ||
           type == Log_event_type::DELETE_ROWS_EVENT_V1)
    thd->row_query = "DELETE FROM ";
  else {
    DBUG_ASSERT(type == Log_event_type::UPDATE_ROWS_EVENT ||
                type == Log_event_type::UPDATE_ROWS_EVENT_V1 ||
                type == Log_event_type::PARTIAL_UPDATE_ROWS_EVENT);
    if (cols != &m_cols_ai) {
      thd->row_query = "UPDATE ";
    }
  }

  if (cols != &m_cols_ai) {
    thd->row_query.append(m_table->s->db.str, m_table->s->db.length);
    thd->row_query.append(".");
    thd->row_query.append(m_table->s->table_name.str,
                          m_table->s->table_name.length);
    thd->row_query.append(" ");
  }

  enum_row_image_type row_image_type;
  if (is_after_image) {
    DBUG_ASSERT(get_general_type_code() != binary_log::DELETE_ROWS_EVENT);
    row_image_type = (get_general_type_code() == binary_log::UPDATE_ROWS_EVENT)
                         ? enum_row_image_type::UPDATE_AI
                         : enum_row_image_type::WRITE_AI;
  } else {
    DBUG_ASSERT(get_general_type_code() != binary_log::WRITE_ROWS_EVENT);
    row_image_type = (get_general_type_code() == binary_log::UPDATE_ROWS_EVENT)
                         ? enum_row_image_type::UPDATE_BI
                         : enum_row_image_type::DELETE_BI;
  }
  bool has_value_options =
      (get_type_code() == binary_log::PARTIAL_UPDATE_ROWS_EVENT);
  DBUG_ASSERT(m_curr_row <= m_rows_end);
  int ret = 0;
  if (unlikely(m_curr_row > m_rows_end)) {
    ret = HA_ERR_CORRUPT_EVENT;
  } else {
    ret = ::unpack_row(rli, m_table, m_width, m_curr_row, cols, &m_curr_row_end,
                       m_rows_end, row_image_type, has_value_options, only_seek,
                       &thd->row_query);
  }
  if (ret) {
    if (m_curr_row <= m_rows_end) {
      ret = thd->get_stmt_da()->mysql_errno();
      DBUG_ASSERT(ret);
    }
    mysql_mutex_unlock(&thd->LOCK_thd_query);
    return ret;
  }
#ifdef HAVE_PSI_THREAD_INTERFACE
  PSI_THREAD_CALL(set_thread_info)
  (thd->row_query.c_str(), thd->row_query.size());
#endif
  mysql_mutex_unlock(&thd->LOCK_thd_query);

  // After the row is unpacked, we need to update all hidden generated columns
  // for functional indexes since those values are not included in the binlog
  // in any mode of binlog_row_image.
  if (is_after_image &&
      !bitmap_is_clear_all(&m_table->fields_for_functional_indexes)) {
    // If there are a different number of columns on the master and slave, we
    // need to adjust the backup bitmap since the bitmap was initialized with
    // the number of columns on the master.
    if (write_set_backup.n_bits != m_table->s->fields) {
      bitmap_free(&write_set_backup);
      if (bitmap_init(&write_set_backup, nullptr, m_table->s->fields)) {
        return HA_ERR_OUT_OF_MEM; /* purecov: deadcode */
      }
    }

    // Make a copy of the write set, and mark all hidden generated columns.
    bitmap_copy(&write_set_backup, m_table->write_set);
    bitmap_union(m_table->write_set, &m_table->fields_for_functional_indexes);

    // Calculate the values for all hidden generated columns.
    bool res = update_generated_write_fields(
        &m_table->fields_for_functional_indexes, m_table);

    // Restore the write set.
    bitmap_copy(m_table->write_set, &write_set_backup);
    if (res) {
      return thd->get_stmt_da()->mysql_errno(); /* purecov: deadcode */
    }
  }

  return 0;
}
#endif  // ifdef MYSQL_SERVER

size_t Rows_log_event::get_data_size() {
  int const general_type_code = get_general_type_code();

  uchar buf[sizeof(m_width) + 1];
  uchar *end = net_store_length(buf, m_width);

  DBUG_EXECUTE_IF("old_row_based_repl_4_byte_map_id_master",
                  return 6 + no_bytes_in_map(&m_cols) + (end - buf) +
                         (general_type_code == binary_log::UPDATE_ROWS_EVENT
                              ? no_bytes_in_map(&m_cols_ai)
                              : 0) +
                         (m_rows_cur - m_rows_buf););

  int data_size = 0;
  bool is_v2_event =
      common_header->type_code > binary_log::DELETE_ROWS_EVENT_V1;
  if (is_v2_event) {
    data_size = Binary_log_event::ROWS_HEADER_LEN_V2;
    if (m_extra_row_info.have_ndb_info())
      data_size +=
          EXTRA_ROW_INFO_TYPECODE_LENGTH + m_extra_row_info.get_ndb_length();

    if (m_extra_row_info.have_part())
      data_size +=
          EXTRA_ROW_INFO_TYPECODE_LENGTH + m_extra_row_info.get_part_length();
  } else {
    data_size = Binary_log_event::ROWS_HEADER_LEN_V1;
  }
  data_size += no_bytes_in_map(&m_cols);
  data_size += (uint)(end - buf);

  if (general_type_code == binary_log::UPDATE_ROWS_EVENT)
    data_size += no_bytes_in_map(&m_cols_ai);

  data_size += (uint)(m_rows_cur - m_rows_buf);
  return data_size;
}

#ifdef MYSQL_SERVER
int Rows_log_event::do_add_row_data(uchar *row_data, size_t length) {
  /*
    When the table has a primary key, we would probably want, by default, to
    log only the primary key value instead of the entire "before image". This
    would save binlog space. TODO
  */
  DBUG_TRACE;
  DBUG_PRINT("enter", ("row_data: %p  length: %lu", row_data, (ulong)length));

  /*
    If length is zero, there is nothing to write, so we just
    return. Note that this is not an optimization, since calling
    realloc() with size 0 means free().
   */
  if (length == 0) {
    m_row_count++;
    return 0;
  }

  DBUG_DUMP("row_data", row_data, min<size_t>(length, 32));

  DBUG_ASSERT(m_rows_buf <= m_rows_cur);
  DBUG_ASSERT(!m_rows_buf || (m_rows_end && m_rows_buf < m_rows_end));
  DBUG_ASSERT(m_rows_cur <= m_rows_end);

  /* The cast will always work since m_rows_cur <= m_rows_end */
  if (static_cast<size_t>(m_rows_end - m_rows_cur) <= length) {
    size_t const block_size = 1024;
    ulong cur_size = m_rows_cur - m_rows_buf;
    DBUG_EXECUTE_IF("simulate_too_big_row_case1",
                    cur_size = UINT_MAX32 - (block_size * 10);
                    length = UINT_MAX32 - (block_size * 10););
    DBUG_EXECUTE_IF("simulate_too_big_row_case2",
                    cur_size = UINT_MAX32 - (block_size * 10);
                    length = block_size * 10;);
    DBUG_EXECUTE_IF("simulate_too_big_row_case3", cur_size = block_size * 10;
                    length = UINT_MAX32 - (block_size * 10););
    DBUG_EXECUTE_IF("simulate_too_big_row_case4",
                    cur_size = UINT_MAX32 - (block_size * 10);
                    length = (block_size * 10) - block_size + 1;);
    ulong remaining_space = UINT_MAX32 - cur_size;
    /* Check that the new data fits within remaining space and we can add
       block_size without wrapping.
     */
    if (length > remaining_space || ((length + block_size) > remaining_space)) {
      LogErr(ERROR_LEVEL, ER_ROW_DATA_TOO_BIG_TO_WRITE_IN_BINLOG);
      return ER_BINLOG_ROW_LOGGING_FAILED;
    }
    const size_t new_alloc =
        block_size * ((cur_size + length + block_size - 1) / block_size);
    if (new_alloc) row.resize(new_alloc);

    /* If the memory moved, we need to move the pointers */
    if (new_alloc && &row[0] != m_rows_buf) {
      m_rows_buf = &row[0];
      common_header->set_is_valid(m_rows_buf && m_cols.bitmap);
      m_rows_cur = m_rows_buf + cur_size;
    }

    /*
       The end pointer should always be changed to point to the end of
       the allocated memory.
    */
    m_rows_end = m_rows_buf + new_alloc;
  }

  DBUG_ASSERT(m_rows_cur + length <= m_rows_end);
  memcpy(m_rows_cur, row_data, length);
  m_rows_cur += length;
  m_row_count++;
  return 0;
}

/**
  Checks if any of the columns in the given table is
  signaled in the bitmap.

  For each column in the given table checks if it is
  signaled in the bitmap. This is most useful when deciding
  whether a before image (BI) can be used or not for
  searching a row. If no column is signaled, then the
  image cannot be used for searching a record (regardless
  of using position(), index scan or table scan). Here is
  an example:

  MASTER> SET @@binlog_row_image='MINIMAL';
  MASTER> CREATE TABLE t1 (a int, b int, c int, primary key(c));
  SLAVE>  CREATE TABLE t1 (a int, b int);
  MASTER> INSERT INTO t1 VALUES (1,2,3);
  MASTER> UPDATE t1 SET a=2 WHERE b=2;

  For the update statement only the PK (column c) is
  logged in the before image (BI). As such, given that
  the slave has no column c, it will not be able to
  find the row, because BI has no values for the columns
  the slave knows about (column a and b).

  @param table   the table reference on the slave.
  @param tabledef the table definition according to the master
  @param cols the bitmap signaling columns available in
                 the BI.

  @return true if BI contains usable colums for searching,
          false otherwise.
*/
static bool is_any_column_signaled_for_table(TABLE *table, table_def *tabledef,
                                             MY_BITMAP *cols,
                                             MY_BITMAP *cols2) {
  DBUG_TRACE;

  if (tabledef->use_column_names(table)) {
    DBUG_ASSERT(tabledef->size() == cols2->n_bits);
    for (uint i = 0; i < cols2->n_bits; ++i) {
      if (bitmap_is_set(cols2, i)) {
        const char *col_name = tabledef->get_column_name(i);
        if (find_field_in_table_sef(table, col_name)) return true;
      }
    }
    return false;
  }

  for (Field **ptr = table->field; *ptr && ((*ptr)->field_index < cols->n_bits);
       ptr++) {
    if (bitmap_is_set(cols, (*ptr)->field_index)) return true;
  }

  return false;
}

/**
  Checks if the fields in the given key are signaled in
  the bitmap.

  Validates whether the before image is usable for the
  given key. It can be the case that the before image
  does not contain values for the key (eg, master was
  using 'minimal' option for image logging and slave has
  different index structure on the table). Here is an
  example:

  MASTER> SET @@binlog_row_image='MINIMAL';
  MASTER> CREATE TABLE t1 (a int, b int, c int, primary key(c));
  SLAVE> CREATE TABLE t1 (a int, b int, c int, key(a,c));
  MASTER> INSERT INTO t1 VALUES (1,2,3);
  MASTER> UPDATE t1 SET a=2 WHERE b=2;

  When finding the row on the slave, one cannot use the
  index (a,c) to search for the row, because there is only
  data in the before image for column c. This function
  checks the fields needed for a given key and searches
  the bitmap to see if all the fields required are
  signaled.

  @param table    table definition
  @param keyinfo  reference to key.
  @param tabledef the table definition according to the master
  @param cols     the bitmap signaling which columns
                  have available data.

  @return true if all fields are signaled in the bitmap
          for the given key, false otherwise.
*/
static bool are_all_columns_signaled_for_key(TABLE *table, KEY *keyinfo,
                                             table_def *tabledef,
                                             MY_BITMAP *cols) {
  DBUG_TRACE;

  for (uint i = 0; i < keyinfo->user_defined_key_parts; i++) {
    if (tabledef->use_column_names(table)) {
      Field *key_field = keyinfo->key_part[i].field;
      const std::pair<uint, bool> res =
          tabledef->get_column_index(key_field->field_name);
      if (!res.second || !bitmap_is_set(cols, res.first)) return false;
    } else {
      uint fieldnr = keyinfo->key_part[i].fieldnr - 1;
      if (fieldnr >= cols->n_bits || !bitmap_is_set(cols, fieldnr))
        return false;
    }
  }

  return true;
}

/**
  Searches the table for a given key that can be used
  according to the existing values, ie, columns set
  in the bitmap.

  The caller can specify which type of key to find by
  setting the following flags in the key_type parameter:

    - PRI_KEY_FLAG
      Returns the primary key.

    - UNIQUE_KEY_FLAG
      Returns a unique key (flagged with HA_NOSAME)

    - MULTIPLE_KEY_FLAG
      Returns a key that is not unique (flagged with HA_NOSAME
      and without HA_NULL_PART_KEY) nor PK.

  The above flags can be used together, in which case, the
  search is conducted in the above listed order. Eg, the
  following flag:

    (PRI_KEY_FLAG | UNIQUE_KEY_FLAG | MULTIPLE_KEY_FLAG)

  means that a primary key is returned if it is suitable. If
  not then the unique keys are searched. If no unique key is
  suitable, then the keys are searched. Finally, if no key
  is suitable, MAX_KEY is returned.

  @param table    reference to the table.
  @param tabledef table definition according to the master
  @param bi_cols  a bitmap that filters out columns that should
                  not be considered while searching the key.
                  Columns that should be considered are set.
  @param key_type the type of key to search for.

  @return MAX_KEY if no key, according to the key_type specified
          is suitable. Returns the key otherwise.

*/
static uint search_key_in_table(TABLE *table, table_def *tabledef,
                                MY_BITMAP *bi_cols, uint key_type) {
  DBUG_TRACE;

  KEY *keyinfo;
  uint res = MAX_KEY;
  uint key;

  if (key_type & PRI_KEY_FLAG && (table->s->primary_key < MAX_KEY)) {
    DBUG_PRINT("debug", ("Searching for PK"));
    keyinfo = table->s->key_info + table->s->primary_key;
    if (are_all_columns_signaled_for_key(table, keyinfo, tabledef, bi_cols))
      return table->s->primary_key;
  }

  if (key_type & UNIQUE_KEY_FLAG) {
    DBUG_PRINT("debug", ("Searching for UK"));
    for (key = 0, keyinfo = table->key_info;
         (key < table->s->keys) && (res == MAX_KEY); key++, keyinfo++) {
      /*
        - Unique keys cannot be disabled, thence we skip the check.
        - Skip unique keys with nullable parts
        - Skip primary keys
        - Skip functional indexes if the slave_rows_search_algorithms=INDEX_SCAN
        - Skip multi-valued keys as they have only part of value and can't
          fully identify a record
      */
      if (!((keyinfo->flags & (HA_NOSAME | HA_NULL_PART_KEY)) == HA_NOSAME) ||
          (key == table->s->primary_key) ||
          ((slave_rows_search_algorithms_options & SLAVE_ROWS_INDEX_SCAN) &&
           keyinfo->is_functional_index()) ||
          keyinfo->flags & HA_MULTI_VALUED_KEY) {
        continue;
      }
      res = are_all_columns_signaled_for_key(table, keyinfo, tabledef, bi_cols)
                ? key
                : MAX_KEY;

      if (res < MAX_KEY) return res;
    }
    DBUG_PRINT("debug", ("UK has NULLABLE parts or not all columns signaled."));
  }

  if (key_type & MULTIPLE_KEY_FLAG && table->s->keys) {
    DBUG_PRINT("debug", ("Searching for K."));
    for (key = 0, keyinfo = table->key_info;
         (key < table->s->keys) && (res == MAX_KEY); key++, keyinfo++) {
      /*
        The following indexes are skipped:
        - Inactive/invisible indexes.
        - UNIQUE NOT NULL indexes.
        - Indexes that do not support ha_index_next() e.g. full-text.
        - Primary key indexes.
        - Functional indexes if the slave_rows_search_algorithms=INDEX_SCAN
        - Skip multi-valued keys as they have only part of value and can't
          fully identify a record
      */
      if (!(table->s->usable_indexes(current_thd).is_set(key)) ||
          ((keyinfo->flags & (HA_NOSAME | HA_NULL_PART_KEY)) == HA_NOSAME) ||
          !(table->file->index_flags(key, 0, true) & HA_READ_NEXT) ||
          (key == table->s->primary_key) ||
          ((slave_rows_search_algorithms_options & SLAVE_ROWS_INDEX_SCAN) &&
           keyinfo->is_functional_index()) ||
          keyinfo->flags & HA_MULTI_VALUED_KEY) {
        continue;
      }

      res = are_all_columns_signaled_for_key(table, keyinfo, tabledef, bi_cols)
                ? key
                : MAX_KEY;

      if (res < MAX_KEY) return res;
    }
    DBUG_PRINT("debug", ("Not all columns signaled for K."));
  }

  return res;
}

void Rows_log_event::decide_row_lookup_algorithm_and_key(table_def *tabledef) {
  DBUG_TRACE;

  /*
    Decision table:
    - I  --> Index scan / search
    - T  --> Table scan
    - Hi --> Hash over index
    - Ht --> Hash over the entire table

    |--------------+-----------+------+------+------|
    | Index\Option | I , T , H | I, T | I, H | T, H |
    |--------------+-----------+------+------+------|
    | PK / UK      | I         | I    | I    | Hi   |
    | K            | Hi        | I    | Hi   | Hi   |
    | No Index     | Ht        | T    | Ht   | Ht   |
    |--------------+-----------+------+------+------|

  */
  TABLE *table = this->m_table;
  uint event_type = this->get_general_type_code();
  MY_BITMAP *cols = &this->m_cols;
  this->m_rows_lookup_algorithm = ROW_LOOKUP_NOT_NEEDED;
  this->m_key_index = MAX_KEY;
  this->m_key_info = nullptr;

  // row lookup not needed
  if (event_type == binary_log::WRITE_ROWS_EVENT ||
      ((event_type == binary_log::DELETE_ROWS_EVENT ||
        event_type == binary_log::UPDATE_ROWS_EVENT) &&
       get_flags(COMPLETE_ROWS_F) && m_table->file->use_read_free_rpl()))
    return;

  if (!(slave_rows_search_algorithms_options & SLAVE_ROWS_INDEX_SCAN))
    goto TABLE_OR_INDEX_HASH_SCAN;

  /* PK or UK => use LOOKUP_INDEX_SCAN */
  this->m_key_index = search_key_in_table(table, tabledef, cols,
                                          (PRI_KEY_FLAG | UNIQUE_KEY_FLAG));
  if (this->m_key_index != MAX_KEY) {
    DBUG_PRINT("info",
               ("decide_row_lookup_algorithm_and_key: decided - INDEX_SCAN"));
    this->m_rows_lookup_algorithm = ROW_LOOKUP_INDEX_SCAN;
    goto end;
  }

TABLE_OR_INDEX_HASH_SCAN:

  /*
     NOTE: Engines like Blackhole cannot use HASH_SCAN, because
           they do not syncronize reads .
   */
  if (!(slave_rows_search_algorithms_options & SLAVE_ROWS_HASH_SCAN) ||
      (table->file->ha_table_flags() & HA_READ_OUT_OF_SYNC))
    goto TABLE_OR_INDEX_FULL_SCAN;

  /* search for a key to see if we can narrow the lookup domain further. */
  this->m_key_index =
      search_key_in_table(table, tabledef, cols,
                          (PRI_KEY_FLAG | UNIQUE_KEY_FLAG | MULTIPLE_KEY_FLAG));
  this->m_rows_lookup_algorithm = ROW_LOOKUP_HASH_SCAN;
  if (m_key_index < MAX_KEY)
    m_distinct_key_spare_buf =
        (uchar *)thd->alloc(table->key_info[m_key_index].key_length);
  DBUG_PRINT("info",
             ("decide_row_lookup_algorithm_and_key: decided - HASH_SCAN"));
  goto end;

TABLE_OR_INDEX_FULL_SCAN:

  this->m_key_index = MAX_KEY;

  /* If we can use an index, try to narrow the scan a bit further. */
  if (slave_rows_search_algorithms_options & SLAVE_ROWS_INDEX_SCAN)
    this->m_key_index = search_key_in_table(
        table, tabledef, cols,
        (PRI_KEY_FLAG | UNIQUE_KEY_FLAG | MULTIPLE_KEY_FLAG));

  if (this->m_key_index != MAX_KEY) {
    DBUG_PRINT("info",
               ("decide_row_lookup_algorithm_and_key: decided - INDEX_SCAN"));
    this->m_rows_lookup_algorithm = ROW_LOOKUP_INDEX_SCAN;
  } else {
    DBUG_PRINT("info",
               ("decide_row_lookup_algorithm_and_key: decided - TABLE_SCAN"));
    this->m_rows_lookup_algorithm = ROW_LOOKUP_TABLE_SCAN;
  }

end:
  /* m_key_index is ready, set m_key_info now. */
  m_key_info = m_table->key_info + m_key_index;
  /*
    m_key_info will influence key comparison code in HASH_SCAN mode,
    so the m_distinct_keys set should still be empty.
  */
  DBUG_ASSERT(m_distinct_keys.empty());

#ifndef DBUG_OFF
  const char *s =
      ((m_rows_lookup_algorithm == Rows_log_event::ROW_LOOKUP_TABLE_SCAN)
           ? "TABLE_SCAN"
           : ((m_rows_lookup_algorithm == Rows_log_event::ROW_LOOKUP_HASH_SCAN)
                  ? "HASH_SCAN"
                  : "INDEX_SCAN"));

  // only for testing purposes
  slave_rows_last_search_algorithm_used = m_rows_lookup_algorithm;
  DBUG_PRINT("debug", ("Row lookup method: %s", s));
#endif
}

/*
  Encapsulates the  operations to be done before applying
  row events for update and delete.

  @ret value error code
             0 success
*/
int Rows_log_event::row_operations_scan_and_key_setup(table_def *tabledef) {
  int error = 0;
  DBUG_TRACE;

  /*
     Prepare memory structures for search operations. If
     search is performed:

     1. using hash search => initialize the hash
     2. using key => decide on key to use and allocate mem structures
     3. using table scan => do nothing
   */
  decide_row_lookup_algorithm_and_key(tabledef);

  switch (m_rows_lookup_algorithm) {
    case ROW_LOOKUP_HASH_SCAN: {
      if (m_hash.init()) error = HA_ERR_OUT_OF_MEM;
      goto err;
    }
    case ROW_LOOKUP_INDEX_SCAN: {
      DBUG_ASSERT(m_key_index < MAX_KEY);
      // Allocate buffer for key searches
      m_key = (uchar *)my_malloc(key_memory_log_event, m_key_info->key_length,
                                 MYF(MY_WME));
      if (!m_key) error = HA_ERR_OUT_OF_MEM;
      goto err;
    }
    case ROW_LOOKUP_TABLE_SCAN:
    default:
      break;
  }
err:
  return error;
}

/*
  Encapsulates the  operations to be done after applying
  row events for update and delete.

  @ret value error code
             0 success
*/

int Rows_log_event::row_operations_scan_and_key_teardown(int error) {
  DBUG_TRACE;

  DBUG_ASSERT(!m_table->file->inited);
  switch (m_rows_lookup_algorithm) {
    case ROW_LOOKUP_HASH_SCAN: {
      m_hash.deinit();  // we don't need the hash anymore.
      goto err;
    }

    case ROW_LOOKUP_INDEX_SCAN: {
      if (m_table->s->keys > 0) {
        my_free(m_key);  // Free for multi_malloc
        m_key = nullptr;
        m_key_index = MAX_KEY;
        m_key_info = nullptr;
      }
      goto err;
    }

    case ROW_LOOKUP_TABLE_SCAN:
    default:
      break;
  }

err:
  m_rows_lookup_algorithm = ROW_LOOKUP_UNDEFINED;
  return error;
}

bool Rows_log_event::is_auto_inc_in_extra_columns() {
  DBUG_ASSERT(m_table);
  return (m_table->next_number_field &&
          this->m_fields.translate_position(
              m_table->next_number_field->field_index) >= m_width);
}

/*
  Compares table->record[0] and table->record[1]

  Returns true if different.
*/
static bool record_compare(TABLE *table, table_def *tabledef, MY_BITMAP *cols) {
  DBUG_TRACE;

  /*
    Need to set the X bit and the filler bits in both records since
    there are engines that do not set it correctly.

    In addition, since MyISAM checks that one hasn't tampered with the
    record, it is necessary to restore the old bytes into the record
    after doing the comparison.

    TODO[record format ndb]: Remove it once NDB returns correct
    records. Check that the other engines also return correct records.
   */

  DBUG_DUMP("record[0]", table->record[0], table->s->reclength);
  DBUG_DUMP("record[1]", table->record[1], table->s->reclength);

  bool result = false;
  uchar saved_x[2] = {0, 0}, saved_filler[2] = {0, 0};

  if (table->s->null_bytes > 0) {
    for (int i = 0; i < 2; ++i) {
      /*
        If we have an X bit then we need to take care of it.
      */
      if (!(table->s->db_options_in_use & HA_OPTION_PACK_RECORD)) {
        saved_x[i] = table->record[i][0];
        table->record[i][0] |= 1U;
      }

      /*
         If (last_null_bit_pos == 0 && null_bytes > 1), then:

         X bit (if any) + N nullable fields + M Field_bit fields = 8 bits

         Ie, the entire byte is used.
      */
      if (table->s->last_null_bit_pos > 0) {
        saved_filler[i] = table->record[i][table->s->null_bytes - 1];
        table->record[i][table->s->null_bytes - 1] |=
            256U - (1U << table->s->last_null_bit_pos);
      }
    }
  }

  /**
    Compare full record only if:
    - there are no blob fields (otherwise we would also need
      to compare blobs contents as well);
    - there are no varchar fields (otherwise we would also need
      to compare varchar contents as well);
    - there are no null fields, otherwise NULLed fields
      contents (i.e., the don't care bytes) may show arbitrary
      values, depending on how each engine handles internally.
    - if all the bitmap is set (both are full rows)
    */
  bool cmp_full_record = (table->s->blob_fields + table->s->varchar_fields +
                          table->s->null_fields) == 0 &&
                         bitmap_is_set_all(cols);
  if (cmp_full_record) {
    result = cmp_record(table, record[1]);
  } else {
    /*
       Fallback to field-by-field comparison:
       1. start by checking if the field is signaled:
       2. if it is, first compare the null bit if the field is nullable
       3. then compare the contents of the field, if it is not
       set to null
     */
    if (tabledef->use_column_names(table)) {
      for (uint i = 0; i < tabledef->size() && !result; ++i) {
        if (!bitmap_is_set(cols, i)) continue;
        const char *col_name = tabledef->get_column_name(i);
        Field *const field = find_field_in_table_sef(table, col_name);
        if (field) {
          /* compare null bit */
          if (field->is_null() != field->is_null_in_record(table->record[1]))
            result = true;

          /* compare content, only if fields are not set to NULL */
          else if (!field->is_null()) {
            result = field->cmp_binary_offset(table->s->rec_buff_length);
          }
        }
      }
    } else {
      for (Field **ptr = table->field;
           *ptr && ((*ptr)->field_index < cols->n_bits) && !result; ptr++) {
        Field *field = *ptr;
        if (bitmap_is_set(cols, field->field_index) &&
            !field->is_virtual_gcol()) {
          /* compare null bit */
          if (field->is_null() != field->is_null_in_record(table->record[1]))
            result = true;

          /* compare content, only if fields are not set to NULL */
          else if (!field->is_null()) {
            result = field->cmp_binary_offset(table->s->rec_buff_length);
          }
        }
      }
    }
  }

  /*
    Restore the saved bytes.

    TODO[record format ndb]: Remove this code once NDB returns the
    correct record format.
  */
  if (table->s->null_bytes > 0) {
    for (int i = 0; i < 2; ++i) {
      if (!(table->s->db_options_in_use & HA_OPTION_PACK_RECORD))
        table->record[i][0] = saved_x[i];

      if (table->s->last_null_bit_pos)
        table->record[i][table->s->null_bytes - 1] = saved_filler[i];
    }
  }

  return result;
}

/**
 * Assumes that the relay log before image is stored in record[1] and the local
 * image read from the storage engine is stored in record[0] and calculates the
 * difference in column values
 *
 * @return A pair of strings where the 1st string is the column values in the
 * source (i.e. relay log) and 2nd is the col values of the local DB. The
 * strings are in col=val format, comma separated if multiple cols have
 * mismatch. Only cols that have mismatch are recorded.
 */
static std::pair<std::string, std::string> calculate_diff(TABLE *table,
                                                          table_def *tabledef,
                                                          MY_BITMAP *cols) {
  DBUG_ENTER("calculate_diff");
  std::string source, local;

  /*
    Need to set the X bit and the filler bits in both records since
    there are engines that do not set it correctly.

    In addition, since MyISAM checks that one hasn't tampered with the
    record, it is necessary to restore the old bytes into the record
    after doing the comparison.

    TODO[record format ndb]: Remove it once NDB returns correct
    records. Check that the other engines also return correct records.
   */
  uchar saved_x[2] = {0, 0}, saved_filler[2] = {0, 0};
  if (table->s->null_bytes > 0) {
    for (int i = 0; i < 2; ++i) {
      /*
        If we have an X bit then we need to take care of it.
      */
      if (!(table->s->db_options_in_use & HA_OPTION_PACK_RECORD)) {
        saved_x[i] = table->record[i][0];
        table->record[i][0] |= 1U;
      }

      /*
         If (last_null_bit_pos == 0 && null_bytes > 1), then:
         X bit (if any) + N nullable fields + M Field_bit fields = 8 bits
         Ie, the entire byte is used.
      */
      if (table->s->last_null_bit_pos > 0) {
        saved_filler[i] = table->record[i][table->s->null_bytes - 1];
        table->record[i][table->s->null_bytes - 1] |=
            256U - (1U << table->s->last_null_bit_pos);
      }
    }
  }

  if (tabledef->use_column_names(table)) {
    for (uint i = 0; i < tabledef->size(); ++i) {
      if (!bitmap_is_set(cols, i)) continue;
      const char *col_name = tabledef->get_column_name(i);
      Field *const field = find_field_in_table_sef(table, col_name);
      if (field) {
        if ((field->is_null() != field->is_null_in_record(table->record[1])) ||
            (!field->is_null() &&
             field->cmp_binary_offset(table->s->rec_buff_length))) {
          String str1, str2;
          const ptrdiff_t offset = table->s->rec_buff_length;
          field->val_str(&str1);
          const std::string local_val =
              field->is_null() ? "NULL" : std::string(str1.c_ptr_safe());
#ifndef DBUG_OFF
          const uchar *prev = field->ptr;
#endif
          field->move_field_offset(offset);
          field->val_str(&str2);
          const std::string source_val =
              field->is_null() ? "NULL" : std::string(str2.c_ptr_safe());

          if (source_val != local_val) {
            local += std::string(field->field_name) + "=" + local_val + ",";
            source += std::string(field->field_name) + "=" + source_val + ",";
          }

          field->move_field_offset(-offset);
          DBUG_ASSERT(field->ptr == prev);
        }
      }
    }
  } else {
    for (Field **ptr = table->field;
         *ptr && ((*ptr)->field_index < cols->n_bits); ptr++) {
      Field *field = *ptr;
      if (bitmap_is_set(cols, field->field_index)) {
        if ((field->is_null() != field->is_null_in_record(table->record[1])) ||
            (!field->is_null() &&
             field->cmp_binary_offset(table->s->rec_buff_length))) {
          String str1, str2;
          const ptrdiff_t offset = table->s->rec_buff_length;
          field->val_str(&str1);
          const std::string local_val =
              field->is_null() ? "NULL" : std::string(str1.c_ptr_safe());
#ifndef DBUG_OFF
          const uchar *prev = field->ptr;
#endif
          field->move_field_offset(offset);
          field->val_str(&str2);
          const std::string source_val =
              field->is_null() ? "NULL" : std::string(str2.c_ptr_safe());

          if (source_val != local_val) {
            local += std::string(field->field_name) + "=" + local_val + ",";
            source += std::string(field->field_name) + "=" + source_val + ",";
          }

          field->move_field_offset(-offset);
          DBUG_ASSERT(field->ptr == prev);
        }
      }
    }
  }

  if (table->s->null_bytes > 0) {
    for (int i = 0; i < 2; ++i) {
      if (!(table->s->db_options_in_use & HA_OPTION_PACK_RECORD))
        table->record[i][0] = saved_x[i];

      if (table->s->last_null_bit_pos > 0)
        table->record[i][table->s->null_bytes - 1] = saved_filler[i];
    }
  }

  // Remove trailing comma
  if (source.size()) source.pop_back();
  if (local.size()) local.pop_back();

  DBUG_RETURN(std::make_pair(source, local));
}

void Rows_log_event::do_post_row_operations(Relay_log_info const *rli,
                                            int error) {
  /*
    If m_curr_row_end  was not set during event execution (e.g., because
    of errors) we can't proceed to the next row. If the error is transient
    (i.e., error==0 at this point) we must call unpack_current_row() to set
    m_curr_row_end.
  */

  DBUG_PRINT("info", ("curr_row: %p; curr_row_end: %p; rows_end: %p",
                      m_curr_row, m_curr_row_end, m_rows_end));

  if (!m_curr_row_end && !error) {
    /*
      This function is always called immediately following a call to
      handle_idempotent_and_ignored_errors which returns 0.  And
      handle_idempotent_and_ignored_errors can only return 0 when
      error==0.  And when error==0, it means that the previous call to
      unpack_currrent_row was successful.  And that means
      m_curr_row_end has been set to a valid pointer.  So it is
      impossible that both error==0 and m_curr_row_end==0 under normal
      conditions. So this is probably a case of a corrupt event.
    */
    const uchar *previous_m_curr_row = m_curr_row;
    error = unpack_current_row(rli, &m_cols, true /*is AI*/);

    if (!error && previous_m_curr_row == m_curr_row) {
      error = 1;
    }
  }

  // at this moment m_curr_row_end should be set
  DBUG_ASSERT(error || m_curr_row_end != nullptr);
  DBUG_ASSERT(error || m_curr_row <= m_curr_row_end);
  DBUG_ASSERT(error || m_curr_row_end <= m_rows_end);

  m_curr_row = m_curr_row_end;

  if (error == 0 && !m_table->file->has_transactions()) {
    thd->get_transaction()->set_unsafe_rollback_flags(Transaction_ctx::SESSION,
                                                      true);
    thd->get_transaction()->set_unsafe_rollback_flags(Transaction_ctx::STMT,
                                                      true);
  }

#ifdef HAVE_PSI_STAGE_INTERFACE
  /*
   Count the number of rows processed unconditionally. Needed instrumentation
   may be toggled while a rows event is being processed.
  */
  m_psi_progress.inc_n_rows_applied(1);

  if (m_curr_row > m_rows_buf) {
    /* Report progress. */
    m_psi_progress.update_work_estimated_and_completed(m_curr_row, m_rows_buf,
                                                       m_rows_end);
  } else if (m_curr_row == m_rows_buf) {
    /*
      Master can generate an empty row, in the following situation:
      mysql> SET SESSION binlog_row_image=MINIMAL;
      mysql> CREATE TABLE t1 (c1 INT DEFAULT 100);
      mysql> INSERT INTO t1 VALUES ();

      Otherwise, m_curr_row must be ahead of m_rows_buf, since we
      have processed the first row already.

      No point in reporting progress, since this would show for a
      very small fraction of time - thence no point in speding extra
      CPU cycles for this.

      Nevertheless assert that the event is a write event, otherwise,
      this should not happen.
    */
    DBUG_ASSERT(get_general_type_code() == binary_log::WRITE_ROWS_EVENT);
  } else
    /* Impossible */
    DBUG_ASSERT(false);

  DBUG_EXECUTE_IF("dbug.rpl_apply_sync_barrier", {
    const char act[] =
        "now SIGNAL signal.rpl_row_apply_progress_updated "
        "WAIT_FOR signal.rpl_row_apply_process_next_row";
    DBUG_ASSERT(opt_debug_sync_timeout > 0);
    DBUG_ASSERT(!debug_sync_set_action(thd, STRING_WITH_LEN(act)));
  };);
#endif /* HAVE_PSI_STAGE_INTERFACE */
}

int Rows_log_event::handle_idempotent_and_ignored_errors(
    Relay_log_info const *rli, int *err) {
  int error = *err;
  if (error) {
    int actual_error = convert_handler_error(error, thd, m_table);
    bool idempotent_error = (idempotent_error_code(error) &&
                             (rbr_exec_mode == RBR_EXEC_MODE_IDEMPOTENT ||
                              m_table->file->last_part_has_ttl_column()));
    bool ignore_delete_error =
        (rbr_exec_mode == RBR_EXEC_MODE_SEMI_STRICT &&
         (error == HA_ERR_RECORD_CHANGED || error == HA_ERR_KEY_NOT_FOUND));
    bool ignored_error = ((idempotent_error == 0 && !ignore_delete_error)
                              ? ignored_error_code(actual_error)
                              : 0);

    if (ignore_delete_error) {
      ++rbr_unsafe_queries;
    }

    if (idempotent_error || ignored_error || ignore_delete_error) {
      loglevel ll;
      if (idempotent_error)
        ll = WARNING_LEVEL;
      else
        ll = INFORMATION_LEVEL;
      slave_rows_error_report(
          ll, error, rli, thd, m_table, get_type_str(),
          const_cast<Relay_log_info *>(rli)->get_rpl_log_name(),
          (ulong)common_header->log_pos);
      thd->get_stmt_da()->reset_condition_info(thd);
      clear_all_errors(thd, const_cast<Relay_log_info *>(rli));
      *err = 0;
      if (idempotent_error == 0 && !ignore_delete_error) return ignored_error;
    }
  }

  return *err;
}

int Rows_log_event::do_apply_row(Relay_log_info const *rli,
                                 table_def *tabledef MY_ATTRIBUTE((unused))) {
  DBUG_TRACE;

  int error = 0;

  /* in_use can have been set to NULL in close_tables_for_reopen */
  THD *old_thd = m_table->in_use;
  if (!m_table->in_use) m_table->in_use = thd;

  error = do_exec_row(rli);

  if (error) {
    DBUG_PRINT("info", ("error: %s", HA_ERR(error)));
    DBUG_ASSERT(error != HA_ERR_RECORD_DELETED);
  }
  m_table->in_use = old_thd;

  return error;
}

/**
   Does the cleanup
     -  closes the index if opened by open_record_scan
     -  closes the table if opened for scanning.
*/
int Rows_log_event::close_record_scan() {
  DBUG_TRACE;
  int error = 0;

  // if there is something to actually close
  if (m_key_index < MAX_KEY) {
    if (m_table->file->inited) error = m_table->file->ha_index_end();
  } else if (m_table->file->inited)
    error = m_table->file->ha_rnd_end();

  return error;
}

/**
  Fetches next row. If it is a HASH_SCAN over an index, it populates
  table->record[0] with the next row corresponding to the index. If
  the indexes are in non-contiguous ranges it fetches record corresponding
  to the key value in the next range.

  @param first_read Signifying if this is the first time we are reading a row
          over an index.
  @retval error code when there are no more records to be fetched or some other
                    error occurred
  @retval 0 otherwise.
*/
int Rows_log_event::next_record_scan(bool first_read) {
  DBUG_TRACE;
  DBUG_ASSERT(m_table->file->inited);
  TABLE *table = m_table;
  int error = 0;

  if (m_key_index >= MAX_KEY)
    error = table->file->ha_rnd_next(table->record[0]);
  else {
    /*
      We need to set the null bytes to ensure that the filler bit are
      all set when returning.  There are storage engines that just set
      the necessary bits on the bytes and don't set the filler bits
      correctly.
    */
    if (table->s->null_bytes > 0)
      table->record[0][table->s->null_bytes - 1] |=
          256U - (1U << table->s->last_null_bit_pos);

    if (!first_read) {
      /*
        if we fail to fetch next record corresponding to a key value, we
        move to the next key value. If we are out of key values as well an error
        will be returned.
       */
      error = table->file->ha_index_next_same(table->record[0], m_key,
                                              m_key_info->key_length);
      if (m_rows_lookup_algorithm == ROW_LOOKUP_HASH_SCAN) {
        /*
          if we are out of rows for this particular key value, we reposition the
          marker according to the next key value that we have in the list.
         */
        if (error) {
          if (m_itr != m_distinct_keys.end()) {
            m_key = *m_itr;
            m_itr++;
            first_read = true;
          } else
            error = HA_ERR_KEY_NOT_FOUND;
        }
      }
    }

    if (first_read)
      if ((error = table->file->ha_index_read_map(
               table->record[0], m_key, HA_WHOLE_KEY, HA_READ_KEY_EXACT))) {
        DBUG_PRINT("info", ("no record matching the key found in the table"));
        error = HA_ERR_KEY_NOT_FOUND;
      }
  }

  return error;
}

/**
  Initializes scanning of rows. Opens an index and initializes an iterator
  over a list of distinct keys (m_distinct_keys) if it is a HASH_SCAN
  over an index or the table if its a HASH_SCAN over the table.
*/
int Rows_log_event::open_record_scan() {
  int error = 0;
  TABLE *table = m_table;
  DBUG_TRACE;

  if (m_key_index < MAX_KEY) {
    if (m_rows_lookup_algorithm == ROW_LOOKUP_HASH_SCAN) {
      /* initialize the iterator over the list of distinct keys that we have */
      m_itr = m_distinct_keys.begin();

      /* get the first element from the list of keys and increment the
         iterator
       */
      m_key = *m_itr;
      m_itr++;
    } else {
      /* this is an INDEX_SCAN we need to store the key in m_key */
      DBUG_ASSERT((m_rows_lookup_algorithm == ROW_LOOKUP_INDEX_SCAN) && m_key);
      key_copy(m_key, m_table->record[0], m_key_info, 0);
    }

    /*
      Save copy of the record in table->record[1]. It might be needed
      later if linear search is used to find exact match.
     */
    store_record(table, record[1]);

    DBUG_PRINT("info", ("locating record using a key (index_read)"));

    /* The m_key_index'th key is active and usable: search the table using the
     * index */
    if (!table->file->inited &&
        (error = table->file->ha_index_init(m_key_index, false))) {
      DBUG_PRINT("info", ("ha_index_init returns error %d", error));
      goto end;
    }

    DBUG_DUMP("key data", m_key, m_key_info->key_length);
  } else {
    if ((error = table->file->ha_rnd_init(true))) {
      DBUG_PRINT("info", ("error initializing table scan"
                          " (ha_rnd_init returns %d)",
                          error));
      table->file->print_error(error, MYF(0));
    }
  }

end:
  return error;
}

/**
  Populates the m_distinct_keys with unique keys to be modified
  during HASH_SCAN over keys.
  @retval 0 success
*/
int Rows_log_event::add_key_to_distinct_keyset() {
  int error = 0;
  DBUG_TRACE;
  DBUG_ASSERT(m_key_index < MAX_KEY);
  key_copy(m_distinct_key_spare_buf, m_table->record[0], m_key_info, 0);
  std::pair<std::set<uchar *, Key_compare>::iterator, bool> ret =
      m_distinct_keys.insert(m_distinct_key_spare_buf);
  if (ret.second) {
    /* Insert is successful, so allocate a new buffer for next key */
    m_distinct_key_spare_buf = (uchar *)thd->alloc(m_key_info->key_length);
    if (!m_distinct_key_spare_buf) {
      error = HA_ERR_OUT_OF_MEM;
      goto err;
    }
  }

err:
  return error;
}

int Rows_log_event::do_index_scan_and_update(Relay_log_info const *rli,
                                             table_def *tabledef) {
  DBUG_TRACE;
  DBUG_ASSERT(m_table && m_table->in_use != nullptr);

  int error = 0;
  const uchar *saved_m_curr_row = m_curr_row;

  /*
    rpl_row_tabledefs.test specifies that
    if the extra field on the slave does not have a default value
    and this is okay with Delete or Update events.
    Todo: fix wl3228 hld that requires defaults for all types of events
  */

  prepare_record(m_table, &this->m_local_cols, false);
  if ((error = unpack_current_row(rli, &m_cols, false /*is not AI*/))) goto end;

  /*
    Trying to do an index scan without a usable key
    This is a valid state because we allow the user
    to set Slave_rows_search_algorithm= 'INDEX_SCAN'.

    Therefore on tables with no indexes we will end
    up here.
   */
  if (m_key_index >= MAX_KEY) {
    error = HA_ERR_END_OF_FILE;
    goto end;
  }

#ifndef DBUG_OFF
  DBUG_PRINT("info", ("looking for the following record"));
  DBUG_DUMP("record[0]", m_table->record[0], m_table->s->reclength);
#endif

  if (m_key_index != m_table->s->primary_key)
    /* we dont have a PK, or PK is not usable */
    goto INDEX_SCAN;

  if ((m_table->file->ha_table_flags() & HA_READ_BEFORE_WRITE_REMOVAL)) {
    /*
      Read removal is possible since the engine supports write without
      previous read using full primary key
    */
    DBUG_PRINT("info", ("using read before write removal"));
    DBUG_ASSERT(m_key_index == m_table->s->primary_key);

    /*
      Tell the handler to ignore if key exists or not, since it's
      not yet known if the key does exist(when using rbwr)
    */
    m_table->file->ha_extra(HA_EXTRA_IGNORE_NO_KEY);

    goto end;
  }

  if ((m_table->file->ha_table_flags() &
       HA_PRIMARY_KEY_REQUIRED_FOR_POSITION)) {
    /*
      Use a more efficient method to fetch the record given by
      table->record[0] if the engine allows it.  We first compute a
      row reference using the position() member function (it will be
      stored in table->file->ref) and then use rnd_pos() to position
      the "cursor" (i.e., record[0] in this case) at the correct row.

    */

    // Save the before image in record[1]
    if (opt_slave_check_before_image_consistency)
      store_record(m_table, record[1]);

    DBUG_PRINT("info", ("locating record using primary key (position)"));
    if (m_table->file->inited && (error = m_table->file->ha_index_end()))
      goto end;

    error = m_table->file->rnd_pos_by_record(m_table->record[0]);

    if (error) {
      DBUG_PRINT("info", ("rnd_pos returns error %d", error));
      if (error == HA_ERR_RECORD_DELETED) error = HA_ERR_KEY_NOT_FOUND;
    }

    goto end;
  }

  // We can't use position() - try other methods.

INDEX_SCAN:

  /* Use the m_key_index'th key */

  // open_record_scan() will store the before image in m_table->record[1]
  if ((error = open_record_scan())) goto end;

  error = next_record_scan(true);
  if (error) {
    DBUG_PRINT("info", ("no record matching the key found in the table"));
    if (error == HA_ERR_RECORD_DELETED) error = HA_ERR_KEY_NOT_FOUND;
    goto end;
  }

  DBUG_PRINT("info", ("found first matching record"));
  DBUG_DUMP("record[0]", m_table->record[0], m_table->s->reclength);
  /*
    Below is a minor "optimization".  If the key (i.e., key number
    0) has the HA_NOSAME flag set, we know that we have found the
    correct record (since there can be no duplicates); otherwise, we
    have to compare the record with the one found to see if it is
    the correct one.

    CAVEAT! This behaviour is essential for the replication of,
    e.g., the mysql.proc table since the correct record *shall* be
    found using the primary key *only*.  There shall be no
    comparison of non-PK columns to decide if the correct record is
    found.  I can see no scenario where it would be incorrect to
    chose the row to change only using a PK or an UNNI.
  */
  if (m_key_info->flags & HA_NOSAME || m_key_index == m_table->s->primary_key) {
    /* Unique does not have non nullable part */
    if (!(m_key_info->flags & (HA_NULL_PART_KEY)))
      goto end;  // record found
    else {
      /*
        Unique has nullable part. We need to check if there is any field in the
        BI image that is null and part of UNNI.
      */
      bool null_found = false;
      for (uint i = 0; i < m_key_info->user_defined_key_parts && !null_found;
           i++) {
        uint fieldnr = m_key_info->key_part[i].fieldnr - 1;
        Field **f = m_table->field + fieldnr;
        null_found = (*f)->is_null();
      }

      if (!null_found) goto end;  // record found

      /* else fall through to index scan */
    }
  }

  /*
    In case key is not unique, we still have to iterate over records found
    and find the one which is identical to the row given. A copy of the
    record we are looking for is stored in record[1].
   */
  DBUG_PRINT("info", ("non-unique index, scanning it to find matching record"));

  while (record_compare(m_table, tabledef, &this->m_local_cols)) {
    while ((error = next_record_scan(false))) {
      /* We just skip records that has already been deleted */
      if (error == HA_ERR_RECORD_DELETED) continue;
      DBUG_PRINT("info", ("no record matching the given row found"));
      goto end;
    }
  }

end:

  DBUG_ASSERT(error != HA_ERR_RECORD_DELETED);

  // Compare with BI of the relay log (which we stored in record[1])
  bool potential_bi_inconsistency =
      (!error && opt_slave_check_before_image_consistency &&
       rbr_exec_mode != RBR_EXEC_MODE_IDEMPOTENT &&
       !(m_table->file->ha_table_flags() & HA_READ_BEFORE_WRITE_REMOVAL) &&
       !m_table->file->last_part_has_ttl_column() &&
       record_compare(m_table, tabledef, &m_cols));

  DBUG_EXECUTE_IF("dbg.fire_bi_inconsistency",
                  potential_bi_inconsistency = true;);

  if (unlikely(potential_bi_inconsistency)) {
    before_image_mismatch mismatch_info;
    char gtid[Gtid_specification::MAX_TEXT_LENGTH];
    global_sid_lock->rdlock();
    thd->variables.gtid_next.to_string(global_sid_map, gtid);
    global_sid_lock->unlock();
    mismatch_info.gtid = gtid;

    mismatch_info.table = std::string(m_table->s->db.str) + "." +
                          std::string(m_table->s->table_name.str);
    mismatch_info.log_pos = std::string(rli->get_rpl_log_name()) + ":" +
                            std::to_string(common_header->log_pos);
    std::tie(mismatch_info.source_img, mismatch_info.local_img) =
        calculate_diff(m_table, tabledef, &m_cols);

    if (likely(!mismatch_info.source_img.empty() &&
               !mismatch_info.local_img.empty())) {
      update_before_image_inconsistencies(mismatch_info);
      if (log_error_verbosity > 1) {
        std::string bi = mismatch_info.source_img;
        std::string li = mismatch_info.local_img;
        if (bi.length() > 256) {
          bi.resize(253);
          bi += "...";
        }
        if (li.length() > 256) {
          li.resize(253);
          li += "...";
        }
        sql_print_warning(
            "Slave before-image consistency check failed at "
            "position: %s (gtid: %s). "
            "Mismatch (table: %s): %s (source) vs. %s (local)",
            mismatch_info.log_pos.c_str(), mismatch_info.gtid.c_str(),
            mismatch_info.table.c_str(), bi.c_str(), li.c_str());
      }
      if (opt_slave_check_before_image_consistency ==
          Log_event::enum_check_before_image_consistency::BI_CHECK_ON) {
        error = HA_ERR_END_OF_FILE;
      }
    }
  }

  if (error && error != HA_ERR_RECORD_DELETED)
    m_table->file->print_error(error, MYF(0));
  else
    error = do_apply_row(rli, tabledef);

  if (!error)
    error = close_record_scan();
  else
    /*
      we are already with errors. Keep the error code and
      try to close the scan anyway.
    */
    (void)close_record_scan();

  int unpack_error = skip_after_image_for_update_event(rli, saved_m_curr_row);
  if (!error) error = unpack_error;

  m_table->default_column_bitmaps();
  return error;
}

int Update_rows_log_event::skip_after_image_for_update_event(
    const Relay_log_info *rli, const uchar *curr_bi_start) {
  if (m_curr_row == curr_bi_start && m_curr_row_end != nullptr) {
    /*
      This handles the case that the BI was read successfully, but an
      error happened while looking up the row.  In this case, the AI
      has not been read, so the read position is between the two
      images.  In case the error is idempotent, we need to move the
      position to the end of the row, and therefore we skip past the
      AI.

      The normal behavior is:

      When unpack_row reads a row image, and there is no error,
      unpack_row sets m_curr_row_end to point to the end of the image,
      and leaves m_curr_row to point at the beginning.

      The AI is read from Update_rows_log_event::do_exec_row. Before
      calling unpack_row, do_exec_row sets m_curr_row=m_curr_row_end,
      so that it actually reads the AI. And again, if there is no
      error, unpack_row sets m_curr_row_end to point to the end of the
      AI.

      Thus, the positions are moved as follows:

                          +--------------+--------------+
                          | BI           | AI           |  NULL
                          +--------------+--------------+
      0. Initial values   ^m_curr_row                      ^m_curr_row_end
      1. Read BI, no error
                          ^m_curr_row    ^m_curr_row_end
      2. Lookup BI
      3. Set m_curr_row
                                         ^m_curr_row
                                         ^m_curr_row_end
      4. Read AI, no error
                                         ^m_curr_row    ^m_curr_row_end

      If an error happened while reading the BI (e.g. corruption),
      then we should not try to read the AI here.  Therefore we do not
      read the AI if m_curr_row_end==NULL.

      If an error happened while looking up BI, then we should try to
      read AI here. Then we know m_curr_row_end points to beginning of
      AI, so we come here, set m_curr_row=m_curr_row_end, and read the
      AI.

      If an error happened while reading the AI, then we should not
      try to read the AI again.  Therefore we do not read the AI if
      m_curr_row==curr_bi_start.
    */
    m_curr_row = m_curr_row_end;
    return unpack_current_row(rli, &m_cols_ai, true /*is AI*/,
                              true /*only_seek*/);
  }
  return 0;
}

int Rows_log_event::do_hash_row(Relay_log_info const *rli) {
  DBUG_TRACE;
  DBUG_ASSERT(m_table && m_table->in_use != nullptr);
  int error = 0;

  /* create an empty entry to add to the hash table */
  HASH_ROW_ENTRY *entry = m_hash.make_entry();

  /* Prepare the record, unpack and save positions. */
  entry->positions->bi_start = m_curr_row;  // save the bi start pos
  prepare_record(m_table, &this->m_local_cols, false);
  if ((error = unpack_current_row(rli, &m_cols, false /*is not AI*/))) {
    hash_slave_rows_free_entry freer;
    freer(entry);
    goto end;
  }
  entry->positions->bi_ends = m_curr_row_end;  // save the bi end pos

  /*
    Now that m_table->record[0] is filled in, we can add the entry
    to the hash table. Note that the put operation calculates the
    key based on record[0] contents (including BLOB fields).
   */
  m_hash.put(m_table, &this->m_local_cols, entry);

  if (m_key_index < MAX_KEY) add_key_to_distinct_keyset();

  /*
    We need to unpack the AI to advance the positions, so we
    know when we have reached m_rows_end and that we do not
    unpack the AI in the next iteration as if it was a BI.
  */
  if (get_general_type_code() == binary_log::UPDATE_ROWS_EVENT) {
    /* Save a copy of the BI. */
    store_record(m_table, record[1]);

    /*
     This is the situation after hashing the BI:

     ===|=== before image ====|=== after image ===|===
        ^                     ^
        m_curr_row            m_curr_row_end
   */

    /* Set the position to the start of the record to be unpacked. */
    m_curr_row = m_curr_row_end;

    /* We shouldn't need this, but lets not leave loose ends */
    prepare_record(m_table, &this->m_local_cols, false);
    error =
        unpack_current_row(rli, &m_cols_ai, true /*is AI*/, true /*only_seek*/);

    /*
      This is the situation after unpacking the AI:

      ===|=== before image ====|=== after image ===|===
                               ^                   ^
                               m_curr_row          m_curr_row_end
    */

    /* Restore back the copy of the BI. */
    restore_record(m_table, record[1]);
  }

end:
  return error;
}

int Rows_log_event::do_scan_and_update(Relay_log_info const *rli,
                                       table_def *tabledef) {
  DBUG_TRACE;
  DBUG_ASSERT(m_table && m_table->in_use != nullptr);
  DBUG_ASSERT(m_hash.is_empty() == false);
  TABLE *table = m_table;
  int error = 0;
  const uchar *saved_last_m_curr_row = nullptr;
  const uchar *saved_last_m_curr_row_end = nullptr;
  /* create an empty entry to add to the hash table */
  HASH_ROW_ENTRY *entry = nullptr;
  int idempotent_errors = 0;
  int i = 0;

  saved_last_m_curr_row = m_curr_row;
  saved_last_m_curr_row_end = m_curr_row_end;

  DBUG_PRINT("info", ("Hash was populated with %d records!", m_hash.size()));

  /* open table or index depending on whether we have set m_key_index or not. */
  if ((error = open_record_scan())) goto err;

  /*
     Scan the table only once and compare against entries in hash.
     When a match is found, apply the changes.
   */
  do {
    /* get the next record from the table */
    error = next_record_scan(i == 0);
    i++;

    if (error) DBUG_PRINT("info", ("error: %s", HA_ERR(error)));
    switch (error) {
      case 0: {
        entry = m_hash.get(table, &this->m_local_cols);
        /**
          The do..while loop takes care of the scenario of same row being
          updated more than once within a single Update_rows_log_event by
          performing the hash lookup for the updated_row(by taking the AI stored
          in table->record[0] after the ha_update_row()) when table has no
          primary key.

          This can happen when update is called from a stored function.
          Ex:
            CREATE FUNCTION f1 () RETURNS INT BEGIN
            UPDATE t1 SET a = 2 WHERE a = 1;
            UPDATE t1 SET a = 3 WHERE a = 2;
            RETURN 0;
            END
        */
        do {
          store_record(table, record[1]);

          /**
             If there are collisions we need to be sure that this is
             indeed the record we want.  Loop through all records for
             the given key and explicitly compare them against the
             record we got from the storage engine.
           */
          while (entry) {
            m_curr_row = entry->positions->bi_start;
            m_curr_row_end = entry->positions->bi_ends;

            prepare_record(table, &this->m_local_cols, false);
            if ((error = unpack_current_row(rli, &m_cols, false /*is not AI*/)))
              goto close_table;

            if (record_compare(table, tabledef, &this->m_local_cols))
              m_hash.next(&entry);
            else
              break;  // we found a match
          }

          /**
             We found the entry we needed, just apply the changes.
           */
          if (entry) {
            // just to be safe, copy the record from the SE to table->record[0]
            restore_record(table, record[1]);

            /**
               At this point, both table->record[0] and
               table->record[1] have the SE row that matched the one
               in the hash table.

               Thence if this is a DELETE we wouldn't need to mess
               around with positions anymore, but since this can be an
               update, we need to provide positions so that AI is
               unpacked correctly to table->record[0] in UPDATE
              implementation of do_exec_row().
            */
            m_curr_row = entry->positions->bi_start;
            m_curr_row_end = entry->positions->bi_ends;

            /* we don't need this entry anymore, just delete it */
            if ((error = m_hash.del(entry))) goto err;

            if ((error = do_apply_row(rli, tabledef))) {
              if (handle_idempotent_and_ignored_errors(rli, &error))
                goto close_table;

              do_post_row_operations(rli, error);
            }
          }
        } while (this->get_type_code() == binary_log::UPDATE_ROWS_EVENT &&
                 table->s->primary_key >= MAX_KEY &&
                 (entry = m_hash.get(table, &m_cols)));
      } break;

      case HA_ERR_RECORD_DELETED:
        // get next
        continue;

      case HA_ERR_KEY_NOT_FOUND:
        /* If the slave exec mode is idempotent or the error is
            skipped error, then don't break */
        if (handle_idempotent_and_ignored_errors(rli, &error)) goto close_table;
        idempotent_errors++;
        continue;

      case HA_ERR_END_OF_FILE:
      default:
        // exception (hash is not empty and we have reached EOF or
        // other error happened)
        goto close_table;
    }
  }
  /**
    if the rbr_exec_mode is set to Idempotent, we cannot expect the hash to
    be empty. In such cases we count the number of idempotent errors and check
    if it is equal to or greater than the number of rows left in the hash.
   */
  while (((idempotent_errors < m_hash.size()) && !m_hash.is_empty()) &&
         (!error || (error == HA_ERR_RECORD_DELETED)));

close_table:
  DBUG_PRINT("info", ("m_hash.size()=%d error=%d idempotent_errors=%d",
                      m_hash.size(), error, idempotent_errors));
  if (error == HA_ERR_RECORD_DELETED) error = 0;

  if (error) {
    table->file->print_error(error, MYF(0));
    DBUG_PRINT("info", ("Failed to get next record"
                        " (ha_rnd_next returns %d)",
                        error));
    /*
      we are already with errors. Keep the error code and
      try to close the scan anyway.
    */
    (void)close_record_scan();
  } else
    error = close_record_scan();

err:

  if ((m_hash.is_empty() && !error) || (idempotent_errors >= m_hash.size())) {
    /**
       Reset the last positions, because the positions are lost while
       handling entries in the hash.
     */
    m_curr_row = saved_last_m_curr_row;
    m_curr_row_end = saved_last_m_curr_row_end;
  }

  return error;
}

int Rows_log_event::do_hash_scan_and_update(Relay_log_info const *rli,
                                            table_def *tabledef) {
  DBUG_TRACE;
  DBUG_ASSERT(m_table && m_table->in_use != nullptr);

  // HASHING PART

  /* unpack the BI (and AI, if it exists) and add it to the hash map. */
  if (int error = this->do_hash_row(rli)) return error;

  /* We have not yet hashed all rows in the buffer. Do not proceed to the SCAN
   * part. */
  if (m_curr_row_end < m_rows_end) return 0;

  DBUG_PRINT("info", ("Hash was populated with %d records!", m_hash.size()));
  DBUG_ASSERT(m_curr_row_end == m_rows_end);

  // SCANNING & UPDATE PART

  return this->do_scan_and_update(rli, tabledef);
}

int Rows_log_event::do_table_scan_and_update(Relay_log_info const *rli,
                                             table_def *tabledef) {
  int error = 0;
  const uchar *saved_m_curr_row = m_curr_row;
  TABLE *table = m_table;

  DBUG_TRACE;
  DBUG_ASSERT(m_curr_row != m_rows_end);
  DBUG_PRINT("info", ("locating record using table scan (ha_rnd_next)"));

  saved_m_curr_row = m_curr_row;

  /** unpack the before image */
  prepare_record(table, &this->m_local_cols, false);
  if (!(error = unpack_current_row(rli, &m_cols, false /*is not AI*/))) {
    /** save a copy so that we can compare against it later */
    store_record(m_table, record[1]);

    int restart_count = 0;  // Number of times scanning has restarted from top

    if ((error = m_table->file->ha_rnd_init(true))) {
      DBUG_PRINT("info", ("error initializing table scan"
                          " (ha_rnd_init returns %d)",
                          error));
      goto end;
    }

    /* Continue until we find the right record or have made a full loop */
    do {
    restart_ha_rnd_next:
      error = m_table->file->ha_rnd_next(m_table->record[0]);
      if (error) DBUG_PRINT("info", ("error: %s", HA_ERR(error)));
      switch (error) {
        case HA_ERR_END_OF_FILE:
          // restart scan from top
          if (++restart_count < 2) {
            if ((error = m_table->file->ha_rnd_init(true))) goto end;
            goto restart_ha_rnd_next;
          }
          break;

        case HA_ERR_RECORD_DELETED:
          // fetch next
          goto restart_ha_rnd_next;
        case 0:
          // we're good, check if record matches
          break;

        default:
          // exception
          goto end;
      }
    } while (restart_count < 2 &&
             record_compare(m_table, tabledef, &this->m_local_cols));
  }

end:

  DBUG_ASSERT(error != HA_ERR_RECORD_DELETED);

  /* either we report error or apply the changes */
  if (error && error != HA_ERR_RECORD_DELETED) {
    DBUG_PRINT("info", ("Failed to get next record"
                        " (ha_rnd_next returns %d)",
                        error));
    m_table->file->print_error(error, MYF(0));
  } else
    error = do_apply_row(rli, tabledef);

  if (!error)
    error = close_record_scan();
  else
    /*
      we are already with errors. Keep the error code and
      try to close the scan anyway.
    */
    (void)close_record_scan();

  int unpack_error = skip_after_image_for_update_event(rli, saved_m_curr_row);
  if (!error) error = unpack_error;

  table->default_column_bitmaps();
  return error;
}

void Rows_log_event::set_readwriteset_from_col_names(TABLE *table,
                                                     table_def *tabledef,
                                                     MY_BITMAP *after_image) {
  DBUG_ASSERT(tabledef->have_column_names());
  bitmap_clear_all(table->read_set);
  bitmap_clear_all(table->write_set);
  // use column names to find out the correct field indices
  // on slave's table.
  for (uint i = 0; i < tabledef->size(); ++i) {
    const char *col_name = tabledef->get_column_name(i);
    Field *const field = find_field_in_table_sef(table, col_name);

    // field may be NULL if the field is removed on slave.
    if (!field) continue;

    // handle write_set
    if (bitmap_is_set(after_image, i))
      bitmap_set_bit(table->write_set, field->field_index);
    // handle read_set
    if ((get_general_type_code() == binary_log::DELETE_ROWS_EVENT ||
         get_general_type_code() == binary_log::UPDATE_ROWS_EVENT) &&
        bitmap_is_set(&m_cols, i))
      bitmap_set_bit(table->read_set, field->field_index);
  }
}

/**
  Restores empty table list as it was before trigger processing

  @note We have a lot of ASSERTS that check the lists when we close tables
  There was the same problem with MERGE MYISAM tables and so here we try to
  go the same way
*/
static void restore_empty_query_table_list(LEX *lex) {
  if (lex->first_not_own_table())
    (*lex->first_not_own_table()->prev_global) = nullptr;
  lex->query_tables = nullptr;
  lex->query_tables_last = &lex->query_tables;
}

int Rows_log_event::force_write_to_binlog(Relay_log_info *rli) {
  int error = 0;

  // reset the row logging function flag
  thd->m_skip_row_logging_functions = false;

  reset_log_pos();
  // Table map events are necessary before every row event.
  if (write_locked_table_maps(thd)) return HA_ERR_RBR_LOGGING_FAILED;

  const uchar *saved_m_curr_row = m_curr_row;
  const uchar *saved_m_curr_row_end = m_curr_row_end;

  bool has_trans = thd->lex->sql_command == SQLCOM_CREATE_TABLE ||
                   m_table->file->has_transactions();

  // Figure out type conversions
  TABLE *conv_table = nullptr;
  if (!rli->tables_to_lock->m_tabledef.compatible_with(
          thd, const_cast<Relay_log_info *>(rli), rli->tables_to_lock->table,
          &conv_table)) {
    sql_print_error("Table schema is not compatible with master's at %s:%llu",
                    rli->get_rpl_log_name(), common_header->log_pos);
    return HA_ERR_GENERIC;
  }

  rli->tables_to_lock->m_conv_table = conv_table;

  m_table->mark_columns_per_binlog_row_image(
      thd, get_general_type_code() == binary_log::WRITE_ROWS_EVENT);

  m_curr_row = m_rows_buf;
  m_curr_row_end = NULL;
  uint i = 0;  // update event count
  while (m_curr_row != m_rows_end) {
    Log_event_type event_type = get_general_type_code();

    bool is_after_image = false;
    MY_BITMAP *cols = &m_cols;  // init with before image bitmap

    if (event_type == binary_log::WRITE_ROWS_EVENT) {
      is_after_image = true;
    }
    // NOTE: In updates, the after image follows the before image, hence
    // every odd index will be an after image
    else if (event_type == binary_log::UPDATE_ROWS_EVENT && i++ % 2 == 1) {
      cols = &m_cols_ai;
      is_after_image = true;
    }

    prepare_record(m_table, cols, false);

    if ((error = unpack_current_row(rli, cols, is_after_image))) goto end;

    // advance m_curr_row
    m_curr_row = m_curr_row_end;
    // write to binlog
    switch (event_type) {
      case binary_log::WRITE_ROWS_EVENT:
        error = Write_rows_log_event::binlog_row_logging_function(
            thd, m_table, has_trans, NULL, m_table->record[0]);
        break;
      case binary_log::UPDATE_ROWS_EVENT:
        if (is_after_image)
          error = Update_rows_log_event::binlog_row_logging_function(
              thd, m_table, has_trans, m_table->record[1], m_table->record[0]);
        else
          store_record(m_table, record[1]);
        break;
      case binary_log::DELETE_ROWS_EVENT:
        error = Delete_rows_log_event::binlog_row_logging_function(
            thd, m_table, has_trans, m_table->record[0], NULL);
        break;
      default:
        sql_print_error("Got event other than rows event at %s:%llu",
                        rli->get_rpl_log_name(), common_header->log_pos);
        error = HA_ERR_GENERIC;
    }
  }

end:
  m_curr_row = saved_m_curr_row;
  m_curr_row_end = saved_m_curr_row_end;
  return error;
}

/**
 * Checks if we can recover this during idempotent recovery
 *
 * @param rli           IN      Relay_log_info object
 * @param err_msg       OUT     The msg string for specific error encountered
 *
 * @return              True if all good, False otherwise
 */
bool Rows_log_event::can_use_idempotent_recovery(Relay_log_info const *rli,
                                                 std::string &err_msg) const {
  DBUG_ASSERT(m_table);

  // case: no idempotent recovery, no problem!
  if (!m_force_binlog_idempotent) return true;

  const std::string table_name = std::string(m_table->s->db.str) + "." +
                                 std::string(m_table->s->table_name.str);

  const std::string pos = std::string(rli->get_rpl_log_name()) + ":" +
                          std::to_string(common_header->log_pos);

  // case: idempotent recovery only works when we can reconstruct the entire row
  // from the event (i.e. binlog_row_image = COMPLETE/FULL)
  if (!bitmap_is_set_all(&m_cols)) {
    err_msg =
        "Found row event with incomplete column information. Check value "
        "of binlog_row_image on the master, only COMPLETE and FULL are "
        "supported when slave_use_idempotent_recovery is enabled. "
        "Log position: " +
        pos;
    return false;
  }

  // case: we can't handle foreign key references but we just print a warning
  // for now
  if (m_table->s->is_referenced_by_foreign_key()) {
    err_msg = "Cannot use idempotent recovery if table uses foreign keys: " +
              table_name + ". Log position: " + pos;
    sql_print_warning("%s", err_msg.c_str());
  }
  // case: if we are not using PK for lookup we abort immediately (except when
  // the engine is BLACKHOLE or no lookup is needed)
  if ((m_table->s->primary_key == MAX_KEY ||
       (m_rows_lookup_algorithm != ROW_LOOKUP_NOT_NEEDED &&
        m_key_index != m_table->s->primary_key)) &&
      strcmp(m_table->file->table_type(), "BLACKHOLE") != 0) {
    err_msg =
        "Found table without primary key while performing idempotent "
        "recovery: " +
        table_name + ". Log position: " + pos;
    return false;
  }
  return true;
}

int Rows_log_event::do_apply_event(Relay_log_info const *rli) {
  DBUG_TRACE;
  TABLE *table = nullptr;
  int error = 0;
  ulonglong init_timer = my_timer_now();

  /*
    'thd' has been set by exec_relay_log_event(), just before calling
    do_apply_event(). We still check here to prevent future coding
    errors.
  */
  DBUG_ASSERT(rli->info_thd == thd);

  /*
    If there is no locks taken, this is the first binrow event seen
    after the table map events.  We should then lock all the tables
    used in the transaction and proceed with execution of the actual
    event.
  */
  if (!thd->lock) {
    /*
      Lock_tables() reads the contents of thd->lex, so they must be
      initialized.

      We also call the mysql_reset_thd_for_next_command(), since this
      is the logical start of the next "statement". Note that this
      call might reset the value of current_stmt_binlog_format, so
      we need to do any changes to that value after this function.
    */
    lex_start(thd);
    mysql_reset_thd_for_next_command(thd);

    enum_gtid_statement_status state = gtid_pre_statement_checks(thd);
    if (state == GTID_STATEMENT_EXECUTE) {
      if (gtid_pre_statement_post_implicit_commit_checks(thd))
        state = GTID_STATEMENT_CANCEL;
    }

    if (state == GTID_STATEMENT_CANCEL) {
      uint mysql_error = thd->get_stmt_da()->mysql_errno();
      DBUG_ASSERT(mysql_error != 0);
      rli->report(ERROR_LEVEL, mysql_error, "Error executing row event: '%s'",
                  thd->get_stmt_da()->message_text());
      thd->is_slave_error = true;
      return -1;
    } else if (state == GTID_STATEMENT_SKIP)
      goto end;

    /*
      The current statement is just about to begin and
      has not yet modified anything. Note, all.modified is reset
      by mysql_reset_thd_for_next_command.
    */
    thd->get_transaction()->reset_unsafe_rollback_flags(Transaction_ctx::STMT);
    /*
      This is a row injection, so we flag the "statement" as
      such. Note that this code is called both when the slave does row
      injections and when the BINLOG statement is used to do row
      injections.
    */
    thd->lex->set_stmt_row_injection();

    /*
      There are a few flags that are replicated with each row event.
      Make sure to set/clear them before executing the main body of
      the event.
    */
    if (get_flags(NO_FOREIGN_KEY_CHECKS_F))
      thd->variables.option_bits |= OPTION_NO_FOREIGN_KEY_CHECKS;
    else
      thd->variables.option_bits &= ~OPTION_NO_FOREIGN_KEY_CHECKS;

    if (get_flags(RELAXED_UNIQUE_CHECKS_F))
      thd->variables.option_bits |= OPTION_RELAXED_UNIQUE_CHECKS;
    else
      thd->variables.option_bits &= ~OPTION_RELAXED_UNIQUE_CHECKS;

    thd->binlog_row_event_extra_data = m_extra_row_info.get_ndb_info();

    /* A small test to verify that objects have consistent types */
    DBUG_ASSERT(sizeof(thd->variables.option_bits) ==
                sizeof(OPTION_RELAXED_UNIQUE_CHECKS));

    DBUG_EXECUTE_IF("rows_log_event_before_open_table", {
      const char action[] =
          "now SIGNAL before_open_table WAIT_FOR go_ahead_sql";
      DBUG_ASSERT(!debug_sync_set_action(thd, STRING_WITH_LEN(action)));
    };);

    if (slave_run_triggers_for_rbr) {
      LEX *lex = thd->lex;
      const auto new_trg_event_map = get_trg_event_map();

      /*
        Trigger's procedures work with global table list. So we have to add
        rli->tables_to_lock content there to get trigger's in the list
        Then restore_empty_query_table_list() restore the list as it was
      */
      DBUG_ASSERT(lex->query_tables == nullptr);
      if ((lex->query_tables = rli->tables_to_lock))
        rli->tables_to_lock->prev_global = &lex->query_tables;
      for (TABLE_LIST *tables = rli->tables_to_lock; tables;
           tables = tables->next_global) {
        tables->trg_event_map = new_trg_event_map;
        lex->query_tables_last = &tables->next_global;
      }
    }

    if (open_and_lock_tables(thd, rli->tables_to_lock, 0)) {
      if (thd->is_error()) {
        uint actual_error = thd->get_stmt_da()->mysql_errno();
        if (ignored_error_code(actual_error)) {
          if (log_error_verbosity >= 2)
            rli->report(WARNING_LEVEL, actual_error,
                        "Error executing row event: '%s'",
                        thd->get_stmt_da()->message_text());
          thd->get_stmt_da()->reset_condition_info(thd);
          clear_all_errors(thd, const_cast<Relay_log_info *>(rli));
          error = 0;
          goto end;
        } else {
          rli->report(ERROR_LEVEL, actual_error,
                      "Error executing row event: '%s'",
                      thd->get_stmt_da()->message_text());
          thd->is_slave_error = true;
          error = actual_error;
          /* remove trigger's tables */
          goto err;
        }
      } else if (thd->is_killed()) {
        rli->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                    "Error executing row event: "
                    "the slave sql worker thread is killed");
        thd->is_slave_error = 1;
        error = ER_SLAVE_FATAL_ERROR;
        goto err;
      } else {
        rli->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                    "Error executing row event: "
                    "unexpected slave fatal error");
        thd->is_slave_error = 1;
        error = ER_SLAVE_FATAL_ERROR;
        goto err;
      }
      return 1;
    }

    /*
      When the open and locking succeeded, we check all tables to
      ensure that they still have the correct type.
    */

    {
      DBUG_PRINT("debug",
                 ("Checking compability of tables to lock - tables_to_lock: %p",
                  rli->tables_to_lock));

      /**
        When using RBR and MyISAM MERGE tables the base tables that make
        up the MERGE table can be appended to the list of tables to lock.

        Thus, we just check compatibility for those that tables that have
        a correspondent table map event (ie, those that are actually going
        to be accessed while applying the event). That's why the loop stops
        at rli->tables_to_lock_count .

        NOTE: The base tables are added here are removed when
              close_thread_tables is called.
       */
      TABLE_LIST *table_list_ptr = rli->tables_to_lock;
      for (uint i = 0; table_list_ptr && (i < rli->tables_to_lock_count);
           table_list_ptr = table_list_ptr->next_global, i++) {
        /*
          Below if condition takes care of skipping base tables that
          make up the MERGE table (which are added by open_tables()
          call). They are added next to the merge table in the list.
          For eg: If RPL_TABLE_LIST is t3->t1->t2 (where t1 and t2
          are base tables for merge table 't3'), open_tables will modify
          the list by adding t1 and t2 again immediately after t3 in the
          list (*not at the end of the list*). New table_to_lock list will
          look like t3->t1'->t2'->t1->t2 (where t1' and t2' are TABLE_LIST
          objects added by open_tables() call). There is no flag(or logic) in
          open_tables() that can skip adding these base tables to the list.
          So the logic here should take care of skipping them.

          tables_to_lock_count logic will take care of skipping base tables
          that are added at the end of the list.
          For eg: If RPL_TABLE_LIST is t1->t2->t3, open_tables will modify
          the list into t1->t2->t3->t1'->t2'. t1' and t2' will be skipped
          because tables_to_lock_count logic in this for loop.
        */
        if (table_list_ptr->parent_l) continue;
        /*
          We can use a down cast here since we know that every table added
          to the tables_to_lock is a RPL_TABLE_LIST (or child table which is
          skipped above).
        */
        RPL_TABLE_LIST *ptr = static_cast<RPL_TABLE_LIST *>(table_list_ptr);
        DBUG_ASSERT(ptr->m_tabledef_valid);
        TABLE *conv_table;
        if (!ptr->m_tabledef.compatible_with(thd,
                                             const_cast<Relay_log_info *>(rli),
                                             ptr->table, &conv_table)) {
          DBUG_PRINT("debug",
                     ("Table: %s.%s is not compatible with master",
                      ptr->table->s->db.str, ptr->table->s->table_name.str));
          if (thd->is_slave_error) {
            error = ERR_BAD_TABLE_DEF;
            /* remove trigger's tables */
            goto err;
          } else {
            thd->get_stmt_da()->reset_condition_info(thd);
            clear_all_errors(thd, const_cast<Relay_log_info *>(rli));
            error = 0;
            goto end;
          }
        }
        DBUG_PRINT("debug", ("Table: %s.%s is compatible with master"
                             " - conv_table: %p",
                             ptr->table->s->db.str,
                             ptr->table->s->table_name.str, conv_table));
        ptr->m_conv_table = conv_table;
      }
    }

    /*
      ... and then we add all the tables to the table map and but keep
      them in the tables to lock list.
     */
    TABLE_LIST *ptr = rli->tables_to_lock;
    for (uint i = 0; ptr && (i < rli->tables_to_lock_count);
         ptr = ptr->next_global, i++) {
      /*
        Please see comment in above 'for' loop to know the reason
        for this if condition
      */
      if (ptr->parent_l) continue;
      const_cast<Relay_log_info *>(rli)->m_table_map.set_table(ptr->table_id,
                                                               ptr->table);
      /*
        Following is passing flag about triggers on the server. The problem was
        to pass it between table map event and row event. I do it via extended
        TABLE_LIST (RPL_TABLE_LIST) but row event uses only TABLE so I need to
        find somehow the corresponding TABLE_LIST.
      */
      ptr->table->master_had_triggers =
          ((RPL_TABLE_LIST *)ptr)->master_had_triggers;
    }

    /*
      Validate applied binlog events with plugin requirements.
    */
    int out_value = 0;
    int hook_error =
        RUN_HOOK(binlog_relay_io, applier_log_event, (thd, out_value));
    if (hook_error || out_value) {
      char buf[256];
      uint applier_error = ER_APPLIER_LOG_EVENT_VALIDATION_ERROR;

      if (hook_error) {
        applier_error = ER_RUN_HOOK_ERROR;
        strcpy(buf, "applier_log_event");
      } else {
        if (!thd->owned_gtid_is_empty() && thd->owned_gtid.sidno > 0) {
          thd->owned_gtid.to_string(thd->owned_sid, buf);
        } else {
          strcpy(buf, "ANONYMOUS");
        }
      }

      if (thd->slave_thread) {
        rli->report(ERROR_LEVEL, applier_error,
                    ER_THD_NONCONST(thd, applier_error), buf);
        thd->is_slave_error = true;
      } else {
        /*
          For the cases in which a 'BINLOG' statement is set to
          execute in a user session
        */
        my_printf_error(applier_error, ER_THD_NONCONST(thd, applier_error),
                        MYF(0), buf);
      }
      /* remove trigger's tables */
      error = applier_error;
      goto err;
    }
  }

  table = m_table =
      const_cast<Relay_log_info *>(rli)->m_table_map.get_table(m_table_id);

  DBUG_PRINT("debug",
             ("m_table: %p, m_table_id: %llu%s", m_table, m_table_id.id(),
              (table && master_had_triggers ? " (master had triggers)" : "")));
  /*
    A row event comprising of a P_S table
    - should not be replicated (i.e executed) by the slave SQL thread.
    - should not be executed by the client in the  form BINLOG '...' stmts.
  */
  if (table && table->s->table_category == TABLE_CATEGORY_PERFORMANCE)
    table = nullptr;

  if (table) {
    this->m_fields.set_thd(thd).set_table(table);

    /*
     Translate received replicated column bitmaps into local table column
     bitmaps. This is needed when the table has columns that are to be excluded
     from replication - hidden generated columns, for instance.
    */
    this->m_fields.translate_bitmap(this->m_cols, this->m_local_cols);
    if (this->m_cols.bitmap != this->m_cols_ai.bitmap)
      this->m_fields.translate_bitmap(this->m_cols_ai, this->m_local_cols_ai);
    else
      this->m_local_cols_ai.bitmap = this->m_local_cols.bitmap;

    /*
      table == NULL means that this table should not be replicated
      (this was set up by Table_map_log_event::do_apply_event()
      which tested replicate-* rules).
    */

    Applier_security_context_guard security_context{rli, thd};
    const char *privilege_missing = nullptr;
    if (!security_context.skip_priv_checks()) {
      std::vector<std::tuple<ulong, const TABLE *, Rows_log_event *>> l;
      switch (get_general_type_code()) {
        case binary_log::WRITE_ROWS_EVENT: {
          l.push_back(std::make_tuple(INSERT_ACL, this->m_table, this));
          if (!security_context.has_access(l)) {
            privilege_missing = "INSERT";
          }
          break;
        }
        case binary_log::DELETE_ROWS_EVENT: {
          l.push_back(std::make_tuple(DELETE_ACL, this->m_table, this));
          if (!security_context.has_access(l)) {
            privilege_missing = "DELETE";
          }
          break;
        }
        case binary_log::UPDATE_ROWS_EVENT:
        case binary_log::PARTIAL_UPDATE_ROWS_EVENT: {
          l.push_back(std::make_tuple(UPDATE_ACL, this->m_table, this));
          if (!security_context.has_access(l)) {
            privilege_missing = "UPDATE";
          }
          break;
        }
        default: {
          DBUG_ASSERT(false);
        }
      }
    }
    if (privilege_missing != nullptr) {
      rli->report(ERROR_LEVEL, ER_TABLEACCESS_DENIED_ERROR,
                  ER_THD(thd, ER_TABLEACCESS_DENIED_ERROR), privilege_missing,
                  security_context.get_username().data(),
                  security_context.get_hostname().data(),
                  table->s->table_name.str);
      return ER_TABLEACCESS_DENIED_ERROR;
    }

    bool no_columns_to_update = false;
    master_had_triggers = table->master_had_triggers;
    // set the database
    LEX_CSTRING thd_db;
    LEX_CSTRING current_db_name_saved = thd->db();
    thd_db.str = table->s->db.str;
    thd_db.length = table->s->db.length;
    thd->reset_db(thd_db);
    PSI_stage_info *stage = nullptr;

    /*
      It's not needed to set_time() but
      1) it continues the property that "Time" in SHOW PROCESSLIST shows how
      much slave is behind
      2) it will be needed when we allow replication from a table with no
      TIMESTAMP column to a table with one.
      So we call set_time(), like in SBR. Presently it changes nothing.
    */
    thd->set_time(&(common_header->when));

    thd->binlog_row_event_extra_data = m_extra_row_info.get_ndb_info();

    /*
      Now we are in a statement and will stay in a statement until we
      see a STMT_END_F.

      We set this flag here, before actually applying any rows, in
      case the SQL thread is stopped and we need to detect that we're
      inside a statement and halting abruptly might cause problems
      when restarting.
     */
    const_cast<Relay_log_info *>(rli)->set_flag(Relay_log_info::IN_STMT);

    if (m_width == this->m_fields.filtered_size() && bitmap_is_set_all(&m_cols))
      set_flags(COMPLETE_ROWS_F);

    /*
      Set tables write and read sets.

      Read_set contains all slave columns (in case we are going to fetch
      a complete record from slave)

      Write_set equals the m_cols bitmap sent from master but it can be
      longer if slave has extra columns.
    */

    bitmap_set_all(table->read_set);
    bitmap_set_all(table->write_set);

    table_def *tabledef = NULL;
    TABLE *conv_table = NULL;
    const auto ret MY_ATTRIBUTE((unused)) =
        rli->get_table_data(table, &tabledef, &conv_table);
    DBUG_ASSERT(ret);

    /*
      Call mark_generated_columns() to set read_set/write_set bits of the
      virtual generated columns as required in order to get these computed.
      This is needed since all columns need to have a value in the before
      image for the record when doing the update (some storage engines will
      use this for maintaining of secondary indexes). This call is required
      even for DELETE events to set write_set bit in order to satisfy
      ASSERTs in Field_*::store functions.

      binlog_prepare_row_image() function, which will be called from
      binlogging functions (binlog_update_row() and binlog_delete_row())
      will take care of removing these spurious fields required during
      execution but not needed for binlogging. In case of inserts, there
      are no spurious fields (all generated columns are required to be written
      into the binlog).
    */
    switch (get_general_type_code()) {
      case binary_log::DELETE_ROWS_EVENT:
        if (tabledef->use_column_names(table))
          set_readwriteset_from_col_names(table, tabledef, &m_cols);
        else
          bitmap_intersect(table->read_set, &this->m_local_cols);
        stage = &stage_rpl_apply_row_evt_delete;
        if (m_table->vfield) m_table->mark_generated_columns(false);
        break;
      case binary_log::UPDATE_ROWS_EVENT:
        if (tabledef->use_column_names(table))
          set_readwriteset_from_col_names(table, tabledef, &m_cols_ai);
        else {
          bitmap_intersect(table->write_set, &this->m_local_cols_ai);
          bitmap_intersect(table->read_set, &this->m_local_cols);
        }
        if (m_table->vfield) m_table->mark_generated_columns(true);
        /* Skip update rows events that don't have data for this server's table.
         */
        if (!is_any_column_signaled_for_table(
                table, tabledef, &this->m_local_cols_ai, &m_cols_ai))
          no_columns_to_update = true;
        stage = &stage_rpl_apply_row_evt_update;
        break;
      case binary_log::WRITE_ROWS_EVENT:
        /*
          For 'WRITE_ROWS_EVENT, the execution order for 'mark_generated_rows()'
          and bitset intersection between 'write_set' and 'm_cols', is inverted.
          This behaviour is necessary due to an inconsistency, between storage
          engines, regarding the 'm_cols' bitset and generated columns: while
          non-NDB engines always include the generated columns for write-rows
          events, NDB doesnot if not necessary. The previous execution order
          would set all generated columns bits to '1' in 'write_set', since
          'mark_generated_columns()' is expecting that every column is present
          in the log event. This would break replication of generated columns
          for NDB.

          For engines that include every column in write-rows events, this order
          makes no difference, assuming that the master uses the same engine,
          since the master will include all the bits in the image.

          For use-cases that use different storage engines, specifically NDB
          and some other, this order may break replication due to the
          differences in behaviour regarding generated columns bits, in
          wrote-rows event bitsets. This issue should be further addressed by
          storage engines handlers, by converging behaviour regarding such use
          cases.
        */
        /* WRITE ROWS EVENTS store the bitmap in the m_cols bitmap */
        if (m_table->vfield) m_table->mark_generated_columns(false);
        if (tabledef->use_column_names(table))
          set_readwriteset_from_col_names(table, tabledef, &m_cols);
        else
          bitmap_intersect(table->write_set, &this->m_local_cols);

        stage = &stage_rpl_apply_row_evt_write;
        break;
      default:
        DBUG_ASSERT(false);
    }

    if (!m_force_binlog_idempotent) {
      if (thd->slave_thread)  // fix the mode for slave
        this->rbr_exec_mode = slave_exec_mode_options;
      else  // set the mode for user thread
        this->rbr_exec_mode = thd->variables.rbr_exec_mode_options;
    }

    // Do event specific preparations
    error = do_before_row_operations(rli, tabledef);

    std::string idempotent_recovery_err_msg;
    if (!can_use_idempotent_recovery(rli, idempotent_recovery_err_msg)) {
      rli->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                  ER_THD(thd, ER_SLAVE_FATAL_ERROR),
                  idempotent_recovery_err_msg.c_str());
      error = ER_SLAVE_FATAL_ERROR;
      thd->reset_db(current_db_name_saved);
      goto err;
    }

    /*
      Bug#56662 Assertion failed: next_insert_id == 0, file handler.cc
      Don't allow generation of auto_increment value when processing
      rows event by setting 'MODE_NO_AUTO_VALUE_ON_ZERO'. The exception
      to this rule happens when the auto_inc column exists on some
      extra columns on the slave. In that case, do not force
      MODE_NO_AUTO_VALUE_ON_ZERO.
    */
    sql_mode_t saved_sql_mode = thd->variables.sql_mode;
    if (!is_auto_inc_in_extra_columns())
      thd->variables.sql_mode = MODE_NO_AUTO_VALUE_ON_ZERO;

    // row processing loop

    /*
      set the initial time of this ROWS statement if it was not done
      before in some other ROWS event.
     */
    const_cast<Relay_log_info *>(rli)->set_row_stmt_start_timestamp();

    const uchar *saved_m_curr_row = m_curr_row;

    int (Rows_log_event::*do_apply_row_ptr)(Relay_log_info const *,
                                            table_def *tabledef) = nullptr;

    /**
       Skip update rows events that don't have data for this slave's
       table.
     */
    if (no_columns_to_update) goto AFTER_MAIN_EXEC_ROW_LOOP;

    /**
       If there are no columns marked in the read_set for this table,
       that means that we cannot lookup any row using the available BI
       in the binarr log. Thence, we immediatly raise an error:
       HA_ERR_END_OF_FILE.
     */

    if ((m_rows_lookup_algorithm != ROW_LOOKUP_NOT_NEEDED) &&
        !is_any_column_signaled_for_table(table, tabledef, &this->m_local_cols,
                                          &m_cols)) {
      error = HA_ERR_END_OF_FILE;
      goto AFTER_MAIN_EXEC_ROW_LOOP;
    }
    switch (m_rows_lookup_algorithm) {
      case ROW_LOOKUP_HASH_SCAN:
        do_apply_row_ptr = &Rows_log_event::do_hash_scan_and_update;
        break;

      case ROW_LOOKUP_INDEX_SCAN:
        do_apply_row_ptr = &Rows_log_event::do_index_scan_and_update;
        break;

      case ROW_LOOKUP_TABLE_SCAN:
        do_apply_row_ptr = &Rows_log_event::do_table_scan_and_update;
        break;

      case ROW_LOOKUP_NOT_NEEDED:
        DBUG_ASSERT(
            get_general_type_code() == binary_log::WRITE_ROWS_EVENT ||
            ((get_general_type_code() == binary_log::DELETE_ROWS_EVENT ||
              get_general_type_code() == binary_log::UPDATE_ROWS_EVENT) &&
             m_table->file->use_read_free_rpl()));

        /* No need to scan for rows, just apply it */
        do_apply_row_ptr = &Rows_log_event::do_apply_row;
        break;

      default:
        DBUG_ASSERT(0);
        error = 1;
        goto AFTER_MAIN_EXEC_ROW_LOOP;
        break;
    }

    DBUG_ASSERT(stage != nullptr);
    THD_STAGE_INFO(thd, *stage);

#ifdef HAVE_PSI_STAGE_INTERFACE
    m_psi_progress.set_progress(mysql_set_stage(stage->m_key));
#endif

    do {
      DBUG_PRINT("info", ("calling do_apply_row_ptr"));

      error = (this->*do_apply_row_ptr)(rli, tabledef);

      if (handle_idempotent_and_ignored_errors(rli, &error)) break;

      /* this advances m_curr_row */
      do_post_row_operations(rli, error);

    } while (!error && (m_curr_row != m_rows_end));

#ifdef HAVE_PSI_STAGE_INTERFACE
    m_psi_progress.end_work();
#endif

  AFTER_MAIN_EXEC_ROW_LOOP:

    if (saved_m_curr_row != m_curr_row && !table->file->has_transactions()) {
      /*
        Usually, the trans_commit_stmt() propagates unsafe_rollback_flags
        from statement to transaction level. However, we cannot rely on
        this when row format is in use as several events can be processed
        before calling this function. This happens because it is called
        only when the latest event generated by a statement is processed.

        There are however upper level functions that execute per event
        and check transaction's status. So if the unsafe_rollback_flags
        are not propagated here, this can lead to errors.

        For example, a transaction that updates non-transactional tables
        may be stopped in the middle thus leading to inconsistencies
        after a restart.
      */
      thd->get_transaction()->mark_modified_non_trans_table(
          Transaction_ctx::STMT);
      thd->get_transaction()->merge_unsafe_rollback_flags();
    }

    /*
      Restore the sql_mode after the rows event is processed.
    */
    thd->variables.sql_mode = saved_sql_mode;

    { /*
          The following failure injecion works in cooperation with tests
          setting @@global.debug= 'd,stop_slave_middle_group'.
          The sql thread receives the killed status and will proceed
          to shutdown trying to finish incomplete events group.
      */
      DBUG_EXECUTE_IF(
          "stop_slave_middle_group",
          if (thd->get_transaction()->cannot_safely_rollback(
                  Transaction_ctx::SESSION)) const_cast<Relay_log_info *>(rli)
              ->abort_slave = 1;);
    }

    if ((error = do_after_row_operations(rli, error)) &&
        ignored_error_code(convert_handler_error(error, thd, table))) {
      slave_rows_error_report(
          INFORMATION_LEVEL, error, rli, thd, table, get_type_str(),
          const_cast<Relay_log_info *>(rli)->get_rpl_log_name(),
          (ulong)common_header->log_pos);
      thd->get_stmt_da()->reset_condition_info(thd);
      clear_all_errors(thd, const_cast<Relay_log_info *>(rli));
      error = 0;
    }

    // Idempotent recovery is enabled so we log this event
    // NOTE: Idempotent recovery requries tables have uniqure key to work
    // correctly NOTE: error will be set to 0 in
    // handle_idempotent_and_ignored_errors() if there was an idempotent error
    if (m_force_binlog_idempotent && !error) {
      DBUG_ASSERT(rbr_exec_mode == RBR_EXEC_MODE_IDEMPOTENT);
      DBUG_ASSERT(thd->is_enabled_idempotent_recovery());

      // reset all the pending rows that might have been added while applying
      // the row events, we will directly write the relay log event
      thd->binlog_reset_pending_rows_event(m_table->file->has_transactions());
      error = force_write_to_binlog(const_cast<Relay_log_info *>(rli));
    }
    // reset back the db
    thd->reset_db(current_db_name_saved);
  }  // if (table)

  command_slave_seconds += my_timer_since(init_timer);

  if (error) {
    slave_rows_error_report(
        ERROR_LEVEL, error, rli, thd, table, get_type_str(),
        const_cast<Relay_log_info *>(rli)->get_rpl_log_name(),
        (ulong)common_header->log_pos);
    /*
      @todo We should probably not call
      reset_current_stmt_binlog_format_row() from here.
      /Sven
    */
    thd->reset_current_stmt_binlog_format_row();
    thd->is_slave_error = true;
    /* remove trigger's tables */
    goto err;
  }

end:
  /* remove trigger's tables */
  if (slave_run_triggers_for_rbr) restore_empty_query_table_list(thd->lex);

  if (get_flags(STMT_END_F)) {
    if ((error = rows_event_stmt_cleanup(rli, thd))) {
      if (table)
        slave_rows_error_report(
            ERROR_LEVEL, thd->is_error() ? 0 : error, rli, thd, table,
            get_type_str(),
            const_cast<Relay_log_info *>(rli)->get_rpl_log_name(),
            (ulong)common_header->log_pos);
      else {
        rli->report(
            ERROR_LEVEL,
            thd->is_error() ? thd->get_stmt_da()->mysql_errno() : error,
            "Error in cleaning up after an event of type:%s; %s; the group"
            " log file/position: %s %lu",
            get_type_str(),
            thd->is_error() ? thd->get_stmt_da()->message_text()
                            : "unexpected error",
            const_cast<Relay_log_info *>(rli)->get_rpl_log_name(),
            (ulong)common_header->log_pos);
      }
    } else if (m_force_binlog_idempotent) {
      // This is end row update in the current statement.
      thd->clear_binlog_table_maps();
    }
    /* We are at end of the statement (STMT_END_F flag), lets clean
      the memory which was used from thd's mem_root now.
      This needs to be done only if we are here in SQL thread context.
      In other flow ( in case of a regular thread which can happen
      when the thread is applying BINLOG'...' row event) we should
      *not* try to free the memory here. It will be done latter
      in dispatch_command() after command execution is completed.
     */
    if (thd->slave_thread) free_root(thd->mem_root, MYF(MY_KEEP_PREALLOC));
    mysql_mutex_lock(&thd->LOCK_thd_query);
    thd->row_query.clear();
    mysql_mutex_unlock(&thd->LOCK_thd_query);
  }
  return error;

err:
  if (slave_run_triggers_for_rbr) restore_empty_query_table_list(thd->lex);
  const_cast<Relay_log_info *>(rli)->slave_close_thread_tables(thd);
  return error;
}

Log_event::enum_skip_reason Rows_log_event::do_shall_skip(Relay_log_info *rli) {
  /*
    If the slave skip counter is 1 and this event does not end a
    statement, then we should not start executing on the next event.
    Otherwise, we defer the decision to the normal skipping logic.
  */
  if (rli->slave_skip_counter == 1 && !get_flags(STMT_END_F))
    return Log_event::EVENT_SKIP_IGNORE;
  else
    return Log_event::do_shall_skip(rli);
}

/**
   The function is called at Rows_log_event statement commit time,
   normally from Rows_log_event::do_update_pos() and possibly from
   Query_log_event::do_apply_event() of the COMMIT.
   The function commits the last statement for engines, binlog and
   releases resources have been allocated for the statement.

   @retval  0         Ok.
   @retval  non-zero  Error at the commit.
 */

static int rows_event_stmt_cleanup(Relay_log_info const *rli, THD *thd) {
  DBUG_TRACE;
  DBUG_EXECUTE_IF("simulate_rows_event_cleanup_failure", {
    char errbuf[MYSQL_ERRMSG_SIZE];
    int err = 149;
    my_error(ER_ERROR_DURING_COMMIT, MYF(0), err,
             my_strerror(errbuf, MYSQL_ERRMSG_SIZE, err));
    return 1;
  });
  int error;
  {
    /*
      This is the end of a statement or transaction, so close (and
      unlock) the tables we opened when processing the
      Table_map_log_event starting the statement.

      OBSERVER.  This will clear *all* mappings, not only those that
      are open for the table. There is not good handle for on-close
      actions for tables.

      NOTE. Even if we have no table ('table' == 0) we still need to be
      here, so that we increase the group relay log position. If we didn't, we
      could have a group relay log position which lags behind "forever"
      (assume the last master's transaction is ignored by the slave because of
      replicate-ignore rules).
    */
    error = thd->binlog_flush_pending_rows_event(true);

    /*
      If this event is not in a transaction, the call below will, if some
      transactional storage engines are involved, commit the statement into
      them and flush the pending event to binlog.
      If this event is in a transaction, the call will do nothing, but a
      Xid_log_event will come next which will, if some transactional engines
      are involved, commit the transaction and flush the pending event to the
      binlog.
      If there was a deadlock the transaction should have been rolled back
      already. So there should be no need to rollback the transaction.
    */
    DBUG_ASSERT(!thd->transaction_rollback_request);
    error |= (error ? trans_rollback_stmt(thd) : trans_commit_stmt(thd));

    /*
      Now what if this is not a transactional engine? we still need to
      flush the pending event to the binlog; we did it with
      thd->binlog_flush_pending_rows_event(). Note that we imitate
      what is done for real queries: a call to
      ha_autocommit_or_rollback() (sometimes only if involves a
      transactional engine), and a call to be sure to have the pending
      event flushed.
    */

    /*
      @todo We should probably not call
      reset_current_stmt_binlog_format_row() from here.

      Btw, the previous comment about transactional engines does not
      seem related to anything that happens here.
      /Sven
    */
    thd->reset_current_stmt_binlog_format_row();

    const_cast<Relay_log_info *>(rli)->cleanup_context(thd, false);

    /*
      Clean sql_command value
    */
    thd->lex->sql_command = SQLCOM_END;
  }
  return error;
}

/**
   The method either increments the relay log position or
   commits the current statement and increments the master group
   possition if the event is STMT_END_F flagged and
   the statement corresponds to the autocommit query (i.e replicated
   without wrapping in BEGIN/COMMIT)

   @retval 0         Success
   @retval non-zero  Error in the statement commit
 */
int Rows_log_event::do_update_pos(Relay_log_info *rli) {
  DBUG_TRACE;
  int error = 0;

  DBUG_PRINT("info", ("flags: %s", get_flags(STMT_END_F) ? "STMT_END_F " : ""));

  /* Worker does not execute binlog update position logics */
  DBUG_ASSERT(!is_mts_worker(rli->info_thd));

  if (get_flags(STMT_END_F)) {
    /*
      Indicate that a statement is finished.
      Step the group log position if we are not in a transaction,
      otherwise increase the event log position.
    */
    error = rli->stmt_done(common_header->log_pos);
  } else {
    rli->inc_event_relay_log_pos();
  }

  return error;
}

bool Rows_log_event::write_data_header(Basic_ostream *ostream) {
  uchar
      buf[Binary_log_event::ROWS_HEADER_LEN_V2];  // No need to init the buffer
  DBUG_ASSERT(m_table_id.is_valid());
  DBUG_EXECUTE_IF("old_row_based_repl_4_byte_map_id_master", {
    int4store(buf + 0, (ulong)m_table_id.id());
    int2store(buf + 4, m_flags);
    return (wrapper_my_b_safe_write(ostream, buf, 6));
  });
  int6store(buf + ROWS_MAPID_OFFSET, m_table_id.id());
  int2store(buf + ROWS_FLAGS_OFFSET, m_flags);
  if (likely(!log_bin_use_v1_row_events)) {
    /*
       v2 event, with variable header portion.
       Determine length of variable header payload(extra_row_info part)
    */
    uint extra_row_info_payloadlen = EXTRA_ROW_INFO_HEADER_LENGTH;
    if (m_extra_row_info.have_ndb_info()) {
      extra_row_info_payloadlen +=
          (EXTRA_ROW_INFO_TYPECODE_LENGTH + m_extra_row_info.get_ndb_length());
      ;
    }

    if (m_extra_row_info.have_part()) {
      extra_row_info_payloadlen +=
          (EXTRA_ROW_INFO_TYPECODE_LENGTH + m_extra_row_info.get_part_length());
    }
    /* Var-size header len includes len itself */
    int2store(buf + ROWS_VHLEN_OFFSET, extra_row_info_payloadlen);
    if (wrapper_my_b_safe_write(ostream, buf,
                                Binary_log_event::ROWS_HEADER_LEN_V2))
      return true;

    /* Write var-sized payload, if any */
    if (m_extra_row_info.have_ndb_info()) {
      /* Add tag and extra row info */
      uint8 type_code = static_cast<uint8>(enum_extra_row_info_typecode::NDB);
      if (wrapper_my_b_safe_write(ostream, &(type_code),
                                  EXTRA_ROW_INFO_TYPECODE_LENGTH))
        return true;
      if (wrapper_my_b_safe_write(ostream, m_extra_row_info.get_ndb_info(),
                                  m_extra_row_info.get_ndb_length()))
        return true;
    }
    if (m_extra_row_info.have_part()) {
      uint8 type_code;
      type_code = static_cast<uint8>(enum_extra_row_info_typecode::PART);
      uchar partition_buf[5];
      uint8 extra_part_info_data_len = 0;
      partition_buf[extra_part_info_data_len++] = type_code;

      // partition_id occupies less than 2 bytes
      // in all the cases because of the current range of allowed number
      // of partitions 8192 for non-ndb and 12288 for ndb.
      // So while writing the partition_id it is okay to use 2 bytes for it.

      int write_partition_id = m_extra_row_info.get_partition_id();
      int2store(partition_buf + extra_part_info_data_len,
                static_cast<uint16>(write_partition_id));
      extra_part_info_data_len += EXTRA_ROW_PART_INFO_VALUE_LENGTH;

      if (get_general_type_code() == binary_log::UPDATE_ROWS_EVENT) {
        write_partition_id = m_extra_row_info.get_source_partition_id();
        int2store(partition_buf + extra_part_info_data_len,
                  static_cast<uint16>(write_partition_id));
        extra_part_info_data_len += EXTRA_ROW_PART_INFO_VALUE_LENGTH;
      }

      if (wrapper_my_b_safe_write(ostream, partition_buf,
                                  extra_part_info_data_len))
        return true;
    }
  } else {
    if (wrapper_my_b_safe_write(ostream, buf,
                                Binary_log_event::ROWS_HEADER_LEN_V1))
      return true;
  }

  return false;
}

bool Rows_log_event::write_data_body(Basic_ostream *ostream) {
  /*
     Note that this should be the number of *bits*, not the number of
     bytes.
  */
  uchar sbuf[sizeof(m_width) + 1];
  ptrdiff_t const data_size = m_rows_cur - m_rows_buf;
  bool res = false;
  uchar *const sbuf_end = net_store_length(sbuf, (size_t)m_width);
  DBUG_ASSERT(static_cast<size_t>(sbuf_end - sbuf) <= sizeof(sbuf));

  DBUG_DUMP("m_width", sbuf, (size_t)(sbuf_end - sbuf));
  res =
      res || wrapper_my_b_safe_write(ostream, sbuf, (size_t)(sbuf_end - sbuf));

  DBUG_DUMP("m_cols", (uchar *)m_cols.bitmap, no_bytes_in_map(&m_cols));
  res = res || wrapper_my_b_safe_write(ostream, (uchar *)m_cols.bitmap,
                                       no_bytes_in_map(&m_cols));
  /*
    TODO[refactor write]: Remove the "down cast" here (and elsewhere).
   */
  if (get_general_type_code() == binary_log::UPDATE_ROWS_EVENT) {
    DBUG_DUMP("m_cols_ai", (uchar *)m_cols_ai.bitmap,
              no_bytes_in_map(&m_cols_ai));
    res = res || wrapper_my_b_safe_write(ostream, (uchar *)m_cols_ai.bitmap,
                                         no_bytes_in_map(&m_cols_ai));
  }
  DBUG_DUMP("rows", m_rows_buf, data_size);
  res = res || wrapper_my_b_safe_write(ostream, m_rows_buf, (size_t)data_size);

  return res;
}

int Rows_log_event::pack_info(Protocol *protocol) {
  char buf[256];
  std::string flagstr;
  if (m_flags) flagstr = get_enum_flag_string();

  size_t bytes = snprintf(buf, sizeof(buf), "table_id: %llu%s", m_table_id.id(),
                          flagstr.c_str());
  protocol->store_string(buf, bytes, &my_charset_bin);
  return 0;
}
#endif  // MYSQL_SERVER

#ifndef MYSQL_SERVER
void Rows_log_event::print_helper(FILE *,
                                  PRINT_EVENT_INFO *print_event_info) const {
  IO_CACHE *const head = &print_event_info->head_cache;
  IO_CACHE *const body = &print_event_info->body_cache;
  if (!print_event_info->short_form) {
    std::string flag_str = get_enum_flag_string();

    bool const last_stmt_event = get_flags(STMT_END_F);
    print_header(head, print_event_info, !last_stmt_event);
    my_b_printf(head, "\t%s: table id %llu%s\n", get_type_str(),
                m_table_id.id(), m_flags ? flag_str.c_str() : "");
    print_base64(body, print_event_info, !last_stmt_event);
  }
}
#endif

/**************************************************************************
        Table_map_log_event member functions and support functions
**************************************************************************/

/**
  @ingroup Replication

  @page PAGE_RPL_FIELD_METADATA How replication of field metadata works.

  When a table map is created, the master first calls
  Table_map_log_event::save_field_metadata() which calculates how many
  values will be in the field metadata. Only those fields that require the
  extra data are added. The method also loops through all of the fields in
  the table calling the method Field::save_field_metadata() which returns the
  values for the field that will be saved in the metadata and replicated to
  the slave. Once all fields have been processed, the table map is written to
  the binlog adding the size of the field metadata and the field metadata to
  the end of the body of the table map.

  When a table map is read on the slave, the field metadata is read from the
  table map and passed to the table_def class constructor which saves the
  field metadata from the table map into an array based on the type of the
  field. Field metadata values not present (those fields that do not use extra
  data) in the table map are initialized as zero (0). The array size is the
  same as the columns for the table on the slave.

  Additionally, values saved for field metadata on the master are saved as a
  string of bytes (uchar) in the binlog. A field may require 1 or more bytes
  to store the information. In cases where values require multiple bytes
  (e.g. values > 255), the endian-safe methods are used to properly encode
  the values on the master and decode them on the slave. When the field
  metadata values are captured on the slave, they are stored in an array of
  type uint. This allows the least number of casts to prevent casting bugs
  when the field metadata is used in comparisons of field attributes. When
  the field metadata is used for calculating addresses in pointer math, the
  type used is uint32.
*/

#if defined(MYSQL_SERVER)
/**
  Save the field metadata based on the real_type of the field.
  The metadata saved depends on the type of the field. Some fields
  store a single byte for pack_length() while others store two bytes
  for field_length (max length).

  @retval  0  Ok.

  @todo
  We may want to consider changing the encoding of the information.
  Currently, the code attempts to minimize the number of bytes written to
  the tablemap. There are at least two other alternatives; 1) using
  net_store_length() to store the data allowing it to choose the number of
  bytes that are appropriate thereby making the code much easier to
  maintain (only 1 place to change the encoding), or 2) use a fixed number
  of bytes for each field. The problem with option 1 is that net_store_length()
  will use one byte if the value < 251, but 3 bytes if it is > 250. Thus,
  for fields like CHAR which can be no larger than 255 characters, the method
  will use 3 bytes when the value is > 250. Further, every value that is
  encoded using 2 parts (e.g., pack_length, field_length) will be numerically
  > 250 therefore will use 3 bytes for eah value. The problem with option 2
  is less wasteful for space but does waste 1 byte for every field that does
  not encode 2 parts.
*/
int Table_map_log_event::save_field_metadata() {
  DBUG_TRACE;
  int index = 0;
  for (auto it = this->m_fields.begin(); it != this->m_fields.end(); ++it) {
    Field *field = *it;
    DBUG_PRINT("debug", ("field_type: %d", m_coltype[it.filtered_pos()]));
    index += field->save_field_metadata(&m_field_metadata[index]);

    DBUG_EXECUTE_IF("inject_invalid_blob_size", {
      if (m_coltype[it.filtered_pos()] == MYSQL_TYPE_BLOB)
        m_field_metadata[index - 1] = 5;
    });
  }
  return index;
}

/*
  Constructor used to build an event for writing to the binary log.
  Mats says tbl->s lives longer than this event so it's ok to copy pointers
  (tbl->s->db etc) and not pointer content.
 */

Table_map_log_event::Table_map_log_event(THD *thd_arg, TABLE *tbl,
                                         const Table_id &tid, bool using_trans)
    : binary_log::Table_map_event(
          tid,
          tbl->s->fields +
              DBUG_EVALUATE_IF("binlog_omit_last_column_from_table_map_event",
                               -1, 0),
          (tbl->s->db.str), ((tbl->s->db.str) ? tbl->s->db.length : 0),
          (tbl->s->table_name.str), (tbl->s->table_name.length)),
      Log_event(thd_arg, 0,
                using_trans ? Log_event::EVENT_TRANSACTIONAL_CACHE
                            : Log_event::EVENT_STMT_CACHE,
                Log_event::EVENT_NORMAL_LOGGING, header(), footer()),
      m_fields{tbl, Replicated_columns_view::OUTBOUND} {
  common_header->type_code = binary_log::TABLE_MAP_EVENT;
  m_table = tbl;
  m_flags = TM_BIT_LEN_EXACT_F;
  if (binlog_row_metadata != BINLOG_ROW_METADATA_FACEBOOK) {
    m_flags |= TM_METADATA_NOT_FB_FORMAT_F;
    m_fb_format = false;
  }

  this->m_colcnt =
      this->m_fields.filtered_size() +
      DBUG_EVALUATE_IF("binlog_omit_last_column_from_table_map_event", -1, 0);

  uchar cbuf[sizeof(m_colcnt) + 1];
  uchar *cbuf_end;
  DBUG_ASSERT(m_table_id.is_valid());
  /*
    In TABLE_SHARE, "db" and "table_name" are 0-terminated (see this comment in
    table.cc / alloc_table_share():
      Use the fact the key is db/0/table_name/0
    As we rely on this let's assert it.
  */
  DBUG_ASSERT((tbl->s->db.str == nullptr) ||
              (tbl->s->db.str[tbl->s->db.length] == 0));
  DBUG_ASSERT(tbl->s->table_name.str[tbl->s->table_name.length] == 0);

  // Trigger changes are not logged if sql_log_bin_triggers is FALSE.
  // In this case, slaves should execute triggers while applying row events.
  if (tbl->triggers && thd->variables.sql_log_bin_triggers)
    m_flags |= TM_BIT_HAS_TRIGGERS_F;

  m_data_size = Binary_log_event::TABLE_MAP_HEADER_LEN;
  DBUG_EXECUTE_IF("old_row_based_repl_4_byte_map_id_master", m_data_size = 6;);
  m_data_size += m_dblen + 2;   // Include length and terminating \0
  m_data_size += m_tbllen + 2;  // Include length and terminating \0
  cbuf_end = net_store_length(cbuf, (size_t)m_colcnt);
  DBUG_ASSERT(static_cast<size_t>(cbuf_end - cbuf) <= sizeof(cbuf));
  m_data_size += (cbuf_end - cbuf) + m_colcnt;  // COLCNT and column types

  m_coltype = (uchar *)my_malloc(key_memory_log_event, m_colcnt, MYF(MY_WME));

  DBUG_ASSERT(m_colcnt ==
              this->m_fields.filtered_size() +
                  DBUG_EVALUATE_IF(
                      "binlog_omit_last_column_from_table_map_event", -1, 0));

  for (auto it = this->m_fields.begin();
       it != this->m_fields.end() &&
       DBUG_EVALUATE_IF("binlog_omit_last_column_from_table_map_event",
                        it.filtered_pos() != this->m_colcnt, true);
       ++it) {
    Field *field = *it;
    m_coltype[it.filtered_pos()] = field->binlog_type();
  }
  DBUG_EXECUTE_IF("inject_invalid_column_type", m_coltype[1] = 230;);

  /*
    Calculate a bitmap for the results of maybe_null() for all columns.
    The bitmap is used to determine when there is a column from the master
    that is not on the slave and is null and thus not in the row data during
    replication.
  */
  uint num_null_bytes = (m_colcnt + 7) / 8;
  m_data_size += num_null_bytes;

  // m_null_bits and m_sign_bits has the same size
  m_sign_bits_size = num_null_bytes;

  /*
    m_null_bits is a pointer indicating which columns can have a null value
    in a particular table.
  */
  m_null_bits =
      (uchar *)my_malloc(key_memory_log_event, num_null_bytes, MYF(MY_WME));

  m_field_metadata =
      (uchar *)my_malloc(key_memory_log_event, (m_colcnt * 4), MYF(MY_WME));
  memset(m_field_metadata, 0, (m_colcnt * 4));

  common_header->set_is_valid(m_null_bits != nullptr &&
                              m_field_metadata != nullptr &&
                              m_coltype != nullptr);
  /*
    Create an array for the field metadata and store it.
  */
  m_field_metadata_size = save_field_metadata();
  DBUG_ASSERT(m_field_metadata_size <= (m_colcnt * 4));

  /*
    Now set the size of the data to the size of the field metadata array
    plus one or three bytes (see pack.c:net_store_length) for number of
    elements in the field metadata array.
  */
  if (m_field_metadata_size < 251)
    m_data_size += m_field_metadata_size + 1;
  else
    m_data_size += m_field_metadata_size + 3;

  memset(m_null_bits, 0, num_null_bytes);
  Bit_writer bit_writer{this->m_null_bits};
  for (auto field : this->m_fields) bit_writer.set(field->is_nullable());
  /*
    Marking event to require sequential execution in MTS
    if the query might have updated FK-referenced db.
    Unlike Query_log_event where this fact is encoded through
    the accessed db list in the Table_map case m_flags is exploited.
  */
  uchar dbs = thd_arg->get_binlog_accessed_db_names()
                  ? thd_arg->get_binlog_accessed_db_names()->elements
                  : 0;
  if (dbs == 1) {
    char *db_name = thd_arg->get_binlog_accessed_db_names()->head();
    if (!strcmp(db_name, "")) m_flags |= TM_REFERRED_FK_DB_F;
  }

  init_metadata_fields();

  if (binlog_row_metadata == BINLOG_ROW_METADATA_FACEBOOK) {
    DBUG_ASSERT(m_fb_format && m_metadata_buf.length() == 0);
    if (m_primary_key_fields_size < 251)
      m_data_size += 1;
    else
      m_data_size += 3;
    m_data_size +=
        m_primary_key_fields_size + m_column_names_size + m_sign_bits_size;
  } else {
    m_data_size += m_metadata_buf.length();
  }

  if (enable_binlog_hlc && maintain_database_hlc && !m_dbnam.empty())
    thd->databases.insert(m_dbnam);
}
#endif /* defined(MYSQL_SERVER) */

/*
  Constructor used by slave to read the event from the binary log.
 */
Table_map_log_event::Table_map_log_event(
    const char *buf, const Format_description_event *description_event)
    : binary_log::Table_map_event(buf, description_event),
      Log_event(header(), footer())
#ifdef MYSQL_SERVER
      ,
      m_table(nullptr),
      m_fields(Replicated_columns_view::INBOUND)
#endif
{
  DBUG_TRACE;
  DBUG_ASSERT(header()->type_code == binary_log::TABLE_MAP_EVENT);
}

Table_map_log_event::~Table_map_log_event() {}

/*
  Return value is an error code, one of:

      -1     Failure to open table   [from open_tables()]
       0     Success
       1     No room for more tables [from set_table()]
       2     Out of memory           [from set_table()]
       3     Wrong table definition
       4     Daisy-chaining RBR with SBR not possible
 */

#if defined(MYSQL_SERVER)

enum enum_tbl_map_status {
  /* no duplicate identifier found */
  OK_TO_PROCESS = 0,

  /* this table map must be filtered out */
  FILTERED_OUT = 1,

  /* identifier mapping table with different properties */
  SAME_ID_MAPPING_DIFFERENT_TABLE = 2,

  /* a duplicate identifier was found mapping the same table */
  SAME_ID_MAPPING_SAME_TABLE = 3,

  /*
    this table must be filtered out but found an active XA transaction. XA
    transactions shouldn't be used with replication filters, until disabling
    the XA read only optimization is a supported feature.
  */
  FILTERED_WITH_XA_ACTIVE = 4
};

/*
  Checks if this table map event should be processed or not. First
  it checks the filtering rules, and then looks for duplicate identifiers
  in the existing list of rli->tables_to_lock.

  It checks that there hasn't been any corruption by verifying that there
  are no duplicate entries with different properties.

  In some cases, some binary logs could get corrupted, showing several
  tables mapped to the same table_id, 0 (see: BUG#56226). Thus we do this
  early sanity check for such cases and avoid that the server crashes
  later.

  In some corner cases, the master logs duplicate table map events, i.e.,
  same id, same database name, same table name (see: BUG#37137). This is
  different from the above as it's the same table that is mapped again
  to the same identifier. Thus we cannot just check for same ids and
  assume that the event is corrupted we need to check every property.

  NOTE: in the event that BUG#37137 ever gets fixed, this extra check
        will still be valid because we would need to support old binary
        logs anyway.

  @param rli The relay log info reference.
  @param table_list A list element containing the table to check against.
  @return OK_TO_PROCESS
            if there was no identifier already in rli->tables_to_lock

          FILTERED_OUT
            if the event is filtered according to the filtering rules

          SAME_ID_MAPPING_DIFFERENT_TABLE
            if the same identifier already maps a different table in
            rli->tables_to_lock

          SAME_ID_MAPPING_SAME_TABLE
            if the same identifier already maps the same table in
            rli->tables_to_lock.
*/
static enum_tbl_map_status check_table_map(Relay_log_info const *rli,
                                           RPL_TABLE_LIST *table_list) {
  DBUG_TRACE;
  enum_tbl_map_status res = OK_TO_PROCESS;

  if (rli->info_thd->slave_thread /* filtering is for slave only */ &&
      (!rli->rpl_filter->db_ok(table_list->db) ||
       (rli->rpl_filter->is_on() &&
        !rli->rpl_filter->tables_ok("", table_list))))
    if (rli->info_thd->get_transaction()->xid_state()->has_state(
            XID_STATE::XA_ACTIVE))
      res = FILTERED_WITH_XA_ACTIVE;
    else
      res = FILTERED_OUT;
  else {
    RPL_TABLE_LIST *ptr = static_cast<RPL_TABLE_LIST *>(rli->tables_to_lock);
    for (uint i = 0; ptr && (i < rli->tables_to_lock_count);
         ptr = static_cast<RPL_TABLE_LIST *>(ptr->next_local), i++) {
      if (ptr->table_id == table_list->table_id) {
        if (strcmp(ptr->db, table_list->db) ||
            strcmp(ptr->alias, table_list->table_name) ||
            ptr->lock_descriptor().type !=
                TL_WRITE)  // the ::do_apply_event always sets TL_WRITE
          res = SAME_ID_MAPPING_DIFFERENT_TABLE;
        else
          res = SAME_ID_MAPPING_SAME_TABLE;

        break;
      }
    }
  }

  DBUG_PRINT("debug", ("check of table map ended up with: %u", res));

  return res;
}

int Table_map_log_event::do_apply_event(Relay_log_info const *rli) {
  RPL_TABLE_LIST *table_list;
  char *db_mem, *tname_mem;
  const char *ptr;
  size_t dummy_len;
  void *memory;
  DBUG_TRACE;
  DBUG_ASSERT(rli->info_thd == thd);

  /* Step the query id to mark what columns that are actually used. */
  thd->set_query_id(next_query_id());

  if (!(memory =
            my_multi_malloc(key_memory_log_event, MYF(MY_WME), &table_list,
                            sizeof(RPL_TABLE_LIST), &db_mem, (uint)NAME_LEN + 1,
                            &tname_mem, (uint)NAME_LEN + 1, NullS)))
    return HA_ERR_OUT_OF_MEM;

  my_stpcpy(db_mem, m_dbnam.c_str());
  my_stpcpy(tname_mem, m_tblnam.c_str());

  if (lower_case_table_names) {
    my_casedn_str(system_charset_info, db_mem);
    my_casedn_str(system_charset_info, tname_mem);
  }

  /* rewrite rules changed the database */
  if (rli->rpl_filter != nullptr &&
      ((ptr = rli->rpl_filter->get_rewrite_db(db_mem, &dummy_len)) != db_mem)) {
    rli->rpl_filter->get_rewrite_db_statistics()->increase_counter();
    my_stpcpy(db_mem, ptr);
  }

  new (table_list) RPL_TABLE_LIST(db_mem, strlen(db_mem), tname_mem,
                                  strlen(tname_mem), tname_mem, TL_WRITE);

  table_list->table_id = DBUG_EVALUATE_IF(
      "inject_tblmap_same_id_maps_diff_table", 0, m_table_id.id());
  table_list->updating = true;
  table_list->required_type = dd::enum_table_type::BASE_TABLE;
  table_list->master_had_triggers = m_flags & TM_BIT_HAS_TRIGGERS_F;
  DBUG_PRINT(
      "debug",
      ("table: %s is mapped to %llu%s", table_list->table_name,
       table_list->table_id.id(),
       (table_list->master_had_triggers ? " (master had triggers)" : "")));

  enum_tbl_map_status tblmap_status = check_table_map(rli, table_list);
  if (tblmap_status == OK_TO_PROCESS) {
    DBUG_ASSERT(thd->lex->query_tables != table_list);

    /*
      Use placement new to construct the table_def instance in the
      memory allocated for it inside table_list.

      The memory allocated by the table_def structure (i.e., not the
      memory allocated *for* the table_def structure) is released
      inside Relay_log_info::clear_tables_to_lock() by calling the
      table_def destructor explicitly.
    */
    new (&table_list->m_tabledef) table_def(
        m_coltype, m_colcnt, m_field_metadata, m_field_metadata_size,
        m_null_bits, m_flags, m_column_names, m_column_names_size, m_sign_bits);

    table_list->m_tabledef_valid = true;
    table_list->m_conv_table = nullptr;
    table_list->open_type = OT_BASE_ONLY;

    /*
      We record in the slave's information that the table should be
      locked by linking the table into the list of tables to lock.
    */
    table_list->next_global = table_list->next_local = rli->tables_to_lock;
    const_cast<Relay_log_info *>(rli)->tables_to_lock = table_list;
    const_cast<Relay_log_info *>(rli)->tables_to_lock_count++;
    /* 'memory' is freed in clear_tables_to_lock */
  } else  // FILTERED_OUT, SAME_ID_MAPPING_*
  {
    if (tblmap_status == FILTERED_WITH_XA_ACTIVE) {
      if (thd->slave_thread)
        rli->report(ERROR_LEVEL, ER_XA_REPLICATION_FILTERS, "%s",
                    ER_THD(thd, ER_XA_REPLICATION_FILTERS));
      else
        /*
          For the cases in which a 'BINLOG' statement is set to
          execute in a user session
         */
        my_printf_error(ER_XA_REPLICATION_FILTERS, "%s", MYF(0),
                        ER_THD(thd, ER_XA_REPLICATION_FILTERS));
    }
    /*
      If mapped already but with different properties, we raise an
      error.
      If mapped already but with same properties we skip the event.
      If filtered out we skip the event.

      In all three cases, we need to free the memory previously
      allocated.
     */
    else if (tblmap_status == SAME_ID_MAPPING_DIFFERENT_TABLE) {
      /*
        Something bad has happened. We need to stop the slave as strange things
        could happen if we proceed: slave crash, wrong table being updated, ...
        As a consequence we push an error in this case.
       */

      char buf[256];

      snprintf(buf, sizeof(buf),
               "Found table map event mapping table id %llu which "
               "was already mapped but with different settings.",
               table_list->table_id.id());

      if (thd->slave_thread)
        rli->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                    ER_THD(thd, ER_SLAVE_FATAL_ERROR), buf);
      else
        /*
          For the cases in which a 'BINLOG' statement is set to
          execute in a user session
         */
        my_printf_error(ER_BINLOG_FATAL_ERROR,
                        ER_THD(thd, ER_BINLOG_FATAL_ERROR), MYF(0), buf);
    }

    my_free(memory);
  }

  return tblmap_status == SAME_ID_MAPPING_DIFFERENT_TABLE;
}

Log_event::enum_skip_reason Table_map_log_event::do_shall_skip(
    Relay_log_info *rli) {
  /*
    If the slave skip counter is 1, then we should not start executing
    on the next event.
  */
  return continue_group(rli);
}

int Table_map_log_event::do_update_pos(Relay_log_info *rli) {
  rli->inc_event_relay_log_pos();
  return 0;
}

bool Table_map_log_event::write_data_header(Basic_ostream *ostream) {
  DBUG_ASSERT(m_table_id.is_valid());
  uchar buf[Binary_log_event::TABLE_MAP_HEADER_LEN];
  DBUG_EXECUTE_IF("old_row_based_repl_4_byte_map_id_master", {
    int4store(buf + 0, static_cast<uint32>(m_table_id.id()));
    int2store(buf + 4, m_flags);
    return (wrapper_my_b_safe_write(ostream, buf, 6));
  });
  int6store(buf + TM_MAPID_OFFSET, m_table_id.id());
  int2store(buf + TM_FLAGS_OFFSET, m_flags);
  return (wrapper_my_b_safe_write(ostream, buf,
                                  Binary_log_event::TABLE_MAP_HEADER_LEN));
}

bool Table_map_log_event::write_data_body(Basic_ostream *ostream) {
  DBUG_ASSERT(!m_dbnam.empty());
  DBUG_ASSERT(!m_tblnam.empty());
  /* We use only one byte per length for storage in event: */
  DBUG_ASSERT(m_dblen <= 128);
  DBUG_ASSERT(m_tbllen <= 128);

  uchar const dbuf[] = {(uchar)m_dblen};
  uchar const tbuf[] = {(uchar)m_tbllen};

  uchar cbuf[sizeof(m_colcnt) + 1];
  uchar *const cbuf_end = net_store_length(cbuf, (size_t)m_colcnt);
  DBUG_ASSERT(static_cast<size_t>(cbuf_end - cbuf) <= sizeof(cbuf));

  /*
    Store the size of the field metadata.
  */
  uchar mbuf[sizeof(m_field_metadata_size)];
  uchar *const mbuf_end = net_store_length(mbuf, m_field_metadata_size);

  return (wrapper_my_b_safe_write(ostream, dbuf, sizeof(dbuf)) ||
          wrapper_my_b_safe_write(ostream, (const uchar *)m_dbnam.c_str(),
                                  m_dblen + 1) ||
          wrapper_my_b_safe_write(ostream, tbuf, sizeof(tbuf)) ||
          wrapper_my_b_safe_write(ostream, (const uchar *)m_tblnam.c_str(),
                                  m_tbllen + 1) ||
          wrapper_my_b_safe_write(ostream, cbuf, (size_t)(cbuf_end - cbuf)) ||
          wrapper_my_b_safe_write(ostream, m_coltype, m_colcnt) ||
          wrapper_my_b_safe_write(ostream, mbuf, (size_t)(mbuf_end - mbuf)) ||
          wrapper_my_b_safe_write(ostream, m_field_metadata,
                                  m_field_metadata_size) ||
          wrapper_my_b_safe_write(ostream, m_null_bits, (m_colcnt + 7) / 8) ||
          (m_fb_format ? write_fb_format_metadata(ostream)
                       : wrapper_my_b_safe_write(
                             ostream, (const uchar *)m_metadata_buf.ptr(),
                             m_metadata_buf.length())));
}

bool Table_map_log_event::write_fb_format_metadata(Basic_ostream *ostream) {
  DBUG_ASSERT(m_fb_format);
  uchar m_size_buf[sizeof(m_primary_key_fields_size)];
  uchar *const m_size_buf_end =
      net_store_length(m_size_buf, m_primary_key_fields_size);
  return (
      wrapper_my_b_safe_write(ostream, m_size_buf,
                              (size_t)(m_size_buf_end - m_size_buf)) ||
      wrapper_my_b_safe_write(ostream, m_primary_key_fields,
                              m_primary_key_fields_size) ||
      wrapper_my_b_safe_write(ostream, m_sign_bits, m_sign_bits_size) ||
      wrapper_my_b_safe_write(ostream, m_column_names, m_column_names_size));
}

/**
   stores an integer into packed format.

   @param[out] str_buf  a buffer where the packed integer will be stored.
   @param[in] length  the integer will be packed.
 */
static inline void store_compressed_length(String &str_buf, ulonglong length) {
  // Store Type and packed length
  uchar buf[4];
  uchar *buf_ptr = net_store_length(buf, length);

  str_buf.append(reinterpret_cast<char *>(buf), buf_ptr - buf);
}

/**
  Write data into str_buf with Type|Length|Value(TLV) format.

  @param[out] str_buf a buffer where the field is stored.
  @param[in] type  type of the field
  @param[in] length  length of the field value
  @param[in] value  value of the field
*/
static inline bool write_tlv_field(
    String &str_buf,
    enum Table_map_log_event::Optional_metadata_field_type type, uint length,
    const uchar *value) {
  /* type is stored in one byte, so it should never bigger than 255. */
  DBUG_ASSERT(static_cast<int>(type) <= 255);
  str_buf.append((char)type);
  store_compressed_length(str_buf, length);
  return str_buf.append(reinterpret_cast<const char *>(value), length);
}

/**
  Write data into str_buf with Type|Length|Value(TLV) format.

  @param[out] str_buf a buffer where the field is stored.
  @param[in] type  type of the field
  @param[in] value  value of the field
*/
static inline bool write_tlv_field(
    String &str_buf,
    enum Table_map_log_event::Optional_metadata_field_type type,
    const String &value) {
  return write_tlv_field(str_buf, type, value.length(),
                         reinterpret_cast<const uchar *>(value.ptr()));
}
#endif  // MYSQL_SERVER

#ifndef MYSQL_SERVER
// For MYSQL_SERVER, the version in field.h is used.
static inline bool is_numeric_type(uint type) {
  switch (type) {
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_LONGLONG:
    case MYSQL_TYPE_NEWDECIMAL:
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE:
      return true;
    default:
      return false;
  }
  return false;
}
#endif  // !MYSQL_SERVER

static inline bool is_character_type(uint type) {
  switch (type) {
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_BLOB:
      return true;
    default:
      return false;
  }
}

static inline bool is_enum_or_set_type(uint type) {
  return type == MYSQL_TYPE_ENUM || type == MYSQL_TYPE_SET;
}

#ifdef MYSQL_SERVER
static inline bool is_numeric_field(const Field *field) {
  return is_numeric_type(field->binlog_type());
}

static inline bool is_character_field(const Field *field) {
  return is_character_type(field->real_type());
}

static inline bool is_enum_field(const Field *field) {
  return field->real_type() == MYSQL_TYPE_ENUM;
}

static inline bool is_set_field(const Field *field) {
  return field->real_type() == MYSQL_TYPE_SET;
}

static inline bool is_enum_or_set_field(const Field *field) {
  return is_enum_or_set_type(field->real_type());
}

static inline bool is_geometry_field(const Field *field) {
  return field->real_type() == MYSQL_TYPE_GEOMETRY;
}

void Table_map_log_event::init_metadata_fields() {
  DBUG_TRACE;
  DBUG_EXECUTE_IF("simulate_no_optional_metadata", return;);

  if (binlog_row_metadata == BINLOG_ROW_METADATA_FACEBOOK) {
    if (init_fb_format_pk_fields()) {
      m_primary_key_fields_size = 0;
      m_primary_key_fields = nullptr;
    }
    if (init_fb_format_sign_bits()) {
      m_sign_bits_size = 0;
      m_sign_bits = nullptr;
    }
    if (init_fb_format_col_names()) {
      m_column_names_size = 0;
      m_column_names = nullptr;
    }
    m_metadata_buf.length(0);
    return;
  }

  if (init_signedness_field() ||
      init_charset_field(&is_character_field, DEFAULT_CHARSET,
                         COLUMN_CHARSET) ||
      init_geometry_type_field()) {
    m_metadata_buf.length(0);
    return;
  }

  if (binlog_row_metadata == BINLOG_ROW_METADATA_FULL) {
    if (DBUG_EVALUATE_IF("dont_log_column_name", 0, init_column_name_field()) ||
        init_charset_field(&is_enum_or_set_field, ENUM_AND_SET_DEFAULT_CHARSET,
                           ENUM_AND_SET_COLUMN_CHARSET) ||
        init_set_str_value_field() || init_enum_str_value_field() ||
        init_primary_key_field()) {
      m_metadata_buf.length(0);
    }
  }
}

bool Table_map_log_event::init_fb_format_pk_fields() {
  KEY *pkey_info = NULL;
  // Validate that there exists a valid primary key
  // and calculate the space required to store primary key
  // column indexes.
  if (m_table->key_info && m_table->s->primary_key < MAX_KEY) {
    pkey_info = m_table->key_info + m_table->s->primary_key;
    // see net_store_length()
    if (pkey_info->user_defined_key_parts < 251)
      m_primary_key_fields_size += 1;
    else
      m_primary_key_fields_size += 3;
    for (uint i = 0; i < pkey_info->user_defined_key_parts; ++i) {
      if ((pkey_info->key_part[i].fieldnr - 1) < 251)
        m_primary_key_fields_size += 1;
      else
        m_primary_key_fields_size += 3;
    }
  }

  if (m_primary_key_fields_size) {
    m_primary_key_fields =
        static_cast<unsigned char *>(bapi_malloc(m_primary_key_fields_size, 0));
    if (!m_primary_key_fields) {
      return true;
    }
    unsigned char *ptr = m_primary_key_fields;
    ptr = net_store_length(ptr, pkey_info->user_defined_key_parts);
    for (uint i = 0; i < pkey_info->user_defined_key_parts; ++i) {
      ptr = net_store_length(ptr, (pkey_info->key_part[i].fieldnr - 1));
    }
  }

  return false;
}

bool Table_map_log_event::init_fb_format_sign_bits() {
  if (m_sign_bits_size * 8 >= m_table->s->fields) {
    // allocate memory for sign bits
    m_sign_bits =
        static_cast<unsigned char *>(bapi_malloc(m_sign_bits_size, 0));
    if (!m_sign_bits) {
      return true;
    }
    memset(m_sign_bits, 0, m_sign_bits_size);
    DBUG_ASSERT(m_sign_bits_size <= m_table->s->fields);
    for (unsigned int i = 0; i < m_table->s->fields; ++i) {
      if (m_table->field[i]->flags & UNSIGNED_FLAG)
        m_sign_bits[(i / 8)] += 1 << (i % 8);
    }
  } else {
    m_sign_bits_size = 0;
  }
  return false;
}

bool Table_map_log_event::init_fb_format_col_names() {
  if (!m_table || !opt_log_column_names) {
    return false;
  }

  // get column_names size.
  for (unsigned int i = 0; i < m_table->s->fields; i++) {
    // + 1 for storing the length of the column name.
    // + 1 for '\0'
    m_column_names_size += strlen(m_table->s->field[i]->field_name) + 1 + 1;
  }

  if (m_column_names_size) {
    // allocate memory for column names.
    m_column_names =
        static_cast<unsigned char *>(bapi_malloc(m_column_names_size, 0));
    if (!m_column_names) {
      return true;
    }
  }

  uint index = 0;
  const auto share = m_table->s;
  for (uint i = 0; i < share->fields; i++) {
    uint length = strlen(share->field[i]->field_name) + 1;
    m_column_names[index++] = length;
    strcpy((char *)(m_column_names + index), share->field[i]->field_name);
    index += length;
  }

  return false;
}

bool Table_map_log_event::init_signedness_field() {
  /* use it to store signed flags, each numeric column take a bit. */
  StringBuffer<128> buf;
  unsigned char flag = 0;
  unsigned char mask = 0x80;

  for (auto field : this->m_fields) {
    if (is_numeric_field(field)) {
      Field_num *field_num = dynamic_cast<Field_num *>(field);
      if (field_num->unsigned_flag) flag |= mask;

      mask >>= 1;

      // 8 fields are tested, store the result and clear the flag.
      if (mask == 0) {
        buf.append(flag);
        flag = 0;
        mask = 0x80;
      }
    }
  }

  // Stores the signedness flags of last few columns
  if (mask != 0x80) buf.append(flag);

  // The table has no numeric column, so don't log SIGNEDNESS field
  if (buf.is_empty()) return false;

  return write_tlv_field(m_metadata_buf, SIGNEDNESS, buf);
}

bool Table_map_log_event::init_charset_field(
    std::function<bool(const Field *)> include_type,
    Optional_metadata_field_type default_charset_type,
    Optional_metadata_field_type column_charset_type) {
  DBUG_EXECUTE_IF("simulate_init_charset_field_error", return true;);

  std::map<uint, uint> collation_map;
  // For counting characters columns
  uint char_col_cnt = 0;

  /* Find the collation number used by most fields */
  for (auto field : this->m_fields) {
    if (include_type(field)) {
      Field_str *field_str = dynamic_cast<Field_str *>(field);

      collation_map[field_str->charset()->number]++;
      char_col_cnt++;
    }
  }

  if (char_col_cnt == 0) return false;

  /* Find the most used collation */
  uint most_used_collation = 0;
  uint most_used_count = 0;
  for (std::map<uint, uint>::iterator it = collation_map.begin();
       it != collation_map.end(); it++) {
    if (it->second > most_used_count) {
      most_used_count = it->second;
      most_used_collation = it->first;
    }
  }

  /*
    Comparing length of COLUMN_CHARSET field and COLUMN_CHARSET_WITH_DEFAULT
    field to decide which field should be logged.

    Length of COLUMN_CHARSET = character column count * collation id size.
    Length of COLUMN_CHARSET_WITH_DEFAULT =
     default collation_id size + count of columns not use default charset *
     (column index size + collation id size)

    Assume column index just uses 1 byte and collation number also uses 1 byte.
  */
  if (char_col_cnt * 1 < (1 + (char_col_cnt - most_used_count) * 2)) {
    StringBuffer<512> buf;

    /*
      Stores character set information into COLUMN_CHARSET format,
      character sets of all columns are stored one by one.
      -----------------------------------------
      | Charset number | .... |Charset number |
      -----------------------------------------
    */
    for (auto field : this->m_fields) {
      if (include_type(field)) {
        Field_str *field_str = dynamic_cast<Field_str *>(field);

        store_compressed_length(buf, field_str->charset()->number);
      }
    }
    return write_tlv_field(m_metadata_buf, column_charset_type, buf);
  } else {
    StringBuffer<512> buf;
    uint char_column_index = 0;
    uint default_collation = most_used_collation;

    /*
      Stores character set information into DEFAULT_CHARSET format,
      First stores the default character set, and then stores the character
      sets different to default character with their column index one by one.
      --------------------------------------------------------
      | Default Charset | Col Index | Charset number | ...   |
      --------------------------------------------------------
    */

    // Store the default collation number
    store_compressed_length(buf, default_collation);

    for (auto field : this->m_fields) {
      if (include_type(field)) {
        Field_str *field_str = dynamic_cast<Field_str *>(field);

        if (field_str->charset()->number != default_collation) {
          store_compressed_length(buf, char_column_index);
          store_compressed_length(buf, field_str->charset()->number);
        }
        char_column_index++;
      }
    }
    return write_tlv_field(m_metadata_buf, default_charset_type, buf);
  }
}

bool Table_map_log_event::init_column_name_field() {
  StringBuffer<2048> buf;

  for (auto field : this->m_fields) {
    size_t len = strlen(field->field_name);

    store_compressed_length(buf, len);
    buf.append(field->field_name, len);
  }
  return write_tlv_field(m_metadata_buf, COLUMN_NAME, buf);
}

bool Table_map_log_event::init_set_str_value_field() {
  StringBuffer<1024> buf;

  /*
    SET string values are stored in the same format:
    ----------------------------------------------
    | Value number | value1 len | value 1|  .... |  // first SET column
    ----------------------------------------------
    | Value number | value1 len | value 1|  .... |  // second SET column
    ----------------------------------------------
   */
  for (auto field : this->m_fields) {
    if (is_set_field(field)) {
      TYPELIB *typelib = dynamic_cast<Field_set *>(field)->typelib;

      store_compressed_length(buf, typelib->count);
      for (unsigned int i = 0; i < typelib->count; i++) {
        store_compressed_length(buf, typelib->type_lengths[i]);
        buf.append(typelib->type_names[i], typelib->type_lengths[i]);
      }
    }
  }
  if (buf.length() > 0)
    return write_tlv_field(m_metadata_buf, SET_STR_VALUE, buf);
  return false;
}

bool Table_map_log_event::init_enum_str_value_field() {
  StringBuffer<1024> buf;

  /* ENUM is same to SET columns, see comment in init_set_str_value_field */
  for (auto field : this->m_fields) {
    if (is_enum_field(field)) {
      TYPELIB *typelib = dynamic_cast<Field_enum *>(field)->typelib;

      store_compressed_length(buf, typelib->count);
      for (unsigned int i = 0; i < typelib->count; i++) {
        store_compressed_length(buf, typelib->type_lengths[i]);
        buf.append(typelib->type_names[i], typelib->type_lengths[i]);
      }
    }
  }

  if (buf.length() > 0)
    return write_tlv_field(m_metadata_buf, ENUM_STR_VALUE, buf);
  return false;
}

bool Table_map_log_event::init_geometry_type_field() {
  StringBuffer<256> buf;

  /* Geometry type of geometry columns is stored one by one as packed length */
  for (auto field : this->m_fields) {
    if (is_geometry_field(field)) {
      int type = dynamic_cast<Field_geom *>(field)->geom_type;
      DBUG_EXECUTE_IF("inject_invalid_geometry_type", type = 100;);
      store_compressed_length(buf, type);
    }
  }

  if (buf.length() > 0)
    return write_tlv_field(m_metadata_buf, GEOMETRY_TYPE, buf);
  return false;
}

bool Table_map_log_event::init_primary_key_field() {
  DBUG_EXECUTE_IF("simulate_init_primary_key_field_error", return true;);

  if (unlikely(m_table->s->is_missing_primary_key())) return false;

  // If any key column uses prefix like KEY(c1(10)) */
  bool has_prefix = false;
  KEY *pk = m_table->key_info + m_table->s->primary_key;

  DBUG_ASSERT(pk->user_defined_key_parts > 0);

  /* Check if any key column uses prefix */
  for (uint i = 0; i < pk->user_defined_key_parts; i++) {
    KEY_PART_INFO *key_part = pk->key_part + i;
    if (key_part->length !=
        m_table->field[key_part->fieldnr - 1]->key_length()) {
      has_prefix = true;
      break;
    }
  }

  StringBuffer<128> buf;

  if (!has_prefix) {
    /* Index of PK columns are stored one by one. */
    for (uint i = 0; i < pk->user_defined_key_parts; i++) {
      KEY_PART_INFO *key_part = pk->key_part + i;
      store_compressed_length(buf, key_part->fieldnr - 1);
    }
    return write_tlv_field(m_metadata_buf, SIMPLE_PRIMARY_KEY, buf);
  } else {
    /* Index of PK columns are stored with a prefix length one by one. */
    for (uint i = 0; i < pk->user_defined_key_parts; i++) {
      KEY_PART_INFO *key_part = pk->key_part + i;
      size_t prefix = 0;

      store_compressed_length(buf, key_part->fieldnr - 1);

      // Store character length but not octet length
      if (key_part->length !=
          m_table->field[key_part->fieldnr - 1]->key_length())
        prefix = key_part->length / key_part->field->charset()->mbmaxlen;
      store_compressed_length(buf, prefix);
    }
    return write_tlv_field(m_metadata_buf, PRIMARY_KEY_WITH_PREFIX, buf);
  }
}

/*
  Print some useful information for the SHOW BINARY LOG information
  field.
 */

int Table_map_log_event::pack_info(Protocol *protocol) {
  char buf[256];
  size_t bytes = snprintf(buf, sizeof(buf), "table_id: %llu (%s.%s)",
                          m_table_id.id(), m_dbnam.c_str(), m_tblnam.c_str());
  DBUG_ASSERT(bytes < 256);
  protocol->store_string(buf, bytes, &my_charset_bin);
  return 0;
}
#endif  // MYSQL_SERVER

#ifndef MYSQL_SERVER
void Table_map_log_event::print(FILE *,
                                PRINT_EVENT_INFO *print_event_info) const {
  if (!print_event_info->short_form) {
    print_header(&print_event_info->head_cache, print_event_info, true);
    my_b_printf(&print_event_info->head_cache,
                "\tTable_map: `%s`.`%s` mapped to number %llu\n",
                m_dbnam.c_str(), m_tblnam.c_str(), m_table_id.id());

    if (print_event_info->print_table_metadata) {
      if (m_fb_format) {
        print_fb_format_primary_key(&print_event_info->head_cache);
      } else {
        Optional_metadata_fields fields(m_optional_metadata,
                                        m_optional_metadata_len);

        if (m_optional_metadata) DBUG_ASSERT(fields.is_valid);
        print_columns(&print_event_info->head_cache, fields);
        print_primary_key(&print_event_info->head_cache, fields);
      }
    }

    print_base64(&print_event_info->body_cache, print_event_info, true);
  }
}

/**
   return the string name of a type.

   @param[in] type  type of a column
   @param[in,out] meta_ptr  the meta_ptr of the column. If the type doesn't have
                            metadata, it will not change  meta_ptr, otherwise
                            meta_ptr will be moved to the end of the column's
                            metadat.
   @param[in] cs charset of the column if it is a character column.
   @param[out] typestr  buffer to storing the string name of the type
   @param[in] typestr_length  length of typestr
   @param[in] geometry_type  internal geometry_type
 */
static void get_type_name(uint type, unsigned char **meta_ptr,
                          const CHARSET_INFO *cs, char *typestr,
                          uint typestr_length, unsigned int geometry_type) {
  switch (type) {
    case MYSQL_TYPE_LONG:
      snprintf(typestr, typestr_length, "%s", "INT");
      break;
    case MYSQL_TYPE_TINY:
      snprintf(typestr, typestr_length, "TINYINT");
      break;
    case MYSQL_TYPE_SHORT:
      snprintf(typestr, typestr_length, "SMALLINT");
      break;
    case MYSQL_TYPE_INT24:
      snprintf(typestr, typestr_length, "MEDIUMINT");
      break;
    case MYSQL_TYPE_LONGLONG:
      snprintf(typestr, typestr_length, "BIGINT");
      break;
    case MYSQL_TYPE_NEWDECIMAL:
      snprintf(typestr, typestr_length, "DECIMAL(%d,%d)", (*meta_ptr)[0],
               (*meta_ptr)[1]);
      (*meta_ptr) += 2;
      break;
    case MYSQL_TYPE_FLOAT:
      snprintf(typestr, typestr_length, "FLOAT");
      (*meta_ptr)++;
      break;
    case MYSQL_TYPE_DOUBLE:
      snprintf(typestr, typestr_length, "DOUBLE");
      (*meta_ptr)++;
      break;
    case MYSQL_TYPE_BIT:
      snprintf(typestr, typestr_length, "BIT(%d)",
               (((*meta_ptr)[0])) + (*meta_ptr)[1] * 8);
      (*meta_ptr) += 2;
      break;
    case MYSQL_TYPE_TIMESTAMP2:
      if (**meta_ptr != 0)
        snprintf(typestr, typestr_length, "TIMESTAMP(%d)", **meta_ptr);
      else
        snprintf(typestr, typestr_length, "TIMESTAMP");
      (*meta_ptr)++;
      break;
    case MYSQL_TYPE_DATETIME2:
      if (**meta_ptr != 0)
        snprintf(typestr, typestr_length, "DATETIME(%d)", **meta_ptr);
      else
        snprintf(typestr, typestr_length, "DATETIME");
      (*meta_ptr)++;
      break;
    case MYSQL_TYPE_TIME2:
      if (**meta_ptr != 0)
        snprintf(typestr, typestr_length, "TIME(%d)", **meta_ptr);
      else
        snprintf(typestr, typestr_length, "TIME");
      (*meta_ptr)++;
      break;
    case MYSQL_TYPE_NEWDATE:
    case MYSQL_TYPE_DATE:
      snprintf(typestr, typestr_length, "DATE");
      break;
    case MYSQL_TYPE_YEAR:
      snprintf(typestr, typestr_length, "YEAR");
      break;
    case MYSQL_TYPE_ENUM:
      snprintf(typestr, typestr_length, "ENUM");
      (*meta_ptr) += 2;
      break;
    case MYSQL_TYPE_SET:
      snprintf(typestr, typestr_length, "SET");
      (*meta_ptr) += 2;
      break;
    case MYSQL_TYPE_BLOB: {
      bool is_text = (cs && cs->number != my_charset_bin.number);
      const char *names[5][2] = {{"INVALID_BLOB(%d)", "INVALID_TEXT(%d)"},
                                 {"TINYBLOB", "TINYTEXT"},
                                 {"BLOB", "TEXT"},
                                 {"MEDIUMBLOB", "MEDIUMTEXT"},
                                 {"LONGBLOB", "LONGTEXT"}};
      unsigned char size = **meta_ptr;

      if (size == 0 || size > 4)
        snprintf(typestr, typestr_length, names[0][is_text], size);
      else
        snprintf(typestr, typestr_length, "%s", names[**meta_ptr][is_text]);

      (*meta_ptr)++;
    } break;
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_VAR_STRING:
      if (cs && cs->number != my_charset_bin.number)
        snprintf(typestr, typestr_length, "VARCHAR(%d)",
                 uint2korr(*meta_ptr) / cs->mbmaxlen);
      else
        snprintf(typestr, typestr_length, "VARBINARY(%d)",
                 uint2korr(*meta_ptr));

      (*meta_ptr) += 2;
      break;
    case MYSQL_TYPE_STRING: {
      uint byte0 = (*meta_ptr)[0];
      uint byte1 = (*meta_ptr)[1];
      uint len = (((byte0 & 0x30) ^ 0x30) << 4) | byte1;

      if (cs && cs->number != my_charset_bin.number)
        snprintf(typestr, typestr_length, "CHAR(%d)", len / cs->mbmaxlen);
      else
        snprintf(typestr, typestr_length, "BINARY(%d)", len);

      (*meta_ptr) += 2;
    } break;
    case MYSQL_TYPE_JSON:
      snprintf(typestr, typestr_length, "JSON");
      (*meta_ptr)++;
      break;
    case MYSQL_TYPE_GEOMETRY: {
      const char *names[8] = {
          "GEOMETRY",   "POINT",           "LINESTRING",   "POLYGON",
          "MULTIPOINT", "MULTILINESTRING", "MULTIPOLYGON", "GEOMCOLLECTION"};
      if (geometry_type < 8)
        snprintf(typestr, typestr_length, "%s", names[geometry_type]);
      else
        snprintf(typestr, typestr_length, "INVALID_GEOMETRY_TYPE(%u)",
                 geometry_type);
      (*meta_ptr)++;
    } break;
    default:
      *typestr = 0;
      break;
  }
}

/**
  Interface for iterator over charset columns.
*/
class Table_map_log_event::Charset_iterator {
 public:
  typedef Table_map_event::Optional_metadata_fields::Default_charset
      Default_charset;
  virtual const CHARSET_INFO *next() = 0;
  virtual ~Charset_iterator() = default;

  /**
    Factory method to create an instance of the appropriate subclass.
  */
  static std::unique_ptr<Charset_iterator> create_charset_iterator(
      const Default_charset &default_charset,
      const std::vector<uint> &column_charset);
};

/**
  Implementation of charset iterator for the DEFAULT_CHARSET type.
*/
class Table_map_log_event::Default_charset_iterator : public Charset_iterator {
 public:
  Default_charset_iterator(const Default_charset &default_charset)
      : m_iterator(default_charset.charset_pairs.begin()),
        m_end(default_charset.charset_pairs.end()),
        m_column_index(0),
        m_default_charset_info(
            get_charset(default_charset.default_charset, 0)) {}

  const CHARSET_INFO *next() override {
    const CHARSET_INFO *ret;
    if (m_iterator != m_end && m_iterator->first == m_column_index) {
      ret = get_charset(m_iterator->second, 0);
      m_iterator++;
    } else
      ret = m_default_charset_info;
    m_column_index++;
    return ret;
  }

 private:
  std::vector<Optional_metadata_fields::uint_pair>::const_iterator m_iterator,
      m_end;
  uint m_column_index;
  const CHARSET_INFO *m_default_charset_info;
};

/**
  Implementation of charset iterator for the COLUMNT_CHARSET type.
*/
class Table_map_log_event::Column_charset_iterator : public Charset_iterator {
 public:
  Column_charset_iterator(const std::vector<uint> &column_charset)
      : m_iterator(column_charset.begin()), m_end(column_charset.end()) {}

  const CHARSET_INFO *next() override {
    const CHARSET_INFO *ret = nullptr;
    if (m_iterator != m_end) {
      ret = get_charset(*m_iterator, 0);
      m_iterator++;
    }
    return ret;
  }

 private:
  std::vector<uint>::const_iterator m_iterator;
  std::vector<uint>::const_iterator m_end;
};

std::unique_ptr<Table_map_log_event::Charset_iterator>
Table_map_log_event::Charset_iterator::create_charset_iterator(
    const Default_charset &default_charset,
    const std::vector<uint> &column_charset) {
  if (!default_charset.empty())
    return std::unique_ptr<Charset_iterator>(
        new Default_charset_iterator(default_charset));
  else
    return std::unique_ptr<Charset_iterator>(
        new Column_charset_iterator(column_charset));
}

void Table_map_log_event::print_columns(
    IO_CACHE *file, const Optional_metadata_fields &fields) const {
  unsigned char *field_metadata_ptr = m_field_metadata;
  std::vector<bool>::const_iterator signedness_it = fields.m_signedness.begin();

  std::unique_ptr<Charset_iterator> charset_it =
      Charset_iterator::create_charset_iterator(fields.m_default_charset,
                                                fields.m_column_charset);
  std::unique_ptr<Charset_iterator> enum_and_set_charset_it =
      Charset_iterator::create_charset_iterator(
          fields.m_enum_and_set_default_charset,
          fields.m_enum_and_set_column_charset);
  std::vector<std::string>::const_iterator col_names_it =
      fields.m_column_name.begin();
  std::vector<Optional_metadata_fields::str_vector>::const_iterator
      set_str_values_it = fields.m_set_str_value.begin();
  std::vector<Optional_metadata_fields::str_vector>::const_iterator
      enum_str_values_it = fields.m_enum_str_value.begin();
  std::vector<unsigned int>::const_iterator geometry_type_it =
      fields.m_geometry_type.begin();
  uint geometry_type = 0;

  my_b_printf(file, "# Columns(");

  for (unsigned long i = 0; i < m_colcnt; i++) {
    uint real_type = m_coltype[i];
    if (real_type == MYSQL_TYPE_STRING &&
        (*field_metadata_ptr == MYSQL_TYPE_ENUM ||
         *field_metadata_ptr == MYSQL_TYPE_SET))
      real_type = *field_metadata_ptr;

    // Get current column's collation id if it is a character, enum,
    // or set column
    const CHARSET_INFO *cs = nullptr;
    if (is_character_type(real_type))
      cs = charset_it->next();
    else if (is_enum_or_set_type(real_type))
      cs = enum_and_set_charset_it->next();

    // Print column name
    if (col_names_it != fields.m_column_name.end()) {
      pretty_print_identifier(file, col_names_it->c_str(),
                              col_names_it->size());
      my_b_printf(file, " ");
      col_names_it++;
    }

    // update geometry_type for geometry columns
    if (real_type == MYSQL_TYPE_GEOMETRY) {
      geometry_type = (geometry_type_it != fields.m_geometry_type.end())
                          ? *geometry_type_it++
                          : 0;
    }

    // print column type
    const uint TYPE_NAME_LEN = 100;
    char type_name[TYPE_NAME_LEN];
    get_type_name(real_type, &field_metadata_ptr, cs, type_name, TYPE_NAME_LEN,
                  geometry_type);

    if (type_name[0] == '\0') {
      my_b_printf(file, "INVALID_TYPE(%d)", real_type);
      continue;
    }
    my_b_printf(file, "%s", type_name);

    // Print UNSIGNED for numeric column
    if (is_numeric_type(real_type) &&
        signedness_it != fields.m_signedness.end()) {
      if (*signedness_it == true) my_b_printf(file, " UNSIGNED");
      signedness_it++;
    }

    // if the column is not marked as 'null', print 'not null'
    if (!(m_null_bits[(i / 8)] & (1 << (i % 8))))
      my_b_printf(file, " NOT NULL");

    // Print string values of SET and ENUM column
    const Optional_metadata_fields::str_vector *str_values = nullptr;
    if (real_type == MYSQL_TYPE_ENUM &&
        enum_str_values_it != fields.m_enum_str_value.end()) {
      str_values = &(*enum_str_values_it);
      enum_str_values_it++;
    } else if (real_type == MYSQL_TYPE_SET &&
               set_str_values_it != fields.m_set_str_value.end()) {
      str_values = &(*set_str_values_it);
      set_str_values_it++;
    }

    if (str_values != nullptr) {
      const char *separator = "(";
      for (Optional_metadata_fields::str_vector::const_iterator it =
               str_values->begin();
           it != str_values->end(); it++) {
        my_b_printf(file, "%s", separator);
        pretty_print_str(file, it->c_str(), it->size());
        separator = ", ";
      }
      my_b_printf(file, ")");
    }

    // Print column character set, except in text columns with binary collation
    if (cs != nullptr &&
        (is_enum_or_set_type(real_type) || cs->number != my_charset_bin.number))
      my_b_printf(file, " CHARSET %s COLLATE %s", cs->csname, cs->name);

    if (i != m_colcnt - 1) my_b_printf(file, ",\n#         ");
  }
  my_b_printf(file, ")");
  my_b_printf(file, "\n");
}

void Table_map_log_event::print_primary_key(
    IO_CACHE *file, const Optional_metadata_fields &fields) const {
  if (!fields.m_primary_key.empty()) {
    my_b_printf(file, "# Primary Key(");

    std::vector<Optional_metadata_fields::uint_pair>::const_iterator it =
        fields.m_primary_key.begin();

    for (; it != fields.m_primary_key.end(); it++) {
      if (it != fields.m_primary_key.begin()) my_b_printf(file, ", ");

      // Print column name or column index
      if (it->first >= fields.m_column_name.size())
        my_b_printf(file, "%u", it->first);
      else
        my_b_printf(file, "%s", fields.m_column_name[it->first].c_str());

      // Print prefix length
      if (it->second != 0) my_b_printf(file, "(%u)", it->second);
    }

    my_b_printf(file, ")\n");
  }
}

void Table_map_log_event::print_fb_format_primary_key(IO_CACHE *file) const {
  DBUG_ASSERT(m_fb_format);
  my_b_printf(file, "#Primary Key Fields: ` ");
  if (m_primary_key_fields) {
    uchar *ptr = m_primary_key_fields;
    uint n = net_field_length(&ptr);
    for (ulong i = 0; i < n; ++i) {
      uint field_index = net_field_length(&ptr);
      my_b_printf(file, "%u ", field_index);
    }
  }
  my_b_printf(file, "`\n");
}
#endif

/**************************************************************************
        Write_rows_log_event member functions
**************************************************************************/

/*
  Constructor used to build an event for writing to the binary log.
 */
#if defined(MYSQL_SERVER)
Write_rows_log_event::Write_rows_log_event(
    THD *thd_arg, TABLE *tbl_arg, const Table_id &tid_arg,
    bool is_transactional, const unsigned char *extra_row_ndb_info)
    : binary_log::Rows_event(log_bin_use_v1_row_events
                                 ? binary_log::WRITE_ROWS_EVENT_V1
                                 : binary_log::WRITE_ROWS_EVENT),
      Rows_log_event(thd_arg, tbl_arg, tid_arg, tbl_arg->write_set,
                     is_transactional,
                     log_bin_use_v1_row_events ? binary_log::WRITE_ROWS_EVENT_V1
                                               : binary_log::WRITE_ROWS_EVENT,
                     extra_row_ndb_info) {
  /*
    If this is a blind 'replace into' optimization, then set the bit
    in the flags so that it can be carried over to the slave
  */
  if (thd_arg && thd_arg->lex->blind_replace_into) {
    set_flags(BLIND_REPLACE_INTO_F);
  }

  common_header->type_code = m_type;
}

bool Write_rows_log_event::binlog_row_logging_function(
    THD *thd_arg, TABLE *table, bool is_transactional,
    const uchar *before_record MY_ATTRIBUTE((unused)),
    const uchar *after_record) {
  return thd_arg->binlog_write_row(table, is_transactional, after_record,
                                   nullptr);
}
#endif

/*
  Constructor used by slave to read the event from the binary log.
 */
Write_rows_log_event::Write_rows_log_event(
    const char *buf, const Format_description_event *description_event)
    : binary_log::Rows_event(buf, description_event),
      Rows_log_event(buf, description_event),
      binary_log::Write_rows_event(buf, description_event) {
  DBUG_ASSERT(header()->type_code == m_type);
}

#if defined(MYSQL_SERVER)
int Write_rows_log_event::do_before_row_operations(
    const Slave_reporting_capability *const, table_def *tabledef) {
  int error = 0;

  m_table->file->rpl_before_write_rows();

  /*
    Increment the global status insert count variable
  */
  if (get_flags(STMT_END_F)) thd->status_var.com_stat[SQLCOM_INSERT]++;

  /*
    Let storage engines treat this event as an INSERT command.

    Set 'sql_command' as SQLCOM_INSERT after the tables are locked.
    When locking the tables, it should be SQLCOM_END.
    THD::decide_logging_format which is called from "lock tables"
    assumes that row_events will have 'sql_command' as SQLCOM_END.
  */
  thd->lex->sql_command = SQLCOM_INSERT;

  /**
     todo: to introduce a property for the event (handler?) which forces
     applying the event in the replace (idempotent) fashion.
  */
  if ((rbr_exec_mode == RBR_EXEC_MODE_IDEMPOTENT) ||
      (m_table->s->db_type()->db_type == DB_TYPE_NDBCLUSTER)) {
    /*
      We are using REPLACE semantics and not INSERT IGNORE semantics
      when writing rows, that is: new rows replace old rows.  We need to
      inform the storage engine that it should use this behaviour.
    */

    /* Tell the storage engine that we are using REPLACE semantics. */
    thd->lex->duplicates = DUP_REPLACE;

    /*
      Pretend we're executing a REPLACE command: this is needed for
      InnoDB and NDB Cluster since they are not (properly) checking the
      lex->duplicates flag.
    */
    thd->lex->sql_command = SQLCOM_REPLACE;
    /*
       Do not raise the error flag in case of hitting to an unique attribute
    */
    m_table->file->ha_extra(HA_EXTRA_IGNORE_DUP_KEY);
    /*
       NDB specific: update from ndb master wrapped as Write_rows
       so that the event should be applied to replace slave's row

       Also following is needed in case if we have AFTER DELETE triggers
    */
    m_table->file->ha_extra(HA_EXTRA_WRITE_CAN_REPLACE);
    /*
       NDB specific: if update from ndb master wrapped as Write_rows
       does not find the row it's assumed idempotent binlog applying
       is taking place; don't raise the error.
    */
    m_table->file->ha_extra(HA_EXTRA_IGNORE_NO_KEY);
    /*
      TODO: the cluster team (Tomas?) says that it's better if the engine knows
      how many rows are going to be inserted, then it can allocate needed memory
      from the start.
    */
  }

  if (slave_run_triggers_for_rbr && !master_had_triggers && m_table->triggers)
    m_table->prepare_triggers_for_insert_stmt_or_event();

  /* Honor next number column if present */
  m_table->next_number_field = m_table->found_next_number_field;
  /*
   * Fixed Bug#45999, In RBR, Store engine of Slave auto-generates new
   * sequence numbers for auto_increment fields if the values of them are 0.
   * If generateing a sequence number is decided by the values of
   * table->autoinc_field_has_explicit_non_null_value and SQL_MODE(if
   * includes MODE_NO_AUTO_VALUE_ON_ZERO) in update_auto_increment function.
   * SQL_MODE of slave sql thread is always consistency with master's.
   * In RBR, auto_increment fields never are NULL, except if the auto_inc
   * column exists only on the slave side (i.e., in an extra column
   * on the slave's table).
   */
  if (!is_auto_inc_in_extra_columns())
    m_table->autoinc_field_has_explicit_non_null_value = true;
  else {
    /*
      Here we have checked that there is an extra field
      on this server's table that has an auto_inc column.

      Mark that the auto_increment field is null and mark
      the read and write set bits.

      (There can only be one AUTO_INC column, it is always
       indexed and it cannot have a DEFAULT value).
    */
    m_table->autoinc_field_has_explicit_non_null_value = false;
    m_table->mark_auto_increment_column();
  }

  /**
     Sets it to ROW_LOOKUP_NOT_NEEDED.
   */
  decide_row_lookup_algorithm_and_key(tabledef);
  DBUG_ASSERT(m_rows_lookup_algorithm == ROW_LOOKUP_NOT_NEEDED);

  return error;
}

int Write_rows_log_event::do_after_row_operations(
    const Slave_reporting_capability *const, int error) {
  int local_error = 0;

  /**
    Clear the write_set bit for auto_inc field that only
    existed on the destination table as an extra column.
   */
  if (is_auto_inc_in_extra_columns()) {
    bitmap_clear_bit(m_table->write_set,
                     m_table->next_number_field->field_index);
    bitmap_clear_bit(m_table->read_set,
                     m_table->next_number_field->field_index);

    if (get_flags(STMT_END_F)) m_table->file->ha_release_auto_increment();
  }
  m_table->next_number_field = nullptr;
  m_table->autoinc_field_has_explicit_non_null_value = false;
  if ((local_error = m_table->file->ha_end_bulk_insert())) {
    m_table->file->print_error(local_error, MYF(0));
  }

  m_rows_lookup_algorithm = ROW_LOOKUP_UNDEFINED;
  m_table->file->rpl_after_write_rows();

  return error ? error : local_error;
}

/*
  Check if there are more UNIQUE keys after the given key.
*/
static int last_uniq_key(TABLE *table, uint keyno) {
  while (++keyno < table->s->keys)
    if (table->key_info[keyno].flags & HA_NOSAME) return 0;
  return 1;
}

/**
  Write the current row into event's table.

  The row is located in the row buffer, pointed by @c m_curr_row member.
  Number of columns of the row is stored in @c m_width member (it can be
  different from the number of columns in the table to which we insert).
  Bitmap @c m_cols indicates which columns are present in the row. It is assumed
  that event's table is already open and pointed by @c m_table.

  If the same record already exists in the table it can be either overwritten
  or an error is reported depending on the value of @c overwrite flag
  (error reporting not yet implemented). Note that the matching record can be
  different from the row we insert if we use primary keys to identify records in
  the table.

  The row to be inserted can contain values only for selected columns. The
  missing columns are filled with default values using @c prepare_record()
  function. If a matching record is found in the table and @c overwritte is
  true, the missing columns are taken from it.

  @param  rli   Relay log info (needed for row unpacking).
  @param  overwrite
                Shall we overwrite if the row already exists or signal
                error (currently ignored).

  @returns Error code on failure, 0 on success.

  This method, if successful, sets @c m_curr_row_end pointer to point at the
  next row in the rows buffer. This is done when unpacking the row to be
  inserted.

  @note If a matching record is found, it is either updated using
  @c ha_update_row() or first deleted and then new record written.
*/

int Write_rows_log_event::write_row(const Relay_log_info *const rli,
                                    const bool overwrite) {
  DBUG_TRACE;
  DBUG_ASSERT(m_table != nullptr && thd != nullptr);

  TABLE *table = m_table;  // pointer to event's table
  int error;
  int keynum = 0;
  char *key = nullptr;

  /*
    If the master executed this as an blind 'replace into', do the same
    on slave if possible. This can be done if:
    (0) Blind replace is enabled (by setting enable_blind_replace sysvar)
    (1) master executed this as an optimized 'replace into' statement
        (as identified by BLIND_REPLACE_INTO_F)
    (2) The table has a well defined primary key (and no hidden pk)
    (3) The table has no secondary keys
    (4) The table has no triggers defined
    If slave is not able to execute this as a blind 'replcae into', then
    convert it into a regular UPDATE or DELETE+INSERT
  */
  if (get_flags(BLIND_REPLACE_INTO_F)) { /* 1 */
    /*
      This was an optimized 'replace into' in master, but slave cannot
      execute it as such due to the constraints explained above.
      So, convert this into a regular UPDATE or DELETE+INSERT. This will make
      binlogs to look different in master and slave, but the logical outcome
      will be the same
    */
    thd->lex->duplicates = DUP_REPLACE;
    thd->lex->blind_replace_into =
        enable_blind_replace &&                 /* 0 */
        table->s->keys == 1 &&                  /* 2, 3 */
        table->s->primary_key != MAX_INDEXES && /* 2 */
        !table->triggers;                       /* 4 */
  }

  const bool invoke_triggers =
      slave_run_triggers_for_rbr && !master_had_triggers && table->triggers;

  prepare_record(table, &this->m_local_cols,
                 table->file->ht->db_type != DB_TYPE_NDBCLUSTER);

  /* unpack row into table->record[0] */
  if ((error = unpack_current_row(rli, &m_cols, true /*is AI*/))) return error;

  /*
    When m_curr_row == m_curr_row_end, it means a row that contains nothing,
    so all the pointers shall be pointing to the same address, or else
    we have corrupt data and shall throw the error.
  */
  DBUG_PRINT("debug", ("m_rows_buf= %p, m_rows_cur= %p, m_rows_end= %p",
                       m_rows_buf, m_rows_cur, m_rows_end));
  DBUG_PRINT("debug", ("m_curr_row= %p, m_curr_row_end= %p", m_curr_row,
                       m_curr_row_end));
  if (m_curr_row == m_curr_row_end &&
      !((m_rows_buf == m_rows_cur) && (m_rows_cur == m_rows_end))) {
    my_error(ER_SLAVE_CORRUPT_EVENT, MYF(0));
    return ER_SLAVE_CORRUPT_EVENT;
  }

  // Invoke check constraints on the unpacked row.
  if (invoke_table_check_constraints(thd, table))
    return ER_CHECK_CONSTRAINT_VIOLATED;

  if (m_curr_row == m_rows_buf && !invoke_triggers) {
    // This table has no triggers so we can do bulk insert
    /* this is the first row to be inserted, we estimate the rows with
       the size of the first row and use that value to initialize
       storage engine for bulk insertion */
    DBUG_ASSERT(!(m_curr_row > m_curr_row_end));
    ulong estimated_rows = 0;
    if (m_curr_row < m_curr_row_end)
      estimated_rows =
          (m_rows_end - m_curr_row) / (m_curr_row_end - m_curr_row);
    else if (m_curr_row == m_curr_row_end)
      estimated_rows = 1;

    m_table->file->ha_start_bulk_insert(estimated_rows);
  }

  /*
    Explicitly set the auto_inc to null to make sure that
    it gets an auto_generated value.
  */
  if (is_auto_inc_in_extra_columns()) m_table->next_number_field->set_null();

#ifndef DBUG_OFF
  DBUG_DUMP("record[0]", table->record[0], table->s->reclength);
  DBUG_PRINT_BITSET("debug", "write_set = %s", table->write_set);
  DBUG_PRINT_BITSET("debug", "read_set = %s", table->read_set);
#endif

  if (invoke_triggers &&
      process_triggers(TRG_EVENT_INSERT, TRG_ACTION_BEFORE, true))
    return HA_ERR_GENERIC;  // in case if error is not set yet

  /*
    Try to write record. If a corresponding record already exists in the table,
    we try to change it using ha_update_row() if possible. Otherwise we delete
    it and repeat the whole process again.

    TODO: Add safety measures against infinite looping.
   */

  m_table->mark_columns_per_binlog_row_image(thd, true);

  while ((error = table->file->ha_write_row(table->record[0]))) {
    if (error == HA_ERR_LOCK_DEADLOCK || error == HA_ERR_LOCK_WAIT_TIMEOUT ||
        (keynum = table->file->get_dup_key(error)) < 0 || !overwrite) {
      DBUG_PRINT("info", ("get_dup_key returns %d)", keynum));
      /*
        Deadlock, waiting for lock or just an error from the handler
        such as HA_ERR_FOUND_DUPP_KEY when overwrite is false and
        this is not a blind 'replace into' optimization.
        Retrieval of the duplicate key number may fail
        - either because the error was not "duplicate key" error
        - or because the information which key is not available
      */
      table->file->print_error(error, MYF(0));
      goto error;
    }
    /*
      key index value is either valid in the range [0-MAX_KEY) or
      has value MAX_KEY as a marker for the case when no information
      about key can be found. In the last case we have to require
      that storage engine has the flag HA_DUPLICATE_POS turned on.
      If this invariant is false then DBUG_ASSERT will crash
      the server built in debug mode. For the server that was built
      without DEBUG we have additional check for the value of key index
      in the code below in order to report about error in any case.
    */
    DBUG_ASSERT(keynum != MAX_KEY ||
                (keynum == MAX_KEY &&
                 (table->file->ha_table_flags() & HA_DUPLICATE_POS)));
    /*
       We need to retrieve the old row into record[1] to be able to
       either update or delete the offending record.  We either:

       - use ha_rnd_pos() with a row-id (available as dupp_row) to the
         offending row, if that is possible (MyISAM and Blackhole), or else

       - use ha_index_read_idx_map() with the key that is duplicated, to
         retrieve the offending row.
     */
    if (table->file->ha_table_flags() & HA_DUPLICATE_POS) {
      DBUG_PRINT("info", ("Locating offending record using ha_rnd_pos()"));

      if (table->file->inited && (error = table->file->ha_index_end())) {
        table->file->print_error(error, MYF(0));
        goto error;
      }
      if ((error = table->file->ha_rnd_init(false))) {
        table->file->print_error(error, MYF(0));
        goto error;
      }

      error = table->file->ha_rnd_pos(table->record[1], table->file->dup_ref);

      table->file->ha_rnd_end();
      if (error) {
        DBUG_PRINT("info", ("ha_rnd_pos() returns error %d", error));
        if (error == HA_ERR_RECORD_DELETED) error = HA_ERR_KEY_NOT_FOUND;
        table->file->print_error(error, MYF(0));
        goto error;
      }
    } else {
      DBUG_PRINT("info", ("Locating offending record using index_read_idx()"));

      if (key == nullptr) {
        key = static_cast<char *>(my_alloca(table->s->max_unique_length));
        if (key == nullptr) {
          DBUG_PRINT("info", ("Can't allocate key buffer"));
          error = ENOMEM;
          goto error;
        }
      }

      if ((uint)keynum < MAX_KEY) {
        key_copy((uchar *)key, table->record[0], table->key_info + keynum, 0);
        error = table->file->ha_index_read_idx_map(
            table->record[1], keynum, (const uchar *)key, HA_WHOLE_KEY,
            HA_READ_KEY_EXACT);
      } else
        /*
          For the server built in non-debug mode returns error if
          handler::get_dup_key() returned MAX_KEY as the value of key index.
        */
        error = HA_ERR_FOUND_DUPP_KEY;

      if (error) {
        DBUG_PRINT("info",
                   ("ha_index_read_idx_map() returns %s", HA_ERR(error)));
        if (error == HA_ERR_RECORD_DELETED) error = HA_ERR_KEY_NOT_FOUND;
        table->file->print_error(error, MYF(0));
        goto error;
      }
    }

    /*
       Now, record[1] should contain the offending row.  That
       will enable us to update it or, alternatively, delete it (so
       that we can insert the new row afterwards).
     */

    /*
      If row is incomplete we will use the record found to fill
      missing columns.
    */
    if (!get_flags(COMPLETE_ROWS_F)) {
      restore_record(table, record[1]);
      error = unpack_current_row(rli, &m_cols, true /*is AI*/);
    }

#ifndef DBUG_OFF
    DBUG_PRINT("debug", ("preparing for update: before and after image"));
    DBUG_DUMP("record[1] (before)", table->record[1], table->s->reclength);
    DBUG_DUMP("record[0] (after)", table->record[0], table->s->reclength);
#endif

    /*
       REPLACE is defined as either INSERT or DELETE + INSERT.  If
       possible, we can replace it with an UPDATE, but that will not
       work on InnoDB if FOREIGN KEY checks are necessary.

       I (Matz) am not sure of the reason for the last_uniq_key()
       check as, but I'm guessing that it's something along the
       following lines.

       Suppose that we got the duplicate key to be a key that is not
       the last unique key for the table and we perform an update:
       then there might be another key for which the unique check will
       fail, so we're better off just deleting the row and inserting
       the correct row.
     */
    if (last_uniq_key(table, keynum) &&
        !table->s->is_referenced_by_foreign_key()) {
      DBUG_PRINT("info", ("Updating row using ha_update_row()"));
      if (invoke_triggers &&
          process_triggers(TRG_EVENT_UPDATE, TRG_ACTION_BEFORE, false))
        error = HA_ERR_GENERIC;  // in case if error is not set yet
      else {
        error = table->file->ha_update_row(table->record[1], table->record[0]);
        switch (error) {
          case HA_ERR_RECORD_IS_THE_SAME:
            DBUG_PRINT("info", ("ignoring HA_ERR_RECORD_IS_THE_SAME error from"
                                " ha_update_row()"));
            error = 0;

          case 0:
            break;

          default:
            DBUG_PRINT("info", ("ha_update_row() returns error %d", error));
            table->file->print_error(error, MYF(0));
        }
        if (invoke_triggers && !error &&
            (process_triggers(TRG_EVENT_UPDATE, TRG_ACTION_AFTER, true) ||
             process_triggers(TRG_EVENT_INSERT, TRG_ACTION_AFTER, true)))
          error = HA_ERR_GENERIC;  // in case if error is not set yet
      }
      goto error;
    } else {
      DBUG_PRINT("info",
                 ("Deleting offending row and trying to write new one again"));
      if (invoke_triggers &&
          process_triggers(TRG_EVENT_DELETE, TRG_ACTION_BEFORE, true)) {
        error = HA_ERR_GENERIC;  // in case if error is not set yet
        goto error;
      } else {
        if ((error = table->file->ha_delete_row(table->record[1]))) {
          DBUG_PRINT("info", ("ha_delete_row() returns error %d", error));
          table->file->print_error(error, MYF(0));
          goto error;
        }
        if (invoke_triggers &&
            process_triggers(TRG_EVENT_DELETE, TRG_ACTION_AFTER, true)) {
          error = HA_ERR_GENERIC;  // in case if error is not set yet
          goto error;
        }
      }
      /* Will retry ha_write_row() with the offending row removed. */
    }
  }

  if (invoke_triggers &&
      process_triggers(TRG_EVENT_INSERT, TRG_ACTION_AFTER, true))
    error = HA_ERR_GENERIC;  // in case if error is not set yet

error:
  m_table->default_column_bitmaps();
  return error;
}

int Write_rows_log_event::do_exec_row(const Relay_log_info *const rli) {
  DBUG_ASSERT(m_table != nullptr);
  const bool overwrite = (rbr_exec_mode == RBR_EXEC_MODE_IDEMPOTENT ||
                          get_flags(BLIND_REPLACE_INTO_F));

  int error = write_row(rli, overwrite);

  if (error && !thd->is_error()) {
    DBUG_ASSERT(0);
    my_error(ER_UNKNOWN_ERROR, MYF(0));
  }

  return error;
}

uint8 Write_rows_log_event::get_trg_event_map() const noexcept {
  return (static_cast<uint8>(1 << static_cast<int>(TRG_EVENT_INSERT)) |
          static_cast<uint8>(1 << static_cast<int>(TRG_EVENT_UPDATE)) |
          static_cast<uint8>(1 << static_cast<int>(TRG_EVENT_DELETE)));
}

#endif /* defined(MYSQL_SERVER) */

#ifndef MYSQL_SERVER
void Write_rows_log_event::print(FILE *file,
                                 PRINT_EVENT_INFO *print_event_info) const {
  DBUG_EXECUTE_IF("simulate_cache_read_error",
                  { DBUG_SET("+d,simulate_my_b_fill_error"); });
  Rows_log_event::print_helper(file, print_event_info);
}
#endif

/**************************************************************************
        Delete_rows_log_event member functions
**************************************************************************/

/*
  Constructor used to build an event for writing to the binary log.
 */

#ifdef MYSQL_SERVER
Delete_rows_log_event::Delete_rows_log_event(THD *thd_arg, TABLE *tbl_arg,
                                             const Table_id &tid,
                                             bool is_transactional,
                                             const uchar *extra_row_ndb_info)
    : binary_log::Rows_event(log_bin_use_v1_row_events
                                 ? binary_log::DELETE_ROWS_EVENT_V1
                                 : binary_log::DELETE_ROWS_EVENT),
      Rows_log_event(thd_arg, tbl_arg, tid, tbl_arg->read_set, is_transactional,
                     log_bin_use_v1_row_events
                         ? binary_log::DELETE_ROWS_EVENT_V1
                         : binary_log::DELETE_ROWS_EVENT,
                     extra_row_ndb_info),
      binary_log::Delete_rows_event() {
  common_header->type_code = m_type;
}

bool Delete_rows_log_event::binlog_row_logging_function(
    THD *thd_arg, TABLE *table, bool is_transactional,
    const uchar *before_record,
    const uchar *after_record MY_ATTRIBUTE((unused))) {
  return thd_arg->binlog_delete_row(table, is_transactional, before_record,
                                    nullptr);
}

#endif /* #if defined(MYSQL_SERVER) */

/*
  Constructor used by slave to read the event from the binary log.
 */
Delete_rows_log_event::Delete_rows_log_event(
    const char *buf, const Format_description_event *description_event)
    : binary_log::Rows_event(buf, description_event),
      Rows_log_event(buf, description_event),
      binary_log::Delete_rows_event(buf, description_event) {
  DBUG_ASSERT(header()->type_code == m_type);
}

#if defined(MYSQL_SERVER)

int Delete_rows_log_event::do_before_row_operations(
    const Slave_reporting_capability *const, table_def *tabledef) {
  int error = 0;
  DBUG_TRACE;
  m_table->file->rpl_before_delete_rows();
  /*
    Increment the global status delete count variable
   */
  if (get_flags(STMT_END_F)) thd->status_var.com_stat[SQLCOM_DELETE]++;

  if (slave_run_triggers_for_rbr && !master_had_triggers)
    m_table->prepare_triggers_for_delete_stmt_or_event();

  /*
    Let storage engines treat this event as a DELETE command.

    Set 'sql_command' as SQLCOM_UPDATE after the tables are locked.
    When locking the tables, it should be SQLCOM_END.
    THD::decide_logging_format which is called from "lock tables"
    assumes that row_events will have 'sql_command' as SQLCOM_END.
  */
  thd->lex->sql_command = SQLCOM_DELETE;

  error = row_operations_scan_and_key_setup(tabledef);
  return error;
}

int Delete_rows_log_event::do_after_row_operations(
    const Slave_reporting_capability *const, int error) {
  DBUG_TRACE;
  error = row_operations_scan_and_key_teardown(error);
  m_table->file->rpl_after_delete_rows();
  return error;
}

int Delete_rows_log_event::do_exec_row(const Relay_log_info *const rli) {
  int error = 0;
  DBUG_ASSERT(m_table != nullptr);
  const bool invoke_triggers =
      slave_run_triggers_for_rbr && !master_had_triggers && m_table->triggers;

  if (m_rows_lookup_algorithm == ROW_LOOKUP_NOT_NEEDED) {
    error = unpack_current_row(rli, &m_cols, false /*not AI*/);
    if (error) return error;
  }

  /* m_table->record[0] contains the BI */
  m_table->mark_columns_per_binlog_row_image(thd);
  if (invoke_triggers &&
      process_triggers(TRG_EVENT_DELETE, TRG_ACTION_BEFORE, false))
    error = HA_ERR_GENERIC;  // in case if error is not set yet
  if (!error) error = m_table->file->ha_delete_row(m_table->record[0]);
  if (invoke_triggers && !error &&
      process_triggers(TRG_EVENT_DELETE, TRG_ACTION_AFTER, false))
    error = HA_ERR_GENERIC;  // in case if error is not set yet
  m_table->default_column_bitmaps();
  return error;
}

uint8 Delete_rows_log_event::get_trg_event_map() const noexcept {
  return static_cast<uint8>(1 << static_cast<int>(TRG_EVENT_DELETE));
}

#endif /* defined(MYSQL_SERVER) */

#ifndef MYSQL_SERVER
void Delete_rows_log_event::print(FILE *file,
                                  PRINT_EVENT_INFO *print_event_info) const {
  Rows_log_event::print_helper(file, print_event_info);
}
#endif

/**************************************************************************
        Update_rows_log_event member functions
**************************************************************************/

#if defined(MYSQL_SERVER)
binary_log::Log_event_type Update_rows_log_event::get_update_rows_event_type(
    const THD *thd_arg) {
  DBUG_TRACE;
  binary_log::Log_event_type type =
      (thd_arg->variables.binlog_row_value_options != 0
           ? binary_log::PARTIAL_UPDATE_ROWS_EVENT
           : (log_bin_use_v1_row_events ? binary_log::UPDATE_ROWS_EVENT_V1
                                        : binary_log::UPDATE_ROWS_EVENT));
  DBUG_PRINT("info", ("update_rows event_type: %s", get_type_str(type)));
  return type;
}

/*
  Constructor used to build an event for writing to the binary log.
 */
Update_rows_log_event::Update_rows_log_event(
    THD *thd_arg, TABLE *tbl_arg, const Table_id &tid, bool is_transactional,
    const unsigned char *extra_row_ndb_info)
    : binary_log::Rows_event(get_update_rows_event_type(thd_arg)),
      Rows_log_event(thd_arg, tbl_arg, tid, tbl_arg->read_set, is_transactional,
                     get_update_rows_event_type(thd_arg), extra_row_ndb_info),
      binary_log::Update_rows_event(get_update_rows_event_type(thd_arg)) {
  DBUG_TRACE;
  DBUG_PRINT("info", ("update_rows event_type: %s", get_type_str()));
  common_header->type_code = m_type;
  init(tbl_arg->write_set);
  common_header->set_is_valid(Rows_log_event::is_valid() && m_cols_ai.bitmap);
}

bool Update_rows_log_event::binlog_row_logging_function(
    THD *thd_arg, TABLE *table, bool is_transactional,
    const uchar *before_record, const uchar *after_record) {
  return thd_arg->binlog_update_row(table, is_transactional, before_record,
                                    after_record, nullptr);
}

void Update_rows_log_event::init(MY_BITMAP const *cols) {
  /* if bitmap_init fails, caught in is_valid() */
  if (likely(!bitmap_init(
          &m_cols_ai,
          m_width <= sizeof(m_bitbuf_ai) * 8 ? m_bitbuf_ai : nullptr,
          m_width))) {
    /* Cols can be zero if this is a dummy binrows event */
    if (likely(cols != nullptr)) {
      memcpy(m_cols_ai.bitmap, cols->bitmap, no_bytes_in_map(cols));
      create_last_word_mask(&m_cols_ai);
    }
  }
}
#endif /* defined(MYSQL_SERVER) */

Update_rows_log_event::~Update_rows_log_event() {
  if (m_cols_ai.bitmap) {
    if (m_cols_ai.bitmap == m_bitbuf_ai)  // no my_malloc happened
      m_cols_ai.bitmap = nullptr;         // so no my_free in bitmap_free
    bitmap_free(&m_cols_ai);              // To pair with bitmap_init().
  }
}

/*
  Constructor used by slave to read the event from the binary log.
 */
Update_rows_log_event::Update_rows_log_event(
    const char *buf, const Format_description_event *description_event)
    : binary_log::Rows_event(buf, description_event),
      Rows_log_event(buf, description_event),
      binary_log::Update_rows_event(buf, description_event) {
  DBUG_TRACE;
  if (!is_valid()) return;
  DBUG_ASSERT(header()->type_code == m_type);
  common_header->set_is_valid(m_cols_ai.bitmap);
}

#if defined(MYSQL_SERVER)

int Update_rows_log_event::do_before_row_operations(
    const Slave_reporting_capability *const, table_def *tabledef) {
  int error = 0;
  DBUG_TRACE;
  m_table->file->rpl_before_update_rows();
  /*
    Increment the global status update count variable
  */
  if (get_flags(STMT_END_F)) thd->status_var.com_stat[SQLCOM_UPDATE]++;

  if (slave_run_triggers_for_rbr && !master_had_triggers)
    m_table->prepare_triggers_for_update_stmt_or_event();

  /*
    Let storage engines treat this event as an UPDATE command.

    Set 'sql_command' as SQLCOM_UPDATE after the tables are locked.
    When locking the tables, it should be SQLCOM_END.
    THD::decide_logging_format which is called from "lock tables"
    assumes that row_events will have 'sql_command' as SQLCOM_END.
   */
  thd->lex->sql_command = SQLCOM_UPDATE;

  error = row_operations_scan_and_key_setup(tabledef);
  return error;
}

int Update_rows_log_event::do_after_row_operations(
    const Slave_reporting_capability *const, int error) {
  DBUG_TRACE;
  error = row_operations_scan_and_key_teardown(error);
  m_table->file->rpl_after_update_rows();
  return error;
}

int Update_rows_log_event::force_update(Relay_log_info const *rli,
                                        const uchar *const curr_row) {
  int error = 0;
  DBUG_ASSERT(m_table);

  /**
   * NOTE: At this point record[1] will have the current engine image for the
   * row (which might be different from the BI in the binlog event) and
   * record[0] will have the after image overlayed on record[1]. When
   * binlog_row_image is FULL record[0] will have the actual binlog row but when
   * it's COMPLETE it'll have only the changed columns overwritten on the
   * current engine image (record[1]). So, to insert the correct values for all
   * columns we unpack the BI again from the binlog and then unpack the AI over
   * it to get the correct full version of the row that needs to be inserted.
   *
   * In idempotent recovery case, just changing the cols in the AI should also
   * work since all mutations to other cols that don't match the current event
   * will be applied eventually. So, theoretically we could directly use
   * record[0]/[1] but this makes thinking about edge cases harder. The
   * invariant that force update of an event (or transaction) will bring the DB
   * to the state as if all rows were updated normally will break if we don't
   * construct records from the event, and that's a good invariant to have
   * during recovery. This is why we don't support row images which do not
   * enable contruction of full rows from BI and AI (e.g. MINIMAL and NOBLOB).
   * This is why we're checking for bitmap_is_set_all(&m_cols) below.
   *
   * TODO: Handle foreign keys
   */

  if (!bitmap_is_set_all(&m_cols)) return HA_ERR_GENERIC;

  error = m_table->file->ha_delete_row(m_table->record[1]);
  if (error == HA_ERR_KEY_NOT_FOUND) error = 0;
  if (error) return error;

  // reconstruct the row that need to be inserted (see note above)
  auto row_start = curr_row;
  uchar *row_end = NULL;
  bool has_value_options =
      (get_type_code() == binary_log::PARTIAL_UPDATE_ROWS_EVENT);
  prepare_record(m_table, &m_cols, false);

  // unpack BI into record[0]
  error = unpack_row(rli, m_table, m_width, row_start, &m_cols,
                     const_cast<const uchar **>(&row_end), m_rows_end,
                     enum_row_image_type::UPDATE_BI, has_value_options, false);
  row_start = row_end;
  // unpack AI into record[0] (if binlog_row_image is FULL we overwrite all
  // cols, if it's COMPLETE we overwrite only changed cols)
  error = error ||
          unpack_row(rli, m_table, m_width, row_start, &m_cols_ai,
                     const_cast<const uchar **>(&row_end), m_rows_end,
                     enum_row_image_type::UPDATE_AI, has_value_options, false);

  char *key = nullptr;
  while (!error && (error = m_table->file->ha_write_row(m_table->record[0]))) {
    if (error != HA_ERR_FOUND_DUPP_UNIQUE && error != HA_ERR_FOUND_DUPP_KEY)
      break;

    int keynum = m_table->file->get_dup_key(error);
    if (keynum < 0) return HA_ERR_GENERIC;

    if ((m_table->file->ha_table_flags() & HA_DUPLICATE_POS)) {
      if (m_table->file->inited) error = m_table->file->ha_index_end();
      error = error || m_table->file->ha_rnd_init(false);
      error = error || m_table->file->ha_rnd_pos(m_table->record[1],
                                                 m_table->file->dup_ref);
      m_table->file->ha_rnd_end();
    } else {
      if (keynum == MAX_KEY) return HA_ERR_GENERIC;

      if (key == nullptr) {
        key = static_cast<char *>(my_alloca(m_table->s->max_unique_length));
        if (key == nullptr) {
          DBUG_PRINT("info", ("Can't allocate key buffer"));
          return ENOMEM;
        }
      }

      key_copy((uchar *)key, m_table->record[0], m_table->key_info + keynum, 0);
      error = m_table->file->ha_index_read_idx_map(
          m_table->record[1], keynum, (const uchar *)key, HA_WHOLE_KEY,
          HA_READ_KEY_EXACT);
    }
    /* At this point record[1] should contain the offending row */
    error = error || m_table->file->ha_delete_row(m_table->record[1]);
  }
  // case: force_update() either passes or fails, so we convert idempotent
  // errors to a hard error which cannot be ignored
  if (error && idempotent_error_code(error)) {
    sql_print_error(
        "Got an idempotent error during "
        "Update_rows_log_event::force_update(), "
        "error: %d, pos: %s:%llu",
        error, rli->get_rpl_log_name(), common_header->log_pos);
    error = HA_ERR_GENERIC;
  }
  return error;
}

int Update_rows_log_event::do_exec_row(const Relay_log_info *const rli) {
  DBUG_ASSERT(m_table != nullptr);
  int error = 0;
  const bool invoke_triggers =
      slave_run_triggers_for_rbr && !master_had_triggers && m_table->triggers;

  if (m_rows_lookup_algorithm == ROW_LOOKUP_NOT_NEEDED) {
    error = unpack_current_row(rli, &m_cols, false /*not AI*/);
    if (error) return error;
  }

  const uchar *const saved_curr_row = m_curr_row;

  /*
    This is the situation after locating BI:

    ===|=== before image ====|=== after image ===|===
       ^                     ^
       m_curr_row            m_curr_row_end

    BI found in the table is stored in record[0]. We copy it to record[1]
    and unpack AI to record[0].
   */

  store_record(m_table, record[1]);

  m_curr_row = m_curr_row_end;
  /* this also updates m_curr_row_end */
  if ((error = unpack_current_row(rli, &m_cols_ai, true /*is AI*/)))
    return error;

  // Invoke check constraints on the unpacked row.
  if (invoke_table_check_constraints(thd, m_table))
    return ER_CHECK_CONSTRAINT_VIOLATED;

  /*
    Now we have the right row to update.  The old row (the one we're
    looking for) is in record[1] and the new row is in record[0].
  */
  DBUG_PRINT("info", ("Updating row in table"));
  DBUG_DUMP("old record", m_table->record[1], m_table->s->reclength);
  DBUG_DUMP("new values", m_table->record[0], m_table->s->reclength);

  m_table->mark_columns_per_binlog_row_image(thd);
  if (invoke_triggers &&
      process_triggers(TRG_EVENT_UPDATE, TRG_ACTION_BEFORE, true)) {
    error = HA_ERR_GENERIC;  // in case if error is not set yet
    goto err;
  }
  if (m_force_binlog_idempotent)
    error = force_update(rli, saved_curr_row);
  else
    error =
        m_table->file->ha_update_row(m_table->record[1], m_table->record[0]);
  if (error == HA_ERR_RECORD_IS_THE_SAME) error = 0;
  if (invoke_triggers && !error &&
      process_triggers(TRG_EVENT_UPDATE, TRG_ACTION_AFTER, true))
    error = HA_ERR_GENERIC;  // in case if error is not set yet

err:
  m_table->default_column_bitmaps();

  return error;
}

uint8 Update_rows_log_event::get_trg_event_map() const noexcept {
  return static_cast<uint8>(1 << static_cast<int>(TRG_EVENT_UPDATE));
}

#endif /* defined(MYSQL_SERVER) */

#ifndef MYSQL_SERVER
void Update_rows_log_event::print(FILE *file,
                                  PRINT_EVENT_INFO *print_event_info) const {
  Rows_log_event::print_helper(file, print_event_info);
}
#endif

Incident_log_event::Incident_log_event(
    const char *buf, const Format_description_event *description_event)
    : binary_log::Incident_event(buf, description_event),
      Log_event(header(), footer()) {
  DBUG_TRACE;
}

Incident_log_event::~Incident_log_event() {
  if (message) bapi_free(message);
}

const char *Incident_log_event::description() const {
  static const char *const description[] = {"NOTHING",  // Not used
                                            "LOST_EVENTS"};

  DBUG_PRINT("info", ("incident: %d", incident));

  return description[incident];
}

#ifdef MYSQL_SERVER
int Incident_log_event::pack_info(Protocol *protocol) {
  char buf[256];
  size_t bytes;
  if (message_length > 0)
    bytes = snprintf(buf, sizeof(buf), "#%d (%s)", incident, description());
  else
    bytes = snprintf(buf, sizeof(buf), "#%d (%s): %s", incident, description(),
                     message);
  protocol->store_string(buf, bytes, &my_charset_bin);
  return 0;
}
#endif

#ifndef MYSQL_SERVER
void Incident_log_event::print(FILE *,
                               PRINT_EVENT_INFO *print_event_info) const {
  if (print_event_info->short_form) return;

  print_header(&print_event_info->head_cache, print_event_info, false);
  my_b_printf(
      &print_event_info->head_cache,
      "\n# Incident: %s\nRELOAD DATABASE; # Shall generate syntax error\n",
      description());
}
#endif

#if defined(MYSQL_SERVER)
int Incident_log_event::do_apply_event(Relay_log_info const *rli) {
  DBUG_TRACE;

  /*
    It is not necessary to do GTID related check if the error
    'ER_SLAVE_INCIDENT' is ignored.
  */
  if (ignored_error_code(ER_SLAVE_INCIDENT)) {
    DBUG_PRINT("info", ("Ignoring Incident"));
    mysql_bin_log.gtid_end_transaction(thd);
    return 0;
  }

  enum_gtid_statement_status state = gtid_pre_statement_checks(thd);
  if (state == GTID_STATEMENT_EXECUTE) {
    if (gtid_pre_statement_post_implicit_commit_checks(thd))
      state = GTID_STATEMENT_CANCEL;
  }

  if (state == GTID_STATEMENT_CANCEL) {
    uint error = thd->get_stmt_da()->mysql_errno();
    DBUG_ASSERT(error != 0);
    rli->report(ERROR_LEVEL, error, "Error executing incident event: '%s'",
                thd->get_stmt_da()->message_text());
    thd->is_slave_error = true;
    return -1;
  } else if (state == GTID_STATEMENT_SKIP) {
    /*
      Make slave skip the Incident event through general commands of GTID
      i.e. 'set gtid_next=<GTID>; begin; commit;'.
    */
    return 0;
  }

  rli->report(ERROR_LEVEL, ER_SLAVE_INCIDENT, ER_THD(thd, ER_SLAVE_INCIDENT),
              description(), message_length > 0 ? message : "<none>");
  return 1;
}

bool Incident_log_event::write_data_header(Basic_ostream *ostream) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("incident: %d", incident));
  uchar buf[sizeof(int16)];
  int2store(buf, (int16)incident);
  return wrapper_my_b_safe_write(ostream, buf, sizeof(buf));
}

/*
  Stores string to an output stream.

  Writes str to file in the following format:
   1. Stores length using only one byte (255 maximum value);
   2. Stores complete str.
*/

static bool write_str_at_most_255_bytes(Basic_ostream *ostream, const char *str,
                                        uint length) {
  uchar tmp[1];

  tmp[0] = (uchar)length;
  return (
      ostream->write(tmp, sizeof(tmp)) ||
      (length > 0 && ostream->write(pointer_cast<const uchar *>(str), length)));
}

bool Incident_log_event::write_data_body(Basic_ostream *ostream) {
  uchar tmp[1];
  DBUG_TRACE;
  tmp[0] = (uchar)message_length;
  crc = checksum_crc32(crc, (uchar *)tmp, 1);
  if (message_length > 0) {
    crc = checksum_crc32(crc, (uchar *)message, message_length);
    // todo: report a bug on write_str accepts uint but treats it as uchar
  }
  return write_str_at_most_255_bytes(ostream, message, (uint)message_length);
}
#endif

Ignorable_log_event::Ignorable_log_event(
    const char *buf, const Format_description_event *descr_event)
    : binary_log::Ignorable_event(buf, descr_event),
      Log_event(header(), footer()) {
  DBUG_TRACE;
}

Ignorable_log_event::~Ignorable_log_event() {}

#ifdef MYSQL_SERVER
/* Pack info for its unrecognized ignorable event */
int Ignorable_log_event::pack_info(Protocol *protocol) {
  char buf[256];
  size_t bytes;
  bytes = snprintf(buf, sizeof(buf), "# Unrecognized ignorable event");
  protocol->store_string(buf, bytes, &my_charset_bin);
  return 0;
}
#endif

#ifndef MYSQL_SERVER
/* Print for its unrecognized ignorable event */
void Ignorable_log_event::print(FILE *,
                                PRINT_EVENT_INFO *print_event_info) const {
  if (print_event_info->short_form) return;

  print_header(&print_event_info->head_cache, print_event_info, false);
  my_b_printf(&print_event_info->head_cache, "\tIgnorable\n");
  my_b_printf(&print_event_info->head_cache,
              "# Unrecognized ignorable event\n");
}
#endif

static const std::string ts_key{"ts"};

ulonglong Rows_query_log_event::extract_last_timestamp() const {
  rapidjson::Document pt;
  std::string json = extract_trx_meta_data();
  if (json.empty()) return 0;
  if (pt.Parse(json.c_str()).HasParseError()) {
    return 0;
  }
  const auto ts_iter = pt.FindMember(ts_key.c_str());
  ulonglong timestamp = 0;
  if (ts_iter != pt.MemberEnd() && ts_iter->value.IsArray()) {
    const auto &arr = ts_iter->value.GetArray();
    if (arr.Size() >= 1) {
      const auto &value = arr[arr.Size() - 1];
      timestamp = my_strtoull(value.GetString(), NULL, 10);
    }
  }
  return timestamp;
}

Rows_query_log_event::Rows_query_log_event(
    const char *buf, const Format_description_event *descr_event)
    : binary_log::Ignorable_event(buf, descr_event),
      Ignorable_log_event(buf, descr_event),
      binary_log::Rows_query_event(buf, descr_event) {
  DBUG_TRACE;
}

#ifdef MYSQL_SERVER
int Rows_query_log_event::pack_info(Protocol *protocol) {
  char *buf;
  size_t bytes;
  size_t len = sizeof("# ") + strlen(m_rows_query);
  if (!(buf = (char *)my_malloc(key_memory_log_event, len, MYF(MY_WME))))
    return 1;
  bytes = snprintf(buf, len, "# %s", m_rows_query);
  protocol->store_string(buf, bytes, &my_charset_bin);
  my_free(buf);
  return 0;
}
#endif

#ifndef MYSQL_SERVER
void Rows_query_log_event::print(FILE *,
                                 PRINT_EVENT_INFO *print_event_info) const {
  if (!print_event_info->short_form && print_event_info->verbose > 1) {
    IO_CACHE *const head = &print_event_info->head_cache;
    IO_CACHE *const body = &print_event_info->body_cache;
    char *token = nullptr, *saveptr = nullptr;
    char *rows_query_copy = nullptr;
    if (!(rows_query_copy =
              my_strdup(key_memory_log_event, m_rows_query, MYF(MY_WME))))
      return;

    print_header(head, print_event_info, false);
    my_b_printf(head, "\tRows_query\n");
    /*
      Prefix every line of a multi-line query with '#' to prevent the
      statement from being executed when binary log will be processed
      using 'mysqlbinlog --verbose --verbose'.
    */
    for (token = my_strtok_r(rows_query_copy, "\n", &saveptr); token;
         token = my_strtok_r(nullptr, "\n", &saveptr))
      my_b_printf(head, "# %s\n", token);
    my_free(rows_query_copy);
    print_base64(body, print_event_info, true);
  }
}
#endif
uint Rows_query_log_event::get_comment_length(const char *str,
                                              uint length) const {
  const char *pos = str;
  const char *end = str + length;
  int comment_length = 0;
comment_begin:
  // In the first loop, look to see if the query text starts with a comment
  while (pos < end) {
    if (my_isspace(&my_charset_latin1, *pos)) {
      // Skip spaces
      pos++;
      continue;
    } else if (*pos == '/' && (pos + 1) < end && *(pos + 1) == '*') {
      // Comment begins here. Find the end of the comment in the while loop
      // below.
      pos += 2;
      break;
    }
    return comment_length ? comment_length : length;
  }
  while (pos < end) {
    // We only get here if the first loop found the start of a comment.
    // Now look for the end of one.
    if (*pos++ == '*') {
      if (pos < end && *pos == '/') {
        pos++;
        comment_length = pos - str;
        // A query can have multiple comments, so start matching
        // the comment again.
        goto comment_begin;
      }
    }
  }
  return comment_length ? comment_length : length;
}

#if defined(MYSQL_SERVER)
bool Rows_query_log_event::write_data_body(Basic_ostream *ostream) {
  DBUG_TRACE;
  /*
   m_rows_query length will be stored using only one byte, but on read
   that length will be ignored and the complete query will be read.
  */
  return write_str_at_most_255_bytes(ostream, m_rows_query,
                                     strlen(m_rows_query));
}

int Rows_query_log_event::do_apply_event(Relay_log_info const *rli) {
  DBUG_TRACE;
  DBUG_ASSERT(rli->info_thd == thd);

  // case: this is not a regular rows query event, it contains meta data about
  // the transaction
  if (has_trx_meta_data()) {
    const_cast<Relay_log_info *>(rli)->trx_meta_data_json =
        extract_trx_meta_data();

    DBUG_ASSERT(strstr(m_rows_query, "*/") != nullptr);
    // actual query starts after the metadata
    auto after_metadata = strstr(m_rows_query, "*/");

    // invalid trx metadata comment
    if (after_metadata == nullptr) {
      return 0;
    }

    // Skip "*/"
    after_metadata += 2;

    auto len = strlen(after_metadata);
    DBUG_PRINT("info", ("query: %s", after_metadata));
    thd->set_query(after_metadata, (uint32)len);
    thd->set_query_for_display(after_metadata, (uint32)len);
  } else {
    /* Set query for writing Rows_query log event into binlog later.*/
    thd->set_query(m_rows_query, (uint32)strlen(m_rows_query));
    thd->set_query_for_display(m_rows_query, (uint32)strlen(m_rows_query));
  }

  DBUG_ASSERT(rli->rows_query_ev == nullptr);

  const_cast<Relay_log_info *>(rli)->rows_query_ev = this;
  /* Tell worker not to free the event */
  worker = nullptr;
  return 0;
}
#endif

const char *Gtid_log_event::SET_STRING_PREFIX = "SET @@SESSION.GTID_NEXT= '";

Gtid_log_event::Gtid_log_event(
    const char *buffer, const Format_description_event *description_event)
    : binary_log::Gtid_event(buffer, description_event),
      Log_event(header(), footer()) {
  DBUG_TRACE;
  if (!is_valid()) return;

#ifndef DBUG_OFF
  uint8_t const common_header_len = description_event->common_header_len;
  uint8 const post_header_len =
      buffer[EVENT_TYPE_OFFSET] == binary_log::ANONYMOUS_GTID_LOG_EVENT
          ? description_event
                ->post_header_len[binary_log::ANONYMOUS_GTID_LOG_EVENT - 1]
          : description_event->post_header_len[binary_log::GTID_LOG_EVENT - 1];
  DBUG_PRINT("info",
             ("event_len: %zu; common_header_len: %d; post_header_len: %d",
              header()->data_written, common_header_len, post_header_len));
#endif

  spec.type = get_type_code() == binary_log::ANONYMOUS_GTID_LOG_EVENT
                  ? ANONYMOUS_GTID
                  : ASSIGNED_GTID;
  sid.copy_from((uchar *)Uuid_parent_struct.bytes);
  spec.gtid.sidno = gtid_info_struct.rpl_gtid_sidno;
  spec.gtid.gno = gtid_info_struct.rpl_gtid_gno;
}

#ifdef MYSQL_SERVER
Gtid_log_event::Gtid_log_event(THD *thd_arg, bool using_trans,
                               int64 last_committed_arg,
                               int64 sequence_number_arg,
                               bool may_have_sbr_stmts_arg,
                               ulonglong original_commit_timestamp_arg,
                               ulonglong immediate_commit_timestamp_arg,
                               uint32_t original_server_version_arg,
                               uint32_t immediate_server_version_arg)
    : binary_log::Gtid_event(
          last_committed_arg, sequence_number_arg, may_have_sbr_stmts_arg,
          original_commit_timestamp_arg, immediate_commit_timestamp_arg,
          original_server_version_arg, immediate_server_version_arg),
      Log_event(thd_arg,
                thd_arg->variables.gtid_next.type == ANONYMOUS_GTID
                    ? LOG_EVENT_IGNORABLE_F
                    : 0,
                using_trans ? Log_event::EVENT_TRANSACTIONAL_CACHE
                            : Log_event::EVENT_STMT_CACHE,
                Log_event::EVENT_NORMAL_LOGGING, header(), footer()) {
  DBUG_TRACE;
  if (thd->owned_gtid.sidno > 0) {
    spec.set(thd->owned_gtid);
    sid = thd->owned_sid;
  } else {
    DBUG_ASSERT(thd->owned_gtid.sidno == THD::OWNED_SIDNO_ANONYMOUS);
    spec.set_anonymous();
    spec.gtid.clear();
    sid.clear();
  }

  Log_event_type event_type =
      (spec.type == ANONYMOUS_GTID ? binary_log::ANONYMOUS_GTID_LOG_EVENT
                                   : binary_log::GTID_LOG_EVENT);
  common_header->type_code = event_type;

#ifndef DBUG_OFF
  char buf[MAX_SET_STRING_LENGTH + 1];
  to_string(buf);
  DBUG_PRINT("info", ("%s", buf));
#endif
  common_header->set_is_valid(true);
}

Gtid_log_event::Gtid_log_event(
    uint32 server_id_arg, bool using_trans, int64 last_committed_arg,
    int64 sequence_number_arg, bool may_have_sbr_stmts_arg,
    ulonglong original_commit_timestamp_arg,
    ulonglong immediate_commit_timestamp_arg, const Gtid_specification spec_arg,
    uint32_t original_server_version_arg, uint32_t immediate_server_version_arg)
    : binary_log::Gtid_event(
          last_committed_arg, sequence_number_arg, may_have_sbr_stmts_arg,
          original_commit_timestamp_arg, immediate_commit_timestamp_arg,
          original_server_version_arg, immediate_server_version_arg),
      Log_event(header(), footer(),
                using_trans ? Log_event::EVENT_TRANSACTIONAL_CACHE
                            : Log_event::EVENT_STMT_CACHE,
                Log_event::EVENT_NORMAL_LOGGING) {
  DBUG_TRACE;
  server_id = server_id_arg;
  common_header->unmasked_server_id = server_id_arg;

  if (spec_arg.type == ASSIGNED_GTID) {
    DBUG_ASSERT(spec_arg.gtid.sidno > 0 && spec_arg.gtid.gno > 0);
    spec.set(spec_arg.gtid);
    global_sid_lock->rdlock();
    sid = global_sid_map->sidno_to_sid(spec_arg.gtid.sidno);
    global_sid_lock->unlock();
  } else {
    DBUG_ASSERT(spec_arg.type == ANONYMOUS_GTID);
    spec.set_anonymous();
    spec.gtid.clear();
    sid.clear();
    common_header->flags |= LOG_EVENT_IGNORABLE_F;
  }

  Log_event_type event_type =
      (spec.type == ANONYMOUS_GTID ? binary_log::ANONYMOUS_GTID_LOG_EVENT
                                   : binary_log::GTID_LOG_EVENT);
  common_header->type_code = event_type;

#ifndef DBUG_OFF
  char buf[MAX_SET_STRING_LENGTH + 1];
  to_string(buf);
  DBUG_PRINT("info", ("%s", buf));
#endif
  common_header->set_is_valid(true);
}

int Gtid_log_event::pack_info(Protocol *protocol) {
  char buffer[MAX_SET_STRING_LENGTH + 1];
  size_t len = to_string(buffer);
  protocol->store_string(buffer, len, &my_charset_bin);
  return 0;
}
#endif  // MYSQL_SERVER

size_t Gtid_log_event::to_string(char *buf) const {
  char *p = buf;
  DBUG_ASSERT(strlen(SET_STRING_PREFIX) == SET_STRING_PREFIX_LENGTH);
  strcpy(p, SET_STRING_PREFIX);
  p += SET_STRING_PREFIX_LENGTH;
  p += spec.to_string(&sid, p);
  *p++ = '\'';
  *p = '\0';
  return p - buf;
}

#ifndef MYSQL_SERVER
void Gtid_log_event::print(FILE *, PRINT_EVENT_INFO *print_event_info) const {
  char buffer[MAX_SET_STRING_LENGTH + 1];
  IO_CACHE *const head = &print_event_info->head_cache;
  if (!print_event_info->short_form) {
    print_header(head, print_event_info, false);
    my_b_printf(head,
                "\t%s\tlast_committed=%llu\tsequence_number=%llu\t"
                "rbr_only=%s\t"
                "original_committed_timestamp=%llu\t"
                "immediate_commit_timestamp=%llu\t"
                "transaction_length=%llu\n",
                get_type_code() == binary_log::GTID_LOG_EVENT
                    ? "GTID"
                    : "Anonymous_GTID",
                last_committed, sequence_number,
                may_have_sbr_stmts ? "no" : "yes", original_commit_timestamp,
                immediate_commit_timestamp, transaction_length);
  }

  /*
    The applier thread can always use "READ COMMITTED" isolation for
    transactions containing only RBR events (Table_map + Rows).

    This would prevent some deadlock issues because InnoDB doesn't
    acquire GAP locks in "READ COMMITTED" isolation level since
    MySQL 5.7.18.
  */
  if (!may_have_sbr_stmts) {
    my_b_printf(head,
                "/*!50718 SET TRANSACTION ISOLATION LEVEL "
                "READ COMMITTED*/%s\n",
                print_event_info->delimiter);
  }

  /*
    We always print the original commit timestamp in order to make
    dumps from binary logs generated on servers without this info on
    GTID events to print "0" (not known) as the session value.
  */
  char llbuf[22];

  char immediate_commit_timestamp_str[256];
  char original_commit_timestamp_str[256];

  microsecond_timestamp_to_str(immediate_commit_timestamp,
                               immediate_commit_timestamp_str);
  microsecond_timestamp_to_str(original_commit_timestamp,
                               original_commit_timestamp_str);

  my_b_printf(head, "# original_commit_timestamp=%s (%s)\n",
              llstr(original_commit_timestamp, llbuf),
              original_commit_timestamp_str);
  my_b_printf(head, "# immediate_commit_timestamp=%s (%s)\n",
              llstr(immediate_commit_timestamp, llbuf),
              immediate_commit_timestamp_str);

  if (DBUG_EVALUATE_IF("do_not_write_rpl_OCT", false, true)) {
    my_b_printf(
        head, "/*!80001 SET @@session.original_commit_timestamp=%s*/%s\n",
        llstr(original_commit_timestamp, llbuf), print_event_info->delimiter);
  }

  my_b_printf(head, "/*!80014 SET @@session.original_server_version=%u*/%s\n",
              original_server_version, print_event_info->delimiter);

  my_b_printf(head, "/*!80014 SET @@session.immediate_server_version=%u*/%s\n",
              immediate_server_version, print_event_info->delimiter);

  to_string(buffer);
  my_b_printf(head, "%s%s\n", buffer, print_event_info->delimiter);
}
#endif

#ifdef MYSQL_SERVER
uint32 Gtid_log_event::write_post_header_to_memory(uchar *buffer) {
  DBUG_TRACE;
  uchar *ptr_buffer = buffer;

  /* Encode the GTID flags */
  uchar gtid_flags = 0;
  gtid_flags |=
      may_have_sbr_stmts ? binary_log::Gtid_event::FLAG_MAY_HAVE_SBR : 0;
  *ptr_buffer = gtid_flags;
  ptr_buffer += ENCODED_FLAG_LENGTH;

#ifndef DBUG_OFF
  char buf[binary_log::Uuid::TEXT_LENGTH + 1];
  sid.to_string(buf);
  DBUG_PRINT("info",
             ("sid=%s sidno=%d gno=%lld", buf, spec.gtid.sidno, spec.gtid.gno));
#endif

  sid.copy_to(ptr_buffer);
  ptr_buffer += ENCODED_SID_LENGTH;

  int8store(ptr_buffer, spec.gtid.gno);
  ptr_buffer += ENCODED_GNO_LENGTH;

  *ptr_buffer = LOGICAL_TIMESTAMP_TYPECODE;
  ptr_buffer += LOGICAL_TIMESTAMP_TYPECODE_LENGTH;

  DBUG_ASSERT((sequence_number == 0 && last_committed == 0) ||
              (sequence_number > last_committed));
  DBUG_EXECUTE_IF("set_commit_parent_100", {
    last_committed =
        max<int64>(sequence_number > 1 ? 1 : 0, sequence_number - 100);
  });
  DBUG_EXECUTE_IF("set_commit_parent_150", {
    last_committed =
        max<int64>(sequence_number > 1 ? 1 : 0, sequence_number - 150);
  });
  DBUG_EXECUTE_IF("feign_commit_parent", { last_committed = sequence_number; });
  int8store(ptr_buffer, last_committed);
  int8store(ptr_buffer + 8, sequence_number);
  ptr_buffer += LOGICAL_TIMESTAMP_LENGTH;

  DBUG_ASSERT(ptr_buffer == (buffer + POST_HEADER_LENGTH));

  return POST_HEADER_LENGTH;
}

#ifdef MYSQL_SERVER
bool Gtid_log_event::write_data_header(Basic_ostream *ostream) {
  DBUG_TRACE;
  uchar buffer[POST_HEADER_LENGTH];
  write_post_header_to_memory(buffer);
  return wrapper_my_b_safe_write(ostream, (uchar *)buffer, POST_HEADER_LENGTH);
}

uint32 Gtid_log_event::write_body_to_memory(uchar *buffer) {
  DBUG_TRACE;
  DBUG_EXECUTE_IF("do_not_write_rpl_timestamps", return 0;);
  uchar *ptr_buffer = buffer;

  /*
    We want to modify immediate_commit_timestamp with the flag written
    in the highest bit(MSB). At the same time, we also want to have the original
    value to be able to use in if() later, so we use a temporary variable here.
  */
  ulonglong immediate_commit_timestamp_with_flag = immediate_commit_timestamp;

  // Transaction did not originate at this server, set highest bit to hint this.
  if (immediate_commit_timestamp != original_commit_timestamp)
    immediate_commit_timestamp_with_flag |=
        (1ULL << ENCODED_COMMIT_TIMESTAMP_LENGTH);
  else  // Clear highest bit(MSB)
    immediate_commit_timestamp_with_flag &=
        ~(1ULL << ENCODED_COMMIT_TIMESTAMP_LENGTH);

  int7store(ptr_buffer, immediate_commit_timestamp_with_flag);
  ptr_buffer += IMMEDIATE_COMMIT_TIMESTAMP_LENGTH;

  if (immediate_commit_timestamp != original_commit_timestamp) {
    int7store(ptr_buffer, original_commit_timestamp);
    ptr_buffer += ORIGINAL_COMMIT_TIMESTAMP_LENGTH;
  }

  // Write the transaction length information
  uchar *ptr_after_length = net_store_length(ptr_buffer, transaction_length);
  ptr_buffer = ptr_after_length;

  /*
    We want to modify immediate_server_version with the flag written to its MSB.
    At the same time, we also want to have the original value to be able to use
    it in if() later, so we use a temporary variable here.
  */
  uint32_t immediate_server_version_with_flag = immediate_server_version;

  if (immediate_server_version != original_server_version)
    immediate_server_version_with_flag |=
        (1ULL << ENCODED_SERVER_VERSION_LENGTH);
  else  // Clear MSB
    immediate_server_version_with_flag &=
        ~(1ULL << ENCODED_SERVER_VERSION_LENGTH);

  int4store(ptr_buffer, immediate_server_version_with_flag);
  ptr_buffer += IMMEDIATE_SERVER_VERSION_LENGTH;

  if (immediate_server_version != original_server_version) {
    int4store(ptr_buffer, original_server_version);
    ptr_buffer += ORIGINAL_SERVER_VERSION_LENGTH;
  }

  return ptr_buffer - buffer;
}

bool Gtid_log_event::write_data_body(Basic_ostream *ostream) {
  DBUG_TRACE;
  uchar buffer[MAX_DATA_LENGTH];
  uint32 len = write_body_to_memory(buffer);
  return wrapper_my_b_safe_write(ostream, (uchar *)buffer, len);
}

#endif  // MYSQL_SERVER

int Gtid_log_event::do_apply_event(Relay_log_info const *rli) {
  DBUG_TRACE;
  DBUG_ASSERT(rli->info_thd == thd);

  /*
    In rare cases it is possible that we already own a GTID (either
    ANONYMOUS or ASSIGNED_GTID). This can happen if a transaction was truncated
    in the middle in the relay log and then next relay log begins with a
    Gtid_log_events without closing the transaction context from the previous
    relay log. In this case the only sensible thing to do is to discard the
    truncated transaction and move on.

    Note that when the applier is "GTID skipping" a transactions it
    owns nothing, but its gtid_next->type == ASSIGNED_GTID.
  */
  const Gtid_specification *gtid_next = &thd->variables.gtid_next;
  if (!thd->owned_gtid_is_empty() ||
      (thd->owned_gtid_is_empty() && gtid_next->type == ASSIGNED_GTID)) {
    /*
      Slave will execute this code if a previous Gtid_log_event was applied
      but the GTID wasn't consumed yet (the transaction was not committed,
      nor rolled back, nor skipped).
      On a client session we cannot do consecutive SET GTID_NEXT without
      a COMMIT or a ROLLBACK in the middle.
      Applying this event without rolling back the current transaction may
      lead to problems, as a "BEGIN" event following this GTID will
      implicitly commit the "partial transaction" and will consume the
      GTID. If this "partial transaction" was left in the relay log by the
      IO thread restarting in the middle of a transaction, you could have
      the partial transaction being logged with the GTID on the slave,
      causing data corruption on replication.
    */
    if (thd->server_status & SERVER_STATUS_IN_TRANS) {
      /* This is not an error (XA is safe), just an information */
      rli->report(INFORMATION_LEVEL, 0,
                  "Rolling back unfinished transaction (no COMMIT "
                  "or ROLLBACK in relay log). A probable cause is partial "
                  "transaction left on relay log because of restarting IO "
                  "thread with auto-positioning protocol.");
      const_cast<Relay_log_info *>(rli)->cleanup_context(thd, true);
    }
    gtid_state->update_on_rollback(thd);
  }

  global_sid_lock->rdlock();

  // make sure that sid has been converted to sidno
  if (spec.type == ASSIGNED_GTID) {
    if (get_sidno(false) < 0) {
      global_sid_lock->unlock();
      return 1;  // out of memory
    }
  }

  // set_gtid_next releases global_sid_lock
  if (set_gtid_next(thd, spec))
    // This can happen e.g. if gtid_mode is incompatible with spec.
    return 1;

  /*
    Set the original_commit_timestamp.
    0 will be used if this event does not contain such information.
  */
  enum_gtid_statement_status state = gtid_pre_statement_checks(thd);
  thd->variables.original_commit_timestamp = original_commit_timestamp;
  thd->set_original_commit_timestamp_for_slave_thread();
  /**
    Set the original/immediate server version.
    It will be set to UNKNOWN_SERVER_VERSION if the event does not contain such
    information.
   */
  thd->variables.original_server_version = original_server_version;
  thd->variables.immediate_server_version = immediate_server_version;
  const_cast<Relay_log_info *>(rli)->started_processing(
      thd->variables.gtid_next.gtid, original_commit_timestamp,
      immediate_commit_timestamp, state == GTID_STATEMENT_SKIP);

  /*
    If the current transaction contains no changes logged with SBR
    we can assume this transaction as a pure row based replicated one.

    Based on this assumption, we can set current transaction tx_isolation to
    READ COMMITTED in order to avoid concurrent transactions to be blocked by
    InnoDB gap locks.

    The session tx_isolation will be restored:
    - When the transaction finishes with QUERY(COMMIT|ROLLBACK),
      as the MySQL server does for ordinary user sessions;
    - When applying a Xid_log_event, after committing the transaction;
    - When applying a XA_prepare_log_event, after preparing the transaction;
    - When the applier needs to abort a transaction execution.

    Notice that when a transaction is being "gtid skipped", its statements are
    not actually executed (see mysql_execute_command()). So, the call to the
    function that would restore the tx_isolation after finishing the transaction
    may not happen.
  */
  if (DBUG_EVALUATE_IF(
          "force_trx_as_rbr_only", true,
          !may_have_sbr_stmts && thd->tx_isolation > ISO_READ_COMMITTED &&
              gtid_pre_statement_checks(thd) != GTID_STATEMENT_SKIP)) {
    DBUG_ASSERT(thd->get_transaction()->is_empty(Transaction_ctx::STMT));
    DBUG_ASSERT(thd->get_transaction()->is_empty(Transaction_ctx::SESSION));
    DBUG_ASSERT(!thd->lock);
    DBUG_PRINT("info", ("setting tx_isolation to READ COMMITTED"));
    set_tx_isolation(thd, ISO_READ_COMMITTED, true /*one_shot*/);
  }

  return 0;
}

int Gtid_log_event::do_update_pos(Relay_log_info *rli) {
  /*
    This event does not increment group positions. This means
    that if there is a failure after it has been processed,
    it will be automatically re-executed.
  */
  rli->inc_event_relay_log_pos();
  DBUG_EXECUTE_IF(
      "crash_after_update_pos_gtid",
      sql_print_information("Crashing crash_after_update_pos_gtid.");
      DBUG_SUICIDE(););
  return 0;
}

Log_event::enum_skip_reason Gtid_log_event::do_shall_skip(Relay_log_info *rli) {
  return Log_event::continue_group(rli);
}

void Gtid_log_event::extract_last_gtid_to(char *last_gtid) const noexcept {
  spec.to_string(&sid, last_gtid);
}
#endif  // MYSQL_SERVER

void Gtid_log_event::set_trx_length_by_cache_size(ulonglong cache_size,
                                                  bool is_checksum_enabled,
                                                  int event_counter) {
  // Transaction content length
  transaction_length = cache_size;
  if (is_checksum_enabled)
    transaction_length += event_counter * BINLOG_CHECKSUM_LEN;

  // GTID length
  transaction_length += LOG_EVENT_HEADER_LEN;
  transaction_length += POST_HEADER_LENGTH;
  transaction_length += get_commit_timestamp_length();
  transaction_length += get_server_version_length();
  transaction_length += is_checksum_enabled ? BINLOG_CHECKSUM_LEN : 0;

  /*
    Notice that it is not possible to determine the transaction_length field
    size using pack.cc:net_length_size() since the length of the field itself
    must be added to the value.

    Example: Suppose transaction_length is 250 without considering the
    transaction_length field. Using net_length_size(250) would return 1, but
    when adding the transaction_length field size to it (+1), the
    transaction_length becomes 251, and the field must be represented using two
    more bytes, so the correct transaction length must be in fact 253.
  */
#ifndef DBUG_OFF
  ulonglong size_without_transaction_length = transaction_length;
#endif
  // transaction_length will use at least TRANSACTION_LENGTH_MIN_LENGTH
  transaction_length += TRANSACTION_LENGTH_MIN_LENGTH;
  DBUG_ASSERT(transaction_length - size_without_transaction_length == 1);
  if (transaction_length >= 251ULL) {
    // transaction_length will use at least 3 bytes
    transaction_length += 2;
    DBUG_ASSERT(transaction_length - size_without_transaction_length == 3);
    if (transaction_length >= 65536ULL) {
      // transaction_length will use at least 4 bytes
      transaction_length += 1;
      DBUG_ASSERT(transaction_length - size_without_transaction_length == 4);
      if (transaction_length >= 16777216ULL) {
        // transaction_length will use 9 bytes
        transaction_length += 5;
        DBUG_ASSERT(transaction_length - size_without_transaction_length == 9);
      }
    }
  }
}

rpl_sidno Gtid_log_event::get_sidno(bool need_lock) {
  if (spec.gtid.sidno < 0) {
    if (need_lock)
      global_sid_lock->rdlock();
    else
      global_sid_lock->assert_some_lock();
    spec.gtid.sidno = global_sid_map->add_sid(sid);
    if (need_lock) global_sid_lock->unlock();
  }
  return spec.gtid.sidno;
}

Previous_gtids_log_event::Previous_gtids_log_event(
    const char *buf_arg, const Format_description_event *description_event)
    : binary_log::Previous_gtids_event(buf_arg, description_event),
      Log_event(header(), footer()) {
  DBUG_TRACE;
}

#ifdef MYSQL_SERVER
Previous_gtids_log_event::Previous_gtids_log_event(const Gtid_set *set)
    : binary_log::Previous_gtids_event(),
      Log_event(header(), footer(), Log_event::EVENT_NO_CACHE,
                Log_event::EVENT_IMMEDIATE_LOGGING) {
  DBUG_TRACE;
  common_header->type_code = binary_log::PREVIOUS_GTIDS_LOG_EVENT;
  common_header->flags |= LOG_EVENT_IGNORABLE_F;
  set->get_sid_map()->get_sid_lock()->assert_some_lock();
  buf_size = set->get_encoded_length();
  uchar *buffer =
      (uchar *)my_malloc(key_memory_log_event, buf_size, MYF(MY_WME));
  if (buffer != nullptr) {
    set->encode(buffer);
    register_temp_buf((char *)buffer);
  }
  buf = buffer;
  // if buf is empty, is_valid will be false
  common_header->set_is_valid(buf != nullptr);
}

int Previous_gtids_log_event::pack_info(Protocol *protocol) {
  size_t length = 0;
  char *str = get_str(&length, &Gtid_set::default_string_format);
  if (str == nullptr) return 1;
  protocol->store_string(str, length, &my_charset_bin);
  my_free(str);
  return 0;
}
#endif  // MYSQL_SERVER

#ifndef MYSQL_SERVER
void Previous_gtids_log_event::print(FILE *,
                                     PRINT_EVENT_INFO *print_event_info) const {
  IO_CACHE *const head = &print_event_info->head_cache;
  char *str = get_str(nullptr, &Gtid_set::commented_string_format);
  if (str != nullptr) {
    if (!print_event_info->short_form) {
      print_header(head, print_event_info, false);
      my_b_printf(head, "\tPrevious-GTIDs\n");
    }
    my_b_printf(head, "%s\n", str);
    my_free(str);
  }
}
#endif

int Previous_gtids_log_event::add_to_set(Gtid_set *target) const {
  DBUG_TRACE;
  size_t end_pos = 0;
  size_t add_size = DBUG_EVALUATE_IF("gtid_has_extra_data", 10, 0);
  /* Silently ignore additional unknown data at the end of the encoding */
  PROPAGATE_REPORTED_ERROR_INT(
      target->add_gtid_encoding(buf, buf_size + add_size, &end_pos));
  DBUG_ASSERT(end_pos <= buf_size);
  return 0;
}

char *Previous_gtids_log_event::get_str(
    size_t *length_p, const Gtid_set::String_format *string_format) const {
  DBUG_TRACE;
  Sid_map sid_map(nullptr);
  Gtid_set set(&sid_map, nullptr);
  DBUG_PRINT("info", ("temp_buf=%p buf=%p", temp_buf, buf));
  if (set.add_gtid_encoding(buf, buf_size) != RETURN_STATUS_OK) return nullptr;
  set.dbug_print("set");
  size_t length = set.get_string_length(string_format);
  DBUG_PRINT("info", ("string length= %lu", (ulong)length));
  char *str = (char *)my_malloc(key_memory_log_event, length + 1, MYF(MY_WME));
  if (str != nullptr) {
    set.to_string(str, false /*need_lock*/, string_format);
    if (length_p != nullptr) *length_p = length;
  }
  return str;
}

#ifdef MYSQL_SERVER
bool Previous_gtids_log_event::write_data_body(Basic_ostream *ostream) {
  DBUG_TRACE;
  DBUG_PRINT("info", ("size=%d", static_cast<int>(buf_size)));
  bool ret = wrapper_my_b_safe_write(ostream, buf, buf_size);
  return ret;
}

int Previous_gtids_log_event::do_update_pos(Relay_log_info *rli) {
  rli->inc_event_relay_log_pos();
  return 0;
}
#endif

#ifdef MYSQL_SERVER
Metadata_log_event::Metadata_log_event(THD *thd_arg, bool using_trans)
    : binary_log::Metadata_event(),
      Log_event(thd_arg, 0,
                using_trans ? Log_event::EVENT_TRANSACTIONAL_CACHE
                            : Log_event::EVENT_STMT_CACHE,
                Log_event::EVENT_NORMAL_LOGGING, header(), footer()) {}

Metadata_log_event::Metadata_log_event(THD *thd_arg, bool using_trans,
                                       uint64_t hlc_time_ns)
    : Metadata_log_event(thd_arg, using_trans) {
  set_hlc_time(hlc_time_ns);
}

Metadata_log_event::Metadata_log_event()
    : binary_log::Metadata_event(),
      Log_event(header(), footer(), Log_event::EVENT_NO_CACHE,
                Log_event::EVENT_IMMEDIATE_LOGGING) {}

Metadata_log_event::Metadata_log_event(const std::string &raft_str)
    : Log_event(header(), footer(), Log_event::EVENT_NO_CACHE,
                Log_event::EVENT_IMMEDIATE_LOGGING) {
  set_raft_str(raft_str);
}

Metadata_log_event::Metadata_log_event(uint64_t prev_hlc_time_ns)
    : binary_log::Metadata_event(),
      Log_event(header(), footer(), Log_event::EVENT_NO_CACHE,
                Log_event::EVENT_IMMEDIATE_LOGGING) {
  set_prev_hlc_time(prev_hlc_time_ns);
}
#endif

Metadata_log_event::Metadata_log_event(
    char const *buffer,
    binary_log::Format_description_event const *description_event)
    : binary_log::Metadata_event(buffer, description_event),
      Log_event(header(), footer()) {}

#ifdef MYSQL_SERVER
bool Metadata_log_event::write_data_body(Basic_ostream *ostream) {
  DBUG_ENTER("Metadata_log_event::write_data_body");

  // Cannot contain both hlc and prev-hlc timestamp in the same metadata event
  DBUG_ASSERT(!(does_exist(Metadata_event_types::HLC_TYPE) &&
                does_exist(Metadata_event_types::PREV_HLC_TYPE)));

  if (write_hlc_time(ostream)) DBUG_RETURN(1);

  if (write_prev_hlc_time(ostream)) DBUG_RETURN(1);

  if (write_raft_term_and_index(ostream)) DBUG_RETURN(1);

  if (write_raft_str(ostream)) DBUG_RETURN(1);

  if (write_raft_prev_opid(ostream)) DBUG_RETURN(1);

  if (write_rotate_tag(ostream)) DBUG_RETURN(1);

  DBUG_RETURN(0);
}

bool Metadata_log_event::write_hlc_time(Basic_ostream *ostream) {
  DBUG_ENTER("Metadata_log_event::write_hlc_time");

  if (!does_exist(Metadata_event_types::HLC_TYPE))
    DBUG_RETURN(0); /* No need to write HLC time */

  if (write_type_and_length(ostream, Metadata_event_types::HLC_TYPE,
                            sizeof(hlc_time_ns_))) {
    DBUG_RETURN(1);
  }

  char buffer[ENCODED_HLC_SIZE];
  char *ptr_buffer = buffer;

  int8store(ptr_buffer, hlc_time_ns_);
  ptr_buffer += ENCODED_HLC_SIZE;

  DBUG_ASSERT(ptr_buffer == (buffer + sizeof(buffer)));

  bool ret = wrapper_my_b_safe_write(ostream, (uchar *)buffer, sizeof(buffer));
  DBUG_RETURN(ret);
}

bool Metadata_log_event::write_prev_hlc_time(Basic_ostream *ostream) {
  DBUG_ENTER("Metadata_log_event::write_prev_hlc_time");

  if (!does_exist(Metadata_event_types::PREV_HLC_TYPE))
    DBUG_RETURN(0); /* No need to write prev hlc time */

  char buffer[ENCODED_PREV_HLC_SIZE];
  char *ptr_buffer = buffer;

  if (write_type_and_length(ostream, Metadata_event_types::PREV_HLC_TYPE,
                            sizeof(prev_hlc_time_ns_))) {
    DBUG_RETURN(1);
  }

  int8store(ptr_buffer, prev_hlc_time_ns_);
  ptr_buffer += ENCODED_PREV_HLC_SIZE;

  DBUG_ASSERT(ptr_buffer == (buffer + sizeof(buffer)));

  bool ret = wrapper_my_b_safe_write(ostream, (uchar *)buffer, sizeof(buffer));
  DBUG_RETURN(ret);
}

bool Metadata_log_event::write_raft_term_and_index(Basic_ostream *ostream) {
  DBUG_ENTER("Metadata_log_event::write_term_and_index");

  if (!does_exist(Metadata_event_types::RAFT_TERM_INDEX_TYPE)) {
    DBUG_RETURN(0); /* No need to write term and index */
  }

  char buffer[ENCODED_RAFT_TERM_INDEX_SIZE];
  char *ptr_buffer = buffer;

  if (write_type_and_length(ostream, Metadata_event_types::RAFT_TERM_INDEX_TYPE,
                            sizeof(raft_term_) + sizeof(raft_index_))) {
    DBUG_RETURN(1);
  }

  int8store(ptr_buffer, raft_term_);
  ptr_buffer += sizeof(raft_term_);

  int8store(ptr_buffer, raft_index_);
  ptr_buffer += sizeof(raft_index_);

  DBUG_ASSERT(ptr_buffer == (buffer + sizeof(buffer)));

  bool ret = wrapper_my_b_safe_write(ostream, (uchar *)buffer, sizeof(buffer));
  DBUG_RETURN(ret);
}

bool Metadata_log_event::write_raft_str(Basic_ostream *ostream) {
  DBUG_ENTER("Metadata_log_event::write_raft_str");

  if (!does_exist(Metadata_event_types::RAFT_GENERIC_STR_TYPE))
    DBUG_RETURN(0); /* No need to write raft string */

  if (write_type_and_length(ostream,
                            Metadata_event_types::RAFT_GENERIC_STR_TYPE,
                            raft_str_.size())) {
    DBUG_RETURN(1);
  }

  bool ret = wrapper_my_b_safe_write(ostream, (const uchar *)raft_str_.c_str(),
                                     raft_str_.size());

  DBUG_RETURN(ret);
}

bool Metadata_log_event::write_raft_prev_opid(Basic_ostream *ostream) {
  DBUG_ENTER("Metadata_log_event::write_raft_prev_opid");

  if (!does_exist(Metadata_event_types::RAFT_PREV_OPID_TYPE))
    DBUG_RETURN(0); /* No need to write term and index */

  char buffer[ENCODED_RAFT_PREV_OPID_SIZE];
  char *ptr_buffer = buffer;

  if (write_type_and_length(ostream, Metadata_event_types::RAFT_PREV_OPID_TYPE,
                            ENCODED_RAFT_PREV_OPID_SIZE)) {
    DBUG_RETURN(1);
  }

  int8store(ptr_buffer, prev_raft_term_);
  ptr_buffer += sizeof(prev_raft_term_);

  int8store(ptr_buffer, prev_raft_index_);
  ptr_buffer += sizeof(prev_raft_index_);

  DBUG_ASSERT(ptr_buffer == (buffer + sizeof(buffer)));
  bool ret = wrapper_my_b_safe_write(ostream, (uchar *)buffer, sizeof(buffer));
  DBUG_RETURN(ret);
}

bool Metadata_log_event::write_rotate_tag(Basic_ostream *ostream) {
  DBUG_ENTER("Metadata_log_event::write_rotate_tag");
  if (!does_exist(Metadata_event_types::RAFT_ROTATE_TAG_TYPE))
    DBUG_RETURN(0); /* No need to write term and index */

  char buffer[ENCODED_RAFT_ROTATE_TAG_SIZE];
  char *ptr_buffer = buffer;

  if (write_type_and_length(ostream, Metadata_event_types::RAFT_ROTATE_TAG_TYPE,
                            ENCODED_RAFT_ROTATE_TAG_SIZE)) {
    DBUG_RETURN(1);
  }

  int2store(ptr_buffer, (uint16_t)raft_rotate_tag_);
  ptr_buffer += sizeof(uint16_t);

  DBUG_ASSERT(ptr_buffer == (buffer + sizeof(buffer)));

  bool ret = wrapper_my_b_safe_write(ostream, (uchar *)buffer, sizeof(buffer));
  DBUG_RETURN(ret);
}

bool Metadata_log_event::write_type_and_length(Basic_ostream *ostream,
                                               Metadata_event_types type,
                                               uint16_t length) {
  DBUG_ENTER("Metadata_log_event::write_type_and_length");

  char buffer[ENCODED_TYPE_SIZE + ENCODED_LENGTH_SIZE];
  char *ptr_buffer = buffer;

  *(unsigned char *)ptr_buffer = static_cast<unsigned char>(type);
  ptr_buffer += ENCODED_TYPE_SIZE;

  int2store(ptr_buffer, length);
  ptr_buffer += ENCODED_LENGTH_SIZE;

  DBUG_ASSERT(ptr_buffer == (buffer + sizeof(buffer)));

  bool ret = wrapper_my_b_safe_write(ostream, (uchar *)buffer, sizeof(buffer));
  DBUG_RETURN(ret);
}

uint32 Metadata_log_event::write_data_body(uchar *obuffer) {
  DBUG_ENTER("Metadata_log_event::write_data_body");

  // Cannot contain both hlc and prev-hlc timestamp in the same metadata event
  DBUG_ASSERT(!(does_exist(Metadata_event_types::HLC_TYPE) &&
                does_exist(Metadata_event_types::PREV_HLC_TYPE)));

  uint32 length = 0;

  length += write_hlc_time(obuffer + length);
  length += write_prev_hlc_time(obuffer + length);
  length += write_raft_term_and_index(obuffer + length);
  length += write_raft_str(obuffer + length);
  length += write_rotate_tag(obuffer + length);
  DBUG_RETURN(length);
}

uint32 Metadata_log_event::write_hlc_time(uchar *obuffer) {
  DBUG_ENTER("Metadata_log_event::write_hlc_time");

  if (!does_exist(Metadata_event_types::HLC_TYPE))
    DBUG_RETURN(0); /* No need to write HLC time */

  uint32 length = 0;

  length += write_type_and_length(
      obuffer + length, Metadata_event_types::HLC_TYPE, sizeof(hlc_time_ns_));

  int8store(obuffer + length, hlc_time_ns_);
  length += sizeof(hlc_time_ns_);

  DBUG_RETURN(length);
}

uint32 Metadata_log_event::write_prev_hlc_time(uchar *obuffer) {
  DBUG_ENTER("Metadata_log_event::write_prev_hlc_time");

  if (!does_exist(Metadata_event_types::PREV_HLC_TYPE))
    DBUG_RETURN(0); /* No need to write prev hlc time */

  uint32 length = 0;

  length += write_type_and_length(obuffer + length,
                                  Metadata_event_types::PREV_HLC_TYPE,
                                  sizeof(prev_hlc_time_ns_));

  int8store(obuffer + length, prev_hlc_time_ns_);
  length += sizeof(prev_hlc_time_ns_);

  DBUG_RETURN(length);
}

uint32 Metadata_log_event::write_raft_term_and_index(uchar *obuffer) {
  DBUG_ENTER("Metadata_log_event::write_term_and_index");

  if (!does_exist(Metadata_event_types::RAFT_TERM_INDEX_TYPE)) {
    DBUG_RETURN(0); /* No need to write term and index */
  }

  uint32 length = 0;

  length += write_type_and_length(obuffer + length,
                                  Metadata_event_types::RAFT_TERM_INDEX_TYPE,
                                  sizeof(raft_term_) + sizeof(raft_index_));

  int8store(obuffer + length, raft_term_);
  length += sizeof(raft_term_);

  int8store(obuffer + length, raft_index_);
  length += sizeof(raft_index_);

  DBUG_RETURN(length);
}

uint32 Metadata_log_event::write_raft_str(uchar *obuffer) {
  DBUG_ENTER("Metadata_log_event::write_raft_str");

  if (!does_exist(Metadata_event_types::RAFT_GENERIC_STR_TYPE)) {
    DBUG_RETURN(0); /* No need to write raft string */
  }

  uint32 length = 0;

  length += write_type_and_length(obuffer + length,
                                  Metadata_event_types::RAFT_GENERIC_STR_TYPE,
                                  raft_str_.size());

  memcpy(obuffer + length, raft_str_.c_str(), raft_str_.size());
  length += raft_str_.length();

  DBUG_RETURN(length);
}

uint32 Metadata_log_event::write_rotate_tag(uchar *obuffer) {
  DBUG_ENTER("Metadata_log_event::write_rotate_tag");
  if (!does_exist(Metadata_event_types::RAFT_ROTATE_TAG_TYPE)) {
    DBUG_RETURN(0); /* No need to write raft string */
  }

  uint32 length =
      write_type_and_length(obuffer, Metadata_event_types::RAFT_ROTATE_TAG_TYPE,
                            ENCODED_RAFT_ROTATE_TAG_SIZE);

  int2store(obuffer + length, (uint16_t)raft_rotate_tag_);
  length += sizeof(uint16_t);

  DBUG_RETURN(length);
}

uint32 Metadata_log_event::write_type_and_length(uchar *obuffer,
                                                 Metadata_event_types type,
                                                 uint16_t length) {
  DBUG_ENTER("Metadata_log_event::write_type_and_length");

  uint32 len = 0;
  *obuffer = static_cast<uchar>(type);
  len += ENCODED_TYPE_SIZE;

  int2store(obuffer + len, length);
  len += ENCODED_LENGTH_SIZE;

  DBUG_RETURN(len);
}

#endif  // MYSQL_SERVER

// The size of the data section of the event
size_t Metadata_log_event::get_data_size() {
  /* data header + data body size for this event when serialized into the
     stream */
  return size_;
}

// Total size of the event including common header
size_t Metadata_log_event::get_total_size() {
  /* common header + data header + data body */
  return LOG_EVENT_HEADER_LEN + get_data_size();
}

#ifdef MYSQL_SERVER
int Metadata_log_event::pack_info(Protocol *protocol) {
  std::string buffer;
  bool field_added = false;

  if (does_exist(Metadata_event_types::HLC_TYPE)) {
    buffer.append("HLC time: ");
    buffer.append(std::to_string(hlc_time_ns_));
    field_added = true;
  }

  if (does_exist(Metadata_event_types::PREV_HLC_TYPE)) {
    if (field_added) buffer.append(" ");
    buffer.append("Prev HLC time: ");
    buffer.append(std::to_string(prev_hlc_time_ns_));
    field_added = true;
  }

  if (does_exist(Metadata_event_types::RAFT_TERM_INDEX_TYPE)) {
    if (field_added) buffer.append(" ");
    buffer.append("Raft term: " + std::to_string(raft_term_) +
                  " Raft Index: " + std::to_string(raft_index_));
    field_added = true;
  }

  if (does_exist(Metadata_event_types::RAFT_PREV_OPID_TYPE)) {
    if (field_added) buffer.append(" ");
    buffer.append("Raft Prev OPID term: " + std::to_string(prev_raft_term_) +
                  " Raft Prev OPID Index: " + std::to_string(prev_raft_index_));
    field_added = true;
  }

  if (does_exist(Metadata_event_types::RAFT_ROTATE_TAG_TYPE)) {
    if (field_added) buffer.append(" ");
    buffer.append("Rotate Event Tag: " + get_rotate_tag_string());
    field_added = true;
  }

  if (buffer.length() > 0)
    protocol->store_string(buffer.c_str(), buffer.length(), &my_charset_bin);

  return 0;
}
#endif

#ifndef MYSQL_SERVER
void Metadata_log_event::print(FILE * /* file */,
                               PRINT_EVENT_INFO *print_event_info) const {
  IO_CACHE *const head = &print_event_info->head_cache;
  IO_CACHE *const body = &print_event_info->body_cache;

  if (!print_event_info->short_form) {
    std::string buffer;
    buffer.append("\tMetadata");

    if (does_exist(Metadata_event_types::HLC_TYPE))
      buffer.append("\tHLC time: " + std::to_string(hlc_time_ns_));

    if (does_exist(Metadata_event_types::PREV_HLC_TYPE))
      buffer.append("\tPrev HLC time: " + std::to_string(prev_hlc_time_ns_));

    if (does_exist(Metadata_event_types::RAFT_TERM_INDEX_TYPE))
      buffer.append("\tRaft term: " + std::to_string(raft_term_) +
                    ", Raft Index: " + std::to_string(raft_index_));

    if (does_exist(Metadata_event_types::RAFT_GENERIC_STR_TYPE))
      buffer.append("\t Raft string: '" + raft_str_ + "'");

    if (does_exist(Metadata_event_types::RAFT_PREV_OPID_TYPE))
      buffer.append(
          "\tRaft Prev OPID term: " + std::to_string(prev_raft_term_) +
          ", Raft Prev OPID Index: " + std::to_string(prev_raft_index_));

    if (does_exist(Metadata_event_types::RAFT_ROTATE_TAG_TYPE))
      buffer.append("\tRotate Event Tag: " + get_rotate_tag_string());
    print_header(head, print_event_info, false);
    my_b_printf(head, "%s\n", buffer.c_str());
    print_base64(body, print_event_info, false);
  }

  return;
}
#endif

#if defined(MYSQL_SERVER)
int Metadata_log_event::do_apply_event(Relay_log_info const *rli) {
  DBUG_ENTER("Metadata_log_event::do_apply_event");
  DBUG_ASSERT(rli->info_thd == thd);
  int error = 0;

  if (does_exist(Metadata_event_types::HLC_TYPE)) {
    // Stash the HLC commit timestamp of this transaction in master
    rli->info_thd->hlc_time_ns_next = hlc_time_ns_;
  }

  if (does_exist(Metadata_event_types::RAFT_TERM_INDEX_TYPE)) {
    // Stash the raft term and index in THD
    thd->set_trans_marker(raft_term_, raft_index_);
  }

  DBUG_RETURN(error);
}

int Metadata_log_event::do_update_pos(Relay_log_info *rli) {
  rli->inc_event_relay_log_pos();
  return 0;
}

Log_event::enum_skip_reason Metadata_log_event::do_shall_skip(
    Relay_log_info *rli) {
  /*
   * Metadata event containing previous hlc timestamp has no meaning for slave.
   * hence slave should skip such events
   */
  if (does_exist(Metadata_event_types::PREV_HLC_TYPE))
    return Log_event::EVENT_SKIP_IGNORE;

  if (does_exist(Metadata_event_types::RAFT_PREV_OPID_TYPE))
    return Log_event::EVENT_SKIP_IGNORE;

  /*
   * A metadata event not in the context of a transaction
   * can be skipped as it is for a rotate/no-op event. Do this only if MTS is
   * enabled (since single threaded slave does not care about free floating
   * metadata event and curr_group_seen_gtid is set only for MTS)
   */
  if (enable_raft_plugin &&
      ((rli->slave_parallel_workers > 0) && (!rli->curr_group_seen_gtid))) {
    if (does_exist(Metadata_event_types::RAFT_TERM_INDEX_TYPE)) {
      // Stash the raft term and index in THD
      thd->set_trans_marker(raft_term_, raft_index_);
    }
    return Log_event::EVENT_SKIP_IGNORE;
  }

  return Log_event::EVENT_SKIP_NOT;
}
#endif

/**************************************************************************
        Transaction_context_log_event methods
**************************************************************************/

#ifdef MYSQL_SERVER
Transaction_context_log_event::Transaction_context_log_event(
    const char *server_uuid_arg, bool using_trans, my_thread_id thread_id_arg,
    bool is_gtid_specified_arg)
    : binary_log::Transaction_context_event(thread_id_arg,
                                            is_gtid_specified_arg),
      Log_event(header(), footer(),
                using_trans ? Log_event::EVENT_TRANSACTIONAL_CACHE
                            : Log_event::EVENT_STMT_CACHE,
                Log_event::EVENT_NORMAL_LOGGING) {
  DBUG_TRACE;
  common_header->flags |= LOG_EVENT_IGNORABLE_F;
  server_uuid = nullptr;
  sid_map = new Sid_map(nullptr);
  snapshot_version = new Gtid_set(sid_map);

  /*
    Copy global_sid_map to a local copy to avoid the acquisition
    of the global_sid_lock for operations on top of this snapshot
    version.
    The Sid_map and Gtid_executed must be read under the protection
    of MYSQL_BIN_LOG.LOCK_commit to avoid race conditions between
    ordered commits in the storage engine and gtid_state update.
  */
  if (mysql_bin_log.get_gtid_executed(sid_map, snapshot_version)) goto err;

  server_uuid = my_strdup(key_memory_log_event, server_uuid_arg, MYF(MY_WME));
  if (server_uuid == nullptr) goto err;

  // These two fields are only populated on event decoding.
  // Encoding is done directly from snapshot_version field.
  encoded_snapshot_version = nullptr;
  encoded_snapshot_version_length = 0;

  // Debug sync point for SQL threads.
  DBUG_EXECUTE_IF(
      "debug.wait_after_set_snapshot_version_on_transaction_context_log_event",
      {
        const char act[] =
            "now wait_for "
            "signal.resume_after_set_snapshot_version_on_transaction_context_"
            "log_event";
        DBUG_ASSERT(opt_debug_sync_timeout > 0);
        DBUG_ASSERT(!debug_sync_set_action(current_thd, STRING_WITH_LEN(act)));
      };);

  common_header->set_is_valid(true);
  return;

err:
  common_header->set_is_valid(false);
}
#endif  // MYSQL_SERVER

Transaction_context_log_event::Transaction_context_log_event(
    const char *buffer, const Format_description_event *descr_event)
    : binary_log::Transaction_context_event(buffer, descr_event),
      Log_event(header(), footer()),
      sid_map(nullptr),
      snapshot_version(nullptr) {
  DBUG_TRACE;
  if (!is_valid()) return;

  common_header->flags |= LOG_EVENT_IGNORABLE_F;

  sid_map = new Sid_map(nullptr);
  snapshot_version = new Gtid_set(sid_map);
}

Transaction_context_log_event::~Transaction_context_log_event() {
  DBUG_TRACE;
  if (server_uuid) my_free(const_cast<char *>(server_uuid));
  server_uuid = nullptr;
  if (encoded_snapshot_version)
    my_free(const_cast<uchar *>(encoded_snapshot_version));
  encoded_snapshot_version = nullptr;
  delete snapshot_version;
  delete sid_map;
}

size_t Transaction_context_log_event::to_string(char *buf, ulong len) const {
  DBUG_TRACE;
  return snprintf(buf, len, "server_uuid=%s\tthread_id=%u", server_uuid,
                  thread_id);
}

#ifdef MYSQL_SERVER
int Transaction_context_log_event::pack_info(Protocol *protocol) {
  DBUG_TRACE;
  char buf[256];
  size_t bytes = to_string(buf, 256);
  protocol->store_string(buf, bytes, &my_charset_bin);
  return 0;
}
#endif

#ifndef MYSQL_SERVER
void Transaction_context_log_event::print(
    FILE *, PRINT_EVENT_INFO *print_event_info) const {
  DBUG_TRACE;
  char buf[256];
  IO_CACHE *const head = &print_event_info->head_cache;

  if (!print_event_info->short_form) {
    to_string(buf, 256);
    print_header(head, print_event_info, false);
    my_b_printf(head, "Transaction_context: %s\n", buf);
  }
}
#endif

#if defined(MYSQL_SERVER)
int Transaction_context_log_event::do_update_pos(Relay_log_info *rli) {
  DBUG_TRACE;
  rli->inc_event_relay_log_pos();
  return 0;
}
#endif

size_t Transaction_context_log_event::get_data_size() {
  DBUG_TRACE;

  size_t size = Binary_log_event::TRANSACTION_CONTEXT_HEADER_LEN;
  size += strlen(server_uuid);
  size += get_snapshot_version_size();
  size += get_data_set_size(&write_set);
  size += get_data_set_size(&read_set);

  return size;
}

#ifdef MYSQL_SERVER
bool Transaction_context_log_event::write_data_header(Basic_ostream *ostream) {
  DBUG_TRACE;
  char buf[Binary_log_event::TRANSACTION_CONTEXT_HEADER_LEN];

  buf[ENCODED_SERVER_UUID_LEN_OFFSET] = (char)strlen(server_uuid);
  int4store(buf + ENCODED_THREAD_ID_OFFSET, thread_id);
  buf[ENCODED_GTID_SPECIFIED_OFFSET] = gtid_specified;
  int4store(buf + ENCODED_SNAPSHOT_VERSION_LEN_OFFSET,
            get_snapshot_version_size());
  int4store(buf + ENCODED_WRITE_SET_ITEMS_OFFSET, write_set.size());
  int4store(buf + ENCODED_READ_SET_ITEMS_OFFSET, read_set.size());
  return wrapper_my_b_safe_write(
      ostream, (const uchar *)buf,
      Binary_log_event::TRANSACTION_CONTEXT_HEADER_LEN);
}

bool Transaction_context_log_event::write_data_body(Basic_ostream *ostream) {
  DBUG_TRACE;

  if (wrapper_my_b_safe_write(ostream, (const uchar *)server_uuid,
                              strlen(server_uuid)) ||
      write_snapshot_version(ostream) || write_data_set(ostream, &write_set) ||
      write_data_set(ostream, &read_set))
    return true;

  return false;
}

bool Transaction_context_log_event::write_snapshot_version(
    Basic_ostream *ostream) {
  DBUG_TRACE;
  bool result = false;

  uint32 len = get_snapshot_version_size();
  uchar *buffer = (uchar *)my_malloc(key_memory_log_event, len, MYF(MY_WME));
  if (buffer == nullptr) return true;

  snapshot_version->encode(buffer);
  if (wrapper_my_b_safe_write(ostream, buffer, len)) result = true;

  my_free(buffer);
  return result;
}

bool Transaction_context_log_event::write_data_set(
    Basic_ostream *ostream, std::list<const char *> *set) {
  DBUG_TRACE;
  for (std::list<const char *>::iterator it = set->begin(); it != set->end();
       ++it) {
    char buf[ENCODED_READ_WRITE_SET_ITEM_LEN];
    const char *hash = *it;
    uint16 len = strlen(hash);

    int2store(buf, len);
    if (wrapper_my_b_safe_write(ostream, (const uchar *)buf,
                                ENCODED_READ_WRITE_SET_ITEM_LEN) ||
        wrapper_my_b_safe_write(ostream, (const uchar *)hash, len))
      return true;
  }

  return false;
}
#endif

bool Transaction_context_log_event::read_snapshot_version() {
  DBUG_TRACE;
  DBUG_ASSERT(snapshot_version->is_empty());

  global_sid_lock->wrlock();
  enum_return_status return_status = global_sid_map->copy(sid_map);
  global_sid_lock->unlock();
  if (return_status != RETURN_STATUS_OK) return true;

  return snapshot_version->add_gtid_encoding(encoded_snapshot_version,
                                             encoded_snapshot_version_length) !=
         RETURN_STATUS_OK;
}

size_t Transaction_context_log_event::get_snapshot_version_size() {
  DBUG_TRACE;
  size_t result = snapshot_version->get_encoded_length();
  return result;
}

int Transaction_context_log_event::get_data_set_size(
    std::list<const char *> *set) {
  DBUG_TRACE;
  int size = 0;

  for (std::list<const char *>::iterator it = set->begin(); it != set->end();
       ++it)
    size += ENCODED_READ_WRITE_SET_ITEM_LEN + strlen(*it);

  return size;
}

void Transaction_context_log_event::add_write_set(const char *hash) {
  DBUG_TRACE;
  write_set.push_back(hash);
}

void Transaction_context_log_event::add_read_set(const char *hash) {
  DBUG_TRACE;
  read_set.push_back(hash);
}

/**************************************************************************
        View_change_log_event methods
**************************************************************************/

#ifdef MYSQL_SERVER
View_change_log_event::View_change_log_event(const char *raw_view_id)
    : binary_log::View_change_event(raw_view_id),
      Log_event(header(), footer(), Log_event::EVENT_TRANSACTIONAL_CACHE,
                Log_event::EVENT_NORMAL_LOGGING) {
  DBUG_TRACE;
  common_header->flags |= LOG_EVENT_IGNORABLE_F;

  common_header->set_is_valid(strlen(view_id) != 0);
}
#endif

View_change_log_event::View_change_log_event(
    const char *buffer, const Format_description_event *descr_event)
    : binary_log::View_change_event(buffer, descr_event),
      Log_event(header(), footer()) {
  DBUG_TRACE;
  if (!is_valid()) return;
  common_header->flags |= LOG_EVENT_IGNORABLE_F;

  // Change the cache/logging types to allow writing to the binary log cache
  event_cache_type = EVENT_TRANSACTIONAL_CACHE;
  event_logging_type = EVENT_NORMAL_LOGGING;
}

View_change_log_event::~View_change_log_event() {
  DBUG_TRACE;
  certification_info.clear();
}

size_t View_change_log_event::get_data_size() {
  DBUG_TRACE;

  size_t size = Binary_log_event::VIEW_CHANGE_HEADER_LEN;
  size += get_size_data_map(&certification_info);

  return size;
}

size_t View_change_log_event::get_size_data_map(
    std::map<std::string, std::string> *map) {
  DBUG_TRACE;
  size_t size = 0;

  std::map<std::string, std::string>::iterator iter;
  size += (ENCODED_CERT_INFO_KEY_SIZE_LEN + ENCODED_CERT_INFO_VALUE_LEN) *
          map->size();
  for (iter = map->begin(); iter != map->end(); iter++)
    size += iter->first.length() + iter->second.length();

  return size;
}

size_t View_change_log_event::to_string(char *buf, ulong len) const {
  DBUG_TRACE;
  return snprintf(buf, len, "view_id=%s", view_id);
}

#ifdef MYSQL_SERVER
int View_change_log_event::pack_info(Protocol *protocol) {
  DBUG_TRACE;
  char buf[256];
  size_t bytes = to_string(buf, 256);
  protocol->store_string(buf, bytes, &my_charset_bin);
  return 0;
}
#endif

#ifndef MYSQL_SERVER
void View_change_log_event::print(FILE *,
                                  PRINT_EVENT_INFO *print_event_info) const {
  DBUG_TRACE;
  char buf[256];
  IO_CACHE *const head = &print_event_info->head_cache;

  if (!print_event_info->short_form) {
    to_string(buf, 256);
    print_header(head, print_event_info, false);
    my_b_printf(head, "View_change_log_event: %s\n", buf);
  }
}
#endif

#if defined(MYSQL_SERVER)

int View_change_log_event::do_apply_event(Relay_log_info const *rli) {
  enum_gtid_statement_status state = gtid_pre_statement_checks(thd);
  if (state == GTID_STATEMENT_SKIP) return 0;

  if (state == GTID_STATEMENT_CANCEL ||
      (state == GTID_STATEMENT_EXECUTE &&
       gtid_pre_statement_post_implicit_commit_checks(thd))) {
    uint error = thd->get_stmt_da()->mysql_errno();
    DBUG_ASSERT(error != 0);
    rli->report(ERROR_LEVEL, error, "Error executing View Change event: '%s'",
                thd->get_stmt_da()->message_text());
    thd->is_slave_error = true;
    return -1;
  }

  if (!opt_bin_log) {
    return 0;
  }

  int error = mysql_bin_log.write_event(this);
  if (error)
    rli->report(ERROR_LEVEL, ER_SLAVE_FATAL_ERROR,
                ER_THD(thd, ER_SLAVE_FATAL_ERROR),
                "Could not write the VIEW CHANGE event in the binary log.");

  return (error);
}

int View_change_log_event::do_update_pos(Relay_log_info *rli) {
  DBUG_TRACE;
  rli->inc_event_relay_log_pos();
  return 0;
}

bool View_change_log_event::write_data_header(Basic_ostream *ostream) {
  DBUG_TRACE;
  char buf[Binary_log_event::VIEW_CHANGE_HEADER_LEN];

  memcpy(buf, view_id, ENCODED_VIEW_ID_MAX_LEN);
  int8store(buf + ENCODED_SEQ_NUMBER_OFFSET, seq_number);
  int4store(buf + ENCODED_CERT_INFO_SIZE_OFFSET, certification_info.size());
  return wrapper_my_b_safe_write(ostream, (const uchar *)buf,
                                 Binary_log_event::VIEW_CHANGE_HEADER_LEN);
}

bool View_change_log_event::write_data_body(Basic_ostream *ostream) {
  DBUG_TRACE;

  if (write_data_map(ostream, &certification_info)) return true;

  return false;
}

bool View_change_log_event::write_data_map(
    Basic_ostream *ostream, std::map<std::string, std::string> *map) {
  DBUG_TRACE;
  bool result = false;

  std::map<std::string, std::string>::iterator iter;
  for (iter = map->begin(); iter != map->end(); iter++) {
    uchar buf_key_len[ENCODED_CERT_INFO_KEY_SIZE_LEN];
    uint16 key_len = iter->first.length();
    int2store(buf_key_len, key_len);

    const char *key = iter->first.c_str();

    uchar buf_value_len[ENCODED_CERT_INFO_VALUE_LEN];
    uint32 value_len = iter->second.length();
    int4store(buf_value_len, value_len);

    const char *value = iter->second.c_str();

    if (wrapper_my_b_safe_write(ostream, buf_key_len,
                                ENCODED_CERT_INFO_KEY_SIZE_LEN) ||
        wrapper_my_b_safe_write(ostream, (const uchar *)key, key_len) ||
        wrapper_my_b_safe_write(ostream, buf_value_len,
                                ENCODED_CERT_INFO_VALUE_LEN) ||
        wrapper_my_b_safe_write(ostream, (const uchar *)value, value_len))
      return result;
  }

  return false;
}

#endif  // MYSQL_SERVER

/*
  Updates the certification info map.
*/
void View_change_log_event::set_certification_info(
    std::map<std::string, std::string> *info, size_t *event_size) {
  DBUG_TRACE;
  certification_info.clear();

  *event_size = Binary_log_event::VIEW_CHANGE_HEADER_LEN;
  std::map<std::string, std::string>::iterator it;
  for (it = info->begin(); it != info->end(); ++it) {
    std::string key = it->first;
    std::string value = it->second;
    certification_info[key] = value;
    *event_size += it->first.length() + it->second.length();
  }
  *event_size +=
      (ENCODED_CERT_INFO_KEY_SIZE_LEN + ENCODED_CERT_INFO_VALUE_LEN) *
      certification_info.size();
}

size_t Transaction_payload_log_event::get_data_size() {
  /* purecov: begin inspected */
  DBUG_TRACE;
  DBUG_ASSERT(false);
  return 0;
  /* purecov: end */
}

#ifdef MYSQL_SERVER
uint8 Transaction_payload_log_event::get_mts_dbs(
    Mts_db_names *arg, Rpl_filter *rpl_filter MY_ATTRIBUTE((unused))) {
  Mts_db_names &mts_dbs = m_applier_ctx.get_mts_db_names();
  if (mts_dbs.num == OVER_MAX_DBS_IN_EVENT_MTS) {
    arg->name[0] = 0;
    arg->num = OVER_MAX_DBS_IN_EVENT_MTS;
  } else {
    for (int i = 0; i < mts_dbs.num; i++) arg->name[i] = mts_dbs.name[i];
    arg->num = mts_dbs.num;
  }

  return arg->num;
}

void Transaction_payload_log_event::set_mts_dbs(Mts_db_names &arg) {
  m_applier_ctx.reset();
  Mts_db_names &mts_dbs = m_applier_ctx.get_mts_db_names();
  mts_dbs.num = arg.num;
  if (mts_dbs.num < MAX_DBS_IN_EVENT_MTS) {
    for (int i = 0; i < arg.num; i++)
      // strndup already adds the string terminator
      mts_dbs.name[i] = strndup(arg.name[i], NAME_LEN);
  }
#ifndef DBUG_OFF
  else
    DBUG_ASSERT(mts_dbs.num == OVER_MAX_DBS_IN_EVENT_MTS);
#endif
}

uint8 Transaction_payload_log_event::mts_number_dbs() {
  return m_applier_ctx.get_mts_db_names().num;
}

int Transaction_payload_log_event::do_apply_event(Relay_log_info const *rli) {
  DBUG_TRACE;
  int res = 0;
  PSI_stage_info old_stage;

  /* apply events in the payload */

  binary_log::transaction::compression::Iterable_buffer it(
      m_payload, m_payload_size, m_uncompressed_size, m_compression_type);

  thd->enter_stage(&stage_binlog_transaction_decompress, &old_stage, __func__,
                   __FILE__, __LINE__);
  for (auto ptr : it) {
    THD_STAGE_INFO(thd, old_stage);
    if ((res = apply_payload_event(rli, (const uchar *)ptr))) break;
    thd->enter_stage(&stage_binlog_transaction_decompress, &old_stage, __func__,
                     __FILE__, __LINE__);
  }
  THD_STAGE_INFO(thd, old_stage);

  return res;
}

static bool shall_delete_event_after_apply(Log_event *ev) {
  bool res = false;
  if (ev == nullptr) return res;
  switch (ev->get_type_code()) {
    case binary_log::FORMAT_DESCRIPTION_EVENT:
      /*
        Format_description_log_event should not be deleted because it will
        be used to read info about the relay log's format; it will be
        deleted when the SQL thread does not need it, i.e. when this
        thread terminates.
      */

      /* fall through */
    case binary_log::ROWS_QUERY_LOG_EVENT:
      /*
         ROWS_QUERY_LOG_EVENT is destroyed at the end of the current statement
         clean-up routine.
      */
      res = false;
      break;
    default:
      DBUG_PRINT("info", ("Deleting the event after it has been executed"));
      res = true;
      break;
  }

  return res;
}

bool Transaction_payload_log_event::apply_payload_event(
    Relay_log_info const *rli, const uchar *event_buf) {
  DBUG_TRACE;
  bool res = false;
  const uchar *ptr = event_buf;
  Log_event *ev = nullptr;
  bool copied_buffer = false;
  uchar *copy_buffer = nullptr;

  /*
    disable checksums - there are no checksums for events inside the tple
    otherwise, the last 4 bytes would be truncated.

    We do this by copying the fdle from the rli. Then we disable the checksum
    in the copy. Then we use it to decode the events in the payload instead
    of the original fdle.

    We allocate the fdle copy in the stack.

    TODO: simplify this by breaking the binlog_event_deserialize API
    and make it take a single boolean instead that states whether the
    event has a checksum in it or not.
  */
  Format_description_event *fde = rli->get_rli_description_event();
  Format_description_log_event fdle(fde->reader().buffer(), fde);
  fdle.footer()->checksum_alg = binary_log::BINLOG_CHECKSUM_ALG_OFF;
  fdle.register_temp_buf(const_cast<char *>(fde->reader().buffer()), false);
  size_t event_len = uint4korr(ptr + EVENT_LEN_OFFSET);
  if (binlog_event_deserialize(ptr, event_len, &fdle, true, &ev)) {
    res = true;
    goto end;
  }

  if (!shall_delete_event_after_apply(ev)) {
    copy_buffer =
        (uchar *)my_malloc(key_memory_log_event, event_len, MYF(MY_WME));
    memcpy(copy_buffer, ptr, event_len);
    copied_buffer = true;
  } else {
    copy_buffer = const_cast<uchar *>(ptr);
    copied_buffer = false;
  }

  ev->register_temp_buf((char *)copy_buffer, copied_buffer);
  ev->common_header->log_pos = header()->log_pos;

  thd->server_id = ev->server_id;  // use the original server id for logging
  thd->unmasked_server_id = ev->common_header->unmasked_server_id;
  thd->set_time();  // time the query
  thd->lex->set_current_select(0);
  if (!ev->common_header->when.tv_sec)
    my_micro_time_to_timeval(my_micro_time(), &ev->common_header->when);
  ev->thd = thd;  // because up to this point, ev->thd == 0

  // TODO: HATE THIS
  if (is_mts_worker(thd)) {
    auto worker =
        static_cast<Slave_worker *>(const_cast<Relay_log_info *>(rli));
    this->worker = worker;

    // set in the event context
    ev->future_event_relay_log_pos = this->future_event_relay_log_pos;
    ev->mts_group_idx = mts_group_idx;
    ev->worker = worker;

    // set in the worker context
    worker->set_future_event_relay_log_pos(ev->future_event_relay_log_pos);
    worker->set_master_log_pos(static_cast<ulong>(ev->common_header->log_pos));
    worker->set_gaq_index(ev->mts_group_idx);

    if (ev->get_type_code() == binary_log::QUERY_EVENT)
      static_cast<Query_log_event *>(ev)
          ->set_skip_temp_tables_handling_by_worker();
    res = ev->do_apply_event_worker(worker);
  } else {
    auto coord = const_cast<Relay_log_info *>(rli);
    ev->future_event_relay_log_pos = coord->get_future_event_relay_log_pos();
    res = ev->apply_event(coord);
  }

  if (shall_delete_event_after_apply(ev)) delete ev;

end:
  return res;
}

Log_event::enum_skip_reason Transaction_payload_log_event::do_shall_skip(
    Relay_log_info *rli) {
  return Log_event::continue_group(rli);
}

bool Transaction_payload_log_event::write(Basic_ostream *ostream) {
  DBUG_TRACE;
  auto codec = binary_log::codecs::Factory::build_codec(header()->type_code);
  auto buffer_size = MAX_DATA_LENGTH + LOG_EVENT_HEADER_LEN;
  unsigned char buffer[MAX_DATA_LENGTH + LOG_EVENT_HEADER_LEN];
  auto result = codec->encode(*this, buffer, buffer_size);
  size_t data_size = result.first + m_payload_size;

  if (result.second == true) goto end;

  return write_header(ostream, data_size) ||
         wrapper_my_b_safe_write(ostream, (uchar *)buffer, result.first) ||
         wrapper_my_b_safe_write(ostream,
                                 reinterpret_cast<const uchar *>(m_payload),
                                 m_payload_size) ||
         write_footer(ostream);
end:
  return true;
}

int Transaction_payload_log_event::pack_info(Protocol *protocol) {
  std::ostringstream oss;
  oss << "compression='";
  oss << binary_log::transaction::compression::type_to_string(
      m_compression_type);
  oss << "', decompressed_size=";
  oss << m_uncompressed_size << " bytes";
  protocol->store(oss.str().c_str(), &my_charset_bin);
  return 0;
}

bool Transaction_payload_log_event::ends_group() const { return true; }

#endif

#ifndef MYSQL_SERVER
/**
  The default values for these variables should be values that are
  *incorrect*, i.e., values that cannot occur in an event.  This way,
  they will always be printed for the first event.
*/
PRINT_EVENT_INFO::PRINT_EVENT_INFO()
    : flags2_inited(false),
      sql_mode_inited(false),
      sql_mode(0),
      auto_increment_increment(0),
      auto_increment_offset(0),
      charset_inited(false),
      lc_time_names_number(~0),
      charset_database_number(ILLEGAL_CHARSET_INFO_NUMBER),
      default_collation_for_utf8mb4_number(ILLEGAL_CHARSET_INFO_NUMBER),
      sql_require_primary_key(0xff),
      thread_id(0),
      thread_id_printed(false),
      default_table_encryption(0xff),
      base64_output_mode(BASE64_OUTPUT_UNSPEC),
      printed_fd_event(false),
      inside_binlog(false),
      have_unflushed_events(false),
      skipped_event_in_transaction(false) {
  /*
    Currently we only use static PRINT_EVENT_INFO objects, so zeroed at
    program's startup, but these explicit memset() is for the day someone
    creates dynamic instances.
  */
  memset(db, 0, sizeof(db));
  memset(charset, 0, sizeof(charset));
  memset(time_zone_str, 0, sizeof(time_zone_str));
  delimiter[0] = ';';
  delimiter[1] = 0;
  myf const flags = MYF(MY_WME | MY_NABP);
  open_cached_file(&head_cache, nullptr, nullptr, 0, flags);
  open_cached_file(&body_cache, nullptr, nullptr, 0, flags);
  open_cached_file(&footer_cache, nullptr, nullptr, 0, flags);
}
#endif

#if defined(MYSQL_SERVER)
Heartbeat_log_event::Heartbeat_log_event(
    const char *buf, const Format_description_event *description_event)
    : binary_log::Heartbeat_event(buf, description_event),
      Log_event(header(), footer()) {
  DBUG_TRACE;
}
#endif

#ifdef MYSQL_SERVER
/*
  This is a utility function that adds a quoted identifier into the a buffer.
  This also escapes any existance of the quote string inside the identifier.

  SYNOPSIS
    my_strmov_quoted_identifier
    thd                   thread handler
    buffer                target buffer
    identifier            the identifier to be quoted
    length                length of the identifier
*/
size_t my_strmov_quoted_identifier(THD *thd, char *buffer,
                                   const char *identifier, size_t length) {
  int q = thd ? get_quote_char_for_identifier(thd, identifier, length) : '`';
  return my_strmov_quoted_identifier_helper(q, buffer, identifier, length);
}
#else
size_t my_strmov_quoted_identifier(char *buffer, const char *identifier) {
  int q = '`';
  return my_strmov_quoted_identifier_helper(q, buffer, identifier, 0);
}

#endif

size_t my_strmov_quoted_identifier_helper(int q, char *buffer,
                                          const char *identifier,
                                          size_t length) {
  size_t written = 0;
  char quote_char;
  size_t id_length = (length) ? length : strlen(identifier);

  if (q == EOF) {
    (void)strncpy(buffer, identifier, id_length);
    return id_length;
  }
  quote_char = (char)q;
  *buffer++ = quote_char;
  written++;
  while (id_length--) {
    if (*identifier == quote_char) {
      *buffer++ = quote_char;
      written++;
    }
    *buffer++ = *identifier++;
    written++;
  }
  *buffer++ = quote_char;
  return ++written;
}

std::pair<bool, binary_log::Log_event_basic_info> extract_log_event_basic_info(
    Log_event *log_event) {
  DBUG_TRACE;

  binary_log::Log_event_basic_info event_info;
  event_info.query_length = 0;
  event_info.event_type = log_event->get_type_code();

  if (binary_log::QUERY_EVENT == event_info.event_type) {
    Query_log_event *qlog_event = static_cast<Query_log_event *>(log_event);
    event_info.query = qlog_event->query;
    if (event_info.query != nullptr)
      event_info.query_length = strlen(event_info.query);
    if (event_info.query_length == 0) {
      DBUG_ASSERT(event_info.query == nullptr); /* purecov: inspected */
      return std::make_pair(true, event_info);  /* purecov: inspected */
    }
  }
  event_info.ignorable_event = log_event->is_ignorable_event();
  return std::make_pair(false, event_info);
}

std::pair<bool, binary_log::Log_event_basic_info> extract_log_event_basic_info(
    const char *buf, size_t length,
    const binary_log::Format_description_event *fd_event) {
  DBUG_TRACE;

  binary_log::Log_event_basic_info event_info;
  event_info.query_length = 0;

  uint header_size = fd_event->common_header_len;
  const char *query = nullptr;

  /* Error if the event content is smaller than header size for the format */
  if (length < header_size) return std::make_pair(true, event_info);

  event_info.event_type = (Log_event_type)buf[EVENT_TYPE_OFFSET];

  if (binary_log::QUERY_EVENT == event_info.event_type) {
    event_info.query_length =
        Query_log_event::get_query(buf, length, fd_event, &query);
    if (event_info.query_length == 0) {
      DBUG_ASSERT(query == nullptr);           /* purecov: inspected */
      return std::make_pair(true, event_info); /* purecov: inspected */
    }
    event_info.query = query;
  }
  event_info.ignorable_event =
      uint2korr(buf + FLAGS_OFFSET) & LOG_EVENT_IGNORABLE_F;
  return std::make_pair(false, event_info);
}

#if defined(MYSQL_SERVER)

void Log_event::prepare_dep(Relay_log_info *rli,
                            std::shared_ptr<Log_event_wrapper> &ev) {
  DBUG_ENTER("Log_event::prepare_dep");

  DBUG_ASSERT(is_mts_parallel_type_dependency(rli));
  auto submode =
      static_cast<Mts_submode_dependency *>(rli->current_mts_submode);
  if (!ev->begin_event()) {
    if (!(get_type_code() == binary_log::INTVAR_EVENT ||
          get_type_code() == binary_log::RAND_EVENT ||
          get_type_code() == binary_log::USER_VAR_EVENT ||
          get_type_code() == binary_log::BEGIN_LOAD_QUERY_EVENT ||
          get_type_code() == binary_log::APPEND_BLOCK_EVENT ||
          is_ignorable_event())) {
      char llbuff[22];
      llstr(rli->get_event_relay_log_pos(), llbuff);
      my_error(ER_MTS_CANT_PARALLEL, MYF(0), get_type_str(),
               rli->get_event_relay_log_name(), llbuff,
               "the event is a part of a group that is unsupported in "
               "the parallel execution mode");
    } else {
      sql_print_information(
          "Got independent event %s, master binlog position: %s:%llu",
          get_type_str(), rli->get_group_master_log_name(),
          rli->get_group_master_log_pos());

      submode->set_dep_sync_group(true);
      ev->is_begin_event = true;
    }
  }

  DBUG_VOID_RETURN;
}

void Query_log_event::prepare_dep(Relay_log_info *rli,
                                  std::shared_ptr<Log_event_wrapper> &ev) {
  DBUG_ENTER("Query_log_event::prepare_dep");

  DBUG_ASSERT(is_mts_parallel_type_dependency(rli));
  auto submode =
      static_cast<Mts_submode_dependency *>(rli->current_mts_submode);

  if (starts_group()) {
    if (!submode->current_begin_event) {
      DBUG_ASSERT(ev->begin_event() == nullptr);
      ev->is_begin_event = true;
    }
  } else if (ends_group()) {
    DBUG_ASSERT(submode->prev_event != nullptr);
    DBUG_ASSERT(ev->begin_event() != nullptr);
    DBUG_ASSERT(!ev->begin_event()->whole_group_scheduled);

    ev->is_end_event = true;
    ev->begin_event()->whole_group_scheduled = true;
  } else {
    // case: DML, DDL statement which are logged without BEGIN and COMMIT
    if (!rli->curr_group_seen_begin) {
      ev->is_end_event = true;
      if (ev->begin_event()) {
        DBUG_ASSERT(rli->curr_group_seen_gtid ||
                    ev->begin_event()->raw_event()->get_type_code() ==
                        binary_log::INTVAR_EVENT ||
                    ev->begin_event()->raw_event()->get_type_code() ==
                        binary_log::RAND_EVENT ||
                    ev->begin_event()->raw_event()->get_type_code() ==
                        binary_log::USER_VAR_EVENT ||
                    ev->begin_event()->raw_event()->get_type_code() ==
                        binary_log::BEGIN_LOAD_QUERY_EVENT ||
                    ev->begin_event()->raw_event()->get_type_code() ==
                        binary_log::APPEND_BLOCK_EVENT ||
                    is_ignorable_event());
        DBUG_ASSERT(submode->prev_event != nullptr);
        ev->begin_event()->whole_group_scheduled = true;
      } else {
        ev->is_begin_event = true;
        ev->whole_group_scheduled = true;
      }
    }

    submode->set_dep_sync_group(true);
  }

  DBUG_VOID_RETURN;
}

void Xid_log_event::prepare_dep(MY_ATTRIBUTE((unused)) Relay_log_info *rli,
                                std::shared_ptr<Log_event_wrapper> &ev) {
  DBUG_ENTER("Xid_log_event::prepare_dep");

  DBUG_ASSERT(is_mts_parallel_type_dependency(rli));
  DBUG_ASSERT(static_cast<Mts_submode_dependency *>(rli->current_mts_submode)
                  ->prev_event != nullptr);
  DBUG_ASSERT(ev->begin_event() != nullptr);
  DBUG_ASSERT(!ev->begin_event()->whole_group_scheduled);

  ev->is_end_event = true;
  ev->begin_event()->whole_group_scheduled = true;

  DBUG_VOID_RETURN;
}

/**
  Callback to deallocate a given key buffer. This is passed as a parameter
  while creating shared pointers to the key_buf.
*/
void key_dealloc_cb(uchar *key_buf) { my_free(key_buf); }

bool Rows_log_event::parse_keys(Relay_log_info *rli, TABLE *table,
                                std::deque<Dependency_key> &keys) {
  DBUG_ENTER("Rows_log_event::parse_keys");
  std::vector<KEY *> key_infos;

  // Find PK and all unique keys
  uint key = 0;
  KEY *keyinfo = table->key_info;
  bool found_pk = false;
  for (; key < table->s->keys; key++, keyinfo++) {
    // Skip non-unique keys
    if (!(keyinfo->flags & HA_NOSAME)) continue;

    // NOTE: We expect the before image to contain all the unique keys
    // (including PK), if any key is not present we fail fast and process the
    // transaction in sync mode (see: @Rows_log_event::prepare_dep)
    if (!are_all_columns_signaled_for_key(
            table, keyinfo, &rli->tables_to_lock->m_tabledef, &m_cols))
      DBUG_RETURN(false);

    if (key == table->s->primary_key) found_pk = true;
    key_infos.push_back(&table->key_info[key]);
  }

  // Case: We cannot function without a PK, abort!
  if (!found_pk) DBUG_RETURN(false);

  // Figure out type conversions
  TABLE *conv_table = nullptr;
  if (!rli->tables_to_lock->m_tabledef.compatible_with(
          thd, rli, rli->tables_to_lock->table, &conv_table)) {
    sql_print_error(
        "Unable to check type compatibility at pos %s:%llu, "
        "syncing group",
        rli->get_rpl_log_name(), common_header->log_pos);
    clear_all_errors(rli->info_thd, rli);
    DBUG_RETURN(false);
  }
  rli->tables_to_lock->m_conv_table = conv_table;

  // This is needed only to satisfy some debug asserts, not strictly required
  // for key parsing because upack_row takes cols as an argument instead of
  // relying on read and write sets
  bitmap_set_all(table->read_set);
  bitmap_set_all(table->write_set);

  uchar *curr_row = nullptr, *key_buf = nullptr, *curr_row_end = nullptr;
  enum_row_image_type row_image_type;
  uint i = 0;

  for (i = 0, curr_row = m_rows_buf; curr_row != m_rows_end;
       curr_row = curr_row_end, i++) {
    MY_BITMAP *cols = &m_cols;  // init with before image bitmap

    if (get_general_type_code() == binary_log::UPDATE_ROWS_EVENT) {
      // NOTE: In updates, the after image follows the before image, hence
      // every odd index will be an after image
      if (i % 2 == 1) {
        cols = &m_cols_ai;
        row_image_type = enum_row_image_type::UPDATE_AI;
      } else {
        row_image_type = enum_row_image_type::UPDATE_BI;
      }
    } else {
      row_image_type = get_general_type_code() == binary_log::WRITE_ROWS_EVENT
                           ? enum_row_image_type::WRITE_AI
                           : enum_row_image_type::DELETE_BI;
    }

    bool has_value_options =
        (get_type_code() == binary_log::PARTIAL_UPDATE_ROWS_EVENT);

    // Unpack the row
    // This codepath should only be invoked through dependency replication
    // The SQL thread should not log the keys to the row_query string
    DBUG_ASSERT(is_mts_parallel_type_dependency(rli));
    if (::unpack_row(rli, table, m_width, curr_row, cols,
                     const_cast<const uchar **>(&curr_row_end), m_rows_end,
                     row_image_type, has_value_options, false)) {
      /* We were unable to unpack the row. This is a serious error. */
      sql_print_error("Unable to unpack row at pos %s:%llu, syncing group",
                      rli->get_rpl_log_name(), common_header->log_pos);
      clear_all_errors(rli->info_thd, rli);
      DBUG_RETURN(false);
    }

    for (const auto &key_info : key_infos) {
      // Case: This key is not present in this row image
      if (!are_all_columns_signaled_for_key(
              table, key_info, &rli->tables_to_lock->m_tabledef, cols)) {
        // NOTE: we've already checked if all keys are present in the before
        // image in the beginning of this method, so this has to be the after
        // image
        DBUG_ASSERT(cols == &m_cols_ai);
        continue;
      }

      // TODO (abhinav): We should check if a key is null to avoid unnecessary
      // dependencies between null unique keys (since nulls are treated as
      // distinct)
      Dependency_key curr_key;

      for (uint j = 0; j < key_info->user_defined_key_parts; j++) {
        curr_key.key_length += key_info->key_part[j].field->sort_length();
      }
      curr_key.table_id = m_table_name;

      key_buf = (uchar *)my_malloc(key_memory_log_event, curr_key.key_length,
                                   MYF(MY_WME));
      if (!key_buf) {
        sql_print_error(
            "Unable to allocate memory for dependency key at "
            "%s:%llu, syncing group",
            rli->get_rpl_log_name(), common_header->log_pos);
        clear_all_errors(rli->info_thd, rli);
        DBUG_RETURN(false);
      }

      memset(key_buf, 0x0, curr_key.key_length);

      auto buf = key_buf;
      for (uint j = 0; j < key_info->user_defined_key_parts; j++) {
        const auto len = key_info->key_part[j].field->sort_length();
        key_info->key_part[j].field->make_sort_key(buf, len);
        buf += len;
      }

      std::shared_ptr<uchar> tmp(key_buf, key_dealloc_cb);
      curr_key.key_buffer = tmp;
      keys.push_back(curr_key);
    }
  }
  // dbug case: either all BIs and AIs will have keys, or only all BIs will have
  // keys
  DBUG_ASSERT(keys.size() == 2 * i || keys.size() == i);
  DBUG_RETURN(true);
}

/**
  Returns the set of keys written by the statement as a deque.
*/
bool Rows_log_event::get_keys(Relay_log_info *rli,
                              std::deque<Dependency_key> &keys) {
  DBUG_ENTER("Rows_log_event::get_keys");

  RPL_TABLE_LIST *table_list = nullptr;
  void *memory = nullptr;
  bool success = false;

  DBUG_ASSERT(is_mts_parallel_type_dependency(rli));

  auto submode =
      static_cast<Mts_submode_dependency *>(rli->current_mts_submode);

  // case: table level dependency, just add the table key and return
  if (rli->mts_dependency_replication == DEP_RPL_TABLE ||
      submode->tbl_mode_tables.count(m_table_name)) {
    Dependency_key table_key;
    table_key.table_id = m_table_name;
    keys.push_back(table_key);
    DBUG_RETURN(true);
  }

  DBUG_ASSERT(rli->mts_dependency_replication == DEP_RPL_STATEMENT);
  DBUG_ASSERT(thd->open_tables == nullptr);
  DBUG_ASSERT(m_key_info == nullptr);
  DBUG_ASSERT(m_table == nullptr);
  DBUG_ASSERT(m_curr_row_end == nullptr);
  DBUG_ASSERT(m_curr_row == m_rows_buf);

  /* Try to get a reference to the table definition. This may fail if the
   * corresponding DDL statement hasn't yet been executed.
   */
  if (unlikely(!get_table_ref(rli, &memory, &table_list))) {
    DBUG_ASSERT(rli->tables_to_lock == nullptr);
    rli->tables_to_lock = table_list;
    success = parse_keys(rli, table_list->table, keys);
    rli->tables_to_lock = nullptr;
  }

  DBUG_ASSERT(memory != nullptr);
  DBUG_ASSERT(table_list->m_tabledef_valid);

  /* Cleanup any state associated with the table definition. */
  close_table_ref(table_list);
  if (table_list && table_list->m_tabledef_valid) {
    table_list->m_tabledef.table_def::~table_def();
    table_list->m_tabledef_valid = false;
  }
  if (table_list && table_list->m_conv_table)
    free_blobs(table_list->m_conv_table);
  my_free(memory);

  /* Cleanup local state used to parse out keys. */
  m_key_info = nullptr;
  m_table = nullptr;
  m_curr_row_end = nullptr;
  m_curr_row = m_rows_buf;

  if (!success) {
    const auto tbe = submode->table_map_events.at(get_table_id());
    sql_print_information(
        "Error while parsing keys for table %s.%s, will schedule trxs for this "
        "table using table dependencies",
        tbe->get_db_name(), tbe->get_table_name());
    submode->tbl_mode_tables.insert(m_table_name);
  }

  DBUG_RETURN(success);
}

/**
  Returns a reference to a single TABLE* in the provided table_list. On return,
  the rli thread has a shared meta-data lock on the TABLE*. This lock is
  released in close_table_ref. This function might break if we're holding more
  meta-data locks and assumes that no meta-data locks are held at the time it
  is called.
*/
bool Rows_log_event::get_table_ref(Relay_log_info *rli, void **memory,
                                   RPL_TABLE_LIST **table_list) {
  DBUG_ENTER("Rows_log_event::get_table_ref");

  DBUG_ASSERT(is_mts_parallel_type_dependency(rli));
  auto submode =
      static_cast<Mts_submode_dependency *>(rli->current_mts_submode);

  DBUG_ASSERT(!thd->mdl_context.has_locks());
  DBUG_ASSERT(submode->table_map_events.count(get_table_id()));
  DBUG_ASSERT(memory != nullptr);

  auto table_map = submode->table_map_events.at(get_table_id());

  if (!(*memory = table_map->setup_table_rli(table_list))) DBUG_RETURN(1);

  DBUG_ASSERT(*memory != nullptr);
  (*table_list)->mdl_request.duration = MDL_STATEMENT;
  (*table_list)->mdl_request.type = MDL_SHARED;

  bool error = get_table_from_cache(thd, (TABLE_LIST *)(*table_list));
  DBUG_ASSERT(thd->mdl_context.has_locks());
  DBUG_RETURN(error);
}

/**
  Close the one and only TABLE* in the given table_list.
*/
void Rows_log_event::close_table_ref(RPL_TABLE_LIST *table_list) {
  DBUG_ENTER("Rows_log_event::close_table_ref");
  DBUG_ASSERT(thd->mdl_context.has_locks());

  return_table_to_cache(thd, (TABLE_LIST *)table_list);

  DBUG_ASSERT(!thd->mdl_context.has_locks());
  DBUG_ASSERT(thd->open_tables == nullptr);
  DBUG_VOID_RETURN;
}

void Rows_log_event::prepare_dep(Relay_log_info *rli,
                                 std::shared_ptr<Log_event_wrapper> &ev) {
  DBUG_ENTER("Rows_log_event::prepare_dep");

  DBUG_ASSERT(is_mts_parallel_type_dependency(rli));
  auto submode =
      static_cast<Mts_submode_dependency *>(rli->current_mts_submode);

  DBUG_ASSERT(submode);
  DBUG_ASSERT(submode->prev_event != nullptr);
  DBUG_ASSERT(submode->table_map_events.count(get_table_id()));

  // case: this group will be synced, so we don't need to parse and store keys
  if (submode->dep_sync_group) {
    DBUG_VOID_RETURN;
  }

  const auto tbe = submode->table_map_events.at(get_table_id());
  DBUG_ASSERT(tbe);
  std::string db_name(tbe->get_db_name());
  std::string table_name(tbe->get_table_name());

  auto full_table_name = db_name.append(std::to_string(db_name.length()))
                             .append(table_name)
                             .append(std::to_string(table_name.length()));

  m_table_name = full_table_name;
  DBUG_ASSERT(!m_table_name.empty());

  submode->add_row_event(rli, this, ev, m_keylist);

  DBUG_VOID_RETURN;
}

void *Table_map_log_event::setup_table_rli(RPL_TABLE_LIST **table_list) {
  DBUG_ENTER("Table_map_log_event::setup_table_rli");

  char *db_mem = nullptr, *tname_mem = nullptr;
  void *memory = nullptr;

  if (!(memory = my_multi_malloc(PSI_NOT_INSTRUMENTED, MYF(MY_WME), table_list,
                                 (uint)sizeof(RPL_TABLE_LIST), &db_mem,
                                 (uint)NAME_LEN + 1, &tname_mem,
                                 (uint)NAME_LEN + 1, NullS)))
    DBUG_RETURN(nullptr);

  strcpy(db_mem, m_dbnam.c_str());
  strcpy(tname_mem, m_tblnam.c_str());

  if (lower_case_table_names == 1) {
    my_casedn_str(system_charset_info, db_mem);
    my_casedn_str(system_charset_info, tname_mem);
  }

  new (*table_list) RPL_TABLE_LIST(db_mem, strlen(db_mem), tname_mem,
                                   strlen(tname_mem), tname_mem, TL_WRITE);

  new (&((*table_list)->m_tabledef)) table_def(
      m_coltype, m_colcnt, m_field_metadata, m_field_metadata_size, m_null_bits,
      m_flags, m_column_names, m_column_names_size, m_sign_bits);
  (*table_list)->m_tabledef_valid = true;
  (*table_list)->m_conv_table = nullptr;
  (*table_list)->open_type = OT_BASE_ONLY;

  DBUG_RETURN(memory);
}

void Gtid_log_event::prepare_dep(Relay_log_info *,
                                 std::shared_ptr<Log_event_wrapper> &ev) {
  DBUG_ENTER("Gtid_log_event::prepare_dep");
  DBUG_ASSERT(ev->begin_event() == nullptr);
  ev->is_begin_event = true;
  DBUG_VOID_RETURN;
}

#endif  // defined(MYSQL_SERVER)
