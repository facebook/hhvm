/*
   Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/table_trigger_dispatcher.h"

#include <sys/types.h>
#include <string>
#include <utility>

#include "m_ctype.h"
#include "m_string.h"
#include "my_alloc.h"
#include "my_dbug.h"
#include "my_sqlcommand.h"
#include "sql/auth/auth_acls.h"
#include "sql/auth/auth_common.h"  // check_global_access
#include "sql/auth/sql_security_ctx.h"
#include "sql/dd/cache/dictionary_client.h"
#include "sql/dd/dd_trigger.h"  // dd::create_trigger
#include "sql/dd/string_type.h"
#include "sql/dd/types/trigger.h"  // name_collation
#include "sql/derror.h"            // ER_THD
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/mysqld.h"  // table_alias_charset
#include "sql/psi_memory_key.h"
#include "sql/sp_head.h"  // sp_head
#include "sql/sql_class.h"
#include "sql/sql_error.h"
#include "sql/sql_lex.h"
#include "sql/sql_list.h"
#include "sql/sql_parse.h"  // create_default_definer
#include "sql/table.h"
#include "sql/thr_malloc.h"
#include "sql/trigger.h"
#include "sql/trigger_chain.h"
#include "thr_lock.h"

namespace dd {
class Schema;
class Table;
}  // namespace dd

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

/**
  Create an instance of Table_trigger_dispatcher for the given subject table.

  @param subject_table  valid (not fake!) TABLE-object representing
                        the subject table

  @return a pointer to a new Table_trigger_dispatcher instance.
*/
Table_trigger_dispatcher *Table_trigger_dispatcher::create(
    TABLE *subject_table) {
  return new (&subject_table->mem_root) Table_trigger_dispatcher(subject_table);
}

/**
  Private form of Table_trigger_dispatcher constructor. In order to construct an
  instance of Table_trigger_dispatcher with a valid pointer to the subject
  table, use Table_trigger_dispatcher::create().
*/
Table_trigger_dispatcher::Table_trigger_dispatcher(TABLE *subject_table)
    : m_subject_table(subject_table),
      m_record1_field(nullptr),
      m_new_field(nullptr),
      m_old_field(nullptr),
      m_has_unparseable_trigger(false) {
  memset(m_trigger_map, 0, sizeof(m_trigger_map));
  m_parse_error_message[0] = 0;
}

Table_trigger_dispatcher::~Table_trigger_dispatcher() {
  // Destroy fields.

  if (m_record1_field) {
    for (Field **fld_ptr = m_record1_field; *fld_ptr; fld_ptr++)
      destroy(*fld_ptr);
  }

  // Destroy trigger chains.

  for (int i = 0; i < (int)TRG_EVENT_MAX; ++i)
    for (int j = 0; j < (int)TRG_ACTION_MAX; ++j) destroy(m_trigger_map[i][j]);
}

/**
  Create trigger for table.

  @param      thd   thread context
  @param[out] binlog_create_trigger_stmt
                    well-formed CREATE TRIGGER statement for putting into binlog
                    (after successful execution)

  @note
    - Assumes that trigger name is fully qualified.
    - NULL-string means the following LEX_STRING instance:
    { str = 0; length = 0 }.
    - In other words, definer_user and definer_host should contain
    simultaneously NULL-strings (non-SUID/old trigger) or valid strings
    (SUID/new trigger).

  @return Operation status.
    @retval false Success
    @retval true  Failure
*/

