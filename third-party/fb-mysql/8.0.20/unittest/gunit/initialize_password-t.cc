/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <gtest/gtest.h>

#include "sql/sql_initialize.cc"

namespace initialize_password_unittest {

static const char *null_s = nullptr;

TEST(initialize_password, random_pwd_10chars) {
  char pass[12];
  static const char chars[] = LOWCHARS SYMCHARS UPCHARS NUMCHARS;

  memset(pass, 0, sizeof(pass));
  bool failed = ::generate_password(&pass[1], 10);
  pass[11] = 0;

  EXPECT_EQ(failed, false);
  EXPECT_EQ(pass[0], 0);
  for (char *ptr = &pass[1]; *ptr; ptr++) {
    const char *s = strchr(chars, *ptr);
    EXPECT_NE(s, null_s);
  }
}

TEST(initialize_password, random_pwd_0) {
  char pass[11];

  memset(pass, 0, sizeof(pass));
  bool failed = ::generate_password(&pass[1], 0);
  EXPECT_EQ(failed, false);

  for (unsigned inx = 0; inx < sizeof(pass); inx++) EXPECT_EQ(pass[inx], 0);
}

TEST(initialize_password, random_pwd_1) {
  char pass[11];
  static const char chars[] = LOWCHARS SYMCHARS UPCHARS NUMCHARS;

  memset(pass, 0, sizeof(pass));
  bool failed = ::generate_password(&pass[1], 1);
  EXPECT_EQ(failed, false);

  EXPECT_EQ(pass[0], 0);

  const char *s = strchr(chars, pass[1]);
  EXPECT_NE(s, null_s);

  for (unsigned inx = 2; inx < sizeof(pass); inx++) EXPECT_EQ(pass[inx], 0);
}

TEST(initialize_password, random_pwd_2) {
  char pass[11];
  static const char chars[] = LOWCHARS SYMCHARS UPCHARS NUMCHARS;
  unsigned inx;

  memset(pass, 0, sizeof(pass));
  bool failed = ::generate_password(&pass[1], 2);

  EXPECT_EQ(failed, false);
  EXPECT_EQ(pass[0], 0);

  for (inx = 0; inx < 2; inx++) {
    const char *s = strchr(chars, pass[1 + inx]);
    EXPECT_NE(s, null_s);
  }

  for (inx = 3; inx < sizeof(pass); inx++) EXPECT_EQ(pass[inx], 0);
}

TEST(initialize_password, random_pwd_3) {
  char pass[11];
  static const char chars[] = LOWCHARS SYMCHARS UPCHARS NUMCHARS;
  unsigned inx;

  memset(pass, 0, sizeof(pass));
  bool failed = ::generate_password(&pass[1], 3);
  EXPECT_EQ(failed, false);

  EXPECT_EQ(pass[0], 0);

  for (inx = 0; inx < 3; inx++) {
    const char *s = strchr(chars, pass[1 + inx]);
    EXPECT_NE(s, null_s);
  }

  for (inx = 4; inx < sizeof(pass); inx++) EXPECT_EQ(pass[inx], 0);
}

TEST(initialize_password, random_pwd_4) {
  char pass[11];
  static const char chars[] = LOWCHARS SYMCHARS UPCHARS NUMCHARS;
  unsigned inx;

  memset(pass, 0, sizeof(pass));
  bool failed = ::generate_password(&pass[1], 4);
  EXPECT_EQ(failed, false);

  EXPECT_EQ(pass[0], 0);

  for (inx = 0; inx < 4; inx++) {
    const char *s = strchr(chars, pass[1 + inx]);
    EXPECT_NE(s, null_s);
  }

  for (inx = 5; inx < sizeof(pass); inx++) EXPECT_EQ(pass[inx], 0);
}

TEST(initialize_password, strong_pwd_10_chars) {
  char pass[12];
  static const char chars[] = LOWCHARS SYMCHARS UPCHARS NUMCHARS;
  static const char low_chars[] = LOWCHARS;
  static const char up_chars[] = UPCHARS;
  static const char sym_chars[] = SYMCHARS;
  static const char num_chars[] = NUMCHARS;
  bool had_low = false, had_up = false, had_sym = false, had_num = false;

  memset(pass, 0, sizeof(pass));
  bool failed = ::generate_password(&pass[1], 10);
  EXPECT_EQ(failed, false);

  EXPECT_EQ(pass[0], 0);
  EXPECT_EQ(pass[11], 0);
  for (char *ptr = &pass[1]; *ptr; ptr++) {
    const char *s = strchr(chars, *ptr);
    EXPECT_NE(s, null_s);

    if (!had_low && nullptr != strchr(low_chars, *ptr))
      had_low = true;
    else if (!had_up && nullptr != strchr(up_chars, *ptr))
      had_up = true;
    else if (!had_sym && nullptr != strchr(sym_chars, *ptr))
      had_sym = true;
    else if (!had_num && nullptr != strchr(num_chars, *ptr))
      had_num = true;
  }

  EXPECT_EQ(had_low, true);
  EXPECT_EQ(had_up, true);
  EXPECT_EQ(had_sym, true);
  EXPECT_EQ(had_num, true);
}

TEST(initialize_password, strong_pwd_4_chars) {
  char pass[12];
  static const char chars[] = LOWCHARS SYMCHARS UPCHARS NUMCHARS;
  static const char low_chars[] = LOWCHARS;
  static const char up_chars[] = UPCHARS;
  static const char sym_chars[] = SYMCHARS;
  static const char num_chars[] = NUMCHARS;
  bool had_low = false, had_up = false, had_sym = false, had_num = false;

  memset(pass, 0, sizeof(pass));
  bool failed = ::generate_password(&pass[1], 4);
  EXPECT_EQ(failed, false);

  EXPECT_EQ(pass[0], 0);
  EXPECT_EQ(pass[5], 0);
  for (char *ptr = &pass[1]; *ptr; ptr++) {
    const char *s = strchr(chars, *ptr);
    EXPECT_NE(s, null_s);

    if (!had_low && nullptr != strchr(low_chars, *ptr))
      had_low = true;
    else if (!had_up && nullptr != strchr(up_chars, *ptr))
      had_up = true;
    else if (!had_sym && nullptr != strchr(sym_chars, *ptr))
      had_sym = true;
    else if (!had_num && nullptr != strchr(num_chars, *ptr))
      had_num = true;
  }

  EXPECT_EQ(had_low, true);
  EXPECT_EQ(had_up, true);
  EXPECT_EQ(had_sym, true);
  EXPECT_EQ(had_num, true);
}

}  // namespace initialize_password_unittest
