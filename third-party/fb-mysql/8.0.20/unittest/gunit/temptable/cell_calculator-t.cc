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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <gtest/gtest.h>
#include <array>
#include <cstring>
#include <memory>

#include "my_byteorder.h"
#include "sql/table.h"
#include "storage/temptable/include/temptable/cell_calculator.h"

namespace temptable_test {

namespace {

class Table_wrapper {
 public:
  static const uint32 FIELD_SHORT_LENGTH = 2;
  static const uint32 FIELD_STRING_PAD_LENGTH = 20 * 4;
  static const uint32 FIELD_STRING_NOPAD_LENGTH = 20 * 4;

  Table_wrapper();

  Field_short *get_field_short();

  Field_string *get_field_string_pad();

  Field_string *get_field_string_nopad();

  void store_short_value(std::array<uchar, FIELD_SHORT_LENGTH> &storage,
                         short value);

 private:
  static const size_t RECORD_SIZE = 1024;

  static const char *FIELD_SHORT_NAME;
  static const int FIELD_SHORT_OFFSET = 100;
  static const int FIELD_SHORT_NULL_BIT = 0;

  static const char *FIELD_STRING_PAD_NAME;
  static const int FIELD_STRING_PAD_OFFSET = 200;
  static const int FIELD_STRING_PAD_NULL_BIT = 1;

  static const char *FIELD_STRING_NOPAD_NAME;
  static const int FIELD_STRING_NOPAD_OFFSET = 300;
  static const int FIELD_STRING_NOPAD_NULL_BIT = 2;

  uint8 *get_data_ptr(int offset);

  uint8 *get_null_ptr(int bit);

  uchar get_null_bit(int bit);

  TABLE m_table;
  TABLE_SHARE m_table_share;
  std::array<uchar, RECORD_SIZE> m_record0;
  std::array<uchar, RECORD_SIZE> m_record1;

