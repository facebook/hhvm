/* Copyright (c) 2002, 2019, Oracle and/or its affiliates. All rights reserved.

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

/**********************************************************************
This file contains the implementation of error and warnings related

  - Whenever an error or warning occurred, it pushes it to a warning list
    that the user can retrieve with SHOW WARNINGS or SHOW ERRORS.

  - For each statement, we return the number of warnings generated from this
    command.  Note that this can be different from @@warning_count as
    we reset the warning list only for questions that uses a table.
    This is done to allow on to do:
    INSERT ...;
    SELECT @@warning_count;
    SHOW WARNINGS;
    (If we would reset after each command, we could not retrieve the number
     of warnings)

  - When client requests the information using SHOW command, then
    server processes from this list and returns back in the form of
    resultset.

    Supported syntaxes:

    SHOW [COUNT(*)] ERRORS [LIMIT [offset,] rows]
    SHOW [COUNT(*)] WARNINGS [LIMIT [offset,] rows]
    SELECT @@warning_count, @@error_count;

***********************************************************************/

#include "sql/sql_error.h"

#include <float.h>
#include <stdarg.h>
#include <algorithm>

#include "decimal.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "my_sys.h"
#include "my_time.h"
#include "mysql/components/services/log_builtins.h"
#include "mysql/components/services/log_shared.h"
#include "mysql/psi/psi_base.h"
#include "mysql_time.h"
#include "mysqld.h"
#include "mysqld_error.h"
#include "sql/derror.h"  // ER_THD
#include "sql/item.h"
#include "sql/log.h"  // sql_print_warning
#include "sql/my_decimal.h"
#include "sql/protocol.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_const.h"
#include "sql/sql_lex.h"
#include "sql/system_variables.h"
#include "sql/thr_malloc.h"

using std::max;
using std::min;

/*
  Design notes about Sql_condition::m_message_text.

  The member Sql_condition::m_message_text contains the text associated with
  an error, warning or note (which are all SQL 'conditions')

  Producer of Sql_condition::m_message_text:
  ----------------------------------------

  (#1) the server implementation itself, when invoking functions like
  my_error() or push_warning()

  (#2) user code in stored programs, when using the SIGNAL statement.

  (#3) user code in stored programs, when using the RESIGNAL statement.

  When invoking my_error(), the error number and message is typically
  provided like this:
  - my_error(ER_WRONG_DB_NAME, MYF(0), ...);
  - my_message(ER_SLAVE_IGNORED_TABLE, ER(ER_SLAVE_IGNORED_TABLE), MYF(0));

  In both cases, the message is retrieved from ER(ER_XXX), which in turn
  is read from the resource file errmsg.sys at server startup.
  The strings stored in the errmsg.sys file are expressed in the character set
  that corresponds to the server --language start option
  (see error_message_charset_info).

  When executing:
  - a SIGNAL statement,
  - a RESIGNAL statement,
  the message text is provided by the user logic, and is expressed in UTF8.

  Storage of Sql_condition::m_message_text:
  ---------------------------------------

  (#4) The class Sql_condition is used to hold the message text member.
  This class represents a single SQL condition.

  (#5) The class Diagnostics_area contains m_condition_list which
  represents a SQL condition area.

  Consumer of Sql_condition::m_message_text:
  ----------------------------------------

  (#6) The statements SHOW WARNINGS and SHOW ERRORS display the content of
  the warning list.

  (#7) The GET DIAGNOSTICS statement reads the content of:
  - the top level statement condition area (when executed in a query),
  - a sub statement (when executed in a stored program)
  and return the data stored in a Sql_condition.

  (#8) The RESIGNAL statement reads the Sql_condition caught by an exception
  handler, to raise a new or modified condition (in #3).

  The big picture
  ---------------
                                                              --------------
                                                              |            ^
                                                              V            |
  my_error(#1)                 SIGNAL(#2)                 RESIGNAL(#3)     |
      |(#A)                       |(#B)                       |(#C)        |
      |                           |                           |            |
      ----------------------------|----------------------------            |
                                  |                                        |
                                  V                                        |
                           Sql_condition(#4)                               |
                                  |                                        |
                                  |                                        |
                                  V                                        |
                         Diagnostics_area(#5)                              |
                                  |                                        |
          -----------------------------------------------------            |
          |                       |                           |            |
          |                       |                           |            |
          |                       |                           |            |
          V                       V                           V            |
   SHOW WARNINGS(#6)      GET DIAGNOSTICS(#7)              RESIGNAL(#8)    |
          |  |                    |                           |            |
          |  --------             |                           V            |
          |         |             |                           --------------
          V         |             |
      Connectors    |             |
          |         |             |
          -------------------------
                    |
                    V
             Client application

  Current implementation status
  -----------------------------

  (#1) (my_error) produces data in the 'error_message_charset_info' CHARSET

  (#2) and (#3) (SIGNAL, RESIGNAL) produces data internally in UTF8

  (#6) (SHOW WARNINGS) produces data in the 'error_message_charset_info' CHARSET

  (#7) (GET DIAGNOSTICS) is implemented.

  (#8) (RESIGNAL) produces data internally in UTF8 (see #3)

  As a result, the design choice for (#4) and (#5) is to store data in
  the 'error_message_charset_info' CHARSET, to minimize impact on the code base.
  This is implemented by using 'String Sql_condition::m_message_text'.

  The UTF8 -> error_message_charset_info conversion is implemented in
  Sql_cmd_common_signal::eval_signal_informations() (for path #B and #C).

  Future work
  -----------

  - Change (#1) (my_error) to generate errors in UTF8.
    See WL#751 (Recoding of error messages)

  - Change (#4 and #5) to store message text in UTF8 natively.
    In practice, this means changing the type of the message text to
    '<UTF8 String 128 class> Sql_condition::m_message_text', and is a direct
    consequence of WL#751.
*/

