/* Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.

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
#include <string.h>
#include <sstream>
#include <string>

#include "decimal.h"
#include "m_ctype.h"
#include "my_time.h"
#include "mysql_time.h"
#include "sql/item.h"
#include "sql/item_timefunc.h"
#include "sql/my_decimal.h"
#include "sql/sql_class.h"
#include "sql/sql_lex.h"
#include "sql/sql_time.h"
#include "unittest/gunit/test_utils.h"

namespace item_timefunc_unittest {

using my_testing::Server_initializer;

class ItemTimeFuncTest : public ::testing::Test {
 protected:
  virtual void SetUp() { initializer.SetUp(); }

  virtual void TearDown() { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;
};

TEST_F(ItemTimeFuncTest, dateAddInterval) {
  Item_int *arg0 = new Item_int(20130122145221LL);  // 2013-01-22 14:52:21
  Item_decimal *arg1 = new Item_decimal(0.1234567);
  Item *item = new Item_date_add_interval(POS(), arg0, arg1,
                                          INTERVAL_SECOND_MICROSECOND, false);
  Parse_context pc(thd(), thd()->lex->current_select());
  EXPECT_FALSE(item->itemize(&pc, &item));
  EXPECT_FALSE(item->fix_fields(thd(), nullptr));

  // The below result is not correct, see Bug#16198372
  EXPECT_DOUBLE_EQ(20130122145222.234567, item->val_real());
}

struct test_data {
  const char *secs;
  unsigned int hour;
  unsigned int minute;
  unsigned int second;
  unsigned long second_part;
};

::std::ostream &operator<<(::std::ostream &os, const struct test_data &data) {
  return os << data.secs;
}

class ItemTimeFuncTestP : public ::testing::TestWithParam<test_data> {
 protected:
  virtual void SetUp() {
    initializer.SetUp();
    m_t = GetParam();
  }

  virtual void TearDown() { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;
  test_data m_t;
};

const test_data test_values[] = {{"0.1234564", 0, 0, 0, 123456},
                                 {"0.1234567", 0, 0, 0, 123457},
                                 {"0.1234", 0, 0, 0, 123400},
                                 {"12.1234567", 0, 0, 12, 123457},
                                 {"123", 0, 2, 3, 0},
                                 {"2378.3422349", 0, 39, 38, 342235},
                                 {"3020398.999999999", 838, 59, 59, 0},
                                 {"3020399", 838, 59, 59, 0},
                                 {"99999999.99999999", 838, 59, 59, 0}};

INSTANTIATE_TEST_CASE_P(a, ItemTimeFuncTestP, ::testing::ValuesIn(test_values));

/**
  Test member function of @c Item_time_func

  @param item     item of a sub-class of @c Item_time_func
  @param ltime    time structure that contains the expected result
  @param decimals number of significant decimals in the expected result
*/
void testItemTimeFunctions(Item_time_func *item, MYSQL_TIME *ltime,
                           int decimals) {
  long long int mysql_time =
      10000 * ltime->hour + 100 * ltime->minute + ltime->second;
  EXPECT_EQ(mysql_time, item->val_int());

  long long int packed = TIME_to_longlong_packed(*ltime);
  EXPECT_EQ(packed, item->val_time_temporal());

  double d = mysql_time + ltime->second_part / 1000000.0;
  EXPECT_DOUBLE_EQ(d, item->val_real());

  my_decimal decval1, decval2;
  my_decimal *dec = item->val_decimal(&decval1);
  double2decimal(d, &decval2);
  EXPECT_EQ(0, my_decimal_cmp(dec, &decval2));

  char s[20];
  sprintf(s, "%02d:%02d:%02d", ltime->hour, ltime->minute, ltime->second);
  if (ltime->second_part > 0) {  // Avoid trailing zeroes
    int decs = ltime->second_part;
    while (decs % 10 == 0) decs /= 10;
    sprintf(s + strlen(s), ".%d", decs);
  } else if (decimals > 0)
    // There were decimals, but they have disappeared due to overflow
    sprintf(s + strlen(s), ".000000");
  String timeStr(20);
  EXPECT_STREQ(s, item->val_str(&timeStr)->c_ptr());

  MYSQL_TIME ldate;
  //> Second argument of Item_func_time::get_date is not used for anything
  item->get_date(&ldate, 0);
  // Todo: Should check that year, month, and day is relative to current date
  EXPECT_EQ(ltime->hour % 24, ldate.hour);
  EXPECT_EQ(ltime->minute, ldate.minute);
  EXPECT_EQ(ltime->second, ldate.second);
  EXPECT_EQ(ltime->second_part, ldate.second_part);

  // Todo: Item_time_func::save_in_field is not tested
}

