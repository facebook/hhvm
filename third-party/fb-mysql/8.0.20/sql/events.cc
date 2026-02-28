/* Copyright (c) 2005, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/events.h"

#include <stdio.h>
#include <string.h>
#include <atomic>
#include <memory>
#include <new>
#include <string>
#include <utility>
#include <vector>

#include "lex_string.h"
#include "m_ctype.h"  // CHARSET_INFO
#include "m_string.h"
#include "my_dbug.h"
#include "my_loglevel.h"
#include "my_macros.h"
#include "my_sys.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/components/services/psi_memory_bits.h"
#include "mysql/psi/mysql_cond.h"
#include "mysql/psi/mysql_memory.h"
#include "mysql/psi/mysql_mutex.h"
#include "mysql/psi/mysql_sp.h"
#include "mysql/psi/mysql_stage.h"
#include "mysql/psi/mysql_thread.h"
#include "mysql/psi/psi_base.h"
#include "mysql_com.h"
#include "mysqld_error.h"  // ER_*
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"  // EVENT_ACL
#include "sql/dd/cache/dictionary_client.h"
#include "sql/dd/dd_schema.h"  // dd::Schema_MDL_locker
#include "sql/dd/string_type.h"
#include "sql/dd/types/event.h"
#include "sql/dd/types/schema.h"
#include "sql/debug_sync.h"
#include "sql/event_data_objects.h"   // Event_queue_element
#include "sql/event_db_repository.h"  // Event_db_repository
#include "sql/event_parse_data.h"     // Event_parse_data
#include "sql/event_queue.h"          // Event_queue
#include "sql/event_scheduler.h"      // Event_scheduler
#include "sql/item.h"
#include "sql/lock.h"  // lock_object_name
#include "sql/log.h"
#include "sql/mdl.h"
#include "sql/mysqld.h"  // LOCK_global_system_variables
#include "sql/protocol.h"
#include "sql/psi_memory_key.h"
#include "sql/set_var.h"
#include "sql/sp_head.h"    // Stored_program_creation_ctx
#include "sql/sql_class.h"  // THD
#include "sql/sql_const.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_show.h"   // append_definer
#include "sql/sql_table.h"  // write_bin_log
#include "sql/system_variables.h"
#include "sql/table.h"
#include "sql/thd_raii.h"
#include "sql/transaction.h"
#include "sql/tztime.h"  // Time_zone
#include "sql_string.h"  // String

/**
  @addtogroup Event_Scheduler
  @{
*/

/*
 TODO list :
 - CREATE EVENT should not go into binary log! Does it now? The SQL statements
   issued by the EVENT are replicated.
   I have an idea how to solve the problem at failover. So the status field
   will be ENUM('DISABLED', 'ENABLED', 'SLAVESIDE_DISABLED').
   In this case when CREATE EVENT is replicated it should go into the binary
   as SLAVESIDE_DISABLED if it is ENABLED, when it's created as DISABLEd it
   should be replicated as disabled. If an event is ALTERed as DISABLED the
   query should go untouched into the binary log, when ALTERed as enable then
   it should go as SLAVESIDE_DISABLED. This is regarding the SQL interface.
   TT routines however modify mysql.event internally and this does not go the
   log so in this case queries has to be injected into the log...somehow... or
   maybe a solution is RBR for this case, because the event may go only from
   ENABLED to DISABLED status change and this is safe for replicating. As well
   an event may be deleted which is also safe for RBR.

 - Add logging to file

*/

/*
  If the user (un)intentionally removes an event directly from mysql.event
  the following sequence has to be used to be able to remove the in-memory
  counterpart.
  1. CREATE EVENT the_name ON SCHEDULE EVERY 1 SECOND DISABLE DO SELECT 1;
  2. DROP EVENT the_name

  In other words, the first one will create a row in mysql.event . In the
  second step because there will be a line, disk based drop will pass and
  the scheduler will remove the memory counterpart. The reason is that
  in-memory queue does not check whether the event we try to drop from memory
  is disabled. Disabled events are not kept in-memory because they are not
  eligible for execution.
*/

Event_queue *Events::event_queue;
Event_scheduler *Events::scheduler;
ulong Events::opt_event_scheduler = Events::EVENTS_OFF;

static bool load_events_from_db(THD *thd, Event_queue *event_queue);

/*
  Compares 2 LEX strings regarding case.

  SYNOPSIS
    sortcmp_lex_string()
      s   First LEX_STRING
      t   Second LEX_STRING
      cs  Charset

  RETURN VALUE
   -1   s < t
    0   s == t
    1   s > t
*/

int sortcmp_lex_string(LEX_CSTRING s, LEX_CSTRING t, CHARSET_INFO *cs) {
  return cs->coll->strnncollsp(cs, pointer_cast<const uchar *>(s.str), s.length,
                               pointer_cast<const uchar *>(t.str), t.length);
}

/*
  Reconstructs interval expression from interval type and expression
  value that is in form of a value of the smalles entity:
  For
    YEAR_MONTH - expression is in months
    DAY_MINUTE - expression is in minutes

  SYNOPSIS
    Events::reconstruct_interval_expression()
      buf         Preallocated String buffer to add the value to
      interval    The interval type (for instance YEAR_MONTH)
      expression  The value in the lowest entity

  RETURN VALUE
    0  OK
    1  Error
*/