bool Table_trigger_dispatcher::create_trigger(
    THD *thd, String *binlog_create_trigger_stmt) {
  DBUG_ASSERT(m_subject_table);
  LEX *lex = thd->lex;
  dd::cache::Dictionary_client::Auto_releaser releaser(thd->dd_client());

  // If this table has broken triggers, CREATE TRIGGER is not allowed.
  if (check_for_broken_triggers()) return true;

  // Check that the new trigger is in the same schema as the base table.
  if (my_strcasecmp(table_alias_charset, m_subject_table->s->db.str,
                    lex->spname->m_db.str)) {
    my_error(ER_TRG_IN_WRONG_SCHEMA, MYF(0));
    return true;
  }

  // Check if a trigger with the same name already exist in this schema.
  const dd::Schema *sch_obj = nullptr;
  if (thd->dd_client()->acquire(lex->spname->m_db.str, &sch_obj)) return true;

  // The table is already open, so the schema must exist.
  DBUG_ASSERT(sch_obj != nullptr);

  dd::String_type table_name;
  if (thd->dd_client()->get_table_name_by_trigger_name(
          *sch_obj, lex->spname->m_name.str, &table_name))
    return true;

  if (table_name != "") {
    my_error(ER_TRG_ALREADY_EXISTS, MYF(0));
    return true;
  }

  // Make sure DEFINER clause is specified.

  if (!lex->definer) {
    /*
      DEFINER-clause is missing.

      If we are in slave thread, this means that we received CREATE TRIGGER
      from the master, that does not support definer in triggers. So, we
      should mark this trigger as non-SUID. Note that this does not happen
      when we parse triggers' definitions during reading metadata from
      the Data Dictionary. LEX::definer is ignored in that case.

      Otherwise, we should use CURRENT_USER() as definer.

      NOTE: when CREATE TRIGGER statement is allowed to be executed in PS/SP,
      it will be required to create the definer below in persistent MEM_ROOT
      of PS/SP.

      NOTE: here we allocate lex->definer on THD->mem_root. Later it will be
      copied into the base table mem-root to be used inside Trigger.
    */

    if (!thd->slave_thread) {
      if (!(lex->definer = create_default_definer(thd))) return true;
    } else {
      my_error(ER_TRG_NO_DEFINER, MYF(0), m_subject_table->s->db.str,
               lex->spname->m_name.str);
      return true;
    }
  }

  /*
    If the specified definer differs from the current user, we should check
    that the current user has SUPER privilege (in order to create trigger
    under another user one must have SUPER privilege).
  */
  Security_context *sctx = thd->security_context();
  if (lex->definer &&
      (strcmp(lex->definer->user.str, sctx->priv_user().str) ||
       my_strcasecmp(system_charset_info, lex->definer->host.str,
                     sctx->priv_host().str))) {
    if (!sctx->check_access(SUPER_ACL) &&
        !sctx->has_global_grant(STRING_WITH_LEN("SET_USER_ID")).first) {
      my_error(ER_SPECIFIC_ACCESS_DENIED_ERROR, MYF(0), "SUPER or SET_USER_ID");
      return true;
    }
    if (sctx->can_operate_with({lex->definer}, consts::system_user, true))
      return true;
  }

  if (lex->definer &&
      !is_acl_user(thd, lex->definer->host.str, lex->definer->user.str)) {
    push_warning_printf(thd, Sql_condition::SL_NOTE, ER_NO_SUCH_USER,
                        ER_THD(thd, ER_NO_SUCH_USER), lex->definer->user.str,
                        lex->definer->host.str);

    if (thd->get_stmt_da()->is_error()) return true;
  }

  /*
    Check if all references to fields in OLD/NEW-rows in this trigger are valid.

    NOTE: Setting m_old_field / m_new_field here is required because of
    Item_trigger_field::fix_fields() later.

    NOTE: We do it here more from ease of use standpoint. We still have to
    do some checks on each execution. E.g. we can catch privilege changes
    only during execution. Also in near future, when we will allow access
    to other tables from trigger we won't be able to catch changes in other
    tables...

    Since we don't plan to access to contents of the fields it does not
    matter that we choose for both OLD and NEW values the same versions
    of Field objects here.
   */

  m_old_field = m_subject_table->field;
  m_new_field = m_subject_table->field;

  if (lex->sphead->setup_trigger_fields(thd, this, nullptr, true)) return true;

  m_old_field = nullptr;
  m_new_field = nullptr;

  // Create new trigger.

  Trigger *t = Trigger::create_from_parser(thd, m_subject_table,
                                           binlog_create_trigger_stmt);

  if (!t) return true;

  // Create trigger chain.

  Trigger_chain *tc = create_trigger_chain(
      &m_subject_table->mem_root, t->get_event(), t->get_action_time());

  if (!tc) {
    destroy(t);
    return true;
  }

  // Add the newly created trigger to the chain.

  if (tc->add_trigger(&m_subject_table->mem_root, t,
                      lex->sphead->m_trg_chistics.ordering_clause,
                      lex->sphead->m_trg_chistics.anchor_trigger_name)) {
    destroy(t);
    return true;
  }

  return dd::create_trigger(thd, t, lex->sphead->m_trg_chistics.ordering_clause,
                            lex->sphead->m_trg_chistics.anchor_trigger_name);
}

