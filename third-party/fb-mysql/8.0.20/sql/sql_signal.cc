/* Copyright (c) 2008, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/sql_signal.h"

#include <sys/types.h>

#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "my_sys.h"
#include "mysql/components/services/psi_error_bits.h"
#include "mysql/psi/mysql_error.h"
#include "mysqld_error.h"     // ER_*
#include "sql/derror.h"       // ER_THD
#include "sql/item.h"         // Item
#include "sql/sp_pcontext.h"  // sp_condition_value
#include "sql/sp_rcontext.h"  // sp_rcontext
#include "sql/sql_class.h"    // THD
#include "sql/sql_lex.h"
#include "sql_string.h"

struct MEM_ROOT;

/*
  The parser accepts any error code (desired)
  The runtime internally supports any error code (desired)
  The client server protocol is limited to 16 bits error codes (restriction)
  Enforcing the 65535 limit in the runtime until the protocol can change.
*/
#define MAX_MYSQL_ERRNO UINT_MAX16

static const LEX_CSTRING CONDITION_ITEM_NAMES[] = {
    {STRING_WITH_LEN("CLASS_ORIGIN")},
    {STRING_WITH_LEN("SUBCLASS_ORIGIN")},
    {STRING_WITH_LEN("CONSTRAINT_CATALOG")},
    {STRING_WITH_LEN("CONSTRAINT_SCHEMA")},
    {STRING_WITH_LEN("CONSTRAINT_NAME")},
    {STRING_WITH_LEN("CATALOG_NAME")},
    {STRING_WITH_LEN("SCHEMA_NAME")},
    {STRING_WITH_LEN("TABLE_NAME")},
    {STRING_WITH_LEN("COLUMN_NAME")},
    {STRING_WITH_LEN("CURSOR_NAME")},
    {STRING_WITH_LEN("MESSAGE_TEXT")},
    {STRING_WITH_LEN("MYSQL_ERRNO")},

    {STRING_WITH_LEN("CONDITION_IDENTIFIER")},
    {STRING_WITH_LEN("CONDITION_NUMBER")},
    {STRING_WITH_LEN("CONNECTION_NAME")},
    {STRING_WITH_LEN("MESSAGE_LENGTH")},
    {STRING_WITH_LEN("MESSAGE_OCTET_LENGTH")},
    {STRING_WITH_LEN("PARAMETER_MODE")},
    {STRING_WITH_LEN("PARAMETER_NAME")},
    {STRING_WITH_LEN("PARAMETER_ORDINAL_POSITION")},
    {STRING_WITH_LEN("RETURNED_SQLSTATE")},
    {STRING_WITH_LEN("ROUTINE_CATALOG")},
    {STRING_WITH_LEN("ROUTINE_NAME")},
    {STRING_WITH_LEN("ROUTINE_SCHEMA")},
    {STRING_WITH_LEN("SERVER_NAME")},
    {STRING_WITH_LEN("SPECIFIC_NAME")},
    {STRING_WITH_LEN("TRIGGER_CATALOG")},
    {STRING_WITH_LEN("TRIGGER_NAME")},
    {STRING_WITH_LEN("TRIGGER_SCHEMA")}};

bool Set_signal_information::set_item(enum_condition_item_name name,
                                      Item *item) {
  if (m_item[name] != nullptr) {
    my_error(ER_DUP_SIGNAL_SET, MYF(0), CONDITION_ITEM_NAMES[name].str);
    return true;
  }
  m_item[name] = item;
  return false;
}

void Sql_cmd_common_signal::assign_defaults(
    THD *thd MY_ATTRIBUTE((unused)), Sql_condition *cond, bool set_level_code,
    Sql_condition::enum_severity_level level, int sqlcode) {
  if (set_level_code) {
    cond->m_severity_level = level;
    cond->m_mysql_errno = sqlcode;
  }
  if (!cond->message_text())
    cond->set_message_text(ER_THD_NONCONST(thd, sqlcode));
}