int Events::reconstruct_interval_expression(String *buf, interval_type interval,
                                            longlong expression) {
  ulonglong expr = expression;
  char tmp_buff[128], *end;
  bool close_quote = true;
  int multipl = 0;
  char separator = ':';

  switch (interval) {
    case INTERVAL_YEAR_MONTH:
      multipl = 12;
      separator = '-';
      goto common_1_lev_code;
    case INTERVAL_DAY_HOUR:
      multipl = 24;
      separator = ' ';
      goto common_1_lev_code;
    case INTERVAL_HOUR_MINUTE:
    case INTERVAL_MINUTE_SECOND:
      multipl = 60;
    common_1_lev_code:
      buf->append('\'');
      end = longlong10_to_str(expression / multipl, tmp_buff, 10);
      buf->append(tmp_buff, (uint)(end - tmp_buff));
      expr = expr - (expr / multipl) * multipl;
      break;
    case INTERVAL_DAY_MINUTE: {
      ulonglong tmp_expr = expr;

      tmp_expr /= (24 * 60);
      buf->append('\'');
      end = longlong10_to_str(tmp_expr, tmp_buff, 10);
      buf->append(tmp_buff, (uint)(end - tmp_buff));  // days
      buf->append(' ');

      tmp_expr = expr - tmp_expr * (24 * 60);  // minutes left
      end = longlong10_to_str(tmp_expr / 60, tmp_buff, 10);
      buf->append(tmp_buff, (uint)(end - tmp_buff));  // hours

      expr = tmp_expr - (tmp_expr / 60) * 60;
      /* the code after the switch will finish */
    } break;
    case INTERVAL_HOUR_SECOND: {
      ulonglong tmp_expr = expr;

      buf->append('\'');
      end = longlong10_to_str(tmp_expr / 3600, tmp_buff, 10);
      buf->append(tmp_buff, (uint)(end - tmp_buff));  // hours
      buf->append(':');

      tmp_expr = tmp_expr - (tmp_expr / 3600) * 3600;
      end = longlong10_to_str(tmp_expr / 60, tmp_buff, 10);
      buf->append(tmp_buff, (uint)(end - tmp_buff));  // minutes

      expr = tmp_expr - (tmp_expr / 60) * 60;
      /* the code after the switch will finish */
    } break;
    case INTERVAL_DAY_SECOND: {
      ulonglong tmp_expr = expr;

      tmp_expr /= (24 * 3600);
      buf->append('\'');
      end = longlong10_to_str(tmp_expr, tmp_buff, 10);
      buf->append(tmp_buff, (uint)(end - tmp_buff));  // days
      buf->append(' ');

      tmp_expr = expr - tmp_expr * (24 * 3600);  // seconds left
      end = longlong10_to_str(tmp_expr / 3600, tmp_buff, 10);
      buf->append(tmp_buff, (uint)(end - tmp_buff));  // hours
      buf->append(':');

      tmp_expr = tmp_expr - (tmp_expr / 3600) * 3600;
      end = longlong10_to_str(tmp_expr / 60, tmp_buff, 10);
      buf->append(tmp_buff, (uint)(end - tmp_buff));  // minutes

      expr = tmp_expr - (tmp_expr / 60) * 60;
      /* the code after the switch will finish */
    } break;
    case INTERVAL_DAY_MICROSECOND:
    case INTERVAL_HOUR_MICROSECOND:
    case INTERVAL_MINUTE_MICROSECOND:
    case INTERVAL_SECOND_MICROSECOND:
    case INTERVAL_MICROSECOND:
      my_error(ER_NOT_SUPPORTED_YET, MYF(0), "MICROSECOND");
      return 1;
      break;
    case INTERVAL_QUARTER:
      expr /= 3;
      close_quote = false;
      break;
    case INTERVAL_WEEK:
      expr /= 7;
      close_quote = false;
      break;
    default:
      close_quote = false;
      break;
  }
  if (close_quote) buf->append(separator);
  end = longlong10_to_str(expr, tmp_buff, 10);
  buf->append(tmp_buff, (uint)(end - tmp_buff));
  if (close_quote) buf->append('\'');

  return 0;
}

/**
  Create a new query string for removing executable comments
  for avoiding leak and keeping consistency of the execution
  on master and slave.

  @param[in] thd                 Thread handler
  @param[in] buf                 Query string

  @return
             0           ok
             1           error
*/
static int create_query_string(THD *thd, String *buf) {
  /* Append the "CREATE" part of the query */
  if (buf->append(STRING_WITH_LEN("CREATE "))) return 1;
  /* Append definer */
  append_definer(thd, buf, thd->lex->definer->user, thd->lex->definer->host);
  /* Append the left part of thd->query after "DEFINER" part */
  if (buf->append(
          thd->lex->stmt_definition_begin,
          thd->lex->stmt_definition_end - thd->lex->stmt_definition_begin))
    return 1;

  return 0;
}