/**
  Prepare array of Field objects referencing to TABLE::record[1] instead
  of record[0] (they will represent OLD.* row values in ON UPDATE trigger
  and in ON DELETE trigger which will be called during REPLACE execution).

  @return Operation status.
    @retval false Success
    @retval true  Failure
*/

bool Table_trigger_dispatcher::prepare_record1_accessors() {
  Field **fld, **old_fld;

  DBUG_ASSERT(m_subject_table);

  m_record1_field = (Field **)m_subject_table->mem_root.Alloc(
      (m_subject_table->s->fields + 1) * sizeof(Field *));

  if (!m_record1_field) return true;

  for (fld = m_subject_table->field, old_fld = m_record1_field; *fld;
       fld++, old_fld++) {
    *old_fld = (*fld)->new_field(&m_subject_table->mem_root, m_subject_table);

    if (!(*old_fld)) return true;

    (*old_fld)->move_field_offset(
        (ptrdiff_t)(m_subject_table->record[1] - m_subject_table->record[0]));
  }

  *old_fld = nullptr;

  return false;
}

/**
  Load triggers for the table.

  @param thd          current thread context
  @param table        table object.

  @return Operation status.
    @retval false Success
    @retval true  Failure
*/

bool Table_trigger_dispatcher::check_n_load(THD *thd, const dd::Table &table) {
  DBUG_ASSERT(m_subject_table);

  // Load triggers from Data Dictionary.

  List<Trigger> triggers;

  if (dd::load_triggers(thd, &m_subject_table->mem_root,
                        m_subject_table->s->db.str,
                        m_subject_table->s->table_name.str, table, &triggers))
    return true;

  // 'false' flag for 'is_upgrade' as we read Trigger from DD.
  parse_triggers(thd, &triggers, false);

  // Create trigger chains and assigns triggers to chains.

  Trigger_chain unparseable_triggers;
  List_iterator_fast<Trigger> it(triggers);
  Trigger *t;

  while ((t = it++)) {
    Trigger_chain *tc =
        t->has_parse_error()
            ? &unparseable_triggers
            : create_trigger_chain(&m_subject_table->mem_root, t->get_event(),
                                   t->get_action_time());

    if (!tc || tc->add_trigger(&m_subject_table->mem_root, t)) return true;
  }

  // Prepare fields for the OLD-row.

  if (has_update_triggers() || has_delete_triggers()) {
    if (prepare_record1_accessors()) return true;
  }

  /*
    Bind Item_trigger_field in the trigger's SP-head to this
    Table_trigger_dispatcher object.
  */

  it.rewind();
  while ((t = it++)) {
    sp_head *sp = t->get_sp();

    if (!sp) continue;

    sp->setup_trigger_fields(thd, this, t->get_subject_table_grant(), false);
  }

  return false;
}

/**
  Make sure there is a chain for the specified event and action time.

  @return A pointer to newly created Trigger_chain object,
  NULL in case of OOM error.
*/

Trigger_chain *Table_trigger_dispatcher::create_trigger_chain(
    MEM_ROOT *mem_root, enum_trigger_event_type event,
    enum_trigger_action_time_type action_time) {
  DBUG_ASSERT(event != TRG_EVENT_MAX);
  DBUG_ASSERT(action_time != TRG_ACTION_MAX);

  Trigger_chain *tc = get_triggers(event, action_time);

  if (tc) return tc;

  tc = new (mem_root) Trigger_chain();

  if (tc) m_trigger_map[event][action_time] = tc;

  return tc;
}

/**
  Get trigger object by trigger name.

  @param [in] trigger_name  trigger name

  @return a pointer to Trigger object, NULL if the trigger not found.
*/

