/* Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.

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

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"

#include <gtest/gtest.h>

#include "sql/field.h"
#include "sql/sql_class.h"
#include "unittest/gunit/fake_table.h"
#include "unittest/gunit/field_temporal_utils.h"
#include "unittest/gunit/test_utils.h"

namespace field_date_unittests {

using my_testing::Mock_error_handler;
using my_testing::Server_initializer;

class FieldDateTest : public ::testing::Test {
 protected:
  virtual void SetUp() { initializer.SetUp(); }
  virtual void TearDown() { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;

  Field_set *create_field_set(TYPELIB *tl);

  // Store zero date using different combinations of SQL modes
  static const int no_modes = 3;
  static const sql_mode_t strict_modes[no_modes];

  static const type_conversion_status nozero_expected_status[];
};

const sql_mode_t FieldDateTest::strict_modes[no_modes] = {
    MODE_STRICT_TRANS_TABLES, MODE_STRICT_ALL_TABLES,
    MODE_STRICT_TRANS_TABLES | MODE_STRICT_ALL_TABLES};

const type_conversion_status FieldDateTest::nozero_expected_status[] = {
    TYPE_ERR_BAD_VALUE, TYPE_ERR_BAD_VALUE, TYPE_ERR_BAD_VALUE};

class Mock_field_date : public Field_newdate {
 public:
  Mock_field_date()
      : Field_newdate(nullptr,                    // ptr_arg
                      &Field::dummy_null_buffer,  // null_ptr_arg
                      1,                          // null_bit_arg
                      Field::NONE,                // auto_flags_arg
                      "field_name")               // field_name_arg
  {}

  void make_writable() { bitmap_set_bit(table->write_set, field_index); }
  void make_readable() { bitmap_set_bit(table->read_set, field_index); }
};

TEST_F(FieldDateTest, StoreLegalStringValues) {
  Mock_field_date field_date;
  Fake_TABLE table(&field_date);
  table.in_use = thd();
  field_date.make_writable();
  field_date.make_readable();

  {
    SCOPED_TRACE("");
    test_store_string(&field_date, STRING_WITH_LEN("2001-01-01"), 0,
                      "2001-01-01", 0, TYPE_OK);
  }
  {
    SCOPED_TRACE("");
    test_store_string(&field_date, STRING_WITH_LEN("0000-00-00"), 0,
                      "0000-00-00", 0, TYPE_OK);
  }
  {
    SCOPED_TRACE("");
    test_store_string(&field_date, STRING_WITH_LEN("0001-00-00"), 0,
                      "0001-00-00", 0, TYPE_OK);
  }
}

TEST_F(FieldDateTest, StoreIllegalStringValues) {
  Mock_field_date field_date;
  Fake_TABLE table(&field_date);
  table.in_use = thd();
  field_date.make_writable();
  field_date.make_readable();
  thd()->check_for_truncated_fields = CHECK_FIELD_WARN;

  // Truncates time
  {
    SCOPED_TRACE("");
    test_store_string(&field_date, STRING_WITH_LEN("2001-01-01 00:00:01"), 0,
                      "2001-01-01", WARN_DATA_TRUNCATED,
                      TYPE_NOTE_TIME_TRUNCATED);
  }

  // Bad year
  {
    SCOPED_TRACE("");
    test_store_string(&field_date, STRING_WITH_LEN("99999-01-01"), 0,
                      "0000-00-00", WARN_DATA_TRUNCATED, TYPE_ERR_BAD_VALUE);
  }

  // Bad month
  {
    SCOPED_TRACE("");
    test_store_string(&field_date, STRING_WITH_LEN("2001-13-01"), 0,
                      "0000-00-00", WARN_DATA_TRUNCATED, TYPE_ERR_BAD_VALUE);
  }

  // Bad day
  {
    SCOPED_TRACE("");
    test_store_string(&field_date, STRING_WITH_LEN("2001-01-32"), 0,
                      "0000-00-00", WARN_DATA_TRUNCATED, TYPE_ERR_BAD_VALUE);
  }

  // Not a date
  {
    SCOPED_TRACE("");
    test_store_string(&field_date, STRING_WITH_LEN("foo"), 0, "0000-00-00",
                      WARN_DATA_TRUNCATED, TYPE_ERR_BAD_VALUE);
  }
}

/**
  Strictness mode test 1:

  Try storing dates with zeroes when no zero-restrictions apply
  (neither NO_ZERO_DATE or NO_ZERO_IN_DATE are set). There should be
  no errors, warnings or notes.
*/
TEST_F(FieldDateTest, StoreZeroDateSqlModeNoZeroRestrictions) {
  Mock_field_date field_date;
  Fake_TABLE table(&field_date);
  table.in_use = thd();
  field_date.make_writable();
  field_date.make_readable();
  thd()->check_for_truncated_fields = CHECK_FIELD_WARN;

  for (int i = 0; i < no_modes; i++) {
    SCOPED_TRACE("");
    store_zero_in_sql_mode(&field_date, STRING_WITH_LEN("0000-00-00"),
                           "0000-00-00", TYPE_OK, strict_modes[i], 0);
  }

  for (int i = 0; i < no_modes; i++) {
    SCOPED_TRACE("");
    store_zero_in_sql_mode(&field_date, STRING_WITH_LEN("0000-01-01"),
                           "0000-01-01", TYPE_OK, strict_modes[i], 0);
  }

  for (int i = 0; i < no_modes; i++) {
    SCOPED_TRACE("");
    store_zero_in_sql_mode(&field_date, STRING_WITH_LEN("2001-00-01"),
                           "2001-00-01", TYPE_OK, strict_modes[i], 0);
  }

  for (int i = 0; i < no_modes; i++) {
    SCOPED_TRACE("");
    store_zero_in_sql_mode(&field_date, STRING_WITH_LEN("2001-01-00"),
                           "2001-01-00", TYPE_OK, strict_modes[i], 0);
  }
}

