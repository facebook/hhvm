/* Copyright (c) 2005, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include "sql/my_decimal.h"

#include "my_config.h"

#include <stdio.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include "decimal.h"
#include "m_ctype.h"
#include "my_dbug.h"
#include "my_sys.h"
#include "my_time.h"          // TIME_to_ulonglong_date
#include "mysql_time.h"       // MYSQL_TIME
#include "mysqld_error.h"     // ER_*
#include "sql/current_thd.h"  // current_thd
#include "sql/derror.h"       // ER_THD
#include "sql/field.h"        // my_charset_numeric
#include "sql/sql_const.h"
#include "sql/sql_error.h"  // Sql_condition

/**
   report result of decimal operation.

   @param mask    bitmask filtering result, most likely E_DEC_FATAL_ERROR
   @param result  decimal library return code (E_DEC_* see include/decimal.h)

   @return
     result
*/
int my_decimal::check_result(uint mask, int result) const {
  if (result & mask) {
    int length = DECIMAL_MAX_STR_LENGTH + 1;
    char strbuff[DECIMAL_MAX_STR_LENGTH + 2];

    switch (result) {
      case E_DEC_TRUNCATED:
        // "Data truncated for column \'%s\' at row %ld"
        push_warning_printf(current_thd, Sql_condition::SL_WARNING,
                            WARN_DATA_TRUNCATED,
                            ER_THD(current_thd, WARN_DATA_TRUNCATED), "", -1L);
        break;
      case E_DEC_OVERFLOW:
        // "Truncated incorrect %-.32s value: \'%-.128s\'"
        decimal2string(this, strbuff, &length);
        push_warning_printf(
            current_thd, Sql_condition::SL_WARNING, ER_TRUNCATED_WRONG_VALUE,
            ER_THD(current_thd, ER_TRUNCATED_WRONG_VALUE), "DECIMAL", strbuff);
        break;
      case E_DEC_DIV_ZERO:
        // "Division by 0"
        push_warning(current_thd, Sql_condition::SL_WARNING,
                     ER_DIVISION_BY_ZERO,
                     ER_THD(current_thd, ER_DIVISION_BY_ZERO));
        break;
      case E_DEC_BAD_NUM:
        // "Incorrect %-.32s value: \'%-.128s\' for column \'%.192s\' at row
        // %ld"
        decimal2string(this, strbuff, &length);
        push_warning_printf(
            current_thd, Sql_condition::SL_WARNING,
            ER_TRUNCATED_WRONG_VALUE_FOR_FIELD,
            ER_THD(current_thd, ER_TRUNCATED_WRONG_VALUE_FOR_FIELD), "DECIMAL",
            strbuff, "", -1L);
        break;
      case E_DEC_OOM:
        my_error(ER_OUT_OF_RESOURCES, MYF(0));
        break;
      default:
        DBUG_ASSERT(0);
    }
  }
  return result;
}

/**
  @brief Converting decimal to string

  @details Convert given my_decimal to String; allocate buffer as needed.

  @param[in]   mask        what problems to warn on (mask of E_DEC_* values)
  @param[in]   d           the decimal to print
  @param[in]   fixed_prec  overall number of digits if ZEROFILL, 0 otherwise
  @param[in]   fixed_dec   number of decimal places (if fixed_prec != 0)
  @param[out]  str         where to store the resulting string

  @return error coce
    @retval E_DEC_OK
    @retval E_DEC_TRUNCATED
    @retval E_DEC_OVERFLOW
    @retval E_DEC_OOM
*/

int my_decimal2string(uint mask, const my_decimal *d, uint fixed_prec,
                      uint fixed_dec, String *str) {
  /*
    Calculate the size of the string: For DECIMAL(a,b), fixed_prec==a
    holds true iff the type is also ZEROFILL, which in turn implies
    UNSIGNED. Hence the buffer for a ZEROFILLed value is the length
    the user requested, plus one for a possible decimal point, plus
    one if the user only wanted decimal places, but we force a leading
    zero on them, plus one for the '\0' terminator. Because the type
    is implicitly UNSIGNED, we do not need to reserve a character for
    the sign. For all other cases, fixed_prec will be 0, and
    my_decimal_string_length() will be called instead to calculate the
    required size of the buffer.
  */
  int length =
      (fixed_prec ? (fixed_prec + ((fixed_prec == fixed_dec) ? 1 : 0) + 1 + 1)
                  : my_decimal_string_length(d));
  int result;
  if (str->alloc(length)) return d->check_result(mask, E_DEC_OOM);
  result = decimal2string(d, str->ptr(), &length, fixed_prec, fixed_dec);
  str->length(length);
  str->set_charset(&my_charset_numeric);
  return d->check_result(mask, result);
}

