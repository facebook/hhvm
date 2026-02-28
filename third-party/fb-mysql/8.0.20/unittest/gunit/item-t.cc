/* Copyright (c) 2011, 2020, Oracle and/or its affiliates. All rights reserved.

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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <limits.h>
#include <stddef.h>
#include <sys/types.h>
#include <memory>
#include <new>

#include "decimal.h"
#include "field_types.h"
#include "lex_string.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_alloc.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "my_table_map.h"
#include "mysql_time.h"
#include "mysys_err.h"
#include "sql/item.h"
#include "sql/item_cmpfunc.h"
#include "sql/item_create.h"
#include "sql/item_strfunc.h"
#include "sql/item_timefunc.h"
#include "sql/json_dom.h"
#include "sql/my_decimal.h"
#include "sql/sql_class.h"
#include "sql/sql_lex.h"
#include "sql/tztime.h"
#include "sql_string.h"
#include "unittest/gunit/fake_table.h"
#include "unittest/gunit/mock_field_timestamp.h"
#include "unittest/gunit/mysys_util.h"
#include "unittest/gunit/test_utils.h"

namespace item_unittest {

using my_testing::Mock_error_handler;
using my_testing::Server_initializer;
using ::testing::Return;

class ItemTest : public ::testing::Test {
 protected:
  virtual void SetUp() { initializer.SetUp(); }
  virtual void TearDown() { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;
};

/**
  This is a simple mock Field class, illustrating how to set expectations on
  type_conversion_status Field_long::store(longlong nr, bool unsigned_val);
*/
class Mock_field_long : public Field_long {
 public:
  Mock_field_long(uint32 length)
      : Field_long(nullptr,      // ptr_arg
                   length,       // len_arg
                   nullptr,      // null_ptr_arg
                   0,            // null_bit_arg
                   Field::NONE,  // auto_flags_arg
                   nullptr,      // field_name_arg
                   false,        // zero_arg
                   false)        // unsigned_arg
  {
    table = new Fake_TABLE(this);
    ptr = table->record[0];

    // Make it possible to write into this field
    bitmap_set_bit(table->write_set, 0);
  }

  ~Mock_field_long() override {
    delete static_cast<Fake_TABLE *>(table);
    table = nullptr;
  }

  // Avoid warning about hiding other overloaded versions of store().
  using Field_long::store;

  /*
    This is the only member function we need to override.
    Note: Sun Studio needs a little help in resolving longlong.
  */
  MOCK_METHOD2(store, type_conversion_status(::longlong nr, bool unsigned_val));
};

/**
  Mock class for CHAR field.
*/

class Mock_field_string : public Field_string {
 private:
  Fake_TABLE *m_fake_tbl;

 public:
  Mock_field_string(uint32 length, const CHARSET_INFO *cs = &my_charset_latin1)
      : Field_string(nullptr,      // ptr_arg
                     length,       // len_arg
                     nullptr,      // null_ptr_arg
                     0,            // null_bit_arg
                     Field::NONE,  // auto_flags_arg
                     nullptr,      // field_name_arg
                     cs)           // char set
  {
    m_fake_tbl = new Fake_TABLE(this);

    // Allocate place for storing the field value
    ptr = m_fake_tbl->record[0];

    // Make it possible to write into this field
    bitmap_set_bit(m_fake_tbl->write_set, 0);

    /*
      check_for_truncated_fields must be set in order for producing
      warning/error for Item_string::save_in_field().
    */
    m_fake_tbl->in_use->check_for_truncated_fields = CHECK_FIELD_WARN;
  }

  ~Mock_field_string() {
    delete m_fake_tbl;
    m_fake_tbl = nullptr;
  }
};

/**
  Mock class for VARCHAR field.
*/

class Mock_field_varstring : public Field_varstring {
 private:
  Fake_TABLE *m_fake_tbl;

 public:
  Mock_field_varstring(uint32 length, TABLE_SHARE *share,
                       const CHARSET_INFO *cs = &my_charset_latin1)
      : Field_varstring(length,   // len_arg
                        false,    // maybe_null_arg
                        nullptr,  // field_name_arg
                        share,    // share
                        cs)       // char set
  {
    m_fake_tbl = new Fake_TABLE(this);

    // Allocate place for storing the field value
    ptr = m_fake_tbl->record[0];

    // Make it possible to write into this field
    bitmap_set_bit(m_fake_tbl->write_set, 0);

    /*
      check_for_truncated_fields must be set in order for producing
      warning/error for Item_string::save_in_field().
    */
    m_fake_tbl->in_use->check_for_truncated_fields = CHECK_FIELD_WARN;
  }

  ~Mock_field_varstring() {
    delete m_fake_tbl;
    m_fake_tbl = nullptr;
  }
};

