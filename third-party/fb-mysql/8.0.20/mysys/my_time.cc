/* Copyright (c) 2004, 2020, Oracle and/or its affiliates. All rights reserved.

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
  @defgroup MY_TIME Mysys time utilities
  @ingroup MYSYS
  @{

  @file mysys/my_time.cc

  Implementation of low level time utilities.
*/

/**
   @ingroup MY_TIME
   @page LOW_LEVEL_FORMATS Low-level memory and disk formats

   - @subpage datetime_and_date_low_level_rep
   - @subpage time_low_level_rep
 */

#include "my_time.h"

#include <assert.h>   // assert
#include <algorithm>  // std::max
#include <cctype>     // std::isspace
#include <climits>    // UINT_MAX
#include <cstdio>     // std::sprintf
#include <cstring>    // std::memset

#include "field_types.h"     // enum_field_types
#include "integer_digits.h"  // count_digits, write_digits, write_two_digits
#include "my_byteorder.h"    // int3store
#include "my_systime.h"      // localtime_r
#include "myisampack.h"      // mi_int2store
#include "template_utils.h"  // pointer_cast

const ulonglong log_10_int[20] = {1,
                                  10,
                                  100,
                                  1000,
                                  10000UL,
                                  100000UL,
                                  1000000UL,
                                  10000000UL,
                                  100000000ULL,
                                  1000000000ULL,
                                  10000000000ULL,
                                  100000000000ULL,
                                  1000000000000ULL,
                                  10000000000000ULL,
                                  100000000000000ULL,
                                  1000000000000000ULL,
                                  10000000000000000ULL,
                                  100000000000000000ULL,
                                  1000000000000000000ULL,
                                  10000000000000000000ULL};

const char my_zero_datetime6[] = "0000-00-00 00:00:00.000000";

/**
   Position for YYYY-DD-MM HH-MM-DD.FFFFFF AM in default format.
*/
static constexpr const uchar internal_format_positions[] = {0, 1, 2, 3,
                                                            4, 5, 6, 255};

static constexpr const char time_separator = ':';

/** Day number with 1970-01-01 as  base. */
static constexpr ulong const days_at_timestart = 719528;
const uchar days_in_month[] = {31, 28, 31, 30, 31, 30, 31,
                               31, 30, 31, 30, 31, 0};

/**
   Offset of system time zone from UTC in seconds used to speed up
   work of my_system_gmt_sec() function.
*/
static long my_time_zone = 0;

// Right-shift of a negative value is implementation-defined
// Assert that we have arithmetic shift of negative numbers
static_assert((-2 >> 1) == -1, "Right shift of negative numbers is arithmetic");
static longlong my_packed_time_get_int_part(longlong i) { return (i >> 24); }

static longlong my_packed_time_make(longlong i, longlong f) {
  assert(std::abs(f) <= 0xffffffLL);
  return (static_cast<ulonglong>(i) << 24) + f;
}

static longlong my_packed_time_make_int(longlong i) {
  return (static_cast<ulonglong>(i) << 24);
}

// The behavior of <cctype> functions is undefined if the argument's value is
// neither representable as unsigned char nor equal to EOF. To use these
// functions safely with plain chars, cast to unsigned char.
static inline int isspace_char(char ch) {
  return std::isspace(static_cast<unsigned char>(ch));
}

static inline int isdigit_char(char ch) {
  return std::isdigit(static_cast<unsigned char>(ch));
}

static inline int ispunct_char(char ch) {
  return std::ispunct(static_cast<unsigned char>(ch));
}

/**
   Calc days in one year.
   @note Works with both two and four digit years.

   @return number of days in that year
*/
uint calc_days_in_year(uint year) {
  return ((year & 3) == 0 && (year % 100 || (year % 400 == 0 && year)) ? 366
                                                                       : 365);
}

/**
   Set MYSQL_TIME structure to 0000-00-00 00:00:00.000000
   @param [out] tm    The value to set.
   @param time_type  Timestasmp type
*/
void set_zero_time(MYSQL_TIME *tm, enum enum_mysql_timestamp_type time_type) {
  memset(tm, 0, sizeof(*tm));
  tm->time_type = time_type;
}

/**
  Set hour, minute and second of a MYSQL_TIME variable to maximum time value.
  Unlike set_max_time(), does not touch the other structure members.
*/
void set_max_hhmmss(MYSQL_TIME *tm) {
  tm->hour = TIME_MAX_HOUR;
  tm->minute = TIME_MAX_MINUTE;
  tm->second = TIME_MAX_SECOND;
}

/**
  Set MYSQL_TIME variable to maximum time value
  @param tm    OUT  The variable to set.
  @param neg        Sign: 1 if negative, 0 if positive.
*/
void set_max_time(MYSQL_TIME *tm, bool neg) {
  set_zero_time(tm, MYSQL_TIMESTAMP_TIME);
  set_max_hhmmss(tm);
  tm->neg = neg;
}

/**
  @brief Check datetime value for validity according to flags.

  @param[in]  my_time        Date to check.
  @param[in]  not_zero_date  my_time is not the zero date
  @param[in]  flags          flags to check
                             (see str_to_datetime() flags in my_time.h)
  @param[out] was_cut        set to 2 if value was invalid according to flags.
                             (Feb 29 in non-leap etc.). This remains unchanged
                             if value is not invalid.

  @details Here we assume that year and month is ok!
    If month is 0 we allow any date. (This only happens if we allow zero
    date parts in str_to_datetime())
    Disallow dates with zero year and non-zero month and/or day.

  @retval false  OK
  @retval true   error
*/
bool check_date(const MYSQL_TIME &my_time, bool not_zero_date,
                my_time_flags_t flags, int *was_cut) {
  if (not_zero_date) {
    if (((flags & TIME_NO_ZERO_IN_DATE) || !(flags & TIME_FUZZY_DATE)) &&
        (my_time.month == 0 || my_time.day == 0)) {
      *was_cut = MYSQL_TIME_WARN_ZERO_IN_DATE;
      return true;
    } else if ((!(flags & TIME_INVALID_DATES) && my_time.month &&
                my_time.day > days_in_month[my_time.month - 1] &&
                (my_time.month != 2 || calc_days_in_year(my_time.year) != 366 ||
                 my_time.day != 29))) {
      *was_cut = MYSQL_TIME_WARN_OUT_OF_RANGE;
      return true;
    }
  } else if (flags & TIME_NO_ZERO_DATE) {
    *was_cut = MYSQL_TIME_WARN_ZERO_DATE;
    return true;
  }
  return false;
}

/**
  Check if TIME fields can be adjusted to make the time value valid.

  @param  my_time Time value.
  @retval true    if the value cannot be made valid.
  @retval false   if the value is already valid or can be adjusted to
                  become valid.
*/
bool check_time_mmssff_range(const MYSQL_TIME &my_time) {
  return my_time.minute >= 60 || my_time.second >= 60 ||
         my_time.second_part > 999999;
}

/**
  Check TIME range. The value can include day part,
  for example:  '1 10:20:30.123456'.

  minute, second and second_part values are not checked
  unless hour is equal TIME_MAX_HOUR.

  @param my_time Time value.
  @returns       Test result.
  @retval        false if value is Ok.
  @retval        true if value is out of range.
*/
bool check_time_range_quick(const MYSQL_TIME &my_time) {
  longlong hour = static_cast<longlong>(my_time.hour) + 24LL * my_time.day;
  /* The input value should not be fatally bad */
  assert(!check_time_mmssff_range(my_time));
  if (hour <= TIME_MAX_HOUR &&
      (hour != TIME_MAX_HOUR || my_time.minute != TIME_MAX_MINUTE ||
       my_time.second != TIME_MAX_SECOND || !my_time.second_part))
    return false;
  return true;
}

/**
  Check datetime, date, or normalized time (i.e. time without days) range.
  @param my_time  Datetime value.
  @retval false on success
  @retval true  on error
*/
bool check_datetime_range(const MYSQL_TIME &my_time) {
  /*
    In case of MYSQL_TIMESTAMP_TIME hour value can be up to TIME_MAX_HOUR.
    In case of MYSQL_TIMESTAMP_DATETIME it cannot be bigger than 23.
  */
  return my_time.year > 9999U || my_time.month > 12U || my_time.day > 31U ||
         my_time.minute > 59U || my_time.second > 59U ||
         my_time.second_part > 999999U ||
         (my_time.hour >
          (my_time.time_type == MYSQL_TIMESTAMP_TIME ? TIME_MAX_HOUR : 23U));
}

#define MAX_DATE_PARTS 8

/**
  Parses a time zone displacement string on the form `{+-}HH:MM`, converting
  to seconds.

  @param[in]  str    Time zone displacement string.
  @param[in]  length Length of said string.
  @param[out] result Calculated displacement in seconds.

  @retval false Ok.
  @retval true  Not a valid time zone displacement string.
*/
bool time_zone_displacement_to_seconds(const char *str, size_t length,
                                       int *result) {
  if (length < 6) return true;

  int sign = str[0] == '+' ? 1 : (str[0] == '-' ? -1 : 0);
  if (sign == 0) return true;

  if (!(std::isdigit(str[1]) && std::isdigit(str[2]))) return true;
  int hours = (str[1] - '0') * 10 + str[2] - '0';

  if (str[3] != ':') return true;

  if (!(std::isdigit(str[4]) && std::isdigit(str[5]))) return true;
  int minutes = (str[4] - '0') * 10 + str[5] - '0';
  if (minutes >= MINS_PER_HOUR) return true;
  int seconds = hours * SECS_PER_HOUR + minutes * SECS_PER_MIN;

  if (seconds > MAX_TIME_ZONE_HOURS * SECS_PER_HOUR) return true;

  // The SQL standard forbids -00:00.
  if (sign == -1 && hours == 0 && minutes == 0) return true;

  for (size_t i = 6; i < length; ++i)
    if (!std::isspace(str[i])) return true;

  *result = seconds * sign;
  return false;
}

/**
   Convert a timestamp string to a MYSQL_TIME value.

   DESCRIPTION
      At least the following formats are recogniced (based on number of digits)
      YYMMDD, YYYYMMDD, YYMMDDHHMMSS, YYYYMMDDHHMMSS
      YY-MM-DD, YYYY-MM-DD, YY-MM-DD HH.MM.SS
      YYYYMMDDTHHMMSS  where T is a the character T (ISO8601)
      Also dates where all parts are zero are allowed

      The second part may have an optional .###### fraction part.
      The datetime value may be followed by a time zone displacement +/-HH:MM.

    NOTES
     This function should work with a format position vector as long as the
     following things holds:
     - All date are kept together and all time parts are kept together
     - Date and time parts must be separated by blank
     - Second fractions must come after second part and be separated
       by a '.'.  (The second fractions are optional)
     - AM/PM must come after second fractions (or after seconds if no fractions)
     - Year must always been specified.
     - If time is before date, then we will use datetime format only if
       the argument consist of two parts, separated by space.
       Otherwise we will assume the argument is a date.
     - The hour part must be specified in hour-minute-second order.

      status->warnings is set to:
      0                            Value OK
      MYSQL_TIME_WARN_TRUNCATED    If value was cut during conversion
      MYSQL_TIME_WARN_OUT_OF_RANGE check_date(date,flags) considers date invalid

      l_time->time_type is set as follows:
      MYSQL_TIMESTAMP_NONE        String wasn't a timestamp, like
                                  [DD [HH:[MM:[SS]]]].fraction.
                                  l_time is not changed.
      MYSQL_TIMESTAMP_DATE        DATE string (YY MM and DD parts ok)
      MYSQL_TIMESTAMP_DATETIME    Full timestamp
      MYSQL_TIMESTAMP_ERROR       Timestamp with wrong values.
                                  All elements in l_time is set to 0

      flags is a bit field with the follwing possible values:
       TIME_FUZZY_DATE
       TIME_DATETIME_ONLY
       TIME_NO_ZERO_IN_DATE
       TIME_NO_ZERO_DATE
       TIME_INVALID_DATES

    @param str          String to parse
    @param length       Length of string
    @param[out] l_time  Date is stored here
    @param flags        Bitfield
   TIME_FUZZY_DATE|TIME_DATETIME_ONLY|TIME_NO_ZERO_IN_DATE|TIME_NO_ZERO_DATE|TIME_INVALID_DATES
   (described above)
    @param status Conversion status and warnings

    @retval false Ok
    @retval true  Error
  */