/**
  Create a new event.

  Atomicity:
    The operation to create an event is atomic/crash-safe.
    Changes to the Data-dictionary and writing event to binlog are
    part of the same transaction. All the changes are done as part
    of the same transaction or do not have any side effects on the
    operation failure. Data-dictionary cache and event queues are
    in sync with operation state. Cache and event queue does
    not contain any stale/incorrect data in case of failure.
    In case of crash, there won't be any discrepancy between
    the data-dictionary table and the binary log.

  @param[in,out]  thd            THD
  @param[in]      parse_data     Event's data from parsing stage
  @param[in]      if_not_exists  Whether IF NOT EXISTS was
                                 specified
  In case there is an event with the same name (db) and
  IF NOT EXISTS is specified, an warning is put into the stack.
  @sa Events::drop_event for the notes about locking, pre-locking
  and Events DDL.

  @retval  false  OK
  @retval  true   Error (reported)
*/

bool Events::create_event(THD *thd, Event_parse_data *parse_data,
                          bool if_not_exists) {
  bool event_already_exists;
  bool event_added_to_event_queue = false;
  std::unique_ptr<Event_queue_element> new_element(nullptr);
  DBUG_TRACE;

  DBUG_EXECUTE_IF("thd_killed_injection", thd->killed = THD::KILL_QUERY;
                  return false;);

  /*
    Perform semantic checks outside of Event_db_repository:
    once CREATE EVENT is supported in prepared statements, the
    checks will be moved to PREPARE phase.
  */
  if (parse_data->check_parse_data(thd)) return true;

  /* At create, one of them must be set */
  DBUG_ASSERT(parse_data->expression || parse_data->execute_at);

  if (check_access(thd, EVENT_ACL, parse_data->dbname.str, nullptr, nullptr,
                   false, false))
    return true;

  // Acquire exclusive MDL lock.
  if (lock_object_name(thd, MDL_key::EVENT, parse_data->dbname.str,
                       parse_data->name.str))
    return true;

  if (parse_data->do_not_create) return false;
  /*
    Turn off row binlogging of this statement and use statement-based
    so that all supporting tables are updated for CREATE EVENT command.
    When we are going out of the function scope, the original binary
    format state will be restored.
  */
  Save_and_Restore_binlog_format_state binlog_format_state(thd);

  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  if (Event_db_repository::create_event(thd, parse_data, if_not_exists,
                                        &event_already_exists)) {
    /* On error conditions my_error() is called so no need to handle here */
    goto err_with_rollback;
  }

  // Create event queue element.
  if (opt_event_scheduler != Events::EVENTS_DISABLED && !event_already_exists) {
    new_element.reset(new Event_queue_element());
    if (!new_element) {
      my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), sizeof(Event_queue_element));
      goto err_with_rollback;
    }

    if (Event_db_repository::load_named_event(
            thd, parse_data->dbname, parse_data->name, new_element.get()))
      goto err_with_rollback;

    // Add new event queue element in the events queue.
    if (event_queue && event_queue->create_event(thd, new_element.get(),
                                                 &event_added_to_event_queue)) {
      my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), 0);
      goto err_with_rollback;
    }

    // Release new_element ownership if it is added to the event queue.
    if (event_added_to_event_queue) new_element.release();
  }

  // Binlog the create event.
  {
    DBUG_ASSERT(thd->query().str && thd->query().length);
    String log_query;
    if (create_query_string(thd, &log_query)) {
      LogErr(ERROR_LEVEL, ER_EVENT_ERROR_CREATING_QUERY_TO_WRITE_TO_BINLOG);
      goto err_with_rollback;
    } else {
      thd->add_to_binlog_accessed_dbs(parse_data->dbname.str);
      /*
        If the definer is not set or set to CURRENT_USER, the value of
        CURRENT_USER will be written into the binary log as the definer for the
        SQL thread.
      */
      if (write_bin_log(thd, true, log_query.c_ptr(), log_query.length(),
                        !event_already_exists))
        goto err_with_rollback;
    }
  }

  // Commit changes to the data-dictionary and binary log.
  if (DBUG_EVALUATE_IF("simulate_create_event_failure", true, false) ||
      trans_commit_stmt(thd) || trans_commit(thd))
    goto err_with_rollback;

  return false;

err_with_rollback:
  DBUG_EXECUTE_IF("simulate_create_event_failure",
                  my_error(ER_UNKNOWN_ERROR, MYF(0)););

  if (event_added_to_event_queue) {
    // Event element is dellocated by the drop_event() method.
    event_queue->drop_event(thd, parse_data->dbname, parse_data->name);
  }

  trans_rollback_stmt(thd);
  /*
    Full rollback in case we have THD::transaction_rollback_request
    and to synchronize DD state in cache and on disk (as statement
    rollback doesn't clear DD cache of modified uncommitted objects).
  */
  trans_rollback(thd);

  return true;
}