TEST_F(ItemTest, ItemInt) {
  const int32 val = 42;
  char stringbuf[10];
  (void)snprintf(stringbuf, sizeof(stringbuf), "%d", val);

  // An Item expects to be owned by current_thd->free_list,
  // so allocate with new, and do not delete it.
  Item_int *item_int = new Item_int(val);

  EXPECT_EQ(Item::INT_ITEM, item_int->type());
  EXPECT_EQ(INT_RESULT, item_int->result_type());
  EXPECT_EQ(MYSQL_TYPE_LONGLONG, item_int->data_type());
  EXPECT_EQ(val, item_int->val_int());
  EXPECT_DOUBLE_EQ((double)val, item_int->val_real());
  EXPECT_TRUE(item_int->basic_const_item());

  my_decimal decimal_val;
  EXPECT_EQ(&decimal_val, item_int->val_decimal(&decimal_val));

  String string_val;
  EXPECT_EQ(&string_val, item_int->val_str(&string_val));
  EXPECT_STREQ(stringbuf, string_val.c_ptr_safe());

  Mock_field_long field_val(item_int->max_length);
  // We expect to be called with arguments(nr == val, unsigned_val == false)
  EXPECT_CALL(field_val, store(val, false))
      .Times(1)
      .WillRepeatedly(Return(TYPE_OK));
  EXPECT_EQ(TYPE_OK, item_int->save_in_field(&field_val, true));

  Item *clone = item_int->clone_item();
  EXPECT_TRUE(item_int->eq(clone, true));
  EXPECT_TRUE(item_int->eq(item_int, true));

  String print_val;
  item_int->print(thd(), &print_val, QT_ORDINARY);
  EXPECT_STREQ(stringbuf, print_val.c_ptr_safe());

  const uint precision = item_int->decimal_precision();
  EXPECT_EQ(MY_INT32_NUM_DECIMAL_DIGITS, precision);

  item_int->neg();
  EXPECT_EQ(-val, item_int->val_int());
  EXPECT_EQ(precision - 1, item_int->decimal_precision());

  // Functions inherited from parent class(es).
  const table_map tmap = 0;
  EXPECT_EQ(tmap, item_int->used_tables());

  /*
   TODO: There are about 100 member functions in Item.
         Figure out which ones are relevant for unit testing here.
  */
}

