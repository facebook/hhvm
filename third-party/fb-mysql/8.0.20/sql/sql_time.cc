/* Copyright (c) 2000, 2020, Oracle and/or its affiliates. All rights reserved.

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

/**
  @defgroup SQL_TIME Server time functions
  @ingroup Runtime_Environment
  @{

  @file sql/sql_time.cc

  Implementation of server functions to handle date and time.

*/

#include "sql/sql_time.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "decimal.h"
#include "m_ctype.h"
#include "m_string.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_macros.h"
#include "mysql_com.h"
#include "mysqld_error.h"
#include "sql/current_thd.h"
#include "sql/derror.h"
#include "sql/field.h"
#include "sql/my_decimal.h"
#include "sql/sql_class.h"  // THD, MODE_STRICT_ALL_TABLES, MODE_STRICT_TRANS_TABLES
#include "sql/sql_const.h"
#include "sql/system_variables.h"
#include "sql/tztime.h"  // struct Time_zone

/**
  Name description of interval names used in statements.

  'interval_type_to_name' is ordered and sorted on interval size and
  interval complexity.
  Order of elements in 'interval_type_to_name' should correspond to
  the order of elements in 'interval_type' enum

  @see interval_type, interval_names
*/
const LEX_CSTRING interval_type_to_name[INTERVAL_LAST] = {
    {STRING_WITH_LEN("YEAR")},
    {STRING_WITH_LEN("QUARTER")},
    {STRING_WITH_LEN("MONTH")},
    {STRING_WITH_LEN("WEEK")},
    {STRING_WITH_LEN("DAY")},
    {STRING_WITH_LEN("HOUR")},
    {STRING_WITH_LEN("MINUTE")},
    {STRING_WITH_LEN("SECOND")},
    {STRING_WITH_LEN("MICROSECOND")},
    {STRING_WITH_LEN("YEAR_MONTH")},
    {STRING_WITH_LEN("DAY_HOUR")},
    {STRING_WITH_LEN("DAY_MINUTE")},
    {STRING_WITH_LEN("DAY_SECOND")},
    {STRING_WITH_LEN("HOUR_MINUTE")},
    {STRING_WITH_LEN("HOUR_SECOND")},
    {STRING_WITH_LEN("MINUTE_SECOND")},
    {STRING_WITH_LEN("DAY_MICROSECOND")},
    {STRING_WITH_LEN("HOUR_MICROSECOND")},
    {STRING_WITH_LEN("MINUTE_MICROSECOND")},
    {STRING_WITH_LEN("SECOND_MICROSECOND")}};

/**
  Convert a string to 8-bit representation,
  for use in str_to_time/str_to_date/str_to_date.

  In the future to_ascii() can be extended to convert
  non-ASCII digits to ASCII digits
  (for example, ARABIC-INDIC, DEVANAGARI, BENGALI, and so on)
  so DATE/TIME/DATETIME values understand digits in the
  respected scripts.

  @return number of bytes written to dst
*/
static uint to_ascii(const CHARSET_INFO *cs, const char *src, size_t src_length,
                     char *dst, size_t dst_length) {
  int cnvres;
  my_wc_t wc;
  const char *srcend = src + src_length;
  char *dst0 = dst;
  char *dstend = dst + dst_length - 1;
  while (dst < dstend &&
         (cnvres = (cs->cset->mb_wc)(cs, &wc, pointer_cast<const uchar *>(src),
                                     pointer_cast<const uchar *>(srcend))) >
             0 &&
         wc < 128) {
    src += cnvres;
    *dst++ = static_cast<char>(wc);
  }
  *dst = '\0';
  return dst - dst0;
}

/**
   Character set-aware version of str_to_time().

   @return False on success, true on error.
*/
bool str_to_time(const CHARSET_INFO *cs, const char *str, size_t length,
                 MYSQL_TIME *l_time, my_time_flags_t flags,
                 MYSQL_TIME_STATUS *status) {
  char cnv[MAX_TIME_FULL_WIDTH + 3];  // +3 for nanoseconds (for rounding)
  if ((cs->state & MY_CS_NONASCII) != 0) {
    length = to_ascii(cs, str, length, cnv, sizeof(cnv));
    str = cnv;
  }

  bool rc = str_to_time(str, length, l_time, status);
  rc = rc || time_add_nanoseconds_adjust_frac(l_time, status->nanoseconds,
                                              &status->warnings,
                                              (flags & TIME_FRAC_TRUNCATE));
  return rc;
}

