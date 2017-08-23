/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include <folly/portability/GTest.h>

#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/memory-manager-defs.h"

namespace HPHP {

TEST(SlabXmapTest, very_small_xmap) {
  // 2m slab / 2k line => 1k header
  const auto LineSize = 2048;
  using Slab = SlabHeader<LineSize>;
  static_assert(sizeof(Slab) < LineSize, "");
  auto slab = static_cast<Slab*>(malloc(kSlabSize));
  SCOPE_EXIT { free(slab); };
  auto start = slab->init();

  // set up slab like this: [header][large...   ][small][hole]
  //                        [line 0   ][line 1]...           ]
  auto large = reinterpret_cast<MallocNode*>(start);
  large->initHeader_32(HeaderKind::SmallMalloc, 0);
  large->nbytes = kMaxSmallSize;

  auto small = reinterpret_cast<MallocNode*>(start + large->nbytes);
  small->initHeader_32(HeaderKind::SmallMalloc, 0);
  small->nbytes = kSmallSizeAlign;

  auto hole = reinterpret_cast<FreeNode*>((char*)small + small->nbytes);
  hole->initHeader_32(HeaderKind::Hole, slab->end() - (char*)hole);
  slab->initCrossingMap([](void*,size_t){});

  // ensure find doesn't walk off beginning of crossing map
  EXPECT_EQ(slab->find(slab).ptr, nullptr);
  EXPECT_EQ(slab->find(start - 1).ptr, nullptr);
  EXPECT_EQ(slab->find(start).ptr, large);
  EXPECT_EQ(slab->find(start + 1).ptr, large);

  // make sure reverse-search works
  static_assert(kMaxSmallSize / LineSize > 128, "");
  EXPECT_EQ(slab->find(start + 1 * LineSize).ptr, large);
  EXPECT_EQ(slab->find(start + 2 * LineSize).ptr, large);
  EXPECT_EQ(slab->find(start + 129 * LineSize).ptr, large);

  // probe edge between large and small
  EXPECT_EQ(slab->find((char*)small - 1).ptr, large);
  EXPECT_EQ(slab->find((char*)small).ptr, small);
  EXPECT_EQ(slab->find((char*)small + 1).ptr, small);

  // probe edge between small and hole
  EXPECT_EQ(slab->find((char*)hole - 1).ptr, small);
  EXPECT_EQ(slab->find((char*)hole).ptr, nullptr);
  EXPECT_EQ(slab->find((char*)hole + 1).ptr, nullptr);
}

}
