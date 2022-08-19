/*
   Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/trigger.h"

#include <stdio.h>
#include <atomic>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_dbug.h"
#include "my_psi_config.h"
#include "mysql/components/services/psi_statement_bits.h"
#include "mysql/psi/mysql_sp.h"
#include "mysqld_error.h"
#include "mysys_err.h"          // EE_OUTOFMEMORY
#include "sql/derror.h"         // ER_THD
#include "sql/error_handler.h"  // Internal_error_handler
#include "sql/sp.h"             // sp_add_used_routine
#include "sql/sp_head.h"        // sp_name
#include "sql/sql_class.h"      // THD
#include "sql/sql_db.h"         // get_default_db_collation
#include "sql/sql_digest_stream.h"
#include "sql/sql_error.h"  // Sql_condition
#include "sql/sql_lex.h"
#include "sql/sql_parse.h"  // parse_sql
#include "sql/sql_show.h"   // append_identifier
#include "sql/strfunc.h"
#include "sql/system_variables.h"
#include "sql/trigger_creation_ctx.h"  // Trigger_creation_ctx
#include "sql_string.h"

class sp_rcontext;
struct MEM_ROOT;

///////////////////////////////////////////////////////////////////////////

/**
  An error handler that catches all non-OOM errors which can occur during
  parsing of trigger body. Such errors are ignored and corresponding error
  message is used to construct a more verbose error message which contains
  name of problematic trigger. This error message is later emitted when
  one tries to perform DML or some of DDL on this table.
  Also, if possible, grabs name of the trigger being parsed so it can be
  used to correctly drop problematic trigger.
*/
class Deprecated_trigger_syntax_handler : public Internal_error_handler {
 private:
  char m_message[MYSQL_ERRMSG_SIZE];
  LEX_STRING *m_trigger_name;

 public:
  Deprecated_trigger_syntax_handler() : m_trigger_name(nullptr) {}

  virtual bool handle_condition(THD *thd, uint sql_errno, const char *,
                                Sql_condition::enum_severity_level *,
                                const char *message) {
    if (sql_errno != EE_OUTOFMEMORY && sql_errno != ER_OUT_OF_RESOURCES) {
      if (thd->lex->spname) m_trigger_name = &thd->lex->spname->m_name;
      if (m_trigger_name)
        snprintf(m_message, sizeof(m_message),
                 ER_THD(thd, ER_ERROR_IN_TRIGGER_BODY), m_trigger_name->str,
                 message);
      else
        snprintf(m_message, sizeof(m_message),
                 ER_THD(thd, ER_ERROR_IN_UNKNOWN_TRIGGER_BODY), message);
      return true;
    }
    return false;
  }

  LEX_STRING *get_trigger_name() { return m_trigger_name; }
  const char *get_error_message() { return m_message; }
};

///////////////////////////////////////////////////////////////////////////

/**
  Create a definer value from its user and host parts

  @param mem_root           mem-root where needed strings will be allocated
  @param[out] definer       pointer to LEX_CSTRING holder where to store a
                            constructed value of definer
  @param definer_user       user part of a definer value
  @param definer_host       host part of a definer value

  @return Operation status.
    @retval false Success
    @retval true  Failure
*/

static bool construct_definer_value(MEM_ROOT *mem_root, LEX_CSTRING *definer,
                                    const LEX_CSTRING &definer_user,
                                    const LEX_CSTRING &definer_host) {
  char definer_buf[USER_HOST_BUFF_SIZE];
  size_t definer_len =
      strxmov(definer_buf, definer_user.str, "@", definer_host.str, NullS) -
      definer_buf;

  return lex_string_strmake(mem_root, definer, definer_buf, definer_len);
}

