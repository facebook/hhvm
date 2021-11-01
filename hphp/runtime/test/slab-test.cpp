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

TEST(Slab, find) {
  auto slab = static_cast<Slab*>(aligned_alloc(kSlabSize, kSlabSize));
  SCOPE_EXIT { free(slab); };
  auto start = slab->init();

  // set up slab like this: [header][large...   ][small][hole]
  //                        [line 0   ][line 1]...           ]
  auto large = reinterpret_cast<MallocNode*>(start);
  EXPECT_FALSE(slab->isCachedStart(large));
  large->initHeader_32(HeaderKind::SmallMalloc, 0);
  large->nbytes = kMaxSmallSize;
  slab->setCachedStart(large);
  EXPECT_TRUE(slab->isCachedStart(large));

  auto small = reinterpret_cast<MallocNode*>(start + large->nbytes);
  small->initHeader_32(HeaderKind::SmallMalloc, 0);
  small->nbytes = kSmallSizeAlign;

  auto hole = reinterpret_cast<FreeNode*>((char*)small + small->nbytes);
  hole->initHeader_32(HeaderKind::Hole, slab->end() - (char*)hole);

  // ensure find doesn't walk off beginning or otherwise blow up if we
  // test addresses in the header area.
  EXPECT_EQ(slab->find(slab), nullptr);
  EXPECT_EQ(slab->find(start - 1), nullptr);
  EXPECT_EQ(slab->find(start), large);
  EXPECT_EQ(slab->find(start + 1), large);

  // probe edge between large and small
  EXPECT_EQ(slab->find((char*)small - 1), large);
  EXPECT_EQ(slab->find((char*)small), small);
  EXPECT_EQ(slab->find((char*)small + 1), small);

  // probe edge between small and hole.
  EXPECT_EQ(slab->find((char*)hole - 1), small);
  EXPECT_EQ(slab->find((char*)hole), hole);
  EXPECT_EQ(slab->find((char*)hole + 1), hole);
}

TEST(SlabManagerTest, tag_overflow) {
  SlabManager mgr;
  auto const slab = static_cast<Slab*>(aligned_alloc(kSlabSize, kSlabSize));
  SCOPE_EXIT { free(slab); };
  mgr.addRange(slab, kSlabSize);
  for (int i = 1; i <= 0x10001; ++i) {
    auto tagged = mgr.tryAlloc();
    ASSERT_EQ(tagged.ptr(), slab);
    auto constexpr mask = TaggedSlabPtr::TagMask >> TaggedSlabPtr::TagShift;
    ASSERT_EQ(tagged.tag(), i & mask);
    mgr.push_front(tagged.ptr(), tagged.tag());
  }
}

}