/**
  Alter an event.

  Atomicity:
    The operation to update an event is atomic/crash-safe.
    Changes to the Data-dictionary and writing event to binlog are
    part of the same transaction. All the changes are done as part
    of the same transaction or do not have any side effects on the
    operation failure. Data-dictionary cache and event queues are
    in sync with operation state. Cache and event queue does
    not contain any stale/incorrect data in case of failure.
    In case of crash, there won't be any discrepancy between
    the data-dictionary table and the binary log.

  @param[in,out] thd         THD
  @param[in]     parse_data  Event's data from parsing stage
  @param[in]     new_dbname  A new schema name for the event. Set in the case of
                             ALTER EVENT RENAME, otherwise is NULL.
  @param[in]     new_name    A new name for the event. Set in the case of
                             ALTER EVENT RENAME

  Parameter 'et' contains data about dbname and event name.
  Parameter 'new_name' is the new name of the event, if not null
  this means that RENAME TO was specified in the query
  @sa Events::drop_event for the locking notes.

  @retval  false  OK
  @retval  true   error (reported)
*/

bool Events::update_event(THD *thd, Event_parse_data *parse_data,
                          const LEX_CSTRING *new_dbname,
                          const LEX_CSTRING *new_name) {
  std::unique_ptr<Event_queue_element> new_element(nullptr);

  DBUG_TRACE;

  if (parse_data->check_parse_data(thd) || parse_data->do_not_create)
    return true;

  if (check_access(thd, EVENT_ACL, parse_data->dbname.str, nullptr, nullptr,
                   false, false))
    return true;

  if (lock_object_name(thd, MDL_key::EVENT, parse_data->dbname.str,
                       parse_data->name.str))
    return true;

  if (new_dbname != nullptr) /* It's a rename */
  {
    /* Check that the new and the old names differ. */
    if (!sortcmp_lex_string(parse_data->dbname, *new_dbname,
                            system_charset_info) &&
        !sortcmp_lex_string(parse_data->name, *new_name, system_charset_info)) {
      my_error(ER_EVENT_SAME_NAME, MYF(0));
      return true;
    }

    /*
      And the user has sufficient privileges to use the target database.
      Do it before checking whether the database exists: we don't want
      to tell the user that a database doesn't exist if they can not
      access it.
    */
    if (check_access(thd, EVENT_ACL, new_dbname->str, nullptr, nullptr, false,
                     false))
      return true;

    //  Acquire mdl exclusive lock on target database name.
    if (lock_object_name(thd, MDL_key::EVENT, new_dbname->str, new_name->str))
      return true;
  }

  /*
    Turn off row binlogging of this statement and use statement-based
    so that all supporting tables are updated for CREATE EVENT command.
    When we are going out of the function scope, the original binary
    format state will be restored.
  */
  Save_and_Restore_binlog_format_state binlog_format_state(thd);

  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  if (Event_db_repository::update_event(thd, parse_data, new_dbname,
                                        new_name)) {
    /* On error conditions my_error() is called so no need to handle here */
    goto err_with_rollback;
  }

  if (opt_event_scheduler != Events::EVENTS_DISABLED) {
    // Create event queue element.
    new_element.reset(new Event_queue_element());
    if (!new_element) {
      my_error(ER_OUTOFMEMORY, MYF(ME_FATALERROR), sizeof(Event_queue_element));
      goto err_with_rollback;
    }

    LEX_CSTRING dbname = new_dbname ? *new_dbname : parse_data->dbname;
    LEX_CSTRING name = new_name ? *new_name : parse_data->name;
    if (Event_db_repository::load_named_event(thd, dbname, name,
                                              new_element.get()))
      goto err_with_rollback;
  }

  /* Binlog the alter event. */
  {
    DBUG_ASSERT(thd->query().str && thd->query().length);

    thd->add_to_binlog_accessed_dbs(parse_data->dbname.str);
    if (new_dbname) thd->add_to_binlog_accessed_dbs(new_dbname->str);

    if (write_bin_log(thd, true, thd->query().str, thd->query().length, true))
      goto err_with_rollback;
  }

  // Commit changes to the data-dictionary and binary log.
  if (DBUG_EVALUATE_IF("simulate_alter_event_failure", true, false) ||
      trans_commit_stmt(thd) || trans_commit(thd))
    goto err_with_rollback;

  // Update element in event queue.
  if (event_queue && new_element != nullptr) {
    /*
      TODO: check if an update actually has inserted an entry into the queue.
            If not, and the element is ON COMPLETION NOT PRESERVE, delete
            it right away.
    */
    event_queue->update_event(thd, parse_data->dbname, parse_data->name,
                              new_element.get());
    new_element.release();
  }

  return false;

err_with_rollback:
  DBUG_EXECUTE_IF("simulate_alter_event_failure",
                  my_error(ER_UNKNOWN_ERROR, MYF(0)););

  trans_rollback_stmt(thd);
  /*
    Full rollback in case we have THD::transaction_rollback_request
    and to synchronize DD state in cache and on disk (as statement
    rollback doesn't clear DD cache of modified uncommitted objects).
  */
  trans_rollback(thd);

  return true;
}

