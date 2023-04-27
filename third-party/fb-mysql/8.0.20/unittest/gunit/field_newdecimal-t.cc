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
#include "unittest/gunit/test_utils.h"

namespace field_newdecimal_unittest {

using my_testing::chars_2_decimal;
using my_testing::Mock_error_handler;
using my_testing::Server_initializer;

class FieldNewDecimalTest : public ::testing::Test {
 protected:
  virtual void SetUp() { initializer.SetUp(); }
  virtual void TearDown() { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;

  Field_set *create_field_set(TYPELIB *tl);
};

class Mock_field_new_decimal : public Field_new_decimal {
 public:
  Mock_field_new_decimal(int decimals)
      : Field_new_decimal(nullptr,                    // ptr_arg
                          8,                          // len_arg
                          &Field::dummy_null_buffer,  // null_ptr_arg
                          1,                          // null_bit_arg
                          Field::NONE,                // auto_flags_arg
                          "field_name",               // field_name_arg
                          decimals,                   // dec_arg
                          false,                      // zero_arg
                          false)                      // unsigned_arg
  {}

  void make_writable() { bitmap_set_bit(table->write_set, field_index); }
  void make_readable() { bitmap_set_bit(table->read_set, field_index); }

  void test_store_string(const char *store_value, const int length,
                         const char *expected_string_result,
                         const longlong expected_int_result,
                         const double expected_real_result,
                         const int expected_error_no,
                         const type_conversion_status expected_status) {
    char buff[MAX_FIELD_WIDTH];
    String str(buff, sizeof(buff), &my_charset_bin);
    String unused;

    Mock_error_handler error_handler(table->in_use, expected_error_no);
    type_conversion_status err = store(store_value, length, &my_charset_latin1);
    val_str(&str, &unused);

    EXPECT_STREQ(expected_string_result, str.ptr());
    EXPECT_EQ(expected_int_result, val_int());
    EXPECT_EQ(expected_real_result, val_real());

    EXPECT_FALSE(is_null());
    EXPECT_EQ(expected_status, err);
    EXPECT_EQ((expected_error_no == 0 ? 0 : 1), error_handler.handle_called());
  }
};

TEST_F(FieldNewDecimalTest, StoreLegalStringValues) {
  // Alows storing this range [-999.999, 999.999]
  Mock_field_new_decimal field_dec(3);
  Fake_TABLE table(&field_dec);
  table.in_use = thd();
  field_dec.make_writable();
  field_dec.make_readable();
  thd()->check_for_truncated_fields = CHECK_FIELD_WARN;

  {
    SCOPED_TRACE("");
    field_dec.test_store_string(STRING_WITH_LEN("10.01"), "10.010", 10, 10.01,
                                0, TYPE_OK);
  }
  {
    SCOPED_TRACE("");
    field_dec.test_store_string(STRING_WITH_LEN("0"), "0.000", 0, 0, 0,
                                TYPE_OK);
  }
}

TEST_F(FieldNewDecimalTest, StoreIllegalStringValues) {
  // Alows storing this range [-999.999, 999.999]
  Mock_field_new_decimal field_dec(3);
  Fake_TABLE table(&field_dec);
  table.in_use = thd();
  field_dec.make_writable();
  field_dec.make_readable();
  thd()->check_for_truncated_fields = CHECK_FIELD_WARN;

  // Truncated (precision beyond 3 decimals is lost)
  {
    SCOPED_TRACE("");
    field_dec.test_store_string(STRING_WITH_LEN("10.0101"), "10.010", 10, 10.01,
                                WARN_DATA_TRUNCATED, TYPE_NOTE_TRUNCATED);
  }
  {
    SCOPED_TRACE("");
    field_dec.test_store_string(STRING_WITH_LEN("10.0109"), "10.011", 10,
                                10.011, WARN_DATA_TRUNCATED,
                                TYPE_NOTE_TRUNCATED);
  }
  // Values higher and lower than valid range for the decimal
  {
    SCOPED_TRACE("");
    field_dec.test_store_string(STRING_WITH_LEN("10000"), "999.999", 1000,
                                999.999, ER_WARN_DATA_OUT_OF_RANGE,
                                TYPE_WARN_OUT_OF_RANGE);
  }

  // Values higher and lower than valid range for the decimal
  {
    SCOPED_TRACE("");
    field_dec.test_store_string(STRING_WITH_LEN("-10000"), "-999.999", -1000,
                                -999.999, ER_WARN_DATA_OUT_OF_RANGE,
                                TYPE_WARN_OUT_OF_RANGE);
  }
}

static void test_store_internal(Field_new_decimal *field, my_decimal *value,
                                const char *expected_string_result,
                                const longlong expected_int_result,
                                const double expected_real_result,
                                const int conversion_error,
                                const int expected_error_no,
                                const type_conversion_status expected_status) {
  char buff[MAX_FIELD_WIDTH];
  String str(buff, sizeof(buff), &my_charset_bin);
  String unused;

  Mock_error_handler error_handler(field->table->in_use, expected_error_no);
  type_conversion_status err =
      store_internal_with_error_check(field, conversion_error, value);
  field->val_str(&str, &unused);
  EXPECT_STREQ(expected_string_result, str.ptr());
  EXPECT_EQ(expected_int_result, field->val_int());
  EXPECT_EQ(expected_real_result, field->val_real());

  EXPECT_EQ(expected_status, err);
}

/**
  Test store_internal_with_error_check(). This is an internal store
  function for Field_new_decimal. The function does not modify the
  NULL value of the field so we don't test field.is_null()
*/
TEST_F(FieldNewDecimalTest, storeInternalWithErrorCheckLegalValues) {
  // Alows storing this range [-99.9999, 99.9999]
  Mock_field_new_decimal field_dec(4);
  Fake_TABLE table(&field_dec);
  table.in_use = thd();
  field_dec.make_writable();
  field_dec.make_readable();
  thd()->check_for_truncated_fields = CHECK_FIELD_WARN;

  my_decimal d10_01;
  my_decimal dMin10_01;
  my_decimal d10_01001;
  my_decimal d10_01009;
  my_decimal dInsignificant;

  EXPECT_EQ(0, chars_2_decimal("10.01", &d10_01));
  EXPECT_EQ(0, chars_2_decimal("-10.01", &dMin10_01));
  EXPECT_EQ(0, chars_2_decimal("10.01001", &d10_01001));
  EXPECT_EQ(0, chars_2_decimal("10.01009", &d10_01009));
  EXPECT_EQ(0, chars_2_decimal("0.00000000001", &dInsignificant));

  // Legal values
  {
    SCOPED_TRACE("");
    test_store_internal(&field_dec, &d10_01, "10.0100", 10, 10.01, E_DEC_OK, 0,
                        TYPE_OK);
  }
  {
    SCOPED_TRACE("");
    test_store_internal(&field_dec, &dMin10_01, "-10.0100", -10, -10.01,
                        E_DEC_OK, 0, TYPE_OK);
  }

  // Legal values, but rounded
  {
    SCOPED_TRACE("");
    test_store_internal(&field_dec, &d10_01001, "10.0100", 10, 10.01, E_DEC_OK,
                        WARN_DATA_TRUNCATED, TYPE_NOTE_TRUNCATED);
  }
  {
    SCOPED_TRACE("");
    test_store_internal(&field_dec, &d10_01009, "10.0101", 10, 10.0101,
                        E_DEC_OK, WARN_DATA_TRUNCATED, TYPE_NOTE_TRUNCATED);
  }
  {
    SCOPED_TRACE("");
    test_store_internal(&field_dec, &dInsignificant, "0.0000", 0, 0, E_DEC_OK,
                        WARN_DATA_TRUNCATED, TYPE_NOTE_TRUNCATED);
  }
}

/**
  Test store_internal_with_error_check() - out of range valuse
*/
TEST_F(FieldNewDecimalTest, storeInternalWithErrorCheckOutOfRange) {
  // Alows storing this range [-99.9999, 99.9999]
  Mock_field_new_decimal field_dec(4);
  Fake_TABLE table(&field_dec);
  table.in_use = thd();
  field_dec.make_writable();
  field_dec.make_readable();
  thd()->check_for_truncated_fields = CHECK_FIELD_WARN;

  my_decimal dTooHigh;
  my_decimal dTooLow;

  EXPECT_EQ(0, chars_2_decimal("1000", &dTooHigh));
  EXPECT_EQ(0, chars_2_decimal("-1000", &dTooLow));

  {
    SCOPED_TRACE("");
    test_store_internal(&field_dec, &dTooHigh, "99.9999", 100, 99.9999,
                        E_DEC_OK, ER_WARN_DATA_OUT_OF_RANGE,
                        TYPE_WARN_OUT_OF_RANGE);
  }
  {
    SCOPED_TRACE("");
    test_store_internal(&field_dec, &dTooLow, "-99.9999", -100, -99.9999,
                        E_DEC_OK, ER_WARN_DATA_OUT_OF_RANGE,
                        TYPE_WARN_OUT_OF_RANGE);
  }
}

/**
  Test store_internal_with_error_check() - Test first parameter: the error.

  When E_DEC_OVERFLOW is specified, the min/max value (depends on
  the sign of the input value) of the field is used to overwrite the
  input decimal value because E_DEC_OVERFLOW indicates that the decimal
  conversion got a number that was too high/low.
*/
TEST_F(FieldNewDecimalTest, storeInternalWithErrorCheckEDecOverflow) {
  // Alows storing this range [-99.9999, 99.9999]
  Mock_field_new_decimal field_dec(4);
  Fake_TABLE table(&field_dec);
  table.in_use = thd();
  field_dec.make_writable();
  field_dec.make_readable();
  thd()->check_for_truncated_fields = CHECK_FIELD_WARN;

  my_decimal d10_01;
  my_decimal dMin10_01;
  my_decimal dInsignificant;
  my_decimal dTooHigh;
  my_decimal dTooLow;

  EXPECT_EQ(0, chars_2_decimal("10.01", &d10_01));
  EXPECT_EQ(0, chars_2_decimal("-10.01", &dMin10_01));
  EXPECT_EQ(0, chars_2_decimal("0.00000000001", &dInsignificant));
  EXPECT_EQ(0, chars_2_decimal("1000", &dTooHigh));
  EXPECT_EQ(0, chars_2_decimal("-1000", &dTooLow));

  // Positive number - the field's MAX value is used
  {
    SCOPED_TRACE("");
    test_store_internal(&field_dec, &d10_01, "99.9999", 100, 99.9999,
                        E_DEC_OVERFLOW, ER_WARN_DATA_OUT_OF_RANGE,
                        TYPE_WARN_OUT_OF_RANGE);
  }
  {
    SCOPED_TRACE("");
    test_store_internal(&field_dec, &dInsignificant, "99.9999", 100, 99.9999,
                        E_DEC_OVERFLOW, ER_WARN_DATA_OUT_OF_RANGE,
                        TYPE_WARN_OUT_OF_RANGE);
  }
  {
    SCOPED_TRACE("");
    test_store_internal(&field_dec, &dTooHigh, "99.9999", 100, 99.9999,
                        E_DEC_OVERFLOW, ER_WARN_DATA_OUT_OF_RANGE,
                        TYPE_WARN_OUT_OF_RANGE);
  }

  // Negative number - the field's MIN value is used
  {
    SCOPED_TRACE("");
    test_store_internal(&field_dec, &dMin10_01, "-99.9999", -100, -99.9999,
                        E_DEC_OVERFLOW, ER_WARN_DATA_OUT_OF_RANGE,
                        TYPE_WARN_OUT_OF_RANGE);
  }
  {
    SCOPED_TRACE("");
    test_store_internal(&field_dec, &dTooLow, "-99.9999", -100, -99.9999,
                        E_DEC_OVERFLOW, ER_WARN_DATA_OUT_OF_RANGE,
                        TYPE_WARN_OUT_OF_RANGE);
  }
}

/**
  Test store_internal_with_error_check() - Test first parameter: the error.

  When E_DEC_TRUNCATED is specified, a truncation warning will
  appear iff Field_new_decimal::store_value() would otherwise not
  issue a warning. The rationale is that E_DEC_TRUNCATED indicates
  that the value to store has already been truncated, something that
  will not be noticed by store_value(). However, the value is not
  automatically changed like in the E_DEC_OVERFLOW case tested
  above.
*/
TEST_F(FieldNewDecimalTest, storeInternalWithErrorCheckEDecTrunkated) {
  // Alows storing this range [-99.9999, 99.9999]
  Mock_field_new_decimal field_dec(4);
  Fake_TABLE table(&field_dec);
  table.in_use = thd();
  field_dec.make_writable();
  field_dec.make_readable();
  thd()->check_for_truncated_fields = CHECK_FIELD_WARN;

  my_decimal d10_01;
  my_decimal dMin10_01;
  my_decimal dInsignificant;
  my_decimal dTooHigh;
  my_decimal dTooLow;

  EXPECT_EQ(0, chars_2_decimal("10.01", &d10_01));
  EXPECT_EQ(0, chars_2_decimal("-10.01", &dMin10_01));
  EXPECT_EQ(0, chars_2_decimal("0.00000000001", &dInsignificant));
  EXPECT_EQ(0, chars_2_decimal("1000", &dTooHigh));
  EXPECT_EQ(0, chars_2_decimal("-1000", &dTooLow));

  // Conversion went fine
  {
    SCOPED_TRACE("");
    test_store_internal(&field_dec, &d10_01, "10.0100", 10, 10.01,
                        E_DEC_TRUNCATED, WARN_DATA_TRUNCATED,
                        TYPE_NOTE_TRUNCATED);
  }
  {
    SCOPED_TRACE("");
    test_store_internal(&field_dec, &dMin10_01, "-10.0100", -10, -10.01,
                        E_DEC_TRUNCATED, WARN_DATA_TRUNCATED,
                        TYPE_NOTE_TRUNCATED);
  }

  {
    SCOPED_TRACE("");
    test_store_internal(&field_dec, &dInsignificant, "0.0000", 0, 0,
                        E_DEC_TRUNCATED, WARN_DATA_TRUNCATED,
                        TYPE_NOTE_TRUNCATED);
  }

  /*
    In what follows, the values are out of range causing warning
    ER_WARN_DATA_OUT_OF_RANGE instead of WARN_DATA_TRUNCATED.
  */
  {
    SCOPED_TRACE("");
    test_store_internal(&field_dec, &dTooHigh, "99.9999", 100, 99.9999,
                        E_DEC_TRUNCATED, ER_WARN_DATA_OUT_OF_RANGE,
                        TYPE_WARN_OUT_OF_RANGE);
  }
  {
    SCOPED_TRACE("");
    test_store_internal(&field_dec, &dTooLow, "-99.9999", -100, -99.9999,
                        E_DEC_TRUNCATED, ER_WARN_DATA_OUT_OF_RANGE,
                        TYPE_WARN_OUT_OF_RANGE);
  }
}

/**
  Test store_internal_with_error_check() - Test first parameter: the error.

  Any E_DEC_* value other than E_DEC_OK, E_DEC_TRUNCATED and
  E_DEC_OVERFLOW will be ignored.
*/
TEST_F(FieldNewDecimalTest, storeInternalWithErrorCheckRestOfParams) {
  // Alows storing this range [-99.9999, 99.9999]
  Mock_field_new_decimal field_dec(4);
  Fake_TABLE table(&field_dec);
  table.in_use = thd();
  field_dec.make_writable();
  field_dec.make_readable();
  thd()->check_for_truncated_fields = CHECK_FIELD_WARN;

  my_decimal d10_01;
  EXPECT_EQ(0, chars_2_decimal("10.01", &d10_01));

  test_store_internal(&field_dec, &d10_01, "10.0100", 10, 10.01, E_DEC_DIV_ZERO,
                      0, TYPE_OK);

  test_store_internal(&field_dec, &d10_01, "10.0100", 10, 10.01, E_DEC_BAD_NUM,
                      0, TYPE_OK);

  test_store_internal(&field_dec, &d10_01, "10.0100", 10, 10.01, E_DEC_OOM, 0,
                      TYPE_OK);
}

}  // namespace field_newdecimal_unittest