bool str_to_datetime(const char *str, std::size_t length, MYSQL_TIME *l_time,
                     my_time_flags_t flags, MYSQL_TIME_STATUS *status) {
  uint field_length = 0;
  uint year_length = 0;
  uint digits;
  uint i;
  uint number_of_fields;
  uint date[MAX_DATE_PARTS];
  uint date_len[MAX_DATE_PARTS];
  uint add_hours = 0;
  uint start_loop;
  ulong not_zero_date;
  ulong allow_space;
  bool is_internal_format = false;
  const char *pos;
  const char *last_field_pos = nullptr;
  const char *end = str + length;
  const uchar *format_position;
  bool found_delimiter = false;
  bool found_space = false;
  bool found_displacement = false;
  uint frac_pos;
  uint frac_len;
  int displacement = 0;

  assert(status->warnings == 0 && status->fractional_digits == 0 &&
         status->nanoseconds == 0);

  /* Skip space at start */
  for (; str != end && isspace_char(*str); str++)
    ;
  if (str == end || !isdigit_char(*str)) {
    status->warnings = MYSQL_TIME_WARN_TRUNCATED;
    l_time->time_type = MYSQL_TIMESTAMP_NONE;
    return true;
  }

  is_internal_format = false;
  /* This has to be changed if want to activate different timestamp formats */
  format_position = internal_format_positions;

  /*
    Calculate number of digits in first part.
    If length= 8 or >= 14 then year is of format YYYY.
    (YYYY-MM-DD,  YYYYMMDD, YYYYYMMDDHHMMSS)
  */
  for (pos = str; pos != end && (isdigit_char(*pos) || *pos == 'T'); pos++)
    ;

  digits = static_cast<uint>(pos - str);
  start_loop = 0;                   /* Start of scan loop */
  date_len[format_position[0]] = 0; /* Length of year field */
  if (pos == end || *pos == '.') {
    /* Found date in internal format (only numbers like YYYYMMDD) */
    year_length = (digits == 4 || digits == 8 || digits >= 14) ? 4 : 2;
    field_length = year_length;
    is_internal_format = true;
    format_position = internal_format_positions;
  } else {
    if (format_position[0] >= 3) /* If year is after HHMMDD */
    {
      /*
        If year is not in first part then we have to determinate if we got
        a date field or a datetime field.
        We do this by checking if there is two numbers separated by
        space in the input.
      */
      while (pos < end && !isspace_char(*pos)) pos++;
      while (pos < end && !isdigit_char(*pos)) pos++;
      if (pos == end) {
        if (flags & TIME_DATETIME_ONLY) {
          status->warnings = MYSQL_TIME_WARN_TRUNCATED;
          l_time->time_type = MYSQL_TIMESTAMP_NONE;
          return true; /* Can't be a full datetime */
        }
        /* Date field.  Set hour, minutes and seconds to 0 */
        date[0] = 0;
        date[1] = 0;
        date[2] = 0;
        date[3] = 0;
        date[4] = 0;
        start_loop = 5; /* Start with first date part */
      }
    }

    field_length = format_position[0] == 0 ? 4 : 2;
  }

  /*
    Only allow space in the first "part" of the datetime field and:
    - after days, part seconds
    - before and after AM/PM (handled by code later)

    2003-03-03 20:00:20 AM
    20:00:20.000000 AM 03-03-2000
  */
  i = *std::max_element(format_position, format_position + 3);

  allow_space = ((1 << i) | (1 << format_position[6]));
  allow_space &= (1 | 2 | 4 | 8 | 64);

  not_zero_date = 0;
  for (i = start_loop;
       i < MAX_DATE_PARTS - 1 && str != end && isdigit_char(*str); i++) {
    const char *start = str;
    ulong tmp_value = static_cast<uchar>(*str++ - '0');

    /*
      Internal format means no delimiters; every field has a fixed
      width. Otherwise, we scan until we find a delimiter and discard
      leading zeroes -- except for the microsecond part, where leading
      zeroes are significant, and where we never process more than six
      digits.
    */
    bool scan_until_delim = !is_internal_format && (i != format_position[6]);

    while (str != end && isdigit_char(str[0]) &&
           (scan_until_delim || --field_length)) {
      tmp_value =
          tmp_value * 10 + static_cast<ulong>(static_cast<uchar>(*str - '0'));
      str++;
    }
    date_len[i] = static_cast<uint>(str - start);
    if (tmp_value > 999999) /* Impossible date part */
    {
      status->warnings = MYSQL_TIME_WARN_TRUNCATED;
      l_time->time_type = MYSQL_TIMESTAMP_NONE;
      return true;
    }
    date[i] = tmp_value;
    not_zero_date |= tmp_value;

    /* Length of next field */
    field_length = format_position[i + 1] == 0 ? 4 : 2;

    if ((last_field_pos = str) == end) {
      i++; /* Register last found part */
      break;
    }
    /* Allow a 'T' after day to allow CCYYMMDDT type of fields */
    if (i == format_position[2] && *str == 'T') {
      str++; /* ISO8601:  CCYYMMDDThhmmss */
      continue;
    }
    if (i == format_position[5]) /* Seconds */
    {
      if (*str == '.') /* Followed by part seconds */
      {
        str++;
        /*
          Shift last_field_pos, so '2001-01-01 00:00:00.'
          is treated as a valid value
        */
        last_field_pos = str;
        field_length = 6; /* 6 digits */
      } else if (isdigit_char(str[0])) {
        /*
          We do not see a decimal point which would have indicated a
          fractional second part in further read. So we skip the further
          processing of digits.
        */
        i++;
        break;
      } else if (str[0] == '+' || str[0] == '-') {
        if (!time_zone_displacement_to_seconds(str, end - str, &displacement)) {
          found_displacement = true;
          str += end - str;
          last_field_pos = str;
        } else {
          status->warnings = MYSQL_TIME_WARN_TRUNCATED;
          l_time->time_type = MYSQL_TIMESTAMP_NONE;
          return true;
        }
      }
      continue;
    }
    if (i == format_position[6] && (str[0] == '+' || str[0] == '-')) {
      if (!time_zone_displacement_to_seconds(str, end - str, &displacement)) {
        found_displacement = true;
        str += end - str;
        last_field_pos = str;
      } else {
        status->warnings = MYSQL_TIME_WARN_TRUNCATED;
        l_time->time_type = MYSQL_TIMESTAMP_NONE;
        return true;
      }
    }

    while (str != end && (ispunct_char(*str) || isspace_char(*str))) {
      if (isspace_char(*str)) {
        if (!(allow_space & (1 << i))) {
          status->warnings = MYSQL_TIME_WARN_TRUNCATED;
          l_time->time_type = MYSQL_TIMESTAMP_NONE;
          return true;
        }
        found_space = true;
      }
      str++;
      found_delimiter = true; /* Should be a 'normal' date */
    }
    /* Check if next position is AM/PM */
    if (i == format_position[6]) /* Seconds, time for AM/PM */
    {
      i++;                           /* Skip AM/PM part */
      if (format_position[7] != 255) /* If using AM/PM */
      {
        if (str + 2 <= end && (str[1] == 'M' || str[1] == 'm')) {
          if (str[0] == 'p' || str[0] == 'P')
            add_hours = 12;
          else if (str[0] != 'a' && str[0] != 'A')
            continue; /* Not AM/PM */
          str += 2;   /* Skip AM/PM */
          /* Skip space after AM/PM */
          while (str != end && isspace_char(*str)) str++;
        }
      }
    }
    last_field_pos = str;
  }
  if (found_delimiter && !found_space && (flags & TIME_DATETIME_ONLY)) {
    status->warnings = MYSQL_TIME_WARN_TRUNCATED;
    l_time->time_type = MYSQL_TIMESTAMP_NONE;
    return true; /* Can't be a datetime */
  }

  str = last_field_pos;

  number_of_fields = i - start_loop;
  while (i < MAX_DATE_PARTS) {
    date_len[i] = 0;
    date[i++] = 0;
  }

  if (!is_internal_format) {
    year_length = date_len[static_cast<uint>(format_position[0])];
    if (!year_length) /* Year must be specified */
    {
      status->warnings = MYSQL_TIME_WARN_TRUNCATED;
      l_time->time_type = MYSQL_TIMESTAMP_NONE;
      return true;
    }

    l_time->year = date[static_cast<uint>(format_position[0])];
    l_time->month = date[static_cast<uint>(format_position[1])];
    l_time->day = date[static_cast<uint>(format_position[2])];
    l_time->hour = date[static_cast<uint>(format_position[3])];
    l_time->minute = date[static_cast<uint>(format_position[4])];
    l_time->second = date[static_cast<uint>(format_position[5])];
    l_time->time_zone_displacement = displacement;

    frac_pos = static_cast<uint>(format_position[6]);
    frac_len = date_len[frac_pos];
    status->fractional_digits = frac_len;
    if (frac_len < 6)
      date[frac_pos] *=
          static_cast<uint>(log_10_int[DATETIME_MAX_DECIMALS - frac_len]);
    l_time->second_part = date[frac_pos];

    if (format_position[7] != static_cast<uchar>(255)) {
      if (l_time->hour > 12) {
        status->warnings = MYSQL_TIME_WARN_TRUNCATED;
        goto err;
      }
      l_time->hour = l_time->hour % 12 + add_hours;
    }
  } else {
    l_time->year = date[0];
    l_time->month = date[1];
    l_time->day = date[2];
    l_time->hour = date[3];
    l_time->minute = date[4];
    l_time->second = date[5];
    if (date_len[6] < 6)
      date[6] *=
          static_cast<uint>(log_10_int[DATETIME_MAX_DECIMALS - date_len[6]]);
    l_time->second_part = date[6];
    l_time->time_zone_displacement = displacement;
    status->fractional_digits = date_len[6];
  }
  l_time->neg = false;

  if (year_length == 2 && not_zero_date)
    l_time->year += (l_time->year < YY_PART_YEAR ? 2000 : 1900);

  /*
    Set time_type before check_datetime_range(),
    as the latter relies on initialized time_type value.
  */
  l_time->time_type =
      (number_of_fields <= 3 ? MYSQL_TIMESTAMP_DATE
                             : (found_displacement ? MYSQL_TIMESTAMP_DATETIME_TZ
                                                   : MYSQL_TIMESTAMP_DATETIME));

  if (number_of_fields < 3 || check_datetime_range(*l_time)) {
    /* Only give warning for a zero date if there is some garbage after */
    if (!not_zero_date) /* If zero date */
    {
      for (; str != end; str++) {
        if (!isspace_char(*str)) {
          not_zero_date = 1; /* Give warning */
          break;
        }
      }
    }
    status->warnings |=
        not_zero_date ? MYSQL_TIME_WARN_TRUNCATED : MYSQL_TIME_WARN_ZERO_DATE;
    goto err;
  }

  if (check_date(*l_time, not_zero_date != 0, flags, &status->warnings))
    goto err;

  /* Scan all digits left after microseconds */
  if (status->fractional_digits == 6 && str != end) {
    if (isdigit_char(*str)) {
      /*
        We don't need the exact nanoseconds value.
        Knowing the first digit is enough for rounding.
      */
      status->nanoseconds = 100 * (*str++ - '0');
      for (; str != end && isdigit_char(*str); str++) {
      }
    }
  }

  if (str != end && (str[0] == '+' || str[0] == '-')) {
    if (time_zone_displacement_to_seconds(str, end - str, &displacement)) {
      status->warnings = MYSQL_TIME_WARN_TRUNCATED;
      l_time->time_type = MYSQL_TIMESTAMP_NONE;
      return true;
    } else {
      l_time->time_type = MYSQL_TIMESTAMP_DATETIME_TZ;
      l_time->time_zone_displacement = displacement;
      return false;
    }
  }

  for (; str != end; str++) {
    if (!isspace_char(*str)) {
      status->warnings = MYSQL_TIME_WARN_TRUNCATED;
      break;
    }
  }

  return false;

err:
  set_zero_time(l_time, MYSQL_TIMESTAMP_ERROR);
  return true;
}

