/* Copyright (c) 2012, 2020, Oracle and/or its affiliates. All rights reserved.

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
#include <algorithm>
#include <memory>
#include <vector>

#include "my_byteorder.h"
#include "my_inttypes.h"
#include "sql/filesort_utils.h"
#include "unittest/gunit/benchmark.h"
#include "unittest/gunit/test_utils.h"

namespace filesort_compare_unittest {

/*
  Below are some performance microbenchmarks in order to compare our sorting
  options:
  std::sort -        requires no extra memory,
                     typically implemented with introsort/insertion sort
  std::stable_sort - requires extra memory: array of n pointers,
                     typically implemented with mergesort

  The record format for filesort is constructed in such a way that we can
  compare records byte-by-byte, without knowing the data types.
  Nullable fields (maybe_null()) are pre-pended with an extra byte.
  If we are sorting in descending mode, all the bytes are simply flipped.

  This means that any variant of memcmp() can be used for comparing record.
  Below we test different variants, including memcmp() itself.
*/

inline int bytes_to_int(const uchar *s) {
  int val = longget(s);
  return val ^ 0x80000000;
}

inline void int_to_bytes(uchar *s, int val) {
  val = val ^ 0x80000000;
  longstore(s, val);
}

TEST(BufferAlignmentTest, IntsToBytesToInt) {
  uchar buf[10];
  memset(buf, 0, sizeof(buf));
  for (int ix = 0; ix < 6; ++ix) {
    int test_data[] = {INT_MIN32, -42, -1, 0, 1, 42, INT_MAX32};
    for (size_t iy = 0; iy < array_elements(test_data); ++iy) {
      int val = test_data[iy];
      int_to_bytes(buf + ix, val);
      EXPECT_EQ(val, bytes_to_int(buf + ix));
    }
  }
}

class FileSortBMHelper {
 public:
  // Number of records.
  static const int num_records = 100 * 100;
  // Number of keys in each record.
  static const int keys_per_record = 4;
  // Size of each record.
  static const int record_size = keys_per_record * sizeof(int);

  // Buffer containing data to be sorted.
  // (actually: we only sort the sort_keys below, data is stable).
  std::vector<int> test_data;

  FileSortBMHelper() {
    test_data.reserve(num_records * keys_per_record);
    union {
      int val;
      uchar buf[sizeof(int)];
    } sort_str;

    for (int ix = 0; ix < num_records * keys_per_record; ++ix) {
      int val = ix / (10 * keys_per_record);
      if (ix % 10 == 0) val = -val;
      int_to_bytes(sort_str.buf, val);
      test_data.push_back(sort_str.val);
    }
    // Comment away shuffling for testing partially pre-sorted data.
    // std::random_shuffle(test_data.begin(), test_data.end());

    sort_keys.reset(new uchar *[num_records]);
    for (int ix = 0; ix < num_records; ++ix)
      sort_keys[ix] = static_cast<uchar *>(
          static_cast<void *>(&test_data[keys_per_record * ix]));
  }

  std::vector<uchar *> GetKeys() const {
    return {sort_keys.get(), sort_keys.get() + num_records};
  }

  std::unique_ptr<uchar *[]> sort_keys;
};

/*
  Some different mem_compare functions.
  The second one seems to win on all platforms, except sparc,
  where the builtin memcmp() wins.

  TODO(sgunders): Consider re-tuning this and switch to one of
  the other ones; ideally, just memcmp().
 */
inline bool mem_compare_0(const uchar *s1, const uchar *s2, size_t len) {
  do {
    if (*s1++ != *s2++) return *--s1 < *--s2;
  } while (--len != 0);
  return s1 > s2;  // Return false for duplicate keys.
}

// This variant is safe against zero-length inputs.
inline bool mem_compare_0_zerosafe(const uchar *s1, const uchar *s2,
                                   size_t len) {
  for (size_t i = 0; i < len; ++i) {
    if (s1[i] != s2[i]) return s1[i] < s2[i];
  }
  return s1 > s2;  // Return false for duplicate keys.
}

inline bool mem_compare_1(const uchar *s1, const uchar *s2, size_t len) {
  do {
    if (*s1++ != *s2++) return *--s1 < *--s2;
  } while (--len != 0);
  return false;
}

inline bool mem_compare_2(const uchar *s1, const uchar *s2, size_t len) {
  int v = 0;
  while (len-- > 0 && v == 0) {
    v = *(s1++) - *(s2++);
  }
  return v < 0;
}

inline bool mem_compare_3(const uchar *s1, const uchar *s2, size_t len) {
  while (--len && (s1[0] == s2[0])) {
    ++s1;
    ++s2;
  }
  return s1[0] < s2[0];
}

#if defined(_WIN32)
#pragma intrinsic(memcmp)
#endif
// For gcc, __builtin_memcmp is actually *slower* than the library call:
// http://gcc.gnu.org/bugzilla/show_bug.cgi?id=43052

