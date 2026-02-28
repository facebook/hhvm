/* Copyright (c) 2012, 2019, Oracle and/or its affiliates. All rights reserved.

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
#include <algorithm>
#include <random>
#include <vector>

#include "my_byteorder.h"
#include "my_compiler.h"
#include "my_inttypes.h"

namespace alignment_unittest {

/*
  Testing performance penalty of accessing un-aligned data.
  Seems to about 2% on my desktop machine.
 */
class AlignmentTest : public ::testing::Test {
 protected:
  // Increase num_iterations for actual benchmarking!
  static const int num_iterations = 1;
  static const int num_records = 100 * 1000;

  static int *aligned_data;
  static uchar *unaligned_data;

  static void SetUpTestCase() {
    aligned_data = new int[num_records];
    unaligned_data = new uchar[(num_records + 1) * sizeof(int)];
    for (int ix = 0; ix < num_records; ++ix) {
      aligned_data[ix] = ix / 10;
    }
    std::random_device rng;
    std::mt19937 urng(rng());
    std::shuffle(aligned_data, aligned_data + num_records, urng);
    memcpy(unaligned_data + 1, aligned_data, num_records * sizeof(int));
  }

  static void TearDownTestCase() {
    delete[] aligned_data;
    delete[] unaligned_data;
  }

  virtual void SetUp() {
    aligned_keys = new uchar *[num_records];
    unaligned_keys = new uchar *[num_records];
    for (int ix = 0; ix < num_records; ++ix) {
      aligned_keys[ix] =
          static_cast<uchar *>(static_cast<void *>(&aligned_data[ix]));
      unaligned_keys[ix] = &unaligned_data[1 + (ix * sizeof(int))];
    }
  }

  virtual void TearDown() {
    delete[] aligned_keys;
    delete[] unaligned_keys;
  }

  uchar **aligned_keys;
  uchar **unaligned_keys;
};

int *AlignmentTest::aligned_data;
uchar *AlignmentTest::unaligned_data;

// A copy of the generic, byte-by-byte getter.
#define sint4korrgeneric(A)                                            \
  (int32)(((int32)((uchar)(A)[0])) + (((int32)((uchar)(A)[1]) << 8)) + \
          (((int32)((uchar)(A)[2]) << 16)) + (((int32)((int16)(A)[3]) << 24)))
class Mem_compare_uchar_int {
 public:
  // SUPPRESS_UBSAN: only executed on intel, misaligned read works OK.
  bool operator()(const uchar *s1, const uchar *s2) SUPPRESS_UBSAN {
    return *pointer_cast<const int *>(s1) < *pointer_cast<const int *>(s2);
  }
};

class Mem_compare_sint4 {
 public:
  bool operator()(const uchar *s1, const uchar *s2) {
    return sint4korr(s1) < sint4korr(s2);
  }
};

class Mem_compare_sint4_generic {
 public:
  bool operator()(const uchar *s1, const uchar *s2) {
    return sint4korrgeneric(s1) < sint4korrgeneric(s2);
  }
};

#if defined(__i386__) || defined(__x86_64__) || defined(_WIN32)

TEST_F(AlignmentTest, AlignedSort) {
  for (int ix = 0; ix < num_iterations; ++ix) {
    std::vector<uchar *> keys(aligned_keys, aligned_keys + num_records);
    std::sort(keys.begin(), keys.end(), Mem_compare_uchar_int());
  }
}

TEST_F(AlignmentTest, UnAlignedSort) {
  for (int ix = 0; ix < num_iterations; ++ix) {
    std::vector<uchar *> keys(unaligned_keys, unaligned_keys + num_records);
    std::sort(keys.begin(), keys.end(), Mem_compare_uchar_int());
  }
}

TEST_F(AlignmentTest, Sint4Sort) {
  for (int ix = 0; ix < num_iterations; ++ix) {
    std::vector<uchar *> keys(unaligned_keys, unaligned_keys + num_records);
    std::sort(keys.begin(), keys.end(), Mem_compare_sint4());
  }
}

TEST_F(AlignmentTest, Sint4SortGeneric) {
  for (int ix = 0; ix < num_iterations; ++ix) {
    std::vector<uchar *> keys(unaligned_keys, unaligned_keys + num_records);
    std::sort(keys.begin(), keys.end(), Mem_compare_sint4_generic());
  }
}

#endif

}  // namespace alignment_unittest