void Sql_cmd_common_signal::eval_defaults(THD *thd, Sql_condition *cond) {
  DBUG_ASSERT(cond);

  const char *sqlstate;
  bool set_defaults = (m_cond != nullptr);

  if (set_defaults) {
    /*
      SIGNAL is restricted in sql_yacc.yy to only signal SQLSTATE conditions.
    */
    DBUG_ASSERT(m_cond->type == sp_condition_value::SQLSTATE);
    sqlstate = m_cond->sql_state;
    cond->set_returned_sqlstate(sqlstate);
  } else
    sqlstate = cond->returned_sqlstate();

  DBUG_ASSERT(sqlstate);
  /* SQLSTATE class "00": illegal, rejected in the parser. */
  DBUG_ASSERT(!is_sqlstate_completion(sqlstate));

  if (is_sqlstate_warning(sqlstate)) {
    /* SQLSTATE class "01": warning. */
    assign_defaults(thd, cond, set_defaults, Sql_condition::SL_WARNING,
                    ER_SIGNAL_WARN);
  } else if (is_sqlstate_not_found(sqlstate)) {
    /* SQLSTATE class "02": not found. */
    assign_defaults(thd, cond, set_defaults, Sql_condition::SL_ERROR,
                    ER_SIGNAL_NOT_FOUND);
  } else {
    /* other SQLSTATE classes : error. */
    assign_defaults(thd, cond, set_defaults, Sql_condition::SL_ERROR,
                    ER_SIGNAL_EXCEPTION);
  }
}

static bool assign_fixed_string(MEM_ROOT *mem_root, CHARSET_INFO *dst_cs,
                                size_t max_char, String *dst,
                                const String *src) {
  bool truncated;
  size_t numchars;
  const CHARSET_INFO *src_cs;
  const char *src_str;
  const char *src_end;
  size_t src_len;
  size_t to_copy;
  char *dst_str;
  size_t dst_len;
  size_t dst_copied;
  size_t dummy_offset;

  src_str = src->ptr();
  if (src_str == nullptr) {
    dst->set((const char *)nullptr, 0, dst_cs);
    return false;
  }

  src_cs = src->charset();
  src_len = src->length();
  src_end = src_str + src_len;
  numchars = src_cs->cset->numchars(src_cs, src_str, src_end);

  if (numchars <= max_char) {
    to_copy = src->length();
    truncated = false;
  } else {
    numchars = max_char;
    to_copy = dst_cs->cset->charpos(dst_cs, src_str, src_end, numchars);
    truncated = true;
  }

  if (String::needs_conversion(to_copy, src_cs, dst_cs, &dummy_offset)) {
    dst_len = numchars * dst_cs->mbmaxlen;
    dst_str = (char *)mem_root->Alloc(dst_len + 1);
    if (dst_str) {
      const char *well_formed_error_pos;
      const char *cannot_convert_error_pos;
      const char *from_end_pos;

      dst_copied = well_formed_copy_nchars(
          dst_cs, dst_str, dst_len, src_cs, src_str, src_len, numchars,
          &well_formed_error_pos, &cannot_convert_error_pos, &from_end_pos);
      DBUG_ASSERT(dst_copied <= dst_len);
      dst_len = dst_copied; /* In case the copy truncated the data */
      dst_str[dst_copied] = '\0';
    }
  } else {
    dst_len = to_copy;
    dst_str = (char *)mem_root->Alloc(dst_len + 1);
    if (dst_str) {
      memcpy(dst_str, src_str, to_copy);
      dst_str[to_copy] = '\0';
    }
  }
  dst->set(dst_str, dst_len, dst_cs);

  return truncated;
}

static int assign_condition_item(MEM_ROOT *mem_root, const char *name, THD *thd,
                                 Item *set, String *ci) {
  char str_buff[(64 + 1) * 4]; /* Room for a null terminated UTF8 String 64 */
  String str_value(str_buff, sizeof(str_buff), &my_charset_utf8_bin);
  String *str;
  bool truncated;

  DBUG_TRACE;

  if (set->is_null()) {
    thd->raise_error_printf(ER_WRONG_VALUE_FOR_VAR, name, "NULL");
    return 1;
  }

  str = set->val_str(&str_value);
  truncated = assign_fixed_string(mem_root, &my_charset_utf8_bin, 64, ci, str);
  if (truncated) {
    if (thd->is_strict_sql_mode()) {
      thd->raise_error_printf(ER_COND_ITEM_TOO_LONG, name);
      return 1;
    }

    thd->raise_warning_printf(WARN_COND_ITEM_TRUNCATED, name);
  }

  return 0;
}

