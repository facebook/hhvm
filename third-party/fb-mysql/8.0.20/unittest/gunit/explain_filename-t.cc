/*
   Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.

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

/** Unit test case for the function explain_filename(). */
#include "my_config.h"

#include <gtest/gtest.h>

#include "mysqld_error.h"
#include "sql/derror.h"
#include "sql/sql_class.h"
#include "sql/sql_locale.h"
#include "sql/sql_table.h"

namespace explain_filename_unittest {

const int BUFLEN = 1000;

char to[BUFLEN];
char from[BUFLEN];

class PartitionTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    // Save global settings.
    m_charset = system_charset_info;
    m_locale = my_default_lc_messages;

    system_charset_info = &my_charset_utf8_bin;
    my_default_lc_messages = &my_locale_en_US;

    /* Populate the necessary error messages */
    MY_LOCALE_ERRMSGS *errmsgs = my_default_lc_messages->errmsgs;
    EXPECT_FALSE(errmsgs->replace_msg(ER_DATABASE_NAME, "Database"));
    EXPECT_FALSE(errmsgs->replace_msg(ER_TABLE_NAME, "Table"));
    EXPECT_FALSE(errmsgs->replace_msg(ER_PARTITION_NAME, "Partition"));
    EXPECT_FALSE(errmsgs->replace_msg(ER_SUBPARTITION_NAME, "Subpartition"));
    EXPECT_FALSE(errmsgs->replace_msg(ER_TEMPORARY_NAME, "Temporary"));
    EXPECT_FALSE(errmsgs->replace_msg(ER_RENAMED_NAME, "Renamed"));
  }

  virtual void TearDown() {
    // Restore global settings.
    system_charset_info = m_charset;
    my_default_lc_messages = m_locale;
  }

 private:
  CHARSET_INFO *m_charset;
  MY_LOCALE *m_locale;
};

void test_1(const char *in, const char *exp, enum_explain_filename_mode mode) {
  char out[BUFLEN];

  size_t len1 = explain_filename(nullptr, in, out, BUFLEN, mode);

  /* expected output and actual output must be same */
  bool pass = (strcmp(exp, out) == 0);

  /* length returned by explain_filename is fine */
  bool length = (len1 == strlen(exp));

  EXPECT_EQ((pass && length), true);
  if (pass && length) {
    // pass
  } else {
    ADD_FAILURE() << "input file name: '" << in << "' explain output: '" << out
                  << "'" << std::endl;
  }
}

TEST_F(PartitionTest, ExplainFilename) {
  test_1("test/t1.ibd", "Database `test`, Table `t1.ibd`", EXPLAIN_ALL_VERBOSE);

  test_1("test/t1.ibd", "`test`.`t1.ibd`", EXPLAIN_PARTITIONS_VERBOSE);

  test_1("test/t1.ibd", "`test`.`t1.ibd`", EXPLAIN_PARTITIONS_AS_COMMENT);

  test_1("test/t1#TMP#", "Database `test`, Table `t1#TMP#`",
         EXPLAIN_ALL_VERBOSE);

  test_1("test/#sql-2882.ibd", "Database `test`, Table `#sql-2882.ibd`",
         EXPLAIN_ALL_VERBOSE);

  test_1("test/t1#REN#", "Database `test`, Table `t1#REN#`",
         EXPLAIN_ALL_VERBOSE);

  test_1("test/t1@0023REN@0023", "Database `test`, Table `t1#REN#`",
         EXPLAIN_ALL_VERBOSE);

  test_1("test/t1#p#p1", "Database `test`, Table `t1`, Partition `p1`",
         EXPLAIN_ALL_VERBOSE);

  test_1("test/t1#P#p1", "`test`.`t1` /* Partition `p1` */",
         EXPLAIN_PARTITIONS_AS_COMMENT);

  test_1("test/t1#P#p1@00231", "`test`.`t1` /* Partition `p1#1` */",
         EXPLAIN_PARTITIONS_AS_COMMENT);

  test_1("test/t1#P#p1#SP#sp1",
         "`test`.`t1` /* Partition `p1`, Subpartition `sp1` */",
         EXPLAIN_PARTITIONS_AS_COMMENT);

  test_1("test/t1#p1#SP#sp1", "`test`.`t1#p1#SP#sp1`",
         EXPLAIN_PARTITIONS_AS_COMMENT);

  test_1("test/t1#p#p1@00232#SP#sp1@00231#REN#",
         "`test`.`t1` /* Renamed Partition `p1#2`, Subpartition `sp1#1` */",
         EXPLAIN_PARTITIONS_AS_COMMENT);

  test_1("test/t1#p#p1#SP#sp1#TMP#",
         "`test`.`t1` /* Temporary Partition `p1`, Subpartition `sp1` */",
         EXPLAIN_PARTITIONS_AS_COMMENT);

  test_1("test/#sql-t1#P#p1#SP#sp1#TMP#",
         "`test`.`#sql-t1#P#p1#SP#sp1#TMP#` /* Temporary Partition `p1`, "
         "Subpartition `sp1` */",
         EXPLAIN_PARTITIONS_AS_COMMENT);

  test_1(
      "test/#sql-t1#P#p1#SP#sp1",
      "`test`.`#sql-t1#P#p1#SP#sp1` /* Partition `p1`, Subpartition `sp1` */",
      EXPLAIN_PARTITIONS_AS_COMMENT);

  test_1("test/#sqlx-33", "`test`.`#sqlx-33`", EXPLAIN_PARTITIONS_AS_COMMENT);

  test_1("test/#mysql50#t", "`test`.`#mysql50#t`",
         EXPLAIN_PARTITIONS_AS_COMMENT);

  test_1("#mysql50#t", "`#mysql50#t`", EXPLAIN_PARTITIONS_AS_COMMENT);

  test_1("@0023t", "`#t`", EXPLAIN_PARTITIONS_AS_COMMENT);

  test_1("test/t@0023", "`test`.`t#`", EXPLAIN_PARTITIONS_AS_COMMENT);

  /*
    If a character not allowed in my_charset_filename is encountered,
    then it will not be converted to system_charset_info!
  */
  test_1("test/t@0023#", "`test`.`t@0023#`", EXPLAIN_PARTITIONS_AS_COMMENT);
}

}  // namespace explain_filename_unittest