TEST_F(ItemTest, ItemString) {
  const char short_str[] = "abc";
  const char long_str[] = "abcd";
  const char space_str[] = "abc ";
  const char bad_char[] = "𝌆abc";
  const char bad_char_end[] = "abc𝌆";

  Item_string *item_short_string =
      new Item_string(STRING_WITH_LEN(short_str), &my_charset_latin1);
  Item_string *item_long_string =
      new Item_string(STRING_WITH_LEN(long_str), &my_charset_latin1);
  Item_string *item_space_string =
      new Item_string(STRING_WITH_LEN(space_str), &my_charset_latin1);
  Item_string *item_bad_char =
      new Item_string(STRING_WITH_LEN(bad_char), &my_charset_bin);
  Item_string *item_bad_char_end =
      new Item_string(STRING_WITH_LEN(bad_char_end), &my_charset_bin);

  /*
    Bug 16407965 ITEM::SAVE_IN_FIELD_NO_WARNING() DOES NOT RETURN CORRECT
                 CONVERSION STATUS
  */

  // Create a CHAR field that can store short_str but not long_str
  Mock_field_string field_string(3);
  EXPECT_EQ(MYSQL_TYPE_STRING, field_string.type());

  // CHAR field for testing strings with illegal values
  Mock_field_string field_string_utf8(20, &my_charset_utf8_general_ci);
  EXPECT_EQ(MYSQL_TYPE_STRING, field_string_utf8.type());

  /*
    Tests of Item_string::save_in_field() when storing into a CHAR field.
  */
  EXPECT_EQ(TYPE_OK, item_short_string->save_in_field(&field_string, true));
  EXPECT_EQ(TYPE_WARN_TRUNCATED,
            item_long_string->save_in_field(&field_string, true));
  // Field_string does not consider trailing spaces when truncating a string
  EXPECT_EQ(TYPE_OK, item_space_string->save_in_field(&field_string, true));
  // When the first character invalid, the whole string is truncated.
  EXPECT_EQ(TYPE_WARN_INVALID_STRING,
            item_bad_char->save_in_field(&field_string_utf8, true));
  // If the string contains an invalid character, the entire string is invalid
  EXPECT_EQ(TYPE_WARN_INVALID_STRING,
            item_bad_char_end->save_in_field(&field_string_utf8, true));

  /*
    Tests of Item_string::save_in_field_no_warnings() when storing into
    a CHAR field.
  */
  EXPECT_EQ(TYPE_OK,
            item_short_string->save_in_field_no_warnings(&field_string, true));
  EXPECT_EQ(TYPE_WARN_TRUNCATED,
            item_long_string->save_in_field_no_warnings(&field_string, true));
  // Field_string does not consider trailing spaces when truncating a string
  EXPECT_EQ(TYPE_OK,
            item_space_string->save_in_field_no_warnings(&field_string, true));
  EXPECT_EQ(TYPE_WARN_INVALID_STRING,
            item_bad_char->save_in_field_no_warnings(&field_string_utf8, true));
  // If the string contains an invalid character, the entire string is invalid
  EXPECT_EQ(
      TYPE_WARN_INVALID_STRING,
      item_bad_char_end->save_in_field_no_warnings(&field_string_utf8, true));

  /*
    Create a VARCHAR field that can store short_str but not long_str.
    Need a table share object since the constructor for Field_varstring
    updates its table share.
  */
  TABLE_SHARE table_share;
  Mock_field_varstring field_varstring(3, &table_share);
  EXPECT_EQ(MYSQL_TYPE_VARCHAR, field_varstring.type());

  // VARCHAR field for testing strings with illegal values
  Mock_field_varstring field_varstring_utf8(20, &table_share,
                                            &my_charset_utf8_general_ci);
  EXPECT_EQ(MYSQL_TYPE_VARCHAR, field_varstring_utf8.type());

  /*
    Tests of Item_string::save_in_field() when storing into a VARCHAR field.
  */
  EXPECT_EQ(TYPE_OK, item_short_string->save_in_field(&field_varstring, true));
  EXPECT_EQ(TYPE_WARN_TRUNCATED,
            item_long_string->save_in_field(&field_varstring, true));
  // Field_varstring produces a note when truncating a string with
  // trailing spaces
  EXPECT_EQ(TYPE_NOTE_TRUNCATED,
            item_space_string->save_in_field(&field_varstring, true));
  // When the first character invalid, the whole string is truncated.
  EXPECT_EQ(TYPE_WARN_INVALID_STRING,
            item_bad_char->save_in_field(&field_varstring_utf8, true));
  // If the string contains an invalid character, the entire string is invalid
  EXPECT_EQ(TYPE_WARN_INVALID_STRING,
            item_bad_char_end->save_in_field(&field_varstring_utf8, true));

  /*
    Tests of Item_string::save_in_field_no_warnings() when storing into
    a VARCHAR field.
  */
  EXPECT_EQ(TYPE_OK, item_short_string->save_in_field_no_warnings(
                         &field_varstring, true));
  EXPECT_EQ(TYPE_WARN_TRUNCATED, item_long_string->save_in_field_no_warnings(
                                     &field_varstring, true));
  // Field_varstring produces a note when truncating a string with
  // trailing spaces
  EXPECT_EQ(TYPE_NOTE_TRUNCATED, item_space_string->save_in_field_no_warnings(
                                     &field_varstring, true));
  EXPECT_EQ(TYPE_WARN_INVALID_STRING, item_bad_char->save_in_field_no_warnings(
                                          &field_varstring_utf8, true));
  // If the string contains an invalid character, the entire string is invalid
  EXPECT_EQ(TYPE_WARN_INVALID_STRING,
            item_bad_char_end->save_in_field_no_warnings(&field_varstring_utf8,
                                                         true));
}

TEST_F(ItemTest, ItemEqual) {
  // Bug#13720201 VALGRIND: VARIOUS BLOCKS OF BYTES DEFINITELY LOST
  Mock_field_timestamp mft;
  mft.table->const_table = true;
  mft.make_readable();
  // foo is longer than STRING_BUFFER_USUAL_SIZE used by cmp_item_sort_string.
  const char foo[] =
      "0123456789012345678901234567890123456789"
      "0123456789012345678901234567890123456789"
      "0123456789012345678901234567890123456789";
  Item_equal *item_equal =
      new Item_equal(new Item_string(STRING_WITH_LEN(foo), &my_charset_bin),
                     new Item_field(&mft));

  EXPECT_FALSE(item_equal->fix_fields(thd(), nullptr));
  EXPECT_EQ(1, item_equal->val_int());
}