int Sql_cmd_common_signal::eval_signal_informations(THD *thd,
                                                    Sql_condition *cond) {
  struct cond_item_map {
    enum_condition_item_name m_item;
    String Sql_condition::*m_member;
  };

  static cond_item_map map[] = {
      {CIN_CLASS_ORIGIN, &Sql_condition::m_class_origin},
      {CIN_SUBCLASS_ORIGIN, &Sql_condition::m_subclass_origin},
      {CIN_CONSTRAINT_CATALOG, &Sql_condition::m_constraint_catalog},
      {CIN_CONSTRAINT_SCHEMA, &Sql_condition::m_constraint_schema},
      {CIN_CONSTRAINT_NAME, &Sql_condition::m_constraint_name},
      {CIN_CATALOG_NAME, &Sql_condition::m_catalog_name},
      {CIN_SCHEMA_NAME, &Sql_condition::m_schema_name},
      {CIN_TABLE_NAME, &Sql_condition::m_table_name},
      {CIN_COLUMN_NAME, &Sql_condition::m_column_name},
      {CIN_CURSOR_NAME, &Sql_condition::m_cursor_name}};

  Item *set;
  String str_value;
  String *str;
  int i;
  uint j;
  int result = 1;
  enum_condition_item_name item_enum;
  String *member;
  const LEX_CSTRING *name;

  DBUG_TRACE;

  for (i = CIN_FIRST_PROPERTY; i <= CIN_LAST_PROPERTY; i++) {
    set = m_set_signal_information->m_item[i];
    if (set) {
      if (!set->fixed) {
        if (set->fix_fields(thd, &set)) goto end;
        m_set_signal_information->m_item[i] = set;
      }
    }
  }

  /*
    Generically assign all the UTF8 String 64 condition items
    described in the map.
  */
  for (j = 0; j < array_elements(map); j++) {
    item_enum = map[j].m_item;
    set = m_set_signal_information->m_item[item_enum];
    if (set != nullptr) {
      member = &(cond->*map[j].m_member);
      name = &CONDITION_ITEM_NAMES[item_enum];
      if (assign_condition_item(cond->m_mem_root, name->str, thd, set, member))
        goto end;
    }
  }

  /*
    Assign the remaining attributes.
  */

  set = m_set_signal_information->m_item[CIN_MESSAGE_TEXT];
  if (set != nullptr) {
    if (set->is_null()) {
      thd->raise_error_printf(ER_WRONG_VALUE_FOR_VAR, "MESSAGE_TEXT", "NULL");
      goto end;
    }
    /*
      Enforce that SET MESSAGE_TEXT = <value> evaluates the value
      as VARCHAR(128) CHARACTER SET UTF8.
    */
    bool truncated;
    String utf8_text;
    str = set->val_str(&str_value);
    truncated = assign_fixed_string(thd->mem_root, &my_charset_utf8_bin, 128,
                                    &utf8_text, str);
    if (truncated) {
      if (thd->is_strict_sql_mode()) {
        thd->raise_error_printf(ER_COND_ITEM_TOO_LONG, "MESSAGE_TEXT");
        goto end;
      }

      thd->raise_warning_printf(WARN_COND_ITEM_TRUNCATED, "MESSAGE_TEXT");
    }

    /*
      See the comments
       "Design notes about Sql_condition::m_message_text."
      in file sql_error.cc
    */
    String converted_text;
    converted_text.set_charset(error_message_charset_info);
    converted_text.append(utf8_text.ptr(), utf8_text.length(),
                          utf8_text.charset());
    cond->set_message_text(converted_text.c_ptr_safe());
  }

  set = m_set_signal_information->m_item[CIN_MYSQL_ERRNO];
  if (set != nullptr) {
    if (set->is_null()) {
      thd->raise_error_printf(ER_WRONG_VALUE_FOR_VAR, "MYSQL_ERRNO", "NULL");
      goto end;
    }
    longlong code = set->val_int();
    if ((code <= 0) || (code > MAX_MYSQL_ERRNO)) {
      str = set->val_str(&str_value);
      thd->raise_error_printf(ER_WRONG_VALUE_FOR_VAR, "MYSQL_ERRNO",
                              str->c_ptr_safe());
      goto end;
    }
    cond->m_mysql_errno = (int)code;
  }

  /*
    The various item->val_xxx() methods don't return an error code,
    but flag thd in case of failure.
  */
  if (!thd->is_error()) result = 0;

end:
  for (i = CIN_FIRST_PROPERTY; i <= CIN_LAST_PROPERTY; i++) {
    set = m_set_signal_information->m_item[i];
    if (set) {
      if (set->fixed) set->cleanup();
    }
  }

  return result;
}

bool Sql_cmd_signal::execute(THD *thd) {
  Sql_condition cond(thd->mem_root);

  DBUG_TRACE;

  /*
    WL#2110 SIGNAL specification says:

      When SIGNAL is executed, it has five effects, in the following order:

        (1) First, the Diagnostics Area is completely cleared. So if the
        SIGNAL is in a DECLARE HANDLER then any pending errors or warnings
        are gone. So is 'row count'.

    This has roots in the SQL standard specification for SIGNAL.
  */

  thd->get_stmt_da()->reset_diagnostics_area();
  thd->set_row_count_func(0);
  thd->get_stmt_da()->reset_condition_info(thd);

  DBUG_ASSERT(thd->lex->query_tables == nullptr);

  eval_defaults(thd, &cond);
  if (eval_signal_informations(thd, &cond)) return true;

  /* SIGNAL should not signal SL_NOTE */
  DBUG_ASSERT((cond.severity() == Sql_condition::SL_WARNING) ||
              (cond.severity() == Sql_condition::SL_ERROR));

  Sql_condition *raised =
      thd->raise_condition(cond.mysql_errno(), cond.returned_sqlstate(),
                           cond.severity(), cond.message_text());
  if (raised) {
    raised->copy_opt_attributes(&cond);
  }

  if (cond.severity() == Sql_condition::SL_WARNING) {
    my_ok(thd);
    return false;
  }

  return true;
}