static void copy_string(MEM_ROOT *mem_root, String *dst, const String *src) {
  size_t len = src->length();
  if (len) {
    char *copy = (char *)mem_root->Alloc(len + 1);
    if (copy) {
      memcpy(copy, src->ptr(), len);
      copy[len] = '\0';
      dst->set(copy, len, src->charset());
    }
  } else
    dst->length(0);
}

Sql_condition::Sql_condition(MEM_ROOT *mem_root)
    : m_class_origin((const char *)nullptr, 0, &my_charset_utf8_bin),
      m_subclass_origin((const char *)nullptr, 0, &my_charset_utf8_bin),
      m_constraint_catalog((const char *)nullptr, 0, &my_charset_utf8_bin),
      m_constraint_schema((const char *)nullptr, 0, &my_charset_utf8_bin),
      m_constraint_name((const char *)nullptr, 0, &my_charset_utf8_bin),
      m_catalog_name((const char *)nullptr, 0, &my_charset_utf8_bin),
      m_schema_name((const char *)nullptr, 0, &my_charset_utf8_bin),
      m_table_name((const char *)nullptr, 0, &my_charset_utf8_bin),
      m_column_name((const char *)nullptr, 0, &my_charset_utf8_bin),
      m_cursor_name((const char *)nullptr, 0, &my_charset_utf8_bin),
      m_message_text(),
      m_mysql_errno(0),
      m_severity_level(Sql_condition::SL_ERROR),
      m_mem_root(mem_root) {
  DBUG_ASSERT(mem_root != nullptr);
  memset(m_returned_sqlstate, 0, sizeof(m_returned_sqlstate));
}

Sql_condition::Sql_condition(MEM_ROOT *mem_root, uint mysql_errno,
                             const char *returned_sqlstate,
                             Sql_condition::enum_severity_level severity,
                             const char *message_text)
    : m_class_origin((const char *)nullptr, 0, &my_charset_utf8_bin),
      m_subclass_origin((const char *)nullptr, 0, &my_charset_utf8_bin),
      m_constraint_catalog((const char *)nullptr, 0, &my_charset_utf8_bin),
      m_constraint_schema((const char *)nullptr, 0, &my_charset_utf8_bin),
      m_constraint_name((const char *)nullptr, 0, &my_charset_utf8_bin),
      m_catalog_name((const char *)nullptr, 0, &my_charset_utf8_bin),
      m_schema_name((const char *)nullptr, 0, &my_charset_utf8_bin),
      m_table_name((const char *)nullptr, 0, &my_charset_utf8_bin),
      m_column_name((const char *)nullptr, 0, &my_charset_utf8_bin),
      m_cursor_name((const char *)nullptr, 0, &my_charset_utf8_bin),
      m_message_text(),
      m_mysql_errno(mysql_errno),
      m_severity_level(severity),
      m_mem_root(mem_root) {
  DBUG_ASSERT(mem_root != nullptr);
  DBUG_ASSERT(mysql_errno != 0);
  DBUG_ASSERT(returned_sqlstate != nullptr);
  DBUG_ASSERT(message_text != nullptr);

  set_message_text(message_text);
  set_returned_sqlstate(returned_sqlstate);
  set_class_origins();
}