TEST_F(ItemTest, ItemFuncExportSet) {
  String str;
  Item *on_string = new Item_string(STRING_WITH_LEN("on"), &my_charset_bin);
  Item *off_string = new Item_string(STRING_WITH_LEN("off"), &my_charset_bin);
  Item *sep_string = new Item_string(STRING_WITH_LEN(","), &my_charset_bin);
  {
    // Testing basic functionality.
    Item *export_set =
        new Item_func_export_set(POS(), new Item_int(2), on_string, off_string,
                                 sep_string, new Item_int(4));
    Parse_context pc(thd(), thd()->lex->current_select());
    EXPECT_FALSE(export_set->itemize(&pc, &export_set));
    EXPECT_FALSE(export_set->fix_fields(thd(), nullptr));
    EXPECT_EQ(&str, export_set->val_str(&str));
    EXPECT_STREQ("off,on,off,off", str.c_ptr_safe());
  }
  {
    // Testing corner case: number_of_bits == zero.
    Item *export_set =
        new Item_func_export_set(POS(), new Item_int(2), on_string, off_string,
                                 sep_string, new Item_int(0));
    Parse_context pc(thd(), thd()->lex->current_select());
    EXPECT_FALSE(export_set->itemize(&pc, &export_set));
    EXPECT_FALSE(export_set->fix_fields(thd(), nullptr));
    EXPECT_EQ(&str, export_set->val_str(&str));
    EXPECT_STREQ("", str.c_ptr_safe());
  }

  /*
    Bug#11765562 58545:
    EXPORT_SET() CAN BE USED TO MAKE ENTIRE SERVER COMPLETELY UNRESPONSIVE
   */
  const ulong max_packet_size = 1024;
  const ulonglong repeat = max_packet_size / 2;
  Item *item_int_repeat = new Item_int(repeat);
  Item *string_x = new Item_string(STRING_WITH_LEN("x"), &my_charset_bin);
  String *const null_string = nullptr;
  thd()->variables.max_allowed_packet = max_packet_size;
  {
    // Testing overflow caused by 'on-string'.
    Mock_error_handler error_handler(thd(), ER_WARN_ALLOWED_PACKET_OVERFLOWED);
    Item *export_set = new Item_func_export_set(
        POS(), new Item_int(0xff),
        new Item_func_repeat(POS(), string_x, item_int_repeat), string_x,
        sep_string);
    Parse_context pc(thd(), thd()->lex->current_select());
    SCOPED_TRACE("");
    EXPECT_FALSE(export_set->itemize(&pc, &export_set));
    EXPECT_FALSE(export_set->fix_fields(thd(), nullptr));
    EXPECT_EQ(null_string, export_set->val_str(&str));
    EXPECT_STREQ("", str.c_ptr_safe());
    EXPECT_EQ(1, error_handler.handle_called());
  }
  {
    // Testing overflow caused by 'off-string'.
    Mock_error_handler error_handler(thd(), ER_WARN_ALLOWED_PACKET_OVERFLOWED);
    Item *export_set = new Item_func_export_set(
        POS(), new Item_int(0xff), string_x,
        new Item_func_repeat(POS(), string_x, item_int_repeat), sep_string);
    Parse_context pc(thd(), thd()->lex->current_select());
    SCOPED_TRACE("");
    EXPECT_FALSE(export_set->itemize(&pc, &export_set));
    EXPECT_FALSE(export_set->fix_fields(thd(), nullptr));
    EXPECT_EQ(null_string, export_set->val_str(&str));
    EXPECT_STREQ("", str.c_ptr_safe());
    EXPECT_EQ(1, error_handler.handle_called());
  }
  {
    // Testing overflow caused by 'separator-string'.
    Mock_error_handler error_handler(thd(), ER_WARN_ALLOWED_PACKET_OVERFLOWED);
    Item *export_set = new Item_func_export_set(
        POS(), new Item_int(0xff), string_x, string_x,
        new Item_func_repeat(POS(), string_x, item_int_repeat));
    Parse_context pc(thd(), thd()->lex->current_select());
    SCOPED_TRACE("");
    EXPECT_FALSE(export_set->itemize(&pc, &export_set));
    EXPECT_FALSE(export_set->fix_fields(thd(), nullptr));
    EXPECT_EQ(null_string, export_set->val_str(&str));
    EXPECT_STREQ("", str.c_ptr_safe());
    EXPECT_EQ(1, error_handler.handle_called());
  }
  {
    // Testing overflow caused by 'on-string'.
    longlong max_size = 1024LL;
    thd()->variables.max_allowed_packet = static_cast<ulong>(max_size);
    Mock_error_handler error_handler(thd(), ER_WARN_ALLOWED_PACKET_OVERFLOWED);
    Item *lpad = new Item_func_lpad(
        POS(), new Item_string(STRING_WITH_LEN("a"), &my_charset_bin),
        new Item_int(max_size),
        new Item_string(STRING_WITH_LEN("pppppppppppppppp"
                                        "pppppppppppppppp"),
                        &my_charset_bin));
    Item *export_set = new Item_func_export_set(
        POS(), new Item_string(STRING_WITH_LEN("1111111"), &my_charset_bin),
        lpad, new Item_int(1));
    Parse_context pc(thd(), thd()->lex->current_select());
    SCOPED_TRACE("");
    EXPECT_FALSE(export_set->itemize(&pc, &export_set));
    EXPECT_FALSE(export_set->fix_fields(thd(), nullptr));
    EXPECT_EQ(null_string, export_set->val_str(&str));
    EXPECT_STREQ("", str.c_ptr_safe());
    EXPECT_EQ(1, error_handler.handle_called());
  }
}