TEST_P(ItemTimeFuncTestP, secToTime) {
  Item_decimal *sec = new Item_decimal(POS(), m_t.secs, strlen(m_t.secs),
                                       &my_charset_latin1_bin);
  Item_func_sec_to_time *time = new Item_func_sec_to_time(POS(), sec);

  Parse_context pc(thd(), thd()->lex->current_select());
  Item *item;
  EXPECT_FALSE(time->itemize(&pc, &item));
  EXPECT_EQ(time, item);
  EXPECT_FALSE(time->fix_fields(thd(), nullptr));

  MYSQL_TIME ltime;
  time->get_time(&ltime);
  EXPECT_EQ(0U, ltime.year);
  EXPECT_EQ(0U, ltime.month);
  EXPECT_EQ(0U, ltime.day);
  EXPECT_EQ(m_t.hour, ltime.hour);
  EXPECT_EQ(m_t.minute, ltime.minute);
  EXPECT_EQ(m_t.second, ltime.second);
  EXPECT_EQ(m_t.second_part, ltime.second_part);

  testItemTimeFunctions(time, &ltime, sec->decimals);
}

// Test for MODE_TIME_TRUNCATE_FRACTIONAL.
class ItemTimeFuncTruncFracTestP : public ::testing::TestWithParam<test_data> {
 protected:
  virtual void SetUp() {
    initializer.SetUp();
    m_t = GetParam();
    save_mode = thd()->variables.sql_mode;
    thd()->variables.sql_mode |= MODE_TIME_TRUNCATE_FRACTIONAL;
  }

  virtual void TearDown() {
    thd()->variables.sql_mode = save_mode;
    initializer.TearDown();
  }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;
  test_data m_t;
  sql_mode_t save_mode;
};

const test_data test_values_trunc_frac[] = {
    {"0.1234564", 0, 0, 0, 123456},
    {"0.1234567", 0, 0, 0, 123456},
    {"0.1234", 0, 0, 0, 123400},
    {"12.1234567", 0, 0, 12, 123456},
    {"123", 0, 2, 3, 0},
    {"2378.3422349", 0, 39, 38, 342234},
    {"3020398.999999999", 838, 59, 58, 999999},
    {"3020399", 838, 59, 59, 0},
    {"99999999.99999999", 838, 59, 59, 0}};

INSTANTIATE_TEST_CASE_P(a, ItemTimeFuncTruncFracTestP,
                        ::testing::ValuesIn(test_values_trunc_frac));

TEST_P(ItemTimeFuncTruncFracTestP, secToTime) {
  Item_decimal *sec = new Item_decimal(POS(), m_t.secs, strlen(m_t.secs),
                                       &my_charset_latin1_bin);
  Item_func_sec_to_time *time = new Item_func_sec_to_time(POS(), sec);

  Parse_context pc(thd(), thd()->lex->current_select());
  Item *item;
  EXPECT_FALSE(time->itemize(&pc, &item));
  EXPECT_EQ(time, item);
  EXPECT_FALSE(time->fix_fields(thd(), nullptr));

  MYSQL_TIME ltime;
  time->get_time(&ltime);
  EXPECT_EQ(0U, ltime.year);
  EXPECT_EQ(0U, ltime.month);
  EXPECT_EQ(0U, ltime.day);
  EXPECT_EQ(m_t.hour, ltime.hour);
  EXPECT_EQ(m_t.minute, ltime.minute);
  EXPECT_EQ(m_t.second, ltime.second);
  EXPECT_EQ(m_t.second_part, ltime.second_part);

  testItemTimeFunctions(time, &ltime, sec->decimals);
}
}  // namespace item_timefunc_unittest