/**
  Constructs CREATE TRIGGER statement taking into account a value of
  the DEFINER clause.

  The point of this method is to create canonical forms of CREATE TRIGGER
  statement for writing into the binlog.

  @note
  A statement for the binlog form must preserve FOLLOWS/PRECEDES clause
  if it was in the original statement. The reason for that difference is this:

    - the Data Dictionary preserves the trigger execution order (action_order),
      thus FOLLOWS/PRECEDES clause is not needed.

    - moreover, FOLLOWS/PRECEDES clause usually makes problem in mysqldump,
      because CREATE TRIGGER statement will have a reference to non-yet-existing
      trigger (which is about to be created right after this one).

    - thus, FOLLOWS/PRECEDES must not be stored in the Data Dictionary.

    - on the other hand, the binlog contains statements in the user order (as
      the user executes them). Thus, it is important to preserve
      FOLLOWS/PRECEDES clause if the user has specified it so that the trigger
      execution order on master and slave will be the same.

  @param thd                thread context
  @param[out] binlog_query  well-formed CREATE TRIGGER statement for putting
                            into binlog (after successful execution)
  @param def_user           user part of a definer value
  @param def_host           host part of a definer value

  @return Operation status.
    @retval false Success
    @retval true  Failure
*/

static bool construct_create_trigger_stmt_with_definer(
    THD *thd, String *binlog_query, const LEX_CSTRING &def_user,
    const LEX_CSTRING &def_host) {
  LEX *lex = thd->lex;

  if (binlog_query->append(STRING_WITH_LEN("CREATE "))) return true;  // OOM

  /*
    Append definer-clause if the trigger is SUID (a usual trigger in
    new MySQL versions).
  */

  append_definer(thd, binlog_query, def_user, def_host);

  return binlog_query->append(
      lex->stmt_definition_begin,
      lex->stmt_definition_end - lex->stmt_definition_begin);
}

static const LEX_CSTRING trg_action_time_type_names[] = {
    {STRING_WITH_LEN("BEFORE")}, {STRING_WITH_LEN("AFTER")}};

static const LEX_CSTRING trg_event_type_names[] = {{STRING_WITH_LEN("INSERT")},
                                                   {STRING_WITH_LEN("UPDATE")},
                                                   {STRING_WITH_LEN("DELETE")}};

const LEX_CSTRING &Trigger::get_action_time_as_string() const {
  return trg_action_time_type_names[m_action_time];
}

const LEX_CSTRING &Trigger::get_event_as_string() const {
  return trg_event_type_names[m_event];
}

///////////////////////////////////////////////////////////////////////////

/**
  Creates a new Trigger-instance with the state from the parser. This method is
  used to create a Trigger-object after CREATE TRIGGER statement is parsed.

  @see also Trigger::create_from_dd()

  @param thd                              Thread context with a valid LEX-tree
                                          of CREATE TRIGGER statement
  @param subject_table                    A valid (not fake!) subject
                                          TABLE-object
  @param [out] binlog_create_trigger_stmt Store CREATE TRIGGER appropriate to
                                          writing into the binlog. It should
                                          have DEFINER clause and should not
                                          have FOLLOWS/PRECEDES clause.

  @return Pointer to a new Trigger instance, NULL in case of error.
*/