void Sql_condition::copy_opt_attributes(const Sql_condition *cond) {
  DBUG_ASSERT(this != cond);
  copy_string(m_mem_root, &m_class_origin, &cond->m_class_origin);
  copy_string(m_mem_root, &m_subclass_origin, &cond->m_subclass_origin);
  copy_string(m_mem_root, &m_constraint_catalog, &cond->m_constraint_catalog);
  copy_string(m_mem_root, &m_constraint_schema, &cond->m_constraint_schema);
  copy_string(m_mem_root, &m_constraint_name, &cond->m_constraint_name);
  copy_string(m_mem_root, &m_catalog_name, &cond->m_catalog_name);
  copy_string(m_mem_root, &m_schema_name, &cond->m_schema_name);
  copy_string(m_mem_root, &m_table_name, &cond->m_table_name);
  copy_string(m_mem_root, &m_column_name, &cond->m_column_name);
  copy_string(m_mem_root, &m_cursor_name, &cond->m_cursor_name);
}

void Sql_condition::set_message_text(const char *message_text) {
  // See the comments "Design notes about Sql_condition::m_message_text."

  const char *copy = strdup_root(m_mem_root, message_text);
  m_message_text.set(copy, strlen(copy), error_message_charset_info);
  DBUG_ASSERT(!m_message_text.is_alloced());
}

static LEX_CSTRING sqlstate_origin[] = {{STRING_WITH_LEN("ISO 9075")},
                                        {STRING_WITH_LEN("MySQL")}};

void Sql_condition::set_class_origins() {
  char cls[2];

  /* Let CLASS = the first two letters of RETURNED_SQLSTATE. */
  cls[0] = m_returned_sqlstate[0];
  cls[1] = m_returned_sqlstate[1];

  /* Only digits and upper case latin letter are allowed. */
  DBUG_ASSERT(my_isdigit(&my_charset_latin1, cls[0]) ||
              my_isupper(&my_charset_latin1, cls[0]));

  DBUG_ASSERT(my_isdigit(&my_charset_latin1, cls[1]) ||
              my_isupper(&my_charset_latin1, cls[1]));

  /*
    If CLASS[1] is any of: 0 1 2 3 4 A B C D E F G H
    and CLASS[2] is any of: 0-9 A-Z,
    then let CLASS_ORIGIN = 'ISO 9075'. Otherwise 'MySQL'.

    Let SUBCLASS = the next three letters of RETURNED_SQLSTATE.
    If CLASS_ORIGIN = 'ISO 9075' or SUBCLASS = '000',
    then let SUBCLASS_ORIGIN = 'ISO 9075'. Otherwise 'MySQL'.
  */
  if (((cls[0] >= '0' && cls[0] <= '4') || (cls[0] >= 'A' && cls[0] <= 'H')) &&
      ((cls[1] >= '0' && cls[1] <= '9') || (cls[1] >= 'A' && cls[1] <= 'Z'))) {
    // ISO 9075
    m_class_origin.set_ascii(sqlstate_origin[0].str, sqlstate_origin[0].length);
    // ISO 9075
    m_subclass_origin.set_ascii(sqlstate_origin[0].str,
                                sqlstate_origin[0].length);
  } else {
    // MySQL
    m_class_origin.set_ascii(sqlstate_origin[1].str, sqlstate_origin[1].length);
    if (!memcmp(m_returned_sqlstate + 2, STRING_WITH_LEN("000")))
      // ISO 9075
      m_subclass_origin.set_ascii(sqlstate_origin[0].str,
                                  sqlstate_origin[0].length);
    else
      // MySQL
      m_subclass_origin.set_ascii(sqlstate_origin[1].str,
                                  sqlstate_origin[1].length);
  }
}

Diagnostics_area::Diagnostics_area(bool allow_unlimited_conditions)
    : m_stacked_da(nullptr),
      m_is_sent(false),
      m_can_overwrite_status(false),
      m_allow_unlimited_conditions(allow_unlimited_conditions),
      m_status(DA_EMPTY),
      m_mysql_errno(0),
      m_affected_rows(0),
      m_last_insert_id(0),
      m_last_statement_cond_count(0),
      m_current_statement_cond_count(0),
      m_current_row_for_condition(1),
      m_saved_error_count(0),
      m_saved_warn_count(0) {
  /* Initialize sub structures */
  init_sql_alloc(PSI_INSTRUMENT_ME, &m_condition_root, WARN_ALLOC_BLOCK_SIZE,
                 0);
  m_conditions_list.empty();
  memset(m_current_statement_cond_count_by_sl, 0,
         sizeof(m_current_statement_cond_count_by_sl));
  m_message_text[0] = '\0';
}

Diagnostics_area::~Diagnostics_area() { free_root(&m_condition_root, MYF(0)); }