/**
  @brief Converting decimal to string with character set conversion

  @details Convert given my_decimal to String; allocate buffer as needed.

  @param[in]   mask        what problems to warn on (mask of E_DEC_* values)
  @param[in]   val         the decimal to print
  @param[out]  str         where to store the resulting string
  @param[in]   cs          character set

  @return error code
    @retval E_DEC_OK
    @retval E_DEC_TRUNCATED
    @retval E_DEC_OVERFLOW
    @retval E_DEC_OOM

  Would be great to make it a method of the String class,
  but this would need to include
  my_decimal.h from sql_string.h and sql_string.cc, which is not desirable.
*/
bool str_set_decimal(uint mask, const my_decimal *val, String *str,
                     const CHARSET_INFO *cs) {
  if (!(cs->state & MY_CS_NONASCII)) {
    /* For ASCII-compatible character sets we can use my_decimal2string */
    my_decimal2string(mask, val, str);
    str->set_charset(cs);
    return false;
  } else {
    /*
      For ASCII-incompatible character sets (like UCS2) we
      call my_decimal2string() on a temporary buffer first,
      and then convert the result to the target character
      with help of str->copy().
    */
    StringBuffer<DECIMAL_MAX_STR_LENGTH + 1> tmp(&my_charset_latin1);
    my_decimal2string(mask, val, &tmp);
    uint errors;
    return str->copy(tmp.ptr(), tmp.length(), &my_charset_latin1, cs, &errors);
  }
}

/*
  Convert from decimal to binary representation

  SYNOPSIS
    my_decimal2binary()
    mask        error processing mask
    d           number for conversion
    bin         pointer to buffer where to write result
    prec        overall number of decimal digits
    scale       number of decimal digits after decimal point

  NOTE
    Before conversion we round number if it need but produce truncation
    error in this case

  RETURN
    E_DEC_OK
    E_DEC_TRUNCATED
    E_DEC_OVERFLOW
*/

int my_decimal2binary(uint mask, const my_decimal *d, uchar *bin, int prec,
                      int scale) {
  int err1 = E_DEC_OK, err2;
  my_decimal rounded;
  my_decimal2decimal(d, &rounded);
  rounded.frac = decimal_actual_fraction(&rounded);
  if (scale < rounded.frac) {
    err1 = E_DEC_TRUNCATED;
    /* decimal_round can return only E_DEC_TRUNCATED */
    decimal_round(&rounded, &rounded, scale, HALF_UP);
  }
  err2 = decimal2bin(&rounded, bin, prec, scale);
  if (!err2) err2 = err1;
  return d->check_result(mask, err2);
}

/*
  Convert string for decimal when string can be in some multibyte charset

  SYNOPSIS
    str2my_decimal()
    mask            error processing mask
    from            string to process
    length          length of given string
    charset         charset of given string
    decimal_value   buffer for result storing

  RESULT
    E_DEC_OK
    E_DEC_TRUNCATED
    E_DEC_OVERFLOW
    E_DEC_BAD_NUM
    E_DEC_OOM
*/

int str2my_decimal(uint mask, const char *from, size_t length,
                   const CHARSET_INFO *charset, my_decimal *decimal_value) {
  const char *end, *from_end;
  int err;
  char buff[STRING_BUFFER_USUAL_SIZE];
  String tmp(buff, sizeof(buff), &my_charset_bin);
  if (charset->mbminlen > 1) {
    uint dummy_errors;
    tmp.copy(from, length, charset, &my_charset_latin1, &dummy_errors);
    from = tmp.ptr();
    length = tmp.length();
    charset = &my_charset_bin;
  }
  from_end = end = from + length;
  err = string2decimal(from, (decimal_t *)decimal_value, &end);
  if (end != from_end && !err) {
    /* Give warning if there is something other than end space */
    for (; end < from_end; end++) {
      if (!my_isspace(&my_charset_latin1, *end)) {
        err = E_DEC_TRUNCATED;
        break;
      }
    }
  }
  check_result_and_overflow(mask, err, decimal_value);
  return err;
}