Trigger *Table_trigger_dispatcher::find_trigger(
    const LEX_STRING &trigger_name) {
  for (int i = 0; i < static_cast<int>(TRG_EVENT_MAX); ++i) {
    for (int j = 0; j < static_cast<int>(TRG_ACTION_MAX); ++j) {
      Trigger_chain *tc = get_triggers(i, j);

      if (tc == nullptr) continue;

      List_iterator<Trigger> it(tc->get_trigger_list());
      Trigger *t;

      while ((t = it++) != nullptr) {
        if (!my_strnncoll(
                dd::Trigger::name_collation(),
                pointer_cast<const uchar *>(t->get_trigger_name().str),
                t->get_trigger_name().length,
                pointer_cast<const uchar *>(trigger_name.str),
                trigger_name.length))
          return t;
      }
    }
  }

  return nullptr;
}

/**
  Parse trigger definition statements (CREATE TRIGGER).

  @param [in] thd         Thread context
  @param [in] is_upgrade  Flag to indicate that trigger being parsed is read
                          from .TRG file in case of upgrade.
  @param [in] triggers    List of triggers to parse
*/

void Table_trigger_dispatcher::parse_triggers(THD *thd, List<Trigger> *triggers,
                                              bool is_upgrade) {
  List_iterator<Trigger> it(*triggers);

  while (true) {
    Trigger *t = it++;

    if (!t) break;

    bool fatal_parse_error = t->parse(thd, is_upgrade);

    /*
      There are two kinds of parse errors here:

        - "soft errors" -- these are the errors when we were able to parse out
          the trigger name and the base table name. However there was some
          parse error that prevents the trigger from being executed.

          This kind of errors is designated by:
            - fatal_parse_error is false
            - Trigger::has_parse_error() returns true

          In case of these errors we put the failed trigger into the list so
          that it will be shown in the informational statements (queries from
          INFORMATION_SCHEMA and so on), but it's forbidden to execute such
          triggers.

        - "fatal errors" -- there are the errors when we were unable to get
          even basic information about the trigger (or out-of-memory error
          happens). The trigger is in completely useless state.

          This kind of errors is signalled by fatal_parse_error being true.

          In case of these errors we just remember the error message and delete
          the trigger instance (do not put it into the list).
    */

    if (fatal_parse_error || t->has_parse_error()) {
      DBUG_ASSERT(!t->get_sp());  // SP must be NULL.

      if (t->has_parse_error())
        set_parse_error_message(t->get_parse_error_message());

      /*
        In case we are upgrading, call set_parse_error_message() to set
        m_has_unparseable_trigger in case of fatal errors too. As return type
        of this function is void, we use m_has_unparseable_trigger to check
        for any errors in Trigger upgrade upgrade.
      */
      if (is_upgrade && fatal_parse_error) {
        set_parse_error_message("Fatal Error in Parsing Trigger.");
      }

      if (fatal_parse_error) {
        destroy(t);
        it.remove();
      }

      continue;
    }

    DBUG_ASSERT(!t->has_parse_error());

    sp_head *sp = t->get_sp();

    if (sp) sp->m_trg_list = this;
  }
}

/**
  Execute trigger for given (event, time) pair.

  The operation executes trigger for the specified event (insert, update,
  delete) and time (after, before) if it is set.

  @param thd                 Thread handle
  @param event               Insert, update or delete
  @param action_time         Before or after
  @param old_row_is_record1  If record1 contains old or new field.

  @return Operation status.
    @retval false Success
    @retval true  Failure
*/

bool Table_trigger_dispatcher::process_triggers(
    THD *thd, enum_trigger_event_type event,
    enum_trigger_action_time_type action_time, bool old_row_is_record1) {
  if (check_for_broken_triggers()) return true;

  Trigger_chain *tc = get_triggers(event, action_time);

  if (!tc) return false;

  DBUG_ASSERT(m_subject_table);

  if (old_row_is_record1) {
    m_old_field = m_record1_field;
    m_new_field = m_subject_table->field;
  } else {
    m_new_field = m_record1_field;
    m_old_field = m_subject_table->field;
  }
  /*
    This trigger must have been processed by the pre-locking
    algorithm.
  */
  DBUG_ASSERT(m_subject_table->pos_in_table_list->trg_event_map &
              static_cast<uint>(1 << static_cast<int>(event)));

  bool rc = tc->execute_triggers(thd);

  m_new_field = nullptr;
  m_old_field = nullptr;

  return rc;
}