Trigger *Trigger::create_from_parser(THD *thd, TABLE *subject_table,
                                     String *binlog_create_trigger_stmt) {
  LEX *lex = thd->lex;

  /*
    Fill character set information:
      - client character set contains charset info only;
      - connection collation contains pair {character set, collation};
      - database collation contains pair {character set, collation};

    NOTE: we must allocate strings on Trigger's mem-root.
  */

  LEX_CSTRING client_cs_name;
  LEX_CSTRING connection_cl_name;
  LEX_CSTRING db_cl_name;
  const CHARSET_INFO *default_db_cl = nullptr;

  if (get_default_db_collation(thd, subject_table->s->db.str, &default_db_cl)) {
    DBUG_ASSERT(thd->is_error() || thd->killed);
    return nullptr;
  }

  default_db_cl = default_db_cl ? default_db_cl : thd->collation();

  if (lex_string_strmake(&subject_table->mem_root, &client_cs_name,
                         thd->charset()->csname,
                         strlen(thd->charset()->csname)) ||
      lex_string_strmake(&subject_table->mem_root, &connection_cl_name,
                         thd->variables.collation_connection->name,
                         strlen(thd->variables.collation_connection->name)) ||
      lex_string_strmake(&subject_table->mem_root, &db_cl_name,
                         default_db_cl->name, strlen(default_db_cl->name)))
    return nullptr;

  // Copy trigger name into the proper mem-root.

  LEX_CSTRING trigger_name;
  if (lex_string_strmake(&subject_table->mem_root, &trigger_name,
                         lex->spname->m_name.str, lex->spname->m_name.length))
    return nullptr;

  // Construct two CREATE TRIGGER statements, allocate DEFINER-clause.

  String dd_create_trigger_stmt;
  dd_create_trigger_stmt.set_charset(system_charset_info);

  LEX_CSTRING definer_user, definer_host;

  /* SUID trigger is only supported (DEFINER is specified by the user). */
  DBUG_ASSERT(lex->definer != nullptr);
  definer_user = lex->definer->user;
  definer_host = lex->definer->host;

  if (construct_create_trigger_stmt_with_definer(
          thd, binlog_create_trigger_stmt, definer_user, definer_host))
    return nullptr;

  // Copy CREATE TRIGGER statement for DD into the proper mem-root.

  LEX_CSTRING definition, definition_utf8;
  if (lex_string_strmake(&subject_table->mem_root, &definition,
                         lex->sphead->m_body.str, lex->sphead->m_body.length))
    return nullptr;

  if (lex_string_strmake(&subject_table->mem_root, &definition_utf8,
                         lex->sphead->m_body_utf8.str,
                         lex->sphead->m_body_utf8.length))
    return nullptr;

  // Create a new Trigger instance.

  timeval created_timestamp_not_set = {0, 0};
  Trigger *t = new (&subject_table->mem_root) Trigger(
      trigger_name, &subject_table->mem_root, subject_table->s->db,
      subject_table->s->table_name, definition, definition_utf8,
      thd->variables.sql_mode, definer_user, definer_host, client_cs_name,
      connection_cl_name, db_cl_name, lex->sphead->m_trg_chistics.event,
      lex->sphead->m_trg_chistics.action_time,
      0,  // Unspecified action order. Actual value of action order
          // is maintained by data dictionary.
      created_timestamp_not_set);

  /*
    NOTE: sp-head is not set in the new trigger object. That's Ok since we're
    not going to execute it, but rather use it for store new trigger in the Data
    Dictionary.
  */

  return t;
}

/**
  Creates a new Trigger-instance with the state loaded from the Data Dictionary.

  @note the Data Dictionary currently stores not all needed information, so the
  complete state of Trigger-object can be obtained only after parsing the
  definition (CREATE TRIGGER) statement. In order to do that, Trigger::parse()
  should be called.

  @see also Trigger::create_from_parser()

  @param [in] mem_root             MEM_ROOT for memory allocation.
  @param [in] trigger_name         name of trigger
  @param [in] db_name              name of schema.
  @param [in] subject_table_name   subject table name.
  @param [in] definition           CREATE TRIGGER statement.
  @param [in] definition_utf8      CREATE TRIGGER statement in UTF8.
  @param [in] sql_mode             sql_mode value.
  @param [in] definer_user         user part of a 'definer' value.
  @param [in] definer_host         host part of a 'definer' value.
  @param [in] client_cs_name       client character set name.
  @param [in] connection_cl_name   connection collation name.
  @param [in] db_cl_name           database collation name.
  @param [in] trg_event_type       trigger event type
  @param [in] trg_time_type        trigger action timing
  @param [in] action_order         action order
  @param [in] created_timestamp    trigger creation time stamp.

  @return Pointer to a new Trigger instance, NULL in case of OOM error.
*/

Trigger *Trigger::create_from_dd(
    MEM_ROOT *mem_root, const LEX_CSTRING &trigger_name,
    const LEX_CSTRING &db_name, const LEX_CSTRING &subject_table_name,
    const LEX_CSTRING &definition, const LEX_CSTRING &definition_utf8,
    sql_mode_t sql_mode, const LEX_CSTRING &definer_user,
    const LEX_CSTRING &definer_host, const LEX_CSTRING &client_cs_name,
    const LEX_CSTRING &connection_cl_name, const LEX_CSTRING &db_cl_name,
    enum_trigger_event_type trg_event_type,
    enum_trigger_action_time_type trg_time_type, uint action_order,
    timeval created_timestamp) {
  return new (mem_root)
      Trigger(trigger_name, mem_root, db_name, subject_table_name, definition,
              definition_utf8, sql_mode, definer_user, definer_host,
              client_cs_name, connection_cl_name, db_cl_name, trg_event_type,
              trg_time_type, action_order, created_timestamp);
}