/**
 Convert a time string to a MYSQL_TIME struct.

 status.warning is set to:
     MYSQL_TIME_WARN_TRUNCATED flag if the input string
                        was cut during conversion, and/or
     MYSQL_TIME_WARN_OUT_OF_RANGE flag, if the value is out of range.

 @note
     Because of the extra days argument, this function can only
     work with times where the time arguments are in the above order.

 @param      str     A string in full TIMESTAMP format or
                         [-] DAYS [H]H:MM:SS, [H]H:MM:SS, [M]M:SS, [H]HMMSS,
                         [M]MSS or [S]S
 @param      length  Length of str
 @param[out] l_time  Store result here
 @param[out] status  Conversion status, including warnings.
 @param      flags   Optional flags to control conversion

 @retval false  Ok
 @retval true   Error
*/
bool str_to_time(const char *str, std::size_t length, MYSQL_TIME *l_time,
                 MYSQL_TIME_STATUS *status, my_time_flags_t flags) {
  ulong date[5];
  ulonglong value;
  const char *end = str + length;
  const char *end_of_days;
  bool found_days;
  bool found_hours;
  uint state;
  const char *start;
  bool seen_colon = false;

  assert(status->warnings == 0 && status->fractional_digits == 0 &&
         status->nanoseconds == 0);

  l_time->neg = false;
  for (; str != end && isspace_char(*str); str++) length--;
  if (str != end && *str == '-') {
    l_time->neg = true;
    str++;
    length--;
  }
  if (str == end) return true;

  // Remember beginning of first non-space/- char.
  start = str;

  /* Check first if this is a full TIMESTAMP */
  if (length >= 12) { /* Probably full timestamp */
    MYSQL_TIME_STATUS tmpstatus;
    (void)str_to_datetime(str, length, l_time,
                          (TIME_FUZZY_DATE | TIME_DATETIME_ONLY), &tmpstatus);
    if (l_time->time_type >= MYSQL_TIMESTAMP_ERROR) {
      *status = tmpstatus;
      return l_time->time_type == MYSQL_TIMESTAMP_ERROR;
    }
    assert(status->warnings == 0 && status->fractional_digits == 0 &&
           status->nanoseconds == 0);
  }

  /* Not a timestamp. Try to get this as a DAYS_TO_SECOND string */
  for (value = 0; str != end && isdigit_char(*str); str++)
    value = value * 10L + static_cast<long>(*str - '0');

  if (value > UINT_MAX) return true;

  /* Skip all space after 'days' */
  end_of_days = str;
  for (; str != end && isspace_char(str[0]); str++)
    ;

  state = 0;
  found_days = found_hours = false;
  if (static_cast<uint>(end - str) > 1 && str != end_of_days &&
      isdigit_char(*str)) { /* Found days part */
    date[0] = static_cast<ulong>(value);
    state = 1; /* Assume next is hours */
    found_days = true;
  } else if ((end - str) > 1 && *str == time_separator &&
             isdigit_char(str[1])) {
    date[0] = 0; /* Assume we found hours */
    date[1] = static_cast<ulong>(value);
    state = 2;
    found_hours = true;
    str++; /* skip ':' */
    seen_colon = true;
  } else {
    /* String given as one number; assume HHMMSS format */
    date[0] = 0;
    date[1] = static_cast<ulong>(value / 10000);
    date[2] = static_cast<ulong>(value / 100 % 100);
    date[3] = static_cast<ulong>(value % 100);
    state = 4;
    goto fractional;
  }

  /* Read hours, minutes and seconds */
  for (;;) {
    for (value = 0; str != end && isdigit_char(*str); str++)
      value = value * 10L + static_cast<long>(*str - '0');
    date[state++] = static_cast<ulong>(value);
    if (state == 4 || (end - str) < 2 || *str != time_separator ||
        !isdigit_char(str[1]))
      break;
    str++; /* Skip time_separator (':') */
    seen_colon = true;
  }

  if (state != 4) { /* Not HH:MM:SS */
    /* Fix the date to assume that seconds was given */
    if (!found_hours && !found_days) {
      std::size_t len = sizeof(long) * (state - 1);
      memmove(pointer_cast<uchar *>(date + 4) - len,
              pointer_cast<uchar *>(date + state) - len, len);
      memset(date, 0, sizeof(long) * (4 - state));
    } else
      memset((date + state), 0, sizeof(long) * (4 - state));
  }

fractional:
  /* Get fractional second part */
  if ((end - str) >= 2 && *str == '.' && isdigit_char(str[1])) {
    int field_length = 5;
    str++;
    value = static_cast<uint>(static_cast<uchar>(*str - '0'));
    while (++str != end && isdigit_char(*str)) {
      if (field_length-- > 0)
        value = value * 10 + static_cast<uint>(static_cast<uchar>(*str - '0'));
    }
    if (field_length >= 0) {
      status->fractional_digits = DATETIME_MAX_DECIMALS - field_length;
      if (field_length > 0)
        value *= static_cast<long>(log_10_int[field_length]);
    } else {
      /* Scan digits left after microseconds */
      status->fractional_digits = 6;
      status->nanoseconds = 100 * (str[-1] - '0');
      for (; str != end && isdigit_char(*str); str++) {
      }
    }
    date[4] = static_cast<ulong>(value);
  } else if ((end - str) == 1 && *str == '.') {
    str++;
    date[4] = 0;
  } else
    date[4] = 0;

  /* Check for exponent part: E<gigit> | E<sign><digit> */
  /* (may occur as result of %g formatting of time value) */
  if ((end - str) > 1 && (*str == 'e' || *str == 'E') &&
      (isdigit_char(str[1]) || ((str[1] == '-' || str[1] == '+') &&
                                (end - str) > 2 && isdigit_char(str[2]))))
    return true;

  if (internal_format_positions[7] != 255) {
    /* Read a possible AM/PM */
    while (str != end && isspace_char(*str)) str++;
    if (str + 2 <= end && (str[1] == 'M' || str[1] == 'm')) {
      if (str[0] == 'p' || str[0] == 'P') {
        str += 2;
        date[1] = date[1] % 12 + 12;
      } else if (str[0] == 'a' || str[0] == 'A')
        str += 2;
    }
  }

  /* Integer overflow checks */
  if (date[0] > UINT_MAX || date[1] > UINT_MAX || date[2] > UINT_MAX ||
      date[3] > UINT_MAX || date[4] > UINT_MAX)
    return true;

  if (!seen_colon && (flags & TIME_STRICT_COLON)) {
    memset(l_time, 0, sizeof(*l_time));
    status->warnings |= MYSQL_TIME_WARN_OUT_OF_RANGE;
    return true;
  }

  l_time->year = 0; /* For protocol::store_time */
  l_time->month = 0;

  l_time->day = 0;
  l_time->hour = date[1] + date[0] * 24; /* Mix days and hours */

  l_time->minute = date[2];
  l_time->second = date[3];
  l_time->second_part = date[4];
  l_time->time_type = MYSQL_TIMESTAMP_TIME;
  l_time->time_zone_displacement = 0;

  if (check_time_mmssff_range(*l_time)) {
    status->warnings |= MYSQL_TIME_WARN_OUT_OF_RANGE;
    return true;
  }

  /* Adjust the value into supported MYSQL_TIME range */
  adjust_time_range(l_time, &status->warnings);

  /* Check if there is garbage at end of the MYSQL_TIME specification */
  if (str != end) {
    do {
      if (!isspace_char(*str)) {
        status->warnings |= MYSQL_TIME_WARN_TRUNCATED;
        // No char was actually used in conversion - bad value
        if (str == start) {
          l_time->time_type = MYSQL_TIMESTAMP_NONE;
          return true;
        }
        break;
      }
    } while (++str != end);
  }
  return false;
}

/**
  Convert number to TIME
  @param nr            Number to convert.
  @param [out] ltime     Variable to convert to.
  @param [out] warnings  Warning vector.

  @retval false OK
  @retval true No. is out of range
*/
bool number_to_time(longlong nr, MYSQL_TIME *ltime, int *warnings) {
  if (nr > TIME_MAX_VALUE) {
    /* For huge numbers try full DATETIME, like str_to_time does. */
    if (nr >= 10000000000LL) /* '0001-00-00 00-00-00' */
    {
      int warnings_backup = *warnings;
      if (number_to_datetime(nr, ltime, 0, warnings) != -1LL) return false;
      *warnings = warnings_backup;
    }
    set_max_time(ltime, false);
    *warnings |= MYSQL_TIME_WARN_OUT_OF_RANGE;
    return true;
  } else if (nr < -TIME_MAX_VALUE) {
    set_max_time(ltime, true);
    *warnings |= MYSQL_TIME_WARN_OUT_OF_RANGE;
    return true;
  }
  if ((ltime->neg = (nr < 0))) nr = -nr;
  if (nr % 100 >= 60 || nr / 100 % 100 >= 60) /* Check hours and minutes */
  {
    set_zero_time(ltime, MYSQL_TIMESTAMP_TIME);
    *warnings |= MYSQL_TIME_WARN_OUT_OF_RANGE;
    return true;
  }
  ltime->time_type = MYSQL_TIMESTAMP_TIME;
  ltime->year = ltime->month = ltime->day = 0;
  TIME_set_hhmmss(ltime, static_cast<uint>(nr));
  ltime->second_part = 0;
  return false;
}

/**
  Adjust 'time' value to lie in the MYSQL_TIME range.
  If the time value lies outside of the range [-838:59:59, 838:59:59],
  set it to the closest endpoint of the range and set
  MYSQL_TIME_WARN_OUT_OF_RANGE flag in the 'warning' variable.

  @param[in,out]  my_time  pointer to MYSQL_TIME value
  @param[out]  warning  set MYSQL_TIME_WARN_OUT_OF_RANGE flag if the value is
  out of range
*/
void adjust_time_range(MYSQL_TIME *my_time, int *warning) {
  assert(!check_time_mmssff_range(*my_time));
  if (check_time_range_quick(*my_time)) {
    my_time->day = my_time->second_part = 0;
    set_max_hhmmss(my_time);
    *warning |= MYSQL_TIME_WARN_OUT_OF_RANGE;
  }
}

/**
   Prepare offset of system time zone from UTC for my_system_gmt_sec() func.
*/
void my_init_time() {
  time_t seconds;
  struct tm *l_time;
  struct tm tm_tmp;
  MYSQL_TIME my_time;
  bool not_used;

  seconds = time(nullptr);
  localtime_r(&seconds, &tm_tmp);
  l_time = &tm_tmp;
  my_time_zone = 3600; /* Comp. for -3600 in my_gmt_sec */
  my_time.year = static_cast<uint>(l_time->tm_year) + 1900;
  my_time.month = static_cast<uint>(l_time->tm_mon) + 1;
  my_time.day = static_cast<uint>(l_time->tm_mday);
  my_time.hour = static_cast<uint>(l_time->tm_hour);
  my_time.minute = static_cast<uint>(l_time->tm_min);
  my_time.second = static_cast<uint>(l_time->tm_sec);
  my_time.time_type = MYSQL_TIMESTAMP_DATETIME;
  my_time.neg = false;
  my_time.second_part = 0;
  my_system_gmt_sec(my_time, &my_time_zone, &not_used); /* Init my_time_zone */
}

/**
  Handle 2 digit year conversions.

  @param year 2 digit year
  @return Year between 1970-2069
*/
uint year_2000_handling(uint year) {
  if ((year = year + 1900) < 1900 + YY_PART_YEAR) year += 100;
  return year;
}

/**
  Calculate nr of day since year 0 in new date-system (from 1615).

  @param year	  Year (exact 4 digit year, no year conversions)
  @param month  Month
  @param day	  Day

  @note 0000-00-00 is a valid date, and will return 0

  @return Days since 0000-00-00
*/
long calc_daynr(uint year, uint month, uint day) {
  long delsum;
  int temp;
  int y = year; /* may be < 0 temporarily */

  if (y == 0 && month == 0) return 0; /* Skip errors */
  /* Cast to int to be able to handle month == 0 */
  delsum = static_cast<long>(365 * y + 31 * (static_cast<int>(month) - 1) +
                             static_cast<int>(day));
  if (month <= 2)
    y--;
  else
    delsum -= static_cast<long>(static_cast<int>(month) * 4 + 23) / 10;
  temp = ((y / 100 + 1) * 3) / 4;
  assert(delsum + static_cast<int>(y) / 4 - temp >= 0);
  return (delsum + static_cast<int>(y) / 4 - temp);
} /* calc_daynr */