/**
  Drops an event

  Atomicity:
    The operation to drop an event is atomic/crash-safe.
    Changes to the Data-dictionary and writing event to binlog are
    part of the same transaction. All the changes are done as part
    of the same transaction or do not have any side effects on the
    operation failure. Data-dictionary cache and event queues are
    in sync with operation state. Cache and event queue does
    not contain any stale/incorrect data in case of failure.
    In case of crash, there won't be any discrepancy between
    the data-dictionary table and the binary log.

  @param[in,out]  thd        THD
  @param[in]      dbname     Event's schema
  @param[in]      name       Event's name
  @param[in]      if_exists  When this is set and the event does not exist
                             a warning is pushed into the warning stack.
                             Otherwise the operation produces an error.

  @note Similarly to DROP PROCEDURE, we do not allow DROP EVENT
  under LOCK TABLES mode, unless table mysql.event is locked.  To
  ensure that, we do not reset & backup the open tables state in
  this function - if in LOCK TABLES or pre-locking mode, this will
  lead to an error 'Table mysql.event is not locked with LOCK
  TABLES' unless it _is_ locked. In pre-locked mode there is
  another barrier - DROP EVENT commits the current transaction,
  and COMMIT/ROLLBACK is not allowed in stored functions and
  triggers.

  @retval  false  OK
  @retval  true   Error (reported)
*/

bool Events::drop_event(THD *thd, LEX_CSTRING dbname, LEX_CSTRING name,
                        bool if_exists) {
  DBUG_TRACE;

  if (check_access(thd, EVENT_ACL, dbname.str, nullptr, nullptr, false, false))
    return true;

  // Acquire exclusive MDL lock.
  if (lock_object_name(thd, MDL_key::EVENT, dbname.str, name.str)) return true;

  DEBUG_SYNC(thd, "after_acquiring_exclusive_lock_on_the_event");

  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  bool event_exists;
  if (Event_db_repository::drop_event(thd, dbname, name, if_exists,
                                      &event_exists)) {
    /* On error conditions my_error() is called so no need to handle here */
    goto err_with_rollback;
  }

  // Binlog the drop event.
  {
    DBUG_ASSERT(thd->query().str && thd->query().length);

    thd->add_to_binlog_accessed_dbs(dbname.str);
    if (write_bin_log(thd, true, thd->query().str, thd->query().length,
                      event_exists))
      goto err_with_rollback;
  }

  // Commit changes to the data-dictionary and binary log.
  if (DBUG_EVALUATE_IF("simulate_drop_event_failure", true, false) ||
      trans_commit_stmt(thd) || trans_commit(thd))
    goto err_with_rollback;

  if (event_queue) event_queue->drop_event(thd, dbname, name);

#ifdef HAVE_PSI_SP_INTERFACE
  /* Drop statistics for this stored program from performance schema. */
  MYSQL_DROP_SP(to_uint(enum_sp_type::EVENT), dbname.str, dbname.length,
                name.str, name.length);
#endif

  return false;

err_with_rollback:
  DBUG_EXECUTE_IF("simulate_drop_event_failure",
                  my_error(ER_UNKNOWN_ERROR, MYF(0)););

  trans_rollback_stmt(thd);
  /*
    Full rollback in case we have THD::transaction_rollback_request
    and to synchronize DD state in cache and on disk (as statement
    rollback doesn't clear DD cache of modified uncommitted objects).
  */
  trans_rollback(thd);

  return true;
}

/**
  Take exclusive metadata lock on all events in a schema.

  @param   thd     Thread handle.
  @param   schema  Schema object.
*/

bool Events::lock_schema_events(THD *thd, const dd::Schema &schema) {
  DBUG_TRACE;

  std::vector<dd::String_type> event_names;
  if (thd->dd_client()->fetch_schema_component_names<dd::Event>(&schema,
                                                                &event_names))
    return true;

  /*
    If lower_case_table_names == 2 then schema names should be lower cased for
    proper hash key comparisons.
  */
  const char *schema_name = schema.name().c_str();
  char schema_name_buf[NAME_LEN + 1];
  if (lower_case_table_names == 2) {
    my_stpcpy(schema_name_buf, schema_name);
    my_casedn_str(system_charset_info, schema_name_buf);
    schema_name = schema_name_buf;
  }

  MDL_request_list mdl_requests;
  for (std::vector<dd::String_type>::const_iterator name = event_names.begin();
       name != event_names.end(); ++name) {
    MDL_key mdl_key;
    dd::Event::create_mdl_key(dd::String_type(schema_name), *name, &mdl_key);

    // Add MDL_request for routine to mdl_requests list.
    MDL_request *mdl_request = new (thd->mem_root) MDL_request;
    MDL_REQUEST_INIT_BY_KEY(mdl_request, &mdl_key, MDL_EXCLUSIVE,
                            MDL_TRANSACTION);
    mdl_requests.push_front(mdl_request);
  }

  return thd->mdl_context.acquire_locks_nsec(
      &mdl_requests, thd->variables.lock_wait_timeout_nsec);
}

/**
  Drops all events from a schema

  @note We allow to drop all events in a schema even if the
  scheduler is disabled. This is to not produce any warnings
  in case of DROP DATABASE and a disabled scheduler.

  @param[in]      thd     THD handle.
  @param[in]      schema  Schema object.

  @returns true   drop events from database failed.
  @returns false  drop events from database succeeded.
*/
bool Events::drop_schema_events(THD *thd, const dd::Schema &schema) {
  LEX_CSTRING db_lex = {schema.name().c_str(), schema.name().length()};

  if (event_queue) event_queue->drop_schema_events(db_lex);

  return Event_db_repository::drop_schema_events(thd, schema);
}

