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

/*
  NOTE: This is a more-or-less direct port of the main() program
  in strings/decimal.c to a Google Test.
 */

#include "my_config.h"

#include <gtest/gtest.h>
#include <math.h>
#include <string.h>

#include "decimal.h"
#include "m_string.h"
#include "my_inttypes.h"
#include "my_macros.h"
#include "sql/my_decimal.h"
#include "unittest/gunit/benchmark.h"

namespace decimal_unittest {

#define DIG_PER_DEC1 9
#define DIG_BASE 1000000000
#define ROUND_UP(X) (((X) + DIG_PER_DEC1 - 1) / DIG_PER_DEC1)
typedef decimal_digit_t dec1;

int full = 0;
decimal_t a, b, c;
decimal_digit_t buf1[50], buf2[50], buf3[50];

void dump_decimal(decimal_t *d) {
  int i;
  printf("/* intg=%d, frac=%d, sign=%d, buf[]={", d->intg, d->frac, d->sign);
  for (i = 0; i < ROUND_UP(d->frac) + ROUND_UP(d->intg) - 1; i++)
    printf("%09d, ", d->buf[i]);
  printf("%09d} */ ", d->buf[i]);
}

/*
  The purpose of all these define wrappers is to get a "call stack"
  whenever some EXPECT_XX generates a failure. A sample error message:

  # .../unittest/gunit/decimal-t.cc:134: FailureValue of: s
  #   Actual: "0"
  # Expected: orig
  # Which is: "1000000000"
  #  arguments were: '999999999', -9, HALF_UP
  # Google Test trace:
  # .../unittest/gunit/decimal-t.cc:387:
  # .../unittest/gunit/decimal-t.cc:686:
 */

#define check_result_code(p1, p2) \
  {                               \
    SCOPED_TRACE("");             \
    do_check_result_code(p1, p2); \
  }

#define print_decimal(p1, p2, p3, p4, p5) \
  {                                       \
    SCOPED_TRACE("");                     \
    do_print_decimal(p1, p2, p3, p4, p5); \
  }

#define test_s2d(p1, p2, p3) \
  {                          \
    SCOPED_TRACE("");        \
    do_test_s2d(p1, p2, p3); \
  }

#define test_d2f(p1, p2) \
  {                      \
    SCOPED_TRACE("");    \
    do_test_d2f(p1, p2); \
  }

#define test_d2b2d(p1, p2, p3, p4, p5) \
  {                                    \
    SCOPED_TRACE("");                  \
    do_test_d2b2d(p1, p2, p3, p4, p5); \
  }

#define test_f2d(p1, p2) \
  {                      \
    SCOPED_TRACE("");    \
    do_test_f2d(p1, p2); \
  }

#define test_ull2d(p1, p2, p3) \
  {                            \
    SCOPED_TRACE("");          \
    do_test_ull2d(p1, p2, p3); \
  }

#define test_ll2d(p1, p2, p3) \
  {                           \
    SCOPED_TRACE("");         \
    do_test_ll2d(p1, p2, p3); \
  }

#define test_d2ull(p1, p2, p3) \
  {                            \
    SCOPED_TRACE("");          \
    do_test_d2ull(p1, p2, p3); \
  }

#define test_d2ll(p1, p2, p3) \
  {                           \
    SCOPED_TRACE("");         \
    do_test_d2ll(p1, p2, p3); \
  }

#define test_da(p1, p2, p3, p4) \
  {                             \
    SCOPED_TRACE("");           \
    do_test_da(p1, p2, p3, p4); \
  }

#define test_ds(p1, p2, p3, p4) \
  {                             \
    SCOPED_TRACE("");           \
    do_test_ds(p1, p2, p3, p4); \
  }

#define test_dc(p1, p2, p3) \
  {                         \
    SCOPED_TRACE("");       \
    do_test_dc(p1, p2, p3); \
  }

#define test_dm(p1, p2, p3, p4) \
  {                             \
    SCOPED_TRACE("");           \
    do_test_dm(p1, p2, p3, p4); \
  }

#define test_dv(p1, p2, p3, p4) \
  {                             \
    SCOPED_TRACE("");           \
    do_test_dv(p1, p2, p3, p4); \
  }

#define test_md(p1, p2, p3, p4) \
  {                             \
    SCOPED_TRACE("");           \
    do_test_md(p1, p2, p3, p4); \
  }

#define test_ro(p1, p2, p3, p4, p5) \
  {                                 \
    SCOPED_TRACE("");               \
    do_test_ro(p1, p2, p3, p4, p5); \
  }

#define test_format(p1, p2, p3, p4, p5) \
  {                                     \
    SCOPED_TRACE("");                   \
    do_test_format(p1, p2, p3, p4, p5); \
  }

#define test_mx(p1, p2, p3) \
  {                         \
    SCOPED_TRACE("");       \
    do_test_mx(p1, p2, p3); \
  }

#define test_pr(p1, p2, p3, p4, p5) \
  {                                 \
    SCOPED_TRACE("");               \
    do_test_pr(p1, p2, p3, p4, p5); \
  }

#define test_widen_fraction(p1, p2, p3) \
  {                                     \
    SCOPED_TRACE("");                   \
    do_test_widen_fraction(p1, p2, p3); \
  }

#define test_sh(p1, p2, p3, p4) \
  {                             \
    SCOPED_TRACE("");           \
    do_test_sh(p1, p2, p3, p4); \
  }

#define test_fr(p1, p2) \
  {                     \
    SCOPED_TRACE("");   \
    do_test_fr(p1, p2); \
  }

void do_check_result_code(int actual, int want) { EXPECT_EQ(want, actual); }

void do_print_decimal(decimal_t *d, const char *orig, int actual, int want,
                      const char *msg) {
  char s[100];
  int slen = sizeof(s);

  if (full) dump_decimal(d);
  decimal2string(d, s, &slen);
  check_result_code(actual, want);
  if (orig) {
    EXPECT_STREQ(orig, s) << " arguments were: " << msg;
  }
}

void test_d2s() {
  char s[100];
  int slen, res;

  /***********************************/
  printf("==== decimal2string ====\n");
  a.buf[0] = 12345;
  a.intg = 5;
  a.frac = 0;
  a.sign = false;
  slen = sizeof(s);
  res = decimal2string(&a, s, &slen);
  dump_decimal(&a);
  printf("  -->  res=%d str='%s' len=%d\n", res, s, slen);

  a.buf[1] = 987000000;
  a.frac = 3;
  slen = sizeof(s);
  res = decimal2string(&a, s, &slen);
  dump_decimal(&a);
  printf("  -->  res=%d str='%s' len=%d\n", res, s, slen);

  a.sign = true;
  slen = sizeof(s);
  res = decimal2string(&a, s, &slen);
  dump_decimal(&a);
  printf("  -->  res=%d str='%s' len=%d\n", res, s, slen);

  slen = 8;
  res = decimal2string(&a, s, &slen);
  dump_decimal(&a);
  printf("  -->  res=%d str='%s' len=%d\n", res, s, slen);

  slen = 5;
  res = decimal2string(&a, s, &slen);
  dump_decimal(&a);
  printf("  -->  res=%d str='%s' len=%d\n", res, s, slen);

  a.buf[0] = 987000000;
  a.frac = 3;
  a.intg = 0;
  slen = sizeof(s);
  res = decimal2string(&a, s, &slen);
  dump_decimal(&a);
  printf("  -->  res=%d str='%s' len=%d\n", res, s, slen);
}

void do_test_s2d(const char *s, const char *orig, int ex) {
  char s1[100];
  int res;
  sprintf(s1, "'%s'", s);
  const char *end = strend(s);
  res = string2decimal(s, &a, &end);
  print_decimal(&a, orig, res, ex, s1);
}

void do_test_d2f(const char *s, int ex) {
  char s1[100];
  double x;
  int res;

  sprintf(s1, "'%s'", s);
  const char *end = strend(s);
  string2decimal(s, &a, &end);
  res = decimal2double(&a, &x);
  if (full) dump_decimal(&a);
  check_result_code(res, ex);
}

void do_test_d2b2d(const char *str, int p, int s, const char *orig, int ex) {
  char s1[100];
  char s2[100 * 2];
  uchar buf[100];
  int res, i, size = decimal_bin_size(p, s);

  sprintf(s1, "'%s'", str);
  const char *end = strend(str);
  string2decimal(str, &a, &end);
  res = decimal2bin(&a, buf, p, s);
  sprintf(s2, "%-31s {%2d, %2d} => res=%d size=%-2d ", s1, p, s, res, size);
  if (full) {
    printf("0x");
    for (i = 0; i < size; i++) printf("%02x", ((uchar *)buf)[i]);
  }
  res = bin2decimal(buf, &a, p, s);
  print_decimal(&a, orig, res, ex, s2);
}

void do_test_f2d(double from, int ex) {
  int res;
  char s1[100];

  res = double2decimal(from, &a);
  sprintf(s1, "%-40.*f => res=%d    ", DBL_DIG - 2, from, res);
  print_decimal(&a, nullptr, res, ex, s1);
}

void do_test_ull2d(ulonglong from, const char *orig, int ex) {
  char s[100];
  char s1[100 * 2];
  int res;

  res = ulonglong2decimal(from, &a);
  longlong10_to_str(from, s, 10);
  sprintf(s1, "%-40s => res=%d    ", s, res);
  print_decimal(&a, orig, res, ex, s1);
}

void do_test_ll2d(longlong from, const char *orig, int ex) {
  char s[100];
  char s1[100 * 2];
  int res;

  res = longlong2decimal(from, &a);
  longlong10_to_str(from, s, -10);
  sprintf(s1, "%-40s => res=%d    ", s, res);
  print_decimal(&a, orig, res, ex, s1);
}

void do_test_d2ull(const char *s, const char *orig, int ex) {
  char s1[100];
  char s2[100 * 2];
  ulonglong x;
  int res;

  const char *end = strend(s);
  string2decimal(s, &a, &end);
  res = decimal2ulonglong(&a, &x);
  if (full) dump_decimal(&a);
  longlong10_to_str(x, s1, 10);
  sprintf(s2, "%-40s => res=%d    %s\n", s, res, s1);
  check_result_code(res, ex);
  if (orig) {
    EXPECT_STREQ(orig, s1) << " arguments were: " << s2;
  }
}

void do_test_d2ll(const char *s, const char *orig, int ex) {
  char s1[100];
  char s2[100 * 2];
  longlong x;
  int res;

  const char *end = strend(s);
  string2decimal(s, &a, &end);
  res = decimal2longlong(&a, &x);
  if (full) dump_decimal(&a);
  longlong10_to_str(x, s1, -10);
  sprintf(s2, "%-40s => res=%d    %s\n", s, res, s1);
  check_result_code(res, ex);
  if (orig) {
    EXPECT_STREQ(orig, s1) << " arguments were: " << s2;
  }
}

void do_test_da(const char *s1, const char *s2, const char *orig, int ex) {
  char s[100];
  int res;
  sprintf(s, "'%s' + '%s'", s1, s2);
  const char *end = strend(s1);
  string2decimal(s1, &a, &end);
  end = strend(s2);
  string2decimal(s2, &b, &end);
  res = decimal_add(&a, &b, &c);
  print_decimal(&c, orig, res, ex, s);
}

void do_test_ds(const char *s1, const char *s2, const char *orig, int ex) {
  char s[100];
  int res;
  sprintf(s, "'%s' - '%s'", s1, s2);
  const char *end = strend(s1);
  string2decimal(s1, &a, &end);
  end = strend(s2);
  string2decimal(s2, &b, &end);
  res = decimal_sub(&a, &b, &c);
  print_decimal(&c, orig, res, ex, s);
}

void do_test_dc(const char *s1, const char *s2, int orig) {
  char s[100];
  int res;
  sprintf(s, "'%s' <=> '%s'", s1, s2);
  const char *end = strend(s1);
  string2decimal(s1, &a, &end);
  end = strend(s2);
  string2decimal(s2, &b, &end);
  res = decimal_cmp(&a, &b);
  EXPECT_EQ(orig, res) << " arguments were: " << s;
}

void do_test_dm(const char *s1, const char *s2, const char *orig, int ex) {
  char s[100];
  int res;
  sprintf(s, "'%s' * '%s'", s1, s2);
  const char *end = strend(s1);
  string2decimal(s1, &a, &end);
  end = strend(s2);
  string2decimal(s2, &b, &end);
  res = decimal_mul(&a, &b, &c);
  print_decimal(&c, orig, res, ex, s);
}

void do_test_dv(const char *s1, const char *s2, const char *orig, int ex) {
  char s[100];
  int res;
  sprintf(s, "'%s' / '%s'", s1, s2);
  const char *end = strend(s1);
  string2decimal(s1, &a, &end);
  end = strend(s2);
  string2decimal(s2, &b, &end);
  res = decimal_div(&a, &b, &c, 5);
  check_result_code(res, ex);
  if (res != E_DEC_DIV_ZERO) print_decimal(&c, orig, res, ex, s);
}

void do_test_md(const char *s1, const char *s2, const char *orig, int ex) {
  char s[100];
  int res;
  sprintf(s, "'%s' %% '%s'", s1, s2);
  const char *end = strend(s1);
  string2decimal(s1, &a, &end);
  end = strend(s2);
  string2decimal(s2, &b, &end);
  res = decimal_mod(&a, &b, &c);
  check_result_code(res, ex);
  if (res != E_DEC_DIV_ZERO) print_decimal(&c, orig, res, ex, s);
}

const char *round_mode[] = {"TRUNCATE", "HALF_EVEN", "HALF_UP", "CEILING",
                            "FLOOR"};

void do_test_ro(const char *s1, int n, decimal_round_mode mode,
                const char *orig, int ex) {
  char s[100];
  int res;
  sprintf(s, "'%s', %d, %s", s1, n, round_mode[mode]);
  const char *end = strend(s1);
  string2decimal(s1, &a, &end);
  res = decimal_round(&a, &b, n, mode);
  print_decimal(&b, orig, res, ex, s);
}

void do_test_format(const char *s1, const char *s2, int n, const char *orig,
                    int ex) {
  char s[200];
  decimal_t a, b, c, d;
  decimal_digit_t buf1[9], buf2[9], buf3[9], buf4[9];
  int res;
  a.buf = buf1;
  b.buf = buf2;
  c.buf = buf3;
  d.buf = buf4;
  a.len = sizeof(buf1) / sizeof(dec1);
  b.len = sizeof(buf2) / sizeof(dec1);
  c.len = sizeof(buf3) / sizeof(dec1);
  d.len = sizeof(buf4) / sizeof(dec1);

  sprintf(s, "'%s' %% '%s'", s1, s2);
  const char *end = strend(s1);
  string2decimal(s1, &a, &end);
  end = strend(s2);
  string2decimal(s2, &b, &end);
  decimal_mod(&a, &b, &c);
  res = decimal_round(&c, &d, n, HALF_UP);
  print_decimal(&d, orig, res, ex, s);
}
void do_test_mx(int precision, int frac, const char *orig) {
  char s[100];
  sprintf(s, "%d, %d", precision, frac);
  max_decimal(precision, frac, &a);
  print_decimal(&a, orig, 0, 0, s);
}

static void do_test_pr(const char *s1, int prec, int dec, const char *orig,
                       int ex) {
  char s[100];
  char s2[100];
  int slen = sizeof(s2);

  sprintf(s, "'%s', %d, %d", s1, prec, dec);
  const char *end = strend(s1);
  string2decimal(s1, &a, &end);
  const int res = decimal2string(&a, s2, &slen, prec, dec);
  check_result_code(res, ex);
  if (orig) {
    EXPECT_STREQ(orig, s2) << " arguments were: " << s;
  }
}

void do_test_widen_fraction(const char *s1, int increase, const char *orig) {
  decimal_t a;
  decimal_digit_t buf1[9];
  a.buf = buf1;
  a.len = array_elements(buf1);

  const char *end = strend(s1);
  string2decimal(s1, &a, &end);
  widen_fraction(a.frac + increase, &a);

  char s[100];
  int slen = sizeof(s);
  int result = decimal2string(&a, s, &slen);
  EXPECT_EQ(result, 0);
  EXPECT_STREQ(orig, s) << " arguments were: " << s1;
}

void do_test_sh(const char *s1, int shift, const char *orig, int ex) {
  char s[100];
  int res;
  sprintf(s, "'%s' %s %d", s1, ((shift < 0) ? ">>" : "<<"), abs(shift));
  const char *end = strend(s1);
  string2decimal(s1, &a, &end);
  res = decimal_shift(&a, shift);
  print_decimal(&a, orig, res, ex, s);
}

void do_test_fr(const char *s1, const char *orig) {
  char s[100];
  sprintf(s, "'%s'", s1);
  const char *end = strend(s1);
  string2decimal(s1, &a, &end);
  a.frac = decimal_actual_fraction(&a);
  print_decimal(&a, orig, 0, 0, s);
}

static void SetupDecimals() {
  a.buf = buf1;
  a.len = sizeof(buf1) / sizeof(dec1);
  b.buf = buf2;
  b.len = sizeof(buf2) / sizeof(dec1);
  c.buf = buf3;
  c.len = sizeof(buf3) / sizeof(dec1);
}

class DecimalTest : public ::testing::Test {
 protected:
  void SetUp() override { SetupDecimals(); }
};

TEST_F(DecimalTest, String2Decimal) {
  test_s2d("12345", "12345", 0);
  test_s2d("12345.", "12345", 0);
  test_s2d("123.45", "123.45", 0);
  test_s2d("-123.45", "-123.45", 0);
  test_s2d(".00012345000098765", "0.00012345000098765", 0);
  test_s2d(".12345000098765", "0.12345000098765", 0);
  test_s2d("-.000000012345000098765", "-0.000000012345000098765", 0);
  test_s2d("1234500009876.5", "1234500009876.5", 0);
  a.len = 1;
  test_s2d("123450000098765", "98765", 2);
  test_s2d("123450.000098765", "123450", 1);
  a.len = sizeof(buf1) / sizeof(dec1);
  test_s2d("123E5", "12300000", 0);
  test_s2d("123E-2", "1.23", 0);
}

TEST_F(DecimalTest, Decimal2Double) {
  test_d2f("12345", 0);
  test_d2f("123.45", 0);
  test_d2f("-123.45", 0);
  test_d2f("0.00012345000098765", 0);
  test_d2f("1234500009876.5", 0);
}

TEST_F(DecimalTest, Double2Decimal) {
  test_f2d(12345, 0);
  test_f2d(1.0 / 3, 0);
  test_f2d(-123.45, 0);
  test_f2d(0.00012345000098765, 0);
  test_f2d(1234500009876.5, 0);
}

TEST_F(DecimalTest, Ulonglong2Decimal) {
  test_ull2d(12345ULL, "12345", 0);
  test_ull2d(0ULL, "0", 0);
  test_ull2d(18446744073709551615ULL, "18446744073709551615", 0);
}

TEST_F(DecimalTest, Decimal2Ulonglong) {
  test_d2ull("12345", "12345", 0);
  test_d2ull("0", "0", 0);
  /* ULLONG_MAX = 18446744073709551615ULL */
  test_d2ull("18446744073709551615", "18446744073709551615", 0);
  test_d2ull("18446744073709551616", "18446744073709551615", 2);
  test_d2ull("-1", "0", 2);
  test_d2ull("1.23", "1", 1);
  test_d2ull("9999999999999999999999999.000", "18446744073709551615", 2);
}

TEST_F(DecimalTest, Longlong2Decimal) {
  test_ll2d(-12345LL, "-12345", 0);
  test_ll2d(-1LL, "-1", 0);
  test_ll2d(-9223372036854775807LL, "-9223372036854775807", 0);
  test_ll2d(9223372036854775808ULL, "-9223372036854775808", 0);
}

TEST_F(DecimalTest, Decimal2Longlong) {
  /* LLONG_MAX = 9223372036854775807LL */
  test_d2ll("18446744073709551615", "9223372036854775807", 2);
  test_d2ll("-1", "-1", 0);
  test_d2ll("-1.23", "-1", 1);
  test_d2ll("-9223372036854775807", "-9223372036854775807", 0);
  test_d2ll("-9223372036854775808", "-9223372036854775808", 0);
  test_d2ll("9223372036854775808", "9223372036854775807", 2);
}

TEST_F(DecimalTest, DoAdd) {
  test_da(".00012345000098765", "123.45", "123.45012345000098765", 0);
  test_da(".1", ".45", "0.55", 0);
  test_da("1234500009876.5", ".00012345000098765",
          "1234500009876.50012345000098765", 0);
  test_da("9999909999999.5", ".555", "9999910000000.055", 0);
  test_da("99999999", "1", "100000000", 0);
  test_da("989999999", "1", "990000000", 0);
  test_da("999999999", "1", "1000000000", 0);
  test_da("12345", "123.45", "12468.45", 0);
  test_da("-12345", "-123.45", "-12468.45", 0);
  test_ds("-12345", "123.45", "-12468.45", 0);
  test_ds("12345", "-123.45", "12468.45", 0);
}

TEST_F(DecimalTest, DoSub) {
  test_ds(".00012345000098765", "123.45", "-123.44987654999901235", 0);
  test_ds("1234500009876.5", ".00012345000098765",
          "1234500009876.49987654999901235", 0);
  test_ds("9999900000000.5", ".555", "9999899999999.945", 0);
  test_ds("1111.5551", "1111.555", "0.0001", 0);
  test_ds(".555", ".555", "0", 0);
  test_ds("10000000", "1", "9999999", 0);
  test_ds("1000001000", ".1", "1000000999.9", 0);
  test_ds("1000000000", ".1", "999999999.9", 0);
  test_ds("12345", "123.45", "12221.55", 0);
  test_ds("-12345", "-123.45", "-12221.55", 0);
  test_da("-12345", "123.45", "-12221.55", 0);
  test_da("12345", "-123.45", "12221.55", 0);
  test_ds("123.45", "12345", "-12221.55", 0);
  test_ds("-123.45", "-12345", "12221.55", 0);
  test_da("123.45", "-12345", "-12221.55", 0);
  test_da("-123.45", "12345", "12221.55", 0);
  test_da("5", "-6.0", "-1.0", 0);
}

TEST_F(DecimalTest, DecimalMul) {
  test_dm("12", "10", "120", 0);
  test_dm("-123.456", "98765.4321", "-12193185.1853376", 0);
  test_dm("-123456000000", "98765432100000", "-12193185185337600000000000", 0);
  test_dm("123456", "987654321", "121931851853376", 0);
  test_dm("123456", "9876543210", "1219318518533760", 0);
  test_dm("123", "0.01", "1.23", 0);
  test_dm("123", "0", "0", 0);
}

TEST_F(DecimalTest, DecimalDiv) {
  test_dv("120", "10", "12.000000000", 0);
  test_dv("123", "0.01", "12300.000000000", 0);
  test_dv("120", "100000000000.00000", "0.000000001200000000", 0);
  test_dv("123", "0", "", 4);
  test_dv("0", "0", "", 4);
  test_dv("-12193185.1853376", "98765.4321", "-123.456000000000000000", 0);
  test_dv("121931851853376", "987654321", "123456.000000000", 0);
  test_dv("0", "987", "0", 0);
  test_dv("1", "3", "0.333333333", 0);
  test_dv("1.000000000000", "3", "0.333333333333333333", 0);
  test_dv("1", "1", "1.000000000", 0);
  test_dv("0.0123456789012345678912345", "9999999999",
          "0.000000000001234567890246913578148141", 0);
  test_dv("10.333000000", "12.34500", "0.837019036046982584042122316", 0);
  test_dv("10.000000000060", "2", "5.000000000030000000", 0);
}

TEST_F(DecimalTest, DecimalMod) {
  test_md("234", "10", "4", 0);
  test_md("234.567", "10.555", "2.357", 0);
  test_md("-234.567", "10.555", "-2.357", 0);
  test_md("234.567", "-10.555", "2.357", 0);
  c.buf[1] = 0x3ABECA;
  test_md("99999999999999999999999999999999999999", "3", "0", 0);
  if (c.buf[1] != 0x3ABECA) {
    ADD_FAILURE() << "overflow " << c.buf[1];
  }
}

TEST_F(DecimalTest, Decimal2BinBin2Decimal) {
  test_d2b2d("-10.55", 4, 2, "-10.55", 0);
  test_d2b2d("0.0123456789012345678912345", 30, 25,
             "0.0123456789012345678912345", 0);
  test_d2b2d("12345", 5, 0, "12345", 0);
  test_d2b2d("12345", 10, 3, "12345.000", 0);
  test_d2b2d("123.45", 10, 3, "123.450", 0);
  test_d2b2d("-123.45", 20, 10, "-123.4500000000", 0);
  test_d2b2d(".00012345000098765", 15, 14, "0.00012345000098", 0);
  test_d2b2d(".00012345000098765", 22, 20, "0.00012345000098765000", 0);
  test_d2b2d(".12345000098765", 30, 20, "0.12345000098765000000", 0);
  test_d2b2d("-.000000012345000098765", 30, 20, "-0.00000001234500009876", 0);
  test_d2b2d("1234500009876.5", 30, 5, "1234500009876.50000", 0);
  test_d2b2d("111111111.11", 10, 2, "11111111.11", 0);
  test_d2b2d("000000000.01", 7, 3, "0.010", 0);
  test_d2b2d("123.4", 10, 2, "123.40", 0);
}

TEST_F(DecimalTest, DecimalCmp) {
  test_dc("12", "13", -1);
  test_dc("13", "12", 1);
  test_dc("-10", "10", -1);
  test_dc("10", "-10", 1);
  test_dc("-12", "-13", 1);
  test_dc("0", "12", -1);
  test_dc("-10", "0", -1);
  test_dc("4", "4", 0);
}

TEST_F(DecimalTest, DecimalRound) {
  test_ro("5678.123451", -4, TRUNCATE, "0", 0);
  test_ro("5678.123451", -3, TRUNCATE, "5000", 0);
  test_ro("5678.123451", -2, TRUNCATE, "5600", 0);
  test_ro("5678.123451", -1, TRUNCATE, "5670", 0);
  test_ro("5678.123451", 0, TRUNCATE, "5678", 0);
  test_ro("5678.123451", 1, TRUNCATE, "5678.1", 0);
  test_ro("5678.123451", 2, TRUNCATE, "5678.12", 0);
  test_ro("5678.123451", 3, TRUNCATE, "5678.123", 0);
  test_ro("5678.123451", 4, TRUNCATE, "5678.1234", 0);
  test_ro("5678.123451", 5, TRUNCATE, "5678.12345", 0);
  test_ro("5678.123451", 6, TRUNCATE, "5678.123451", 0);
  test_ro("-5678.123451", -4, TRUNCATE, "0", 0);
  memset(buf2, 33, sizeof(buf2));
  test_ro("99999999999999999999999999999999999999", -31, TRUNCATE,
          "99999990000000000000000000000000000000", 0);
  test_ro("15.1", 0, HALF_UP, "15", 0);
  test_ro("15.5", 0, HALF_UP, "16", 0);
  test_ro("15.9", 0, HALF_UP, "16", 0);
  test_ro("-15.1", 0, HALF_UP, "-15", 0);
  test_ro("-15.5", 0, HALF_UP, "-16", 0);
  test_ro("-15.9", 0, HALF_UP, "-16", 0);
  test_ro("15.1", 1, HALF_UP, "15.1", 0);
  test_ro("-15.1", 1, HALF_UP, "-15.1", 0);
  test_ro("15.17", 1, HALF_UP, "15.2", 0);
  test_ro("15.4", -1, HALF_UP, "20", 0);
  test_ro("-15.4", -1, HALF_UP, "-20", 0);
  test_ro("5.4", -1, HALF_UP, "10", 0);
  test_ro(".999", 0, HALF_UP, "1", 0);
  memset(buf2, 33, sizeof(buf2));
  test_ro("999999999", -9, HALF_UP, "1000000000", 0);
  test_ro("15.1", 0, HALF_EVEN, "15", 0);
  test_ro("15.5", 0, HALF_EVEN, "16", 0);
  test_ro("14.5", 0, HALF_EVEN, "14", 0);
  test_ro("15.9", 0, HALF_EVEN, "16", 0);
  test_ro("15.1", 0, CEILING, "16", 0);
  test_ro("-15.1", 0, CEILING, "-15", 0);
  test_ro("15.1", 0, FLOOR, "15", 0);
  test_ro("-15.1", 0, FLOOR, "-16", 0);
  test_ro("999999999999999999999.999", 0, CEILING, "1000000000000000000000", 0);
  test_ro("-999999999999999999999.999", 0, FLOOR, "-1000000000000000000000", 0);

  b.buf[0] = DIG_BASE + 1;
  b.buf++;
  test_ro(".3", 0, HALF_UP, "0", 0);
  b.buf--;
  if (b.buf[0] != DIG_BASE + 1) {
    ADD_FAILURE() << "underflow " << b.buf[0];
  }
}

TEST_F(DecimalTest, FormatFunc) {
  test_format("999999999999999999999999999999999999999999999999999999999999999",
              "999999999999999999999999999999999999999999999999999999999999999",
              42, "0.000000000000000000000000000000000000000000", 0);
}

TEST_F(DecimalTest, MaxDecimal) {
  test_mx(1, 1, "0.9");
  test_mx(1, 0, "9");
  test_mx(2, 1, "9.9");
  test_mx(4, 2, "99.99");
  test_mx(6, 3, "999.999");
  test_mx(8, 4, "9999.9999");
  test_mx(10, 5, "99999.99999");
  test_mx(12, 6, "999999.999999");
  test_mx(14, 7, "9999999.9999999");
  test_mx(16, 8, "99999999.99999999");
  test_mx(18, 9, "999999999.999999999");
  test_mx(20, 10, "9999999999.9999999999");
  test_mx(20, 20, "0.99999999999999999999");
  test_mx(20, 0, "99999999999999999999");
  test_mx(40, 20, "99999999999999999999.99999999999999999999");
}

TEST_F(DecimalTest, Decimal2String) {
  test_pr("123.123", 0, 0, "123.123", 0);
  /* For fixed precision, we no longer count the '.' here. */
  test_pr("123.123", 6, 3, "123.123", 0);
  test_pr("123.123", 8, 3, "00123.123", 0);
  test_pr("123.123", 8, 4, "0123.1230", 0);
  test_pr("123.123", 8, 5, "123.12300", 0);
  test_pr("123.123", 8, 2, "000123.12", 1);
  test_pr("123.123", 8, 6, "23.123000", 2);
}

TEST_F(DecimalTest, WidenFraction) {
  test_widen_fraction("123.0", 1, "123.00");
  test_widen_fraction("1234567890.123456789", 1, "1234567890.1234567890");
  test_widen_fraction("123.0", 0, "123.0");
  test_widen_fraction("123.0", 4, "123.00000");
}

TEST_F(DecimalTest, DecimalShift) {
  test_sh("123.123", 1, "1231.23", 0);
  test_sh("123457189.123123456789000", 1, "1234571891.23123456789", 0);
  test_sh("123457189.123123456789000", 4, "1234571891231.23456789", 0);
  test_sh("123457189.123123456789000", 8, "12345718912312345.6789", 0);
  test_sh("123457189.123123456789000", 9, "123457189123123456.789", 0);
  test_sh("123457189.123123456789000", 10, "1234571891231234567.89", 0);
  test_sh("123457189.123123456789000", 17, "12345718912312345678900000", 0);
  test_sh("123457189.123123456789000", 18, "123457189123123456789000000", 0);
  test_sh("123457189.123123456789000", 19, "1234571891231234567890000000", 0);
  test_sh("123457189.123123456789000", 26,
          "12345718912312345678900000000000000", 0);
  test_sh("123457189.123123456789000", 27,
          "123457189123123456789000000000000000", 0);
  test_sh("123457189.123123456789000", 28,
          "1234571891231234567890000000000000000", 0);
  test_sh("000000000000000000000000123457189.123123456789000", 26,
          "12345718912312345678900000000000000", 0);
  test_sh("00000000123457189.123123456789000", 27,
          "123457189123123456789000000000000000", 0);
  test_sh("00000000000000000123457189.123123456789000", 28,
          "1234571891231234567890000000000000000", 0);
  test_sh("123", 1, "1230", 0);
  test_sh("123", 10, "1230000000000", 0);
  test_sh(".123", 1, "1.23", 0);
  test_sh(".123", 10, "1230000000", 0);
  test_sh(".123", 14, "12300000000000", 0);
  test_sh("000.000", 1000, "0", 0);
  test_sh("000.", 1000, "0", 0);
  test_sh(".000", 1000, "0", 0);
  test_sh("1", 1000, "1", 2);
  test_sh("123.123", -1, "12.3123", 0);
  test_sh("123987654321.123456789000", -1, "12398765432.1123456789", 0);
  test_sh("123987654321.123456789000", -2, "1239876543.21123456789", 0);
  test_sh("123987654321.123456789000", -3, "123987654.321123456789", 0);
  test_sh("123987654321.123456789000", -8, "1239.87654321123456789", 0);
  test_sh("123987654321.123456789000", -9, "123.987654321123456789", 0);
  test_sh("123987654321.123456789000", -10, "12.3987654321123456789", 0);
  test_sh("123987654321.123456789000", -11, "1.23987654321123456789", 0);
  test_sh("123987654321.123456789000", -12, "0.123987654321123456789", 0);
  test_sh("123987654321.123456789000", -13, "0.0123987654321123456789", 0);
  test_sh("123987654321.123456789000", -14, "0.00123987654321123456789", 0);
  test_sh("00000087654321.123456789000", -14, "0.00000087654321123456789", 0);
  a.len = 2;
  test_sh("123.123", -2, "1.23123", 0);
  test_sh("123.123", -3, "0.123123", 0);
  test_sh("123.123", -6, "0.000123123", 0);
  test_sh("123.123", -7, "0.0000123123", 0);
  test_sh("123.123", -15, "0.000000000000123123", 0);
  test_sh("123.123", -16, "0.000000000000012312", 1);
  test_sh("123.123", -17, "0.000000000000001231", 1);
  test_sh("123.123", -18, "0.000000000000000123", 1);
  test_sh("123.123", -19, "0.000000000000000012", 1);
  test_sh("123.123", -20, "0.000000000000000001", 1);
  test_sh("123.123", -21, "0", 1);
  test_sh(".000000000123", -1, "0.0000000000123", 0);
  test_sh(".000000000123", -6, "0.000000000000000123", 0);
  test_sh(".000000000123", -7, "0.000000000000000012", 1);
  test_sh(".000000000123", -8, "0.000000000000000001", 1);
  test_sh(".000000000123", -9, "0", 1);
  test_sh(".000000000123", 1, "0.00000000123", 0);
  test_sh(".000000000123", 8, "0.0123", 0);
  test_sh(".000000000123", 9, "0.123", 0);
  test_sh(".000000000123", 10, "1.23", 0);
  test_sh(".000000000123", 17, "12300000", 0);
  test_sh(".000000000123", 18, "123000000", 0);
  test_sh(".000000000123", 19, "1230000000", 0);
  test_sh(".000000000123", 20, "12300000000", 0);
  test_sh(".000000000123", 21, "123000000000", 0);
  test_sh(".000000000123", 22, "1230000000000", 0);
  test_sh(".000000000123", 23, "12300000000000", 0);
  test_sh(".000000000123", 24, "123000000000000", 0);
  test_sh(".000000000123", 25, "1230000000000000", 0);
  test_sh(".000000000123", 26, "12300000000000000", 0);
  test_sh(".000000000123", 27, "123000000000000000", 0);
  test_sh(".000000000123", 28, "0.000000000123", 2);
  test_sh("123456789.987654321", -1, "12345678.998765432", 1);
  test_sh("123456789.987654321", -2, "1234567.899876543", 1);
  test_sh("123456789.987654321", -8, "1.234567900", 1);
  test_sh("123456789.987654321", -9, "0.123456789987654321", 0);
  test_sh("123456789.987654321", -10, "0.012345678998765432", 1);
  test_sh("123456789.987654321", -17, "0.000000001234567900", 1);
  test_sh("123456789.987654321", -18, "0.000000000123456790", 1);
  test_sh("123456789.987654321", -19, "0.000000000012345679", 1);
  test_sh("123456789.987654321", -26, "0.000000000000000001", 1);
  test_sh("123456789.987654321", -27, "0", 1);
  test_sh("123456789.987654321", 1, "1234567900", 1);
  test_sh("123456789.987654321", 2, "12345678999", 1);
  test_sh("123456789.987654321", 4, "1234567899877", 1);
  test_sh("123456789.987654321", 8, "12345678998765432", 1);
  test_sh("123456789.987654321", 9, "123456789987654321", 0);
  test_sh("123456789.987654321", 10, "123456789.987654321", 2);
  test_sh("123456789.987654321", 0, "123456789.987654321", 0);
  a.len = sizeof(buf1) / sizeof(dec1);
}

TEST_F(DecimalTest, DecimalActualFraction) {
  test_fr("1.123456789000000000", "1.123456789");
  test_fr("1.12345678000000000", "1.12345678");
  test_fr("1.1234567000000000", "1.1234567");
  test_fr("1.123456000000000", "1.123456");
  test_fr("1.12345000000000", "1.12345");
  test_fr("1.1234000000000", "1.1234");
  test_fr("1.123000000000", "1.123");
  test_fr("1.12000000000", "1.12");
  test_fr("1.1000000000", "1.1");
  test_fr("1.000000000", "1");
  test_fr("1.0", "1");
  test_fr("10000000000000000000.0", "10000000000000000000");
}

// Some test data from DBT-3.
static const char *decimal_testdata[] = {
    "45983.16", "0.09",     "983",      "0.09",     "36.00",    "45983.16",
    "0.09",     "0.1",      "8.00",     "13309.60", "0.10",     "28955.64",
    "0",        "28.00",    "28955.64", "0.09",     "0",        "24.00",
    "22824.48", "0.10",     "49620.16", "0.07",     "32.00",    "49620.16",
    "0.07",     "0.0",      "38.00",    "44694.46", "0.00",     "45.00",
    "4058",     "54058.05", "0.06",     "058",      "0.06",     "45.00",
    "4058",     "0.0",      "49.00",    "796",      "46796.47", "73426.50",
    "0.08",     "0",        "37.00",    "61998.31", "0.08",     "13608.60",
    "0.07",     "12.00",    "13608.60", "0.07",     "0.0",      "9.00",
    "11594.16", "0.08",     "81639.88", "0",        "46.00",    "81639.88",
    "0.10",     "0",        "28.00",    "31809.96", "0.03",     "73943.82",
    "0.08",     "38.00",    "73943.82", "0.08",     "0.0",      "35.00",
    "43058.75", "0.06",     "6476.15",  "0",        "5.00",     "6476.15",
    "0.04",     "0",        "28.00",    "47227.60", "0.05",     "64605.44",
    "0.02",     "32.00",    "64605.44", "0.02",     "0.0",      "2.00",
    "2210.32",  "0.09",     "6582.96",  "0",        "4.00",     "6582.96",
    "0.09",     "0",        "44.00",    "79059.64", "0.05",     "9159.66",
    "0.04",     "6.00",     "9159.66",  "0.04",     "0.0",      "31.00",
    "40217.23", "0.09",     "47344.32", "0",        "32.00",    "47344.32",
    "0.02",     "0",        "5.00",     "7532.30",  "0.05",     "41.00",
    "28",       "75928.31", "0.09",     "41.00",    "75928.31", "0.09",
    "0.0",      "24.00",    "32410.80", "32410.80", "0.02",     "68065.96",
    "0",        "34.00",    "68065.96", "0.06",     "0",        "7.00",
    "13418.23", "0.06",     "29004.25", "0.06",     "25.00",    "29004.25",
    "0.06",     "0.0",      "34.00",    "65854.94", "0.08",     "47397.28",
    "0",        "28.00",    "47397.28", "0.03",     "0",        "42.00",
    "75043.92", "0.09",     "62105.20", "0.09",     "40.00",    "62105.20",
    "0.09",     "0.0",      "39.00",    "70542.42", "0.05",     "78083.70",
    "0",        "43.00",    "78083.70", "0.05",     "0",        "44.00",
    "84252.52", "0.04",     "53782.08", "0.09",     "44.00",    "53782.08",
    "0.09",     "0.0",      "26.00",    "43383.08", "0.08",     "82746.18",
    "0",        "46.00",    "82746.18", "0.06",     "0",        "32.00",
    "48338.88", "0.07",     "63360.93", "0.01",     "43.00",    "63360.93",
    "0.01",     "0.0",      "40.00",    "54494.40", "0.06",     "21.00",
    "75",       "40675.95", "0",        "21.00",    "40675.95", "0.05",
    "0",        "26.00",    "42995.94", "0.03",     "39353.82", "0.00",
    "22.00",    "39353.82", "0.00",     "0.0",      "21.00",    "27076.98",
    "0.09",     "31.00"};

static void BM_Decimal2Bin_10_2(size_t iters) {
  StopBenchmarkTiming();
  constexpr size_t num_elements = array_elements(decimal_testdata);
  decimal_t decimals[num_elements];
  decimal_digit_t decimal_buf[num_elements][9];

  for (size_t i = 0; i < num_elements; ++i) {
    const char *end = strend(decimal_testdata[i]);
    decimals[i].buf = decimal_buf[i];
    decimals[i].len = array_elements(decimal_buf[i]);
    int res = string2decimal(decimal_testdata[i], &decimals[i], &end);
    ASSERT_EQ(E_DEC_OK, res) << decimal_testdata[i] << " wasn't converted";
  }
  StartBenchmarkTiming();

  int dummy = 0;
  for (size_t i = 0; i < iters; ++i) {
    uchar buf[100];
    decimal2bin(&decimals[i % num_elements], buf, 10, 2);
    dummy += buf[0];
  }

  ASSERT_NE(0, dummy);  // To keep the optimizer from removing the loop.
}
BENCHMARK(BM_Decimal2Bin_10_2)

static void BM_Bin2Decimal_10_2(size_t iters) {
  StopBenchmarkTiming();
  constexpr size_t num_elements = array_elements(decimal_testdata);
  constexpr int bin_size = 5;
  ASSERT_EQ(bin_size, decimal_bin_size(10, 2)) << "Need to adjust bin_size";
  uchar packed_buf[num_elements][bin_size];

  decimal_t decimal;
  decimal_digit_t decimal_buf[9];
  decimal.buf = decimal_buf;
  decimal.len = array_elements(decimal_buf);

  for (size_t i = 0; i < num_elements; ++i) {
    const char *end = strend(decimal_testdata[i]);
    int res = string2decimal(decimal_testdata[i], &decimal, &end);
    ASSERT_EQ(E_DEC_OK, res) << decimal_testdata[i] << " wasn't converted";
    res = decimal2bin(&decimal, packed_buf[i], 10, 2);
    ASSERT_EQ(E_DEC_OK, res)
        << decimal_testdata[i] << " wasn't converted in stage 2";
  }
  StartBenchmarkTiming();

  size_t dummy = 0;
  for (size_t i = 0; i < iters; ++i) {
    bin2decimal(packed_buf[i % num_elements], &decimal, 10, 2);
    dummy += decimal_buf[0];
  }

  ASSERT_NE(static_cast<size_t>(-1),
            dummy);  // To keep the optimizer from removing the loop.
}
BENCHMARK(BM_Bin2Decimal_10_2)

static void BM_Decimal2String(size_t iterations) {
  StopBenchmarkTiming();
  constexpr size_t num_elements = array_elements(decimal_testdata);
  decimal_t decimals[num_elements];
  decimal_digit_t decimal_buf[num_elements][9];

  for (size_t i = 0; i < num_elements; ++i) {
    const char *end = strend(decimal_testdata[i]);
    decimals[i].buf = decimal_buf[i];
    decimals[i].len = array_elements(decimal_buf[i]);
    int res = string2decimal(decimal_testdata[i], &decimals[i], &end);
    ASSERT_EQ(E_DEC_OK, res) << decimal_testdata[i] << " wasn't converted";
  }
  StartBenchmarkTiming();

  for (size_t i = 0; i < iterations; ++i) {
    for (const decimal_t &dec : decimals) {
      char buffer[20];
      int length = sizeof(buffer);
      decimal2string(&dec, buffer, &length);
    }
  }
}
BENCHMARK(BM_Decimal2String)

struct DecimalToStringParam {
  const char *input_string;
  int buffer_size;
  const char *result_string;
  int error_code;
};

class DecimalToStringTest
    : public testing::TestWithParam<DecimalToStringParam> {
 protected:
  void SetUp() override { SetupDecimals(); }
};

TEST_P(DecimalToStringTest, DecimalToString) {
  const char *end = strend(GetParam().input_string);
  EXPECT_EQ(E_DEC_OK, string2decimal(GetParam().input_string, &a, &end));

  char buffer[DECIMAL_MAX_STR_LENGTH + 1];
  int length = GetParam().buffer_size;
  EXPECT_EQ(GetParam().error_code, decimal2string(&a, buffer, &length));
  EXPECT_EQ(strlen(GetParam().result_string), length);
  EXPECT_STREQ(GetParam().result_string, buffer);
}

// Tests how decimal2string() handles the case where the buffer is too small to
// hold the full decimal value.
static const DecimalToStringParam TO_STRING_OVERFLOW_TRUNCATE[] = {
    {"123.456", 2, "3", E_DEC_OVERFLOW},
    {"123.456", 3, "23", E_DEC_OVERFLOW},
    {"123.456", 4, "123", E_DEC_TRUNCATED},
    {"123.456", 5, "123", E_DEC_TRUNCATED},
    {"123.456", 6, "123.4", E_DEC_TRUNCATED},
    {"123.456", 7, "123.45", E_DEC_TRUNCATED},
    {"123.456", 8, "123.456", E_DEC_OK},
    {"1230000", 3, "00", E_DEC_OVERFLOW},
    {"1230000", 4, "000", E_DEC_OVERFLOW},
    {"1230000", 5, "0000", E_DEC_OVERFLOW},
    {"1230000", 6, "30000", E_DEC_OVERFLOW},
    {"0.12345", 2, "0", E_DEC_TRUNCATED},
    {"0.12345", 3, "0", E_DEC_TRUNCATED},
    {"0.12345", 4, "0.1", E_DEC_TRUNCATED},
    {"0.00000", 2, "0", E_DEC_TRUNCATED},
    {"0.00000", 3, "0", E_DEC_TRUNCATED},
    {"0.00000", 4, "0.0", E_DEC_TRUNCATED},
};

INSTANTIATE_TEST_CASE_P(OverflowTruncate, DecimalToStringTest,
                        testing::ValuesIn(TO_STRING_OVERFLOW_TRUNCATE));

}  // namespace decimal_unittest