/**
  Convert time in MYSQL_TIME representation in system time zone to its
  my_time_t form (number of seconds in UTC since begginning of Unix Epoch).

  @param my_time         - time value to be converted
  @param my_timezone     - pointer to long where offset of system time zone
                           from UTC will be stored for caching
  @param in_dst_time_gap - set to true if time falls into spring time-gap

  @note
    The idea is to cache the time zone offset from UTC (including daylight
    saving time) for the next call to make things faster. But currently we
    just calculate this offset during startup (by calling my_init_time()
    function) and use it all the time.
    Time value provided should be legal time value (e.g. '2003-01-01 25:00:00'
    is not allowed).

  @return Time in UTC seconds since Unix Epoch representation.
*/
my_time_t my_system_gmt_sec(const MYSQL_TIME &my_time, long *my_timezone,
                            bool *in_dst_time_gap) {
  uint loop;
  time_t tmp = 0;
  int shift = 0;
  MYSQL_TIME tmp_time;
  MYSQL_TIME *t = &tmp_time;
  struct tm *l_time;
  struct tm tm_tmp;
  long diff, current_timezone;

  /*
    Use temp variable to avoid trashing input data, which could happen in
    case of shift required for boundary dates processing.
  */
  // memcpy(&tmp_time, &my_time, sizeof(MYSQL_TIME));
  tmp_time = my_time;

  if (!validate_timestamp_range(*t)) return 0;

  /*
    Calculate the gmt time based on current time and timezone
    The -1 on the end is to ensure that if have a date that exists twice
    (like 2002-10-27 02:00:0 MET), we will find the initial date.

    By doing -3600 we will have to call localtime_r() several times, but
    I couldn't come up with a better way to get a repeatable result :(

    We can't use mktime() as it's buggy on many platforms and not thread safe.

    Note: this code assumes that our time_t estimation is not too far away
    from real value (we assume that localtime_r(tmp) will return something
    within 24 hrs from t) which is probably true for all current time zones.

    Note2: For the dates, which have time_t representation close to
    MAX_INT32 (efficient time_t limit for supported platforms), we should
    do a small trick to avoid overflow. That is, convert the date, which is
    two days earlier, and then add these days to the final value.

    The same trick is done for the values close to 0 in time_t
    representation for platfroms with unsigned time_t (QNX).

    To be more verbose, here is a sample (extracted from the code below):
    (calc_daynr(2038, 1, 19) - (long) days_at_timestart)*86400L + 4*3600L
    would return -2147480896 because of the long type overflow. In result
    we would get 1901 year in localtime_r(), which is an obvious error.

    Alike problem raises with the dates close to Epoch. E.g.
    (calc_daynr(1969, 12, 31) - (long) days_at_timestart)*86400L + 23*3600L
    will give -3600.

    On some platforms, (E.g. on QNX) time_t is unsigned and localtime(-3600)
    wil give us a date around 2106 year. Which is no good.

    Theoreticaly, there could be problems with the latter conversion:
    there are at least two timezones, which had time switches near 1 Jan
    of 1970 (because of political reasons). These are America/Hermosillo and
    America/Mazatlan time zones. They changed their offset on
    1970-01-01 08:00:00 UTC from UTC-8 to UTC-7. For these zones
    the code below will give incorrect results for dates close to
    1970-01-01, in the case OS takes into account these historical switches.
    Luckily, it seems that we support only one platform with unsigned
    time_t. It's QNX. And QNX does not support historical timezone data at all.
    E.g. there are no /usr/share/zoneinfo/ files or any other mean to supply
    historical information for localtime_r() etc. That is, the problem is not
    relevant to QNX.

    We are safe with shifts close to MAX_INT32, as there are no known
    time switches on Jan 2038 yet :)
  */
  if ((t->year == TIMESTAMP_MAX_YEAR) && (t->month == 1) && (t->day > 4)) {
    /*
      Below we will pass static_cast<uint>(t->day - shift) to calc_daynr.
      As we don't want to get an overflow here, we will shift
      only safe dates. That's why we have (t->day > 4) above.
    */
    t->day -= 2;
    shift = 2;
  }

  tmp = static_cast<time_t>(
      ((calc_daynr(static_cast<uint>(t->year), static_cast<uint>(t->month),
                   static_cast<uint>(t->day)) -
        static_cast<long>(days_at_timestart)) *
           SECONDS_IN_24H +
       static_cast<long>(t->hour) * 3600L +
       static_cast<long>(t->minute * 60 + t->second)) +
      static_cast<time_t>(my_time_zone) - 3600);

  current_timezone = my_time_zone;
  localtime_r(&tmp, &tm_tmp);
  l_time = &tm_tmp;
  for (loop = 0; loop < 2 && (t->hour != static_cast<uint>(l_time->tm_hour) ||
                              t->minute != static_cast<uint>(l_time->tm_min) ||
                              t->second != static_cast<uint>(l_time->tm_sec));
       loop++) { /* One check should be enough ? */
    /* Get difference in days */
    int days = t->day - l_time->tm_mday;
    if (days < -1)
      days = 1; /* Month has wrapped */
    else if (days > 1)
      days = -1;
    diff = (3600L * static_cast<long>(days * 24 + (static_cast<int>(t->hour) -
                                                   l_time->tm_hour)) +
            static_cast<long>(60 *
                              (static_cast<int>(t->minute) - l_time->tm_min)) +
            static_cast<long>(static_cast<int>(t->second) - l_time->tm_sec));
    current_timezone += diff + 3600; /* Compensate for -3600 above */
    tmp += static_cast<time_t>(diff);
    localtime_r(&tmp, &tm_tmp);
    l_time = &tm_tmp;
  }
  /*
    Fix that if we are in the non existing daylight saving time hour
    we move the start of the next real hour.

    This code doesn't handle such exotical thing as time-gaps whose length
    is more than one hour or non-integer (latter can theoretically happen
    if one of seconds will be removed due leap correction, or because of
    general time correction like it happened for Africa/Monrovia time zone
    in year 1972).
  */
  if (loop == 2 && t->hour != static_cast<uint>(l_time->tm_hour)) {
    int days = t->day - l_time->tm_mday;
    if (days < -1)
      days = 1; /* Month has wrapped */
    else if (days > 1)
      days = -1;
    diff = (3600L * static_cast<long>(days * 24 + (static_cast<int>(t->hour) -
                                                   l_time->tm_hour)) +
            static_cast<long>(60 *
                              (static_cast<int>(t->minute) - l_time->tm_min)) +
            static_cast<long>(static_cast<int>(t->second) - l_time->tm_sec));
    if (diff == 3600)
      tmp += 3600 - t->minute * 60 - t->second; /* Move to next hour */
    else if (diff == -3600)
      tmp -= t->minute * 60 + t->second; /* Move to previous hour */

    *in_dst_time_gap = true;
  }
  *my_timezone = current_timezone;

  /* shift back, if we were dealing with boundary dates */
  tmp += shift * SECONDS_IN_24H;

  /*
    This is possible for dates, which slightly exceed boundaries.
    Conversion will pass ok for them, but we don't allow them.
    First check will pass for platforms with signed time_t.
    instruction above (tmp+= shift*86400L) could exceed
    MAX_INT32 (== TIMESTAMP_MAX_VALUE) and overflow will happen.
    So, tmp < TIMESTAMP_MIN_VALUE will be triggered. On platfroms
    with unsigned time_t tmp+= shift*86400L might result in a number,
    larger then TIMESTAMP_MAX_VALUE, so another check will work.
  */
  if (!is_time_t_valid_for_timestamp(tmp)) tmp = 0;

  return static_cast<my_time_t>(tmp);
} /* my_system_gmt_sec */

/**
  Writes a two-digit number to a string, padded with zero if it is less than 10.
  If the number is greater than or equal to 100, "00" is written to the string.
  The number should be less than 100 for valid temporal values, but the
  formatting functions need to handle invalid values too, since they are used
  for formatting the values in error/warning messages when invalid values have
  been given by the user.
*/
static char *format_two_digits(int value, char *to) {
  if (value < 0 || value >= 100) value = 0;
  return write_two_digits(value, to);
}

/**
  Print the microsecond part with the specified precision.

  @param[out] to   The string pointer to print at
  @param useconds  The microseconds value
  @param dec       Precision, between 1 and 6

  @return          The length of the result string
*/
static int my_useconds_to_str(char *to, unsigned useconds, unsigned dec) {
  assert(dec <= DATETIME_MAX_DECIMALS);

  // Write the decimal point and the terminating zero character.
  to[0] = '.';
  to[dec + 1] = '\0';

  // Write the dec most significant digits of the microsecond value.
  for (int i = DATETIME_MAX_DECIMALS - dec; i > 0; --i) useconds /= 10;
  write_digits(useconds, dec, to + 1);
  return dec + 1;
}

/**
  Converts a time value to a string with the format HH:MM:SS[.fraction].

  This function doesn't check that the given MYSQL_TIME structure members
  are in the valid range. If they are not, the returned value won't reflect
  any valid time either. Additionally, it doesn't take into
  account time->day member: it's assumed that days have been converted
  to hours already.

  @param      my_time Source time value
  @param[out] to      Destnation char array
  @param      dec     Precision, in the range 0..6

  @return number of characters written to 'to'
*/

int my_time_to_str(const MYSQL_TIME &my_time, char *to, uint dec) {
  const char *const start = to;
  if (my_time.neg) *to++ = '-';

  // Hours should be zero-padded up to two digits. It might have more digits.
  to = write_digits(my_time.hour, std::max(2, count_digits(my_time.hour)), to);

  *to++ = ':';
  to = format_two_digits(my_time.minute, to);
  *to++ = ':';
  to = format_two_digits(my_time.second, to);

  const int length = to - start;
  if (dec) return length + my_useconds_to_str(to, my_time.second_part, dec);
  *to = '\0';
  return length;
}

/**
  Converts a date value to a string with the format 'YYYY-MM-DD'.

  This function doesn't check that the given MYSQL_TIME structure members are
  in the valid range. If they are not, the returned value won't reflect any
  valid date either.

  @param      my_time Source time value
  @param[out] to      Destination character array

  @return number of characters written to 'to'
*/
int my_date_to_str(const MYSQL_TIME &my_time, char *to) {
  const char *const start = to;
  to = format_two_digits(my_time.year / 100, to);
  to = format_two_digits(my_time.year % 100, to);
  *to++ = '-';
  to = format_two_digits(my_time.month, to);
  *to++ = '-';
  to = format_two_digits(my_time.day, to);
  *to = '\0';
  return to - start;
}

/**
  Convert datetime to a string 'YYYY-MM-DD hh:mm:ss'.
  Open coded for better performance.
  This code previously resided in field.cc, in Field_timestamp::val_str().

  @param      my_time  The src MYSQL_TIME value.
  @param[out] to       The string pointer to print at.
  @return The length of the result string.
*/
static int TIME_to_datetime_str(const MYSQL_TIME &my_time, char *to) {
  /* Year */
  to = format_two_digits(my_time.year / 100, to);
  to = format_two_digits(my_time.year % 100, to);
  *to++ = '-';
  /* Month */
  to = format_two_digits(my_time.month, to);
  *to++ = '-';
  /* Day */
  to = format_two_digits(my_time.day, to);
  *to++ = ' ';
  /* Hour */
  to = format_two_digits(my_time.hour, to);
  *to++ = ':';
  /* Minute */
  to = format_two_digits(my_time.minute, to);
  *to++ = ':';
  /* Second */
  format_two_digits(my_time.second, to);
  return 19;
}

/**
  Print a datetime value with an optional fractional part.

  @param       my_time The MYSQL_TIME value to print
  @param [out] to      The string pointer to print at
  @param       dec     Precision, in the range 0..6

  @return The length of the result string.
*/
int my_datetime_to_str(const MYSQL_TIME &my_time, char *to, uint dec) {
  int len = TIME_to_datetime_str(my_time, to);
  if (dec) len += my_useconds_to_str(to + len, my_time.second_part, dec);
  if (my_time.time_type == MYSQL_TIMESTAMP_DATETIME_TZ) {
    int tzd = my_time.time_zone_displacement;
    len += sprintf(to + len, "%+02i:%02i", tzd / SECS_PER_HOUR,
                   std::abs(tzd) / SECS_PER_MIN % MINS_PER_HOUR);
  } else
    to[len] = '\0';
  return len;
}

/**
  Convert struct DATE/TIME/DATETIME value to string using built-in
  MySQL time conversion formats.

  @note The string must have at least MAX_DATE_STRING_REP_LENGTH bytes reserved.
  @param my_time      The MYSQL_TIME value to print
  @param [out] to     The string pointer to print at
  @param dec          Precision, in the range 0..6

  @return number of bytes written
*/
int my_TIME_to_str(const MYSQL_TIME &my_time, char *to, uint dec) {
  switch (my_time.time_type) {
    case MYSQL_TIMESTAMP_DATETIME:
    case MYSQL_TIMESTAMP_DATETIME_TZ:
      return my_datetime_to_str(my_time, to, dec);
    case MYSQL_TIMESTAMP_DATE:
      return my_date_to_str(my_time, to);
    case MYSQL_TIMESTAMP_TIME:
      return my_time_to_str(my_time, to, dec);
    case MYSQL_TIMESTAMP_NONE:
    case MYSQL_TIMESTAMP_ERROR:
      to[0] = '\0';
      return 0;
    default:
      assert(false);
      return 0;
  }
}

/**
  Print a timestamp with an oprional fractional part: XXXXX[.YYYYY]

  @param      tm  The timestamp value to print.
  @param [out] to  The string pointer to print at.
  @param      dec Precision, in the range 0..6.
  @return         The length of the result string.
*/
int my_timeval_to_str(const struct timeval *tm, char *to, uint dec) {
  int len = sprintf(to, "%d", static_cast<int>(tm->tv_sec));
  if (dec) len += my_useconds_to_str(to + len, tm->tv_usec, dec);
  return len;
}

