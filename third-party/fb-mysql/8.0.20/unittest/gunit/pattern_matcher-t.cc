/*
   Copyright (c) 2017, Oracle and/or its affiliates. All rights reserved.

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
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include <gtest/gtest.h>
#include <my_sys.h>
#include "client/pattern_matcher.cc"

namespace mysql_client_test_ns {

class PatternMatcherTest : public ::testing::Test {
 public:
  CHARSET_INFO *cs_info;

 protected:
  virtual void SetUp() {
    MY_CHARSET_LOADER loader;
    my_charset_loader_init_mysys(&loader);
    cs_info = my_collation_get_by_name(&loader, "utf8mb4_0900_ai_ci", MYF(0));
  }
};

/** Verifies that pattern list parsing is done correctly */
TEST_F(PatternMatcherTest, PatternParser) {
  Pattern_matcher matcher;

  EXPECT_EQ((size_t)0, matcher.add_patterns(":::"));
  EXPECT_EQ((size_t)1, matcher.add_patterns(":First:"));
  EXPECT_EQ((size_t)2, matcher.add_patterns("::First*:Second:"));
  EXPECT_EQ((size_t)3, matcher.add_patterns(":1:Second::Third:"));
}

/** Verifies that default set of patterns is working */
TEST_F(PatternMatcherTest, DefaultPatterns) {
  Pattern_matcher matcher;
  matcher.add_patterns("*IDENTIFIED*:*PASSWORD*");

  // positive tests - text should be matched
  EXPECT_TRUE(matcher.is_matching("set password = 'mypass';", cs_info));
  EXPECT_TRUE(matcher.is_matching("SET PASSWORD = 'mypass';", cs_info));
  EXPECT_TRUE(matcher.is_matching(
      "create user 'myuser'@'localhost' identified by 'mypass';", cs_info));
  EXPECT_TRUE(matcher.is_matching(
      "CREATE USER 'myuser'@'localhost' IDENTIFIED BY 'mypass';", cs_info));

  // negative tests - text mustn't match
  EXPECT_FALSE(matcher.is_matching("SELECT * FROM my_table;", cs_info));
  EXPECT_FALSE(matcher.is_matching("DROP TABLE IF EXISTS my_table", cs_info));
  EXPECT_FALSE(matcher.is_matching(
      "UPDATE my_table SET name='Sakila' WHERE id=1", cs_info));
  EXPECT_FALSE(matcher.is_matching(
      "INSERT INTO my_table (my_col1,my_col2) VALUES(1,2);", cs_info));
  EXPECT_FALSE(matcher.is_matching(
      "GRANT ALL PRIVILEGES ON *.* TO 'myuser'@'localhost';", cs_info));
}

/** Various wildcard pattern testing */
TEST_F(PatternMatcherTest, WildCard) {
  Pattern_matcher matcher;

  // ========== SINGLE CHARACTER PATTERNS ========== //

  matcher.add_patterns("??SQL");
  EXPECT_TRUE(matcher.is_matching("MySQL", cs_info));
  EXPECT_TRUE(matcher.is_matching("NoSQL", cs_info));
  EXPECT_FALSE(matcher.is_matching("SQL", cs_info));
  EXPECT_FALSE(matcher.is_matching("xSQL", cs_info));
  EXPECT_FALSE(matcher.is_matching("123SQL", cs_info));
  matcher.clear();

  matcher.add_patterns("S?L");
  EXPECT_TRUE(matcher.is_matching("SQL", cs_info));
  EXPECT_TRUE(matcher.is_matching("STL", cs_info));
  EXPECT_FALSE(matcher.is_matching("SL", cs_info));
  EXPECT_FALSE(matcher.is_matching("S123L", cs_info));
  matcher.clear();

  // ========== MULTI CHARACTER PATTERNS =========== //

  matcher.add_patterns("My*");
  EXPECT_TRUE(matcher.is_matching("My", cs_info));
  EXPECT_TRUE(matcher.is_matching("MySQL", cs_info));
  EXPECT_FALSE(matcher.is_matching("y123", cs_info));
  matcher.clear();

  matcher.add_patterns("FROM *-users");
  EXPECT_TRUE(matcher.is_matching("FROM admin-users", cs_info));
  EXPECT_TRUE(matcher.is_matching("FROM regular-users", cs_info));
  EXPECT_FALSE(matcher.is_matching("FROM users", cs_info));
  matcher.clear();
}

}  // namespace mysql_client_test_ns