/**
   Character set-aware version of str_to_datetime().

   @return False on success, true on error.
*/
bool str_to_datetime(const CHARSET_INFO *cs, const char *str, size_t length,
                     MYSQL_TIME *l_time, my_time_flags_t flags,
                     MYSQL_TIME_STATUS *status) {
  char cnv[MAX_DATETIME_FULL_WIDTH + 3];  // +3 for nanoseconds (for rounding)
  if ((cs->state & MY_CS_NONASCII) != 0) {
    length = to_ascii(cs, str, length, cnv, sizeof(cnv));
    str = cnv;
  }

  bool rc = str_to_datetime(str, length, l_time, flags, status);
  rc = rc || datetime_add_nanoseconds_adjust_frac(l_time, status->nanoseconds,
                                                  &status->warnings,
                                                  flags & TIME_FRAC_TRUNCATE);
  return rc;
}

/**
  Convert a timestamp string to a MYSQL_TIME value and produce a warning
  if string was truncated during conversion.

  @note See description of str_to_datetime() for more information.
  @note Uses current_thd

  @return False on success, true on error.
*/
bool str_to_datetime_with_warn(String *str, MYSQL_TIME *l_time,
                               my_time_flags_t flags) {
  MYSQL_TIME_STATUS status;
  THD *thd = current_thd;
  if (thd->variables.sql_mode & MODE_NO_ZERO_DATE) flags |= TIME_NO_ZERO_DATE;
  if (thd->variables.sql_mode & MODE_INVALID_DATES) flags |= TIME_INVALID_DATES;
  if (thd->is_fsp_truncate_mode()) flags |= TIME_FRAC_TRUNCATE;
  bool ret_val = propagate_datetime_overflow(
      thd, &status.warnings, str_to_datetime(str, l_time, flags, &status));
  if (ret_val || status.warnings) {
    if (make_truncated_value_warning(current_thd, Sql_condition::SL_WARNING,
                                     ErrConvString(str), l_time->time_type,
                                     NullS))
      return true;
  }

  adjust_time_zone_displacement(thd->time_zone(), l_time);

  return ret_val;
}

/**
  Convert lldiv_t to datetime.

  @param         lld      The value to convert from.
  @param[out]    ltime    The variable to convert to.
  @param         flags    Conversion flags.
  @param[in,out] warnings Warning flags.

  @return False on success, true on error.
*/
static bool lldiv_t_to_datetime(lldiv_t lld, MYSQL_TIME *ltime,
                                my_time_flags_t flags, int *warnings) {
  if (lld.rem < 0 ||  // Catch negative numbers with zero int part, e.g: -0.1
      number_to_datetime(lld.quot, ltime, flags, warnings) == -1LL) {
    /* number_to_datetime does not clear ltime in case of ZERO DATE */
    set_zero_time(ltime, MYSQL_TIMESTAMP_ERROR);
    if (!*warnings) /* Neither sets warnings in case of ZERO DATE */
      *warnings |= MYSQL_TIME_WARN_TRUNCATED;
    return true;
  } else if (ltime->time_type == MYSQL_TIMESTAMP_DATE) {
    /*
      Generate a warning in case of DATE with fractional part:
        20011231.1234 -> '2001-12-31'
      unless the caller does not want the warning: for example, CAST does.
    */
    if (lld.rem && !(flags & TIME_NO_DATE_FRAC_WARN))
      *warnings |= MYSQL_TIME_WARN_TRUNCATED;
  } else {
    ltime->second_part = static_cast<ulong>(lld.rem / 1000);
    return datetime_add_nanoseconds_adjust_frac(ltime, lld.rem % 1000, warnings,
                                                (flags & TIME_FRAC_TRUNCATE));
  }
  return false;
}

