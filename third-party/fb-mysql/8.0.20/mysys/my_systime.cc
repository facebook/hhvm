/* Copyright (c) 2004, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   Without limiting anything contained in the foregoing, this file,
   which is part of C Driver for MySQL (Connector/C), is also subject to the
   Universal FOSS Exception, version 1.0, a copy of which can be found at
   http://oss.oracle.com/licenses/universal-foss-exception.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file mysys/my_systime.cc Functions for manipulating timespec
  structs and get_date for converting a time_t to string using various
  date formats.
*/

#include "my_systime.h"
#include "my_config.h"

#include <assert.h>
#include <algorithm>  // std::min
#include <chrono>
#include <cstdio>  // std::sprintf()
#include <limits>  // std::numeric_limits

// Note that timespec is in time.h in C99, but std::timespec will not
// be in ctime until C++17
#include <time.h>  // time_t, timespec

/**
   Set the value of a timespec object to the current time plus a
   number of nanosconds.

   @note the sec value is capped at std::chrono::nanoseconds::max()

   @param[out] abstime time value being modified
   @param nsec number of nanoseconds to add to current time
 */
void set_timespec_nsec(struct timespec *abstime, Timeout_type nsec) {
  assert(nsec != std::numeric_limits<Timeout_type>::max());
  if (nsec == TIMEOUT_INF) {
    *abstime = TIMESPEC_POSINF;
    return;
  }
  unsigned long long int now = my_getsystime() + (nsec / 100);
  unsigned long long int tv_sec = now / 10000000ULL;
#if SIZEOF_TIME_T < SIZEOF_LONG_LONG
  /* Ensure that the number of seconds don't overflow. */
  tv_sec = std::min(tv_sec, static_cast<unsigned long long int>(
                                std::numeric_limits<time_t>::max()));
#endif
  abstime->tv_sec = static_cast<time_t>(tv_sec);
  abstime->tv_nsec = (now % 10000000ULL) * 100 + (nsec % 100);
}

/**
   Set the value of a timespec object to the current time plus a
   number of seconds using seconds.

   @note the sec value is capped at std::chrono::seconds::max()

   @param[out] abstime time value being modified
   @param sec number of seconds to add to current time
 */
void set_timespec(struct timespec *abstime, Timeout_type sec) {
  assert(sec != std::numeric_limits<Timeout_type>::max());
  if (sec == TIMEOUT_INF) {
    *abstime = TIMESPEC_POSINF;
    return;
  }

  set_timespec_nsec(abstime, sec * 1000000000ULL);
}

/**
    Store textual representation of date in a character array.

    @param[out] to    character array where date will be written
    @param      flag  format of date:
          If flag & GETDATE_TIME	Return date and time
          If flag & GETDATE_SHORT_DATE	Return short date format YYMMDD
          If flag & GETDATE_HHMMSSTIME	Return time in HHMMDD format.
          If flag & GETDATE_GMT		Date/time in GMT
          If flag & GETDATE_FIXEDLENGTH	Return fixed length date/time
    @param      date  time_t value for conversion.

    @note If flag & GETDATE_T_DELIMITER Append 'T' between date and time.
    If flag & GETDATE_SHORT_DATE_FULL_YEAR Return compact date format YYYYMMDD
*/
void get_date(char *to, int flag, time_t date) {
  struct tm *start_time;
  time_t skr;
  struct tm tm_tmp;

  skr = date ? (time_t)date : my_time(0);
  if (flag & GETDATE_GMT)
    gmtime_r(&skr, &tm_tmp);
  else
    localtime_r(&skr, &tm_tmp);
  start_time = &tm_tmp;

  if (flag & GETDATE_SHORT_DATE) {
    to += std::sprintf(to, "%02d%02d%02d", start_time->tm_year % 100,
                       start_time->tm_mon + 1, start_time->tm_mday);
  } else if (flag & GETDATE_SHORT_DATE_FULL_YEAR) {
    to += std::sprintf(to, "%4d%02d%02d", start_time->tm_year + 1900,
                       start_time->tm_mon + 1, start_time->tm_mday);
  } else {
    to += std::sprintf(
        to, ((flag & GETDATE_FIXEDLENGTH) ? "%4d-%02d-%02d" : "%d-%02d-%02d"),
        start_time->tm_year + 1900, start_time->tm_mon + 1,
        start_time->tm_mday);
  }
  if (flag & GETDATE_DATE_TIME) {
    if (flag & GETDATE_T_DELIMITER) {
      *to = 'T';
      ++to;
    }
    to += std::sprintf(
        to,
        ((flag & GETDATE_FIXEDLENGTH) ? " %02d:%02d:%02d" : " %2d:%02d:%02d"),
        start_time->tm_hour, start_time->tm_min, start_time->tm_sec);
  } else if (flag & GETDATE_HHMMSSTIME) {
    if (flag & GETDATE_T_DELIMITER) {
      *to = 'T';
      ++to;
    }
    to += std::sprintf(to, "%02d%02d%02d", start_time->tm_hour,
                       start_time->tm_min, start_time->tm_sec);
  }
}