/**
  Trigger constructor.
*/
Trigger::Trigger(
    const LEX_CSTRING &trigger_name, MEM_ROOT *mem_root,
    const LEX_CSTRING &db_name, const LEX_CSTRING &subject_table_name,
    const LEX_CSTRING &definition, const LEX_CSTRING &definition_utf8,
    sql_mode_t sql_mode, const LEX_CSTRING &definer_user,
    const LEX_CSTRING &definer_host, const LEX_CSTRING &client_cs_name,
    const LEX_CSTRING &connection_cl_name, const LEX_CSTRING &db_cl_name,
    enum_trigger_event_type event_type,
    enum_trigger_action_time_type action_time, uint action_order,
    timeval created_timestamp)
    : m_mem_root(mem_root),
      m_db_name(db_name),
      m_subject_table_name(subject_table_name),
      m_definition(definition),
      m_definition_utf8(definition_utf8),
      m_sql_mode(sql_mode),
      m_definer_user(definer_user),
      m_definer_host(definer_host),
      m_client_cs_name(client_cs_name),
      m_connection_cl_name(connection_cl_name),
      m_db_cl_name(db_cl_name),
      m_event(event_type),
      m_action_time(action_time),
      m_action_order(action_order),
      m_trigger_name(trigger_name),
      m_sp(nullptr),
      m_has_parse_error(false) {
  m_created_timestamp = created_timestamp;

  m_parse_error_message[0] = 0;

  construct_definer_value(mem_root, &m_definer, definer_user, definer_host);
}

/**
  Destroy associated SP (if any).
*/
Trigger::~Trigger() { sp_head::destroy(m_sp); }

/**
  Execute trigger's body.

  @param [in] thd   Thread context

  @return Operation status
    @retval true   Trigger execution failed or trigger has compilation errors
    @retval false  Success
*/

bool Trigger::execute(THD *thd) {
  if (m_has_parse_error) return true;

  bool err_status;
  Sub_statement_state statement_state;
  SELECT_LEX *save_current_select;

  thd->reset_sub_statement_state(&statement_state, SUB_STMT_TRIGGER);

  const bool disable_binlog = !thd->variables.sql_log_bin_triggers;
  if (disable_binlog) {
    // option_bits restored back using statement_state in
    // restore_sub_statement_state().
    thd->variables.option_bits &= ~OPTION_BIN_LOG;
  }

  /*
    Reset current_select before call execute_trigger() and
    restore it after return from one. This way error is set
    in case of failure during trigger execution.
  */
  save_current_select = thd->lex->current_select();
  thd->lex->set_current_select(nullptr);
  err_status = m_sp->execute_trigger(thd, m_db_name, m_subject_table_name,
                                     &m_subject_table_grant);
  thd->lex->set_current_select(save_current_select);

  thd->restore_sub_statement_state(&statement_state);

  return err_status;
}

bool Trigger::create_full_trigger_definition(
    const THD *thd, String *full_trg_definition) const {
  bool ret = full_trg_definition->append(STRING_WITH_LEN("CREATE "));
  append_definer(thd, full_trg_definition, get_definer_user(),
                 get_definer_host());
  ret |= full_trg_definition->append(STRING_WITH_LEN("TRIGGER "));
  append_identifier(thd, full_trg_definition, get_trigger_name().str,
                    get_trigger_name().length);
  ret |= full_trg_definition->append(' ');
  ret |= full_trg_definition->append(get_action_time_as_string().str,
                                     get_action_time_as_string().length);
  ret |= full_trg_definition->append(' ');
  ret |= full_trg_definition->append(get_event_as_string().str,
                                     get_event_as_string().length);
  ret |= full_trg_definition->append(STRING_WITH_LEN(" ON "));
  append_identifier(thd, full_trg_definition, get_subject_table_name().str,
                    get_subject_table_name().length);
  ret |= full_trg_definition->append(STRING_WITH_LEN(" FOR EACH ROW "));
  ret |= full_trg_definition->append(get_definition().str,
                                     get_definition().length);
  return ret;
}

/**
  Parse CREATE TRIGGER statement.

  @param [in] thd         Thread context
  @param [in] is_upgrade  Flag to indicate that trigger being parsed is read
                          from .TRG file in case of upgrade.

  @return true if a fatal parse error happened (the parser failed to extract
  even the trigger name), false otherwise (Trigger::has_parse_error() might
  still return true in this case).
*/

