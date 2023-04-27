/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA  */

#ifndef TABLE_HELPER_H
#define TABLE_HELPER_H

#include <gtest/gtest.h>
#include <array>
#include <cstring>
#include <vector>

#include "my_alloc.h"
#include "my_inttypes.h"
#include "sql/current_thd.h"
#include "sql/field.h"
#include "sql/handler.h"
#include "sql/key.h"
#include "sql/sql_class.h"
#include "sql/table.h"
#include "unittest/gunit/mock_field_long.h"
#include "unittest/gunit/temptable/mock_field_varstring.h"

namespace temptable_test {

class Table_helper {
 public:
  Table_helper(const char *name, THD *thd);

  void add_field_long(const char *name, bool is_nullable);

  void add_field_varstring(const char *name, uint char_len, bool is_nullable);

  void add_index(enum ha_key_alg algorithm, bool unique,
                 const std::vector<int> &columns);

  void finalize();

  void set_handler(handler *handler);

  TABLE *table();

  TABLE_SHARE *table_share();

  uchar *record_0();

  uchar *record_1();

  void copy_record_1_to_0();

  template <typename T>
  T *field(int index);

 private:
  struct Index_def {
    enum ha_key_alg algorithm;
    bool unique;
    std::vector<int> columns;
  };

  bool m_finalized;
  char m_name[32];
  THD *m_thd;

  TABLE_SHARE m_table_share;
  TABLE m_table;

  std::vector<Index_def> m_index_defs;
  uint32 all_set_buf;

  std::vector<Field *> m_fields;

  MY_BITMAP write_set_struct;
  uint32 write_set_buf;
  MY_BITMAP read_set_struct;
  uint32 read_set_buf;

  std::array<uchar, 512> m_record_0;
  std::array<uchar, 512> m_record_1;

  std::vector<KEY> m_indexes;
  std::vector<KEY_PART_INFO> m_index_parts;

