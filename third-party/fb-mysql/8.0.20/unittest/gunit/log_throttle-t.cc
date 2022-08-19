/* Copyright (c) 2013, 2018, Oracle and/or its affiliates. All rights reserved.

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

// First include (the generated) my_config.h, to get correct platform defines,
// then gtest.h (before any other MySQL headers), to avoid min() macros etc ...
#include "my_config.h"

#include <gtest/gtest.h>
#include <sys/types.h>

#include "sql/log.h"
#include "unittest/gunit/test_utils.h"

namespace log_throttle_unittest {

using my_testing::Server_initializer;

int summary_count = 0;
char last_query[10];

static bool slow_logger(THD *, const char *query, size_t,
                        struct System_status_var *) {
  summary_count++;
  strcpy(last_query, query);
  return false;
}

class LogThrottleTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    initializer.SetUp();
    mysql_mutex_init(0, &m_mutex, MY_MUTEX_INIT_FAST);
    summary_count = 0;
  }

  virtual void TearDown() {
    initializer.TearDown();
    mysql_mutex_destroy(&m_mutex);
  }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;
  mysql_mutex_t m_mutex;
};

// Slow_log_throttles test cases starts from here.

/*
  Test basic functionality - throttling, eligibility, printing of summary of
                             Slow_log_throttle.
*/
TEST_F(LogThrottleTest, SlowLogBasic) {
  ulong threshold = 2;
  ulong window = 1000000;
  Slow_log_throttle throttle(&threshold, &m_mutex, window, slow_logger, "%lu");

  // Should not be throttled
  EXPECT_FALSE(throttle.log(thd(), true));
  EXPECT_FALSE(throttle.log(thd(), true));

  // Should not be throttled, not eligible
  EXPECT_FALSE(throttle.log(thd(), false));

  // Flush and check that summary was not printed
  EXPECT_FALSE(throttle.flush(thd()));
  EXPECT_EQ(0, summary_count);

  // Should be throttled
  EXPECT_TRUE(throttle.log(thd(), true));

  // Flush and check that summary was printed
  EXPECT_TRUE(throttle.flush(thd()));
  EXPECT_EQ(1, summary_count);

  // Flush and check that summary was not printed again
  EXPECT_FALSE(throttle.flush(thd()));
  EXPECT_EQ(1, summary_count);

  // Get another summary printed
  EXPECT_FALSE(throttle.log(thd(), true));
  EXPECT_FALSE(throttle.log(thd(), true));
  EXPECT_TRUE(throttle.log(thd(), true));
  EXPECT_TRUE(throttle.flush(thd()));
  EXPECT_EQ(2, summary_count);
}

// Test changes to the threshold level of slow logger
TEST_F(LogThrottleTest, SlowLogThresholdChange) {
  ulong threshold = 2;
  ulong window = 1000000;
  Slow_log_throttle throttle(&threshold, &m_mutex, window, slow_logger, "%lu");

  // Reach threshold
  EXPECT_FALSE(throttle.log(thd(), true));
  EXPECT_FALSE(throttle.log(thd(), true));

  // Flush and check that summary was not printed
  EXPECT_FALSE(throttle.flush(thd()));
  EXPECT_EQ(0, summary_count);

  // Reduce threshold, flush and check that summary was printed
  threshold = 1;
  EXPECT_TRUE(throttle.flush(thd()));
  EXPECT_EQ(1, summary_count);

  // Increase threshold and reach it
  threshold = 3;
  EXPECT_FALSE(throttle.log(thd(), true));
  EXPECT_FALSE(throttle.log(thd(), true));
  EXPECT_FALSE(throttle.log(thd(), true));

  // Flush and check that summary was not printed
  EXPECT_FALSE(throttle.flush(thd()));
  EXPECT_EQ(1, summary_count);
}

// Test number of suppressed messages written by slow logger
TEST_F(LogThrottleTest, SlowLogSuppressCount) {
  ulong threshold = 1;
  ulong window = 1000000;
  Slow_log_throttle throttle(&threshold, &m_mutex, window, slow_logger, "%lu");

  // Suppress 3 events
  EXPECT_FALSE(throttle.log(thd(), true));
  EXPECT_TRUE(throttle.log(thd(), true));
  EXPECT_TRUE(throttle.log(thd(), true));
  EXPECT_TRUE(throttle.log(thd(), true));
  EXPECT_TRUE(throttle.flush(thd()));
  EXPECT_EQ(1, summary_count);
  EXPECT_STREQ("3", last_query);

  // Suppress 2 events
  EXPECT_FALSE(throttle.log(thd(), true));
  EXPECT_FALSE(throttle.log(thd(), false));
  EXPECT_TRUE(throttle.log(thd(), true));
  EXPECT_TRUE(throttle.log(thd(), true));
  EXPECT_FALSE(throttle.log(thd(), false));
  EXPECT_TRUE(throttle.flush(thd()));
  EXPECT_EQ(2, summary_count);
  EXPECT_STREQ("2", last_query);
}
// End of Slow_log_throttles test cases.

}  // namespace log_throttle_unittest
