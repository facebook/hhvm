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

#include <string>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "plugin/ddl_rewriter/ddl_rewriter.h"

namespace ddl_rewriter_unittest {

class DDL_rewriter_test : public ::testing::Test {
 public:
  DDL_rewriter_test() = default;
  ~DDL_rewriter_test() = default;
};

struct Query_pair {
  const std::string query;
  const std::string rewritten_query;
};

std::vector<Query_pair> basic_queries{
    {"CREATE TABLE t(i int)", "CREATE TABLE t(i int)"},
    {"CREATE TABLE t(i int) ENCRYPTION='N'", "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int)\n"
     "ENCRYPTION='N'\n"
     "ENGINE=MyISAM",
     "CREATE TABLE t(i int) ENGINE=MyISAM"}};

std::vector<Query_pair> case_sensitivity_queries{
    {"create TABLE t(i int) ENCRYPTION = 'N'", "create TABLE t(i int) "},
    {"CREATE TABLE t(i int) ENCRYPTION = 'N'", "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int) encryption = 'N'", "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int) ENCRYPTION = 'n'", "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int) ENCRYPTION = 'Y'", "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int) ENCRYPTION = 'y'", "CREATE TABLE t(i int) "},

    {"create TABLE t(i int) DATA DIRECTORY = '\\tmp'",
     "create TABLE t(i int) "},
    {"CREATE TABLE t(i int) DATA DIRECTORY = '\\tmp'",
     "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int) data directory = '\\tmp'",
     "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int) DATA DIRECTORY = '\\tmp'",
     "CREATE TABLE t(i int) "},

    {"create TABLE t(i int) INDEX DIRECTORY = '\\tmp'",
     "create TABLE t(i int) "},
    {"CREATE TABLE t(i int) INDEX DIRECTORY = '\\tmp'",
     "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int) index directory = '\\tmp'",
     "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int) INDEX DIRECTORY = '\\tmp'",
     "CREATE TABLE t(i int) "}};

std::vector<Query_pair> quoting_queries{
    {"CREATE TABLE t(i int) ENCRYPTION = 'N'", "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int) ENCRYPTION = \"N\"", "CREATE TABLE t(i int) "},

    {"CREATE TABLE t(i int) DATA DIRECTORY = '\\tmp'",
     "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int) DATA DIRECTORY = \"\\tmp\"",
     "CREATE TABLE t(i int) "},

    {"CREATE TABLE t(i int) INDEX DIRECTORY = '\\tmp'",
     "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int) INDEX DIRECTORY = \"\\tmp\"",
     "CREATE TABLE t(i int) "}};

std::vector<Query_pair> clause_separation_queries{
    {"CREATE TABLE t(i int) ENCRYPTION = 'N' DATA DIRECTORY = '\\tmp' INDEX "
     "DIRECTORY = '\\tmp'",
     "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int)  ENCRYPTION = 'N'  DATA DIRECTORY = '\\tmp'  INDEX "
     "DIRECTORY = '\\tmp'",
     "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int)  ENCRYPTION = 'N',DATA DIRECTORY = '\\tmp',INDEX "
     "DIRECTORY = '\\tmp'",
     "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int)  ENCRYPTION = 'N' , DATA DIRECTORY = '\\tmp' , "
     "INDEX DIRECTORY = '\\tmp'",
     "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int)  ENCRYPTION = 'N'  ,  DATA DIRECTORY = '\\tmp'  ,  "
     "INDEX DIRECTORY = '\\tmp'",
     "CREATE TABLE t(i int) "}};

std::vector<Query_pair> clause_assignment_queries{
    {"CREATE TABLE t(i int) ENCRYPTION 'N'", "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int) ENCRYPTION  'N'", "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int) ENCRYPTION='N'", "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int) ENCRYPTION ='N'", "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int) ENCRYPTION= 'N'", "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int) ENCRYPTION = 'N'", "CREATE TABLE t(i int) "},
    {"CREATE TABLE t(i int) ENCRYPTION  =  'N'", "CREATE TABLE t(i int) "}};

std::vector<Query_pair> multi_line_queries{
    {"CREATE TABLE t(i int)\n"
     "ENCRYPTION = 'N' \n"
     "DATA DIRECTORY = '\\tmp' INDEX DIRECTORY = '\\tmp',\n"
     "ENGINE = MyISAM",
     "CREATE TABLE t(i int) ENGINE = MyISAM"},
    {"CREATE TABLE t(i int)\n"
     "\n"
     "ENCRYPTION = 'N' \n"
     ",\n"
     "DATA DIRECTORY\n"
     "=\n"
     "'\\tm\n"
     "p', INDEX\n"
     "DIRECTORY =\n"
     "'\\tmp',\n"
     "ENGINE = MyISAM",
     "CREATE TABLE t(i int) ENGINE = MyISAM"}};

std::vector<Query_pair> partitioned_table_queries{
    {"CREATE TABLE t (i int PRIMARY KEY) ENGINE = InnoDB\n"
     "  DATA DIRECTORY = '/tmp'\n"
     "  ENCRYPTION = 'y'\n"
     "  PARTITION BY LIST (i) (\n"
     "    PARTITION p0 VALUES IN (0) ENGINE = InnoDB,\n"
     "    PARTITION p1 VALUES IN (1)\n"
     "    DATA DIRECTORY = '/tmp',\n"
     "    ENGINE = InnoDB\n"
     "  )",
     "CREATE TABLE t (i int PRIMARY KEY) ENGINE = InnoDB PARTITION BY LIST (i) "
     "(\n"
     "    PARTITION p0 VALUES IN (0) ENGINE = InnoDB,\n"
     "    PARTITION p1 VALUES IN (1) ENGINE = InnoDB\n"
     "  )"}};

std::vector<Query_pair> false_pos_queries{
    {"CREATE TABLE t(i int) /* ENCRYPTION='N' */",
     "CREATE TABLE t(i int) /* */"},
    {"CREATE TABLE t(`i ENCRYPTION='N' i` int)", "CREATE TABLE t(`i i` int)"}};

std::vector<Query_pair> missing_rewrite_queries{
    {" CREATE TABLE t(i int) ENCRYPTION='N'",
     " CREATE TABLE t(i int) ENCRYPTION='N'"}};

void test_queries(const std::vector<Query_pair> &queries) {
  for (auto &q : queries) {
    std::string rewrite_result;
    bool expect_rewrite = (q.query != q.rewritten_query);
    EXPECT_EQ(query_rewritten(q.query, &rewrite_result), expect_rewrite);
    if (expect_rewrite) {
      EXPECT_EQ(rewrite_result, q.rewritten_query);
    }
  }
}

TEST_F(DDL_rewriter_test, DDLQueryTest) {
  test_queries(basic_queries);
  test_queries(case_sensitivity_queries);
  test_queries(quoting_queries);
  test_queries(clause_separation_queries);
  test_queries(clause_assignment_queries);
  test_queries(multi_line_queries);
  test_queries(partitioned_table_queries);
  test_queries(missing_rewrite_queries);
  test_queries(false_pos_queries);
}

}  // namespace ddl_rewriter_unittest