  void add_field(const Field &field);
  void initialize_share();
  void initialize_table();
  void finalize_fields();
  uint set_field_pointers(uchar *record);
};

inline Table_helper::Table_helper(const char *name, THD *thd)
    : m_finalized(false), m_thd(thd) {
  EXPECT_TRUE(name != nullptr);
  EXPECT_LT(std::strlen(name), sizeof(m_name));
  std::strcpy(m_name, name);
}

inline void Table_helper::add_field_long(const char *name, bool is_nullable) {
  add_field(Mock_field_long(name, is_nullable));
}

inline void Table_helper::add_field_varstring(const char *name, uint char_len,
                                              bool nullable) {
  add_field(Mock_field_varstring(&m_table_share, name, char_len, nullable));
}

inline void Table_helper::add_field(const Field &field) {
  EXPECT_FALSE(m_finalized);

  auto new_field = field.clone(m_thd->mem_root);
  new_field->field_index = static_cast<uint16>(m_fields.size());

  m_fields.push_back(new_field);
}

inline void Table_helper::add_index(ha_key_alg algorithm, bool unique,
                                    const std::vector<int> &columns) {
  EXPECT_FALSE(m_finalized);
  EXPECT_GT(columns.size(), 0);

  m_index_defs.push_back({algorithm, unique, columns});
}

inline void Table_helper::finalize() {
  EXPECT_FALSE(m_finalized);
  m_finalized = true;

  /* fill table info */
  initialize_share();
  initialize_table();
  finalize_fields();

  EXPECT_EQ(0, bitmap_init(&m_table_share.all_set, &all_set_buf,
                           m_table_share.fields));
  bitmap_set_above(&m_table_share.all_set, 0, true);

  m_table_share.key_parts = 0;
  for (auto &iter : m_index_defs) {
    m_table_share.key_parts += static_cast<uint>(iter.columns.size());
  }

  m_indexes.resize(m_index_defs.size());
  m_index_parts.resize(m_table_share.key_parts);

  size_t part_index = 0;
  for (size_t i = 0; i < m_index_defs.size(); ++i) {
    m_indexes[i].flags = 0;
    if (m_index_defs[i].unique) {
      m_indexes[i].flags |= HA_NOSAME;
    }
    m_indexes[i].algorithm = m_index_defs[i].algorithm;
    m_indexes[i].user_defined_key_parts =
        static_cast<uint>(m_index_defs[i].columns.size());
    m_indexes[i].key_part = &m_index_parts[part_index];
    for (auto field_idx : m_index_defs[i].columns) {
      auto field = m_fields[field_idx];
      m_index_parts[part_index].init_from_field(field);

      ++part_index;
    }
  }
  m_table_share.keys = static_cast<uint>(m_index_defs.size());
  m_table_share.key_info = m_table.key_info = m_indexes.data();
}

inline void Table_helper::set_handler(handler *handler) {
  EXPECT_TRUE(handler != nullptr);

  m_table.file = handler;
  handler->change_table_ptr(&m_table, m_table.s);
}

inline TABLE *Table_helper::table() { return &m_table; }

inline TABLE_SHARE *Table_helper::table_share() { return m_table.s; }

inline uchar *Table_helper::record_0() { return m_record_0.data(); }

inline uchar *Table_helper::record_1() { return m_record_1.data(); }

inline void Table_helper::copy_record_1_to_0() {
  std::memcpy(record_0(), record_1(), m_table_share.rec_buff_length);
}

template <typename T>
inline T *Table_helper::field(int index) {
  return static_cast<T *>(m_table.field[index]);
}

inline void Table_helper::initialize_share() {
  m_table_share.table_name.str = m_name;
  m_table_share.table_name.length = std::strlen(m_name);

  m_table_share.field = m_fields.data();
  m_table_share.fields = static_cast<uint>(m_fields.size());
  m_table_share.db_create_options = 0;
  m_table_share.primary_key = 0;
  m_table_share.column_bitmap_size = sizeof(int);
  m_table_share.tmp_table = NO_TMP_TABLE;
  m_table_share.db_low_byte_first = true;

  static const char *fakepath = "fakepath";
  m_table_share.path.str = const_cast<char *>(fakepath);
  m_table_share.path.length = std::strlen(fakepath);
}

inline void Table_helper::initialize_table() {
  m_table.s = &m_table_share;
  m_table.in_use = m_thd;

  m_table.alias = m_name;

  m_table.field = m_table_share.field;

  m_table.null_row = '\0';
  m_table.read_set = &read_set_struct;
  m_table.write_set = &write_set_struct;
  m_table.next_number_field = nullptr;  // No autoinc column
  m_table.pos_in_table_list = nullptr;
  EXPECT_EQ(0,
            bitmap_init(m_table.write_set, &write_set_buf, m_table.s->fields));
  EXPECT_EQ(0, bitmap_init(m_table.read_set, &read_set_buf, m_table.s->fields));

  m_table.const_table = false;
}

inline void Table_helper::finalize_fields() {
  for (uint i = 0; i < m_table_share.fields; ++i) {
    Field *field = m_table_share.field[i];

    field->init(&m_table);
    field->field_index = i;

    bitmap_set_bit(m_table.read_set, i);
    bitmap_set_bit(m_table.write_set, i);
  }

  m_table.record[0] = m_record_0.data();
  m_table.record[1] = m_record_1.data();
  m_table.s->rec_buff_length = set_field_pointers(m_table.record[0]);
}

inline uint Table_helper::set_field_pointers(uchar *record) {
  uchar *nul_ptr = record;
  uint nul_bit = 0;
  uint needed_nul_bytes = (static_cast<uint>(m_fields.size()) + 7) / 8;

  uchar *buf_ptr = nul_ptr + needed_nul_bytes;

  for (auto &field : m_fields) {
    /* Use the flag, it should be valid as field was cloned */
    if (field->flags & NOT_NULL_FLAG) {
      field->set_null_ptr(nullptr, 0);
    } else {
      field->set_null_ptr(nul_ptr, nul_bit);
      if (++nul_bit == 8) {
        ++nul_ptr;
        nul_bit = 0;
      }
    }

    field->ptr = buf_ptr;
    buf_ptr += field->pack_length();
  }

  return (buf_ptr - record);
}

}  // namespace temptable_test

#endif  // TABLE_HELPER_H
