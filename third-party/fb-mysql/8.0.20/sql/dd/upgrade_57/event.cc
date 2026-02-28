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

#include "sql/dd/upgrade_57/event.h"

#include <string.h>
#include <sys/types.h>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_base.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "my_time.h"
#include "my_user.h"  // parse_user
#include "mysql/components/services/log_builtins.h"
#include "mysql/psi/psi_base.h"
#include "mysql/udf_registration_types.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/dd/cache/dictionary_client.h"  // dd::cache::Dictionary_client
#include "sql/dd/dd_event.h"                 // create_event
#include "sql/dd/impl/upgrade/server.h"
#include "sql/dd/types/event.h"
#include "sql/dd/upgrade_57/global.h"
#include "sql/event_db_repository.h"  // Events
#include "sql/event_parse_data.h"     // Event_parse_data
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/histograms/value_map.h"
#include "sql/key.h"
#include "sql/log.h"       // LogErr()
#include "sql/mysqld.h"    // default_tz_name
#include "sql/sp.h"        // load_charset
#include "sql/sql_base.h"  // open_tables
#include "sql/sql_class.h"
#include "sql/sql_const.h"
#include "sql/sql_servers.h"
#include "sql/sql_time.h"  // interval_type_to_name
#include "sql/system_variables.h"
#include "sql/table.h"  // Table_check_intact
#include "sql/thd_raii.h"
#include "sql/thr_malloc.h"
#include "sql/transaction.h"  // trans_commit
#include "sql/tztime.h"       // my_tz_find
#include "sql_string.h"
#include "thr_lock.h"

namespace dd {
class Schema;
}  // namespace dd