/**
  Convert datetime value specified as number to broken-down TIME
  representation and form value of DATETIME type as side-effect.

  Convert a datetime value of formats YYMMDD, YYYYMMDD, YYMMDDHHMSS,
  YYYYMMDDHHMMSS to broken-down MYSQL_TIME representation. Return value in
  YYYYMMDDHHMMSS format as side-effect.

  This function also checks if datetime value fits in DATETIME range.

    Datetime value in YYYYMMDDHHMMSS format.

    was_cut         if return value -1: one of
                      - MYSQL_TIME_WARN_OUT_OF_RANGE
                      - MYSQL_TIME_WARN_ZERO_DATE
                      - MYSQL_TIME_WARN_TRUNCATED
                    otherwise 0.

  @param         nr        datetime value as number
  @param[in,out] time_res  pointer for structure for broken-down
                            representation
  @param         flags     TIME_NO_ZERO_DATE and flags used by check_date()
  @param[out]    was_cut   0 Value ok
                           1 If value was cut during conversion
                           2 check_date(date,flags) considers date invalid

  @retval -1              Timestamp with wrong values, e.g. nr == 0 with
                          TIME_NO_ZERO_DATE
  @retval anything else   DATETIME as integer in YYYYMMDDHHMMSS format
*/
longlong number_to_datetime(longlong nr, MYSQL_TIME *time_res,
                            my_time_flags_t flags, int *was_cut) {
  long part1;
  long part2;

  *was_cut = 0;
  memset(time_res, 0, sizeof(*time_res));
  time_res->time_type = MYSQL_TIMESTAMP_DATE;

  if (nr == 0LL || nr >= 10000101000000LL) {
    time_res->time_type = MYSQL_TIMESTAMP_DATETIME;
    if (nr > 99999999999999LL) /* 9999-99-99 99:99:99 */
    {
      *was_cut = MYSQL_TIME_WARN_OUT_OF_RANGE;
      return -1LL;
    }
    goto ok;
  }
  if (nr < 101) goto err;
  if (nr <= (YY_PART_YEAR - 1) * 10000L + 1231L) {
    nr = (nr + 20000000L) * 1000000L; /* YYMMDD, year: 2000-2069 */
    goto ok;
  }
  if (nr < (YY_PART_YEAR)*10000L + 101L) goto err;
  if (nr <= 991231L) {
    nr = (nr + 19000000L) * 1000000L; /* YYMMDD, year: 1970-1999 */
    goto ok;
  }
  /*
    Though officially we support DATE values from 1000-01-01 only, one can
    easily insert a value like 1-1-1. So, for consistency reasons such dates
    are allowed when TIME_FUZZY_DATE is set.
  */
  if (nr < 10000101L && !(flags & TIME_FUZZY_DATE)) goto err;
  if (nr <= 99991231L) {
    nr = nr * 1000000L;
    goto ok;
  }
  if (nr < 101000000L) goto err;

  time_res->time_type = MYSQL_TIMESTAMP_DATETIME;

  if (nr <= (YY_PART_YEAR - 1) * 10000000000LL + 1231235959LL) {
    nr = nr + 20000000000000LL; /* YYMMDDHHMMSS, 2000-2069 */
    goto ok;
  }
  if (nr < YY_PART_YEAR * 10000000000LL + 101000000LL) goto err;
  if (nr <= 991231235959LL)
    nr = nr + 19000000000000LL; /* YYMMDDHHMMSS, 1970-1999 */

ok:
  part1 = static_cast<long>(nr / 1000000LL);
  part2 = static_cast<long>(nr - static_cast<longlong>(part1) * 1000000LL);
  time_res->year = static_cast<int>(part1 / 10000L);
  part1 %= 10000L;
  time_res->month = static_cast<int>(part1) / 100;
  time_res->day = static_cast<int>(part1) % 100;
  time_res->hour = static_cast<int>(part2 / 10000L);
  part2 %= 10000L;
  time_res->minute = static_cast<int>(part2) / 100;
  time_res->second = static_cast<int>(part2) % 100;

  if (!check_datetime_range(*time_res) &&
      !check_date(*time_res, (nr != 0), flags, was_cut))
    return nr;

  /* Don't want to have was_cut get set if TIME_NO_ZERO_DATE was violated. */
  if (!nr && (flags & TIME_NO_ZERO_DATE)) return -1LL;

err:
  *was_cut = MYSQL_TIME_WARN_TRUNCATED;
  return -1LL;
}

/**
  Convert time value to integer in YYYYMMDDHHMMSS.

  @param  my_time  The MYSQL_TIME value to convert.

  @return          A number in format YYYYMMDDHHMMSS.
*/
ulonglong TIME_to_ulonglong_datetime(const MYSQL_TIME &my_time) {
  return (static_cast<ulonglong>(my_time.year * 10000UL +
                                 my_time.month * 100UL + my_time.day) *
              1000000ULL +
          static_cast<ulonglong>(my_time.hour * 10000UL +
                                 my_time.minute * 100UL + my_time.second));
}

/**
  Convert MYSQL_TIME value to integer in YYYYMMDD format

  @param my_time  The MYSQL_TIME value to convert.
  @return         A number in format YYYYMMDD.
*/
ulonglong TIME_to_ulonglong_date(const MYSQL_TIME &my_time) {
  return static_cast<ulonglong>(my_time.year * 10000UL + my_time.month * 100UL +
                                my_time.day);
}

/**
  Convert MYSQL_TIME value to integer in HHMMSS format.
  This function doesn't take into account time->day member:
  it's assumed that days have been converted to hours already.

  @param my_time  The TIME value to convert.
  @return         The number in HHMMSS format.
*/
ulonglong TIME_to_ulonglong_time(const MYSQL_TIME &my_time) {
  return static_cast<ulonglong>(my_time.hour * 10000UL +
                                my_time.minute * 100UL + my_time.second);
}

/**
  Set day, month and year from a number.

  @param ltime    MYSQL_TIME variable
  @param yymmdd   Number in YYYYMMDD format
*/
void TIME_set_yymmdd(MYSQL_TIME *ltime, uint yymmdd) {
  ltime->day = static_cast<int>(yymmdd % 100);
  ltime->month = static_cast<int>(yymmdd / 100) % 100;
  ltime->year = static_cast<int>(yymmdd / 10000);
}

/**
  Set hour, minute and secondr from a number.

  @param ltime    MYSQL_TIME variable
  @param hhmmss   Number in HHMMSS format
*/
void TIME_set_hhmmss(MYSQL_TIME *ltime, uint hhmmss) {
  ltime->second = static_cast<int>(hhmmss % 100);
  ltime->minute = static_cast<int>(hhmmss / 100) % 100;
  ltime->hour = static_cast<int>(hhmmss / 10000);
}

/**
  Convert struct MYSQL_TIME (date and time split into year/month/day/hour/...
  to a number in format YYYYMMDDHHMMSS (DATETIME),
  YYYYMMDD (DATE) or HHMMSS (TIME).

  The function is used when we need to convert value of time item
  to a number if it's used in numeric context, i. e.:
  SELECT NOW()+1, CURDATE()+0, CURTIMIE()+0;
  SELECT ?+1;

  @param my_time Source time value

  @retval 0 in case of errors!
  @retval number in format YYYYMMDDHHMMSS (DATETIME), YYYYMMDD (DATE) or HHMMSS
  (TIME), otherwise.

  @note
    This function doesn't check that given MYSQL_TIME structure members are
    in valid range. If they are not, return value won't reflect any
    valid date either.
*/
ulonglong TIME_to_ulonglong(const MYSQL_TIME &my_time) {
  switch (my_time.time_type) {
    case MYSQL_TIMESTAMP_DATETIME:
      return TIME_to_ulonglong_datetime(my_time);
    case MYSQL_TIMESTAMP_DATE:
      return TIME_to_ulonglong_date(my_time);
    case MYSQL_TIMESTAMP_TIME:
      return TIME_to_ulonglong_time(my_time);
    case MYSQL_TIMESTAMP_NONE:
    case MYSQL_TIMESTAMP_ERROR:
      return 0ULL;
    default:
      assert(false);
  }
  return 0;
}

/**
   Round MYSQL_TIME datetime value and convert to ulonglong representation.

   @param my_time input time
   @param[out] warnings warning vector

   @return time in (u)longlong format
 */
ulonglong TIME_to_ulonglong_datetime_round(const MYSQL_TIME &my_time,
                                           int *warnings) {
  // Catch simple cases
  if (my_time.second_part < 500000) return TIME_to_ulonglong_datetime(my_time);
  if (my_time.second < 59) return TIME_to_ulonglong_datetime(my_time) + 1;
  // Corner case e.g. 'YYYY-MM-DD hh:mm:59.5'. Proceed with slower method.
  MYSQL_TIME tmp = my_time;
  my_datetime_adjust_frac(&tmp, 0, warnings, false);
  return TIME_to_ulonglong_datetime(tmp);  // + TIME_microseconds_round(ltime);
}

/**
   Round MYSQL_TIME time value and convert to to ulonglong representation.

   @param my_time input time
   @return time in (u)longlong format
 */
ulonglong TIME_to_ulonglong_time_round(const MYSQL_TIME &my_time) {
  if (my_time.second_part < 500000) return TIME_to_ulonglong_time(my_time);
  if (my_time.second < 59) return TIME_to_ulonglong_time(my_time) + 1;
  // Corner case e.g. 'hh:mm:59.5'. Proceed with slower method.
  MYSQL_TIME tmp = my_time;
  my_time_adjust_frac(&tmp, 0, false);
  return TIME_to_ulonglong_time(tmp);
}

/**
   @page time_low_level_rep TIME

  In-memory format:

| Bits  | Field         | Value range |
| ----: | :----         | :---- |
|   1   | sign          |(Used for sign, when on disk) |
|   1   | unused        |(Reserved for wider hour range, e.g. for intervals) |
|   10  | hour          |(0-838) |
|   6   | minute        |(0-59) |
|   6   | second        |(0-59) |
|  24   | microseconds  |(0-999999) |

 Total: 48 bits = 6 bytes

@verbatim
Format: Suhhhhhh.hhhhmmmm.mmssssss.ffffffff.ffffffff.ffffffff
@endverbatim
*/

/**
  Convert time value to numeric packed representation.

  @param    my_time The value to convert.
  @return           Numeric packed representation.
*/
longlong TIME_to_longlong_time_packed(const MYSQL_TIME &my_time) {
  /* If month is 0, we mix day with hours: "1 00:10:10" -> "24:00:10" */
  long hms = (((my_time.month ? 0 : my_time.day * 24) + my_time.hour) << 12) |
             (my_time.minute << 6) | my_time.second;
  longlong tmp = my_packed_time_make(hms, my_time.second_part);
  return my_time.neg ? -tmp : tmp;
}

/**
  Convert time packed numeric representation to time.

  @param [out] ltime  The MYSQL_TIME variable to set.
  @param      tmp    The packed numeric representation.
*/
void TIME_from_longlong_time_packed(MYSQL_TIME *ltime, longlong tmp) {
  longlong hms;
  if ((ltime->neg = (tmp < 0))) tmp = -tmp;
  hms = my_packed_time_get_int_part(tmp);
  ltime->year = static_cast<uint>(0);
  ltime->month = static_cast<uint>(0);
  ltime->day = static_cast<uint>(0);
  ltime->hour =
      static_cast<uint>(hms >> 12) % (1 << 10); /* 10 bits starting at 12th */
  ltime->minute =
      static_cast<uint>(hms >> 6) % (1 << 6); /* 6 bits starting at 6th   */
  ltime->second =
      static_cast<uint>(hms) % (1 << 6); /* 6 bits starting at 0th   */
  ltime->second_part = my_packed_time_get_frac_part(tmp);
  ltime->time_type = MYSQL_TIMESTAMP_TIME;
}

/**
  On disk we convert from signed representation to unsigned
  representation using TIMEF_OFS, so all values become binary comparable.
*/
#define TIMEF_OFS 0x800000000000LL
#define TIMEF_INT_OFS 0x800000LL

/**
  Convert in-memory numeric time representation to on-disk representation

  @param       nr   Value in packed numeric time format.
  @param [out] ptr  The buffer to put value at.
  @param       dec  Precision.
*/
void my_time_packed_to_binary(longlong nr, uchar *ptr, uint dec) {
  assert(dec <= DATETIME_MAX_DECIMALS);
  /* Make sure the stored value was previously properly rounded or truncated */
  assert((my_packed_time_get_frac_part(nr) %
          static_cast<int>(log_10_int[DATETIME_MAX_DECIMALS - dec])) == 0);

  switch (dec) {
    case 0:
    default:
      mi_int3store(ptr, TIMEF_INT_OFS + my_packed_time_get_int_part(nr));
      break;

    case 1:
    case 2:
      mi_int3store(ptr, TIMEF_INT_OFS + my_packed_time_get_int_part(nr));
      ptr[3] = static_cast<unsigned char>(
          static_cast<char>(my_packed_time_get_frac_part(nr) / 10000));
      break;

    case 4:
    case 3:
      mi_int3store(ptr, TIMEF_INT_OFS + my_packed_time_get_int_part(nr));
      mi_int2store(ptr + 3, my_packed_time_get_frac_part(nr) / 100);
      break;

    case 5:
    case 6:
      mi_int6store(ptr, nr + TIMEF_OFS);
      break;
  }
}

