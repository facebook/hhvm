/* Copyright (c) 2011, 2017, Oracle and/or its affiliates. All rights reserved.

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
#include <sys/types.h>

#include "sql/item.h"
#include "sql/item_timefunc.h"
#include "sql/rpl_handler.h"  // delegates_init()
#include "sql/sql_class.h"
#include "sql/tztime.h"
#include "unittest/gunit/mock_field_datetime.h"
#include "unittest/gunit/mock_field_timestamp.h"
#include "unittest/gunit/mock_field_timestampf.h"
#include "unittest/gunit/test_utils.h"

namespace item_func_now_local_unittest {

using my_testing::Mock_error_handler;
using my_testing::Server_initializer;

const int CURRENT_TIMESTAMP_WHOLE_SECONDS = 123456;
const int CURRENT_TIMESTAMP_FRACTIONAL_SECONDS = 654321;

/*
  Test of the interface of Item_func_now_local.
 */
class ItemFuncNowLocalTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    initializer.SetUp();
    timeval now = {CURRENT_TIMESTAMP_WHOLE_SECONDS,
                   CURRENT_TIMESTAMP_FRACTIONAL_SECONDS};
    get_thd()->set_time(&now);
  }

  virtual void TearDown() { initializer.TearDown(); }

  THD *get_thd() { return initializer.thd(); }

  Server_initializer initializer;
};

/*
  Tests that the THD start time is stored correctly in a Field_timestamp using
  the Item::save_in_field() interface.
*/
TEST_F(ItemFuncNowLocalTest, saveInField) {
  Item_func_now_local *item = new Item_func_now_local(0);
  Mock_field_timestamp f;

  EXPECT_FALSE(item->resolve_type(get_thd()));
  f.make_writable();
  item->save_in_field(&f, true);

  EXPECT_EQ(get_thd()->query_start_timeval_trunc(0).tv_sec,
            f.to_timeval().tv_sec);
  // CURRENT_TIMESTAMP should truncate.
  EXPECT_EQ(0, f.to_timeval().tv_usec);
}

/*
  Tests that Item_func_now_local::store_in() goes through the optimized
  interface Field::store_timestamp() on a Field_timestamp.
*/
TEST_F(ItemFuncNowLocalTest, storeInTimestamp) {
  Mock_field_timestamp f;
  Item_func_now_local::store_in(&f);

  EXPECT_EQ(get_thd()->query_start_timeval_trunc(0).tv_sec,
            f.to_timeval().tv_sec);
  // CURRENT_TIMESTAMP should truncate.
  EXPECT_EQ(0, f.to_timeval().tv_usec);
  EXPECT_TRUE(f.store_timestamp_called);
}

int powers_of_10[DATETIME_MAX_DECIMALS + 1] = {1,     10,     100,    1000,
                                               10000, 100000, 1000000};

/*
  Truncates the number n to a precision of ( DATETIME_MAX_DECIMALS - scale ).
*/
int truncate(int n, int scale) {
  EXPECT_TRUE(scale >= 0);
  EXPECT_TRUE(scale <= DATETIME_MAX_DECIMALS);
  return (n / powers_of_10[DATETIME_MAX_DECIMALS - scale]) *
         powers_of_10[DATETIME_MAX_DECIMALS - scale];
}

/*
  Tests that Item_func_now_local::store_in() goes through the optimized
  interface Field_temporal_with_date_and_time::store_timestamp_internal() on a
  Field_timestampf.

  We also test that the CURRENT_TIMESTAMP value gets truncated, not rounded.
*/
TEST_F(ItemFuncNowLocalTest, storeInTimestampf) {
  for (ulong scale = 0; scale <= DATETIME_MAX_DECIMALS; ++scale) {
    Mock_field_timestampf f(Field::NONE, scale);
    f.make_writable();
    Item_func_now_local::store_in(&f);

    EXPECT_EQ(get_thd()->query_start_timeval_trunc(0).tv_sec,
              f.to_timeval().tv_sec);
    // CURRENT_TIMESTAMP should truncate.
    EXPECT_EQ(truncate(CURRENT_TIMESTAMP_FRACTIONAL_SECONDS, scale),
              f.to_timeval().tv_usec);
    EXPECT_TRUE(f.store_timestamp_internal_called);
  }
}

/*
  Tests that Item_func_now_local::store_in() works correctly even though it does
  not use the optimized interface.
*/
TEST_F(ItemFuncNowLocalTest, storeInDatetime) {
  Mock_field_datetime f;
  MYSQL_TIME now_time;
  THD *thd = get_thd();
  timeval now = {1313677243,
                 1234};  // Thu Aug 18 16:20:43 CEST 2011 and 1234 ms
  thd->set_time(&now);

  Item_func_now_local::store_in(&f);
  thd->variables.time_zone->gmt_sec_to_TIME(&now_time, thd->start_time);
  MYSQL_TIME stored_time;
  f.get_time(&stored_time);

  EXPECT_EQ(now_time.year, stored_time.year);
  EXPECT_EQ(now_time.month, stored_time.month);
  EXPECT_EQ(now_time.day, stored_time.day);
  EXPECT_EQ(now_time.hour, stored_time.hour);
  EXPECT_EQ(now_time.minute, stored_time.minute);
  EXPECT_EQ(now_time.second, stored_time.second);
  // CURRENT_TIMESTAMP truncates.
  EXPECT_EQ(0u, stored_time.second_part);
  EXPECT_EQ(now_time.neg, stored_time.neg);
  EXPECT_EQ(now_time.time_type, stored_time.time_type);
}

}  // namespace item_func_now_local_unittest