class Mem_compare_memcmp {
 public:
  explicit Mem_compare_memcmp(size_t n) : m_size(n) {}
  bool operator()(const uchar *s1, const uchar *s2) {
    return memcmp(s1, s2, m_size) < 0;
  }
  size_t m_size;
};

class Mem_compare_0 {
 public:
  explicit Mem_compare_0(size_t n) : m_size(n) {}
  bool operator()(const uchar *s1, const uchar *s2) {
    return mem_compare_0(s1, s2, m_size);
  }
  size_t m_size;
};

class Mem_compare_0_zerosafe {
 public:
  explicit Mem_compare_0_zerosafe(size_t n) : m_size(n) {}
  bool operator()(const uchar *s1, const uchar *s2) {
    return mem_compare_0_zerosafe(s1, s2, m_size);
  }
  size_t m_size;
};

class Mem_compare_1 {
 public:
  explicit Mem_compare_1(size_t n) : m_size(n) {}
  bool operator()(const uchar *s1, const uchar *s2) {
    return mem_compare_1(s1, s2, m_size);
  }
  size_t m_size;
};

class Mem_compare_2 {
 public:
  explicit Mem_compare_2(size_t n) : m_size(n) {}
  bool operator()(const uchar *s1, const uchar *s2) {
    return mem_compare_2(s1, s2, m_size);
  }
  size_t m_size;
};

class Mem_compare_3 {
 public:
  explicit Mem_compare_3(size_t n) : m_size(n) {}
  bool operator()(const uchar *s1, const uchar *s2) {
    return mem_compare_3(s1, s2, m_size);
  }
  size_t m_size;
};

#define COMPARE(N) \
  if (s1[N] != s2[N]) return s1[N] < s2[N]

class Mem_compare_4 {
 public:
  explicit Mem_compare_4(size_t n) : m_size(n) {}
  bool operator()(const uchar *s1, const uchar *s2) {
    size_t len = m_size;
    while (len > 0) {
      COMPARE(0);
      COMPARE(1);
      COMPARE(2);
      COMPARE(3);
      len -= 4;
      s1 += 4;
      s2 += 4;
    }
    return false;
  }
  size_t m_size;
};

class Mem_compare_5 {
 public:
  explicit Mem_compare_5(size_t n) : m_size(n) {}
  bool operator()(const uchar *s1, const uchar *s2) {
    COMPARE(0);
    COMPARE(1);
    COMPARE(2);
    COMPARE(3);
    return memcmp(s1 + 4, s2 + 4, m_size - 4) < 0;
  }
  size_t m_size;
};

// This one works for any number of keys.
// We treat the first key as int, the rest byte-by-byte.
class Mem_compare_int {
 public:
  explicit Mem_compare_int(size_t n) : m_size(n), rest(n - sizeof(int)) {}
  bool operator()(const uchar *s1, const uchar *s2) {
    int int1 = bytes_to_int(s1);
    int int2 = bytes_to_int(s2);
    if (int1 == int2) return mem_compare_1(s1 + rest, s2 + rest, rest);
    return int1 < int2;
  }

 private:
  size_t m_size;
  const size_t rest;
};

class Mem_compare_int_4 {
 public:
  explicit Mem_compare_int_4(size_t) : keyno(1) {}
  bool operator()(const uchar *s1, const uchar *s2) {
    int inta1 = bytes_to_int(s1);
    int intb1 = bytes_to_int(s2);
    if (keyno < 4 && inta1 == intb1) {
      ++keyno;
      return operator()(s1 + sizeof(int), s2 + sizeof(int));
    }
    return inta1 < intb1;
  }
  int keyno;
};

template <class Compare>
static inline void RunSortBenchmark(size_t num_iterations, bool stable_sort) {
  StopBenchmarkTiming();
  FileSortBMHelper helper;
  for (size_t ix = 0; ix < num_iterations; ++ix) {
    std::vector<uchar *> keys = helper.GetKeys();
    StartBenchmarkTiming();
    if (stable_sort) {
      std::stable_sort(keys.begin(), keys.end(), Compare(helper.record_size));
    } else {
      std::sort(keys.begin(), keys.end(), Compare(helper.record_size));
    }
    StopBenchmarkTiming();
  }
}

/*
  Several sorting tests below, each one runs num_iterations.
  For each iteration we take a copy of the key pointers, and sort the copy.
  Most of the tests below are run with std::sort and std::stable_sort.
  Stable sort seems to be faster for all test cases, on all platforms.
 */
static void BM_StdSortmemcmp(size_t num_iterations) {
  RunSortBenchmark<Mem_compare_memcmp>(num_iterations, /*stable_sort=*/false);
}
BENCHMARK(BM_StdSortmemcmp)