/**
  Add triggers for table to the set of routines used by statement.
  Add tables used by them to statement table list. Do the same for
  routines used by triggers.

  @param thd             Thread context.
  @param prelocking_ctx  Prelocking context of the statement.
  @param table_list      Table list element for table with trigger.

  @return Operation status.
    @retval false Success
    @retval true  Failure
*/

bool Table_trigger_dispatcher::add_tables_and_routines_for_triggers(
    THD *thd, Query_tables_list *prelocking_ctx, TABLE_LIST *table_list) {
  DBUG_ASSERT(static_cast<int>(table_list->lock_descriptor().type) >=
              static_cast<int>(TL_WRITE_ALLOW_WRITE));

  for (int i = 0; i < (int)TRG_EVENT_MAX; ++i) {
    if (table_list->trg_event_map & static_cast<uint8>(1 << i)) {
      for (int j = 0; j < (int)TRG_ACTION_MAX; ++j) {
        Trigger_chain *tc = table_list->table->triggers->get_triggers(i, j);

        if (tc) tc->add_tables_and_routines(thd, prelocking_ctx, table_list);
      }
    }
  }
  return false;
}

/**
  Mark all trigger fields as "temporary nullable" and remember the current
  THD::check_for_truncated_fields value.

  @param thd Thread context.
*/

void Table_trigger_dispatcher::enable_fields_temporary_nullability(THD *thd) {
  DBUG_ASSERT(m_subject_table);

  for (Field **next_field = m_subject_table->field; *next_field; ++next_field) {
    (*next_field)->set_tmp_nullable();
    (*next_field)
        ->set_check_for_truncated_fields(thd->check_for_truncated_fields);

    /*
      For statement LOAD INFILE we set field values during parsing of data file
      and later run fill_record_n_invoke_before_triggers() to invoke table's
      triggers. fill_record_n_invoke_before_triggers() calls this method
      to enable temporary nullability before running trigger's instructions
      Since for the case of handling statement LOAD INFILE the null value of
      fields have been already set we don't have to reset these ones here.
      In case of handling statements INSERT/REPLACE/INSERT SELECT/
      REPLACE SELECT we set field's values inside method fill_record
      that is called from fill_record_n_invoke_before_triggers()
      after the method enable_fields_temporary_nullability has been executed.
    */
    if (thd->lex->sql_command != SQLCOM_LOAD) (*next_field)->reset_tmp_null();
  }
}

/**
  Reset "temporary nullable" flag from trigger fields.
*/

void Table_trigger_dispatcher::disable_fields_temporary_nullability() {
  DBUG_ASSERT(m_subject_table);

  for (Field **next_field = m_subject_table->field; *next_field; ++next_field)
    (*next_field)->reset_tmp_nullable();
}

/**
  Iterate along triggers and print necessary upgrade warnings.

  Now it prints the warning about missing 'CREATED' attribute.

  @param thd Thread context.
*/

void Table_trigger_dispatcher::print_upgrade_warnings(THD *thd) {
  for (int i = 0; i < static_cast<int>(TRG_EVENT_MAX); ++i) {
    for (int j = 0; j < static_cast<int>(TRG_ACTION_MAX); ++j) {
      Trigger_chain *tc = get_triggers(i, j);

      if (tc == nullptr) continue;

      List_iterator<Trigger> it(tc->get_trigger_list());
      Trigger *t;

      while ((t = it++) != nullptr) {
        t->print_upgrade_warning(thd);
      }
    }
  }
}

/**
  Mark fields of subject table which we read/set in its triggers
  as such.

  This method marks fields of subject table which are read/set in its
  triggers as such (by properly updating TABLE::read_set/write_set)
  and thus informs handler that values for these fields should be
  retrieved/stored during execution of statement.

  @param event  Type of event triggers for which we are going to inspect

  @returns false if success, true if error
*/

bool Table_trigger_dispatcher::mark_fields(enum_trigger_event_type event) {
  if (check_for_broken_triggers()) return true;

  DBUG_ASSERT(m_subject_table);

  for (int i = 0; i < (int)TRG_ACTION_MAX; ++i) {
    Trigger_chain *tc = get_triggers(event, i);

    if (!tc) continue;

    tc->mark_fields(m_subject_table);
  }

  m_subject_table->file->column_bitmaps_signal();
  return false;
}