bool Trigger::parse(THD *thd, bool is_upgrade) {
  sql_mode_t sql_mode_saved = thd->variables.sql_mode;
  thd->variables.sql_mode = m_sql_mode;

  Parser_state parser_state;
  String full_trigger_definition;

  // Trigger definition contains full trigger statement in .TRG file.
  if (is_upgrade) {
    if (full_trigger_definition.append(get_definition().str,
                                       get_definition().length)) {
      thd->variables.sql_mode = sql_mode_saved;
      return true;
    }
  } else if (create_full_trigger_definition(thd, &full_trigger_definition)) {
    thd->variables.sql_mode = sql_mode_saved;
    return true;
  }

  /*
    Allocate a memory buffer on the memroot and copy there a full trigger
    definition statement.
  */
  if (lex_string_strmake(m_mem_root, &m_full_trigger_definition,
                         full_trigger_definition.c_ptr_quick(),
                         full_trigger_definition.length()))
    return true;

  if (parser_state.init(thd, m_full_trigger_definition.str,
                        m_full_trigger_definition.length)) {
    thd->variables.sql_mode = sql_mode_saved;
    return true;
  }

  LEX *lex_saved = thd->lex;

  LEX lex;
  thd->lex = &lex;
  lex_start(thd);

  LEX_CSTRING current_db_name_saved = thd->db();
  thd->reset_db(m_db_name);

  Deprecated_trigger_syntax_handler error_handler;
  thd->push_internal_handler(&error_handler);

  sp_rcontext *sp_runtime_ctx_saved = thd->sp_runtime_ctx;
  thd->sp_runtime_ctx = nullptr;

  sql_digest_state *digest_saved = thd->m_digest;
  PSI_statement_locker *statement_locker_saved = thd->m_statement_psi;
  thd->m_digest = nullptr;
  thd->m_statement_psi = nullptr;

  Trigger_creation_ctx *creation_ctx = Trigger_creation_ctx::create(
      thd, m_db_name, m_subject_table_name, m_client_cs_name,
      m_connection_cl_name, m_db_cl_name);
  bool parse_error = false;
  if (creation_ctx != nullptr)
    parse_error = parse_sql(thd, &parser_state, creation_ctx);

  thd->m_digest = digest_saved;
  thd->m_statement_psi = statement_locker_saved;
  thd->sp_runtime_ctx = sp_runtime_ctx_saved;
  thd->variables.sql_mode = sql_mode_saved;

  thd->pop_internal_handler();

  bool fatal_error = false;
  if (creation_ctx == nullptr) {
    fatal_error = true;
    goto cleanup;
  }
  /*
    Not strictly necessary to invoke this method here, since we know
    that we've parsed CREATE TRIGGER and not an
    UPDATE/DELETE/INSERT/REPLACE/LOAD/CREATE TABLE, but we try to
    maintain the invariant that this method is called for each
    distinct statement, in case its logic is extended with other
    types of analyses in future.
  */
  lex.set_trg_event_type_for_tables();

  // Ensure that lex.sp_head is NULL in case of parse errors.

  DBUG_ASSERT(!parse_error || (parse_error && lex.sphead == nullptr));

  // That's it in case of parse error.

  if (parse_error) {
    // Remember parse error message.
    set_parse_error_message(error_handler.get_error_message());
    goto cleanup;
  }

  /*
    Set trigger name, event and action time for upgrade scenario.
    .TRG file does not contain these fields explicitly. Their value
    can be determined while parsing the trigger definition.
  */
  if (is_upgrade) {
    // Make a copy of trigger name and set it.
    LEX_CSTRING trigger_name;
    if (lex_string_strmake(m_mem_root, &trigger_name, lex.spname->m_name.str,
                           lex.spname->m_name.length)) {
      fatal_error = true;
      goto cleanup;
    }

    LEX_CSTRING trigger_def;
    if (lex_string_strmake(m_mem_root, &trigger_def, lex.sphead->m_body.str,
                           lex.sphead->m_body.length)) {
      fatal_error = true;
      goto cleanup;
    }

    LEX_CSTRING trigger_def_utf8;
    if (lex_string_strmake(m_mem_root, &trigger_def_utf8,
                           lex.sphead->m_body_utf8.str,
                           lex.sphead->m_body_utf8.length)) {
      fatal_error = true;
      goto cleanup;
    }

    set_trigger_name(trigger_name);
    set_trigger_def(trigger_def);
    set_trigger_def_utf8(trigger_def_utf8);

    // Set correct m_event and m_action_time.
    DBUG_ASSERT(m_event == TRG_EVENT_MAX);
    DBUG_ASSERT(m_action_time == TRG_ACTION_MAX);

    m_event = lex.sphead->m_trg_chistics.event;
    m_action_time = lex.sphead->m_trg_chistics.action_time;
  }

  DBUG_ASSERT(m_event == lex.sphead->m_trg_chistics.event);
  DBUG_ASSERT(m_action_time == lex.sphead->m_trg_chistics.action_time);

  // Take ownership of SP object.

  DBUG_ASSERT(!m_sp);

  m_sp = lex.sphead;
  lex.sphead = nullptr; /* Prevent double cleanup. */

  /*
    Set some SP attributes.

    NOTE: sp_head::set_info() is required on slave.
  */

  m_sp->set_info(0,  // CREATED timestamp (not used for triggers)
                 0,  // MODIFIED timestamp (not used for triggers)
                 &lex.sp_chistics, m_sql_mode);

  DBUG_ASSERT(!m_sp->get_creation_ctx());
  m_sp->set_creation_ctx(creation_ctx);

  /*
    construct_definer_value() that is called from the constructor of
    class Trigger guarantees that the definer has not empty value.
  */
  DBUG_ASSERT(m_definer.length);

  // Set the definer attribute in SP.
  m_sp->set_definer(m_definer.str, m_definer.length);

#ifdef HAVE_PSI_SP_INTERFACE
  m_sp->m_sp_share = MYSQL_GET_SP_SHARE(to_uint(enum_sp_type::TRIGGER),
                                        m_sp->m_db.str, m_sp->m_db.length,
                                        m_sp->m_name.str, m_sp->m_name.length);
#endif

cleanup:
  lex_end(&lex);
  thd->reset_db(current_db_name_saved);
  thd->lex = lex_saved;

  return fatal_error;
}

