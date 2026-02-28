/* Copyright (c) 2018, Oracle and/or its affiliates. All rights reserved.

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

#include "my_config.h"
#include "my_getopt.h"

#include <gtest/gtest.h>

namespace mysys_my_getopt_unittest {

constexpr unsigned long long one_kilo = 1024ULL;
constexpr unsigned long long one_mega = 1024ULL * one_kilo;
constexpr unsigned long long one_giga = 1024ULL * one_mega;
constexpr unsigned long long one_tera = 1024ULL * one_giga;
constexpr unsigned long long one_peta = 1024ULL * one_tera;
constexpr unsigned long long one_exa = 1024ULL * one_peta;

void no_error_report(loglevel, uint, ...) {}
void no_message_hook(loglevel, uint, va_list) {}

class MysysMyGetopTest : public ::testing::Test {
  virtual void SetUp() {
    m_foo = my_getopt_error_reporter;
    my_getopt_error_reporter = &no_error_report;
    m_bar = local_message_hook;
    local_message_hook = &no_message_hook;
  }
  virtual void TearDown() {
    my_getopt_error_reporter = m_foo;
    local_message_hook = m_bar;
  }

 protected:
  int m_error;

 private:
  my_error_reporter m_foo;
  void (*m_bar)(loglevel, uint, va_list);
};

TEST_F(MysysMyGetopTest, GarbageInput) {
  eval_num_suffix<unsigned long long>("X", &m_error, "");
  EXPECT_EQ(m_error, 1);

  eval_num_suffix<unsigned long long>("+X", &m_error, "");
  EXPECT_EQ(m_error, 1);

  eval_num_suffix<unsigned long long>("-X", &m_error, "");
  EXPECT_EQ(m_error, 1);

  eval_num_suffix<unsigned long long>("0X", &m_error, "");
  EXPECT_EQ(m_error, 1);

  eval_num_suffix<unsigned long long>("0Xm", &m_error, "");
  EXPECT_EQ(m_error, 1);

  eval_num_suffix<unsigned long long>("99999999999999999999999", &m_error, "");
  EXPECT_EQ(m_error, 1);

  eval_num_suffix<unsigned long long>("99999999999999999999999k", &m_error, "");
  EXPECT_EQ(m_error, 1);

  eval_num_suffix<long long>("X", &m_error, "");
  EXPECT_EQ(m_error, 1);

  eval_num_suffix<long long>("+X", &m_error, "");
  EXPECT_EQ(m_error, 1);

  eval_num_suffix<long long>("-X", &m_error, "");
  EXPECT_EQ(m_error, 1);

  eval_num_suffix<long long>("0X", &m_error, "");
  EXPECT_EQ(m_error, 1);

  eval_num_suffix<long long>("0Xm", &m_error, "");
  EXPECT_EQ(m_error, 1);

  eval_num_suffix<long long>("-99999999999999999999999", &m_error, "");
  EXPECT_EQ(m_error, 1);

  eval_num_suffix<long long>("-99999999999999999999999k", &m_error, "");
  EXPECT_EQ(m_error, 1);
}

TEST_F(MysysMyGetopTest, Unsigned) {
  unsigned long long result;
  result = eval_num_suffix<unsigned long long>("16E", &m_error, "");
  EXPECT_EQ(m_error, 1);

  result = eval_num_suffix<unsigned long long>("1t", &m_error, "");
  EXPECT_EQ(result, one_tera);

  result = eval_num_suffix<unsigned long long>("1000000E", &m_error, "");
  EXPECT_EQ(result, 0);
  EXPECT_EQ(m_error, 1);

  result =
      eval_num_suffix<unsigned long long>("18446744073709551615", &m_error, "");
  EXPECT_EQ(m_error, 0);

  result =
      eval_num_suffix<unsigned long long>("18446744073709551616", &m_error, "");
  EXPECT_EQ(m_error, 1);
}

TEST_F(MysysMyGetopTest, UnsignedLoop) {
  unsigned long long result;
  char buffer[30];

  for (unsigned long long counter = 0; counter < 100ULL; ++counter) {
    sprintf(buffer, "%lluE", counter);
    result = eval_num_suffix<unsigned long long>(buffer, &m_error, "");
    if (counter >= 16ULL) {
      EXPECT_EQ(result, 0);
      EXPECT_EQ(m_error, 1);
    } else {
      EXPECT_EQ(result, counter * one_exa) << counter;
      EXPECT_EQ(m_error, 0);
    }
  }
}

// -9223372036854775808 .. 9223372036854775807
TEST_F(MysysMyGetopTest, Signed) {
  long long result;
  result = eval_num_suffix<long long>("-1t", &m_error, "");
  EXPECT_EQ(result, -1024LL * 1024LL * 1024LL * 1024LL);
  EXPECT_EQ(m_error, 0);

  result = eval_num_suffix<long long>("-1E", &m_error, "");
  EXPECT_EQ(result, -one_exa);
  EXPECT_EQ(m_error, 0);

  result = eval_num_suffix<long long>("-1000000E", &m_error, "");
  EXPECT_EQ(result, 0);
  EXPECT_EQ(m_error, 1);

  result = eval_num_suffix<long long>("-9223372036854775808", &m_error, "");
  EXPECT_EQ(result, LLONG_MIN);
  EXPECT_EQ(m_error, 0);

  result = eval_num_suffix<long long>("-9223372036854775808k", &m_error, "");
  EXPECT_EQ(result, 0);
  EXPECT_EQ(m_error, 1);

  result = eval_num_suffix<long long>("-9223372036854775807", &m_error, "");
  EXPECT_EQ(result, LLONG_MIN + 1);
  EXPECT_EQ(m_error, 0);
}

TEST_F(MysysMyGetopTest, SignedLoop) {
  long long counter;
  long long result;
  char buffer[30];

  for (counter = 0; counter < 100LL; ++counter) {
    sprintf(buffer, "-%lldE", counter);
    result = eval_num_suffix<long long>(buffer, &m_error, "");
    if (counter >= 9) {
      EXPECT_EQ(result, 0);
      EXPECT_EQ(m_error, 1);
    } else {
      EXPECT_EQ(result, (longlong)(-counter * one_exa)) << counter;
      EXPECT_EQ(m_error, 0);
    }
  }
  for (counter = 0; counter < 100LL; ++counter) {
    sprintf(buffer, "%+lldE", counter);
    result = eval_num_suffix<long long>(buffer, &m_error, "");
    if (counter >= 8) {
      EXPECT_EQ(result, 0);
      EXPECT_EQ(m_error, 1);
    } else {
      EXPECT_EQ(result, counter * one_exa) << counter;
      EXPECT_EQ(m_error, 0);
    }
  }
}

}  // namespace mysys_my_getopt_unittest
