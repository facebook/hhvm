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
  EXPECT_FALSE(slab->isStart(large));
  slab->setStart(large);
  EXPECT_TRUE(slab->isStart(large));
  large->initHeader_32(HeaderKind::SmallMalloc, 0);
  large->nbytes = kMaxSmallSize;

  auto small = reinterpret_cast<MallocNode*>(start + large->nbytes);
  slab->setStart(small);
  small->initHeader_32(HeaderKind::SmallMalloc, 0);
  small->nbytes = kSmallSizeAlign;

  auto hole = reinterpret_cast<FreeNode*>((char*)small + small->nbytes);
  slab->setStart(hole);
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

/*
 * test one example of setting start bits in bulk:
 * 1. initialize two slabs identically, with a 10101010 bit pattern everywhere
 * except the range to be tested.
 * 2. fill the range [offset, ofset+len) in s1 with start bits according
 * to size class 'index', one at a time
 * 3. fill s2 the same way, with Slab::setStarts()
 * 4. validate s1 & s2 have exactly the same start-bits set.
 * 5. check that we haven't overrun the starts_ array, using a redzone value.
 */
void test_starts(size_t index, size_t offset, size_t len, Slab* s1, Slab* s2) {
  auto const Q = kSmallSizeAlign;
  auto const nbytes = kSizeIndex2Size[index];
  auto const redzone = 0x123456789abcdef0ll;
  auto setup = [&](Slab* slab) {
    slab->init();
    auto const start = slab->start() + offset;
    auto const end = start + len;
    for (auto p = start - Q; p >= slab->start(); p -= 2*Q) {
      slab->setStart(p);
    }
    for (auto p = end; p < slab->end(); p += 2*Q) {
      slab->setStart(p);
    }
    *((long*)slab->start()) = redzone;
  };
  setup(s1);
  setup(s2);
  // init s1 the manual way
  for (auto p = s1->start() + offset; p < s1->start() + offset + len;
       p += nbytes) {
    s1->setStart(p);
  }
  // init s2 the fast way
  s2->setStarts(s2->start() + offset, s2->start() + offset + len,
                nbytes, index);

  for (auto p = s1->start(), q = s2->start(); p < s1->end();
       p += 64*Q, q += 64*Q) {
    EXPECT_EQ(s1->start_bits(p), s2->start_bits(q));
  }
  EXPECT_EQ((*(long*)s1->start()), redzone);
  EXPECT_EQ((*(long*)s2->start()), redzone);
}

/*
 * exhaustively test Slab::setStarts over the range of possible size-classes,
 * alignments, lengths, and adjacency to the slab start and end, trying
 * to cover all the corner cases.
 */
TEST(Slab, set_starts) {
  auto const Q = kSmallSizeAlign;
  auto const line_size = 64 * Q;
  auto const slab_cap = kSlabSize - sizeof(Slab);
  auto const numSizes = MemoryManager::size2Index(line_size) + 2;
  auto s1 = static_cast<Slab*>(aligned_alloc(kSlabSize, kSlabSize));
  auto s2 = static_cast<Slab*>(aligned_alloc(kSlabSize, kSlabSize));
  SCOPE_EXIT { free(s1); free(s2); };
  for (size_t index = 0; index < numSizes; ++index) {
    auto const nbytes = kSizeIndex2Size[index];
    auto const len = std::max(4 * line_size / nbytes, 2 * nbytes);
    for (size_t n = 0; n < len; n += nbytes) {
      for (size_t offset = 0; offset < line_size; offset += Q) {
        test_starts(index, offset, n, s1, s2);
      }
      for (size_t offset = slab_cap - len - line_size;
           offset < slab_cap - len; offset += Q) {
        test_starts(index, offset, n, s1, s2);
      }
    }
  }
}

TEST(SlabManagerTest, tag_overflow) {
  SlabManager mgr;
  auto const slab = static_cast<Slab*>(aligned_alloc(kSlabSize, kSlabSize));
  SCOPE_EXIT { free(slab); };
  mgr.addRange(slab, kSlabSize);
  for (int i = 1; i <= 0x10001; ++i) {
    auto tagged = mgr.tryAlloc();
    ASSERT_EQ(tagged.ptr(), slab);
    ASSERT_EQ(tagged.tag(), i & TaggedSlabPtr::TagMask);
    mgr.push_front(tagged.ptr(), tagged.tag());
  }
}

}
