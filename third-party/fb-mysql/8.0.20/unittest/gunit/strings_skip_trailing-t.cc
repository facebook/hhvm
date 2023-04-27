/* Copyright (c) 2013, 2018, Oracle and/or its affiliates. All rights reserved.

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
  Bug#68477    Suboptimal code in skip_trailing_space()
  Bug#16395778 SUBOPTIMAL CODE IN SKIP_TRAILING_SPACE()

  Below we test some alternative implementations for skip_trailing_space.
 */

#include <gtest/gtest.h>
#include <string>

#include "m_string.h"
#include "template_utils.h"
#include "unittest/gunit/benchmark.h"
#include "unittest/gunit/skip_trailing.h"

namespace skip_trailing_space_unittest {

static inline void benchmark_func(size_t iters,
                                  const uchar *func(const uchar *, size_t),
                                  size_t length) {
  StopBenchmarkTiming();
  // Insert something else (or nothing) here,
  //   to see effects of alignment of data:
  std::string str = "1";
  str.append(length, ' ');
  StartBenchmarkTiming();

  for (size_t i = 0; i < iters; ++i) {
    func(pointer_cast<const uchar *>(str.data()), length);
  }
}

#define INSTANTIATE_TEST(name, func, length)                              \
  static void name(size_t iters) { benchmark_func(iters, func, length); } \
  BENCHMARK(name)

INSTANTIATE_TEST(BM_SkipTrailingSpaceTest_Unaligned_0, skip_trailing_unalgn, 0)
INSTANTIATE_TEST(BM_SkipTrailingSpaceTest_Unaligned_24, skip_trailing_unalgn,
                 24)
INSTANTIATE_TEST(BM_SkipTrailingSpaceTest_Unaligned_100, skip_trailing_unalgn,
                 100)
INSTANTIATE_TEST(BM_SkipTrailingSpaceTest_Unaligned_150, skip_trailing_unalgn,
                 150)

INSTANTIATE_TEST(BM_SkipTrailingSpaceTest_Original_0, skip_trailing_orig, 0)
INSTANTIATE_TEST(BM_SkipTrailingSpaceTest_Original_24, skip_trailing_orig, 24)
INSTANTIATE_TEST(BM_SkipTrailingSpaceTest_Original_100, skip_trailing_orig, 100)
INSTANTIATE_TEST(BM_SkipTrailingSpaceTest_Original_150, skip_trailing_orig, 150)

INSTANTIATE_TEST(BM_SkipTrailingSpaceTest_FourByte_0, skip_trailing_4byte, 0)
INSTANTIATE_TEST(BM_SkipTrailingSpaceTest_FourByte_24, skip_trailing_4byte, 24)
INSTANTIATE_TEST(BM_SkipTrailingSpaceTest_FourByte_100, skip_trailing_4byte,
                 100)
INSTANTIATE_TEST(BM_SkipTrailingSpaceTest_FourByte_150, skip_trailing_4byte,
                 150)

INSTANTIATE_TEST(BM_SkipTrailingSpaceTest_EightByte_0, skip_trailing_8byte, 0)
INSTANTIATE_TEST(BM_SkipTrailingSpaceTest_EightByte_24, skip_trailing_8byte, 24)
INSTANTIATE_TEST(BM_SkipTrailingSpaceTest_EightByte_100, skip_trailing_8byte,
                 100)
INSTANTIATE_TEST(BM_SkipTrailingSpaceTest_EightByte_150, skip_trailing_8byte,
                 150)

INSTANTIATE_TEST(BM_SkipTrailingSpaceTest_Current_0, skip_trailing_space, 0)
INSTANTIATE_TEST(BM_SkipTrailingSpaceTest_Current_24, skip_trailing_space, 24)
INSTANTIATE_TEST(BM_SkipTrailingSpaceTest_Current_100, skip_trailing_space, 100)
INSTANTIATE_TEST(BM_SkipTrailingSpaceTest_Current_150, skip_trailing_space, 150)

}  // namespace skip_trailing_space_unittest
