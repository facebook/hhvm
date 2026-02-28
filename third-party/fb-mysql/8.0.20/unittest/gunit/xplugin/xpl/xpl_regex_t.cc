/*
 * Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2.0,
 * as published by the Free Software Foundation.
 *
 * This program is also distributed with certain software (including
 * but not limited to OpenSSL) that is licensed under separate terms,
 * as designated in a particular file or component or in included license
 * documentation.  The authors of MySQL hereby grant you an additional
 * permission to link the program and your derivative works with the
 * separately licensed software that they have included with MySQL.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License, version 2.0, for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include <gmock/gmock.h>

#include "plugin/x/src/xpl_regex.h"

namespace xpl {
namespace test {

struct Param_regex_match {
  bool expect;
  std::string value;
};

class Regex_match_test : public testing::TestWithParam<Param_regex_match> {
 public:
  const Regex m_re{"DECIMAL(?:\\([0-9]+(?:,[0-9]+)?\\))?"};
};

TEST_P(Regex_match_test, regex_match) {
  const auto &param = GetParam();
  EXPECT_EQ(param.expect, m_re.match(param.value.c_str()));
}

Param_regex_match regex_match_param[] = {
    {true, "DECIMAL"},
    {true, "decimal"},
    {true, "decimal(10)"},
    {true, "decimal(10,5)"},
    {false, "DEC"},
    {false, "decimal()"},
    {false, "decimal(abc)"},
    {false, "decimal(a,c)"},
    {false, "decimal(10,5,1)"},
    {false, "decimal(10)(5)"},
    {false, "decimal(10.5)"},
    {false, "(10,5)"},
    {false, " decimal(10,5)"},
    {false, "decimal (10,5)"},
    {false, "decimal( 10,5)"},
    {false, "decimal(10 ,5)"},
    {false, "decimal(10, 5)"},
    {false, "decimal(10,5 )"},
    {false, "decimal(10,5) "},
};

INSTANTIATE_TEST_CASE_P(regex_match_test, Regex_match_test,
                        testing::ValuesIn(regex_match_param));

struct Param_regex_match_groups {
  std::vector<std::string> expect_groups;
  std::string value;
};

class Regex_match_groups_test
    : public testing::TestWithParam<Param_regex_match_groups> {
 public:
  const Regex m_re{
      "(INT)|"
      "(CHAR|TEXT)(?:\\(([0-9]+)\\))?(?: CHARSET \\w+)?(?: COLLATE \\w+)?|"
      "(DECIMAL)(?:\\(([0-9]+)(?:,([0-9]+))?\\))?|"
      "\\w+(?:\\(([0-9]+)(?:,([0-9]+))?\\))?( UNSIGNED)?"};
};

TEST_P(Regex_match_groups_test, regex_match_groups) {
  const auto &param = GetParam();
  Regex::Group_list groups;
  EXPECT_TRUE(m_re.match_groups(param.value.c_str(), &groups));
  EXPECT_EQ(param.expect_groups, groups);
}

Param_regex_match_groups regex_match_groups_param[] = {
    {{"int", "int"}, "int"},
    {{"char(5)", "char", "5"}, "char(5)"},
    {{"char", "char"}, "char"},
    {{"text(64)", "text", "64"}, "text(64)"},
    {{"text", "text"}, "text"},
    {{"decimal(10,5)", "decimal", "10", "5"}, "decimal(10,5)"},
    {{"decimal(10)", "decimal", "10"}, "decimal(10)"},
    {{"decimal", "decimal"}, "decimal"},
    {{"qwe(10,5) unsigned", "10", "5", " unsigned"}, "qwe(10,5) unsigned"},
    {{"qwe(10) unsigned", "10", " unsigned"}, "qwe(10) unsigned"},
    {{"char(20) charset latin1", "char", "20"}, "char(20) charset latin1"},
    {{"text(30) charset latin1 collate latin1_bin", "text", "30"},
     "text(30) charset latin1 collate latin1_bin"},
};

INSTANTIATE_TEST_CASE_P(regex_match_groups_test, Regex_match_groups_test,
                        testing::ValuesIn(regex_match_groups_param));

}  // namespace test
}  // namespace xpl