TEST_F(ItemTest, ItemFuncIntDivOverflow) {
  const char dividend_str[] =
      "99999999999999999999999999999999999999999"
      "99999999999999999999999999999999999999999";
  const char divisor_str[] = "0.5";
  Item_float *dividend = new Item_float(dividend_str, sizeof(dividend_str));
  Item_float *divisor = new Item_float(divisor_str, sizeof(divisor_str));
  Item_func_int_div *quotient = new Item_func_int_div(dividend, divisor);

  Mock_error_handler error_handler(thd(), ER_TRUNCATED_WRONG_VALUE);
  EXPECT_FALSE(quotient->fix_fields(thd(), nullptr));
  initializer.set_expected_error(ER_DATA_OUT_OF_RANGE);
  quotient->val_int();
}

TEST_F(ItemTest, ItemFuncIntDivUnderflow) {
  // Bug #11792200 - DIVIDING LARGE NUMBERS CAUSES STACK CORRUPTIONS
  const char dividend_str[] = "1.175494351E-37";
  const char divisor_str[] = "1.7976931348623157E+308";
  Item_float *dividend = new Item_float(dividend_str, sizeof(dividend_str));
  Item_float *divisor = new Item_float(divisor_str, sizeof(divisor_str));
  Item_func_int_div *quotient = new Item_func_int_div(dividend, divisor);

  Mock_error_handler error_handler(thd(), ER_TRUNCATED_WRONG_VALUE);
  EXPECT_FALSE(quotient->fix_fields(thd(), nullptr));
  EXPECT_EQ(0, quotient->val_int());
}

TEST_F(ItemTest, ItemFuncNegLongLongMin) {
  // Bug#14314156 MAIN.FUNC_MATH TEST FAILS ON MYSQL-TRUNK ON PB2
  const longlong longlong_min = LLONG_MIN;
  Item_func_neg *item_neg = new Item_func_neg(new Item_int(longlong_min));

  EXPECT_FALSE(item_neg->fix_fields(thd(), nullptr));
  initializer.set_expected_error(ER_DATA_OUT_OF_RANGE);
  EXPECT_EQ(0, item_neg->int_op());
}

/*
  This is not an exhaustive test. It simply demonstrates that more of the
  initializations in mysqld.cc are needed for testing Item_xxx classes.
*/
TEST_F(ItemTest, ItemFuncSetUserVar) {
  const longlong val1 = 1;
  Item_decimal *item_dec = new Item_decimal(val1, false);
  Item_string *item_str = new Item_string("1", 1, &my_charset_latin1);

  LEX_CSTRING var_name = {STRING_WITH_LEN("a")};
  Item_func_set_user_var *user_var =
      new Item_func_set_user_var(var_name, item_str, false);
  EXPECT_FALSE(user_var->set_entry(thd(), true));
  EXPECT_FALSE(user_var->fix_fields(thd(), nullptr));
  EXPECT_EQ(val1, user_var->val_int());

  my_decimal decimal;
  my_decimal *decval_1 = user_var->val_decimal(&decimal);
  user_var->save_item_result(item_str);
  my_decimal *decval_2 = user_var->val_decimal(&decimal);
  user_var->save_item_result(item_dec);

  EXPECT_EQ(decval_1, decval_2);
  EXPECT_EQ(decval_1, &decimal);
}

// Test of Item::operator new() when we simulate out-of-memory.
TEST_F(ItemTest, OutOfMemory) {
  Item_int *item = new Item_int(42);
  EXPECT_NE(nullptr, item);

#if !defined(DBUG_OFF)
  // Setting debug flags triggers enter/exit trace, so redirect to /dev/null.
  DBUG_SET("o," IF_WIN("NUL", "/dev/null"));

  DBUG_SET("+d,simulate_out_of_memory");
  initializer.set_expected_error(EE_OUTOFMEMORY);
  item = new Item_int(42);
  EXPECT_EQ(nullptr, item);

  DBUG_SET("+d,simulate_out_of_memory");
  item = new (thd()->mem_root) Item_int(42);
  EXPECT_EQ(nullptr, item);
#endif
}

// We never use dynamic_cast, but we expect it to work.
TEST_F(ItemTest, DynamicCast) {
  Item *item = new Item_int(42);
  const Item_int *null_item = nullptr;
  EXPECT_NE(null_item, dynamic_cast<Item_int *>(item));
}