/**
  Convert decimal value to datetime value with a warning.
  @param       decimal The value to convert from.
  @param[out]  ltime   The variable to convert to.
  @param       flags   Conversion flags.

  @return False on success, true on error.
*/
bool my_decimal_to_datetime_with_warn(const my_decimal *decimal,
                                      MYSQL_TIME *ltime,
                                      my_time_flags_t flags) {
  lldiv_t lld;
  int warnings = 0;
  bool rc;

  if ((rc = my_decimal2lldiv_t(0, decimal, &lld))) {
    warnings |= MYSQL_TIME_WARN_TRUNCATED;
    set_zero_time(ltime, MYSQL_TIMESTAMP_NONE);
  } else
    rc = propagate_datetime_overflow(
        current_thd, &warnings,
        lldiv_t_to_datetime(lld, ltime, flags, &warnings));

  if (warnings) {
    if (make_truncated_value_warning(current_thd, Sql_condition::SL_WARNING,
                                     ErrConvString(decimal), ltime->time_type,
                                     NullS))
      return true;
  }
  return rc;
}

/**
  Convert double value to datetime value with a warning.
  @param       nr      The value to convert from.
  @param[out]  ltime   The variable to convert to.
  @param       flags   Conversion flags.

  @return False on success, true on error.
*/
bool my_double_to_datetime_with_warn(double nr, MYSQL_TIME *ltime,
                                     my_time_flags_t flags) {
  lldiv_t lld;
  int warnings = 0;
  bool rc;

  if ((rc = (double2lldiv_t(nr, &lld) != E_DEC_OK))) {
    warnings |= MYSQL_TIME_WARN_TRUNCATED;
    set_zero_time(ltime, MYSQL_TIMESTAMP_NONE);
  } else
    rc = propagate_datetime_overflow(
        current_thd, &warnings,
        lldiv_t_to_datetime(lld, ltime, flags, &warnings));

  if (warnings) {
    if (make_truncated_value_warning(current_thd, Sql_condition::SL_WARNING,
                                     ErrConvString(nr), ltime->time_type,
                                     NullS))
      return true;
  }
  return rc;
}

/**
  Convert longlong value to datetime value with a warning.
  @param       nr      The value to convert from.
  @param[out]  ltime   The variable to convert to.
  @param       flags

  @return False on success, true on error.
*/
bool my_longlong_to_datetime_with_warn(longlong nr, MYSQL_TIME *ltime,
                                       my_time_flags_t flags) {
  int warnings = 0;
  bool rc = propagate_datetime_overflow(
                current_thd, &warnings,
                number_to_datetime(nr, ltime, flags, &warnings)) == -1LL;
  if (warnings) {
    if (make_truncated_value_warning(current_thd, Sql_condition::SL_WARNING,
                                     ErrConvString(nr), MYSQL_TIMESTAMP_NONE,
                                     NullS))
      return true;
  }
  return rc;
}

/**
  Convert lldiv_t value to time with nanosecond rounding.

  @param         lld      The value to convert from.
  @param[out]    ltime    The variable to convert to,
  @param[in,out] warnings Warning flags.

  @return False on success, true on error.
*/
static bool lldiv_t_to_time(lldiv_t lld, MYSQL_TIME *ltime, int *warnings) {
  if (number_to_time(lld.quot, ltime, warnings)) return true;
  /*
    Both lld.quot and lld.rem can give negative result value,
    thus combine them using "|=".
  */
  if ((ltime->neg |= (lld.rem < 0))) lld.rem = -lld.rem;
  ltime->second_part = static_cast<ulong>(lld.rem / 1000);
  return time_add_nanoseconds_adjust_frac(ltime, lld.rem % 1000, warnings,
                                          current_thd->is_fsp_truncate_mode());
}

/**
  Convert decimal number to TIME
  @param      decimal  The number to convert from.
  @param[out] ltime          The variable to convert to.

  @return False on success, true on error.
*/
bool my_decimal_to_time_with_warn(const my_decimal *decimal,
                                  MYSQL_TIME *ltime) {
  lldiv_t lld;
  int warnings = 0;
  bool rc;

  if ((rc = my_decimal2lldiv_t(0, decimal, &lld))) {
    warnings |= MYSQL_TIME_WARN_TRUNCATED;
    set_zero_time(ltime, MYSQL_TIMESTAMP_TIME);
  } else
    rc = propagate_datetime_overflow(current_thd, &warnings,
                                     lldiv_t_to_time(lld, ltime, &warnings));

  if (warnings) {
    if (make_truncated_value_warning(current_thd, Sql_condition::SL_WARNING,
                                     ErrConvString(decimal),
                                     MYSQL_TIMESTAMP_TIME, NullS))
      return true;
  }
  return rc;
}

