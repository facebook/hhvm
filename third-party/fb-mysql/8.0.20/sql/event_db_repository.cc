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

#include "sql/event_db_repository.h"

#include <vector>

#include "lex_string.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "mysqld_error.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/sql_security_ctx.h"
#include "sql/dd/cache/dictionary_client.h"  // fetch_schema_components
#include "sql/dd/dd_event.h"
#include "sql/dd/string_type.h"
#include "sql/dd/types/event.h"
#include "sql/derror.h"
#include "sql/event_data_objects.h"
#include "sql/event_parse_data.h"
#include "sql/sp_head.h"
#include "sql/sql_class.h"
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/thd_raii.h"
#include "sql/transaction.h"
#include "sql/tztime.h"  // struct Time_zone

/**
  @addtogroup Event_Scheduler
  @{
*/

/**
  Creates an event object and persist to Data Dictionary.

  @pre All semantic checks must be performed outside.

  @param[in,out] thd                   THD
  @param[in]     parse_data            Parsed event definition
  @param[in]     create_if_not         true if IF NOT EXISTS clause was provided
                                       to CREATE EVENT statement
  @param[out]    event_already_exists  When method is completed successfully
                                       set to true if event already exists else
                                       set to false
  @retval false  Success
  @retval true   Error
*/

bool Event_db_repository::create_event(THD *thd, Event_parse_data *parse_data,
                                       bool create_if_not,
                                       bool *event_already_exists) {
  DBUG_TRACE;
  sp_head *sp = thd->lex->sphead;
  DBUG_ASSERT(sp);

  const dd::Schema *schema = nullptr;
  const dd::Event *event = nullptr;
  if (thd->dd_client()->acquire(parse_data->dbname.str, &schema) ||
      thd->dd_client()->acquire(parse_data->dbname.str, parse_data->name.str,
                                &event))
    return true;

  if (schema == nullptr) {
    my_error(ER_BAD_DB_ERROR, MYF(0), parse_data->dbname.str);
    return true;
  }

  *event_already_exists = (event != nullptr);

  if (*event_already_exists) {
    if (create_if_not) {
      push_warning_printf(thd, Sql_condition::SL_NOTE, ER_EVENT_ALREADY_EXISTS,
                          ER_THD(thd, ER_EVENT_ALREADY_EXISTS),
                          parse_data->name.str);
      return false;
    }
    my_error(ER_EVENT_ALREADY_EXISTS, MYF(0), parse_data->name.str);
    return true;
  }

  return dd::create_event(thd, *schema, parse_data->name.str, sp->m_body.str,
                          sp->m_body_utf8.str, thd->lex->definer, parse_data);
}

/**
  Used to execute ALTER EVENT. Pendant to Events::update_event().

  @param[in]      thd         THD context
  @param[in]      parse_data  parsed event definition
  @param[in]      new_dbname  not NULL if ALTER EVENT RENAME
                              points at a new database name
  @param[in]      new_name    not NULL if ALTER EVENT RENAME
                              points at a new event name

  @pre All semantic checks are performed outside this function.

  @retval false Success
  @retval true Error (reported)
*/

bool Event_db_repository::update_event(THD *thd, Event_parse_data *parse_data,
                                       const LEX_CSTRING *new_dbname,
                                       const LEX_CSTRING *new_name) {
  DBUG_TRACE;
  sp_head *sp = thd->lex->sphead;

  /* None or both must be set */
  DBUG_ASSERT((new_dbname && new_name) || new_dbname == new_name);

  DBUG_PRINT("info", ("dbname: %s", parse_data->dbname.str));
  DBUG_PRINT("info", ("name: %s", parse_data->name.str));
  DBUG_PRINT("info", ("user: %s", parse_data->definer.str));

  /* first look whether we overwrite */
  if (new_name) {
    DBUG_PRINT("info", ("rename to: %s@%s", new_dbname->str, new_name->str));

    const dd::Event *new_event = nullptr;
    if (thd->dd_client()->acquire(new_dbname->str, new_name->str, &new_event))
      return true;

    if (new_event != nullptr) {
      my_error(ER_EVENT_ALREADY_EXISTS, MYF(0), new_name->str);
      return true;
    }
  }

  const dd::Schema *new_schema = nullptr;
  if (new_dbname != nullptr) {
    if (thd->dd_client()->acquire(new_dbname->str, &new_schema)) return true;

    if (new_schema == nullptr) {
      my_error(ER_BAD_DB_ERROR, MYF(0), new_dbname->str);
      return true;
    }
  }

  const dd::Schema *schema = nullptr;
  dd::Event *event = nullptr;
  if (thd->dd_client()->acquire(parse_data->dbname.str, &schema) ||
      thd->dd_client()->acquire_for_modification(parse_data->dbname.str,
                                                 parse_data->name.str, &event))
    return true;

  if (event == nullptr) {
    my_error(ER_EVENT_DOES_NOT_EXIST, MYF(0), parse_data->name.str);
    return true;
  }
  DBUG_ASSERT(schema != nullptr);  // Must exist if event exists.

  /*
    If definer has the SYSTEM_USER privilege then invoker can alter event
    only if latter also has same privilege.
  */
  Security_context *sctx = thd->security_context();
  Auth_id definer(event->definer_user().c_str(), event->definer_host().c_str());
  if (sctx->can_operate_with(definer, consts::system_user, true)) return true;

  // Update Event in the data dictionary with altered event object attributes.
  bool ret = dd::update_event(
      thd, event, *schema, new_schema, new_name != nullptr ? new_name->str : "",
      (parse_data->body_changed) ? sp->m_body.str : event->definition(),
      (parse_data->body_changed) ? sp->m_body_utf8.str
                                 : event->definition_utf8(),
      thd->lex->definer, parse_data);
  return ret;
}

