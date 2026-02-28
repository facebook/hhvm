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
#include <limits>
#include <string>
#include "m_string.h"
#include "my_inttypes.h"
#include "my_systime.h"
#include "my_time.h"
#include "unittest/gunit/benchmark.h"
#include "unittest/gunit/mysys_util.h"

// Unit tests for mysys time functions

namespace mysys_my_time {
longlong DRV_my_packed_time_get_int_part(longlong i);
longlong DRV_my_packed_time_make(longlong i, longlong f);
longlong DRV_my_packed_time_make_int(longlong i);

// Compare MYSQL_TIME structs for absolute equality. Which is
// different from the semantics implemented by my_time_compare()
// which considers different time_types with the same value to be
// equal.
inline bool operator==(const MYSQL_TIME &a, const MYSQL_TIME &b) {
  return a.year == b.year && a.month == b.month && a.day == b.day &&
         a.hour == b.hour && a.minute == b.minute && a.second == b.second &&
         a.neg == b.neg && a.time_type == b.time_type;
}

// Envelope struct containing results from creating a datetime from
// a string.
struct DatetimeResult {
  MysqlTime t;
  bool stdt{false};
  MYSQL_TIME_STATUS s;
  bool ct{false};
  int was_cut{false};
};

// Utility function for creating datetime MYSQL_TIMEs from a string
DatetimeResult make_datetime_from_string(const std::string &s,
                                         my_time_flags_t f) {
  DatetimeResult r;
  r.stdt = str_to_datetime(s.c_str(), s.length(), &r.t, f, &r.s);
  r.ct = check_date(r.t, false, f, &r.was_cut);
  return r;
}

/// Utility function for creating a MYSQL_TIME from a string.
DatetimeResult make_time_from_string(const std::string &s) {
  DatetimeResult r;
  r.t.time_zone_displacement = -1;
  r.stdt = str_to_time(s.c_str(), s.length(), &r.t, &r.s);
  r.ct = check_date(r.t, false, 0, &r.was_cut);
  return r;
}

// Helper macro which checks all values in a DatetimeResult
#define EXPECT_VALID_DATETIME(time_res) \
  EXPECT_EQ(false, (time_res).stdt);    \
  EXPECT_EQ(0, (time_res).s.warnings);  \
  EXPECT_EQ(false, (time_res).t.neg);   \
  EXPECT_EQ(false, (time_res).ct);      \
  EXPECT_EQ(0, (time_res).was_cut);

// Test various ways of specifying a zero date
TEST(MysysMyTime, StrToDatetime) {
  DatetimeResult tr = make_datetime_from_string("00000", 0);
  EXPECT_VALID_DATETIME(tr);
  tr = make_datetime_from_string("10101120000AM", 0);
  // EXPECT_VALID_DATETIME(tr);
  EXPECT_EQ(true, tr.stdt);
  tr = make_datetime_from_string("000000", 0);
  EXPECT_VALID_DATETIME(tr);
  tr = make_datetime_from_string("0000000", 0);
  EXPECT_VALID_DATETIME(tr);
  tr = make_datetime_from_string("00000000", 0);
  EXPECT_VALID_DATETIME(tr);
  tr = make_datetime_from_string("000000000", 0);
  EXPECT_VALID_DATETIME(tr);
  tr = make_datetime_from_string("0000000000", 0);
  EXPECT_VALID_DATETIME(tr);
  tr = make_datetime_from_string("00000000000", 0);
  EXPECT_VALID_DATETIME(tr);
  tr = make_datetime_from_string("000000000000", 0);
  EXPECT_VALID_DATETIME(tr);
  tr = make_datetime_from_string("0000000000000", 0);
  EXPECT_EQ(false, tr.stdt);
  EXPECT_EQ(MYSQL_TIME_WARN_TRUNCATED, tr.s.warnings);
  EXPECT_EQ(false, tr.ct);
  EXPECT_EQ(0, tr.was_cut);
  tr = make_datetime_from_string("00000000000000", 0);
  EXPECT_VALID_DATETIME(tr);
  tr = make_datetime_from_string("000000000000000", 0);
  EXPECT_EQ(false, tr.stdt);
  EXPECT_EQ(MYSQL_TIME_WARN_TRUNCATED, tr.s.warnings);
  EXPECT_EQ(false, tr.ct);
  EXPECT_EQ(0, tr.was_cut);
  tr = make_datetime_from_string("00000000000000.0", 0);
  EXPECT_VALID_DATETIME(tr);
  tr = make_datetime_from_string("00000000000000.00", 0);
  EXPECT_VALID_DATETIME(tr);
  tr = make_datetime_from_string("00000000000000.000", 0);
  EXPECT_VALID_DATETIME(tr);
  tr = make_datetime_from_string("00000000000000.0000", 0);
  EXPECT_VALID_DATETIME(tr);
  tr = make_datetime_from_string("00000000000000.00000", 0);
  EXPECT_VALID_DATETIME(tr);
  tr = make_datetime_from_string("00000000000000.000000", 0);
  EXPECT_VALID_DATETIME(tr);
  tr = make_datetime_from_string("00000000000000.0000009", 0);
  EXPECT_VALID_DATETIME(tr);
  EXPECT_EQ(0, tr.t.second_part);
  tr = make_datetime_from_string("00000000000000.00000099", 0);
  EXPECT_VALID_DATETIME(tr);
  EXPECT_EQ(0, tr.t.second_part);
  tr = make_datetime_from_string("00000000000000.000000999", 0);
  EXPECT_VALID_DATETIME(tr);
  EXPECT_EQ(0, tr.t.second_part);
  tr = make_datetime_from_string("00000000000000.0000009999", 0);
  EXPECT_VALID_DATETIME(tr);
  EXPECT_EQ(0, tr.t.second_part);
  tr = make_datetime_from_string("00000000000000.00000099999", 0);
  EXPECT_VALID_DATETIME(tr);
  EXPECT_EQ(0, tr.t.second_part);
  tr = make_datetime_from_string("00000000000000.000000999999", 0);
  EXPECT_VALID_DATETIME(tr);
  EXPECT_EQ(0, tr.t.second_part);
  tr = make_datetime_from_string("00000000000000.0000009999999", 0);
  EXPECT_VALID_DATETIME(tr);
  EXPECT_EQ(0, tr.t.second_part);

  tr = make_datetime_from_string("00000000000000AM", 0);
  EXPECT_TRUE(tr.stdt);
  EXPECT_EQ(MYSQL_TIME_WARN_TRUNCATED, tr.s.warnings);
  EXPECT_EQ(false, tr.t.neg);
  EXPECT_EQ(false, tr.ct);
  EXPECT_EQ(0, tr.was_cut);
  EXPECT_EQ(0, tr.t.second_part);

  tr = make_datetime_from_string("00000000000000 AM", 0);
  EXPECT_TRUE(tr.stdt);
  EXPECT_EQ(MYSQL_TIME_WARN_TRUNCATED, tr.s.warnings);
  EXPECT_EQ(false, tr.t.neg);
  EXPECT_EQ(false, tr.ct);
  EXPECT_EQ(0, tr.was_cut);
  EXPECT_EQ(0, tr.t.second_part);

  tr = make_datetime_from_string("00000000000000.000000AM", 0);
  EXPECT_FALSE(tr.stdt);
  EXPECT_EQ(MYSQL_TIME_WARN_TRUNCATED, tr.s.warnings);
  EXPECT_EQ(false, tr.t.neg);
  EXPECT_EQ(false, tr.ct);
  EXPECT_EQ(0, tr.was_cut);
  EXPECT_EQ(0, tr.t.second_part);

  tr = make_datetime_from_string("00000000000000.000000 PM", 0);
  EXPECT_FALSE(tr.stdt);
  EXPECT_EQ(MYSQL_TIME_WARN_TRUNCATED, tr.s.warnings);
  EXPECT_EQ(false, tr.t.neg);
  EXPECT_EQ(false, tr.ct);
  EXPECT_EQ(0, tr.was_cut);
  EXPECT_EQ(0, tr.t.second_part);

  EXPECT_EQ(
      0,
      make_datetime_from_string("20071108181000", 0).t.time_zone_displacement);
}

TEST(MysysMyTime, StrToTime) {
  EXPECT_EQ(0, make_time_from_string("1").t.time_zone_displacement);
}

// Different ways to specify a zero date using -
TEST(MysysMyTime, StrToDatetimeNullDash) {
  DatetimeResult tr = make_datetime_from_string("00-00-00", 0);
  EXPECT_VALID_DATETIME(tr);

  tr = make_datetime_from_string("0000-00-00", 0);
  EXPECT_VALID_DATETIME(tr);

  tr = make_datetime_from_string("00-00-00 00.00.00", 0);
  EXPECT_VALID_DATETIME(tr);

  tr = make_datetime_from_string("00-00-00 00.00.00.000000", 0);
  EXPECT_VALID_DATETIME(tr);

  tr = make_datetime_from_string("00-00-00 00.00.00.0000009999", 0);
  EXPECT_VALID_DATETIME(tr);
  EXPECT_EQ(0, tr.t.second_part);

  tr = make_datetime_from_string("0000-00-00 00.00.00.000000", 0);
  EXPECT_VALID_DATETIME(tr);
  EXPECT_EQ(0, tr.t.second_part);
}

// Invalid - strings
TEST(MysysMyTime, StrToDatetimeNullDashInvalid) {
  DatetimeResult tr = make_datetime_from_string("-00-00", TIME_FUZZY_DATE);
  EXPECT_EQ(true, tr.stdt);

  tr = make_datetime_from_string("0000-00-", TIME_FUZZY_DATE);
  EXPECT_EQ(true, tr.stdt);

  tr = make_datetime_from_string("0000--00", TIME_FUZZY_DATE);
  EXPECT_EQ(true, tr.stdt);

  tr = make_datetime_from_string("0000-00-00 .00.00", 0);
  EXPECT_VALID_DATETIME(tr);

  tr = make_datetime_from_string("00-00-00 00..00.000000", 0);
  EXPECT_VALID_DATETIME(tr);

  tr = make_datetime_from_string("00-00-00 00.00..0000009999", 0);
  EXPECT_EQ(true, tr.stdt);

  tr = make_datetime_from_string("0000-00-00 00...000000", 0);
  EXPECT_VALID_DATETIME(tr);
}

// Create datetime from (some) iso8601 strings
TEST(MysysMyTime, StrToDatetimeNullIso8601) {
  DatetimeResult tr = make_datetime_from_string("00000000T000000", 0);
  EXPECT_VALID_DATETIME(tr);

  tr = make_datetime_from_string("00000000T000000.000000", 0);
  EXPECT_VALID_DATETIME(tr);

  tr = make_datetime_from_string("00000000T000000.999999", 0);
  EXPECT_EQ(true, tr.stdt);

  tr = make_datetime_from_string("00000000T123030.999999", 0);
  EXPECT_EQ(true, tr.stdt);

  tr = make_datetime_from_string("20000228T123030.999999", 0);
  EXPECT_VALID_DATETIME(tr);
  EXPECT_EQ(999999UL, tr.t.second_part);
}

// Check TIME_FUZZY_DATE and TIME_FUZZY_DATE|TIME_NO_ZERO_IN_DATE
TEST(MysysMyTime, StrToDatetimeFuzzyDate) {
  DatetimeResult tr =
      make_datetime_from_string("19800031T000000", TIME_FUZZY_DATE);
  EXPECT_VALID_DATETIME(tr);

  tr = make_datetime_from_string("19901100T000000", TIME_FUZZY_DATE);
  EXPECT_VALID_DATETIME(tr);

  tr = make_datetime_from_string("00001130T000000", TIME_FUZZY_DATE);
  EXPECT_VALID_DATETIME(tr);

  tr = make_datetime_from_string("19800031T000000",
                                 TIME_FUZZY_DATE | TIME_NO_ZERO_IN_DATE);
  EXPECT_EQ(true, tr.stdt);

  tr = make_datetime_from_string("19901100T000000",
                                 TIME_FUZZY_DATE | TIME_NO_ZERO_IN_DATE);
  EXPECT_EQ(true, tr.stdt);

  tr = make_datetime_from_string("00001130T000000",
                                 TIME_FUZZY_DATE | TIME_NO_ZERO_IN_DATE);
  EXPECT_VALID_DATETIME(tr);

  tr = make_datetime_from_string("19991130T000000", TIME_NO_ZERO_IN_DATE);
  EXPECT_VALID_DATETIME(tr);
}

// Check MYSQL_TIME -> ulonglong -> longlong -> MYSQL_TIME conversion
TEST(MysysMyTime, NumberToDatetime) {
  DatetimeResult tr = make_datetime_from_string("19991130T120000", 0);
  EXPECT_VALID_DATETIME(tr);

  ulonglong ullt = TIME_to_ulonglong_datetime(tr.t);
  MYSQL_TIME t;
  int was_cut = 0;
  EXPECT_EQ(19991130120000ULL, number_to_datetime(ullt, &t, 0, &was_cut));
  EXPECT_EQ(0, was_cut);
  EXPECT_EQ(true, (tr.t == t));
}

// Check fast-path rounding to ulonglong
TEST(MysysMyTime, TimeToUlonglongDatetimeRoundFast) {
  DatetimeResult tr = make_datetime_from_string("19991130T120000.670000", 0);
  EXPECT_VALID_DATETIME(tr);

  int warnings = 0;
  EXPECT_EQ(19991130120001, TIME_to_ulonglong_datetime_round(tr.t, &warnings));
  EXPECT_EQ(0, warnings);
}

// Check slow-path rounding to ulonglong
TEST(MysysMyTime, TimeToUlonglongDatetimeRoundSlow) {
  DatetimeResult tr = make_datetime_from_string("19991130T120059.670000", 0);
  EXPECT_VALID_DATETIME(tr);

  int warnings = 0;
  EXPECT_EQ(19991130120100, TIME_to_ulonglong_datetime_round(tr.t, &warnings));
  EXPECT_EQ(0, warnings);
}

// Check that the fractional part is correct for positive and
// negative numbers
TEST(MysysMyTime, MyPackedTimeGetFracPart) {
  EXPECT_EQ(0xbadcaf, my_packed_time_get_frac_part(0x123badcaf));

  longlong ll = std::numeric_limits<longlong>::min();
  ll <<= 24;
  ll |= 0xbadcaf;
  EXPECT_EQ(0xbadcaf, my_packed_time_get_frac_part(ll));
}

// Conversion MYSQL_TIME -> longlong packed -> MYSQL_TIME
TEST(MysysMyTime, LonglongDatetimePacked) {
  DatetimeResult tr = make_datetime_from_string("19991130T120059.670000", 0);
  EXPECT_VALID_DATETIME(tr);

  longlong ll = TIME_to_longlong_datetime_packed(tr.t);
  EXPECT_EQ(1829513407452821808LL, ll);

  MYSQL_TIME t;
  TIME_from_longlong_datetime_packed(&t, ll);
  EXPECT_EQ(true, (tr.t == t));
}

// Conversion MYSQL_TIME -> longlong packed -> binary -> longlong
// packed -> MYSQL_TIME
TEST(MysysMyTime, DatetimePackedBinary) {
  DatetimeResult tr = make_datetime_from_string("20000228T120059.670000", 0);
  EXPECT_VALID_DATETIME(tr);

  longlong ll = TIME_to_longlong_datetime_packed(tr.t);
  EXPECT_EQ(1829790484383021360LL, ll);

  uchar buf[256];
  my_datetime_packed_to_binary(ll, buf, 6);

  longlong llb = my_datetime_packed_from_binary(buf, 6);
  EXPECT_EQ(ll, llb);
}

// Add microsecond interval and verify that datetime wraps correctly
// to new date
TEST(MysysMyTime, DatetimeAddInterval) {
  DatetimeResult tr = make_datetime_from_string("20200229T235959.670000", 0);
  EXPECT_VALID_DATETIME(tr);
  EXPECT_EQ(670000LL, tr.t.second_part);
  Interval i = {0, 0, 0, 0, 0, 0, 330000ull, false};
  int warnings = 0;
  EXPECT_EQ(false,
            date_add_interval(&tr.t, INTERVAL_MICROSECOND, i, &warnings));
  MysqlTime ex(2020, 3, 1, 0, 0, 0, 0, false, MYSQL_TIMESTAMP_DATETIME);
  EXPECT_EQ(true, (tr.t == ex));
}

// Test packed access functions on positive time values
TEST(MysysMyTime, MyPackedTimeGetFracPart2) {
  const MysqlTime mt(2020U, 2U, 29U, 23U, 59U, 59U, 670000);
  longlong pt = TIME_to_longlong_datetime_packed(mt);
  EXPECT_EQ(670000LL, my_packed_time_get_frac_part(pt));
}

TEST(MysysMyTime, MyPackedTimeGetIntPart) {
  const MysqlTime mt(2020U, 2U, 29U, 23U, 59U, 59U, 670000UL);
  longlong pt = TIME_to_longlong_datetime_packed(mt);

  EXPECT_EQ(110154710779LL, DRV_my_packed_time_get_int_part(pt));
}

TEST(MysysMyTime, MyPackedTimeMake) {
  EXPECT_EQ(17447216LL, DRV_my_packed_time_make(1LL, 670000LL));
}

TEST(MysysMyTime, MyPackedTimeMakeInt) {
  const MysqlTime mt(2020U, 2U, 29U, 23U, 59U, 59U, 670000);
  longlong pt = TIME_to_longlong_datetime_packed(mt);

  EXPECT_EQ(9149918308668014592LL, DRV_my_packed_time_make_int(pt));
}

// Test packed access functions on negative time values
TEST(MysysMyTime, MyPackedTimeGetFracPartNeg) {
  const MysqlTime mt(2020U, 2U, 29U, 23U, 59U, 59U, 670000, true,
                     MYSQL_TIMESTAMP_DATETIME);
  longlong pt = TIME_to_longlong_datetime_packed(mt);
  EXPECT_EQ(-670000LL, my_packed_time_get_frac_part(pt));
}

TEST(MysysMyTime, MyPackedTimeGetIntPartNeg) {
  const MysqlTime mt(2020U, 2U, 29U, 23U, 59U, 59U, 670000, true,
                     MYSQL_TIMESTAMP_DATETIME);
  longlong pt = TIME_to_longlong_datetime_packed(mt);

  EXPECT_EQ(-110154710780LL, DRV_my_packed_time_get_int_part(pt));
}

TEST(MysysMyTime, MyPackedTimeMakeNeg) {
  EXPECT_EQ(-16107216LL, DRV_my_packed_time_make(-1LL, 670000LL));
}

TEST(MysysMyTime, MyPackedTimeMakeIntNeg) {
  const MysqlTime mt(2020U, 2U, 29U, 23U, 59U, 59U, 670000, true,
                     MYSQL_TIMESTAMP_DATETIME);
  longlong pt = TIME_to_longlong_datetime_packed(mt);

  EXPECT_EQ(-9149918308668014592LL, DRV_my_packed_time_make_int(pt));
}

/**
  Convenience function for an expected successful call to
  time_zone_displacement_to_seconds().
*/
int TzDisplacementToSeconds(const char *s) {
  int secs;
  EXPECT_FALSE(time_zone_displacement_to_seconds(s, strlen(s), &secs))
      << '"' << s << '"'
      << " is supposed to be a valid time zone displacement.";
  return secs;
}

/**
  Convenience function checking the return value of
  time_zone_displacement_to_seconds().
*/
bool CheckTimeZoneDisplacement(std::string s) {
  int secs;
  return time_zone_displacement_to_seconds(s.c_str(), s.length(), &secs);
}

TEST(MysysMyTime, TimeZoneDisplacementToSeconds) {
  EXPECT_EQ(0, TzDisplacementToSeconds("+00:00"));
  EXPECT_EQ(60, TzDisplacementToSeconds("+00:01"));
  EXPECT_EQ(36000, TzDisplacementToSeconds("+10:00"));
  EXPECT_EQ(45240, TzDisplacementToSeconds("+12:34"));
  EXPECT_EQ(50400, TzDisplacementToSeconds("+14:00"));
  EXPECT_EQ(-50400, TzDisplacementToSeconds("-14:00"));

  int int_max = std::numeric_limits<int>::max();
  std::cout << "int max " << int_max << std::endl;

  // Various syntactical errors.
  EXPECT_TRUE(CheckTimeZoneDisplacement("+:00"));
  EXPECT_TRUE(CheckTimeZoneDisplacement("00:00"));
  EXPECT_TRUE(CheckTimeZoneDisplacement("+00:"));
  EXPECT_TRUE(CheckTimeZoneDisplacement("+00::00"));
  EXPECT_TRUE(CheckTimeZoneDisplacement("+00::"));
  EXPECT_TRUE(CheckTimeZoneDisplacement("+0:0"));
  EXPECT_TRUE(CheckTimeZoneDisplacement("+0:1"));
  EXPECT_TRUE(CheckTimeZoneDisplacement("+1:1"));

  // Various invalid hours.
  EXPECT_TRUE(CheckTimeZoneDisplacement("+15"));
  EXPECT_TRUE(CheckTimeZoneDisplacement("-15"));
  EXPECT_TRUE(CheckTimeZoneDisplacement(("+" + std::to_string(int_max + 1))))
      << "Int wrap-around in hours not caught.";

  // Various invalid minutes.
  EXPECT_TRUE(CheckTimeZoneDisplacement("+14:01"));
  EXPECT_TRUE(CheckTimeZoneDisplacement("-14:01"));
  EXPECT_TRUE(CheckTimeZoneDisplacement("+0:60"));
  EXPECT_TRUE(CheckTimeZoneDisplacement("+00:60"));
  EXPECT_TRUE(CheckTimeZoneDisplacement("+00:61"));
  EXPECT_TRUE(CheckTimeZoneDisplacement(("+0:" + std::to_string(int_max + 1))))
      << "Int wrap-around in minutes not caught.";

  EXPECT_TRUE(CheckTimeZoneDisplacement("-00:00"));
}

std::string MyDatetimeToStr(const MysqlTime &mt) {
  std::string res;
  res.resize(MAX_DATE_STRING_REP_LENGTH);
  res.resize(static_cast<size_t>(my_datetime_to_str(mt, &res.at(0), 0)));
  return res;
}

#define EXPECT_STD_STREQ(X, Y) EXPECT_STREQ((X), (Y).c_str())

TEST(MysysMyTime, MyDatetimeToStr) {
  EXPECT_STD_STREQ("2019-06-24 15:17:33-12:34",
                   MyDatetimeToStr({2019, 06, 24, 15, 17, 33, 123456, -45240}));
}

/*
  Microbenchmark which tests the performance of clock-reading functions.
*/
static void BM_my_getsystime(size_t num_iterations) {
  StopBenchmarkTiming();

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    my_getsystime();
  }