void Diagnostics_area::reset_diagnostics_area() {
  DBUG_TRACE;
#ifdef DBUG_OFF
  set_overwrite_status(false);
  // Don't take chances in production.
  m_message_text[0] = '\0';
  m_mysql_errno = 0;
  m_affected_rows = 0;
  m_last_insert_id = 0;
  m_last_statement_cond_count = 0;
#endif
  set_is_sent(false);
  // Tiny reset in debug mode to see garbage right away.
  m_status = DA_EMPTY;
}

void Diagnostics_area::set_ok_status(ulonglong affected_rows,
                                     ulonglong last_insert_id,
                                     const char *message_text) {
  DBUG_TRACE;
  DBUG_ASSERT(!is_set());
  /*
    In production, refuse to overwrite an error or a custom response
    with an OK packet.
  */
  if (is_error() || is_disabled()) return;

  m_last_statement_cond_count = current_statement_cond_count();
  m_affected_rows = affected_rows;
  m_last_insert_id = last_insert_id;
  if (message_text)
    strmake(m_message_text, message_text, sizeof(m_message_text) - 1);
  else
    m_message_text[0] = '\0';
  m_status = DA_OK;
}

void Diagnostics_area::set_eof_status(THD *thd) {
  DBUG_TRACE;
  /* Only allowed to report eof if has not yet reported an error */
  DBUG_ASSERT(!is_set());
  /*
    In production, refuse to overwrite an error or a custom response
    with an EOF packet.
  */
  if (is_error() || is_disabled()) return;

  /*
    If inside a stored procedure, do not return the total
    number of warnings, since they are not available to the client
    anyway.
  */
  m_last_statement_cond_count =
      (thd->sp_runtime_ctx ? 0 : current_statement_cond_count());

  m_status = DA_EOF;
}

void Diagnostics_area::set_error_status(THD *thd, uint mysql_errno) {
  set_error_status(mysql_errno, ER_THD_NONCONST(thd, mysql_errno),
                   mysql_errno_to_sqlstate(mysql_errno));
}

void Diagnostics_area::set_error_status(uint mysql_errno,
                                        const char *message_text,
                                        const char *returned_sqlstate) {
  DBUG_TRACE;
  /*
    Only allowed to report error if has not yet reported a success
    The only exception is when we flush the message to the client,
    an error can happen during the flush.
  */
  DBUG_ASSERT(!is_set() || m_can_overwrite_status);

  // message must be set properly by the caller.
  DBUG_ASSERT(message_text);

  // sqlstate must be set properly by the caller.
  DBUG_ASSERT(returned_sqlstate);

#ifdef DBUG_OFF
  /*
    In production, refuse to overwrite a custom response with an
    ERROR packet.
  */
  if (is_disabled()) return;
#endif

  m_mysql_errno = mysql_errno;
  memcpy(m_returned_sqlstate, returned_sqlstate, SQLSTATE_LENGTH);
  m_returned_sqlstate[SQLSTATE_LENGTH] = '\0';
  strmake(m_message_text, message_text, sizeof(m_message_text) - 1);

  m_status = DA_ERROR;
}

bool Diagnostics_area::has_sql_condition(const char *message_text,
                                         size_t message_length) const {
  Sql_condition_iterator it(m_conditions_list);
  const Sql_condition *err;

  while ((err = it++)) {
    if (strncmp(message_text, err->message_text(), message_length) == 0)
      return true;
  }

  return false;
}

bool Diagnostics_area::has_sql_condition(uint sql_errno) const {
  Sql_condition_iterator it(m_conditions_list);
  const Sql_condition *err;

  while ((err = it++)) {
    if (err->mysql_errno() == sql_errno) return true;
  }
  return false;
}

const char *Diagnostics_area::get_first_condition_message() {
  if (m_conditions_list.elements())
    return m_conditions_list.front()->message_text();
  return "";
}

void Diagnostics_area::reset_condition_info(THD *thd) {
  /*
    Special case: @@session.error_count, @@session.warning_count
    These appear in non-diagnostics statements (SELECT ... [INTO ...], etc.),
    so we must clear the DA rather than keep it.  To keep these legacy
    system variables working, we save the counts before clearing the
    (rest of the) DA.  The system variables have special getters that access
    the saved values where applicable.
  */
  if (thd->lex->keep_diagnostics == DA_KEEP_COUNTS) {
    m_saved_error_count =
        m_current_statement_cond_count_by_sl[(uint)Sql_condition::SL_ERROR];
    m_saved_warn_count =
        m_current_statement_cond_count_by_sl[(uint)Sql_condition::SL_NOTE] +
        m_current_statement_cond_count_by_sl[(uint)Sql_condition::SL_ERROR] +
        m_current_statement_cond_count_by_sl[(uint)Sql_condition::SL_WARNING];
  }

  m_conditions_list.empty();
  m_preexisting_sql_conditions.empty();
  free_root(&m_condition_root, MYF(0));
  memset(m_current_statement_cond_count_by_sl, 0,
         sizeof(m_current_statement_cond_count_by_sl));
  m_current_statement_cond_count = 0;
  m_current_row_for_condition = 1; /* Start counting from the first row */
}

