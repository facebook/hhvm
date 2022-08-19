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
#include "unittest/gunit/mock_field_long.h"
#include "unittest/gunit/test_utils.h"

namespace field_long_unittest {

using my_testing::Mock_error_handler;
using my_testing::Server_initializer;

class FieldLongTest : public ::testing::Test {
 protected:
  virtual void SetUp() { initializer.SetUp(); }
  virtual void TearDown() { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;

  Field_set *create_field_set(TYPELIB *tl);
};

void test_store_long(Field_long *field, const longlong store_value,
                     const longlong expected_result,
                     const int expected_error_no,
                     const type_conversion_status expected_status) {
  Mock_error_handler error_handler(field->table->in_use, expected_error_no);
  type_conversion_status err = field->store(store_value, false);  // signed
  EXPECT_EQ(expected_result, field->val_int());
  EXPECT_FALSE(field->is_null());
  EXPECT_EQ(expected_status, err);
  EXPECT_EQ((expected_error_no == 0 ? 0 : 1), error_handler.handle_called());
}

void test_store_string(Field_long *field, const char *store_value,
                       const int length, const longlong expected_result,
                       const int expected_error_no,
                       const type_conversion_status expected_status) {
  Mock_error_handler error_handler(field->table->in_use, expected_error_no);
  type_conversion_status err =
      field->store(store_value, length, &my_charset_latin1);
  EXPECT_EQ(expected_result, field->val_int());
  EXPECT_FALSE(field->is_null());
  EXPECT_EQ(expected_status, err);
  EXPECT_EQ((expected_error_no == 0 ? 0 : 1), error_handler.handle_called());
}

TEST_F(FieldLongTest, StoreLegalIntValues) {
  Mock_field_long field_long;
  Fake_TABLE table(&field_long);
  table.in_use = thd();
  field_long.make_writable();
  field_long.make_readable();
  thd()->check_for_truncated_fields = CHECK_FIELD_WARN;

  SCOPED_TRACE("");
  test_store_long(&field_long, 0, 0, 0, TYPE_OK);
  SCOPED_TRACE("");
  test_store_long(&field_long, 5, 5, 0, TYPE_OK);
  SCOPED_TRACE("");
  test_store_long(&field_long, -1, -1, 0, TYPE_OK);

  {
    SCOPED_TRACE("");
    test_store_long(&field_long, INT_MIN32, INT_MIN32, 0, TYPE_OK);
  }
  {
    SCOPED_TRACE("");
    test_store_long(&field_long, INT_MAX32, INT_MAX32, 0, TYPE_OK);
  }

  {
    Mock_error_handler error_handler(thd(), 0);
    type_conversion_status err;
    err = set_field_to_null(&field_long);

    EXPECT_EQ(0, field_long.val_int());
    EXPECT_TRUE(field_long.is_null());
    EXPECT_EQ(TYPE_OK, err);

    field_long.set_notnull();
    EXPECT_EQ(0, field_long.val_int());
    EXPECT_FALSE(field_long.is_null());

    // None of the above should generate warnings
    EXPECT_EQ(0, error_handler.handle_called());
  }
}

// Values higher and lower than valid range for the Field_long
TEST_F(FieldLongTest, StoreOutOfRangeIntValues) {
  Mock_field_long field_long;
  Fake_TABLE table(&field_long);
  table.in_use = thd();
  field_long.make_writable();
  field_long.make_readable();
  thd()->check_for_truncated_fields = CHECK_FIELD_WARN;

  // Field_long is signed
  {
    SCOPED_TRACE("");
    test_store_long(&field_long, INT_MAX32 + 1LL, INT_MAX32,
                    ER_WARN_DATA_OUT_OF_RANGE, TYPE_WARN_OUT_OF_RANGE);
  }
  {
    SCOPED_TRACE("");
    test_store_long(&field_long, INT_MIN32 - 1LL, INT_MIN32,
                    ER_WARN_DATA_OUT_OF_RANGE, TYPE_WARN_OUT_OF_RANGE);
  }

  // Field_long is unsigned
  {
    SCOPED_TRACE("");
    field_long.unsigned_flag = true;
  }
  {
    SCOPED_TRACE("");
    test_store_long(&field_long, -1LL, 0, ER_WARN_DATA_OUT_OF_RANGE,
                    TYPE_WARN_OUT_OF_RANGE);
  }
  {
    SCOPED_TRACE("");
    test_store_long(&field_long, INT_MIN32, 0, ER_WARN_DATA_OUT_OF_RANGE,
                    TYPE_WARN_OUT_OF_RANGE);
  }
}

TEST_F(FieldLongTest, StoreLegalStringValues) {
  Mock_field_long field_long;

  Fake_TABLE table(&field_long);
  table.in_use = thd();
  field_long.make_writable();
  field_long.make_readable();
  thd()->check_for_truncated_fields = CHECK_FIELD_WARN;

  const char min_int[] = "-2147483648";
  const char max_int[] = "2147483647";
  const char max_int_plus1[] = "2147483648";
  const char max_uint[] = "4294967295";

  // Field_long is signed
  {
    SCOPED_TRACE("");
    test_store_string(&field_long, STRING_WITH_LEN("0"), 0, 0, TYPE_OK);
  }
  {
    SCOPED_TRACE("");
    test_store_string(&field_long, STRING_WITH_LEN("1"), 1, 0, TYPE_OK);
  }
  {
    SCOPED_TRACE("");
    test_store_string(&field_long, STRING_WITH_LEN("-1"), -1, 0, TYPE_OK);
  }
  {
    SCOPED_TRACE("");
    test_store_string(&field_long, STRING_WITH_LEN(max_int), INT_MAX32, 0,
                      TYPE_OK);
  }
  {
    SCOPED_TRACE("");
    test_store_string(&field_long, STRING_WITH_LEN(min_int), INT_MIN32, 0,
                      TYPE_OK);
  }

  // Field_long is unsigned
  field_long.unsigned_flag = true;
  {
    SCOPED_TRACE("");
    test_store_string(&field_long, STRING_WITH_LEN(max_int_plus1),
                      INT_MAX32 + 1LL, 0, TYPE_OK);
  }
  {
    SCOPED_TRACE("");
    test_store_string(&field_long, STRING_WITH_LEN(max_uint), UINT_MAX32, 0,
                      TYPE_OK);
  }
}

TEST_F(FieldLongTest, StoreIllegalStringValues) {
  Mock_field_long field_long;

  Fake_TABLE table(&field_long);
  table.in_use = thd();
  field_long.make_writable();
  field_long.make_readable();
  thd()->check_for_truncated_fields = CHECK_FIELD_WARN;

  const char max_int_plus1[] = "2147483648";
  const char min_int_minus1[] = "-2147483649";
  const char very_high[] = "999999999999999";
  const char very_low[] = "-999999999999999";

  // Field_long is signed - Stored value is INT_MIN32/INT_MAX32
  //                        depending on sign of string to store
  {
    SCOPED_TRACE("");
    test_store_string(&field_long, STRING_WITH_LEN(max_int_plus1), INT_MAX32,
                      ER_WARN_DATA_OUT_OF_RANGE, TYPE_WARN_OUT_OF_RANGE);
  }
  {
    SCOPED_TRACE("");
    test_store_string(&field_long, STRING_WITH_LEN(very_high), INT_MAX32,
                      ER_WARN_DATA_OUT_OF_RANGE, TYPE_WARN_OUT_OF_RANGE);
  }
  {
    SCOPED_TRACE("");
    test_store_string(&field_long, STRING_WITH_LEN(min_int_minus1), INT_MIN32,
                      ER_WARN_DATA_OUT_OF_RANGE, TYPE_WARN_OUT_OF_RANGE);
  }
  {
    SCOPED_TRACE("");
    test_store_string(&field_long, STRING_WITH_LEN(very_low), INT_MIN32,
                      ER_WARN_DATA_OUT_OF_RANGE, TYPE_WARN_OUT_OF_RANGE);
  }

  // Field_long is unsigned - Stored value is 0/UINT_MAX32
  //                          depending on sign of string to store
  const char min_int[] = "-2147483648";
  const char max_uint_plus1[] = "4294967296";
  field_long.unsigned_flag = true;

  {
    SCOPED_TRACE("");
    test_store_string(&field_long, STRING_WITH_LEN(max_uint_plus1), UINT_MAX32,
                      ER_WARN_DATA_OUT_OF_RANGE, TYPE_WARN_OUT_OF_RANGE);
  }
  {
    SCOPED_TRACE("");
    test_store_string(&field_long, STRING_WITH_LEN(very_high), UINT_MAX32,
                      ER_WARN_DATA_OUT_OF_RANGE, TYPE_WARN_OUT_OF_RANGE);
  }
  {
    SCOPED_TRACE("");
    test_store_string(&field_long, STRING_WITH_LEN("-1"), 0,
                      ER_WARN_DATA_OUT_OF_RANGE, TYPE_WARN_OUT_OF_RANGE);
  }
  {
    SCOPED_TRACE("");
    test_store_string(&field_long, STRING_WITH_LEN(min_int), 0,
                      ER_WARN_DATA_OUT_OF_RANGE, TYPE_WARN_OUT_OF_RANGE);
  }
  {
    SCOPED_TRACE("");
    test_store_string(&field_long, STRING_WITH_LEN(very_low), 0,
                      ER_WARN_DATA_OUT_OF_RANGE, TYPE_WARN_OUT_OF_RANGE);
  }

  // Invalid value
  {
    SCOPED_TRACE("");
    test_store_string(&field_long, STRING_WITH_LEN("foo"), 0,
                      ER_TRUNCATED_WRONG_VALUE_FOR_FIELD, TYPE_ERR_BAD_VALUE);
  }
}

TEST_F(FieldLongTest, StoreNullValue) {
  Mock_field_long field_long;

  Fake_TABLE table(&field_long);
  table.in_use = thd();
  field_long.make_writable();
  field_long.make_readable();
  thd()->check_for_truncated_fields = CHECK_FIELD_WARN;

  type_conversion_status err;

  // Save NULL value in a field that can have NULL value
  {
    Mock_error_handler error_handler(thd(), 0);
    err = set_field_to_null(&field_long);
    EXPECT_EQ(0, field_long.val_int());
    EXPECT_EQ(TYPE_OK, err);

    err = set_field_to_null_with_conversions(&field_long, true);
    EXPECT_EQ(0, field_long.val_int());
    EXPECT_EQ(TYPE_OK, err);

    err = set_field_to_null_with_conversions(&field_long, false);
    EXPECT_EQ(0, field_long.val_int());
    EXPECT_EQ(TYPE_OK, err);

    EXPECT_EQ(0, error_handler.handle_called());
  }

  // Save NULL value in a field that can NOT have NULL value
  field_long.set_null_ptr(nullptr, 0);
  {
    Mock_error_handler error_handler(thd(), 0);
    // Save NULL value in a field that can be set to NULL temporary
    field_long.set_tmp_nullable();
    err = set_field_to_null(&field_long);
    EXPECT_EQ(0, field_long.val_int());
    EXPECT_EQ(TYPE_OK, err);
    EXPECT_EQ(0, error_handler.handle_called());
    field_long.reset_tmp_nullable();
  }

  {
    Mock_error_handler error_handler(thd(), 0);
    err = set_field_to_null_with_conversions(&field_long, true);
    EXPECT_EQ(0, field_long.val_int());
    EXPECT_EQ(TYPE_ERR_NULL_CONSTRAINT_VIOLATION, err);
    EXPECT_EQ(0, error_handler.handle_called());
  }

  {
    Mock_error_handler error_handler(thd(), ER_BAD_NULL_ERROR);
    err = set_field_to_null_with_conversions(&field_long, false);
    EXPECT_EQ(0, field_long.val_int());
    EXPECT_EQ(TYPE_OK, err);
    EXPECT_EQ(1, error_handler.handle_called());
  }
}

}  // namespace field_long_unittest
