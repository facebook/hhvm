/* Copyright (c) 2013, 2017, Oracle and/or its affiliates. All rights reserved.

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

/* See http://code.google.com/p/googletest/wiki/Primer */

// First include (the generated) my_config.h, to get correct platform defines.
#include "my_config.h"

#include <gtest/gtest.h>

#include "storage/innobase/include/univ.i"
#include "storage/innobase/include/ut0mem.h"

namespace innodb_ut0mem_unittest {

/* test ut_str_sql_format() */
TEST(ut0mem, utstrsqlformat) {
  const char *buf_initial = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
  ulint buf_initial_sz = strlen(buf_initial) + 1;
  char buf[64];
  ulint ret;
  struct {
    const char *in;
    ulint in_len;
    ulint buf_size;
    ulint ret_expected;
    const char *buf_expected;
  } test_data[] = {
      {"abcd", 4, 0, 0, buf_initial},
      {"abcd", 4, 1, 1, ""},
      {"abcd", 4, 2, 1, ""},
      {"abcd", 0, 3, 3, "''"},
      {"abcd", 1, 3, 1, ""},
      {"abcd", 2, 3, 1, ""},
      {"abcd", 3, 3, 1, ""},
      {"abcd", 4, 3, 1, ""},
      {"abcd", 0, 4, 3, "''"},
      {"abcd", 1, 4, 4, "'a'"},
      {"abcd", 2, 4, 4, "'a'"},
      {"abcd", 3, 4, 4, "'a'"},
      {"abcd", 4, 4, 4, "'a'"},
      {"abcde", 5, 4, 4, "'a'"},
      {"'", 1, 4, 3, "''"},
      {"''", 2, 4, 3, "''"},
      {"a'", 2, 4, 4, "'a'"},
      {"'a", 2, 4, 3, "''"},
      {"ab", 2, 4, 4, "'a'"},
      {"abcdef", 0, 5, 3, "''"},
      {"abcdef", 1, 5, 4, "'a'"},
      {"abcdef", 2, 5, 5, "'ab'"},
      {"abcdef", 3, 5, 5, "'ab'"},
      {"abcdef", 4, 5, 5, "'ab'"},
      {"abcdef", 5, 5, 5, "'ab'"},
      {"abcdef", 6, 5, 5, "'ab'"},
      {"'", 1, 5, 5, "''''"},
      {"''", 2, 5, 5, "''''"},
      {"a'", 2, 5, 4, "'a'"},
      {"'a", 2, 5, 5, "''''"},
      {"ab", 2, 5, 5, "'ab'"},
      {"abc", 3, 5, 5, "'ab'"},
      {"ab", 2, 6, 5, "'ab'"},
      {"a'b'c", 5, 32, 10, "'a''b''c'"},
      {"a'b'c'", 6, 32, 12, "'a''b''c'''"},
  };

  for (ulint i = 0; i < UT_ARR_SIZE(test_data); i++) {
    memcpy(buf, buf_initial, buf_initial_sz);

    ret = ut_str_sql_format(test_data[i].in, test_data[i].in_len, buf,
                            test_data[i].buf_size);

    EXPECT_EQ(test_data[i].ret_expected, ret);
    EXPECT_STREQ(test_data[i].buf_expected, buf);
  }
}

}  // namespace innodb_ut0mem_unittest