  std::unique_ptr<Field_short> m_field_short;
  std::unique_ptr<Field_string> m_field_string_pad;
  std::unique_ptr<Field_string> m_field_string_nopad;
};

const char *Table_wrapper::FIELD_SHORT_NAME = "Short";
const char *Table_wrapper::FIELD_STRING_PAD_NAME = "StringPad";
const char *Table_wrapper::FIELD_STRING_NOPAD_NAME = "StringNoPad";

Table_wrapper::Table_wrapper() {
  m_table_share.db_low_byte_first = false;
  m_table.s = &m_table_share;
  m_table.record[0] = m_record0.data();
  m_table.record[1] = m_record1.data();

  m_field_short.reset(new Field_short(
      get_data_ptr(FIELD_SHORT_OFFSET), FIELD_SHORT_LENGTH,
      get_null_ptr(FIELD_SHORT_NULL_BIT), get_null_bit(FIELD_SHORT_NULL_BIT), 0,
      FIELD_SHORT_NAME, false, false));
  m_field_short->table = &m_table;

  m_field_string_pad.reset(new Field_string(
      get_data_ptr(FIELD_STRING_PAD_OFFSET), FIELD_STRING_PAD_LENGTH,
      get_null_ptr(FIELD_STRING_PAD_NULL_BIT),
      get_null_bit(FIELD_STRING_PAD_NULL_BIT), 0, FIELD_STRING_PAD_NAME,
      &my_charset_utf8mb4_general_ci));
  m_field_string_pad->table = &m_table;

  m_field_string_nopad.reset(new Field_string(
      get_data_ptr(FIELD_STRING_NOPAD_OFFSET), FIELD_STRING_NOPAD_LENGTH,
      get_null_ptr(FIELD_STRING_NOPAD_NULL_BIT),
      get_null_bit(FIELD_STRING_NOPAD_NULL_BIT), 0, FIELD_STRING_NOPAD_NAME,
      &my_charset_utf8mb4_0900_ai_ci));
  m_field_string_nopad->table = &m_table;
}

Field_short *Table_wrapper::get_field_short() { return m_field_short.get(); }

Field_string *Table_wrapper::get_field_string_pad() {
  return m_field_string_pad.get();
}

Field_string *Table_wrapper::get_field_string_nopad() {
  return m_field_string_nopad.get();
}

uint8 *Table_wrapper::get_data_ptr(int offset) {
  return m_record0.data() + offset;
}

uint8 *Table_wrapper::get_null_ptr(int bit) {
  int offset = bit / 8;
  return m_record0.data() + offset;
}

uchar Table_wrapper::get_null_bit(int bit) { return bit % 8; }

/* must mimic the Field_short::store() */
void Table_wrapper::store_short_value(
    std::array<uchar, Table_wrapper::FIELD_SHORT_LENGTH> &storage,
    short value) {
#ifdef WORDS_BIGENDIAN
  if (m_table.s->db_low_byte_first)
    int2store(storage.data(), value);
  else
#endif
    shortstore(storage.data(), value);
}

}  // namespace

TEST(CellCalculator, NullEmpty) {
  Table_wrapper table;

  std::array<uchar, Table_wrapper::FIELD_SHORT_LENGTH> value;

  table.store_short_value(value, 0x1234);

  temptable::Cell cell_valid(false, value.size(), value.data());
  temptable::Cell cell_empty(false, 0, value.data());
  temptable::Cell cell_null1(true, 0, nullptr);
  temptable::Cell cell_null2(true, value.size(), value.data());

  KEY_PART_INFO key;
  key.init_from_field(table.get_field_short());

  temptable::Cell_calculator calculator(key);

  EXPECT_EQ(calculator.hash(cell_null1), 1);
  EXPECT_EQ(calculator.hash(cell_null2), 1);
  EXPECT_EQ(calculator.hash(cell_empty), 0);

  EXPECT_EQ(calculator.compare(cell_null1, cell_null2), 0);

  EXPECT_LT(calculator.compare(cell_null1, cell_valid), 0);
  EXPECT_LT(calculator.compare(cell_null2, cell_valid), 0);

  EXPECT_GT(calculator.compare(cell_valid, cell_null1), 0);
  EXPECT_GT(calculator.compare(cell_valid, cell_null2), 0);
}

TEST(CellCalculator, Short) {
  Table_wrapper table;

  std::array<uchar, Table_wrapper::FIELD_SHORT_LENGTH> value1a;
  std::array<uchar, Table_wrapper::FIELD_SHORT_LENGTH> value1b;
  std::array<uchar, Table_wrapper::FIELD_SHORT_LENGTH> value2a;
  std::array<uchar, Table_wrapper::FIELD_SHORT_LENGTH> value2b;

  table.store_short_value(value1a, 0x1234);
  table.store_short_value(value1b, 0x1234);
  table.store_short_value(value2a, 0x4321);
  table.store_short_value(value2b, 0x4321);

  temptable::Cell cell1a(false, value1a.size(), value1a.data());
  temptable::Cell cell1b(false, value1b.size(), value1b.data());
  temptable::Cell cell2a(false, value2a.size(), value2a.data());
  temptable::Cell cell2b(false, value2b.size(), value2b.data());

  /* check for field */

  temptable::Cell_calculator field_calculator(table.get_field_short());

  EXPECT_EQ(field_calculator.hash(cell1a), field_calculator.hash(cell1b));
  EXPECT_EQ(field_calculator.hash(cell2a), field_calculator.hash(cell2b));

  EXPECT_EQ(field_calculator.compare(cell1a, cell1b), 0);
  EXPECT_EQ(field_calculator.compare(cell1a, cell1b), 0);

  EXPECT_LT(field_calculator.compare(cell1a, cell2a), 0);
  EXPECT_GT(field_calculator.compare(cell2a, cell1a), 0);

  EXPECT_GT(field_calculator.compare(cell2b, cell1b), 0);
  EXPECT_LT(field_calculator.compare(cell1b, cell2b), 0);

  /* check for key */

  KEY_PART_INFO key;
  key.init_from_field(table.get_field_short());

  temptable::Cell_calculator key_calculator(key);

  EXPECT_EQ(key_calculator.hash(cell1a), key_calculator.hash(cell1b));
  EXPECT_EQ(key_calculator.hash(cell2a), key_calculator.hash(cell2b));

  EXPECT_EQ(key_calculator.compare(cell1a, cell1b), 0);
  EXPECT_EQ(key_calculator.compare(cell1a, cell1b), 0);

  EXPECT_LT(key_calculator.compare(cell1a, cell2a), 0);
  EXPECT_GT(key_calculator.compare(cell2a, cell1a), 0);

  EXPECT_GT(key_calculator.compare(cell2b, cell1b), 0);
  EXPECT_LT(key_calculator.compare(cell1b, cell2b), 0);
}

TEST(CellCalculator, StringPad) {
  Table_wrapper table;

  std::array<uchar, Table_wrapper::FIELD_STRING_PAD_LENGTH> value1a;
  std::array<uchar, Table_wrapper::FIELD_STRING_PAD_LENGTH> value1b;
  std::array<uchar, Table_wrapper::FIELD_STRING_PAD_LENGTH> value2a;
  std::array<uchar, Table_wrapper::FIELD_STRING_PAD_LENGTH> value3a;

  value1a.fill(' ');
  std::memcpy(value1a.data(), "abcde", 5);
  value1b.fill(' ');
  std::memcpy(value1b.data(), "abcde", 5);
  value2a.fill(' ');
  std::memcpy(value2a.data(), "aaaaabcde", 10);
  value3a.fill(' ');
  std::memcpy(value3a.data(), "abcdfg", 6);

  temptable::Cell cell1a(false, value1a.size(), value1a.data());
  temptable::Cell cell1b(false, value1b.size(), value1b.data());
  temptable::Cell cell2a(false, value2a.size(), value2a.data());
  temptable::Cell cell3a(false, value3a.size(), value3a.data());

  /* check for field */

  temptable::Cell_calculator field_calculator(table.get_field_string_pad());

  EXPECT_EQ(field_calculator.hash(cell1a), field_calculator.hash(cell1b));

  EXPECT_EQ(field_calculator.compare(cell1a, cell1b), 0);

  EXPECT_LT(field_calculator.compare(cell2a, cell1a), 0);
  EXPECT_GT(field_calculator.compare(cell1a, cell2a), 0);

  EXPECT_GT(field_calculator.compare(cell3a, cell1a), 0);
  EXPECT_LT(field_calculator.compare(cell1a, cell3a), 0);

  EXPECT_LT(field_calculator.compare(cell2a, cell3a), 0);

  /* check for full (whole column) key */

  KEY_PART_INFO full_key;
  full_key.init_from_field(table.get_field_string_pad());

  temptable::Cell_calculator full_key_calculator(full_key);

  EXPECT_EQ(full_key_calculator.hash(cell1a), full_key_calculator.hash(cell1b));

  EXPECT_EQ(full_key_calculator.compare(cell1a, cell1b), 0);

  EXPECT_LT(full_key_calculator.compare(cell2a, cell1a), 0);
  EXPECT_GT(full_key_calculator.compare(cell1a, cell2a), 0);

  EXPECT_GT(full_key_calculator.compare(cell3a, cell1a), 0);
  EXPECT_LT(full_key_calculator.compare(cell1a, cell3a), 0);

  EXPECT_LT(full_key_calculator.compare(cell2a, cell3a), 0);

  /* check for prefix key */

  KEY_PART_INFO prefix_key;
  prefix_key.init_from_field(table.get_field_string_pad());
  prefix_key.key_part_flag |= HA_PART_KEY_SEG;
  prefix_key.length =
      3 * table.get_field_string_pad()->charset_for_protocol()->mbmaxlen;

  temptable::Cell_calculator prefix_key_calculator(prefix_key);

  EXPECT_EQ(prefix_key_calculator.hash(cell1a),
            prefix_key_calculator.hash(cell1b));
  EXPECT_EQ(prefix_key_calculator.hash(cell1a),
            prefix_key_calculator.hash(cell3a));

  EXPECT_EQ(prefix_key_calculator.compare(cell1a, cell1b), 0);
  EXPECT_EQ(prefix_key_calculator.compare(cell1a, cell3a), 0);
  EXPECT_EQ(prefix_key_calculator.compare(cell1b, cell3a), 0);

  EXPECT_LT(prefix_key_calculator.compare(cell2a, cell1a), 0);
  EXPECT_GT(prefix_key_calculator.compare(cell1a, cell2a), 0);

  EXPECT_LT(prefix_key_calculator.compare(cell2a, cell3a), 0);
  EXPECT_GT(prefix_key_calculator.compare(cell3a, cell2a), 0);
}

TEST(CellCalculator, StringNoPad) {
  Table_wrapper table;

  std::array<uchar, Table_wrapper::FIELD_STRING_NOPAD_LENGTH> value1a;
  std::array<uchar, Table_wrapper::FIELD_STRING_NOPAD_LENGTH> value1b;
  std::array<uchar, Table_wrapper::FIELD_STRING_NOPAD_LENGTH> value2a;
  std::array<uchar, Table_wrapper::FIELD_STRING_NOPAD_LENGTH> value3a;

  value1a.fill(' ');
  std::memcpy(value1a.data(), "abcde", 5);
  value1b.fill(' ');
  std::memcpy(value1b.data(), "abcde", 5);
  value2a.fill(' ');
  std::memcpy(value2a.data(), "aaaaabcde", 10);
  value3a.fill(' ');
  std::memcpy(value3a.data(), "abcdfg", 6);

  temptable::Cell cell1a(false, value1a.size(), value1a.data());
  temptable::Cell cell1b(false, value1b.size(), value1b.data());
  temptable::Cell cell2a(false, value2a.size(), value2a.data());
  temptable::Cell cell3a(false, value3a.size(), value3a.data());

  /* check for field */

  temptable::Cell_calculator field_calculator(table.get_field_string_nopad());

  EXPECT_EQ(field_calculator.hash(cell1a), field_calculator.hash(cell1b));

  EXPECT_EQ(field_calculator.compare(cell1a, cell1b), 0);

  EXPECT_LT(field_calculator.compare(cell2a, cell1a), 0);
  EXPECT_GT(field_calculator.compare(cell1a, cell2a), 0);

  EXPECT_GT(field_calculator.compare(cell3a, cell1a), 0);
  EXPECT_LT(field_calculator.compare(cell1a, cell3a), 0);

  EXPECT_LT(field_calculator.compare(cell2a, cell3a), 0);

  /* check for full (whole column) key */

  KEY_PART_INFO full_key;
  full_key.init_from_field(table.get_field_string_nopad());

  temptable::Cell_calculator full_key_calculator(full_key);

  EXPECT_EQ(full_key_calculator.hash(cell1a), full_key_calculator.hash(cell1b));

  EXPECT_EQ(full_key_calculator.compare(cell1a, cell1b), 0);

  EXPECT_LT(full_key_calculator.compare(cell2a, cell1a), 0);
  EXPECT_GT(full_key_calculator.compare(cell1a, cell2a), 0);

  EXPECT_GT(full_key_calculator.compare(cell3a, cell1a), 0);
  EXPECT_LT(full_key_calculator.compare(cell1a, cell3a), 0);

  EXPECT_LT(full_key_calculator.compare(cell2a, cell3a), 0);

  /* check for prefix key */

  KEY_PART_INFO prefix_key;
  prefix_key.init_from_field(table.get_field_string_nopad());
  prefix_key.key_part_flag |= HA_PART_KEY_SEG;
  prefix_key.length =
      3 * table.get_field_string_nopad()->charset_for_protocol()->mbmaxlen;

  temptable::Cell_calculator prefix_key_calculator(prefix_key);

  EXPECT_EQ(prefix_key_calculator.hash(cell1a),
            prefix_key_calculator.hash(cell1b));
  EXPECT_EQ(prefix_key_calculator.hash(cell1a),
            prefix_key_calculator.hash(cell3a));

  EXPECT_EQ(prefix_key_calculator.compare(cell1a, cell1b), 0);
  EXPECT_EQ(prefix_key_calculator.compare(cell1a, cell3a), 0);
  EXPECT_EQ(prefix_key_calculator.compare(cell1b, cell3a), 0);

  EXPECT_LT(prefix_key_calculator.compare(cell2a, cell1a), 0);
  EXPECT_GT(prefix_key_calculator.compare(cell1a, cell2a), 0);

  EXPECT_LT(prefix_key_calculator.compare(cell2a, cell3a), 0);
  EXPECT_GT(prefix_key_calculator.compare(cell3a, cell2a), 0);
}

}  // namespace temptable_test