/**
  Convert on-disk time representation to in-memory packed numeric
  representation.

  @param   ptr  The pointer to read the value at.
  @param   dec  Precision.
  @return       Packed numeric time representation.
*/
longlong my_time_packed_from_binary(const uchar *ptr, uint dec) {
  assert(dec <= DATETIME_MAX_DECIMALS);

  switch (dec) {
    case 0:
    default: {
      longlong intpart = mi_uint3korr(ptr) - TIMEF_INT_OFS;
      return my_packed_time_make_int(intpart);
    }
    case 1:
    case 2: {
      longlong intpart = mi_uint3korr(ptr) - TIMEF_INT_OFS;
      int frac = static_cast<uint>(ptr[3]);
      if (intpart < 0 && frac) {
        /*
           Negative values are stored with
           reverse fractional part order,
           for binary sort compatibility.

            Disk value  intpart frac   Time value   Memory value
            800000.00    0      0      00:00:00.00  0000000000.000000
            7FFFFF.FF   -1      255   -00:00:00.01  FFFFFFFFFF.FFD8F0
            7FFFFF.9D   -1      99    -00:00:00.99  FFFFFFFFFF.F0E4D0
            7FFFFF.00   -1      0     -00:00:01.00  FFFFFFFFFF.000000
            7FFFFE.FF   -1      255   -00:00:01.01  FFFFFFFFFE.FFD8F0
            7FFFFE.F6   -2      246   -00:00:01.10  FFFFFFFFFE.FE7960

            Formula to convert fractional part from disk format
            (now stored in "frac" variable) to absolute value: "0x100 - frac".
            To reconstruct in-memory value, we shift
            to the next integer value and then substruct fractional part.
        */
        intpart++;     /* Shift to the next integer value */
        frac -= 0x100; /* -(0x100 - frac) */
      }
      return my_packed_time_make(intpart, frac * 10000);
    }

    case 3:
    case 4: {
      longlong intpart = mi_uint3korr(ptr) - TIMEF_INT_OFS;
      int frac = mi_uint2korr(ptr + 3);
      if (intpart < 0 && frac) {
        /*
          Fix reverse fractional part order: "0x10000 - frac".
          See comments for FSP=1 and FSP=2 above.
        */
        intpart++;       /* Shift to the next integer value */
        frac -= 0x10000; /* -(0x10000-frac) */
      }
      return my_packed_time_make(intpart, frac * 100);
    }

    case 5:
    case 6:
      return (static_cast<longlong>(mi_uint6korr(ptr))) - TIMEF_OFS;
  }
}

/**
   @page datetime_and_date_low_level_rep DATETIME and DATE

| Bits  | Field         | Value |
| ----: | :----         | :---- |
|    1  | sign          |(used when on disk) |
|   17  | year*13+month |(year 0-9999, month 0-12) |
|    5  | day           |(0-31)|
|    5  | hour          |(0-23)|
|    6  | minute        |(0-59)|
|    6  | second        |(0-59)|
|   24  | microseconds  |(0-999999)|

   Total: 64 bits = 8 bytes

@verbatim
Format: SYYYYYYY.YYYYYYYY.YYdddddh.hhhhmmmm.mmssssss.ffffffff.ffffffff.ffffffff
@endverbatim

*/

/**
  Convert datetime to packed numeric datetime representation.

  @param my_time  The value to convert.
  @return       Packed numeric representation of my_time.
*/
longlong TIME_to_longlong_datetime_packed(const MYSQL_TIME &my_time) {
  longlong ymd = ((my_time.year * 13 + my_time.month) << 5) | my_time.day;
  longlong hms = (my_time.hour << 12) | (my_time.minute << 6) | my_time.second;
  longlong tmp = my_packed_time_make(((ymd << 17) | hms), my_time.second_part);
  assert(!check_datetime_range(my_time)); /* Make sure no overflow */
  return my_time.neg ? -tmp : tmp;
}

/**
  Convert date to packed numeric date representation.
  Numeric packed date format is similar to numeric packed datetime
  representation, with zero hhmmss part.

  @param my_time The value to convert.
  @return      Packed numeric representation of ltime.
*/
longlong TIME_to_longlong_date_packed(const MYSQL_TIME &my_time) {
  longlong ymd = ((my_time.year * 13 + my_time.month) << 5) | my_time.day;
  return my_packed_time_make_int(ymd << 17);
}

/**
  Convert year to packed numeric date representation.
  Packed value for YYYY is the same to packed value for date YYYY-00-00.

  @return packed value for date YYYY-00-00.
*/
longlong year_to_longlong_datetime_packed(long year) {
  longlong ymd = ((year * 13) << 5);
  return my_packed_time_make_int(ymd << 17);
}

/**
  Convert packed numeric datetime representation to MYSQL_TIME.

  @param [out] ltime The datetime variable to convert to.
  @param      tmp   The packed numeric datetime value.
*/
void TIME_from_longlong_datetime_packed(MYSQL_TIME *ltime, longlong tmp) {
  longlong ymd;
  longlong hms;
  longlong ymdhms;
  longlong ym;

  if ((ltime->neg = (tmp < 0))) tmp = -tmp;

  ltime->second_part = my_packed_time_get_frac_part(tmp);
  ymdhms = my_packed_time_get_int_part(tmp);

  ymd = ymdhms >> 17;
  ym = ymd >> 5;
  hms = ymdhms % (1 << 17);

  ltime->day = ymd % (1 << 5);
  ltime->month = ym % 13;
  ltime->year = static_cast<uint>(ym / 13);

  ltime->second = hms % (1 << 6);
  ltime->minute = (hms >> 6) % (1 << 6);
  ltime->hour = static_cast<uint>(hms >> 12);

  ltime->time_type = MYSQL_TIMESTAMP_DATETIME;
  ltime->time_zone_displacement = 0;
}

/**
  Convert packed numeric date representation to MYSQL_TIME.

  @param [out] ltime The date variable to convert to.
  @param      tmp   The packed numeric date value.
*/
void TIME_from_longlong_date_packed(MYSQL_TIME *ltime, longlong tmp) {
  TIME_from_longlong_datetime_packed(ltime, tmp);
  ltime->time_type = MYSQL_TIMESTAMP_DATE;
}

/**
  On disk we store as unsigned number with DATETIMEF_INT_OFS offset,
  for HA_KETYPE_BINARY compatibilty purposes.
*/
#define DATETIMEF_INT_OFS 0x8000000000LL

/**
  Convert on-disk datetime representation
  to in-memory packed numeric representation.

  @param ptr   The pointer to read value at.
  @param dec   Precision.
  @return      In-memory packed numeric datetime representation.
*/
longlong my_datetime_packed_from_binary(const uchar *ptr, uint dec) {
  longlong intpart = mi_uint5korr(ptr) - DATETIMEF_INT_OFS;
  int frac;
  assert(dec <= DATETIME_MAX_DECIMALS);
  switch (dec) {
    case 0:
    default:
      return my_packed_time_make_int(intpart);
    case 1:
    case 2:
      frac = (static_cast<int>(static_cast<signed char>(ptr[5]))) * 10000;
      break;
    case 3:
    case 4:
      frac = mi_sint2korr(ptr + 5) * 100;
      break;
    case 5:
    case 6:
      frac = mi_sint3korr(ptr + 5);
      break;
  }
  return my_packed_time_make(intpart, frac);
}

/**
  Store in-memory numeric packed datetime representation to disk.

  @param      nr  In-memory numeric packed datetime representation.
  @param [out] ptr The pointer to store at.
  @param      dec Precision, 1-6.
*/
void my_datetime_packed_to_binary(longlong nr, uchar *ptr, uint dec) {
  assert(dec <= DATETIME_MAX_DECIMALS);
  /* The value being stored must have been properly rounded or truncated */
  assert((my_packed_time_get_frac_part(nr) %
          static_cast<int>(log_10_int[DATETIME_MAX_DECIMALS - dec])) == 0);

  mi_int5store(ptr, my_packed_time_get_int_part(nr) + DATETIMEF_INT_OFS);
  switch (dec) {
    case 0:
    default:
      break;
    case 1:
    case 2:
      ptr[5] = static_cast<unsigned char>(
          static_cast<char>(my_packed_time_get_frac_part(nr) / 10000));
      break;
    case 3:
    case 4:
      mi_int2store(ptr + 5, my_packed_time_get_frac_part(nr) / 100);
      break;
    case 5:
    case 6:
      mi_int3store(ptr + 5, my_packed_time_get_frac_part(nr));
  }
}

/*** TIMESTAMP low-level memory and disk representation routines ***/

/**
  Convert binary timestamp representation to in-memory representation.

  @param [out] tm  The variable to convert to.
  @param      ptr The pointer to read the value from.
  @param      dec Precision.
*/
void my_timestamp_from_binary(struct timeval *tm, const uchar *ptr, uint dec) {
  assert(dec <= DATETIME_MAX_DECIMALS);
  tm->tv_sec = mi_uint4korr(ptr);
  switch (dec) {
    case 0:
    default:
      tm->tv_usec = 0;
      break;
    case 1:
    case 2:
      tm->tv_usec = (static_cast<int>(ptr[4])) * 10000;
      break;
    case 3:
    case 4:
      tm->tv_usec = mi_sint2korr(ptr + 4) * 100;
      break;
    case 5:
    case 6:
      tm->tv_usec = mi_sint3korr(ptr + 4);
  }
}

/**
  Convert in-memory timestamp representation to on-disk representation.

  @param        tm   The value to convert.
  @param [out]  ptr  The pointer to store the value to.
  @param        dec  Precision.
*/
void my_timestamp_to_binary(const struct timeval *tm, uchar *ptr, uint dec) {
  assert(dec <= DATETIME_MAX_DECIMALS);
  /* Stored value must have been previously properly rounded or truncated */
  assert((tm->tv_usec %
          static_cast<int>(log_10_int[DATETIME_MAX_DECIMALS - dec])) == 0);
  mi_int4store(ptr, tm->tv_sec);
  switch (dec) {
    case 0:
    default:
      break;
    case 1:
    case 2:
      ptr[4] =
          static_cast<unsigned char>(static_cast<char>(tm->tv_usec / 10000));
      break;
    case 3:
    case 4:
      mi_int2store(ptr + 4, tm->tv_usec / 100);
      break;
      /* Impossible second precision. Fall through */
    case 5:
    case 6:
      mi_int3store(ptr + 4, tm->tv_usec);
  }
}
/**
  Convert in-memory date representation to on-disk representation.

  @param        ltime The value to convert.
  @param [out]  ptr   The pointer to store the value to.
*/
void my_date_to_binary(const MYSQL_TIME *ltime, uchar *ptr) {
  long tmp = ltime->day + ltime->month * 32 + ltime->year * 16 * 32;
  int3store(ptr, tmp);
}

/**
  Convert a temporal value to packed numeric temporal representation,
  depending on its time_type.

  @param my_time   The value to convert.
  @return  Packed numeric time/date/datetime representation.
*/
longlong TIME_to_longlong_packed(const MYSQL_TIME &my_time) {
  switch (my_time.time_type) {
    case MYSQL_TIMESTAMP_DATE:
      return TIME_to_longlong_date_packed(my_time);
    case MYSQL_TIMESTAMP_DATETIME_TZ:
      assert(false);  // Should not be this type at this point.
    case MYSQL_TIMESTAMP_DATETIME:
      return TIME_to_longlong_datetime_packed(my_time);
    case MYSQL_TIMESTAMP_TIME:
      return TIME_to_longlong_time_packed(my_time);
    case MYSQL_TIMESTAMP_NONE:
    case MYSQL_TIMESTAMP_ERROR:
      return 0;
  }
  assert(false);
  return 0;
}

/**
    Change a daynr to year, month and day. Daynr 0 is returned as date
    00.00.00
*/
void get_date_from_daynr(long daynr, uint *ret_year, uint *ret_month,
                         uint *ret_day) {
  uint year;
  uint temp;
  uint leap_day;
  uint day_of_year;
  uint days_in_year;
  const uchar *month_pos;

  if (daynr <= 365L || daynr >= 3652500) { /* Fix if wrong daynr */
    *ret_year = *ret_month = *ret_day = 0;
  } else {
    year = static_cast<uint>(daynr * 100 / 36525L);
    temp = (((year - 1) / 100 + 1) * 3) / 4;
    day_of_year = static_cast<uint>(daynr - static_cast<long>(year) * 365L) -
                  (year - 1) / 4 + temp;
    while (day_of_year > (days_in_year = calc_days_in_year(year))) {
      day_of_year -= days_in_year;
      (year)++;
    }
    leap_day = 0;
    if (days_in_year == 366) {
      if (day_of_year > 31 + 28) {
        day_of_year--;
        if (day_of_year == 31 + 28) leap_day = 1; /* Handle leapyears leapday */
      }
    }
    *ret_month = 1;
    for (month_pos = days_in_month; day_of_year > static_cast<uint>(*month_pos);
         day_of_year -= *(month_pos++), (*ret_month)++)
      ;
    *ret_year = year;
    *ret_day = day_of_year + leap_day;
  }
}

/**
   Calc weekday from daynr.

   @retval 0 for Monday
   @retval 6 for Sunday
*/
int calc_weekday(long daynr, bool sunday_first_day_of_week) {
  return (static_cast<int>((daynr + 5L + (sunday_first_day_of_week ? 1L : 0L)) %
                           7));
}