/**
  A helper function to generate SHOW CREATE EVENT output from a named event.

  @param[in]    thd      THD handle.
  @param[in]    et       Pointer to Event_timed object.
  @param[in]    protocol Pointer to Protocol object.
*/
static bool send_show_create_event(THD *thd, Event_timed *et,
                                   Protocol *protocol) {
  char show_str_buf[10 * STRING_BUFFER_USUAL_SIZE];
  String show_str(show_str_buf, sizeof(show_str_buf), system_charset_info);
  List<Item> field_list;
  LEX_STRING sql_mode;
  const String *tz_name;

  DBUG_TRACE;
  show_str.length(0);

  if (et->get_create_event(thd, &show_str)) return true;

  field_list.push_back(new Item_empty_string("Event", NAME_CHAR_LEN));

  if (sql_mode_string_representation(thd, et->m_sql_mode, &sql_mode))
    return true;

  field_list.push_back(
      new Item_empty_string("sql_mode", (uint)sql_mode.length));

  tz_name = et->m_time_zone->get_name();

  field_list.push_back(new Item_empty_string("time_zone", tz_name->length()));

  field_list.push_back(
      new Item_empty_string("Create Event", show_str.length()));

  field_list.push_back(
      new Item_empty_string("character_set_client", MY_CS_NAME_SIZE));

  field_list.push_back(
      new Item_empty_string("collation_connection", MY_CS_NAME_SIZE));

  field_list.push_back(
      new Item_empty_string("Database Collation", MY_CS_NAME_SIZE));

  if (thd->send_result_metadata(&field_list,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF))
    return true;

  protocol->start_row();

  protocol->store_string(et->m_event_name.str, et->m_event_name.length,
                         system_charset_info);
  protocol->store_string(sql_mode.str, sql_mode.length, system_charset_info);
  protocol->store_string(tz_name->ptr(), tz_name->length(),
                         system_charset_info);
  protocol->store_string(show_str.c_ptr(), show_str.length(),
                         et->m_creation_ctx->get_client_cs());
  protocol->store_string(et->m_creation_ctx->get_client_cs()->csname,
                         strlen(et->m_creation_ctx->get_client_cs()->csname),
                         system_charset_info);
  protocol->store_string(et->m_creation_ctx->get_connection_cl()->name,
                         strlen(et->m_creation_ctx->get_connection_cl()->name),
                         system_charset_info);
  protocol->store_string(et->m_creation_ctx->get_db_cl()->name,
                         strlen(et->m_creation_ctx->get_db_cl()->name),
                         system_charset_info);

  if (protocol->end_row()) return true;

  my_eof(thd);

  return false;
}

/**
  Implement SHOW CREATE EVENT statement

      thd   Thread context
      spn   The name of the event (db, name)

  @retval  false  OK
  @retval  true   error (reported)
*/

bool Events::show_create_event(THD *thd, LEX_CSTRING dbname, LEX_CSTRING name) {
  Event_timed et;
  bool ret;

  DBUG_TRACE;
  DBUG_PRINT("enter", ("name: %s@%s", dbname.str, name.str));

  if (check_access(thd, EVENT_ACL, dbname.str, nullptr, nullptr, false, false))
    return true;

  // We must make sure the schema is released and unlocked in the right
  // order. Fail if we are unable to get a meta data lock on the schema
  // name.
  dd::Schema_MDL_locker mdl_handler(thd);
  if (mdl_handler.ensure_locked(dbname.str)) return true;

  // Grab MDL lock on object in shared mode.
  MDL_key mdl_key;
  dd::Event::create_mdl_key(dbname.str, name.str, &mdl_key);
  MDL_request event_mdl_request;
  MDL_REQUEST_INIT_BY_KEY(&event_mdl_request, &mdl_key, MDL_SHARED_HIGH_PRIO,
                          MDL_TRANSACTION);
  if (thd->mdl_context.acquire_lock_nsec(&event_mdl_request,
                                         thd->variables.lock_wait_timeout_nsec))
    return true;

  DEBUG_SYNC(thd, "after_acquiring_shared_lock_on_the_event");

  /*
    We would like to allow SHOW CREATE EVENT under LOCK TABLES and
    in pre-locked mode. mysql.event table is marked as a system table.
    This flag reduces the set of its participation scenarios in LOCK TABLES
    operation, and therefore an out-of-bound open of this table
    for reading like the one below (sic, only for reading) is
    more or less deadlock-free. For additional information about when a
    deadlock can occur please refer to the description of 'system table'
    flag.
  */
  ret = Event_db_repository::load_named_event(thd, dbname, name, &et);
  if (!ret) ret = send_show_create_event(thd, &et, thd->get_protocol());

  return ret;
}

/**
  Initializes the scheduler's structures.

  @param  opt_noacl_or_bootstrap
                     true if there is --skip-grant-tables or
                     --initialize. In that case we disable the event
                     scheduler.

  @note   This function is not synchronized.

  @retval  false   Perhaps there was an error, and the event scheduler
                   is disabled. But the error is not fatal and the
                   server start up can continue.
  @retval  true    Fatal error. Startup must terminate (call unireg_abort()).
*/

