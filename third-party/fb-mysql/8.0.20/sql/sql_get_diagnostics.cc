/* Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/sql_get_diagnostics.h"

#include <string.h>
#include <sys/types.h>

#include "m_ctype.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysqld_error.h"
#include "sql/item.h"
#include "sql/sp_rcontext.h"  // sp_rcontext
#include "sql/sql_class.h"    // THD
#include "sql/sql_error.h"    // Diagnostics_area
#include "sql/sql_list.h"     // List_iterator
#include "sql_string.h"

/**
  Execute this GET DIAGNOSTICS statement.

  @param thd The current thread.

  @remark Errors or warnings occurring during the execution of the GET
          DIAGNOSTICS statement should not affect the Diagnostics Area
          of a previous statement as the diagnostics information there
          would be wiped out. Thus, in order to preserve the contents
          of the Diagnostics Area from which information is being
          retrieved, the GET DIAGNOSTICS statement is executed under
          a separate Diagnostics Area. If any errors or warnings occur
          during the execution of the GET DIAGNOSTICS statement, these
          error or warnings (conditions) are appended to the list of
          the original Diagnostics Area. The only exception to this is
          fatal errors, which must always cause the statement to fail.

  @retval false on success.
  @retval true on error
*/

bool Sql_cmd_get_diagnostics::execute(THD *thd) {
  bool rc;
  Diagnostics_area new_stmt_da(false);
  Diagnostics_area *first_da = thd->get_stmt_da();
  const Diagnostics_area *second_da = thd->get_stacked_da();
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

  if (m_info->get_which_da() == Diagnostics_information::STACKED_AREA) {
    // STACKED_AREA only allowed inside handlers
    if (!thd->sp_runtime_ctx || !thd->sp_runtime_ctx->current_handler_frame()) {
      my_error(ER_GET_STACKED_DA_WITHOUT_ACTIVE_HANDLER, MYF(ME_FATALERROR));
      rc = true;
    } else
      rc = m_info->aggregate(thd, second_da);
  } else
    rc = m_info->aggregate(thd, first_da);
  thd->pop_diagnostics_area();

  /* Bail out early if statement succeeded. */
  if (!rc) {
    thd->get_stmt_da()->set_ok_status(0, 0, nullptr);
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

  /* Appending might have failed. */
  if (!(rc = thd->is_error())) thd->get_stmt_da()->set_ok_status(0, 0, nullptr);

  return rc;
}

/**
  Set a value for this item.

  @param thd    The current thread.
  @param value  The obtained value.

  @retval false on success.
  @retval true on error.
*/

bool Diagnostics_information_item::set_value(THD *thd, Item **value) {
  bool rc;
  Settable_routine_parameter *srp;
  DBUG_TRACE;

  /* Get a settable reference to the target. */
  srp = m_target->get_settable_routine_parameter();

  DBUG_ASSERT(srp);

  /* Set variable/parameter value. */
  rc = srp->set_value(thd, thd->sp_runtime_ctx, value);

  return rc;
}

/**
  Obtain statement information in the context of a given Diagnostics Area.

  @param thd  The current thread.
  @param da   The Diagnostics Area.

  @retval false on success.
  @retval true on error
*/

bool Statement_information::aggregate(THD *thd, const Diagnostics_area *da) {
  bool rv = false;
  Statement_information_item *stmt_info_item;
  List_iterator<Statement_information_item> it(*m_items);
  DBUG_TRACE;

  /*
    Each specified target gets the value of each given
    information item obtained from the Diagnostics Area.
  */
  while ((stmt_info_item = it++)) {
    if ((rv = evaluate(thd, stmt_info_item, da))) break;
  }

  return rv;
}

/**
  Obtain the value of this statement information item in the context of
  a given Diagnostics Area.

  @param thd  The current thread.
  @param da   The Diagnostics Area.

  @retval Item representing the value.
  @retval NULL on error.
*/

Item *Statement_information_item::get_value(THD *thd,
                                            const Diagnostics_area *da) {
  Item *value = nullptr;
  DBUG_TRACE;

  switch (m_name) {
    /*
      The number of condition areas that have information. That is,
      the number of errors and warnings within the Diagnostics Area.
    */
    case NUMBER: {
      ulong count = da->cond_count();
      value = new (thd->mem_root) Item_uint(count);
      break;
    }
    /*
      Number that shows how many rows were directly affected by
      a data-change statement (INSERT, UPDATE, DELETE, MERGE,
      REPLACE, LOAD).
    */
    case ROW_COUNT:
      value = new (thd->mem_root) Item_int(thd->get_row_count_func());
      break;
  }

  return value;
}

/**
  Obtain condition information in the context of a given Diagnostics Area.

  @param thd  The current thread.
  @param da   The Diagnostics Area.

  @retval false on success.
  @retval true on error
*/

bool Condition_information::aggregate(THD *thd, const Diagnostics_area *da) {
  bool rv = false;
  longlong cond_number;
  const Sql_condition *cond = nullptr;
  Condition_information_item *cond_info_item;
  Diagnostics_area::Sql_condition_iterator it_conds = da->sql_conditions();
  List_iterator_fast<Condition_information_item> it_items(*m_items);
  DBUG_TRACE;

  /* Prepare the expression for evaluation. */
  if (!m_cond_number_expr->fixed &&
      m_cond_number_expr->fix_fields(thd, &m_cond_number_expr))
    return true;

  cond_number = m_cond_number_expr->val_int();

  /*
    Limit to the number of available conditions. Diagnostics_area::warn_count()
    is not used because it indicates the number of condition regardless of
    @@max_error_count, which prevents conditions from being pushed, but not
    counted.
  */
  if (cond_number < 1 || (ulonglong)cond_number > da->cond_count()) {
    my_error(ER_DA_INVALID_CONDITION_NUMBER, MYF(0));
    return true;
  }

  /* Advance to the requested condition. */
  while (cond_number--) cond = it_conds++;

  DBUG_ASSERT(cond);

  /* Evaluate the requested information in the context of the condition. */
  while ((cond_info_item = it_items++)) {
    if ((rv = evaluate(thd, cond_info_item, cond))) break;
  }

  return rv;
}

/**
  Create an UTF-8 string item to represent a condition item string.

  @remark The string might not have a associated charset. For example,
          this can be the case if the server does not or fails to process
          the error message file.

  @remark See "Design notes about Sql_condition::m_message_text." in
  sql_error.cc

  @return Pointer to an string item, NULL on failure.
*/

Item *Condition_information_item::make_utf8_string_item(THD *thd,
                                                        const String *str) {
  /* Default is utf8 character set and utf8_general_ci collation. */
  const CHARSET_INFO *to_cs = &my_charset_utf8_general_ci;
  /* If a charset was not set, assume that no conversion is needed. */
  const CHARSET_INFO *from_cs = str->charset() ? str->charset() : to_cs;
  Item_string *item = new Item_string(str->ptr(), str->length(), from_cs);
  /* If necessary, convert the string (ignoring errors), then copy it over. */
  return item ? item->charset_converter(thd, to_cs, false) : nullptr;
}

/**
  Obtain the value of this condition information item in the context of
  a given condition.

  @param thd  The current thread.
  @param cond  The SQL condition.

  @retval Item representing the value.
  @retval NULL on error.
*/

Item *Condition_information_item::get_value(THD *thd,
                                            const Sql_condition *cond) {
  String str;
  Item *value = nullptr;
  DBUG_TRACE;

  switch (m_name) {
    case CLASS_ORIGIN:
      value = make_utf8_string_item(thd, &(cond->m_class_origin));
      break;
    case SUBCLASS_ORIGIN:
      value = make_utf8_string_item(thd, &(cond->m_subclass_origin));
      break;
    case CONSTRAINT_CATALOG:
      value = make_utf8_string_item(thd, &(cond->m_constraint_catalog));
      break;
    case CONSTRAINT_SCHEMA:
      value = make_utf8_string_item(thd, &(cond->m_constraint_schema));
      break;
    case CONSTRAINT_NAME:
      value = make_utf8_string_item(thd, &(cond->m_constraint_name));
      break;
    case CATALOG_NAME:
      value = make_utf8_string_item(thd, &(cond->m_catalog_name));
      break;
    case SCHEMA_NAME:
      value = make_utf8_string_item(thd, &(cond->m_schema_name));
      break;
    case TABLE_NAME:
      value = make_utf8_string_item(thd, &(cond->m_table_name));
      break;
    case COLUMN_NAME:
      value = make_utf8_string_item(thd, &(cond->m_column_name));
      break;
    case CURSOR_NAME:
      value = make_utf8_string_item(thd, &(cond->m_cursor_name));
      break;
    case MESSAGE_TEXT:
      value = make_utf8_string_item(thd, &(cond->m_message_text));
      break;
    case MYSQL_ERRNO:
      value = new (thd->mem_root) Item_uint(cond->m_mysql_errno);
      break;
    case RETURNED_SQLSTATE:
      str.set_ascii(cond->returned_sqlstate(),
                    strlen(cond->returned_sqlstate()));
      value = make_utf8_string_item(thd, &str);
      break;
  }

  return value;
}