/**
  Execute RESIGNAL SQL-statement.

  @param thd Thread context.

  @return Error status
  @retval true  in case of error
  @retval false on success
*/

bool Sql_cmd_resignal::execute(THD *thd) {
  sp_rcontext::Handler_call_frame *frame = nullptr;

  DBUG_TRACE;

  if (!thd->sp_runtime_ctx ||
      !(frame = thd->sp_runtime_ctx->current_handler_frame())) {
    thd->raise_error(ER_RESIGNAL_WITHOUT_ACTIVE_HANDLER);
    return true;
  }

  thd->pop_diagnostics_area();

  Diagnostics_area *da = thd->get_stmt_da();

  // allow set_error_status(), in raise_condition() or here.
  da->reset_diagnostics_area();

  Sql_condition *raised = nullptr;

  // RESIGNAL with signal_value.
  if (m_cond) {
    // Make a temporary Sql_condition for modification
    Sql_condition signaled_err(
        thd->mem_root, frame->sql_condition->mysql_errno(),
        frame->sql_condition->returned_sqlstate(),
        frame->sql_condition->severity(), frame->sql_condition->message_text());

    eval_defaults(thd, &signaled_err);
    if (!eval_signal_informations(thd, &signaled_err)) {
      // Make room for the new RESIGNAL condition.
      da->reserve_number_of_conditions(thd, 1);

      raised = thd->raise_condition(
          signaled_err.mysql_errno(), signaled_err.returned_sqlstate(),
          signaled_err.severity(), signaled_err.message_text());
      if (raised) {
        raised->copy_opt_attributes(&signaled_err);
      }
    }
  } else  // RESIGNAL modifying an existing condition.
  {
    /*
      Get the raised condition from Handler_call_frame so it can be
      modified directly.
      Note: This condition might not be in the Diagnostics Area
      if it was full when the condition was raised or if
      Diagnostics_area::set_error_status() was used directly.
    */
    raised = frame->sql_condition;
    eval_defaults(thd, raised);
    if (!eval_signal_informations(thd, raised)) {
      if (raised->severity() == Sql_condition::SL_ERROR)
        da->set_error_status(raised->mysql_errno(), raised->message_text(),
                             raised->returned_sqlstate());
    }
    MYSQL_LOG_ERROR(raised->mysql_errno(), PSI_ERROR_OPERATION_RAISED);
  }

  // RESIGNAL should not resignal SL_NOTE
  DBUG_ASSERT(!raised || (raised->severity() == Sql_condition::SL_WARNING) ||
              (raised->severity() == Sql_condition::SL_ERROR));

  /*
    We now have the following possibilities:
    1) We RESIGNAL a warning
    2) We RESIGNAL an error for which there is another CONTINUE handler.
    3) We RESIGNAL an error for which there is another EXIT handler.
    4) We RESIGNAL an error for which there is no appropriate handler.

    In 1) and 2) we will continue executing the handler. Therefore we
    should add the Diagnostics Area we just popped, back. By doing
    pop+push rather than nothing, we make sure that the condition modified
    by RESIGNAL (if any) is modified on both the first and the second
    Diagnostics Area (not just in the first).

    For 3) we will anyway pop the handler frame, thus also popping the
    handler DA and we should also add it back.

    For 4) we could avoid pushing the DA back, but it's hard to detect
    here if we are in fact in situation 4). Therefore it's easiest to
    just add the DA back. It will in any case be popped by the SP
    exit code.
  */
  frame->handler_da.reset_condition_info(thd);
  frame->handler_da.reset_diagnostics_area();
  thd->push_diagnostics_area(&frame->handler_da);

  // Transfer any exception condition information.
  if (da->is_error())
    frame->handler_da.set_error_status(da->mysql_errno(), da->message_text(),
                                       da->returned_sqlstate());

  /*
    Reset the DA which was used during RESIGNAL execution.
    This prepares it for the statement which will be executed after
    next pop.
  */
  da->reset_diagnostics_area();

  return thd->is_error();
}