/**
  Convert double number to TIME

  @param      nr      The number to convert from.
  @param[out] ltime   The variable to convert to.

  @return False on success, true on error.
*/
bool my_double_to_time_with_warn(double nr, MYSQL_TIME *ltime) {
  lldiv_t lld;
  int warnings = 0;
  bool rc;

  if ((rc = (double2lldiv_t(nr, &lld) != E_DEC_OK))) {
    warnings |= MYSQL_TIME_WARN_TRUNCATED;
    set_zero_time(ltime, MYSQL_TIMESTAMP_TIME);
  } else
    rc = propagate_datetime_overflow(current_thd, &warnings,
                                     lldiv_t_to_time(lld, ltime, &warnings));

  if (warnings) {
    if (make_truncated_value_warning(current_thd, Sql_condition::SL_WARNING,
                                     ErrConvString(nr), MYSQL_TIMESTAMP_TIME,
                                     NullS))
      return true;
  }
  return rc;
}

/**
  Convert longlong number to TIME
  @param      nr     The number to convert from.
  @param[out] ltime  The variable to convert to.

  @return False on success, true on error.
*/
bool my_longlong_to_time_with_warn(longlong nr, MYSQL_TIME *ltime) {
  int warnings = 0;
  bool rc = propagate_datetime_overflow(current_thd, &warnings,
                                        number_to_time(nr, ltime, &warnings));
  if (warnings) {
    if (make_truncated_value_warning(current_thd, Sql_condition::SL_WARNING,
                                     ErrConvString(nr), MYSQL_TIMESTAMP_TIME,
                                     NullS))
      return true;
  }
  return rc;
}

/**
  Convert a datetime from broken-down MYSQL_TIME representation
  to corresponding TIMESTAMP value.

  @param  thd             - current thread
  @param  t               - datetime in broken-down representation,
  @param  in_dst_time_gap - pointer to bool which is set to true if t represents
                            value which doesn't exists (falls into the spring
                            time-gap) or to false otherwise.

  @retval  Number seconds in UTC since start of Unix Epoch corresponding to t.
  @retval  0 - t contains datetime value which is out of TIMESTAMP range.
*/
my_time_t TIME_to_timestamp(THD *thd, const MYSQL_TIME *t,
                            bool *in_dst_time_gap) {
  my_time_t timestamp;

  *in_dst_time_gap = false;

  timestamp = thd->time_zone()->TIME_to_gmt_sec(t, in_dst_time_gap);
  if (timestamp) {
    return timestamp;
  }

  /* If we are here we have range error. */
  return (0);
}

/**
  Convert a datetime MYSQL_TIME representation
  to corresponding "struct timeval" value.

  ltime must previously be checked for TIME_NO_ZERO_IN_DATE.
  Things like '0000-01-01', '2000-00-01', '2000-01-00' are not allowed
  and asserted.

  Things like '0000-00-00 10:30:30' or '0000-00-00 00:00:00.123456'
  (i.e. empty date with non-empty time) return error.

  Zero datetime '0000-00-00 00:00:00.000000'
  is allowed and is mapper to {tv_sec=0, tv_usec=0}.

  Note: In case of error, tm value is not initialized.

  Note: "warnings" is not initialized to zero,
  so new warnings are added to the old ones.
  Caller must make sure to initialize "warnings".

  @param[in]  thd       current thd
  @param[in]  ltime     datetime value
  @param[out] tm        timeval value
  @param[out] warnings  pointer to warnings vector

  @return False on success, true on error.
*/
bool datetime_with_no_zero_in_date_to_timeval(THD *thd, const MYSQL_TIME *ltime,
                                              struct timeval *tm,
                                              int *warnings) {
  if (!ltime->month) /* Zero date */
  {
    DBUG_ASSERT(!ltime->year && !ltime->day);
    if (non_zero_time(*ltime)) {
      /*
        Return error for zero date with non-zero time, e.g.:
        '0000-00-00 10:20:30' or '0000-00-00 00:00:00.123456'
      */
      *warnings |= MYSQL_TIME_WARN_TRUNCATED;
      return true;
    }
    tm->tv_sec = tm->tv_usec = 0;  // '0000-00-00 00:00:00.000000'
    return false;
  }

  bool in_dst_time_gap;
  if (!(tm->tv_sec = TIME_to_timestamp(thd, ltime, &in_dst_time_gap))) {
    /*
      Date was outside of the supported timestamp range.
      For example: '3001-01-01 00:00:00' or '1000-01-01 00:00:00'
    */
    *warnings |= MYSQL_TIME_WARN_OUT_OF_RANGE;
    return true;
  } else if (in_dst_time_gap) {
    /*
      Set MYSQL_TIME_WARN_INVALID_TIMESTAMP warning to indicate
      that date was fine but pointed to winter/summer time switch gap.
      In this case tm is set to the fist second after gap.
      For example: '2003-03-30 02:30:00 MSK' -> '2003-03-30 03:00:00 MSK'
    */
    *warnings |= MYSQL_TIME_WARN_INVALID_TIMESTAMP;
  }
  tm->tv_usec = ltime->second_part;
  return false;
}

