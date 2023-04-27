/* Copyright (c) 2019, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/sql_check_constraint.h"

#include "libbinlogevents/include/binlog_event.h"  // UNDEFINED_SERVER_VERSION
#include "m_ctype.h"                               // CHARSET_INFO
#include "my_inttypes.h"                           // MYF, uchar
#include "my_sys.h"                                // my_error
#include "mysql/thread_type.h"                     // SYSTEM_THREAD_SLAVE_*
#include "mysql_com.h"                             // NAME_CHAR_LEN
#include "mysqld_error.h"                          // ER_*
#include "sql/create_field.h"                      // Create_field
#include "sql/enum_query_type.h"                   // QT_*
#include "sql/field.h"             // pre_validate_value_generator_expr
#include "sql/item.h"              // Item, Item_field
#include "sql/sql_class.h"         // THD
#include "sql/sql_const.h"         // enum_walk
#include "sql/sql_list.h"          // List
#include "sql/sql_parse.h"         // check_string_char_length
#include "sql/system_variables.h"  // System_variables
#include "sql/thd_raii.h"          // Sql_mode_parse_guard
#include "sql_string.h"            // String

bool Sql_check_constraint_spec::pre_validate() {
  /*
    Validate check constraint expression name. If name is not specified for the
    check constraint then name is generated before calling this method.
  */
  if (check_string_char_length(to_lex_cstring(name), "", NAME_CHAR_LEN,
                               system_charset_info, true)) {
    my_error(ER_TOO_LONG_IDENT, MYF(0), name.str);
    return true;
  }

  /*
    If this is a column check constraint then expression should refer only its
    column.
  */
  if (column_name.length != 0) {
    if (!check_constraint_expr_refers_to_only_column(check_expr,
                                                     column_name.str)) {
      my_error(ER_COLUMN_CHECK_CONSTRAINT_REFERENCES_OTHER_COLUMN, MYF(0),
               name.str);
      return true;
    }
  }

  // Check constraint expression must be a boolean expression.
  if (!check_expr->is_bool_func()) {
    my_error(ER_NON_BOOLEAN_EXPR_FOR_CHECK_CONSTRAINT, MYF(0), name.str);
    return true;
  }

  /*
    Pre-validate expression to determine if it is allowed for the check
    constraint.
  */
  if (pre_validate_value_generator_expr(check_expr, name.str,
                                        VGS_CHECK_CONSTRAINT))
    return true;

  return false;
}

void Sql_check_constraint_spec::print_expr(THD *thd, String &out) {
  out.length(0);
  Sql_mode_parse_guard parse_guard(thd);
  auto flags = enum_query_type(QT_NO_DB | QT_NO_TABLE | QT_FORCE_INTRODUCERS);
  check_expr->print(thd, &out, flags);
}

bool Sql_check_constraint_spec::expr_refers_column(const char *column_name) {
  List<Item_field> fields;
  check_expr->walk(&Item::collect_item_field_processor, enum_walk::POSTFIX,
                   (uchar *)&fields);

  Item_field *cur_item;
  List_iterator<Item_field> fields_it(fields);
  while ((cur_item = fields_it++)) {
    if (cur_item->type() == Item::FIELD_ITEM &&
        !my_strcasecmp(system_charset_info, cur_item->field_name, column_name))
      return true;
  }
  return false;
}

bool is_slave_with_master_without_check_constraints_support(THD *thd) {
  return ((thd->system_thread &
           (SYSTEM_THREAD_SLAVE_SQL | SYSTEM_THREAD_SLAVE_WORKER)) &&
          (thd->variables.original_server_version == UNDEFINED_SERVER_VERSION ||
           thd->variables.original_server_version < 80016));
}

bool check_constraint_expr_refers_to_only_column(Item *check_expr,
                                                 const char *column_name) {
  List<Item_field> fields;
  check_expr->walk(&Item::collect_item_field_processor, enum_walk::POSTFIX,
                   (uchar *)&fields);

  // Expression does not refer to any columns.
  if (fields.elements == 0) return false;

  Item_field *cur_item;
  List_iterator<Item_field> fields_it(fields);
  while ((cur_item = fields_it++)) {
    // Expression refers to some other column.
    if (cur_item->type() == Item::FIELD_ITEM &&
        my_strcasecmp(system_charset_info, cur_item->field_name, column_name))
      return false;
  }
  return true;
}

bool Check_constraint_column_dependency_checker::
    any_check_constraint_uses_column(const char *column_name) {
  auto column_used_by_constraint =
      [column_name](Sql_check_constraint_spec *cc_spec) {
        if (cc_spec->expr_refers_column(column_name)) {
          my_error(ER_DEPENDENT_BY_CHECK_CONSTRAINT, MYF(0), cc_spec->name.str,
                   column_name);
          return true;
        }
        return false;
      };

  return std::any_of(m_check_constraint_list.begin(),
                     m_check_constraint_list.end(), column_used_by_constraint);
}

bool Check_constraint_column_dependency_checker::operator()(
    const Alter_drop *drop) {
  if (drop->type != Alter_drop::COLUMN) return false;
  return any_check_constraint_uses_column(drop->name);
}

bool Check_constraint_column_dependency_checker::operator()(
    const Alter_column *alter_column) {
  if (alter_column->change_type() != Alter_column::Type::RENAME_COLUMN)
    return false;
  if (my_strcasecmp(system_charset_info, alter_column->name,
                    alter_column->m_new_name) == 0)
    return false;
  return any_check_constraint_uses_column(alter_column->name);
}

bool Check_constraint_column_dependency_checker::operator()(
    const Create_field &fld) {
  if (fld.change == nullptr) return false;
  if (my_strcasecmp(system_charset_info, fld.field_name, fld.change) == 0)
    return false;
  return any_check_constraint_uses_column(fld.change);
}