/**
  Strictness mode test 2:

  Try storing dates with zeroes when NO_ZERO_DATE flag is set. There
  should be no errors, warnings or notes unless the entire date is
  zero: "0000-00-00"
*/
TEST_F(FieldDateTest, StoreZeroDateSqlModeNoZeroDate) {
  Mock_field_date field_date;
  Fake_TABLE table(&field_date);
  table.in_use = thd();
  field_date.make_writable();
  field_date.make_readable();
  thd()->check_for_truncated_fields = CHECK_FIELD_WARN;

  // With "MODE_NO_ZERO_DATE" set - Errors if date is all null
  for (int i = 0; i < no_modes; i++) {
    SCOPED_TRACE("");
    store_zero_in_sql_mode(&field_date, STRING_WITH_LEN("0000-00-00"),
                           "0000-00-00", nozero_expected_status[i],
                           MODE_NO_ZERO_DATE | strict_modes[i],
                           ER_TRUNCATED_WRONG_VALUE);
  }

  // Zero year, month or day is fine
  for (int i = 0; i < no_modes; i++) {
    SCOPED_TRACE("");
    store_zero_in_sql_mode(&field_date, STRING_WITH_LEN("0000-01-01"),
                           "0000-01-01", TYPE_OK,
                           MODE_NO_ZERO_DATE | strict_modes[i], 0);
  }

  for (int i = 0; i < no_modes; i++) {
    SCOPED_TRACE("");
    store_zero_in_sql_mode(&field_date, STRING_WITH_LEN("2001-00-01"),
                           "2001-00-01", TYPE_OK,
                           MODE_NO_ZERO_DATE | strict_modes[i], 0);
  }

  for (int i = 0; i < no_modes; i++) {
    SCOPED_TRACE("");
    store_zero_in_sql_mode(&field_date, STRING_WITH_LEN("2001-01-00"),
                           "2001-01-00", TYPE_OK,
                           MODE_NO_ZERO_DATE | strict_modes[i], 0);
  }
}

/**
  Strictness mode test 3:

  Try storing dates with zeroes when NO_ZERO_IN_DATE flag is set. There
  should be no errors unless either month or day is zero.
*/
TEST_F(FieldDateTest, StoreZeroDateSqlModeNoZeroInDate) {
  Mock_field_date field_date;
  Fake_TABLE table(&field_date);
  table.in_use = thd();
  field_date.make_writable();
  field_date.make_readable();
  thd()->check_for_truncated_fields = CHECK_FIELD_WARN;

  // With "MODE_NO_ZERO_IN_DATE" set - Entire date zero is ok
  for (int i = 0; i < no_modes; i++) {
    SCOPED_TRACE("");
    store_zero_in_sql_mode(&field_date, STRING_WITH_LEN("0000-00-00"),
                           "0000-00-00", TYPE_OK,
                           MODE_NO_ZERO_IN_DATE | strict_modes[i], 0);
  }

  // Year 0 is valid in strict mode too
  for (int i = 0; i < no_modes; i++) {
    SCOPED_TRACE("");
    store_zero_in_sql_mode(&field_date, STRING_WITH_LEN("0000-01-01"),
                           "0000-01-01", TYPE_OK,
                           MODE_NO_ZERO_IN_DATE | strict_modes[i], 0);
  }

  // Month 0 is NOT valid in strict mode, stores all-zero date
  for (int i = 0; i < no_modes; i++) {
    SCOPED_TRACE("");
    store_zero_in_sql_mode(&field_date, STRING_WITH_LEN("2001-00-01"),
                           "0000-00-00", nozero_expected_status[i],
                           MODE_NO_ZERO_IN_DATE | strict_modes[i],
                           ER_TRUNCATED_WRONG_VALUE);
  }

  // Day 0 is NOT valid in strict mode, stores all-zero date
  for (int i = 0; i < no_modes; i++) {
    SCOPED_TRACE("");
    store_zero_in_sql_mode(&field_date, STRING_WITH_LEN("2001-01-00"),
                           "0000-00-00", nozero_expected_status[i],
                           MODE_NO_ZERO_IN_DATE | strict_modes[i],
                           ER_TRUNCATED_WRONG_VALUE);
  }
}

}  // namespace field_date_unittests
