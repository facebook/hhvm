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

#include "sql/dd/upgrade_57/routine.h"

#include <string.h>
#include <sys/types.h>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_base.h"
#include "my_inttypes.h"
#include "my_loglevel.h"
#include "my_sys.h"
#include "my_user.h"  // parse_user
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/psi/psi_base.h"
#include "mysql/udf_registration_types.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/dd/types/schema.h"
#include "sql/dd/upgrade_57/global.h"
#include "sql/dd_sp.h"  // prepare_sp_chistics_from_dd_routine
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/key.h"
#include "sql/log.h"       // LogErr()
#include "sql/sp.h"        // db_load_routine
#include "sql/sp_head.h"   // sp_head
#include "sql/sql_base.h"  // open_tables
#include "sql/sql_class.h"
#include "sql/sql_connect.h"
#include "sql/sql_const.h"
#include "sql/sql_lex.h"
#include "sql/sql_servers.h"
#include "sql/system_variables.h"
#include "sql/table.h"  // Table_check_intact
#include "sql/thd_raii.h"
#include "sql/thr_malloc.h"
#include "sql_string.h"
#include "thr_lock.h"

#include "sql/dd/impl/upgrade/server.h"

namespace dd {

namespace upgrade_57 {

static Check_table_intact table_intact;

/**
  Column definitions for 5.7 mysql.proc table (5.7.13 and up).
*/
static const TABLE_FIELD_TYPE proc_table_fields[MYSQL_PROC_FIELD_COUNT] = {
    {{STRING_WITH_LEN("db")},
     {STRING_WITH_LEN("char(64)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("name")},
     {STRING_WITH_LEN("char(64)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("type")},
     {STRING_WITH_LEN("enum('FUNCTION','PROCEDURE')")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("specific_name")},
     {STRING_WITH_LEN("char(64)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("language")},
     {STRING_WITH_LEN("enum('SQL')")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("sql_data_access")},
     {STRING_WITH_LEN(
         "enum('CONTAINS_SQL','NO_SQL','READS_SQL_DATA','MODIFIES_SQL_DATA')")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("is_deterministic")},
     {STRING_WITH_LEN("enum('YES','NO')")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("security_type")},
     {STRING_WITH_LEN("enum('INVOKER','DEFINER')")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("param_list")}, {STRING_WITH_LEN("blob")}, {nullptr, 0}},

    {{STRING_WITH_LEN("returns")}, {STRING_WITH_LEN("longblob")}, {nullptr, 0}},
    {{STRING_WITH_LEN("body")}, {STRING_WITH_LEN("longblob")}, {nullptr, 0}},
    {{STRING_WITH_LEN("definer")},
     {STRING_WITH_LEN("char(93)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("created")},
     {STRING_WITH_LEN("timestamp")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("modified")},
     {STRING_WITH_LEN("timestamp")},
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
     {STRING_WITH_LEN("text")},
     {STRING_WITH_LEN("utf8")}},
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

static const TABLE_FIELD_DEF proc_table_def = {MYSQL_PROC_FIELD_COUNT,
                                               proc_table_fields};

/**
  Column definitions for 5.7 mysql.proc table (before 5.7.13).
*/

static const TABLE_FIELD_TYPE proc_table_fields_old[MYSQL_PROC_FIELD_COUNT] = {
    {{STRING_WITH_LEN("db")},
     {STRING_WITH_LEN("char(64)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("name")},
     {STRING_WITH_LEN("char(64)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("type")},
     {STRING_WITH_LEN("enum('FUNCTION','PROCEDURE')")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("specific_name")},
     {STRING_WITH_LEN("char(64)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("language")},
     {STRING_WITH_LEN("enum('SQL')")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("sql_data_access")},
     {STRING_WITH_LEN(
         "enum('CONTAINS_SQL','NO_SQL','READS_SQL_DATA','MODIFIES_SQL_DATA')")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("is_deterministic")},
     {STRING_WITH_LEN("enum('YES','NO')")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("security_type")},
     {STRING_WITH_LEN("enum('INVOKER','DEFINER')")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("param_list")}, {STRING_WITH_LEN("blob")}, {nullptr, 0}},

    {{STRING_WITH_LEN("returns")}, {STRING_WITH_LEN("longblob")}, {nullptr, 0}},
    {{STRING_WITH_LEN("body")}, {STRING_WITH_LEN("longblob")}, {nullptr, 0}},
    {{STRING_WITH_LEN("definer")},
     {STRING_WITH_LEN("char(77)")},
     {STRING_WITH_LEN("utf8")}},
    {{STRING_WITH_LEN("created")},
     {STRING_WITH_LEN("timestamp")},
     {nullptr, 0}},
    {{STRING_WITH_LEN("modified")},
     {STRING_WITH_LEN("timestamp")},
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
     {STRING_WITH_LEN("text")},
     {STRING_WITH_LEN("utf8")}},
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

static const TABLE_FIELD_DEF proc_table_def_old = {MYSQL_PROC_FIELD_COUNT,
                                                   proc_table_fields_old};

/**
  Set st_sp_chistics for routines.
*/

static bool set_st_sp_chistics(THD *thd, TABLE *proc_table,
                               st_sp_chistics *chistics) {
  char *ptr;
  size_t length;
  char buff[65];
  String str(buff, sizeof(buff), &my_charset_bin);

  memset(chistics, 0, sizeof(st_sp_chistics));

  if ((ptr = get_field(thd->mem_root,
                       proc_table->field[MYSQL_PROC_FIELD_ACCESS])) == nullptr)
    return true;

  switch (ptr[0]) {
    case 'N':
      chistics->daccess = SP_NO_SQL;
      break;
    case 'C':
      chistics->daccess = SP_CONTAINS_SQL;
      break;
    case 'R':
      chistics->daccess = SP_READS_SQL_DATA;
      break;
    case 'M':
      chistics->daccess = SP_MODIFIES_SQL_DATA;
      break;
    default:
      chistics->daccess = SP_DEFAULT_ACCESS_MAPPING;
  }

  // Deterministic
  if ((ptr = get_field(thd->mem_root,
                       proc_table->field[MYSQL_PROC_FIELD_DETERMINISTIC])) ==
      nullptr)
    return true;

  chistics->detistic = (ptr[0] != 'N');

  // Security type
  if ((ptr = get_field(thd->mem_root,
                       proc_table->field[MYSQL_PROC_FIELD_SECURITY_TYPE])) ==
      nullptr)
    return true;

  chistics->suid = (ptr[0] == 'I' ? SP_IS_NOT_SUID : SP_IS_SUID);

  // Fetch SP/SF comment
  proc_table->field[MYSQL_PROC_FIELD_COMMENT]->val_str(&str, &str);

  ptr = nullptr;
  if ((length = str.length()))
    ptr = strmake_root(thd->mem_root, str.ptr(), length);
  chistics->comment.str = ptr;
  chistics->comment.length = length;

  return false;
}

/**
  This function migrate one SP/SF from mysql.proc to routines DD table.
  One record in mysql.proc is metadata for one SP/SF. This function
  parses one record to extract metadata required and store it in DD table.
*/

static bool migrate_routine_to_dd(THD *thd, TABLE *proc_table) {
  const char *params, *returns, *body, *definer;
  char *sp_db, *sp_name1;
  sp_head *sp = nullptr;
  enum_sp_type routine_type;
  LEX_USER user_info;

  // Fetch SP/SF name, datbase name, definer and type.
  if ((sp_db = get_field(thd->mem_root,
                         proc_table->field[MYSQL_PROC_FIELD_DB])) == nullptr)
    return true;

  if ((sp_name1 = get_field(
           thd->mem_root, proc_table->field[MYSQL_PROC_FIELD_NAME])) == nullptr)
    return true;

  if ((definer = get_field(thd->mem_root,
                           proc_table->field[MYSQL_PROC_FIELD_DEFINER])) ==
      nullptr)
    return true;

  routine_type =
      (enum_sp_type)proc_table->field[MYSQL_PROC_MYSQL_TYPE]->val_int();

  // Fetch SP/SF parameters string
  if ((params = get_field(thd->mem_root,
                          proc_table->field[MYSQL_PROC_FIELD_PARAM_LIST])) ==
      nullptr)
    params = "";

  // Create return type string for SF
  if (routine_type == enum_sp_type::PROCEDURE)
    returns = "";
  else if ((returns = get_field(thd->mem_root,
                                proc_table->field[MYSQL_PROC_FIELD_RETURNS])) ==
           nullptr)
    return true;

  st_sp_chistics chistics;
  if (set_st_sp_chistics(thd, proc_table, &chistics)) return true;

  // Fetch SP/SF created and modified timestamp
  longlong created = proc_table->field[MYSQL_PROC_FIELD_CREATED]->val_int();
  longlong modified = proc_table->field[MYSQL_PROC_FIELD_MODIFIED]->val_int();

  // Fetch SP/SF body
  if ((body = get_field(thd->mem_root,
                        proc_table->field[MYSQL_PROC_FIELD_BODY])) == nullptr)
    return true;

  dd::upgrade::Routine_event_context_guard routine_ctx_guard(thd);

  thd->variables.sql_mode =
      (sql_mode_t)(proc_table->field[MYSQL_PROC_FIELD_SQL_MODE]->val_int() &
                   MODE_ALLOWED_MASK);

  LEX_CSTRING sp_db_str;
  LEX_STRING sp_name_str;

  sp_db_str.str = sp_db;
  sp_db_str.length = strlen(sp_db);
  sp_name_str.str = sp_name1;
  sp_name_str.length = strlen(sp_name1);

  sp_name sp_name_obj = sp_name(sp_db_str, sp_name_str, true);
  sp_name_obj.init_qname(thd);

  // Create SP creation context to be used in db_load_routine()
  Stored_program_creation_ctx *creation_ctx =
      Stored_routine_creation_ctx::load_from_db(thd, &sp_name_obj, proc_table);

  /*
    Update character set info in thread variable.
    Restore will be taken care by Routine_event_context_guard
  */
  thd->variables.character_set_client = creation_ctx->get_client_cs();
  thd->variables.collation_connection = creation_ctx->get_connection_cl();
  thd->update_charset();

  // Holders for user name and host name used in parse user.
  char definer_user_name_holder[USERNAME_LENGTH + 1];
  char definer_host_name_holder[HOSTNAME_LENGTH + 1];
  memset(&user_info, 0, sizeof(LEX_USER));
  user_info.user = {definer_user_name_holder, USERNAME_LENGTH};
  user_info.host = {definer_host_name_holder, HOSTNAME_LENGTH};

  // Parse user string to separate user name and host
  parse_user(definer, strlen(definer), definer_user_name_holder,
             &user_info.user.length, definer_host_name_holder,
             &user_info.host.length);

  // Disable autocommit option in thd variable
  Disable_autocommit_guard autocommit_guard(thd);

  // This function fixes sp_head to use in sp_create_routine()
  if (db_load_routine(thd, routine_type, sp_db_str.str, sp_db_str.length,
                      sp_name_str.str, sp_name_str.length, &sp,
                      thd->variables.sql_mode, params, returns, body, &chistics,
                      definer_user_name_holder, definer_host_name_holder,
                      created, modified, creation_ctx)) {
    /*
      Parsing of routine body failed. Use empty routine body and report a
      warning if the routine does not belong to sys schema. Sys schema routines
      will get fixed when mysql_upgrade is executed.
    */
    if (strcmp(sp_db_str.str, "sys") != 0) {
      if (dd::upgrade::Syntax_error_handler::is_parse_error) {
        LogErr(ERROR_LEVEL, ER_UPGRADE_PARSE_ERROR, "Routine", sp_db_str.str,
               sp_name_str.str,
               dd::upgrade::Syntax_error_handler::error_message());
        return false;
      }
      LogErr(WARNING_LEVEL, ER_CANT_PARSE_STORED_ROUTINE_BODY, sp_db_str.str,
             sp_name_str.str, " Creating routine without parsing routine body");
    }

    LEX_CSTRING sr_body;
    if (routine_type == enum_sp_type::FUNCTION)
      sr_body = {STRING_WITH_LEN("RETURN NULL")};
    else
      sr_body = {STRING_WITH_LEN("BEGIN END")};

    if (db_load_routine(
            thd, routine_type, sp_db_str.str, sp_db_str.length, sp_name_str.str,
            sp_name_str.length, &sp, thd->variables.sql_mode, params, returns,
            sr_body.str, &chistics, definer_user_name_holder,
            definer_host_name_holder, created, modified, creation_ctx))
      goto err;

    // Set actual routine body.
    sp->m_body.str = body;
    sp->m_body.length = strlen(body);
  }

  // Create entry for SP/SF in DD table.
  if (sp_create_routine(thd, sp, &user_info)) goto err;

  if (sp != nullptr)  // To be safe
    sp_head::destroy(sp);

  return false;

err:
  LogErr(ERROR_LEVEL, ER_DD_CANT_CREATE_SP, sp_db_str.str, sp_name_str.str);
  if (sp != nullptr)  // To be safe
    sp_head::destroy(sp);
  return true;
}

/**
  Migrate Stored Procedure and Functions
  from mysql.proc to routines dd table.
*/

bool migrate_routines_to_dd(THD *thd) {
  TABLE *proc_table;
  int error = 0;
  uint flags = MYSQL_LOCK_IGNORE_TIMEOUT;
  DML_prelocking_strategy prelocking_strategy;
  MEM_ROOT records_mem_root;
  Thd_mem_root_guard root_guard(thd, &records_mem_root);

  TABLE_LIST tables("mysql", "proc", TL_READ);
  auto table_list = &tables;

  if (open_and_lock_tables(thd, table_list, flags, &prelocking_strategy)) {
    LogErr(ERROR_LEVEL, ER_CANT_OPEN_TABLE_MYSQL_PROC);
    return true;
  }

  proc_table = tables.table;
  proc_table->use_all_columns();

  if (table_intact.check(thd, proc_table, &proc_table_def)) {
    // Check with old format too before returning error
    if (table_intact.check(thd, proc_table, &proc_table_def_old)) {
      close_thread_tables(thd);
      return true;
    }
  }

  System_table_close_guard proc_table_guard(thd, proc_table);

  if (proc_table->file->ha_index_init(0, true)) {
    LogErr(ERROR_LEVEL, ER_CANT_READ_TABLE_MYSQL_PROC);
    return true;
  }

  // Read first record from mysql.proc table. Return if table is empty.
  if ((error = proc_table->file->ha_index_first(proc_table->record[0]))) {
    if (error == HA_ERR_END_OF_FILE) return false;
    LogErr(ERROR_LEVEL, ER_CANT_READ_TABLE_MYSQL_PROC);
    return true;
  }

  // Migrate first record read to dd routines table.
  if (migrate_routine_to_dd(thd, proc_table)) return true;

  // Read one record from mysql.proc table and
  // migrate it until all records are finished
  while (!(error = proc_table->file->ha_index_next(proc_table->record[0])) &&
         !dd::upgrade::Syntax_error_handler::has_too_many_errors()) {
    if (migrate_routine_to_dd(thd, proc_table)) return true;
  }

  if (error != HA_ERR_END_OF_FILE) {
    LogErr(ERROR_LEVEL, ER_CANT_READ_TABLE_MYSQL_PROC);
    return true;
  }

  return dd::upgrade::Syntax_error_handler::has_errors();
}

}  // namespace upgrade_57
}  // namespace dd