ulong Diagnostics_area::error_count(THD *thd) const {
  // DA_KEEP_COUNTS: it was SELECT @@error_count, not SHOW COUNT(*) ERRORS
  if (thd->lex->keep_diagnostics == DA_KEEP_COUNTS) return m_saved_error_count;
  return m_current_statement_cond_count_by_sl[(uint)Sql_condition::SL_ERROR];
}

ulong Diagnostics_area::warn_count(THD *thd) const {
  // DA_KEEP_COUNTS: it was SELECT @@warning_count, not SHOW COUNT(*) ERRORS
  if (thd->lex->keep_diagnostics == DA_KEEP_COUNTS) return m_saved_warn_count;
  /*
    This may be higher than warn_list.elements() if we have
    had more warnings than thd->variables.max_error_count.
  */
  return m_current_statement_cond_count_by_sl[(uint)Sql_condition::SL_NOTE] +
         m_current_statement_cond_count_by_sl[(uint)Sql_condition::SL_ERROR] +
         m_current_statement_cond_count_by_sl[(uint)Sql_condition::SL_WARNING];
}

void Diagnostics_area::copy_sql_conditions_from_da(
    THD *thd, const Diagnostics_area *src_da) {
  Sql_condition_iterator it(src_da->m_conditions_list);
  const Sql_condition *err;

  while ((err = it++)) {
    // Do not use ::push_warning() to avoid invocation of THD-internal-handlers.
    Diagnostics_area::push_warning(thd, err);
  }
}

void Diagnostics_area::copy_non_errors_from_da(THD *thd,
                                               const Diagnostics_area *src_da) {
  Sql_condition_iterator it(src_da->m_conditions_list);
  const Sql_condition *cond;

  while ((cond = it++)) {
    if (cond->severity() == Sql_condition::SL_ERROR) continue;

    // Do not use ::push_warning() to avoid invocation of THD-internal-handlers
    Diagnostics_area::push_warning(thd, cond);
  }
}

void Diagnostics_area::mark_preexisting_sql_conditions() {
  Sql_condition_iterator it(m_conditions_list);
  const Sql_condition *cond;

  while ((cond = it++))
    m_preexisting_sql_conditions.push_back(cond, &m_condition_root);
}

void Diagnostics_area::copy_new_sql_conditions(THD *thd,
                                               const Diagnostics_area *src_da) {
  Sql_condition_iterator it(src_da->m_conditions_list);
  const Sql_condition *cond;

  while ((cond = it++)) {
    const bool is_new = std::none_of(
        src_da->m_preexisting_sql_conditions.begin(),
        src_da->m_preexisting_sql_conditions.end(),
        [&](const Sql_condition &preexisting) { return cond == &preexisting; });

    // Do not use ::push_warning() to avoid invocation of THD-internal-handlers
    if (is_new) Diagnostics_area::push_warning(thd, cond);
  }
}

Sql_condition *Diagnostics_area::error_condition() const {
  Sql_condition_list::Iterator it(m_conditions_list);
  Sql_condition *cond;

  while ((cond = it++)) {
    if (cond->mysql_errno() == mysql_errno() &&
        cond->severity() == Sql_condition::SL_ERROR &&
        strcmp(cond->returned_sqlstate(), returned_sqlstate()) == 0)
      return cond;
  }
  return nullptr;
}

void Diagnostics_area::reserve_number_of_conditions(THD *thd, uint count) {
  while (m_conditions_list.elements() &&
         (m_conditions_list.elements() + count) >
             thd->variables.max_error_count)
    m_conditions_list.remove(m_conditions_list.front());
}

Sql_condition *Diagnostics_area::push_warning(
    THD *thd, uint mysql_errno, const char *returned_sqlstate,
    Sql_condition::enum_severity_level severity, const char *message_text) {
  Sql_condition *cond = nullptr;

  if (m_allow_unlimited_conditions ||
      m_conditions_list.elements() < thd->variables.max_error_count) {
    cond = new (&m_condition_root)
        Sql_condition(&m_condition_root, mysql_errno, returned_sqlstate,
                      severity, message_text);
    if (cond) m_conditions_list.push_back(cond);
  }
  m_current_statement_cond_count_by_sl[(uint)severity]++;
  m_current_statement_cond_count++;
  return cond;
}