TEST_F(ItemTest, ItemFuncXor) {
  const uint length = 1U;
  Item_int *item_zero = new Item_int(0, length);
  Item_int *item_one_a = new Item_int(1, length);

  Item_func_xor *item_xor = new Item_func_xor(item_zero, item_one_a);

  EXPECT_FALSE(item_xor->fix_fields(thd(), nullptr));
  EXPECT_EQ(1, item_xor->val_int());
  EXPECT_EQ(1U, item_xor->decimal_precision());

  Item_int *item_one_b = new Item_int(1, length);

  Item_func_xor *item_xor_same = new Item_func_xor(item_one_a, item_one_b);

  EXPECT_FALSE(item_xor_same->fix_fields(thd(), nullptr));
  EXPECT_EQ(0, item_xor_same->val_int());
  EXPECT_FALSE(item_xor_same->val_bool());
  EXPECT_FALSE(item_xor_same->is_null());

  String print_buffer;
  item_xor->print(thd(), &print_buffer, QT_ORDINARY);
  EXPECT_STREQ("(0 xor 1)", print_buffer.c_ptr_safe());

  Item *neg_xor = item_xor->truth_transformer(thd(), Item::BOOL_NEGATED);
  EXPECT_FALSE(neg_xor->fix_fields(thd(), nullptr));
  EXPECT_EQ(0, neg_xor->val_int());
  EXPECT_DOUBLE_EQ(0.0, neg_xor->val_real());
  EXPECT_FALSE(neg_xor->val_bool());
  EXPECT_FALSE(neg_xor->is_null());

  print_buffer = String();
  neg_xor->print(thd(), &print_buffer, QT_ORDINARY);
  EXPECT_STREQ("((not(0)) xor 1)", print_buffer.c_ptr_safe());

  Item_func_xor *item_xor_null = new Item_func_xor(item_zero, new Item_null());
  EXPECT_FALSE(item_xor_null->fix_fields(thd(), nullptr));

  EXPECT_EQ(0, item_xor_null->val_int());
  EXPECT_TRUE(item_xor_null->is_null());
}

/*
  Testing MYSQL_TIME_cache.
*/
TEST_F(ItemTest, MysqlTimeCache) {
  String str_buff, *str;
  MysqlTime datetime6(2011, 11, 7, 10, 20, 30, 123456, false,
                      MYSQL_TIMESTAMP_DATETIME);
  MysqlTime time6(0, 0, 0, 10, 20, 30, 123456, false, MYSQL_TIMESTAMP_TIME);
  struct timeval tv6 = {1320661230, 123456};
  const MYSQL_TIME *ltime;
  MYSQL_TIME_cache cache;

  /*
    Testing DATETIME(6).
    Initializing from MYSQL_TIME.
  */
  cache.set_datetime(&datetime6, 6);
  EXPECT_EQ(1840440237558456896LL, cache.val_packed());
  EXPECT_EQ(6, cache.decimals());
  // Call val_str() then cptr()
  str = cache.val_str(&str_buff);
  EXPECT_STREQ("2011-11-07 10:20:30.123456", str->c_ptr_safe());
  EXPECT_STREQ("2011-11-07 10:20:30.123456", cache.cptr());
  cache.set_datetime(&datetime6, 6);
  // Now call the other way around: cptr() then val_str()
  EXPECT_STREQ("2011-11-07 10:20:30.123456", cache.cptr());
  EXPECT_STREQ("2011-11-07 10:20:30.123456", str->c_ptr_safe());
  // Testing get_TIME_ptr()
  ltime = cache.get_TIME_ptr();
  EXPECT_EQ(ltime->year, datetime6.year);
  EXPECT_EQ(ltime->month, datetime6.month);
  EXPECT_EQ(ltime->day, datetime6.day);
  EXPECT_EQ(ltime->hour, datetime6.hour);
  EXPECT_EQ(ltime->minute, datetime6.minute);
  EXPECT_EQ(ltime->second, datetime6.second);
  EXPECT_EQ(ltime->second_part, datetime6.second_part);
  EXPECT_EQ(ltime->neg, datetime6.neg);
  EXPECT_EQ(ltime->time_type, datetime6.time_type);
  // Testing eq()
  {
    MYSQL_TIME datetime6_2 = datetime6;
    MYSQL_TIME_cache cache2;
    datetime6_2.second_part += 1;
    cache2.set_datetime(&datetime6_2, 6);
    EXPECT_EQ(cache.eq(cache), true);
    EXPECT_EQ(cache.eq(cache2), false);
    EXPECT_EQ(cache2.eq(cache2), true);
    EXPECT_EQ(cache2.eq(cache), false);
  }

  /*
     Testing DATETIME(6).
     Initializing from "struct timeval".
  */
  cache.set_datetime(tv6, 6, my_tz_UTC);
  EXPECT_EQ(1840440237558456896LL, cache.val_packed());
  EXPECT_EQ(6, cache.decimals());
  str = cache.val_str(&str_buff);
  EXPECT_STREQ("2011-11-07 10:20:30.123456", str->c_ptr_safe());
  EXPECT_STREQ("2011-11-07 10:20:30.123456", cache.cptr());

  /*
    Testing TIME(6).
    Initializing from MYSQL_TIME.
  */
  cache.set_time(&time6, 6);
  EXPECT_EQ(709173043776LL, cache.val_packed());
  EXPECT_EQ(6, cache.decimals());
  // Call val_str() then cptr()
  str = cache.val_str(&str_buff);
  EXPECT_STREQ("10:20:30.123456", str->c_ptr_safe());
  EXPECT_STREQ("10:20:30.123456", cache.cptr());

  /*
    Testing TIME(6).
    Initializing from "struct timeval".
  */
  cache.set_time(tv6, 6, my_tz_UTC);
  EXPECT_EQ(709173043776LL, cache.val_packed());
  EXPECT_EQ(6, cache.decimals());
  str = cache.val_str(&str_buff);
  EXPECT_STREQ("10:20:30.123456", str->c_ptr_safe());
  EXPECT_STREQ("10:20:30.123456", cache.cptr());

  /*
    Testing DATETIME(5)
  */
  MysqlTime datetime5(2011, 11, 7, 10, 20, 30, 123450, false,
                      MYSQL_TIMESTAMP_DATETIME);
  cache.set_datetime(&datetime5, 5);
  EXPECT_EQ(1840440237558456890LL, cache.val_packed());
  EXPECT_EQ(5, cache.decimals());
  /* Call val_str() then cptr() */
  str = cache.val_str(&str_buff);
  EXPECT_STREQ("2011-11-07 10:20:30.12345", str->c_ptr_safe());
  EXPECT_STREQ("2011-11-07 10:20:30.12345", cache.cptr());
  cache.set_datetime(&datetime5, 5);
  /* Now call the other way around: cptr() then val_str() */
  EXPECT_STREQ("2011-11-07 10:20:30.12345", cache.cptr());
  EXPECT_STREQ("2011-11-07 10:20:30.12345", str->c_ptr_safe());

  /*
    Testing DATE.
    Initializing from MYSQL_TIME.
  */
  MysqlTime date(2011, 11, 7, 0, 0, 0, 0, false, MYSQL_TIMESTAMP_DATE);
  cache.set_date(&date);
  EXPECT_EQ(1840439528385413120LL, cache.val_packed());
  EXPECT_EQ(0, cache.decimals());
  str = cache.val_str(&str_buff);
  EXPECT_STREQ("2011-11-07", str->c_ptr_safe());
  EXPECT_STREQ("2011-11-07", cache.cptr());

  /*
    Testing DATE.
    Initializing from "struct tm".
  */
  cache.set_date(tv6, my_tz_UTC);
  EXPECT_EQ(1840439528385413120LL, cache.val_packed());
  EXPECT_EQ(0, cache.decimals());
  str = cache.val_str(&str_buff);
  EXPECT_STREQ("2011-11-07", str->c_ptr_safe());
  EXPECT_STREQ("2011-11-07", cache.cptr());
}

