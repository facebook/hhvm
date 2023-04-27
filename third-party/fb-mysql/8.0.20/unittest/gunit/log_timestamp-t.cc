/* Copyright (c) 2018, 2020, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

// First include (the generated) my_config.h, to get correct platform defines,
// then gtest.h (before any other MySQL headers), to avoid min() macros etc ...
#include <gtest/gtest.h>
#include <stdlib.h>
#include "my_config.h"
#include "test_utils.h"

#include <mysql/components/services/log_shared.h>
#include "../sql/log.h"

#define MAKE_TIMESTAMP_UTC 0
#define MAKE_TIMESTAMP_SYSTEM 1

// CET: 32 bytes
//   date (10), 'T', time (8), '.', microseconds (6), timezone offset (6)
#define LEN_MS_CET 32

// UTC: 27 bytes
//   date (10), 'T', time (8), '.', microseconds (6), 'Z'
#define LEN_MS_UTC 27

// we use micro-seconds since the epoch
#define MICRO_FAC ((ulonglong)1000000L)

namespace log_timestamp_unittest {

using my_testing::Server_initializer;

class LogTimestampTest : public ::testing::Test {
 protected:
  virtual void SetUp() { initializer.SetUp(); }
  virtual void TearDown() { initializer.TearDown(); }

  THD *thd() { return initializer.thd(); }

  Server_initializer initializer;
};

/*
  Test basic functionality - throttling, eligibility, printing of summary of
                             Slow_log_throttle.
*/
TEST_F(LogTimestampTest, iso8601) {
  char time_buff[iso8601_size];
#ifdef WIN32
  char tz[] = "TZ=CET-1CES";
#else
  char tz[] = "TZ=CET";
#endif
  int time_buff_len;

  EXPECT_EQ(((iso8601_size)-1), LEN_MS_CET);
  EXPECT_EQ(((LEN_MS_CET)-5), LEN_MS_UTC);

  // set up timezone (central european time)
  putenv(tz);
  EXPECT_STREQ(&(tz[3]), getenv("TZ"));
  tzset();

  /// 1970/01/01 .000001  (1)

  // UTC (winter)
  time_buff_len = make_iso8601_timestamp(time_buff, 1, MAKE_TIMESTAMP_UTC);
  EXPECT_EQ(LEN_MS_UTC, time_buff_len);
  EXPECT_STREQ("1970-01-01T00:00:00.000001Z", time_buff);

  // CET (winter) +1h
  time_buff_len = make_iso8601_timestamp(time_buff, 1, MAKE_TIMESTAMP_SYSTEM);
  EXPECT_EQ(LEN_MS_CET, time_buff_len);
  EXPECT_STREQ("1970-01-01T01:00:00.000001+01:00", time_buff);

  /// 2011-07-07  (1309996800)

  // UTC (summer)
  time_buff_len = make_iso8601_timestamp(time_buff, MICRO_FAC * 1309996800,
                                         MAKE_TIMESTAMP_UTC);
  EXPECT_EQ(LEN_MS_UTC, time_buff_len);
  EXPECT_STREQ("2011-07-07T00:00:00.000000Z", time_buff);

  // CET (summer) +2h
  time_buff_len = make_iso8601_timestamp(time_buff, MICRO_FAC * 1309996800,
                                         MAKE_TIMESTAMP_SYSTEM);
  EXPECT_EQ(LEN_MS_CET, time_buff_len);
  EXPECT_STREQ("2011-07-07T02:00:00.000000+02:00", time_buff);

  /// 1987-06-05 04:03:02.123456

  // UTC
  time_buff_len = make_iso8601_timestamp(
      time_buff, (MICRO_FAC * 549864182) + 123456, MAKE_TIMESTAMP_UTC);
  EXPECT_EQ(LEN_MS_UTC, time_buff_len);
  EXPECT_STREQ("1987-06-05T04:03:02.123456Z", time_buff);
}
// End of Error_log_timestamp test cases.

}  // namespace log_timestamp_unittest