/**
  Convert a datetime MYSQL_TIME representation
  to corresponding "struct timeval" value.

  Things like '0000-01-01', '2000-00-01', '2000-01-00'
  (i.e. incomplete date) return error.

  Things like '0000-00-00 10:30:30' or '0000-00-00 00:00:00.123456'
  (i.e. empty date with non-empty time) return error.

  Zero datetime '0000-00-00 00:00:00.000000'
  is allowed and is mapper to {tv_sec=0, tv_usec=0}.

  Note: In case of error, tm value is not initialized.

  Note: "warnings" is not initialized to zero,
  so new warnings are added to the old ones.
  Caller must make sure to initialize "warnings".

  @param[in]  thd       current thd
  @param[in]  ltime     datetime value
  @param[out] tm        timeval value
  @param[out] warnings  pointer to warnings vector

  @return False on success, true on error.
*/
bool datetime_to_timeval(THD *thd, const MYSQL_TIME *ltime, struct timeval *tm,
                         int *warnings) {
  return check_date(*ltime, non_zero_date(*ltime), TIME_NO_ZERO_IN_DATE,
                    warnings) ||
         datetime_with_no_zero_in_date_to_timeval(thd, ltime, tm, warnings);
}

/**
  Convert a time string to a MYSQL_TIME struct and produce a warning
  if string was cut during conversion.

  @note See str_to_time() for more info.

  @return False on success, true on error.
*/
bool str_to_time_with_warn(String *str, MYSQL_TIME *l_time) {
  MYSQL_TIME_STATUS status;
  my_time_flags_t flags = 0;
  THD *thd = current_thd;

  if (current_thd->is_fsp_truncate_mode()) flags = TIME_FRAC_TRUNCATE;

  bool ret_val = propagate_datetime_overflow(
      current_thd, &status.warnings, str_to_time(str, l_time, flags, &status));
  if (ret_val || status.warnings) {
    if (make_truncated_value_warning(thd, Sql_condition::SL_WARNING,
                                     ErrConvString(str), MYSQL_TIMESTAMP_TIME,
                                     NullS))
      return true;
  }

  if (!ret_val) adjust_time_zone_displacement(thd->time_zone(), l_time);

  return ret_val;
}

/**
  Convert time to datetime.

  The time value is added to the current datetime value.
  @param thd
  @param [in] ltime    Time value to convert from.
  @param [out] ltime2   Datetime value to convert to.
*/
void time_to_datetime(THD *thd, const MYSQL_TIME *ltime, MYSQL_TIME *ltime2) {
  thd->variables.time_zone->gmt_sec_to_TIME(
      ltime2, static_cast<my_time_t>(thd->query_start_in_secs()));
  ltime2->hour = ltime2->minute = ltime2->second = ltime2->second_part = 0;
  ltime2->time_type = MYSQL_TIMESTAMP_DATE;
  mix_date_and_time(ltime2, *ltime);
}

/**
   Return format string according format name.
   If name is unknown, result is NULL

   @param format
   @param type

   @return False on success, true on error.
*/
const char *get_date_time_format_str(const Known_date_time_format *format,
                                     enum_mysql_timestamp_type type) {
  switch (type) {
    case MYSQL_TIMESTAMP_DATE:
      return format->date_format;
    case MYSQL_TIMESTAMP_DATETIME:
      return format->datetime_format;
    case MYSQL_TIMESTAMP_TIME:
      return format->time_format;
    default:
      DBUG_ASSERT(0);  // Impossible
      return nullptr;
  }
}

/**
   @ingroup SQL_TIME
   @page DEFAULT_TIME_FUNCS Functions to create default time/date/datetime
   strings
   @note
    For the moment the Date_time_format argument is ignored becasue
    MySQL doesn't support comparing of date/time/datetime strings that
    are not in arbutary order as dates are compared as strings in some
    context)
    This functions don't check that given MYSQL_TIME structure members are
    in valid range. If they are not, return value won't reflect any
    valid date either. Additionally, make_time doesn't take into
    account time->day member: it's assumed that days have been converted
    to hours already.
*/

/**
  Convert TIME value to String.
  @param      format   Format (unused, see comments above)
  @param      l_time   TIME value
  @param[out] str      String to convert to
  @param      dec      Number of fractional digits.
*/
void make_time(const Date_time_format *format MY_ATTRIBUTE((unused)),
               const MYSQL_TIME *l_time, String *str, uint dec) {
  uint length = static_cast<uint>(my_time_to_str(*l_time, str->ptr(), dec));
  str->length(length);
  str->set_charset(&my_charset_numeric);
}

/**
  Convert DATE value to String.
  @param      format   Format (unused, see comments above)
  @param      l_time   DATE value
  @param[out] str      String to convert to
*/
void make_date(const Date_time_format *format MY_ATTRIBUTE((unused)),
               const MYSQL_TIME *l_time, String *str) {
  uint length = static_cast<uint>(my_date_to_str(*l_time, str->ptr()));
  str->length(length);
  str->set_charset(&my_charset_numeric);
}

/**
  Convert DATETIME value to String.
  @param      format   Format (unused, see comments above)
  @param      l_time   DATE value
  @param[out] str      String to convert to
  @param      dec      Number of fractional digits.
*/
void make_datetime(const Date_time_format *format MY_ATTRIBUTE((unused)),
                   const MYSQL_TIME *l_time, String *str, uint dec) {
  uint length = static_cast<uint>(my_datetime_to_str(*l_time, str->ptr(), dec));
  str->length(length);
  str->set_charset(&my_charset_numeric);
}

/**
  Convert TIME/DATE/DATETIME value to String.
  @param      ltime    DATE value
  @param[out] str      String to convert to
  @param      dec      Number of fractional digits.
*/
bool my_TIME_to_str(const MYSQL_TIME *ltime, String *str, uint dec) {
  if (str->alloc(MAX_DATE_STRING_REP_LENGTH)) return true;
  str->set_charset(&my_charset_numeric);
  str->length(my_TIME_to_str(*ltime, str->ptr(), dec));
  return false;
}

/**
   Create and add a truncated value warning to the THD.

   @returns value of thd->is_error() after adding the warning
 */
bool make_truncated_value_warning(THD *thd,
                                  Sql_condition::enum_severity_level level,
                                  const ErrConvString &val,
                                  enum_mysql_timestamp_type time_type,
                                  const char *field_name) {
  char warn_buff[MYSQL_ERRMSG_SIZE];
  const char *type_str;
  CHARSET_INFO *cs = system_charset_info;

  switch (time_type) {
    case MYSQL_TIMESTAMP_DATE:
      type_str = "date";
      break;
    case MYSQL_TIMESTAMP_TIME:
      type_str = "time";
      break;
    case MYSQL_TIMESTAMP_DATETIME:  // FALLTHROUGH
    default:
      type_str = "datetime";
      break;
  }
  if (field_name)
    cs->cset->snprintf(
        cs, warn_buff, sizeof(warn_buff),
        ER_THD(thd, ER_TRUNCATED_WRONG_VALUE_FOR_FIELD), type_str, val.ptr(),
        field_name,
        static_cast<long>(thd->get_stmt_da()->current_row_for_condition()));
  else {
    if (time_type > MYSQL_TIMESTAMP_ERROR)
      cs->cset->snprintf(cs, warn_buff, sizeof(warn_buff),
                         ER_THD(thd, ER_TRUNCATED_WRONG_VALUE), type_str,
                         val.ptr());
    else
      cs->cset->snprintf(cs, warn_buff, sizeof(warn_buff),
                         ER_THD(thd, ER_WRONG_VALUE), type_str, val.ptr());
  }
  push_warning(thd, level, ER_TRUNCATED_WRONG_VALUE, warn_buff);

  // strict mode can convert warning to error. Check for error while returning.
  return thd->is_error();
}

/**
   Uses propagate_datetime_overflow() to handle and propagate any warnings from
   date_add_interval() to the THD.

   @return False on success, true on error.
 */
bool date_add_interval_with_warn(THD *thd, MYSQL_TIME *ltime,
                                 interval_type int_type, Interval interval) {
  return propagate_datetime_overflow(thd, [&](int *w) {
    return date_add_interval(ltime, int_type, interval, w);
  });
}

/**
   Propagates a DATETIME_OVERFLOW warning from warnings bitfield to DA in thd.

   @param thd thread context
   @param[in,out] warnings bitfield of warnings set
 */
void propagate_datetime_overflow_helper(THD *thd, int *warnings) {
  if (warnings && (*warnings & MYSQL_TIME_WARN_DATETIME_OVERFLOW) != 0) {
    push_warning_printf(thd, Sql_condition::SL_WARNING,
                        ER_DATETIME_FUNCTION_OVERFLOW,
                        ER_THD(thd, ER_DATETIME_FUNCTION_OVERFLOW), "datetime");
    *warnings &= ~(MYSQL_TIME_WARN_DATETIME_OVERFLOW);
  }
}

/**
  Unpack packed numeric temporal value to date/time value
  and then convert to decimal representation.

  @param [out] dec          The variable to write to.
  @param      type         MySQL field type.
  @param      packed_value Packed numeric temporal representation.
  @return     A decimal value in on of the following formats, depending
              on type: YYYYMMDD, hhmmss.ffffff or YYMMDDhhmmss.ffffff.
*/
my_decimal *my_decimal_from_datetime_packed(my_decimal *dec,
                                            enum enum_field_types type,
                                            longlong packed_value) {
  MYSQL_TIME ltime;
  switch (type) {
    case MYSQL_TYPE_TIME:
      TIME_from_longlong_time_packed(&ltime, packed_value);
      return time2my_decimal(&ltime, dec);
    case MYSQL_TYPE_DATE:
      TIME_from_longlong_date_packed(&ltime, packed_value);
      ulonglong2decimal(TIME_to_ulonglong_date(ltime), dec);
      return dec;
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TIMESTAMP:
      TIME_from_longlong_datetime_packed(&ltime, packed_value);
      return date2my_decimal(&ltime, dec);
    default:
      DBUG_ASSERT(0);
      ulonglong2decimal(0, dec);
      return dec;
  }
}

/**
   This function gets GMT time and adds value of time_zone to get
   the local time. This function is used when server wants a timestamp
   value from dictionary system.

   @param gmt_time     GMT time value.
   @return time as ulonglong
*/
ulonglong gmt_time_to_local_time(ulonglong gmt_time) {
  MYSQL_TIME time;
  bool not_used;

  THD *thd = current_thd;
  Time_zone *tz = thd->variables.time_zone;

  // Convert longlong time to MYSQL_TIME format
  my_longlong_to_datetime_with_warn(gmt_time, &time, MYF(0));

  // Convert MYSQL_TIME to epoc second according to GMT time_zone.
  my_time_t timestamp;
  timestamp = my_tz_OFFSET0->TIME_to_gmt_sec(&time, &not_used);

  // Convert epoc seconds to local time
  tz->gmt_sec_to_TIME(&time, timestamp);

  // Return ulonglong value from MYSQL_TIME
  return TIME_to_ulonglong_datetime(time);
}

MYSQL_TIME my_time_set(uint y, uint m, uint d, uint h, uint mi, uint s,
                       unsigned long ms, bool negative,
                       enum_mysql_timestamp_type type) {
  return {y, m, d, h, mi, s, ms, negative, type, 0};
}

uint actual_decimals(const MYSQL_TIME *ts) {
  uint count = DATETIME_MAX_DECIMALS;
  for (int i = 1; i <= DATETIME_MAX_DECIMALS; i++) {
    if (ts->second_part % log_10_int[i] != 0) break;
    count--;
  }
  return count;
}

size_t max_fraction(uint decimals) {
  size_t res = 0;
  for (uint i = 1; i <= DATETIME_MAX_DECIMALS; i++) {
    res *= 10;
    if (i <= decimals) res += 9;
  }
  return res;
}

/**
   @} (end of defgroup SQL_TIME)
*/
