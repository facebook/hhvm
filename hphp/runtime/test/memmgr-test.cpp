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
#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/dummy-resource.h"
#include "hphp/runtime/ext/collections/ext_collections-vector.h"

namespace HPHP {

static void allocAndJoin(size_t size, bool free) {
  std::thread thread([&]() {
      MemoryManager::TlsWrapper::getCheck();
      if (free) {
        String str(size, ReserveString);
      } else {
        StringData::Make(size); // Leak.
      }
    });
  thread.join();
}

TEST(MemoryManager, OnThreadExit) {
  allocAndJoin(42, true);
  allocAndJoin(kMaxSmallSize + 1, true);
#ifdef DEBUG
  EXPECT_DEATH(allocAndJoin(42, false), "");
  EXPECT_DEATH(allocAndJoin(kMaxSmallSize + 1, false), "");
#endif
}

TEST(MemoryManager, ComputeSize2Index) {
  // this test starts by requesting 2 bytes because computeSize2Index does not
  // support inputs < 2; the others support inputs >= 1
  EXPECT_EQ(MemoryManager::computeSize2Index(2), 0);
  for (size_t index = 0; index < kNumSizeClasses - 1; index++) {
    auto allocSize = kSizeIndex2Size[index];
    EXPECT_EQ(MemoryManager::computeSize2Index(allocSize - 1), index);
    EXPECT_EQ(MemoryManager::computeSize2Index(allocSize), index);
    EXPECT_EQ(MemoryManager::computeSize2Index(allocSize + 1), index + 1);
  }
  EXPECT_EQ(
    MemoryManager::computeSize2Index(kSizeIndex2Size[kNumSizeClasses - 1] - 1),
    kNumSizeClasses - 1
  );
  EXPECT_EQ(
    MemoryManager::computeSize2Index(kSizeIndex2Size[kNumSizeClasses - 1]),
    kNumSizeClasses - 1
  );
}

TEST(MemoryManager, LookupSmallSize2Index) {
  constexpr size_t kNumLookupIndices = 28;
  EXPECT_EQ(kMaxSmallSizeLookup, kSizeIndex2Size[kNumLookupIndices - 1]);

  EXPECT_EQ(MemoryManager::lookupSmallSize2Index(1), 0);
  for (size_t index = 0; index < kNumLookupIndices - 1; index++) {
    auto allocSize = kSizeIndex2Size[index];
    EXPECT_EQ(MemoryManager::lookupSmallSize2Index(allocSize - 1), index);
    EXPECT_EQ(MemoryManager::lookupSmallSize2Index(allocSize), index);
    EXPECT_EQ(MemoryManager::lookupSmallSize2Index(allocSize + 1), index + 1);
  }
  EXPECT_EQ(
    MemoryManager::lookupSmallSize2Index(kMaxSmallSizeLookup - 1),
    kNumLookupIndices - 1
  );
  EXPECT_EQ(
    MemoryManager::lookupSmallSize2Index(kMaxSmallSizeLookup),
    kNumLookupIndices - 1
  );
}

TEST(MemoryManager, SmallSize2Index) {
  EXPECT_EQ(MemoryManager::smallSize2Index(1), 0);
  for (size_t index = 0; index < kNumSmallSizes - 1; index++) {
    auto allocSize = kSizeIndex2Size[index];
    EXPECT_EQ(MemoryManager::smallSize2Index(allocSize - 1), index);
    EXPECT_EQ(MemoryManager::smallSize2Index(allocSize), index);
    EXPECT_EQ(MemoryManager::smallSize2Index(allocSize + 1), index + 1);
  }
  EXPECT_EQ(
    MemoryManager::smallSize2Index(kSizeIndex2Size[kNumSmallSizes - 1] - 1),
    kNumSmallSizes - 1
  );
  EXPECT_EQ(
    MemoryManager::smallSize2Index(kSizeIndex2Size[kNumSmallSizes - 1]),
    kNumSmallSizes - 1
  );
}

TEST(MemoryManager, Size2Index) {
  EXPECT_EQ(MemoryManager::size2Index(1), 0);
  for (size_t index = 0; index < kNumSizeClasses - 1; index++) {
    auto allocSize = kSizeIndex2Size[index];
    EXPECT_EQ(MemoryManager::size2Index(allocSize - 1), index);
    EXPECT_EQ(MemoryManager::size2Index(allocSize), index);
    EXPECT_EQ(MemoryManager::size2Index(allocSize + 1), index + 1);
  }
  EXPECT_EQ(
    MemoryManager::size2Index(kSizeIndex2Size[kNumSizeClasses - 1] - 1),
    kNumSizeClasses - 1
  );
  EXPECT_EQ(
    MemoryManager::size2Index(kSizeIndex2Size[kNumSizeClasses - 1]),
    kNumSizeClasses - 1
  );
}

TEST(MemoryManager, SmallSizeClass) {
  // this test starts by requesting 2 bytes because smallSizeClass does not
  // support inputs < 2; the others support inputs >= 1
  EXPECT_EQ(MemoryManager::smallSizeClass(2), kSmallSizeAlign);
  for (size_t index = 0; index < kNumSmallSizes - 1; index++) {
    auto allocSize = kSizeIndex2Size[index];
    EXPECT_EQ(MemoryManager::smallSizeClass(allocSize - 1), allocSize);
    EXPECT_EQ(MemoryManager::smallSizeClass(allocSize), allocSize);
    EXPECT_GT(MemoryManager::smallSizeClass(allocSize + 1), allocSize);
  }
  EXPECT_EQ(
    MemoryManager::smallSizeClass(kSizeIndex2Size[kNumSmallSizes - 1] - 1),
    kSizeIndex2Size[kNumSmallSizes - 1]
  );
  EXPECT_EQ(
    MemoryManager::smallSizeClass(kSizeIndex2Size[kNumSmallSizes - 1]),
    kSizeIndex2Size[kNumSmallSizes - 1]
  );
}

TEST(MemoryManager, realloc) {
  auto p = req::malloc_noptrs(kMaxSmallSize*2);
  auto const n = static_cast<MallocNode*>(p) - 1;
  EXPECT_EQ(n->kind(), HeaderKind::BigMalloc);
  auto p2 = req::realloc_noptrs(p, kMaxSmallSize/2);
  auto const n2 = static_cast<MallocNode*>(p2) - 1;
  EXPECT_EQ(n2->kind(), HeaderKind::SmallMalloc);
  req::free(p2);
}

TEST(MemoryManager, Find) {
  for (size_t index = 0; index < 1000; ++index) {
    auto p = req::malloc_noptrs(kMaxSmallSize*2);
    auto const n = static_cast<MallocNode*>(p) - 1;
    auto p2 = req::malloc_noptrs(kMaxSmallSize/2);
    auto const n2 = static_cast<MallocNode*>(p2) - 1;
    EXPECT_TRUE(MM().find(n));
    EXPECT_TRUE(MM().find(n2));
    req::free(p);
    req::free(p2);
    EXPECT_FALSE(MM().find(n));
    EXPECT_TRUE(MM().find(n2));
  }
}

TEST(MemoryManager, Contains) {
  // note that contains() does not check BigAlloc
  for (size_t index = 0; index < 1000; ++index) {
    auto p = req::malloc_noptrs(kMaxSmallSize/2);
    auto const n = static_cast<MallocNode*>(p) - 1;
    EXPECT_TRUE(MM().contains(n));
    req::free(p);
    EXPECT_TRUE(MM().contains(n));
  }
}

}