  StopBenchmarkTiming();
}
BENCHMARK(BM_my_getsystime)

static void BM_my_time(size_t num_iterations) {
  StopBenchmarkTiming();

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    my_time(0);
  }

  StopBenchmarkTiming();
}
BENCHMARK(BM_my_time)

static void BM_my_micro_time(size_t num_iterations) {
  StopBenchmarkTiming();

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    my_micro_time();
  }

  StopBenchmarkTiming();
}
BENCHMARK(BM_my_micro_time)

static void BM_my_time_to_str(size_t num_iterations) {
  StopBenchmarkTiming();

  const MysqlTime date(0, 0, 0, 123, 59, 59, 670000, false,
                       MYSQL_TIMESTAMP_TIME);
  char buffer[MAX_DATE_STRING_REP_LENGTH];

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    my_time_to_str(date, buffer, 6);
  }

  StopBenchmarkTiming();
}
BENCHMARK(BM_my_time_to_str)

static void BM_my_date_to_str(size_t num_iterations) {
  StopBenchmarkTiming();

  const MysqlTime date(2020, 2, 29, 0, 0, 0, 0, false, MYSQL_TIMESTAMP_DATE);
  char buffer[MAX_DATE_STRING_REP_LENGTH];

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    my_date_to_str(date, buffer);
  }

  StopBenchmarkTiming();
}
BENCHMARK(BM_my_date_to_str)

static void BM_my_datetime_to_str(size_t num_iterations) {
  StopBenchmarkTiming();

  const MysqlTime date(2020, 2, 29, 23, 59, 59, 670000, false,
                       MYSQL_TIMESTAMP_DATETIME);
  char buffer[MAX_DATE_STRING_REP_LENGTH];

  StartBenchmarkTiming();

  for (size_t i = 0; i < num_iterations; ++i) {
    my_datetime_to_str(date, buffer, 6);
  }

  StopBenchmarkTiming();
}
BENCHMARK(BM_my_datetime_to_str)

}  // namespace mysys_my_time
