/* Copyright (c) 2012, 2017, Oracle and/or its affiliates. All rights reserved.

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

extern CHARSET_INFO *system_charset_info;

TEST(StringTest, EmptyString) {
  String s;
  const uint32 len = 0;
  EXPECT_EQ(len, s.length());
  EXPECT_EQ(len, s.alloced_length());
}

TEST(StringTest, ShrinkString) {
  const uint32 len = 3;
  char foo[len] = {'a', 'b', 0};
  String foos(foo, len, &my_charset_bin);
  foos.shrink(1);
  EXPECT_EQ(len, foos.length());
  EXPECT_STREQ("ab", foo);
}

TEST(StringDeathTest, AppendEmptyString) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe";
  String tbl_name;
  const char db_name[] = "aaaaaaa";
  const char table_name[] = "";
  tbl_name.append(String(db_name, system_charset_info));
  tbl_name.append('.');
  tbl_name.append(String(table_name, system_charset_info));
  // We now have eight characters, c_ptr() is not safe.
#ifndef DBUG_OFF
  EXPECT_DEATH_IF_SUPPORTED(tbl_name.c_ptr(), ".*m_alloced_length >= .*");
#endif
  EXPECT_STREQ("aaaaaaa.", tbl_name.c_ptr_safe());
}

TEST(StringTest, StringBuffer) {
  StringBuffer<3> sb("abc", 3, &my_charset_bin);
  sb.append("def");
  EXPECT_STREQ("abcdef", sb.c_ptr());
  EXPECT_EQ(6u, sb.length());
}