/**
  Calculate the week number from a MYSQL_TIME value.

  The bits in week_format has the following meaning:
   WEEK_MONDAY_FIRST (0)  If not set	Sunday is first day of week
                          If set	Monday is first day of week
   WEEK_YEAR (1)	  If not set	Week is in range 0-53

        Week 0 is returned for the the last week of the previous year (for
        a date at start of january) In this case one can get 53 for the
        first week of next year.  This flag ensures that the week is
        relevant for the given year. Note that this flag is only
        releveant if WEEK_JANUARY is not set.

                          If set	 Week is in range 1-53.

        In this case one may get week 53 for a date in January (when
        the week is that last week of previous year) and week 1 for a
        date in December.

  WEEK_FIRST_WEEKDAY (2)  If not set	Weeks are numbered according
                                        to ISO 8601:1988
                          If set	The week that contains the first
                                        'first-day-of-week' is week 1.

        ISO 8601:1988 means that if the week containing January 1 has
        four or more days in the new year, then it is week 1;
        Otherwise it is the last week of the previous year, and the
        next week is week 1.

   @param my_time         Source time value
   @param week_behaviour  Parameter controlling how weeks are counted
   @param[out] year       The year of the week number (which may be different
                          from my_time.year as descibed above)

   @return week number
*/
uint calc_week(const MYSQL_TIME &my_time, uint week_behaviour, uint *year) {
  uint days;
  ulong daynr = calc_daynr(my_time.year, my_time.month, my_time.day);
  ulong first_daynr = calc_daynr(my_time.year, 1, 1);
  bool monday_first = (week_behaviour & WEEK_MONDAY_FIRST);
  bool week_year = (week_behaviour & WEEK_YEAR);
  bool first_weekday = (week_behaviour & WEEK_FIRST_WEEKDAY);

  uint weekday = calc_weekday(first_daynr, !monday_first);
  *year = my_time.year;

  if (my_time.month == 1 && my_time.day <= 7 - weekday) {
    if (!week_year &&
        ((first_weekday && weekday != 0) || (!first_weekday && weekday >= 4)))
      return 0;
    week_year = true;
    (*year)--;
    first_daynr -= (days = calc_days_in_year(*year));
    weekday = (weekday + 53 * 7 - days) % 7;
  }

  if ((first_weekday && weekday != 0) || (!first_weekday && weekday >= 4))
    days = daynr - (first_daynr + (7 - weekday));
  else
    days = daynr - (first_daynr - weekday);

  if (week_year && days >= 52 * 7) {
    weekday = (weekday + calc_days_in_year(*year)) % 7;
    if ((!first_weekday && weekday < 4) || (first_weekday && weekday == 0)) {
      (*year)++;
      return 1;
    }
  }
  return days / 7 + 1;
}

/**
   Predicate for the validity of a period.

   @param period
   @retval true if ?
   @retval false if ?
 */
bool valid_period(long long period) {
  if (period <= 0) return false;
  if ((period % 100) == 0) return false;
  if ((period % 100) > 12) return false;
  return true;
}

/**
   Calculate month from period.

   @return month
 */
ulong convert_period_to_month(ulong period) {
  ulong a;
  ulong b;
  if (period == 0) return 0L;
  if ((a = period / 100) < YY_PART_YEAR)
    a += 2000;
  else if (a < 100)
    a += 1900;
  b = period % 100;
  return a * 12 + b - 1;
}

/**
   Convert month to period.

   @return period
 */
ulong convert_month_to_period(ulong month) {
  ulong year;
  if (month == 0L) return 0L;
  if ((year = month / 12) < 100) {
    year += (year < YY_PART_YEAR) ? 2000 : 1900;
  }
  return year * 100 + month % 12 + 1;
}

/** Daynumber from year 0 to 9999-12-31 */
#define MAX_DAY_NUMBER 3652424UL

/**
   Add an interval to a MYSQL_TIME struct.

   @retval true if error
   @retval false otherwise
 */
bool date_add_interval(MYSQL_TIME *ltime, interval_type int_type,
                       Interval interval, int *warnings) {
  ltime->neg = false;

  long long sign = (interval.neg ? -1 : 1);

  switch (int_type) {
    case INTERVAL_SECOND:
    case INTERVAL_SECOND_MICROSECOND:
    case INTERVAL_MICROSECOND:
    case INTERVAL_MINUTE:
    case INTERVAL_HOUR:
    case INTERVAL_MINUTE_MICROSECOND:
    case INTERVAL_MINUTE_SECOND:
    case INTERVAL_HOUR_MICROSECOND:
    case INTERVAL_HOUR_SECOND:
    case INTERVAL_HOUR_MINUTE:
    case INTERVAL_DAY_MICROSECOND:
    case INTERVAL_DAY_SECOND:
    case INTERVAL_DAY_MINUTE:
    case INTERVAL_DAY_HOUR: {
      longlong sec, days, daynr, microseconds, extra_sec;
      ltime->time_type = MYSQL_TIMESTAMP_DATETIME;  // Return full date
      microseconds = ltime->second_part + sign * interval.second_part;
      extra_sec = microseconds / 1000000L;
      microseconds = microseconds % 1000000L;

      if (interval.day > MAX_DAY_NUMBER) goto invalid_date;
      if (interval.hour > MAX_DAY_NUMBER * 24ULL) goto invalid_date;
      if (interval.minute > MAX_DAY_NUMBER * 24ULL * 60ULL) goto invalid_date;
      if (interval.second > MAX_DAY_NUMBER * 24ULL * 60ULL * 60ULL)
        goto invalid_date;
      sec =
          ((ltime->day - 1) * 3600LL * 24LL + ltime->hour * 3600LL +
           ltime->minute * 60LL + ltime->second +
           sign * static_cast<longlong>(
                      interval.day * 3600ULL * 24ULL + interval.hour * 3600ULL +
                      interval.minute * 60ULL + interval.second)) +
          extra_sec;
      if (microseconds < 0) {
        microseconds += 1000000LL;
        sec--;
      }
      days = sec / (3600 * 24LL);
      sec -= days * 3600 * 24LL;
      if (sec < 0) {
        days--;
        sec += 3600 * 24LL;
      }
      ltime->second_part = static_cast<uint>(microseconds);
      ltime->second = static_cast<uint>(sec % 60);
      ltime->minute = static_cast<uint>(sec / 60 % 60);
      ltime->hour = static_cast<uint>(sec / 3600);
      daynr = calc_daynr(ltime->year, ltime->month, 1) + days;
      /* Day number from year 0 to 9999-12-31 */
      if (static_cast<ulonglong>(daynr) > MAX_DAY_NUMBER) goto invalid_date;
      get_date_from_daynr(static_cast<long>(daynr), &ltime->year, &ltime->month,
                          &ltime->day);
      break;
    }
    case INTERVAL_DAY:
    case INTERVAL_WEEK: {
      unsigned long period;
      period = calc_daynr(ltime->year, ltime->month, ltime->day);
      if (interval.neg) {
        if (period < interval.day)  // Before 0.
          goto invalid_date;
        period -= interval.day;
      } else {
        if (period + interval.day < period)  // Overflow.
          goto invalid_date;
        if (period + interval.day > MAX_DAY_NUMBER)  // After 9999-12-31.
          goto invalid_date;
        period += interval.day;
      }
      get_date_from_daynr(static_cast<long>(period), &ltime->year,
                          &ltime->month, &ltime->day);
    } break;
    case INTERVAL_YEAR:
      if (interval.year > 10000UL) goto invalid_date;
      ltime->year += sign * static_cast<long>(interval.year);
      if (static_cast<ulong>(ltime->year) >= 10000L) goto invalid_date;
      if (ltime->month == 2 && ltime->day == 29 &&
          calc_days_in_year(ltime->year) != 366)
        ltime->day = 28;  // Was leap-year
      break;
    case INTERVAL_YEAR_MONTH:
    case INTERVAL_QUARTER:
    case INTERVAL_MONTH: {
      unsigned long long period;

      // Simple guards against arithmetic overflow when calculating period.
      if (interval.month >= UINT_MAX / 2) goto invalid_date;
      if (interval.year >= UINT_MAX / 12) goto invalid_date;

      period = (ltime->year * 12ULL +
                sign * static_cast<unsigned long long>(interval.year) * 12ULL +
                ltime->month - 1ULL +
                sign * static_cast<unsigned long long>(interval.month));
      if (period >= 120000LL) goto invalid_date;
      ltime->year = period / 12;
      ltime->month = (period % 12L) + 1;
      /* Adjust day if the new month doesn't have enough days */
      if (ltime->day > days_in_month[ltime->month - 1]) {
        ltime->day = days_in_month[ltime->month - 1];
        if (ltime->month == 2 && calc_days_in_year(ltime->year) == 366)
          ltime->day++;  // Leap-year
      }
    } break;
    default:
      fprintf(stderr, "Unexpected interval type: %u\n",
              static_cast<unsigned int>(int_type));
      assert(false);
      goto null_date;
  }

  return false;  // Ok

invalid_date:
  if (warnings) {
    *warnings |= MYSQL_TIME_WARN_DATETIME_OVERFLOW;
  }

null_date:

  return true;
}

/**
  Add nanoseconds to a time value with truncation.

  @param [in,out] ltime        MYSQL_TIME variable to add to.
  @param          nanoseconds  Nanoseconds value.
  @param [in,out] warnings     Warning flag vector.
  @retval                      False on success. No real failure case here.
*/
bool time_add_nanoseconds_with_truncate(MYSQL_TIME *ltime, uint nanoseconds,
                                        int *warnings) {
  /*
    If second_part is not set then only add nanoseconds to it.
    If second_part is already set and then nanoseconds just represent
    additional numbers which help rounding, so we can ignore them.
  */
  if (ltime->second_part == 0) ltime->second_part = nanoseconds / 1000;

  adjust_time_range(ltime, warnings);
  return false;
}

/**
   Add nanoseconds to a datetime value with truncation.

   @param [in,out] ltime        MYSQL_TIME variable to add to.
   @param          nanoseconds  Nanoseconds value.
   @retval                      False on success. No real failure case here.
*/
bool datetime_add_nanoseconds_with_truncate(MYSQL_TIME *ltime,
                                            uint nanoseconds) {
  /*
    If second_part is not set then only add nanoseconds to it.
    If second_part is already set and then nanoseconds just represent
    additional numbers which help rounding, so we can ignore them.
  */
  if (ltime->second_part == 0) ltime->second_part = nanoseconds / 1000;
  return false;
}

/**
  Add nanoseconds to a time value with rounding.

  @param [in,out] ltime        MYSQL_TIME variable to add to.
  @param          nanoseconds  Nanoseconds value.
  @param [in,out] warnings     Warning flag vector.
  @retval                      False on success, true on error.
*/
bool time_add_nanoseconds_with_round(MYSQL_TIME *ltime, uint nanoseconds,
                                     int *warnings) {
  /* We expect correct input data */
  assert(nanoseconds < 1000000000);
  assert(!check_time_mmssff_range(*ltime));

  if (nanoseconds < 500) return false;

  ltime->second_part += (nanoseconds + 500) / 1000;
  if (ltime->second_part < 1000000) goto ret;

  ltime->second_part %= 1000000;
  if (ltime->second < 59) {
    ltime->second++;
    goto ret;
  }

  ltime->second = 0;
  if (ltime->minute < 59) {
    ltime->minute++;
    goto ret;
  }
  ltime->minute = 0;
  ltime->hour++;

ret:
  /*
    We can get '838:59:59.000001' at this point, which
    is bigger than the maximum possible value '838:59:59.000000'.
    Checking only "hour > 838" is not enough.
    Do full adjust_time_range().
  */
  adjust_time_range(ltime, warnings);
  return false;
}

/**
  Add nanoseconds to a datetime value with rounding.

  @param [in,out] ltime        MYSQL_TIME variable to add to.
  @param          nanoseconds  Nanoseconds value.
  @param [in,out] warnings     Warning flag vector.
  @retval                      False on success, true on error.
*/
bool datetime_add_nanoseconds_with_round(MYSQL_TIME *ltime, uint nanoseconds,
                                         int *warnings) {
  assert(nanoseconds < 1000000000);
  if (nanoseconds < 500) return false;

  ltime->second_part += (nanoseconds + 500) / 1000;
  if (ltime->second_part < 1000000) return false;

  ltime->second_part %= 1000000;
  Interval interval;
  memset(&interval, 0, sizeof(interval));
  interval.second = 1;
  /* date_add_interval cannot handle bad dates */
  if (check_date(*ltime, non_zero_date(*ltime),
                 (TIME_NO_ZERO_IN_DATE | TIME_NO_ZERO_DATE), warnings))
    return true;

  if (date_add_interval(ltime, INTERVAL_SECOND, interval, warnings)) {
    *warnings |= MYSQL_TIME_WARN_OUT_OF_RANGE;
    return true;
  }
  return false;
}

/**
  Add nanoseconds to time and round or tuncate as indicated by argument.

  @param [in,out] ltime        MYSQL_TIME variable to add to.
  @param          nanoseconds  Nanosecons value.
  @param [in,out] warnings     Warning flag vector.
  @param          truncate     Decides whether fractional part of seconds will
                               be truncated/rounded.
  @retval                      False on success. No real failure case here.
*/
bool time_add_nanoseconds_adjust_frac(MYSQL_TIME *ltime, uint nanoseconds,
                                      int *warnings, bool truncate) {
  if (truncate)
    return time_add_nanoseconds_with_truncate(ltime, nanoseconds, warnings);
  else
    return time_add_nanoseconds_with_round(ltime, nanoseconds, warnings);
}

