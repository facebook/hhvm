/* Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.

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
  @file

  @brief
  join cache optimizations
*/

#include "sql/sql_join_buffer.h"

#include <limits.h>
#include <sys/types.h>
#include <unordered_map>

#include "my_alloc.h"
#include "my_bitmap.h"
#include "my_dbug.h"
#include "sql/field.h"
#include "sql/sql_bitmap.h"
#include "sql/sql_const.h"
#include "sql/sql_executor.h"
#include "sql/table.h"

/**
  Filter base columns of virtual generated columns that might not be read
  by a dynamic range scan.

  A dynamic range scan will read the data from a table using either a
  table scan, a range scan on a covering index, or a range scan on a
  non-covering index. The table's read set contains all columns that
  will be read by the table scan. This might be base columns that are
  used to evaluate virtual column values that are part of an
  index. When the table is read using a table scan, these base columns
  will be read from the storage engine, but when a index/range scan on
  a covering index is used, the base columns will not be read by the
  storage engine. To avoid that these potentially un-read columns are
  inserted into the join buffer, we need to adjust the read set to
  only contain columns that are read independently of which access
  method that is used: these are the only columns needed in the join
  buffer for the query.

  This function does the following manipulations of table's read_set:

  * if one or more of the alternative range scan indexes are covering,
    then the table's read_set is intersected with the read_set for
    each of the covering indexes.

  For potential range indexes that are not covering, no adjustment to
  the read_set is done.

  @note The table->read_set will be changed by this function. It is
  the caller's responsibility to save a copy of this in
  table->tmp_set.

  @param tab the query execution tab
*/

static void filter_gcol_for_dynamic_range_scan(const QEP_TAB *tab) {
  TABLE *table = tab->table();
  DBUG_ASSERT(tab->dynamic_range() && table->vfield);

  for (uint key = 0; key < table->s->keys; ++key) {
    /*
      We only need to consider indexes that are:
      1. Candidates for being used for range scan.
      2. A covering index for the query.
    */
    if (tab->keys().is_set(key) && table->covering_keys.is_set(key)) {
      my_bitmap_map
          bitbuf[(bitmap_buffer_size(MAX_FIELDS) / sizeof(my_bitmap_map)) + 1];
      MY_BITMAP range_read_set;
      bitmap_init(&range_read_set, bitbuf, table->s->fields);

      // Make a bitmap of which fields this covering index can read
      table->mark_columns_used_by_index_no_reset(key, &range_read_set,
                                                 UINT_MAX);

      // Compute the minimal read_set that must be included in the join buffer
      bitmap_intersect(table->read_set, &range_read_set);
    }
  }
}

void filter_virtual_gcol_base_cols(const QEP_TAB *qep_tab) {
  TABLE *table = qep_tab->table();
  if (table->vfield == nullptr) return;

  const uint index = qep_tab->effective_index();
  const bool cov_index =
      index != MAX_KEY && table->index_contains_some_virtual_gcol(index) &&
      /*
        There are two cases:
        - If the table scan uses covering index scan, we can get the value
          of virtual generated column from index
        - If not, JOIN_CACHE only needs the value of virtual generated
          columns (This is why the index can be chosen as a covering index).
          After restore the base columns, the value of virtual generated
          columns can be calculated correctly.
      */
      table->covering_keys.is_set(index);
  if (!(cov_index || qep_tab->dynamic_range())) return;

  if (cov_index) {
    /*
      Save of a copy of table->read_set in save_read_set so that we can
      intersect with it. tmp_set cannot be used as recipient for this as it's
      already used in other parts of JOIN_CACHE::init().
    */
    my_bitmap_map
        bitbuf[(bitmap_buffer_size(MAX_FIELDS) / sizeof(my_bitmap_map)) + 1];
    MY_BITMAP save_read_set;
    bitmap_init(&save_read_set, bitbuf, table->s->fields);
    bitmap_copy(&save_read_set, table->read_set);

    bitmap_clear_all(table->read_set);
    table->mark_columns_used_by_index_no_reset(index, table->read_set);
    if (table->s->primary_key != MAX_KEY)
      table->mark_columns_used_by_index_no_reset(table->s->primary_key,
                                                 table->read_set);
    bitmap_intersect(table->read_set, &save_read_set);
  } else if (qep_tab->dynamic_range()) {
    filter_gcol_for_dynamic_range_scan(qep_tab);
  }
}

void add_virtual_gcol_base_cols(TABLE *table, MEM_ROOT *mem_root,
                                MY_BITMAP *completed_read_set) {
  MY_BITMAP *original_read_set = table->read_set;

  auto bitbuf =
      mem_root->ArrayAlloc<my_bitmap_map>(table->s->column_bitmap_size);
  bitmap_init(completed_read_set, bitbuf, table->s->fields);
  bitmap_copy(completed_read_set, table->read_set);
  table->read_set = completed_read_set;
  for (Field **field_ptr = table->field; *field_ptr != nullptr; ++field_ptr) {
    Field *field = *field_ptr;
    if (bitmap_is_set(table->read_set, field->field_index)) {
      if (field->is_virtual_gcol()) {
        table->mark_gcol_in_maps(field);
      }
    }
  }

  table->read_set = original_read_set;
}