Sql_condition *Diagnostics_area::push_warning(
    THD *thd, const Sql_condition *sql_condition) {
  Sql_condition *new_condition = push_warning(
      thd, sql_condition->mysql_errno(), sql_condition->returned_sqlstate(),
      sql_condition->severity(), sql_condition->message_text());

  if (new_condition) new_condition->copy_opt_attributes(sql_condition);

  return new_condition;
}

void Diagnostics_area::push_diagnostics_area(THD *thd, Diagnostics_area *da,
                                             bool copy_conditions) {
  DBUG_ASSERT(da->m_stacked_da == nullptr);
  da->m_stacked_da = this;
  if (copy_conditions) {
    da->copy_sql_conditions_from_da(thd, this);
    da->m_saved_warn_count = m_saved_warn_count;
    da->m_saved_error_count = m_saved_error_count;
  }
}

Diagnostics_area *Diagnostics_area::pop_diagnostics_area() {
  DBUG_ASSERT(m_stacked_da);
  Diagnostics_area *da = m_stacked_da;
  m_stacked_da = nullptr;
  return da;
}

/**
  Push the warning to error list if there is still room in the list.

  @param thd           Thread handle
  @param severity      Severity of warning (note, warning)
  @param code          Error number
  @param message_text  Clear error message
*/

void push_warning(THD *thd, Sql_condition::enum_severity_level severity,
                  uint code, const char *message_text) {
  DBUG_TRACE;
  DBUG_PRINT("enter", ("code: %d, msg: %s", code, message_text));

  /*
    Calling push_warning/push_warning_printf with a level of
    SL_ERROR *is* a bug.  Either use my_printf_error(),
    my_error(), or SL_WARNING.
  */
  DBUG_ASSERT(severity != Sql_condition::SL_ERROR);

  if (severity == Sql_condition::SL_ERROR) severity = Sql_condition::SL_WARNING;

  (void)thd->raise_condition(code, nullptr, severity, message_text);
}

void push_warning(THD *thd, uint code) {
  push_warning(thd, Sql_condition::SL_WARNING, code, nullptr);
}

/**
  Push the warning to error list if there is still room in the list

  @param thd      Thread handle
  @param severity Severity of warning (note, warning)
  @param code     Error number
  @param format   Error message printf format
*/

void push_warning_printf(THD *thd, Sql_condition::enum_severity_level severity,
                         uint code, const char *format, ...) {
  va_list args;
  char warning[MYSQL_ERRMSG_SIZE];
  DBUG_TRACE;
  DBUG_PRINT("enter", ("warning: %u", code));

  DBUG_ASSERT(code != 0);
  DBUG_ASSERT(format != nullptr);

  va_start(args, format);
  vsnprintf(warning, sizeof(warning), format, args);
  va_end(args);
  push_warning(thd, severity, code, warning);
}

void push_deprecated_warn(THD *thd, const char *old_syntax,
                          const char *new_syntax) {
  if (!enable_deprecation_warning) return;

  if (thd != nullptr)
    push_warning_printf(
        thd, Sql_condition::SL_WARNING, ER_WARN_DEPRECATED_SYNTAX,
        ER_THD(thd, ER_WARN_DEPRECATED_SYNTAX), old_syntax, new_syntax);
  else
    LogErr(WARNING_LEVEL, ER_DEPRECATED_SYNTAX_WITH_REPLACEMENT, old_syntax,
           new_syntax);
}

void push_deprecated_warn_no_replacement(THD *thd, const char *old_syntax) {
  if (!enable_deprecation_warning) return;

  if (thd != nullptr)
    push_warning_printf(thd, Sql_condition::SL_WARNING,
                        ER_WARN_DEPRECATED_SYNTAX_NO_REPLACEMENT,
                        ER_THD(thd, ER_WARN_DEPRECATED_SYNTAX_NO_REPLACEMENT),
                        old_syntax);
  else
    LogErr(WARNING_LEVEL, ER_DEPRECATED_SYNTAX_NO_REPLACEMENT, old_syntax);
}

const LEX_CSTRING warning_level_names[] = {{STRING_WITH_LEN("Note")},
                                           {STRING_WITH_LEN("Warning")},
                                           {STRING_WITH_LEN("Error")},
                                           {STRING_WITH_LEN("?")}};

/**
  Send all notes, errors or warnings to the client in a result set. The function
  takes into account the current LIMIT.

  @param thd            Thread handler
  @param levels_to_show Bitmap for which levels to show

  @return error status.
*/

