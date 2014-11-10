/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <gtest/gtest.h>

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/repo-auth-type-array.h"

namespace HPHP {

TEST(ARRAY, Capacity) {
  EXPECT_TRUE(kPackedCapCodeThreshold == 0x10000);

#define MP(a, b) std::make_pair(a, b)
  // Update the numbers if we change kPackedCapCodeThreshold
#if (LG_SMART_SIZES_PER_DOUBLING == 1)
  std::pair<uint32_t, uint32_t> caps [] = {
    MP(3, 0),
    MP(4, 5),
    MP(5, 0),
    MP(6, 7),
    MP(7, 0),
    MP(8, 11),
    MP(12, 15),
    MP(127, 0),
    MP(128, 191),
    MP(0xFFFF, 0),
    MP(0x10000, 0x17F00),
    MP(0x10001, 0)
  };
#elif (LG_SMART_SIZES_PER_DOUBLING == 2)
  std::pair<uint32_t, uint32_t> caps [] = {
    MP(3, 0),
    MP(4, 0),
    MP(5, 0),
    MP(6, 0),
    MP(7, 0),
    MP(8, 9),
    MP(12, 13),
    MP(127, 0),
    MP(128, 159),
    MP(0xFFFF, 0),
    MP(0x10000, 0x13F00),
    MP(0x10001, 0)
  };
#else
#error Unknown LG_SMART_SIZES_PER_DOUBLING
#endif
#undef MP

  for (size_t i = 0; i != sizeof(caps) / sizeof(caps[0]); ++i) {
    EXPECT_TRUE(PackedArray::getMaxCapInPlaceFast(caps[i].first) ==
                caps[i].second);
  }
}

}
