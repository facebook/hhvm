/* Copyright (c) 2000, 2019, Oracle and/or its affiliates. All rights reserved.

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

/**
  @file sql_data_change.cc

  Contains classes representing SQL-data change statements. Currently
  the only data change functionality implemented here is function
  defaults.
*/

#include "sql/sql_data_change.h"

#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"
#include "sql/current_thd.h"
#include "sql/field.h"
#include "sql/item.h"
#include "sql/sql_class.h"  // THD
#include "sql/sql_list.h"
#include "sql/table.h"  // TABLE

/**
   Allocates and initializes a MY_BITMAP bitmap, containing one bit per column
   in the table. The table THD's MEM_ROOT is used to allocate memory.

   @param      table   The table whose columns should be used as a template
                       for the bitmap.
   @param[out] bitmap  A pointer to the allocated bitmap.

   @retval false Success.
   @retval true Memory allocation error.
*/
static bool allocate_column_bitmap(TABLE *table, MY_BITMAP **bitmap) {
  DBUG_TRACE;
  const uint number_bits = table->s->fields;
  MY_BITMAP *the_struct;
  my_bitmap_map *the_bits;

  DBUG_ASSERT(current_thd == table->in_use);
  if (multi_alloc_root(table->in_use->mem_root, &the_struct, sizeof(MY_BITMAP),
                       &the_bits, bitmap_buffer_size(number_bits),
                       NULL) == nullptr)
    return true;

  if (bitmap_init(the_struct, the_bits, number_bits) != 0) return true;

  *bitmap = the_struct;

  return false;
}

bool COPY_INFO::get_function_default_columns(TABLE *table) {
  DBUG_TRACE;

  if (m_function_default_columns != nullptr) return false;

  if (allocate_column_bitmap(table, &m_function_default_columns)) return true;

  if (!m_manage_defaults) return false;  // leave bitmap full of zeroes

  /*
    Find columns with function default on insert or update, mark them in
    bitmap.
  */
  for (uint i = 0; i < table->s->fields; ++i) {
    Field *f = table->field[i];
    if ((m_optype == INSERT_OPERATION &&
         f->has_insert_default_datetime_value_expression()) ||
        (m_optype == UPDATE_OPERATION &&
         f->has_update_default_datetime_value_expression()))
      bitmap_set_bit(m_function_default_columns, f->field_index);
    // if it's a default expression also mark the columns it reads
    if (m_optype == INSERT_OPERATION &&
        f->has_insert_default_general_value_expression()) {
      bitmap_set_bit(m_function_default_columns, f->field_index);
      for (uint j = 0; j < table->s->fields; j++) {
        if (bitmap_is_set(&f->m_default_val_expr->base_columns_map, j)) {
          bitmap_set_bit(table->read_set, j);
        }
      }
    }
  }

  if (bitmap_is_clear_all(m_function_default_columns))
    return false;  // no bit set, next step unneeded

  /*
    Remove explicitly assigned columns from the bitmap. The assignment
    target (lvalue) may not always be a column (Item_field), e.g. we could
    be inserting into a view, whose column is actually a base table's column
    converted with COLLATE: the lvalue would then be an
    Item_func_set_collation.
    If the lvalue is an expression tree, we clear all columns in it from the
    bitmap.
  */
  List<Item> *all_changed_columns[2] = {m_changed_columns, m_changed_columns2};
  for (uint i = 0; i < 2; i++) {
    if (all_changed_columns[i] != nullptr) {
      List_iterator<Item> lvalue_it(*all_changed_columns[i]);
      Item *lvalue_item;
      while ((lvalue_item = lvalue_it++) != nullptr)
        lvalue_item->walk(
            &Item::remove_column_from_bitmap, enum_walk::SUBQUERY_POSTFIX,
            reinterpret_cast<uchar *>(m_function_default_columns));
    }
  }

  return false;
}

bool COPY_INFO::set_function_defaults(TABLE *table) {
  DBUG_TRACE;

  DBUG_ASSERT(m_function_default_columns != nullptr);

  /* Quick reject test for checking the case when no defaults are invoked. */
  if (bitmap_is_clear_all(m_function_default_columns)) return false;

  for (uint i = 0; i < table->s->fields; ++i)
    if (bitmap_is_set(m_function_default_columns, i)) {
      DBUG_ASSERT(bitmap_is_set(table->write_set, i));
      switch (m_optype) {
        case INSERT_OPERATION:
          table->field[i]->evaluate_insert_default_function();
          break;
        case UPDATE_OPERATION:
          table->field[i]->evaluate_update_default_function();
          break;
      }
      // If there was an error while executing the default expression
      if (table->in_use->is_error()) return true;
    }

  /**
    @todo combine this call to update_generated_write_fields() with the
    one in fill_record() to avoid updating virtual generated fields twice.
    blobs_need_not_keep_old_value() is called to unset the m_keep_old_value
    flag. Allowing this flag to remain might interfere with the way the old
    BLOB value is handled. When update_generated_write_fields() is removed,
    blobs_need_not_keep_old_value() can also be removed.
  */
  bool res = false;
  if (table->has_gcol()) {
    table->blobs_need_not_keep_old_value();
    res = update_generated_write_fields(table->write_set, table);
  }

  return res;
}

bool COPY_INFO::ignore_last_columns(TABLE *table, uint count) {
  if (get_function_default_columns(table)) return true;
  for (uint i = 0; i < count; i++)
    bitmap_clear_bit(m_function_default_columns, table->s->fields - 1 - i);
  return false;
}