/**
  Delete event.

  @param[in]     thd            THD context
  @param[in]     db             Database name
  @param[in]     name           Event name
  @param[in]     drop_if_exists DROP IF EXISTS clause was specified.
                                If set, and the event does not exist,
                                the error is downgraded to a warning.
  @param[out]   event_exists    Set to true if event exists. Set to
                                false otherwise.

  @retval false success
  @retval true error (reported)
*/

bool Event_db_repository::drop_event(THD *thd, LEX_CSTRING db, LEX_CSTRING name,
                                     bool drop_if_exists, bool *event_exists) {
  DBUG_TRACE;
  /*
    Turn off row binlogging of this statement and use statement-based
    so that all supporting tables are updated for CREATE EVENT command.
    When we are going out of the function scope, the original binary
    format state will be restored.
  */
  Save_and_Restore_binlog_format_state binlog_format_state(thd);

  DBUG_PRINT("enter", ("%s@%s", db.str, name.str));

  const dd::Event *event_ptr = nullptr;
  if (thd->dd_client()->acquire(db.str, name.str, &event_ptr)) {
    // Error is reported by the dictionary subsystem.
    return true;
  }

  if (event_ptr == nullptr) {
    *event_exists = false;

    // Event not found
    if (!drop_if_exists) {
      my_error(ER_EVENT_DOES_NOT_EXIST, MYF(0), name.str);
      return true;
    }

    push_warning_printf(thd, Sql_condition::SL_NOTE, ER_SP_DOES_NOT_EXIST,
                        ER_THD(thd, ER_SP_DOES_NOT_EXIST), "Event", name.str);
    return false;
  }
  /*
    If definer has the SYSTEM_USER privilege then invoker can drop event
    only if latter also has same privilege.
  */
  Auth_id definer(event_ptr->definer_user().c_str(),
                  event_ptr->definer_host().c_str());
  Security_context *sctx = thd->security_context();
  if (sctx->can_operate_with(definer, consts::system_user, true)) return true;

  *event_exists = true;
  return thd->dd_client()->drop(event_ptr);
}

/**
  Drops all events in the selected database.

  @param      thd     THD context
  @param      schema  The database under which events are to be dropped.

  @returns true on error, false on success.
*/

bool Event_db_repository::drop_schema_events(THD *thd,
                                             const dd::Schema &schema) {
  DBUG_TRACE;

  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  std::vector<const dd::Event *> events;
  if (thd->dd_client()->fetch_schema_components(&schema, &events)) return true;

  for (const dd::Event *event_obj : events) {
    if (thd->dd_client()->drop(event_obj)) {
      my_error(ER_SP_DROP_FAILED, MYF(0), "Drop failed for Event: %s",
               event_obj->name().c_str());
      return true;
    }
  }

  return false;
}

/**
  Looks for a named event in the Data Dictionary and load it.

  @pre The given thread does not have open tables.

  @retval false  success
  @retval true   error
*/

bool Event_db_repository::load_named_event(THD *thd, LEX_CSTRING dbname,
                                           LEX_CSTRING name, Event_basic *etn) {
  const dd::Event *event_obj = nullptr;

  DBUG_TRACE;
  DBUG_PRINT("enter", ("thd: %p  name: %*s", thd, (int)name.length, name.str));

  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  if (thd->dd_client()->acquire(dbname.str, name.str, &event_obj)) {
    // Error is reported by the dictionary subsystem.
    return true;
  }

  if (event_obj == nullptr) {
    my_error(ER_EVENT_DOES_NOT_EXIST, MYF(0), name.str);
    return true;
  }

  if (etn->fill_event_info(thd, *event_obj, dbname.str)) {
    my_error(ER_CANNOT_LOAD_FROM_TABLE_V2, MYF(0), "mysql", "events");
    return true;
  }

  return false;
}

/**
   Update the event in Data Dictionary with changed status
   and/or last execution time.
*/

bool Event_db_repository::update_timing_fields_for_event(
    THD *thd, LEX_CSTRING event_db_name, LEX_CSTRING event_name,
    my_time_t last_executed, ulonglong status) {
  DBUG_TRACE;
  // Turn off autocommit.
  Disable_autocommit_guard autocommit_guard(thd);

  /*
    Turn off row binlogging of this statement and use statement-based
    so that all supporting tables are updated for CREATE EVENT command.
    When we are going out of the function scope, the original binary
    format state will be restored.
  */
  Save_and_Restore_binlog_format_state binlog_format_state(thd);

  DBUG_ASSERT(thd->security_context()->check_access(SUPER_ACL));

  dd::Event *event = nullptr;
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());
  if (thd->dd_client()->acquire_for_modification(event_db_name.str,
                                                 event_name.str, &event))
    return true;
  if (event == nullptr) return true;

  if (dd::update_event_time_and_status(thd, event, last_executed, status)) {
    trans_rollback_stmt(thd);
    // Full rollback in case we have THD::transaction_rollback_request.
    trans_rollback(thd);
    return true;
  }

  return trans_commit_stmt(thd) || trans_commit(thd);
}

/**
  @} (End of group Event_Scheduler)
*/
// XXX:
