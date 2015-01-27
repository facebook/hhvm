/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2013 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Shane Caraveo             <shane@caraveo.com>               |
   |          Colin Viebrock            <colin@easydns.com>               |
   |          Hartmut Holzgraefe        <hholzgra@php.net>                |
   |          Wez Furlong               <wez@thebrainroom.com>            |
   +----------------------------------------------------------------------+
 */
/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef PHP_WIN32
#define _WINNLS_
#endif

#include "php.h"
#include <ext/standard/info.h>
#include "php_calendar.h"
#include "sdncal.h"

#include <stdio.h>

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_unixtojd, 0, 0, 0)
  ZEND_ARG_INFO(0, timestamp)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_jdtounix, 0)
  ZEND_ARG_INFO(0, jday)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_cal_info, 0, 0, 0)
  ZEND_ARG_INFO(0, calendar)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_cal_days_in_month, 0)
  ZEND_ARG_INFO(0, calendar)
  ZEND_ARG_INFO(0, month)
  ZEND_ARG_INFO(0, year)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_cal_to_jd, 0)
  ZEND_ARG_INFO(0, calendar)
  ZEND_ARG_INFO(0, month)
  ZEND_ARG_INFO(0, day)
  ZEND_ARG_INFO(0, year)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_cal_from_jd, 0)
  ZEND_ARG_INFO(0, jd)
  ZEND_ARG_INFO(0, calendar)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_jdtogregorian, 0)
  ZEND_ARG_INFO(0, juliandaycount)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_gregoriantojd, 0)
  ZEND_ARG_INFO(0, month)
  ZEND_ARG_INFO(0, day)
  ZEND_ARG_INFO(0, year)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_jdtojulian, 0)
  ZEND_ARG_INFO(0, juliandaycount)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_juliantojd, 0)
  ZEND_ARG_INFO(0, month)
  ZEND_ARG_INFO(0, day)
  ZEND_ARG_INFO(0, year)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_jdtojewish, 0, 0, 1)
  ZEND_ARG_INFO(0, juliandaycount)
  ZEND_ARG_INFO(0, hebrew)
  ZEND_ARG_INFO(0, fl)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_jewishtojd, 0)
  ZEND_ARG_INFO(0, month)
  ZEND_ARG_INFO(0, day)
  ZEND_ARG_INFO(0, year)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_jdtofrench, 0)
  ZEND_ARG_INFO(0, juliandaycount)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_frenchtojd, 0)
  ZEND_ARG_INFO(0, month)
  ZEND_ARG_INFO(0, day)
  ZEND_ARG_INFO(0, year)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_jddayofweek, 0, 0, 1)
  ZEND_ARG_INFO(0, juliandaycount)
  ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_jdmonthname, 0)
  ZEND_ARG_INFO(0, juliandaycount)
  ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_easter_date, 0, 0, 0)
  ZEND_ARG_INFO(0, year)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_easter_days, 0, 0, 0)
  ZEND_ARG_INFO(0, year)
  ZEND_ARG_INFO(0, method)
ZEND_END_ARG_INFO()

/* }}} */

const zend_function_entry calendar_functions[] = {
  PHP_FE(jdtogregorian, arginfo_jdtogregorian)
  PHP_FE(gregoriantojd, arginfo_gregoriantojd)
  PHP_FE(jdtojulian, arginfo_jdtojulian)
  PHP_FE(juliantojd, arginfo_juliantojd)
  PHP_FE(jdtojewish, arginfo_jdtojewish)
  PHP_FE(jewishtojd, arginfo_jewishtojd)
  PHP_FE(jdtofrench, arginfo_jdtofrench)
  PHP_FE(frenchtojd, arginfo_frenchtojd)
  PHP_FE(jddayofweek, arginfo_jddayofweek)
  PHP_FE(jdmonthname, arginfo_jdmonthname)
  PHP_FE(easter_date, arginfo_easter_date)
  PHP_FE(easter_days, arginfo_easter_days)
  PHP_FE(unixtojd, arginfo_unixtojd)
  PHP_FE(jdtounix, arginfo_jdtounix)
  PHP_FE(cal_to_jd, arginfo_cal_to_jd)
  PHP_FE(cal_from_jd, arginfo_cal_from_jd)
  PHP_FE(cal_days_in_month, arginfo_cal_days_in_month)
  PHP_FE(cal_info, arginfo_cal_info)
  PHP_FE_END
};


zend_module_entry calendar_module_entry = {
  STANDARD_MODULE_HEADER,
  "calendar",
  calendar_functions,
  PHP_MINIT(calendar),
  NULL,
  NULL,
  NULL,
  PHP_MINFO(calendar),
  NO_VERSION_YET,
  STANDARD_MODULE_PROPERTIES,
};

#ifdef COMPILE_DL_CALENDAR
ZEND_GET_MODULE(calendar)
#endif

/* this order must match the conversion table below */
enum cal_name_type_t {
  CAL_GREGORIAN = 0,
  CAL_JULIAN,
  CAL_JEWISH,
  CAL_FRENCH,
  CAL_NUM_CALS
};

typedef long int (*cal_to_jd_func_t) (int month, int day, int year);
typedef void (*cal_from_jd_func_t) (long int jd, int *year, int *month, int *day);
typedef char *(*cal_as_string_func_t) (int year, int month, int day);

struct cal_entry_t {
  char *name;
  char *symbol;
  cal_to_jd_func_t to_jd;
  cal_from_jd_func_t from_jd;
  int num_months;
  int max_days_in_month;
  char **month_name_short;
  char **month_name_long;
};

static struct cal_entry_t cal_conversion_table[CAL_NUM_CALS] = {
  {"Gregorian", "CAL_GREGORIAN", GregorianToSdn, SdnToGregorian, 12, 31,
   MonthNameShort, MonthNameLong},
  {"Julian", "CAL_JULIAN", JulianToSdn, SdnToJulian, 12, 31,
   MonthNameShort, MonthNameLong},
  {"Jewish", "CAL_JEWISH", JewishToSdn, SdnToJewish, 13, 30,
   JewishMonthNameLeap, JewishMonthNameLeap},
  {"French", "CAL_FRENCH", FrenchToSdn, SdnToFrench, 13, 30,
   FrenchMonthName, FrenchMonthName}
};

static inline bool isJewishLeapYear(int year) {
  if (year < 1) return false;
  return monthsPerYear[(year - 1) % 19] == 13;
}

#define JEWISH_MONTH_NAME(year)                                         \
    (isJewishLeapYear(year) ? JewishMonthNameLeap : JewishMonthName)
#define JEWISH_HEB_MONTH_NAME(year)                                     \
    (isJewishLeapYear(year) ? JewishMonthHebNameLeap : JewishMonthHebName)

/* For jddayofweek */
enum { CAL_DOW_DAYNO, CAL_DOW_SHORT, CAL_DOW_LONG };

/* For jdmonthname */
enum { CAL_MONTH_GREGORIAN_SHORT, CAL_MONTH_GREGORIAN_LONG,
  CAL_MONTH_JULIAN_SHORT, CAL_MONTH_JULIAN_LONG, CAL_MONTH_JEWISH,
  CAL_MONTH_FRENCH
};

/* for heb_number_to_chars */
static char alef_bet[25] = "0\xE0\xE1\xE2\xE3\xE4\xE5\xE6\xE7\xE8\xE9\xEB\xEC\xEE\xF0\xF1\xF2\xF4\xF6\xF7\xF8\xF9\xFA";

#define CAL_JEWISH_ADD_ALAFIM_GERESH 0x2
#define CAL_JEWISH_ADD_ALAFIM 0x4
#define CAL_JEWISH_ADD_GERESHAYIM 0x8

PHP_MINIT_FUNCTION(calendar)
{
  REGISTER_LONG_CONSTANT("CAL_GREGORIAN", CAL_GREGORIAN, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("CAL_JULIAN", CAL_JULIAN, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("CAL_JEWISH", CAL_JEWISH, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("CAL_FRENCH", CAL_FRENCH, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("CAL_NUM_CALS", CAL_NUM_CALS, CONST_CS | CONST_PERSISTENT);
/* constants for jddayofweek */
  REGISTER_LONG_CONSTANT("CAL_DOW_DAYNO", CAL_DOW_DAYNO, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("CAL_DOW_SHORT", CAL_DOW_SHORT, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("CAL_DOW_LONG", CAL_DOW_LONG, CONST_CS | CONST_PERSISTENT);
/* constants for jdmonthname */
  REGISTER_LONG_CONSTANT("CAL_MONTH_GREGORIAN_SHORT", CAL_MONTH_GREGORIAN_SHORT, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("CAL_MONTH_GREGORIAN_LONG", CAL_MONTH_GREGORIAN_LONG, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("CAL_MONTH_JULIAN_SHORT", CAL_MONTH_JULIAN_SHORT, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("CAL_MONTH_JULIAN_LONG", CAL_MONTH_JULIAN_LONG, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("CAL_MONTH_JEWISH", CAL_MONTH_JEWISH, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("CAL_MONTH_FRENCH", CAL_MONTH_FRENCH, CONST_CS | CONST_PERSISTENT);
/* constants for easter calculation */
  REGISTER_LONG_CONSTANT("CAL_EASTER_DEFAULT", CAL_EASTER_DEFAULT, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("CAL_EASTER_ROMAN", CAL_EASTER_ROMAN, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("CAL_EASTER_ALWAYS_GREGORIAN", CAL_EASTER_ALWAYS_GREGORIAN, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("CAL_EASTER_ALWAYS_JULIAN", CAL_EASTER_ALWAYS_JULIAN, CONST_CS | CONST_PERSISTENT);
/* constants for Jewish date formatting */
  REGISTER_LONG_CONSTANT("CAL_JEWISH_ADD_ALAFIM_GERESH", CAL_JEWISH_ADD_ALAFIM_GERESH, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("CAL_JEWISH_ADD_ALAFIM", CAL_JEWISH_ADD_ALAFIM, CONST_CS | CONST_PERSISTENT);
  REGISTER_LONG_CONSTANT("CAL_JEWISH_ADD_GERESHAYIM", CAL_JEWISH_ADD_GERESHAYIM, CONST_CS | CONST_PERSISTENT);
  return SUCCESS;
}

PHP_MINFO_FUNCTION(calendar)
{
  php_info_print_table_start();
  php_info_print_table_row(2, "Calendar support", "enabled");
  php_info_print_table_end();
}

static void _php_cal_info(int cal, zval **ret)
{
  zval *months, *smonths;
  int i;
  struct cal_entry_t *calendar;

  calendar = &cal_conversion_table[cal];
  array_init(*ret);

  MAKE_STD_ZVAL(months);
  MAKE_STD_ZVAL(smonths);
  array_init(months);
  array_init(smonths);

  for (i = 1; i <= calendar->num_months; i++) {
    add_index_string(months, i, calendar->month_name_long[i], 1);
    add_index_string(smonths, i, calendar->month_name_short[i], 1);
  }
  add_assoc_zval(*ret, "months", months);
  add_assoc_zval(*ret, "abbrevmonths", smonths);
  add_assoc_long(*ret, "maxdaysinmonth", calendar->max_days_in_month);
  add_assoc_string(*ret, "calname", calendar->name, 1);
  add_assoc_string(*ret, "calsymbol", calendar->symbol, 1);

}

/* {{{ proto array cal_info([int calendar])
   Returns information about a particular calendar */
PHP_FUNCTION(cal_info)
{
  long cal = -1;


  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &cal) == FAILURE) {
    RETURN_FALSE;
  }

  if (cal == -1) {
    int i;
    zval *val;

    array_init(return_value);

    for (i = 0; i < CAL_NUM_CALS; i++) {
      MAKE_STD_ZVAL(val);
      _php_cal_info(i, &val);
      add_index_zval(return_value, i, val);
    }
    return;
  }


  if (cal != -1 && (cal < 0 || cal >= CAL_NUM_CALS)) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "invalid calendar ID %ld.", cal);
    RETURN_FALSE;
  }

  _php_cal_info(cal, &return_value);

}
/* }}} */

/* {{{ proto int cal_days_in_month(int calendar, int month, int year)
   Returns the number of days in a month for a given year and calendar */
PHP_FUNCTION(cal_days_in_month)
{
  long cal, month, year;
  struct cal_entry_t *calendar;
  long sdn_start, sdn_next;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lll", &cal, &month, &year) == FAILURE) {
    RETURN_FALSE;
  }

  if (cal < 0 || cal >= CAL_NUM_CALS) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "invalid calendar ID %ld.", cal);
    RETURN_FALSE;
  }

  calendar = &cal_conversion_table[cal];

  sdn_start = calendar->to_jd(year, month, 1);

  if (sdn_start == 0) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "invalid date.");
    RETURN_FALSE;
  }

  sdn_next = calendar->to_jd(year, 1 + month, 1);

  if (sdn_next == 0) {
    /* If the next month is invalid, then we need to try the first month of
     * the next year, bearing in mind that the next year after 1 BCE is
     * actually 1 AD and not 0. */
    if (year == -1) {
      sdn_next = calendar->to_jd(1, 1, 1);
    }
    else {
      sdn_next = calendar->to_jd(year + 1, 1, 1);
    }
  }

  RETURN_LONG(sdn_next - sdn_start);
}
/* }}} */

/* {{{ proto int cal_to_jd(int calendar, int month, int day, int year)
   Converts from a supported calendar to Julian Day Count */
PHP_FUNCTION(cal_to_jd)
{
  long cal, month, day, year;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "llll", &cal, &month, &day, &year) != SUCCESS) {
    RETURN_FALSE;
  }

  if (cal < 0 || cal >= CAL_NUM_CALS) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "invalid calendar ID %ld.", cal);
    RETURN_FALSE;
  }

  RETURN_LONG(cal_conversion_table[cal].to_jd(year, month, day));
}
/* }}} */

/* {{{ proto array cal_from_jd(int jd, int calendar)
   Converts from Julian Day Count to a supported calendar and return extended information */
PHP_FUNCTION(cal_from_jd)
{
  long jd, cal;
  int month, day, year, dow;
  char date[16];
  struct cal_entry_t *calendar;

  if (zend_parse_parameters(ZEND_NUM_ARGS()TSRMLS_CC, "ll", &jd, &cal) == FAILURE) {
    RETURN_FALSE;
  }

  if (cal < 0 || cal >= CAL_NUM_CALS) {
    php_error_docref(NULL TSRMLS_CC, E_WARNING, "invalid calendar ID %ld", cal);
    RETURN_FALSE;
  }
  calendar = &cal_conversion_table[cal];

  array_init(return_value);

  calendar->from_jd(jd, &year, &month, &day);

  snprintf(date, sizeof(date), "%i/%i/%i", month, day, year);
  add_assoc_string(return_value, "date", date, 1);

  add_assoc_long(return_value, "month", month);
  add_assoc_long(return_value, "day", day);
  add_assoc_long(return_value, "year", year);

/* day of week */
  dow = DayOfWeek(jd);
  add_assoc_long(return_value, "dow", dow);
  add_assoc_string(return_value, "abbrevdayname", DayNameShort[dow], 1);
  add_assoc_string(return_value, "dayname", DayNameLong[dow], 1);
/* month name */
  if(cal == CAL_JEWISH) {
    /* special case for Jewish calendar */
    add_assoc_string(return_value, "abbrevmonth", JEWISH_MONTH_NAME(year)[month], 1);
    add_assoc_string(return_value, "monthname", JEWISH_MONTH_NAME(year)[month], 1);
  } else {
    add_assoc_string(return_value, "abbrevmonth", calendar->month_name_short[month], 1);
    add_assoc_string(return_value, "monthname", calendar->month_name_long[month], 1);
  }
}
/* }}} */

/* {{{ proto string jdtogregorian(int juliandaycount)
   Converts a julian day count to a gregorian calendar date */
PHP_FUNCTION(jdtogregorian)
{
  long julday;
  int year, month, day;
  char date[16];

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &julday) == FAILURE) {
    RETURN_FALSE;
  }

  SdnToGregorian(julday, &year, &month, &day);
  snprintf(date, sizeof(date), "%i/%i/%i", month, day, year);

  RETURN_STRING(date, 1);
}
/* }}} */

/* {{{ proto int gregoriantojd(int month, int day, int year)
   Converts a gregorian calendar date to julian day count */
PHP_FUNCTION(gregoriantojd)
{
  long year, month, day;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lll", &month, &day, &year) == FAILURE) {
    RETURN_FALSE;
  }

  RETURN_LONG(GregorianToSdn(year, month, day));
}
/* }}} */

/* {{{ proto string jdtojulian(int juliandaycount)
   Convert a julian day count to a julian calendar date */
PHP_FUNCTION(jdtojulian)
{
  long julday;
  int year, month, day;
  char date[16];

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &julday) == FAILURE) {
    RETURN_FALSE;
  }

  SdnToJulian(julday, &year, &month, &day);
  snprintf(date, sizeof(date), "%i/%i/%i", month, day, year);

  RETURN_STRING(date, 1);
}
/* }}} */

/* {{{ proto int juliantojd(int month, int day, int year)
   Converts a julian calendar date to julian day count */
PHP_FUNCTION(juliantojd)
{
  long year, month, day;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lll", &month, &day, &year) == FAILURE) {
    RETURN_FALSE;
  }

  RETURN_LONG(JulianToSdn(year, month, day));
}
/* }}} */

/* {{{ heb_number_to_chars*/
/*
caution: the Hebrew format produces non unique result.
for example both: year '5' and year '5000' produce '\xE4'.
use the numeric one for calculations.
 */
static char *heb_number_to_chars(int n, int fl, char **ret)
{
  char *p, old[18], *endofalafim;

  p = endofalafim = old;
/*
   prevents the option breaking the jewish beliefs, and some other
   critical resources ;)
 */
  if (n > 9999 || n < 1) {
    *ret = NULL;
    return NULL;
  }

/* alafim (thousands) case */
  if (n / 1000) {
    *p = alef_bet[n / 1000];
    p++;

    if (CAL_JEWISH_ADD_ALAFIM_GERESH & fl) {
      *p = '\'';
      p++;
    }
    if (CAL_JEWISH_ADD_ALAFIM & fl) {
      strcpy(p, " \xE0\xEC\xF4\xE9\xED ");
      p += 7;
    }

    endofalafim = p;
    n = n % 1000;
  }

/* tav-tav (tav=400) case */
  while (n >= 400) {
    *p = alef_bet[22];
    p++;
    n -= 400;
  }

/* meot (hundreads) case */
  if (n >= 100) {
    *p = alef_bet[18 + n / 100];
    p++;
    n = n % 100;
  }

/* tet-vav & tet-zain case (special case for 15 and 16) */
  if (n == 15 || n == 16) {
    *p = alef_bet[9];
    p++;
    *p = alef_bet[n - 9];
    p++;
  } else {
/* asarot (tens) case */
    if (n >= 10) {
      *p = alef_bet[9 + n / 10];
      p++;
      n = n % 10;
    }

/* yehidot (ones) case */
    if (n > 0) {
      *p = alef_bet[n];
      p++;
    }
  }

  if (CAL_JEWISH_ADD_GERESHAYIM & fl) {
    switch (p - endofalafim) {
    case 0:
      break;
    case 1:
      *p = '\'';
      p++;
      break;
    default:
      *(p) = *(p - 1);
      *(p - 1) = '"';
      p++;
    }
  }

  *p = '\0';
  *ret = estrndup(old, (p - old) + 1);
  p = *ret;
  return p;
}
/* }}} */

/* {{{ proto string jdtojewish(int juliandaycount [, bool hebrew [, int fl]])
   Converts a julian day count to a jewish calendar date */
PHP_FUNCTION(jdtojewish)
{
  long julday, fl = 0;
  zend_bool heb   = 0;
  int year, month, day;
  char date[16], hebdate[32];
  char *dayp, *yearp;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|bl", &julday, &heb, &fl) == FAILURE) {
    RETURN_FALSE;
  }

  SdnToJewish(julday, &year, &month, &day);
  if (!heb) {
    snprintf(date, sizeof(date), "%i/%i/%i", month, day, year);
    RETURN_STRING(date, 1);
  } else {
    if (year <= 0 || year > 9999) {
      php_error_docref(NULL TSRMLS_CC, E_WARNING, "Year out of range (0-9999).");
      RETURN_FALSE;
    }

    snprintf(hebdate, sizeof(hebdate), "%s %s %s", heb_number_to_chars(day, fl, &dayp), JEWISH_HEB_MONTH_NAME(year)[month], heb_number_to_chars(year, fl, &yearp));

    if (dayp) {
      efree(dayp);
    }
    if (yearp) {
      efree(yearp);
    }

    RETURN_STRING(hebdate, 1);

  }
}
/* }}} */

/* {{{ proto int jewishtojd(int month, int day, int year)
   Converts a jewish calendar date to a julian day count */
PHP_FUNCTION(jewishtojd)
{
  long year, month, day;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lll", &month, &day, &year) == FAILURE) {
    RETURN_FALSE;
  }

  RETURN_LONG(JewishToSdn(year, month, day));
}
/* }}} */

/* {{{ proto string jdtofrench(int juliandaycount)
   Converts a julian day count to a french republic calendar date */
PHP_FUNCTION(jdtofrench)
{
  long julday;
  int year, month, day;
  char date[16];

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &julday) == FAILURE) {
    RETURN_FALSE;
  }

  SdnToFrench(julday, &year, &month, &day);
  snprintf(date, sizeof(date), "%i/%i/%i", month, day, year);

  RETURN_STRING(date, 1);
}
/* }}} */

/* {{{ proto int frenchtojd(int month, int day, int year)
   Converts a french republic calendar date to julian day count */
PHP_FUNCTION(frenchtojd)
{
  long year, month, day;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lll", &month, &day, &year) == FAILURE) {
    RETURN_FALSE;
  }

  RETURN_LONG(FrenchToSdn(year, month, day));
}
/* }}} */

/* {{{ proto mixed jddayofweek(int juliandaycount [, int mode])
   Returns name or number of day of week from julian day count */
PHP_FUNCTION(jddayofweek)
{
  long julday, mode = CAL_DOW_DAYNO;
  int day;
  char *daynamel, *daynames;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|l", &julday, &mode) == FAILURE) {
    RETURN_FALSE;
  }

  day = DayOfWeek(julday);
  daynamel = DayNameLong[day];
  daynames = DayNameShort[day];

  switch (mode) {
  case CAL_DOW_SHORT:
    RETURN_STRING(daynamel, 1);
    break;
  case CAL_DOW_LONG:
    RETURN_STRING(daynames, 1);
    break;
  case CAL_DOW_DAYNO:
  default:
    RETURN_LONG(day);
    break;
  }
}
/* }}} */

/* {{{ proto string jdmonthname(int juliandaycount, int mode)
   Returns name of month for julian day count */
PHP_FUNCTION(jdmonthname)
{
  long julday, mode;
  char *monthname = NULL;
  int month, day, year;

  if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &julday, &mode) == FAILURE) {
    RETURN_FALSE;
  }

  switch (mode) {
  case CAL_MONTH_GREGORIAN_LONG:  /* gregorian or julian month */
    SdnToGregorian(julday, &year, &month, &day);
    monthname = MonthNameLong[month];
    break;
  case CAL_MONTH_JULIAN_SHORT:  /* gregorian or julian month */
    SdnToJulian(julday, &year, &month, &day);
    monthname = MonthNameShort[month];
    break;
  case CAL_MONTH_JULIAN_LONG:  /* gregorian or julian month */
    SdnToJulian(julday, &year, &month, &day);
    monthname = MonthNameLong[month];
    break;
  case CAL_MONTH_JEWISH:    /* jewish month */
    SdnToJewish(julday, &year, &month, &day);
    monthname = JEWISH_MONTH_NAME(year)[month];
    break;
  case CAL_MONTH_FRENCH:    /* french month */
    SdnToFrench(julday, &year, &month, &day);
    monthname = FrenchMonthName[month];
    break;
  default:          /* default gregorian */
  case CAL_MONTH_GREGORIAN_SHORT:  /* gregorian or julian month */
    SdnToGregorian(julday, &year, &month, &day);
    monthname = MonthNameShort[month];
    break;
  }

  RETURN_STRING(monthname, 1);
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