/**
  Add tables and routines used by trigger to the set of elements
  used by statement.

  @param [in]     thd               thread handle
  @param [in,out] prelocking_ctx    prelocking context of the statement
  @param [in]     table_list        TABLE_LIST for the table
*/

void Trigger::add_tables_and_routines(THD *thd,
                                      Query_tables_list *prelocking_ctx,
                                      TABLE_LIST *table_list) {
  if (has_parse_error()) return;

  if (sp_add_used_routine(prelocking_ctx, thd->stmt_arena,
                          Sroutine_hash_entry::TRIGGER, m_sp->m_db.str,
                          m_sp->m_db.length, m_sp->m_name.str,
                          m_sp->m_name.length,
                          /*
                            Db name should be already in lower case if
                            lower_case_table_name > 0.
                          */
                          false,
                          /*
                            Normalize(lowercase name and remove accent of)
                            trigger name to ensure that we can use binary
                            comparison for Sroutine_hash_entry key.

                            TODO: In 8.0 trigger names are always case and
                            accent insensitive. If we decide to return
                            to 5.7 behavior, where it is dependent on
                            lower-case-table-name value, then we need
                            to pass different value below.
                          */
                          Sp_name_normalize_type::UNACCENT_AND_LOWERCASE_NAME,
                          false,  // This is not "own", directly-used routine.
                          table_list->belong_to_view)) {
    m_sp->add_used_tables_to_table_list(thd, &prelocking_ctx->query_tables_last,
                                        prelocking_ctx->sql_command,
                                        table_list->belong_to_view);
    sp_update_stmt_used_routines(thd, prelocking_ctx, &m_sp->m_sroutines,
                                 table_list->belong_to_view);
    m_sp->propagate_attributes(prelocking_ctx);
  }
}

/**
  Print upgrade warnings (if any).

  @param [in]  thd        Thread handle.
*/

void Trigger::print_upgrade_warning(THD *thd) {
  if (!is_created_timestamp_null()) return;

  push_warning_printf(
      thd, Sql_condition::SL_WARNING, ER_WARN_TRIGGER_DOESNT_HAVE_CREATED,
      ER_THD(thd, ER_WARN_TRIGGER_DOESNT_HAVE_CREATED), get_db_name().str,
      get_subject_table_name().str, get_trigger_name().str);
}