bool mysqld_show_warnings(THD *thd, ulong levels_to_show) {
  List<Item> field_list;
  Diagnostics_area new_stmt_da(false);
  Diagnostics_area *first_da = thd->get_stmt_da();
  bool rc = false;
  DBUG_TRACE;

  /* Push new Diagnostics Area, execute statement and pop. */
  thd->push_diagnostics_area(&new_stmt_da);
  /*
    Reset the condition counter.
    This statement has just started and has not generated any conditions
    on its own. However the condition counter will have been updated by
    push_diagnostics_area() to match the number of conditions present in
    first_da. It is therefore necessary to reset so we don't inherit the
    old counter value.
  */
  new_stmt_da.reset_statement_cond_count();

  field_list.push_back(new Item_empty_string("Level", 7));
  field_list.push_back(new Item_return_int("Code", 4, MYSQL_TYPE_LONG));
  field_list.push_back(new Item_empty_string("Message", MYSQL_ERRMSG_SIZE));

  if (thd->send_result_metadata(&field_list,
                                Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF))
    rc = true;

  SELECT_LEX *sel = thd->lex->select_lex;
  SELECT_LEX_UNIT *unit = thd->lex->unit;
  ulonglong idx = 0;
  Protocol *protocol = thd->get_protocol();

  unit->set_limit(thd, sel);

  Diagnostics_area::Sql_condition_iterator it = first_da->sql_conditions();
  const Sql_condition *err = nullptr;
  while (!rc && (err = it++)) {
    /* Skip levels that the user is not interested in */
    if (!(levels_to_show & ((ulong)1 << err->severity()))) continue;
    if (++idx <= unit->offset_limit_cnt) continue;
    if (idx > unit->select_limit_cnt) break;
    protocol->start_row();
    protocol->store_string(warning_level_names[err->severity()].str,
                           warning_level_names[err->severity()].length,
                           system_charset_info);
    protocol->store((uint32)err->mysql_errno());
    protocol->store_string(err->message_text(), err->message_octet_length(),
                           system_charset_info);
    if (protocol->end_row()) rc = true;
  }
  thd->pop_diagnostics_area();

  if (!rc) {
    my_eof(thd);
    return false;
  }

  /* Statement failed, retrieve the error information for propagation. */
  uint sql_errno = new_stmt_da.mysql_errno();
  const char *message = new_stmt_da.message_text();
  const char *sqlstate = new_stmt_da.returned_sqlstate();

  /* In case of a fatal error, set it into the original DA.*/
  if (thd->is_fatal_error()) {
    first_da->set_error_status(sql_errno, message, sqlstate);
    return true;
  }

  /* Otherwise, just append the new error as a exception condition. */
  first_da->push_warning(thd, sql_errno, sqlstate, Sql_condition::SL_ERROR,
                         message);
  return true;
}

ErrConvString::ErrConvString(double nr) {
  // enough to print '-[digits].E+###'
  DBUG_ASSERT(sizeof(err_buffer) > DBL_DIG + 8);
  buf_length =
      my_gcvt(nr, MY_GCVT_ARG_DOUBLE, static_cast<int>(sizeof(err_buffer)) - 1,
              err_buffer, nullptr);
}

ErrConvString::ErrConvString(const my_decimal *nr) {
  int len = sizeof(err_buffer);
  (void)decimal2string(nr, err_buffer, &len);
  buf_length = (uint)len;
}

ErrConvString::ErrConvString(const MYSQL_TIME *ltime, uint dec) {
  buf_length =
      my_TIME_to_str(*ltime, err_buffer, min(dec, uint{DATETIME_MAX_DECIMALS}));
}

/**
   Convert value for dispatch to error message(see WL#751).

   @param buff        buffer for converted string, 0-terminated
   @param to_length   size of the buffer
   @param from        string which should be converted
   @param from_length string length
   @param from_cs     charset from convert

   @retval
   number of bytes written to "to"
*/

size_t err_conv(char *buff, size_t to_length, const char *from,
                size_t from_length, const CHARSET_INFO *from_cs) {
  char *to = buff;
  const char *from_start = from;
  size_t res;

  DBUG_ASSERT(to_length > 0);
  to_length--;
  if (from_cs == &my_charset_bin) {
    uchar char_code;
    res = 0;
    while (true) {
      if ((uint)(from - from_start) >= from_length || res >= to_length) {
        *to = 0;
        break;
      }

      char_code = ((uchar)*from);
      if (char_code >= 0x20 && char_code <= 0x7E) {
        *to++ = char_code;
        from++;
        res++;
      } else {
        if (res + 4 >= to_length) {
          *to = 0;
          break;
        }
        res += snprintf(to, 5, "\\x%02X", (uint)char_code);
        to += 4;
        from++;
      }
    }
  } else {
    uint errors;
    res = copy_and_convert(to, to_length, system_charset_info, from,
                           from_length, from_cs, &errors);
    to += res;
    *to = 0;
  }
  return to - buff;
}