static void BM_StdStableSortmemcmp(size_t num_iterations) {
  RunSortBenchmark<Mem_compare_memcmp>(num_iterations, /*stable_sort=*/true);
}
BENCHMARK(BM_StdStableSortmemcmp)

static void BM_StdSortCompare0(size_t num_iterations) {
  RunSortBenchmark<Mem_compare_0>(num_iterations, /*stable_sort=*/false);
}
BENCHMARK(BM_StdSortCompare0)

static void BM_StdStableSortCompare0(size_t num_iterations) {
  RunSortBenchmark<Mem_compare_0>(num_iterations, /*stable_sort=*/true);
}
BENCHMARK(BM_StdStableSortCompare0)

static void BM_StdSortCompare0ZeroSafe(size_t num_iterations) {
  RunSortBenchmark<Mem_compare_0_zerosafe>(num_iterations,
                                           /*stable_sort=*/false);
}
BENCHMARK(BM_StdSortCompare0ZeroSafe)

static void BM_StdStableSortCompare0ZeroSafe(size_t num_iterations) {
  RunSortBenchmark<Mem_compare_0_zerosafe>(num_iterations,
                                           /*stable_sort=*/true);
}
BENCHMARK(BM_StdStableSortCompare0ZeroSafe)

static void BM_StdSortCompare1(size_t num_iterations) {
  RunSortBenchmark<Mem_compare_1>(num_iterations, /*stable_sort=*/false);
}
BENCHMARK(BM_StdSortCompare1)

static void BM_StdStableSortCompare1(size_t num_iterations) {
  RunSortBenchmark<Mem_compare_1>(num_iterations, /*stable_sort=*/true);
}
BENCHMARK(BM_StdStableSortCompare1)

static void BM_StdSortCompare2(size_t num_iterations) {
  RunSortBenchmark<Mem_compare_2>(num_iterations, /*stable_sort=*/false);
}
BENCHMARK(BM_StdSortCompare2)

static void BM_StdStableSortCompare2(size_t num_iterations) {
  RunSortBenchmark<Mem_compare_2>(num_iterations, /*stable_sort=*/true);
}
BENCHMARK(BM_StdStableSortCompare2)

static void BM_StdSortCompare3(size_t num_iterations) {
  RunSortBenchmark<Mem_compare_3>(num_iterations, /*stable_sort=*/false);
}
BENCHMARK(BM_StdSortCompare3)

static void BM_StdStableSortCompare3(size_t num_iterations) {
  RunSortBenchmark<Mem_compare_3>(num_iterations, /*stable_sort=*/true);
}
BENCHMARK(BM_StdStableSortCompare3)

static void BM_StdSortCompare4(size_t num_iterations) {
  RunSortBenchmark<Mem_compare_4>(num_iterations, /*stable_sort=*/false);
}
BENCHMARK(BM_StdSortCompare4)

static void BM_StdStableSortCompare4(size_t num_iterations) {
  RunSortBenchmark<Mem_compare_4>(num_iterations, /*stable_sort=*/true);
}
BENCHMARK(BM_StdStableSortCompare4)

static void BM_StdSortCompare5(size_t num_iterations) {
  RunSortBenchmark<Mem_compare_5>(num_iterations, /*stable_sort=*/false);
}
BENCHMARK(BM_StdSortCompare5)

static void BM_StdStableSortCompare5(size_t num_iterations) {
  RunSortBenchmark<Mem_compare_5>(num_iterations, /*stable_sort=*/true);
}
BENCHMARK(BM_StdStableSortCompare5)

// Disabled: experimental.
static void MY_ATTRIBUTE((unused)) BM_StdSortIntCompare(size_t num_iterations) {
  RunSortBenchmark<Mem_compare_int>(num_iterations, /*stable_sort=*/false);
}
// BENCHMARK(BM_StdSortIntCompare)

static void MY_ATTRIBUTE((unused))
    BM_StdStableSortIntCompare(size_t num_iterations) {
  RunSortBenchmark<Mem_compare_int>(num_iterations, /*stable_sort=*/true);
}
// BENCHMARK(BM_StdStableSortIntCompare)

// Disabled: experimental.
static void MY_ATTRIBUTE((unused))
    BM_StdSortIntIntIntInt(size_t num_iterations) {
  RunSortBenchmark<Mem_compare_int_4>(num_iterations, /*stable_sort=*/false);
}
// BENCHMARK(BM_StdSortIntIntIntInt)

// Disabled: experimental.
static void MY_ATTRIBUTE((unused))
    BM_StdStableSortIntIntIntInt(size_t num_iterations) {
  RunSortBenchmark<Mem_compare_int_4>(num_iterations, /*stable_sort=*/true);
}
// BENCHMARK(BM_StdStableSortIntIntIntInt)

}  // namespace filesort_compare_unittest