bool Events::init(bool opt_noacl_or_bootstrap) {
  THD *thd;
  int err_no;
  bool res = false;

  DBUG_TRACE;

  // If event scheduler was explicitly disabled from command-line, do not
  // initialize it.
  if (opt_event_scheduler == Events::EVENTS_DISABLED) return res;

  //  If run with --skip-grant-tables or --initialize, disable the event
  //  scheduler and return.
  if (opt_noacl_or_bootstrap) {
    opt_event_scheduler = Events::EVENTS_DISABLED;
    return res;
  }

  /*
    We need a temporary THD during boot

    Current time is stored in data member start_time of THD class
    and initialized by THD::set_time() called by ctor->THD::init()
    Subsequently, this value is used to check whether event was expired
    when make loading events from storage. Check for event expiration time
    is done at Event_queue_element::compute_next_execution_time() where
    event's status set to Event_parse_data::DISABLED and dropped flag set
    to true if event was expired.
  */
  if (!(thd = new THD())) {
    res = true;
    goto end;
  }
  /*
    The thread stack does not start from this function but we cannot
    guess the real value. So better some value that doesn't assert than
    no value.
  */
  thd->thread_stack = (char *)&thd;
  thd->store_globals();

  DBUG_ASSERT(opt_event_scheduler == Events::EVENTS_ON ||
              opt_event_scheduler == Events::EVENTS_OFF);

  if (!(event_queue = new Event_queue) ||
      !(scheduler = new Event_scheduler(event_queue))) {
    res = true; /* fatal error: request unireg_abort */
    goto end;
  }

  if (event_queue->init_queue() || load_events_from_db(thd, event_queue) ||
      (opt_event_scheduler == EVENTS_ON && scheduler->start(&err_no))) {
    LogErr(ERROR_LEVEL, ER_EVENT_SCHEDULER_ERROR_LOADING_FROM_DB);
    res = true; /* fatal error: request unireg_abort */
    goto end;
  }

end:
  if (res) {
    delete event_queue;
    event_queue = nullptr;
    delete scheduler;
    scheduler = nullptr;
  }
  delete thd;

  return res;
}

/*
  Cleans up scheduler's resources. Called at server shutdown.

  SYNOPSIS
    Events::deinit()

  NOTES
    This function is not synchronized.
*/

void Events::deinit() {
  DBUG_TRACE;

  if (opt_event_scheduler != EVENTS_DISABLED) {
    delete scheduler;
    scheduler = nullptr; /* safety */
    delete event_queue;
    event_queue = nullptr; /* safety */
  }
}

#ifdef HAVE_PSI_INTERFACE
PSI_mutex_key key_LOCK_event_queue, key_event_scheduler_LOCK_scheduler_state;

/* clang-format off */
static PSI_mutex_info all_events_mutexes[]=
{
  { &key_LOCK_event_queue, "LOCK_event_queue", PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME},
  { &key_event_scheduler_LOCK_scheduler_state, "Event_scheduler::LOCK_scheduler_state", PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME}
};
/* clang-format on */

PSI_cond_key key_event_scheduler_COND_state, key_COND_queue_state;

static PSI_cond_info all_events_conds[] = {
    {&key_event_scheduler_COND_state, "Event_scheduler::COND_state",
     PSI_FLAG_SINGLETON, 0, PSI_DOCUMENT_ME},
    {&key_COND_queue_state, "COND_queue_state", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME},
};

PSI_thread_key key_thread_event_scheduler, key_thread_event_worker;

static PSI_thread_info all_events_threads[] = {
    {&key_thread_event_scheduler, "event_scheduler", PSI_FLAG_SINGLETON, 0,
     PSI_DOCUMENT_ME},
    {&key_thread_event_worker, "event_worker", 0, 0, PSI_DOCUMENT_ME}};
#endif /* HAVE_PSI_INTERFACE */

PSI_stage_info stage_waiting_on_empty_queue = {0, "Waiting on empty queue", 0,
                                               PSI_DOCUMENT_ME};
PSI_stage_info stage_waiting_for_next_activation = {
    0, "Waiting for next activation", 0, PSI_DOCUMENT_ME};
PSI_stage_info stage_waiting_for_scheduler_to_stop = {
    0, "Waiting for the scheduler to stop", 0, PSI_DOCUMENT_ME};

PSI_memory_key key_memory_event_basic_root;

#ifdef HAVE_PSI_INTERFACE
PSI_stage_info *all_events_stages[] = {&stage_waiting_on_empty_queue,
                                       &stage_waiting_for_next_activation,
                                       &stage_waiting_for_scheduler_to_stop};

static PSI_memory_info all_events_memory[] = {
    {&key_memory_event_basic_root, "Event_basic::mem_root",
     PSI_FLAG_ONLY_GLOBAL_STAT, 0, PSI_DOCUMENT_ME}};

static void init_events_psi_keys(void) {
  const char *category = "sql";
  int count;

  count = static_cast<int>(array_elements(all_events_mutexes));
  mysql_mutex_register(category, all_events_mutexes, count);

  count = static_cast<int>(array_elements(all_events_conds));
  mysql_cond_register(category, all_events_conds, count);

  count = static_cast<int>(array_elements(all_events_threads));
  mysql_thread_register(category, all_events_threads, count);

  count = static_cast<int>(array_elements(all_events_stages));
  mysql_stage_register(category, all_events_stages, count);

  count = static_cast<int>(array_elements(all_events_memory));
  mysql_memory_register(category, all_events_memory, count);
}
#endif /* HAVE_PSI_INTERFACE */