/**
  Convert lldiv_t value to my_decimal value.
  Integer part of the result is set to lld->quot.
  Fractional part of the result is set to lld->rem divided to 1000000000.

  @param       lld  The lldiv_t variable to convert from.
  @param       neg  Sign flag (negative, 0 positive).
  @param [out] dec  Decimal numbert to convert to.
*/
static my_decimal *lldiv_t2my_decimal(const lldiv_t *lld, bool neg,
                                      my_decimal *dec) {
  if (int2my_decimal(E_DEC_FATAL_ERROR, lld->quot, false, dec)) return dec;
  if (lld->rem) {
    dec->buf[(dec->intg - 1) / 9 + 1] = static_cast<decimal_digit_t>(lld->rem);
    dec->frac = 6;
  }
  if (neg) my_decimal_neg(dec);
  return dec;
}

/**
  Convert datetime value to my_decimal in format YYYYMMDDhhmmss.ffffff
  @param ltime  Date value to convert from.
  @param dec    Decimal value to convert to.
*/
my_decimal *date2my_decimal(const MYSQL_TIME *ltime, my_decimal *dec) {
  lldiv_t lld;
  lld.quot = ltime->time_type > MYSQL_TIMESTAMP_DATE
                 ? TIME_to_ulonglong_datetime(*ltime)
                 : TIME_to_ulonglong_date(*ltime);
  lld.rem = (longlong)ltime->second_part * 1000;
  return lldiv_t2my_decimal(&lld, ltime->neg, dec);
}

/**
  Convert time value to my_decimal in format hhmmss.ffffff
  @param ltime  Date value to convert from.
  @param dec    Decimal value to convert to.
*/
my_decimal *time2my_decimal(const MYSQL_TIME *ltime, my_decimal *dec) {
  lldiv_t lld;
  lld.quot = TIME_to_ulonglong_time(*ltime);
  lld.rem = (longlong)ltime->second_part * 1000;
  return lldiv_t2my_decimal(&lld, ltime->neg, dec);
}

/**
  Convert timeval value to my_decimal.
*/
my_decimal *timeval2my_decimal(const struct timeval *tm, my_decimal *dec) {
  lldiv_t lld;
  lld.quot = tm->tv_sec;
  lld.rem = (longlong)tm->tv_usec * 1000;
  return lldiv_t2my_decimal(&lld, false, dec);
}

void my_decimal_trim(ulong *precision, uint *scale) {
  if (!(*precision) && !(*scale)) {
    *precision = 10;
    *scale = 0;
    return;
  }
}

#ifndef DBUG_OFF
/* routines for debugging print */

#define DIG_PER_DEC1 9
#define ROUND_UP(X) (((X) + DIG_PER_DEC1 - 1) / DIG_PER_DEC1)

/* print decimal */
void print_decimal(const my_decimal *dec) {
  int i, end;
  char buff[512], *pos;
  pos = buff;
  pos += sprintf(buff, "Decimal: sign: %d  intg: %d  frac: %d  { ", dec->sign(),
                 dec->intg, dec->frac);
  end = ROUND_UP(dec->frac) + ROUND_UP(dec->intg) - 1;
  for (i = 0; i < end; i++) pos += sprintf(pos, "%09d, ", dec->buf[i]);
  pos += sprintf(pos, "%09d }\n", dec->buf[i]);
  fputs(buff, DBUG_FILE);
}

/* print decimal with its binary representation */
void print_decimal_buff(const my_decimal *dec, const uchar *ptr, int length) {
  print_decimal(dec);
  fprintf(DBUG_FILE, "Record: ");
  for (int i = 0; i < length; i++) {
    fprintf(DBUG_FILE, "%02X ", ptr[i]);
  }
  fprintf(DBUG_FILE, "\n");
}

const char *dbug_decimal_as_string(char *buff, const my_decimal *val) {
  int length = DECIMAL_MAX_STR_LENGTH + 1; /* minimum size for buff */
  if (!val) return "NULL";
  (void)decimal2string(val, buff, &length);
  return buff;
}

#endif /*DBUG_OFF*/