/**
   Convert string for dispatch to client(see WL#751).

   @param to          buffer to convert
   @param to_length   buffer length
   @param to_cs       chraset to convert
   @param from        string from convert
   @param from_length string length
   @param from_cs     charset from convert
   @param errors      count of errors during convertion

   @retval
   length of converted string
*/

size_t convert_error_message(char *to, size_t to_length,
                             const CHARSET_INFO *to_cs, const char *from,
                             size_t from_length, const CHARSET_INFO *from_cs,
                             uint *errors) {
  int cnvres;
  my_wc_t wc;
  const uchar *from_end = (const uchar *)from + from_length;
  char *to_start = to;
  uchar *to_end;
  my_charset_conv_mb_wc mb_wc = from_cs->cset->mb_wc;
  my_charset_conv_wc_mb wc_mb;
  uint error_count = 0;
  size_t length;

  DBUG_ASSERT(to_length > 0);
  /* Make room for the null terminator. */
  to_length--;
  to_end = (uchar *)(to + to_length);

  if (!to_cs || from_cs == to_cs || to_cs == &my_charset_bin) {
    length = min(to_length, from_length);
    memmove(to, from, length);
    to[length] = 0;
    return length;
  }

  wc_mb = to_cs->cset->wc_mb;
  while (true) {
    if ((cnvres = (*mb_wc)(from_cs, &wc, pointer_cast<const uchar *>(from),
                           from_end)) > 0) {
      if (!wc) break;
      from += cnvres;
    } else if (cnvres == MY_CS_ILSEQ) {
      wc = (ulong)(uchar)*from;
      from += 1;
    } else
      break;

    if ((cnvres = (*wc_mb)(to_cs, wc, (uchar *)to, to_end)) > 0)
      to += cnvres;
    else if (cnvres == MY_CS_ILUNI) {
      length =
          (wc <= 0xFFFF) ? 6 /* '\1234' format*/ : 9 /* '\+123456' format*/;
      if ((uchar *)(to + length) >= to_end) break;
      cnvres = snprintf(to, 9, (wc <= 0xFFFF) ? "\\%04X" : "\\+%06X", (uint)wc);
      to += cnvres;
    } else
      break;
  }

  *to = 0;
  *errors = error_count;
  return (uint32)(to - to_start);
}

/**
  Sanity check for SQLSTATEs. The function does not check if it's really an
  existing SQL-state (there are just too many), it just checks string length and
  looks for bad characters.

  @param sqlstate the condition SQLSTATE.

  @retval true if it's ok.
  @retval false if it's bad.
*/

bool is_sqlstate_valid(const LEX_STRING *sqlstate) {
  if (sqlstate->length != 5) return false;

  for (int i = 0; i < 5; ++i) {
    char c = sqlstate->str[i];

    if ((c < '0' || '9' < c) && (c < 'A' || 'Z' < c)) return false;
  }

  return true;
}

/**
  Output warnings on deprecated character sets

  @param [in] thd       The connection handler.
  @param [in] cs        The character set to check for a deprecation.
  @param [in] alias     The name/alias of @c cs.
  @param [in] option    Command line/config file option name, otherwise NULL.
*/
void warn_on_deprecated_charset(THD *thd, const CHARSET_INFO *cs,
                                const char *alias, const char *option) {
  if (cs == &my_charset_utf8_general_ci) {
    if (native_strcasecmp(alias, "utf8") == 0) {
      if (option == nullptr)
        push_warning(thd, ER_DEPRECATED_UTF8_ALIAS);
      else
        LogErr(WARNING_LEVEL, ER_WARN_DEPRECATED_UTF8_ALIAS_OPTION, option);
    } else {
      if (option == nullptr)
        push_deprecated_warn(thd, "utf8mb3", "utf8mb4");
      else
        LogErr(WARNING_LEVEL, ER_WARN_DEPRECATED_UTF8MB3_CHARSET_OPTION,
               option);
    }
  }
}

/**
  Output warnings on deprecated character collations

  @param [in] thd       The connection handler.
  @param [in] collation The collation to check for a deprecation.
  @param [in] option    Command line/config file option name, otherwise NULL.
*/
void warn_on_deprecated_collation(THD *thd, const CHARSET_INFO *collation,
                                  const char *option) {
  if (my_charset_same(collation, &my_charset_utf8_general_ci)) {
    if (option == nullptr)
      push_warning_printf(
          thd, Sql_condition::SL_WARNING, ER_WARN_DEPRECATED_UTF8MB3_COLLATION,
          ER_THD(thd, ER_WARN_DEPRECATED_UTF8MB3_COLLATION), collation->name);
    else
      LogErr(WARNING_LEVEL, ER_WARN_DEPRECATED_UTF8MB3_COLLATION_OPTION, option,
             collation->name);
  }
}
