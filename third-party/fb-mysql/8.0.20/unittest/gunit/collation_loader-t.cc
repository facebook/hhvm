/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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
  We do benchmark test of character set loader below.
  1. Load a collation. This happens in query like 'SELECT a FROM tbl ORDER BY
     a COLLATE coll_name'. 'coll_name' is the name of collation which is used
     to do ordering, like 'latin1_swedish_ci'.
  2. Load a character set. In MySQL, when you ask for a 'character set', you
     actually get a collation. For example, when executing 'SELECT _utf16
     0x0100'. MySQL uses the pointer to collation 'utf16_general_ci' to handle
     this character.
*/

#include <gtest/gtest.h>
#include <thread>
#include <vector>

#include "m_ctype.h"
#include "my_sys.h"
#include "unittest/gunit/benchmark.h"

namespace collation_loader_unittest {

namespace {

CHARSET_INFO *lookup_collation(const char *name) {
  MY_CHARSET_LOADER loader;
  my_charset_loader_init_mysys(&loader);
  return my_collation_get_by_name(&loader, name, MYF(0));
}

CHARSET_INFO *lookup_charset(const char *name, int flag) {
  MY_CHARSET_LOADER loader;
  my_charset_loader_init_mysys(&loader);
  return my_charset_get_by_name(&loader, name, flag, MYF(0));
}

}  // namespace

static void BM_LookupAllCollations(size_t num_iterations) {
  StopBenchmarkTiming();

  // Look up one collation to initialize the all_charsets array.
  lookup_collation("latin1_swedish_ci");
  size_t num_charsets = array_elements(all_charsets);

  StartBenchmarkTiming();
  for (size_t i = 0; i < num_iterations; i++) {
    const CHARSET_INFO *cs = all_charsets[i % num_charsets];
    if (cs) {
      EXPECT_NE(lookup_collation(cs->name), nullptr);
    }
  }
}
BENCHMARK(BM_LookupAllCollations)

static std::vector<std::string> charsets = {
    "armscii8", "ascii",   "big5",   "binary",  "cp1250",  "cp1251",
    "cp1256",   "cp1257",  "cp850",  "cp852",   "cp866",   "cp932",
    "dec8",     "eucjpms", "euckr",  "gb18030", "gb2312",  "gbk",
    "geostd8",  "greek",   "hebrew", "hp8",     "keybcs2", "koi8r",
    "koi8u",    "latin1",  "latin2", "latin5",  "latin7",  "macce",
    "macroman", "sjis",    "swe7",   "tis620",  "ucs2",    "ujis",
    "utf16",    "utf16le", "utf32",  "utf8",    "utf8mb4",
};

static void BM_LookupAllCharsets(size_t num_iterations) {
  StopBenchmarkTiming();

  // Look up one collation to initialize the all_charsets array.
  lookup_collation("latin1_swedish_ci");

  StartBenchmarkTiming();
  for (size_t i = 0; i < num_iterations; i++) {
    const std::string &charset = charsets[i % charsets.size()];
    EXPECT_NE(lookup_charset(charset.c_str(), MY_CS_PRIMARY), nullptr);
    EXPECT_NE(lookup_charset(charset.c_str(), MY_CS_BINSORT), nullptr);
  }
}
BENCHMARK(BM_LookupAllCharsets)

void TestRandomCollation() {
  char coll_name[65];
  for (int ix = 0; ix < 100000; ix++) {
    memset(coll_name, 0, 65);
    // Generate a random collation name whose length is in range [1, 64].
    size_t random_coll_name_len = 1 + rand() % 64;
    for (size_t i = 0; i < random_coll_name_len; i++) {
      // Use ASCII latin letters to generate the name.
      coll_name[i] = 'a' + rand() % 26;
    }
    int rc = get_collation_number(coll_name);
    // Random collation that doesn't exist should return 0.
    EXPECT_EQ(0, rc);
  }
}

TEST(CollationLoaderTest, RandomCollation) {
  // Look up one collation to initialize the all_charsets array.
  lookup_collation("latin1_swedish_ci");
  std::thread test_thd[4];
  for (int i = 0; i < 4; i++) test_thd[i] = std::thread(TestRandomCollation);
  for (int i = 0; i < 4; i++) test_thd[i].join();
}
}  // namespace collation_loader_unittest