extern "C" {
// Verifies that Item_func_conv::val_str does not call my_strntoll()
longlong fail_strntoll(const CHARSET_INFO *, const char *, size_t, int,
                       const char **, int *) {
  ADD_FAILURE() << "Unexpected call";
  return 0;
}
}

class Mock_charset : public CHARSET_INFO {
 public:
  Mock_charset(const CHARSET_INFO &csi) {
    CHARSET_INFO *this_as_cset = this;
    *this_as_cset = csi;

    number = 666;
    m_cset_handler = *(csi.cset);
    m_cset_handler.strntoll = fail_strntoll;
    cset = &m_cset_handler;
  }

 private:
  MY_CHARSET_HANDLER m_cset_handler;
};

TEST_F(ItemTest, ItemFuncConvIntMin) {
  Mock_charset charset(*system_charset_info);
  SCOPED_TRACE("");
  Item *item_conv = new Item_func_conv(POS(), new Item_string("5", 1, &charset),
                                       new Item_int(INT_MIN),   // from_base
                                       new Item_int(INT_MIN));  // to_base
  Parse_context pc(thd(), thd()->lex->current_select());
  EXPECT_FALSE(item_conv->itemize(&pc, &item_conv));
  EXPECT_FALSE(item_conv->fix_fields(thd(), nullptr));
  const String *null_string = nullptr;
  String str;
  EXPECT_EQ(null_string, item_conv->val_str(&str));
}

TEST_F(ItemTest, ItemDecimalTypecast) {
  const char msg[] = "";
  POS pos;
  pos.cpp.start = pos.cpp.end = pos.raw.start = pos.raw.end = msg;
  // Sun Studio needs this null_item,
  // it fails to compile EXPECT_EQ(NULL, create_func_cast());
  const Item *null_item = nullptr;

  Cast_type type;
  type.target = ITEM_CAST_DECIMAL;

  type.length = "123456789012345678901234567890";
  type.dec = nullptr;

  {
    initializer.set_expected_error(ER_TOO_BIG_PRECISION);
    EXPECT_EQ(null_item, create_func_cast(thd(), pos, nullptr, &type));
  }

  {
    char buff[20];
    snprintf(buff, sizeof(buff) - 1, "%d", DECIMAL_MAX_PRECISION + 1);
    type.length = buff;
    type.dec = nullptr;
    initializer.set_expected_error(ER_TOO_BIG_PRECISION);
    EXPECT_EQ(null_item, create_func_cast(thd(), pos, nullptr, &type));
  }

  {
    type.length = nullptr;
    type.dec = "123456789012345678901234567890";
    initializer.set_expected_error(ER_TOO_BIG_SCALE);
    EXPECT_EQ(null_item, create_func_cast(thd(), pos, nullptr, &type));
  }

  {
    char buff[20];
    snprintf(buff, sizeof(buff) - 1, "%d", DECIMAL_MAX_SCALE + 1);
    type.length = buff;
    type.dec = buff;
    initializer.set_expected_error(ER_TOO_BIG_SCALE);
    EXPECT_EQ(null_item, create_func_cast(thd(), pos, nullptr, &type));
  }
}

