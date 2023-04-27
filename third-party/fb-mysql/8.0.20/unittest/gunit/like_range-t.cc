/* Copyright (c) 2010, 2019, Oracle and/or its affiliates. All rights reserved.

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

#include <gtest/gtest.h>

#include "m_ctype.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_sys.h"

namespace like_range_unittest {

/*
  Test that like_range() returns well-formed results.
*/
static void test_like_range_for_charset(CHARSET_INFO *cs, const char *src,
                                        size_t src_len) {
  char min_str[32], max_str[32];
  size_t min_len, max_len, min_well_formed_len, max_well_formed_len;
  int error = 0;

  cs->coll->like_range(cs, src, src_len, '\\', '_', '%', sizeof(min_str),
                       min_str, max_str, &min_len, &max_len);
  // diag("min_len=%d\tmax_len=%d\t%s", (int) min_len, (int) max_len, cs->name);
  min_well_formed_len =
      cs->cset->well_formed_len(cs, min_str, min_str + min_len, 10000, &error);
  max_well_formed_len =
      cs->cset->well_formed_len(cs, max_str, max_str + max_len, 10000, &error);
  EXPECT_EQ(min_len, min_well_formed_len)
      << "Bad min_str: min_well_formed_len=" << min_well_formed_len
      << " min_str[" << min_well_formed_len
      << "]=" << (uchar)min_str[min_well_formed_len];
  EXPECT_EQ(max_len, max_well_formed_len)
      << "Bad max_str: max_well_formed_len=" << max_well_formed_len
      << " max_str[" << max_well_formed_len
      << "]=" << (uchar)max_str[max_well_formed_len];
}

static const char *charset_list[] = {
    "big5_chinese_ci",    "big5_bin",

    "euckr_korean_ci",    "euckr_bin",

    "gb2312_chinese_ci",  "gb2312_bin",

    "gbk_chinese_ci",     "gbk_bin",

    "gb18030_chinese_ci", "gb18030_bin",

    "latin1_swedish_ci",  "latin1_bin",

    "sjis_japanese_ci",   "sjis_bin",

    "tis620_thai_ci",     "tis620_bin",

    "ujis_japanese_ci",   "ujis_bin",

    "utf8_general_ci",    "utf8_unicode_ci", "utf8_bin",
};

class LikeRangeTest : public ::testing::TestWithParam<const char *> {
 protected:
  virtual void SetUp() {
    MY_CHARSET_LOADER loader;
    my_charset_loader_init_mysys(&loader);
    m_charset = my_collation_get_by_name(&loader, GetParam(), MYF(0));
    DBUG_ASSERT(m_charset);
  }
  CHARSET_INFO *m_charset;
};

INSTANTIATE_TEST_CASE_P(Foo1, LikeRangeTest, ::testing::ValuesIn(charset_list));

TEST_P(LikeRangeTest, TestLikeRange) {
  test_like_range_for_charset(m_charset, "abc%", 4);
}

}  // namespace like_range_unittest