namespace dd {
namespace upgrade_57 {

static Check_table_intact table_intact;

/**
  Column definitions for 5.7 mysql.event table (5.7.13 and up).
*/
const TABLE_FIELD_TYPE event_table_fields[ET_FIELD_COUNT] = {
    {{STRING_WITH_LEN("db")},
     {STRING_WITH_LEN("char(64)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("name")},
     {STRING_WITH_LEN("char(64)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("body")}, {STRING_WITH_LEN("longblob")}, {nullptr, 0}},
    {{STRING_WITH_LEN("definer")},
     {STRING_WITH_LEN("char(93)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("execute_at")},
     {STRING_WITH_LEN("datetime")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("interval_value")},
     {STRING_WITH_LEN("int")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("interval_field")},
     {STRING_WITH_LEN(
         "enum('YEAR','QUARTER','MONTH','DAY',"
         "'HOUR','MINUTE','WEEK','SECOND','MICROSECOND','YEAR_MONTH','DAY_HOUR'"
         ","
         "'DAY_MINUTE','DAY_SECOND','HOUR_MINUTE','HOUR_SECOND','MINUTE_SECOND'"
         ","
         "'DAY_MICROSECOND','HOUR_MICROSECOND','MINUTE_MICROSECOND',"
         "'SECOND_MICROSECOND')")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("created")},
     {STRING_WITH_LEN("timestamp")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("modified")},
     {STRING_WITH_LEN("timestamp")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("last_executed")},
     {STRING_WITH_LEN("datetime")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("starts")}, {STRING_WITH_LEN("datetime")}, {nullptr, 0}},
    {{STRING_WITH_LEN("ends")}, {STRING_WITH_LEN("datetime")}, {nullptr, 0}},
    {{STRING_WITH_LEN("status")},
     {STRING_WITH_LEN("enum('ENABLED','DISABLED','SLAVESIDE_DISABLED')")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("on_completion")},
     {STRING_WITH_LEN("enum('DROP','PRESERVE')")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("sql_mode")},
     {STRING_WITH_LEN(
         "set('REAL_AS_FLOAT','PIPES_AS_CONCAT','ANSI_QUOTES',"
         "'IGNORE_SPACE','NOT_USED','ONLY_FULL_GROUP_BY','NO_UNSIGNED_"
         "SUBTRACTION',"
         "'NO_DIR_IN_CREATE','POSTGRESQL','ORACLE','MSSQL','DB2','MAXDB',"
         "'NO_KEY_OPTIONS','NO_TABLE_OPTIONS','NO_FIELD_OPTIONS','MYSQL323','"
         "MYSQL40',"
         "'ANSI','NO_AUTO_VALUE_ON_ZERO','NO_BACKSLASH_ESCAPES','STRICT_TRANS_"
         "TABLES',"
         "'STRICT_ALL_TABLES','NO_ZERO_IN_DATE','NO_ZERO_DATE','INVALID_DATES',"
         "'ERROR_FOR_DIVISION_BY_ZERO','TRADITIONAL','NO_AUTO_CREATE_USER',"
         "'HIGH_NOT_PRECEDENCE','NO_ENGINE_SUBSTITUTION','PAD_CHAR_TO_FULL_"
         "LENGTH')")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("comment")},
     {STRING_WITH_LEN("char(64)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("originator")}, {STRING_WITH_LEN("int")}, {nullptr, 0}},
    {{STRING_WITH_LEN("time_zone")},
     {STRING_WITH_LEN("char(64)")},
     {STRING_WITH_LEN("latin1")}},
    {{STRING_WITH_LEN("character_set_client")},
     {STRING_WITH_LEN("char(32)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("collation_connection")},
     {STRING_WITH_LEN("char(32)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("db_collation")},
     {STRING_WITH_LEN("char(32)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("body_utf8")},
     {STRING_WITH_LEN("longblob")},
     {nullptr, 0}}};

static const TABLE_FIELD_DEF event_table_def = {ET_FIELD_COUNT,
                                                event_table_fields};

/**
  Column definitions for 5.7 mysql.event table (before 5.7.13).
*/

static const TABLE_FIELD_TYPE event_table_fields_old[ET_FIELD_COUNT] = {
    {{STRING_WITH_LEN("db")},
     {STRING_WITH_LEN("char(64)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("name")},
     {STRING_WITH_LEN("char(64)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("body")}, {STRING_WITH_LEN("longblob")}, {nullptr, 0}},
    {{STRING_WITH_LEN("definer")},
     {STRING_WITH_LEN("char(77)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("execute_at")},
     {STRING_WITH_LEN("datetime")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("interval_value")},
     {STRING_WITH_LEN("int")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("interval_field")},
     {STRING_WITH_LEN(
         "enum('YEAR','QUARTER','MONTH','DAY',"
         "'HOUR','MINUTE','WEEK','SECOND','MICROSECOND','YEAR_MONTH','DAY_HOUR'"
         ","
         "'DAY_MINUTE','DAY_SECOND','HOUR_MINUTE','HOUR_SECOND','MINUTE_SECOND'"
         ","
         "'DAY_MICROSECOND','HOUR_MICROSECOND','MINUTE_MICROSECOND',"
         "'SECOND_MICROSECOND')")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("created")},
     {STRING_WITH_LEN("timestamp")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("modified")},
     {STRING_WITH_LEN("timestamp")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("last_executed")},
     {STRING_WITH_LEN("datetime")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("starts")}, {STRING_WITH_LEN("datetime")}, {nullptr, 0}},
    {{STRING_WITH_LEN("ends")}, {STRING_WITH_LEN("datetime")}, {nullptr, 0}},
    {{STRING_WITH_LEN("status")},
     {STRING_WITH_LEN("enum('ENABLED','DISABLED','SLAVESIDE_DISABLED')")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("on_completion")},
     {STRING_WITH_LEN("enum('DROP','PRESERVE')")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("sql_mode")},
     {STRING_WITH_LEN(
         "set('REAL_AS_FLOAT','PIPES_AS_CONCAT','ANSI_QUOTES',"
         "'IGNORE_SPACE','NOT_USED','ONLY_FULL_GROUP_BY','NO_UNSIGNED_"
         "SUBTRACTION',"
         "'NO_DIR_IN_CREATE','POSTGRESQL','ORACLE','MSSQL','DB2','MAXDB',"
         "'NO_KEY_OPTIONS','NO_TABLE_OPTIONS','NO_FIELD_OPTIONS','MYSQL323','"
         "MYSQL40',"
         "'ANSI','NO_AUTO_VALUE_ON_ZERO','NO_BACKSLASH_ESCAPES','STRICT_TRANS_"
         "TABLES',"
         "'STRICT_ALL_TABLES','NO_ZERO_IN_DATE','NO_ZERO_DATE','INVALID_DATES',"
         "'ERROR_FOR_DIVISION_BY_ZERO','TRADITIONAL','NO_AUTO_CREATE_USER',"
         "'HIGH_NOT_PRECEDENCE','NO_ENGINE_SUBSTITUTION','PAD_CHAR_TO_FULL_"
         "LENGTH')")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("comment")},
     {STRING_WITH_LEN("char(64)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("originator")}, {STRING_WITH_LEN("int")}, {nullptr, 0}},
    {{STRING_WITH_LEN("time_zone")},
     {STRING_WITH_LEN("char(64)")},
     {STRING_WITH_LEN("latin1")}},
    {{STRING_WITH_LEN("character_set_client")},
     {STRING_WITH_LEN("char(32)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("collation_connection")},
     {STRING_WITH_LEN("char(32)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("db_collation")},
     {STRING_WITH_LEN("char(32)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("body_utf8")},
     {STRING_WITH_LEN("longblob")},
     {nullptr, 0}}};

static const TABLE_FIELD_DEF event_table_def_old = {ET_FIELD_COUNT,
                                                    event_table_fields_old};

/**
   Load the charset and time zone information for an event.
*/
static void load_event_creation_context(THD *thd, TABLE *table,
                                        Event_parse_data *et_parse_data) {
  LEX_STRING tz_name;
  const CHARSET_INFO *client_cs;
  const CHARSET_INFO *connection_cl;
  thd->variables.time_zone = my_tz_SYSTEM;

  if ((tz_name.str = get_field(thd->mem_root,
                               table->field[ET_FIELD_TIME_ZONE])) == nullptr) {
    LogErr(WARNING_LEVEL, ER_EVENT_CANT_GET_TIMEZONE_FROM_FIELD,
           et_parse_data->dbname.str, et_parse_data->name.str);
  } else {
    tz_name.length = strlen(tz_name.str);
    String tz_str(tz_name.str, &my_charset_latin1);
    if ((thd->variables.time_zone = my_tz_find(thd, &tz_str)) == nullptr) {
      thd->variables.time_zone = my_tz_SYSTEM;
      LogErr(WARNING_LEVEL, ER_EVENT_CANT_FIND_TIMEZONE,
             et_parse_data->dbname.str, et_parse_data->name.str);
    }
  }

  if (load_charset(thd->mem_root, table->field[ET_FIELD_CHARACTER_SET_CLIENT],
                   thd->variables.character_set_client, &client_cs)) {
    LogErr(WARNING_LEVEL, ER_EVENT_CANT_GET_CHARSET, et_parse_data->dbname.str,
           et_parse_data->name.str);
  }

  if (load_collation(thd->mem_root, table->field[ET_FIELD_COLLATION_CONNECTION],
                     thd->variables.collation_connection, &connection_cl)) {
    LogErr(WARNING_LEVEL, ER_EVENT_CANT_GET_COLLATION,
           et_parse_data->dbname.str, et_parse_data->name.str);
  }

  thd->variables.character_set_client = client_cs;
  thd->variables.collation_connection = connection_cl;
}

/**
   Update the created, last modified and last executed
   time for the event with the values read from the old
   data dir.
*/

static bool update_event_timing_fields(THD *thd, TABLE *table,
                                       const char *event_db_name,
                                       const char *event_name) {
  dd::Event *new_event = nullptr;
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  if (thd->dd_client()->acquire_for_modification(event_db_name, event_name,
                                                 &new_event))
    return true;

  if (new_event == nullptr) return true;

  if (!table->field[ET_FIELD_LAST_EXECUTED]->is_null()) {
    MYSQL_TIME time;
    my_time_t last_executed;
    bool not_used = false;
    table->field[ET_FIELD_LAST_EXECUTED]->get_date(&time, TIME_NO_ZERO_DATE);
    last_executed = my_tz_OFFSET0->TIME_to_gmt_sec(&time, &not_used);
    new_event->set_last_executed(last_executed);
  }

  new_event->set_created(table->field[ET_FIELD_CREATED]->val_int());
  new_event->set_last_altered(table->field[ET_FIELD_MODIFIED]->val_int());

  if (thd->dd_client()->update(new_event)) {
    trans_rollback_stmt(thd);
    return true;
  }

  return trans_commit_stmt(thd) || trans_commit(thd);
}

/**
  Searches for a LEX_STRING in an LEX_STRING array.

  @param[in] haystack  The array.
  @param[in] needle    The string to search for.
  @param[in] cs        Charset info.

  @note The last LEX_STRING in the array should have str member set to NULL.

  @retval -1   Not found.
  @retval >=0  Ordinal position.
*/

static int find_string_in_array(const LEX_CSTRING *haystack,
                                const LEX_CSTRING *needle,
                                const CHARSET_INFO *cs) {
  const LEX_CSTRING *pos;
  for (pos = haystack; pos->str; pos++) {
    if (!cs->coll->strnncollsp(
            cs, pointer_cast<const uchar *>(pos->str), pos->length,
            pointer_cast<const uchar *>(needle->str), needle->length)) {
      return static_cast<int>(pos - haystack);
    }
  }

  return -1;
}

/**
  Update the event's interval and status information in the DD.
*/

static bool set_status_and_interval_for_event(THD *thd, TABLE *table,
                                              Event_parse_data *et_parse_data) {
  char *ptr;
  bool not_used = false;
  MYSQL_TIME time;

  if (!table->field[ET_FIELD_INTERVAL_EXPR]->is_null())
    et_parse_data->expression = table->field[ET_FIELD_INTERVAL_EXPR]->val_int();
  else
    et_parse_data->expression = 0;

  /*
    If neither STARTS and ENDS is set, then both fields are empty.
    Hence, if ET_FIELD_EXECUTE_AT is empty there is an error.
  */
  et_parse_data->execute_at_null = table->field[ET_FIELD_EXECUTE_AT]->is_null();
  if (!et_parse_data->expression && !et_parse_data->execute_at_null) {
    if (table->field[ET_FIELD_EXECUTE_AT]->get_date(&time, TIME_NO_ZERO_DATE))
      return true;
    et_parse_data->execute_at =
        my_tz_OFFSET0->TIME_to_gmt_sec(&time, &not_used);
  }

  /*
    We load the interval type from disk as string and then map it to
    an integer. This decouples the values of enum interval_type
    and values actually stored on disk. Therefore the type can be
    reordered without risking incompatibilities of data between versions.
  */
  if (!table->field[ET_FIELD_TRANSIENT_INTERVAL]->is_null()) {
    int i;
    char buff[MAX_FIELD_WIDTH];
    String str(buff, sizeof(buff), &my_charset_bin);
    LEX_CSTRING tmp;

    table->field[ET_FIELD_TRANSIENT_INTERVAL]->val_str(&str);
    if (!(tmp.length = str.length())) return true;

    tmp.str = str.c_ptr_safe();

    i = find_string_in_array(interval_type_to_name, &tmp, system_charset_info);
    if (i < 0) return true;
    et_parse_data->interval = (interval_type)i;
  }

  if ((ptr = get_field(thd->mem_root, table->field[ET_FIELD_STATUS])) ==
      nullptr)
    return true;

  switch (ptr[0]) {
    case 'E':
      et_parse_data->status = Event_parse_data::ENABLED;
      break;
    case 'S':
      et_parse_data->status = Event_parse_data::SLAVESIDE_DISABLED;
      break;
    case 'D':
    default:
      et_parse_data->status = Event_parse_data::DISABLED;
      break;
  }
  return false;
}

/**
   Create an entry in the DD for the event by reading all the
   event attributes stored in 'mysql.event' table.
*/
static bool migrate_event_to_dd(THD *thd, TABLE *event_table) {
  char *ptr;
  MYSQL_TIME time;
  LEX_USER user_info;
  Event_parse_data et_parse_data;
  LEX_STRING event_body, event_body_utf8;

  et_parse_data.interval = INTERVAL_LAST;
  et_parse_data.identifier = nullptr;

  if ((et_parse_data.definer.str = get_field(
           thd->mem_root, event_table->field[ET_FIELD_DEFINER])) == nullptr)
    return true;
  et_parse_data.definer.length = strlen(et_parse_data.definer.str);

  if ((et_parse_data.name.str = get_field(
           thd->mem_root, event_table->field[ET_FIELD_NAME])) == nullptr)
    return true;
  et_parse_data.name.length = strlen(et_parse_data.name.str);

  if ((et_parse_data.dbname.str = get_field(
           thd->mem_root, event_table->field[ET_FIELD_DB])) == nullptr)
    return true;
  et_parse_data.dbname.length = strlen(et_parse_data.dbname.str);

  if ((et_parse_data.comment.str = get_field(
           thd->mem_root, event_table->field[ET_FIELD_COMMENT])) == nullptr)
    et_parse_data.comment.length = 0;
  else
    et_parse_data.comment.length = strlen(et_parse_data.comment.str);

  bool not_used = false;
  et_parse_data.starts_null = event_table->field[ET_FIELD_STARTS]->is_null();
  if (!et_parse_data.starts_null) {
    event_table->field[ET_FIELD_STARTS]->get_date(&time, TIME_NO_ZERO_DATE);
    et_parse_data.starts = my_tz_OFFSET0->TIME_to_gmt_sec(&time, &not_used);
  }

  et_parse_data.ends_null = event_table->field[ET_FIELD_ENDS]->is_null();
  if (!et_parse_data.ends_null) {
    event_table->field[ET_FIELD_ENDS]->get_date(&time, TIME_NO_ZERO_DATE);
    et_parse_data.ends = my_tz_OFFSET0->TIME_to_gmt_sec(&time, &not_used);
  }

  et_parse_data.originator = event_table->field[ET_FIELD_ORIGINATOR]->val_int();

  if (set_status_and_interval_for_event(thd, event_table, &et_parse_data))
    return true;

  if ((ptr = get_field(thd->mem_root,
                       event_table->field[ET_FIELD_ORIGINATOR])) == nullptr)
    return true;

  if ((ptr = get_field(thd->mem_root,
                       event_table->field[ET_FIELD_ON_COMPLETION])) == nullptr)
    return true;

  et_parse_data.on_completion =
      (ptr[0] == 'D' ? Event_parse_data::ON_COMPLETION_DROP
                     : Event_parse_data::ON_COMPLETION_PRESERVE);

  // Set up the event body.
  if ((event_body.str = get_field(
           thd->mem_root, event_table->field[ET_FIELD_BODY])) == nullptr)
    return true;
  event_body.length = strlen(event_body.str);

  if ((event_body_utf8.str = get_field(
           thd->mem_root, event_table->field[ET_FIELD_BODY_UTF8])) == nullptr)
    return true;
  event_body_utf8.length = strlen(event_body_utf8.str);
  et_parse_data.body_changed = true;

  dd::upgrade::Routine_event_context_guard event_ctx_guard(thd);

  thd->variables.sql_mode = (sql_mode_t)(
      event_table->field[ET_FIELD_SQL_MODE]->val_int() & MODE_ALLOWED_MASK);

  // Holders for user name and host name used in parse user.
  char definer_user_name_holder[USERNAME_LENGTH + 1];
  char definer_host_name_holder[HOSTNAME_LENGTH + 1];
  memset(&user_info, 0, sizeof(LEX_USER));
  user_info.user = {definer_user_name_holder, USERNAME_LENGTH};
  user_info.host = {definer_host_name_holder, HOSTNAME_LENGTH};

  parse_user(et_parse_data.definer.str, et_parse_data.definer.length,
             definer_user_name_holder, &user_info.user.length,
             definer_host_name_holder, &user_info.host.length);

  load_event_creation_context(thd, event_table, &et_parse_data);

  dd::String_type event_sql;
  if (dd::upgrade::build_event_sp(thd, et_parse_data.name.str,
                                  et_parse_data.name.length, event_body.str,
                                  event_body.length, &event_sql) ||
      dd::upgrade::invalid_sql(thd, et_parse_data.dbname.str, event_sql)) {
    LogErr(ERROR_LEVEL, ER_UPGRADE_PARSE_ERROR, "Event",
           et_parse_data.dbname.str, et_parse_data.name.str,
           dd::upgrade::Syntax_error_handler::error_message());
    return false;
  }

  // Disable autocommit option in thd variable
  Disable_autocommit_guard autocommit_guard(thd);

  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  const dd::Schema *schema = nullptr;
  if (thd->dd_client()->acquire(et_parse_data.dbname.str, &schema)) return true;
  DBUG_ASSERT(schema != nullptr);

  if (dd::create_event(thd, *schema, et_parse_data.name.str, event_body.str,
                       event_body_utf8.str, &user_info, &et_parse_data)) {
    trans_rollback_stmt(thd);
    // Full rollback we have THD::transaction_rollback_request.
    trans_rollback(thd);
    return true;
  }

  if (trans_commit_stmt(thd) || trans_commit(thd)) return true;

  return (update_event_timing_fields(thd, event_table, et_parse_data.dbname.str,
                                     et_parse_data.name.str));
}

/**
   Migrate all the events from 'mysql.event' to 'events' DD table.
*/

bool migrate_events_to_dd(THD *thd) {
  TABLE *event_table;
  int error = 0;
  uint flags = MYSQL_LOCK_IGNORE_TIMEOUT;
  DML_prelocking_strategy prelocking_strategy;
  MEM_ROOT records_mem_root;
  Thd_mem_root_guard root_guard(thd, &records_mem_root);

  TABLE_LIST tables("mysql", "event", TL_READ);
  auto table_list = &tables;

  if (open_and_lock_tables(thd, table_list, flags, &prelocking_strategy)) {
    LogErr(ERROR_LEVEL, ER_EVENT_CANT_OPEN_TABLE_MYSQL_EVENT);
    return true;
  }

  event_table = tables.table;
  event_table->use_all_columns();

  if (table_intact.check(thd, event_table, &event_table_def)) {
    // check with table format too before returning error.
    if (table_intact.check(thd, event_table, &event_table_def_old)) {
      close_thread_tables(thd);
      return true;
    }
  }

  System_table_close_guard event_table_guard(thd, event_table);

  // Initialize time zone support infrastructure since the information
  // is not available during upgrade.
  my_tz_init(thd, default_tz_name, false);

  if (event_table->file->ha_index_init(0, true)) {
    LogErr(ERROR_LEVEL, ER_EVENT_CANT_OPEN_TABLE_MYSQL_EVENT);
    goto err;
  }

  // Read the first row in the 'event' table via index.
  if ((error = event_table->file->ha_index_first(event_table->record[0]))) {
    if (error == HA_ERR_END_OF_FILE) {
      my_tz_free();
      return false;
    }
    LogErr(ERROR_LEVEL, ER_EVENT_CANT_OPEN_TABLE_MYSQL_EVENT);
    goto err;
  }

  if (migrate_event_to_dd(thd, event_table)) goto err;

  // Read the next row in 'event' table via index.
  while (!(error = event_table->file->ha_index_next(event_table->record[0])) &&
         !dd::upgrade::Syntax_error_handler::has_too_many_errors()) {
    if (migrate_event_to_dd(thd, event_table)) goto err;
  }

  if (error != HA_ERR_END_OF_FILE) {
    LogErr(ERROR_LEVEL, ER_EVENT_CANT_OPEN_TABLE_MYSQL_EVENT);
    goto err;
  }

  my_tz_free();
  return dd::upgrade::Syntax_error_handler::has_errors();

err:
  my_tz_free();
  return true;
}

}  // namespace upgrade_57
}  // namespace dd