/**
   Add nanoseconds to datetime and round or tuncate as indicated by argument.

  @param [in,out] ltime        MYSQL_TIME variable to add to.
  @param          nanoseconds  Nanoseconds value.
  @param [in,out] warnings     Warning flag vector.
  @param          truncate     Decides whether fractional part of seconds will
                               be truncated/rounded.
  @retval                      False on success, true on error.
*/
bool datetime_add_nanoseconds_adjust_frac(MYSQL_TIME *ltime, uint nanoseconds,
                                          int *warnings, bool truncate) {
  if (truncate)
    return datetime_add_nanoseconds_with_truncate(ltime, nanoseconds);
  else
    return datetime_add_nanoseconds_with_round(ltime, nanoseconds, warnings);
}

/** Rounding functions */
static constexpr const uint msec_round_add[7] = {
    500000000, 50000000, 5000000, 500000, 50000, 5000, 0};

/**
  Round/Truncate time value to the given precision.

  @param [in,out]  ltime    The value to round.
  @param           dec      Precision.
  @param           truncate Decides whether fractional part of seconds will be
                            truncated/rounded.
  @return                   False on success, true on error.
*/
bool my_time_adjust_frac(MYSQL_TIME *ltime, uint dec, bool truncate) {
  int warnings = 0;
  assert(dec <= DATETIME_MAX_DECIMALS);
  /* Add half away from zero */
  bool rc = time_add_nanoseconds_adjust_frac(ltime, msec_round_add[dec],
                                             &warnings, truncate);

  /* Truncate non-significant digits */
  my_time_trunc(ltime, dec);
  return rc;
}

/**
  Round/Truncate datetime value to the given precision.

  @param [in,out]  ltime    The value to round.
  @param           dec      Precision.
  @param [in,out]  warnings Warning flag vector
  @param           truncate Decides whether fractional part of seconds will be
                            truncated/rounded.
  @return                   False on success, true on error.
*/
bool my_datetime_adjust_frac(MYSQL_TIME *ltime, uint dec, int *warnings,
                             bool truncate) {
  assert(dec <= DATETIME_MAX_DECIMALS);
  /* Add half away from zero */
  bool rc = datetime_add_nanoseconds_adjust_frac(ltime, msec_round_add[dec],
                                                 warnings, truncate);
  /* Truncate non-significant digits */
  my_time_trunc(ltime, dec);
  return rc;
}

/**
  Round timeval value to the given precision.

  @param [in,out]  tv       The value to round.
  @param           decimals Precision.
  @return                   False on success, true on error.
*/
bool my_timeval_round(struct timeval *tv, uint decimals) {
  assert(decimals <= DATETIME_MAX_DECIMALS);
  uint nanoseconds = msec_round_add[decimals];
  tv->tv_usec += (nanoseconds + 500) / 1000;
  if (tv->tv_usec < 1000000) goto ret;

  tv->tv_usec = 0;
  tv->tv_sec++;
  if (!is_time_t_valid_for_timestamp(tv->tv_sec)) {
    tv->tv_sec = TIMESTAMP_MAX_VALUE;
    return true;
  }

ret:
  my_timeval_trunc(tv, decimals);
  return false;
}

/**
  Mix a date value and a time value.

  @param [in,out] ldate    Date value.
  @param          my_time  Time value.
*/
void mix_date_and_time(MYSQL_TIME *ldate, const MYSQL_TIME &my_time) {
  assert(ldate->time_type == MYSQL_TIMESTAMP_DATE ||
         ldate->time_type == MYSQL_TIMESTAMP_DATETIME);

  if (!my_time.neg && my_time.hour < 24) {
    /*
      Simple case: TIME is within normal 24 hours internal.
      Mix DATE part of ltime2 and TIME part of ltime together.
    */
    ldate->hour = my_time.hour;
    ldate->minute = my_time.minute;
    ldate->second = my_time.second;
    ldate->second_part = my_time.second_part;
  } else {
    /* Complex case: TIME is negative or outside of 24 hours internal. */
    longlong seconds;
    long days, useconds;
    int sign = my_time.neg ? 1 : -1;
    ldate->neg = calc_time_diff(*ldate, my_time, sign, &seconds, &useconds);
    assert(!ldate->neg);

    /*
      We pass current date to mix_date_and_time. If we want to use
      this function with arbitrary dates, this code will need
      to cover cases when ltime is negative and "ldate < -ltime".
    */
    assert(ldate->year > 0);

    days = static_cast<long>(seconds / SECONDS_IN_24H);
    calc_time_from_sec(ldate, seconds % SECONDS_IN_24H, useconds);
    get_date_from_daynr(days, &ldate->year, &ldate->month, &ldate->day);
  }
  ldate->time_type = MYSQL_TIMESTAMP_DATETIME;
}

/**
   Converts a timepoint in a posix tm struct to a MYSQL_TIME struct.

   @param [out] to store converted timepoint here
   @param from posix tm struct holding a valid timepoint
 */
void localtime_to_TIME(MYSQL_TIME *to, const struct tm *from) {
  to->neg = false;
  to->second_part = 0;
  to->year = ((from->tm_year + 1900) % 10000);
  to->month = from->tm_mon + 1;
  to->day = from->tm_mday;
  to->hour = from->tm_hour;
  to->minute = from->tm_min;
  to->second = from->tm_sec;
  to->time_zone_displacement = 0;
}

/**
   Initialize MYSQL_TIME with MYSQL_TIMESTAMP_TIME from given number
   of seconds and microseconds.
 */
void calc_time_from_sec(MYSQL_TIME *to, longlong seconds, long microseconds) {
  long t_seconds;
  // to->neg is not cleared, it may already be set to a useful value
  to->time_type = MYSQL_TIMESTAMP_TIME;
  to->year = 0;
  to->month = 0;
  to->day = 0;
  assert(seconds < (0xFFFFFFFFLL * 3600LL));
  to->hour = static_cast<long>(seconds / 3600L);
  t_seconds = static_cast<long>(seconds % 3600L);
  to->minute = t_seconds / 60L;
  to->second = t_seconds % 60L;
  to->second_part = microseconds;
}

/**
  Calculate difference between two datetime values as seconds + microseconds.

  @param my_time1         - TIME/DATE/DATETIME value
  @param my_time2         - TIME/DATE/DATETIME value
  @param l_sign           - 1 absolute values are substracted, -1 absolute
  values are added.
  @param[out] seconds_out - where difference between my_time1 and my_time2
                            in seconds is stored.
  @param[out] microseconds_out - where microsecond part of difference between
                                 my_time1 and my_time2 is stored.

  @note This function calculates difference between my_time1 and
    my_time2 absolute values. So one should set l_sign and correct
    result if he want to take signs into account (i.e. for MYSQL_TIME
    values).

  @returns Sign of difference.
    @retval 1 means negative result
    @retval 0 means positive result
*/
bool calc_time_diff(const MYSQL_TIME &my_time1, const MYSQL_TIME &my_time2,
                    int l_sign, longlong *seconds_out, long *microseconds_out) {
  long days;
  bool neg;
  longlong microseconds;

  /*
    We suppose that if first argument is MYSQL_TIMESTAMP_TIME
    the second argument should be TIMESTAMP_TIME also.
    We should check it before calc_time_diff call.
  */
  if (my_time1.time_type == MYSQL_TIMESTAMP_TIME)  // Time value
    days = static_cast<long>(my_time1.day) -
           l_sign * static_cast<long>(my_time2.day);
  else {
    days = calc_daynr(static_cast<uint>(my_time1.year),
                      static_cast<uint>(my_time1.month),
                      static_cast<uint>(my_time1.day));
    if (my_time2.time_type == MYSQL_TIMESTAMP_TIME)
      days -= l_sign * static_cast<long>(my_time2.day);
    else
      days -= l_sign * calc_daynr(static_cast<uint>(my_time2.year),
                                  static_cast<uint>(my_time2.month),
                                  static_cast<uint>(my_time2.day));
  }

  microseconds =
      (static_cast<longlong>(days) * SECONDS_IN_24H +
       static_cast<longlong>(my_time1.hour * 3600L + my_time1.minute * 60L +
                             my_time1.second) -
       l_sign *
           static_cast<longlong>(my_time2.hour * 3600L + my_time2.minute * 60L +
                                 my_time2.second)) *
          1000000LL +
      static_cast<longlong>(my_time1.second_part) -
      l_sign * static_cast<longlong>(my_time2.second_part);

  neg = false;
  if (microseconds < 0) {
    microseconds = -microseconds;
    neg = true;
  }
  *seconds_out = microseconds / 1000000L;
  *microseconds_out = static_cast<long>(microseconds % 1000000L);
  return neg;
}

/**
   Compare tow MYSQL_TIME objects.

   @retval 0 if a and b are equal
   @retval -1 if a comes before b
   @retval 1 if b comes before a
 */
int my_time_compare(const MYSQL_TIME &my_time_a, const MYSQL_TIME &my_time_b) {
  ulonglong a_t = TIME_to_ulonglong_datetime(my_time_a);
  ulonglong b_t = TIME_to_ulonglong_datetime(my_time_b);

  if (a_t < b_t) return -1;
  if (a_t > b_t) return 1;

  if (my_time_a.second_part < my_time_b.second_part) return -1;
  if (my_time_a.second_part > my_time_b.second_part) return 1;

  return 0;
}

/**
  Convert MYSQL_TIME value to its packed numeric representation,
  using field type.

  @param my_time The time value to convert.
  @param type    MySQL field type.
  @return        Packed numeric representation.
*/
longlong TIME_to_longlong_packed(const MYSQL_TIME &my_time,
                                 enum enum_field_types type) {
  switch (type) {
    case MYSQL_TYPE_TIME:
      return TIME_to_longlong_time_packed(my_time);
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TIMESTAMP:
      return TIME_to_longlong_datetime_packed(my_time);
    case MYSQL_TYPE_DATE:
      return TIME_to_longlong_date_packed(my_time);
    default:
      return TIME_to_longlong_packed(my_time);
  }
}

/**
  Convert packed numeric temporal representation to time, date or datetime,
  using field type.

  @param[out] ltime        The variable to write to.
  @param      type         MySQL field type.
  @param      packed_value Numeric datetype representation.
*/
void TIME_from_longlong_packed(MYSQL_TIME *ltime, enum enum_field_types type,
                               longlong packed_value) {
  switch (type) {
    case MYSQL_TYPE_TIME:
      TIME_from_longlong_time_packed(ltime, packed_value);
      break;
    case MYSQL_TYPE_DATE:
      TIME_from_longlong_date_packed(ltime, packed_value);
      break;
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TIMESTAMP:
      TIME_from_longlong_datetime_packed(ltime, packed_value);
      break;
    default:
      assert(false);
      set_zero_time(ltime, MYSQL_TIMESTAMP_ERROR);
      break;
  }
}

/**
  Convert packed numeric representation to
  unpacked numeric representation.

  @param type           MySQL field type.
  @param packed_value   Packed numeric temporal value.
  @return               Number in one of the following formats,
                        depending on type: YYMMDD, YYMMDDhhmmss, hhmmss.
*/
longlong longlong_from_datetime_packed(enum enum_field_types type,
                                       longlong packed_value) {
  MYSQL_TIME ltime;
  switch (type) {
    case MYSQL_TYPE_TIME:
      TIME_from_longlong_time_packed(&ltime, packed_value);
      return TIME_to_ulonglong_time(ltime);
    case MYSQL_TYPE_DATE:
      TIME_from_longlong_date_packed(&ltime, packed_value);
      return TIME_to_ulonglong_date(ltime);
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TIMESTAMP:
      TIME_from_longlong_datetime_packed(&ltime, packed_value);
      return TIME_to_ulonglong_datetime(ltime);
    default:
      assert(false);
      return 0;
  }
}

/**
  Convert packed numeric temporal representation to unpacked numeric
  representation.

  @param type           MySQL field type.
  @param packed_value   Numeric packed temporal representation.
  @return               A double value in on of the following formats,
                        depending  on type:
                        YYYYMMDD, hhmmss.ffffff or YYMMDDhhmmss.ffffff.
*/
double double_from_datetime_packed(enum enum_field_types type,
                                   longlong packed_value) {
  longlong result = longlong_from_datetime_packed(type, packed_value);
  return result +
         (static_cast<double>(my_packed_time_get_frac_part(packed_value))) /
             1000000;
}

/**
   @} (end of defgroup MY_TIME)
*/

// Non-static driver functions for unit tests
namespace mysys_my_time {
longlong DRV_my_packed_time_get_int_part(longlong i) {
  return my_packed_time_get_int_part(i);
}
longlong DRV_my_packed_time_make(longlong i, longlong f) {
  return my_packed_time_make(i, f);
}
longlong DRV_my_packed_time_make_int(longlong i) {
  return my_packed_time_make_int(i);
}
}  // namespace mysys_my_time