TEST_F(ItemTest, NormalizedPrint) {
  Item_null *item_null = new Item_null;
  {
    String s;
    item_null->print(thd(), &s, QT_ORDINARY);
    EXPECT_STREQ("NULL", s.c_ptr());
  }
  {
    String s;
    item_null->print(thd(), &s, QT_NORMALIZED_FORMAT);
    EXPECT_STREQ("?", s.c_ptr());
  }
}

TEST_F(ItemTest, CompareEmptyStrings) {
  Item *item1 = new Item_string(nullptr, 0, &my_charset_bin);
  Item *item2 = new Item_string(nullptr, 0, &my_charset_bin);
  Item_result_field *owner = new Item_func_le(item1, item2);
  EXPECT_FALSE(item1->fix_fields(thd(), nullptr));
  EXPECT_FALSE(item2->fix_fields(thd(), nullptr));

  Arg_comparator comparator(&item1, &item2);
  comparator.set_cmp_func(owner, &item1, &item2, false);

  EXPECT_EQ(0, comparator.compare_binary_string());
}

TEST_F(ItemTest, ItemJson) {
  MEM_ROOT *const mem_root = initializer.thd()->mem_root;

  const Item_name_string name(Name_string(STRING_WITH_LEN("json")));

  Json_string jstr("123");
  Item_json *item = new Item_json(
      make_unique_destroy_only<Json_wrapper>(mem_root, &jstr, true), name);

  Json_wrapper wr;
  EXPECT_FALSE(item->val_json(&wr));
  EXPECT_EQ(&jstr, wr.get_dom());

  String string_buffer;
  const String *str = item->val_str(&string_buffer);
  EXPECT_EQ("\"123\"", to_string(*str));
  EXPECT_EQ(item->collation.collation, str->charset());

  EXPECT_EQ(123.0, item->val_real());
  EXPECT_EQ(123, item->val_int());

  my_decimal decimal_buffer;
  const my_decimal *decimal = item->val_decimal(&decimal_buffer);
  double dbl = 0;
  EXPECT_EQ(E_DEC_OK, decimal2double(decimal, &dbl));
  EXPECT_EQ(123.0, dbl);

  Item *clone = item->clone_item();
  EXPECT_NE(item, clone);
  EXPECT_TRUE(item->eq(clone, true));
  EXPECT_FALSE(clone->val_json(&wr));
  EXPECT_NE(&jstr, wr.get_dom());
  EXPECT_EQ(0, wr.compare(Json_wrapper(&jstr, true)));
  EXPECT_EQ(123, clone->val_int());

  const MysqlTime date(2020, 1, 2);
  EXPECT_EQ(MYSQL_TIMESTAMP_DATE, date.time_type);
  item = new Item_json(
      make_unique_destroy_only<Json_wrapper>(
          mem_root, std::unique_ptr<Json_dom>(new (std::nothrow) Json_datetime(
                        date, MYSQL_TYPE_DATE))),
      name);
  MYSQL_TIME time_result;
  EXPECT_FALSE(item->get_date(&time_result, 0));
  EXPECT_EQ(date.time_type, time_result.time_type);
  EXPECT_EQ(date.year, time_result.year);
  EXPECT_EQ(date.month, time_result.month);
  EXPECT_EQ(date.day, time_result.day);

  const MysqlTime time(0, 0, 0, 10, 20, 30, 40, false, MYSQL_TIMESTAMP_TIME);
  item = new Item_json(
      make_unique_destroy_only<Json_wrapper>(
          mem_root, std::unique_ptr<Json_dom>(new (std::nothrow) Json_datetime(
                        time, MYSQL_TYPE_TIME))),
      name);
  EXPECT_FALSE(item->get_time(&time_result));
  EXPECT_EQ(time.time_type, time_result.time_type);
  EXPECT_EQ(time.hour, time_result.hour);
  EXPECT_EQ(time.minute, time_result.minute);
  EXPECT_EQ(time.second, time_result.second);
  EXPECT_EQ(time.second_part, time_result.second_part);
  EXPECT_EQ(time.neg, time_result.neg);
}

}  // namespace item_unittest