/*
  Inits Events mutexes

  SYNOPSIS
    Events::init_mutexes()
      thd  Thread
*/

void Events::init_mutexes() {
#ifdef HAVE_PSI_INTERFACE
  init_events_psi_keys();
#endif
}

/*
  Dumps the internal status of the scheduler and the memory cache
  into a table with two columns - Name & Value. Different properties
  which could be useful for debugging for instance deadlocks are
  returned.

  SYNOPSIS
    Events::dump_internal_status()
*/

void Events::dump_internal_status() {
  DBUG_TRACE;
  puts("\n\n\nEvents status:");
  puts("LLA = Last Locked At  LUA = Last Unlocked At");
  puts("WOC = Waiting On Condition  DL = Data Locked");

  /*
    opt_event_scheduler should only be accessed while
    holding LOCK_global_system_variables.
  */
  mysql_mutex_lock(&LOCK_global_system_variables);
  if (opt_event_scheduler == EVENTS_DISABLED)
    puts("The Event Scheduler is disabled");
  else {
    scheduler->dump_internal_status();
    event_queue->dump_internal_status();
  }

  mysql_mutex_unlock(&LOCK_global_system_variables);
}

bool Events::start(int *err_no) {
  bool ret = false;
  if (scheduler) ret = scheduler->start(err_no);
  return ret;
}

bool Events::stop() {
  bool ret = false;
  if (scheduler) ret = scheduler->stop();
  return ret;
}

/**
   Loads all ENABLED events into a prioritized queue.

   This function is called during the server start up. It reads
   every event, computes the next execution time, and if the event
   needs execution, adds it to a prioritized queue. Otherwise, if
   ON COMPLETION DROP is specified, the event is automatically
   removed from the table.

   @param  thd           THD context. Used for memory allocation in some cases.
   @param  event_queue   Pointer to Event_queue object.

   @retval  false  success
   @retval  true   error, the load is aborted

   @note Reports the error to the console
*/

static bool load_events_from_db(THD *thd, Event_queue *event_queue) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("thd: %p", thd));

  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  // Fetch all Schemas
  std::vector<const dd::Schema *> schemas;
  if (thd->dd_client()->fetch_global_components(&schemas)) return true;

  std::vector<std::pair<const dd::Schema *, const dd::Event *>>
      drop_events_vector;

  for (const dd::Schema *schema_obj : schemas) {
    // Fetch all events in a schema
    std::vector<const dd::Event *> events;
    if (thd->dd_client()->fetch_schema_components(schema_obj, &events))
      return true;

    for (const dd::Event *ev_obj : events) {
      std::unique_ptr<Event_queue_element> et(new (std::nothrow)
                                                  Event_queue_element);
      if (et == nullptr) {
        LogErr(ERROR_LEVEL, ER_EVENT_SCHEDULER_ERROR_GETTING_EVENT_OBJECT);
        return true;
      }

      if (et->fill_event_info(thd, *ev_obj, schema_obj->name().c_str())) {
        LogErr(ERROR_LEVEL, ER_EVENT_SCHEDULER_GOT_BAD_DATA_FROM_TABLE);
        return true;
      }
      bool drop_event = et->m_dropped;  // create_event may free et.
      bool created = false;
      if (event_queue->create_event(thd, et.get(), &created)) {
        /* Out of memory */
        return true;
      }
      if (created) et.release();

      if (drop_event) {
        /*
          If not created, a stale event - drop if immediately if
          ON COMPLETION NOT PRESERVE.
          XXX: This won't be replicated, thus the drop won't appear in
          in the slave. When the slave is restarted it will drop events.
          However, as the slave will be "out of sync", it might happen that
          an event created on the master, after master restart, won't be
          replicated to the slave correctly, as the create will fail there.
        */
        drop_events_vector.push_back(std::make_pair(schema_obj, ev_obj));
      }
    }
  }

  bool error = false;
  for (auto event_info : drop_events_vector) {
    if (lock_object_name(thd, MDL_key::EVENT, event_info.first->name().c_str(),
                         event_info.second->name().c_str())) {
      LogErr(WARNING_LEVEL, ER_EVENT_CANT_GET_LOCK_FOR_DROPPING_EVENT,
             event_info.second->name().c_str(),
             event_info.first->name().c_str());
      continue;
    }

    if (thd->dd_client()->drop(event_info.second)) {
      error = true;
      LogErr(WARNING_LEVEL, ER_EVENT_UNABLE_TO_DROP_EVENT,
             event_info.second->name().c_str(),
             event_info.first->name().c_str());
      break;
    }
  }
  if (error) {
    trans_rollback_stmt(thd);
    // Full rollback in case we have THD::transaction_rollback_request.
    trans_rollback(thd);
  } else
    error = trans_commit_stmt(thd) || trans_commit(thd);

  // Note that locks are released here before the Auto_releaser
  // goes out of scope. This is safe since no cached objects were
  // acquired.
  thd->mdl_context.release_transactional_locks();
  return error;
}

/**
  @} (End of group Event_Scheduler)
*/
