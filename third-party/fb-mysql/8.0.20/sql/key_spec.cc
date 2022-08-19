/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/key_spec.h"

#include <stddef.h>
#include <algorithm>
#include <cstring>
#include <string>

#include "m_ctype.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/create_field.h"   // Create_field
#include "sql/dd/dd.h"          // dd::get_dictionary
#include "sql/dd/dictionary.h"  // dd::Dictionary::check_dd...
#include "sql/derror.h"         // ER_THD
#include "sql/item.h"           // Item_field
#include "sql/item_func.h"      // Item_field
#include "sql/sql_class.h"      // THD
#include "sql/sql_parse.h"      // check_string_char_length
#include "sql/table.h"
#include "sql_lex.h"  // LEX

KEY_CREATE_INFO default_key_create_info;

bool Key_part_spec::operator==(const Key_part_spec &other) const {
  return get_prefix_length() == other.get_prefix_length() &&
         is_ascending() == other.is_ascending() &&
         !my_strcasecmp(system_charset_info, get_field_name(),
                        other.get_field_name());
}

bool foreign_key_prefix(const Key_spec *a, const Key_spec *b) {
  /* Ensure that 'a' is the generated key */
  if (a->generated) {
    if (b->generated && a->columns.size() > b->columns.size())
      std::swap(a, b);  // Put shorter key in 'a'
  } else {
    if (!b->generated) return true;  // No foreign key
    std::swap(a, b);                 // Put generated key in 'a'
  }

  /* Test if 'a' is a prefix of 'b' */
  if (a->columns.size() > b->columns.size()) return true;  // Can't be prefix

#ifdef ENABLE_WHEN_INNODB_CAN_HANDLE_SWAPED_FOREIGN_KEY_COLUMNS
  while ((col1 = col_it1++)) {
    bool found = false;
    col_it2.rewind();
    while ((col2 = col_it2++)) {
      if (*col1 == *col2) {
        found = true;
        break;
      }
    }
    if (!found) return true;  // Error
  }
  return false;  // Is prefix
#else
  for (size_t i = 0; i < a->columns.size(); i++) {
    if (!(*(a->columns[i]) == (*b->columns[i]))) return true;
  }
  return false;  // Is prefix
#endif
}

bool Key_part_spec::resolve_expression(THD *thd) {
  DBUG_ASSERT(has_expression());
  if (get_expression()->fixed) {
    return false;
  }

  get_expression()->allow_array_cast();
  return get_expression()->fix_fields(thd, &m_expression);
}

void Key_part_spec::set_name_and_prefix_length(const char *name,
                                               uint prefix_length) {
  m_prefix_length = prefix_length;
  m_field_name = name;
}

bool Foreign_key_spec::validate(THD *thd, const char *table_name,
                                List<Create_field> &table_fields) const {
  DBUG_TRACE;

  // Reject FKs to inaccessible DD tables.
  const dd::Dictionary *dictionary = dd::get_dictionary();
  if (dictionary && !dictionary->is_dd_table_access_allowed(
                        thd->is_dd_system_thread(), true, ref_db.str,
                        ref_db.length, ref_table.str)) {
    my_error(ER_NO_SYSTEM_TABLE_ACCESS, MYF(0),
             ER_THD_NONCONST(thd, dictionary->table_type_error_code(
                                      ref_db.str, ref_table.str)),
             ref_db.str, ref_table.str);
    return true;
  }

  Create_field *sql_field;
  List_iterator<Create_field> it(table_fields);
  if (ref_columns.size() != columns.size()) {
    my_error(ER_WRONG_FK_DEF, MYF(0),
             (has_explicit_name ? name.str : "foreign key without name"),
             ER_THD(thd, ER_KEY_REF_DO_NOT_MATCH_TABLE_REF));
    return true;
  }
  for (const Key_part_spec *column : columns) {
    // Index prefixes on foreign keys columns are not supported.
    if (column->get_prefix_length() > 0) {
      my_error(ER_CANNOT_ADD_FOREIGN, MYF(0), table_name);
      return true;
    }

    it.rewind();
    while ((sql_field = it++) &&
           my_strcasecmp(system_charset_info, column->get_field_name(),
                         sql_field->field_name)) {
    }
    if (!sql_field) {
      my_error(ER_KEY_COLUMN_DOES_NOT_EXITS, MYF(0), column->get_field_name());
      return true;
    }
    if (sql_field->gcol_info) {
      if (delete_opt == FK_OPTION_SET_NULL) {
        my_error(ER_WRONG_FK_OPTION_FOR_GENERATED_COLUMN, MYF(0),
                 "ON DELETE SET NULL");
        return true;
      }
      if (update_opt == FK_OPTION_SET_NULL) {
        my_error(ER_WRONG_FK_OPTION_FOR_GENERATED_COLUMN, MYF(0),
                 "ON UPDATE SET NULL");
        return true;
      }
      if (update_opt == FK_OPTION_CASCADE) {
        my_error(ER_WRONG_FK_OPTION_FOR_GENERATED_COLUMN, MYF(0),
                 "ON UPDATE CASCADE");
        return true;
      }
    }
  }

  for (const Key_part_spec *fk_col : ref_columns) {
    if (check_column_name(fk_col->get_field_name())) {
      my_error(ER_WRONG_COLUMN_NAME, MYF(0), fk_col->get_field_name());
      return true;
    }
  }

  return false;
}
