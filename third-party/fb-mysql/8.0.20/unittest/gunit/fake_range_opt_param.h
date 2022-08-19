/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef FAKE_RANGE_OPT_PARAM
#define FAKE_RANGE_OPT_PARAM

#include <gmock/gmock.h>

#include "sql/opt_range.cc"
#include "unittest/gunit/fake_table.h"

using ::testing::_;
using ::testing::Return;

class Fake_RANGE_OPT_PARAM : public RANGE_OPT_PARAM {
  KEY_PART m_key_parts[64];
  Mem_root_array<KEY_PART_INFO> m_kpis;
  Fake_TABLE fake_table;

 public:
  /**
    Creates a Fake_RANGE_OPT_PARAM and optionally a Fake_TABLE.

    @note The Fake_TABLE is always created, but with zero columns if
    number_columns is zero. However, it won't be used since
    Fake_RANGE_OPT_PARAM::table is NULL.

    @param number_columns If non-zero, a Fake_TABLE is created with
    this many columns.

    @param columns_nullable Creates nullable columns, if applicable.
  */
  Fake_RANGE_OPT_PARAM(THD *thd_arg, MEM_ROOT *alloc_arg, int number_columns,
                       bool columns_nullable)
      : m_kpis(alloc_arg), fake_table(number_columns, columns_nullable) {
    m_kpis.reserve(64);

    thd = thd_arg;
    mem_root = alloc_arg;

    if (number_columns != 0) {
      table = &fake_table;
      current_table = table->pos_in_table_list->map();
    } else {
      table = nullptr;
      current_table = 1;
    }

    using_real_indexes = true;
    key_parts = m_key_parts;
    key_parts_end = m_key_parts;
    keys = 0;
    /*
      Controls whether or not ranges that do not have conditions on
      the first keypart are removed before two trees are ORed in such
      a way that index merge is required. The value of 'true' means
      that such ranges are removed.
    */
    remove_jump_scans = true;

    const Mock_HANDLER *mock_handler = &fake_table.mock_handler;

    ON_CALL(*mock_handler, index_flags(_, _, true))
        .WillByDefault(Return(HA_READ_RANGE));
  }

  void add_key(List<Field> fields_in_index) {
    List_iterator<Field> it(fields_in_index);
    int cur_kp = 0;

    table->key_info[keys].actual_key_parts = 0;
    for (Field *cur_field = it++; cur_field; cur_field = it++, cur_kp++) {
      KEY_PART_INFO *kpi = m_kpis.end();  // Points past the end.
      m_kpis.push_back(KEY_PART_INFO());  // kpi now points to a new element
      kpi->init_from_field(cur_field);

      key_parts_end->key = keys;
      key_parts_end->part = cur_kp;
      key_parts_end->length = kpi->store_length;
      key_parts_end->store_length = kpi->store_length;
      key_parts_end->field = kpi->field;
      key_parts_end->null_bit = kpi->null_bit;
      key_parts_end->flag = static_cast<uint8>(kpi->key_part_flag);
      key_parts_end->image_type = Field::itRAW;

      key_parts_end++;
      table->key_info[keys].key_part[cur_kp] = *kpi;
      table->key_info[keys].actual_key_parts++;
    }
    table->key_info[keys].user_defined_key_parts =
        table->key_info[keys].actual_key_parts;
    real_keynr[keys] = keys;
    keys++;
  }

  void add_key(Field *field_to_index) {
    List<Field> index_list;
    index_list.push_back(field_to_index);
    add_key(index_list);
  }

  void add_key(Field *field_to_index1, Field *field_to_index2) {
    List<Field> index_list;
    index_list.push_back(field_to_index1);
    index_list.push_back(field_to_index2);
    add_key(index_list);
  }

  /// Creates an index over all columns in the RANGE_OPT_PARAM's table.
  void add_key() {
    List<Field> index_list;
    for (uint i = 0; i < table->s->fields; ++i)
      index_list.push_back(table->field[i]);
    add_key(index_list);
  }

  ~Fake_RANGE_OPT_PARAM() {
    for (uint i = 0; i < keys; i++) {
      table->key_info[i].actual_key_parts = 0;
      table->key_info[i].user_defined_key_parts = 0;
    }
  }
};

#endif  // FAKE_RANGE_OPT_PARAM
