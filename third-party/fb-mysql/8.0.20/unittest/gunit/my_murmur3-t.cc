/* Copyright (c) 2013, 2019, Oracle and/or its affiliates. All rights reserved.

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
#include <sys/types.h>

#include "my_inttypes.h"
#include "my_murmur3.h"
#include "template_utils.h"

/*
  Putting everything in a namespace prevents any (unintentional)
  name clashes with the code under test.
*/

namespace murmur3_unittest {

/* Simple test checking that hash for fixed key has correct value. */

TEST(Murmur3, Basic) {
  const char *str = "To be, or not to be, that is the question;";

  uint hash = murmur3_32(pointer_cast<const uchar *>(str), strlen(str), 0);
  EXPECT_EQ(2385370181U, hash);
}

/* Test for empty key. */

TEST(Murmur3, Empty) {
  uint hash = murmur3_32(nullptr, 0, 0);
  EXPECT_EQ(0U, hash);
}

/*
  Test that shows that when we hash zero-keys of different length we
  get different results. Our my_hash_sort_bin is not good at that.
*/

TEST(Murmur3, Zeroes) {
  uchar buff[32];
  memset(buff, 0, sizeof(buff));

  uint hash1 = murmur3_32(buff, sizeof(buff) / 2, 0);
  uint hash2 = murmur3_32(buff, sizeof(buff), 0);
  EXPECT_NE(hash1, hash2);
}

/* Test that seed matters. */

TEST(Murmur3, Seed) {
  const char *str = "Whether 'tis nobler in the mind to suffer";

  uint hash1 = murmur3_32(pointer_cast<const uchar *>(str), strlen(str), 0);
  uint hash2 = murmur3_32(pointer_cast<const uchar *>(str), strlen(str), 1);
  EXPECT_NE(hash1, hash2);
}

/*
  Test for bug #16396598 "MDL HASH CAN STILL BE CONCURRENCY BOTTLENECK".
  Hashes for 8 keys from the bug report should have sufficiently different
  lower bits, so corresponding MDL objects won't fall into the same MDL map
  partitions.
*/

TEST(Murmur3, Bug16396598) {
  const char keys[8][14] = {"\002test\000sbtest1", "\002test\000sbtest2",
                            "\002test\000sbtest3", "\002test\000sbtest4",
                            "\002test\000sbtest5", "\002test\000sbtest6",
                            "\002test\000sbtest7", "\002test\000sbtest8"};
  /* Array for number of keys falling into n-th bucket. */
  uint buckets[8];
  int i;

  memset(buckets, 0, sizeof(buckets));

  for (i = 0; i < 8; ++i)
    buckets[murmur3_32((const uchar *)keys[i], sizeof(keys[0]), 0) % 8]++;

  /*
    It is OK for a few keys to fall into the same bucket.
    But not for the half of the keys.
  */
  for (i = 0; i < 8; ++i) EXPECT_GT(4U, buckets[i]);
}

}  // namespace murmur3_unittest
