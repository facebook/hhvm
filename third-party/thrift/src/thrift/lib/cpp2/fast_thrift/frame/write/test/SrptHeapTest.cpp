/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/fast_thrift/frame/write/SrptHeap.h>

#include <folly/portability/GTest.h>

#include <algorithm>
#include <memory>
#include <numeric>
#include <vector>

namespace apache::thrift::fast_thrift::frame::write {
namespace {

// Simple value type with a priority field.
struct TestEntry {
  size_t remaining{0};

  TestEntry() = default;
  explicit TestEntry(size_t r) : remaining(r) {}

  TestEntry(TestEntry&&) = default;
  TestEntry& operator=(TestEntry&&) = default;
};

struct TestKeyFn {
  size_t operator()(const TestEntry& e) const noexcept { return e.remaining; }
};

using TestHeap = SrptHeap<TestEntry, TestKeyFn>;

// Move-only value type to verify ownership semantics.
struct MoveOnlyEntry {
  size_t remaining{0};
  std::unique_ptr<int> data;

  MoveOnlyEntry() = default;
  MoveOnlyEntry(size_t r, int d)
      : remaining(r), data(std::make_unique<int>(d)) {}
  MoveOnlyEntry(MoveOnlyEntry&&) = default;
  MoveOnlyEntry& operator=(MoveOnlyEntry&&) = default;
};

struct MoveOnlyKeyFn {
  size_t operator()(const MoveOnlyEntry& e) const noexcept {
    return e.remaining;
  }
};

using MoveOnlyHeap = SrptHeap<MoveOnlyEntry, MoveOnlyKeyFn>;

// ============================================================================
// Basic Operations
// ============================================================================

TEST(SrptHeapTest, EmptyHeap) {
  TestHeap heap;
  EXPECT_TRUE(heap.empty());
  EXPECT_EQ(heap.size(), 0);
  EXPECT_FALSE(heap.contains(1));
  EXPECT_EQ(heap.find(1), nullptr);
}

TEST(SrptHeapTest, InsertSingle) {
  TestHeap heap;
  auto& ref = heap.insert(1, TestEntry(100));
  EXPECT_EQ(ref.remaining, 100);
  EXPECT_EQ(heap.size(), 1);
  EXPECT_FALSE(heap.empty());
  EXPECT_TRUE(heap.contains(1));
}

TEST(SrptHeapTest, FindExisting) {
  TestHeap heap;
  heap.insert(1, TestEntry(100));
  auto* ptr = heap.find(1);
  ASSERT_NE(ptr, nullptr);
  EXPECT_EQ(ptr->remaining, 100);
}

TEST(SrptHeapTest, FindMissing) {
  TestHeap heap;
  heap.insert(1, TestEntry(100));
  EXPECT_EQ(heap.find(2), nullptr);
}

TEST(SrptHeapTest, PeekMin) {
  TestHeap heap;
  heap.insert(1, TestEntry(100));
  heap.insert(3, TestEntry(10));
  heap.insert(5, TestEntry(50));

  EXPECT_EQ(heap.peekMin().remaining, 10);
  EXPECT_EQ(heap.peekMinStreamId(), 3);
}

TEST(SrptHeapTest, ExtractMin) {
  TestHeap heap;
  heap.insert(1, TestEntry(100));
  heap.insert(3, TestEntry(10));
  heap.insert(5, TestEntry(50));

  auto val = heap.extractMin();
  EXPECT_EQ(val.remaining, 10);
  EXPECT_EQ(heap.size(), 2);
  EXPECT_FALSE(heap.contains(3));

  // Next min should be 50
  EXPECT_EQ(heap.peekMin().remaining, 50);
}

TEST(SrptHeapTest, ExtractAllInOrder) {
  TestHeap heap;
  heap.insert(1, TestEntry(50));
  heap.insert(3, TestEntry(10));
  heap.insert(5, TestEntry(30));
  heap.insert(7, TestEntry(20));
  heap.insert(9, TestEntry(40));

  std::vector<size_t> extracted;
  while (!heap.empty()) {
    extracted.push_back(heap.extractMin().remaining);
  }
  std::vector<size_t> expected = {10, 20, 30, 40, 50};
  EXPECT_EQ(extracted, expected);
}

// ============================================================================
// Erase
// ============================================================================

TEST(SrptHeapTest, EraseExisting) {
  TestHeap heap;
  heap.insert(1, TestEntry(100));
  heap.insert(3, TestEntry(10));
  EXPECT_TRUE(heap.erase(1));
  EXPECT_EQ(heap.size(), 1);
  EXPECT_FALSE(heap.contains(1));
  EXPECT_TRUE(heap.contains(3));
}

TEST(SrptHeapTest, EraseMissing) {
  TestHeap heap;
  heap.insert(1, TestEntry(100));
  EXPECT_FALSE(heap.erase(999));
  EXPECT_EQ(heap.size(), 1);
}

TEST(SrptHeapTest, EraseMin) {
  TestHeap heap;
  heap.insert(1, TestEntry(10));
  heap.insert(3, TestEntry(50));
  heap.insert(5, TestEntry(30));

  heap.erase(1); // erase the min
  EXPECT_EQ(heap.peekMin().remaining, 30);
  EXPECT_EQ(heap.size(), 2);
}

TEST(SrptHeapTest, EraseLast) {
  TestHeap heap;
  heap.insert(1, TestEntry(10));
  heap.erase(1);
  EXPECT_TRUE(heap.empty());
}

TEST(SrptHeapTest, EraseMiddle) {
  TestHeap heap;
  heap.insert(1, TestEntry(10));
  heap.insert(3, TestEntry(50));
  heap.insert(5, TestEntry(30));
  heap.insert(7, TestEntry(20));
  heap.insert(9, TestEntry(40));

  heap.erase(5); // erase entry with remaining=30
  EXPECT_EQ(heap.size(), 4);
  EXPECT_FALSE(heap.contains(5));

  // Heap property preserved — extract all and verify sorted
  std::vector<size_t> extracted;
  while (!heap.empty()) {
    extracted.push_back(heap.extractMin().remaining);
  }
  std::vector<size_t> expected = {10, 20, 40, 50};
  EXPECT_EQ(extracted, expected);
}

// ============================================================================
// Update (priority change)
// ============================================================================

TEST(SrptHeapTest, UpdateDecreasePriority) {
  TestHeap heap;
  heap.insert(1, TestEntry(100));
  heap.insert(3, TestEntry(50));
  heap.insert(5, TestEntry(80));

  // Decrease stream 1's remaining from 100 to 5 — should become new min
  auto* ptr = heap.find(1);
  ptr->remaining = 5;
  heap.update(1);

  EXPECT_EQ(heap.peekMin().remaining, 5);
  EXPECT_EQ(heap.peekMinStreamId(), 1);
}

TEST(SrptHeapTest, UpdateIncreasePriority) {
  TestHeap heap;
  heap.insert(1, TestEntry(10));
  heap.insert(3, TestEntry(50));
  heap.insert(5, TestEntry(30));

  // Increase stream 1's remaining from 10 to 100 — should no longer be min
  auto* ptr = heap.find(1);
  ptr->remaining = 100;
  heap.update(1);

  EXPECT_EQ(heap.peekMin().remaining, 30);
  EXPECT_EQ(heap.peekMinStreamId(), 5);
}

TEST(SrptHeapTest, UpdateNonExistent) {
  TestHeap heap;
  heap.insert(1, TestEntry(10));
  heap.update(999); // no-op
  EXPECT_EQ(heap.size(), 1);
}

TEST(SrptHeapTest, UpdateAfterFlush) {
  // Simulates the fragmentation pattern: peekMin, flush some bytes, update
  TestHeap heap;
  heap.insert(1, TestEntry(1000));
  heap.insert(3, TestEntry(200));
  heap.insert(5, TestEntry(500));

  // SRPT: flush 100 bytes from min (stream 3, remaining 200)
  EXPECT_EQ(heap.peekMinStreamId(), 3);
  heap.peekMin().remaining -= 100;
  heap.update(3);
  EXPECT_EQ(heap.peekMin().remaining, 100);

  // Flush another 100 — stream 3 done
  heap.peekMin().remaining -= 100;
  EXPECT_EQ(heap.peekMin().remaining, 0);
  heap.extractMin();

  // Next min is stream 5 (500)
  EXPECT_EQ(heap.peekMinStreamId(), 5);
  EXPECT_EQ(heap.peekMin().remaining, 500);
}

// ============================================================================
// SRPT scheduling correctness
// ============================================================================

TEST(SrptHeapTest, SrptFlushOrder) {
  // Verify that repeatedly flushing min produces optimal SRPT order
  TestHeap heap;
  heap.insert(1, TestEntry(50));
  heap.insert(3, TestEntry(10));
  heap.insert(5, TestEntry(30));
  heap.insert(7, TestEntry(20));

  // SRPT order: flush stream with least remaining first
  // Expected flush order: 3(10), 7(20), 5(30), 1(50)
  std::vector<uint32_t> flushOrder;
  while (!heap.empty()) {
    flushOrder.push_back(heap.peekMinStreamId());
    // Simulate flushing all remaining bytes at once
    heap.extractMin();
  }

  std::vector<uint32_t> expected = {3, 7, 5, 1};
  EXPECT_EQ(flushOrder, expected);
}

TEST(SrptHeapTest, SrptWithPartialFlush) {
  // Simulate partial flushes (fragment by fragment)
  TestHeap heap;
  heap.insert(1, TestEntry(30)); // 3 fragments of 10
  heap.insert(3, TestEntry(10)); // 1 fragment

  constexpr size_t kFragmentSize = 10;
  std::vector<uint32_t> flushOrder;

  while (!heap.empty()) {
    uint32_t id = heap.peekMinStreamId();
    auto& min = heap.peekMin();
    flushOrder.push_back(id);

    if (min.remaining <= kFragmentSize) {
      heap.extractMin();
    } else {
      min.remaining -= kFragmentSize;
      heap.update(id);
    }
  }

  // stream 3 (10 bytes) flushed first in 1 step
  // then stream 1 flushed in 3 steps of 10 bytes each
  std::vector<uint32_t> expected = {3, 1, 1, 1};
  EXPECT_EQ(flushOrder, expected);
}

TEST(SrptHeapTest, SrptDynamicArrival) {
  // New streams arrive while flushing
  TestHeap heap;
  heap.insert(1, TestEntry(100));

  constexpr size_t kFragmentSize = 10;

  // Flush 2 fragments from stream 1
  heap.peekMin().remaining -= kFragmentSize;
  heap.update(1);
  heap.peekMin().remaining -= kFragmentSize;
  heap.update(1);
  EXPECT_EQ(heap.peekMin().remaining, 80);

  // New small stream arrives — should preempt
  heap.insert(3, TestEntry(15));
  EXPECT_EQ(heap.peekMinStreamId(), 3);
  EXPECT_EQ(heap.peekMin().remaining, 15);
}

// ============================================================================
// Move-only values
// ============================================================================

TEST(SrptHeapTest, MoveOnlyInsertAndExtract) {
  MoveOnlyHeap heap;
  heap.insert(1, MoveOnlyEntry(100, 42));
  heap.insert(3, MoveOnlyEntry(10, 7));

  EXPECT_EQ(heap.peekMin().remaining, 10);
  EXPECT_NE(heap.peekMin().data, nullptr);
  EXPECT_EQ(*heap.peekMin().data, 7);

  auto val = heap.extractMin();
  EXPECT_EQ(val.remaining, 10);
  EXPECT_NE(val.data, nullptr);
  EXPECT_EQ(*val.data, 7);
}

// ============================================================================
// forEach
// ============================================================================

TEST(SrptHeapTest, ForEach) {
  TestHeap heap;
  heap.insert(1, TestEntry(10));
  heap.insert(3, TestEntry(30));
  heap.insert(5, TestEntry(50));

  size_t totalRemaining = 0;
  size_t count = 0;
  heap.forEach([&](uint32_t, TestEntry& e) {
    totalRemaining += e.remaining;
    count++;
  });

  EXPECT_EQ(count, 3);
  EXPECT_EQ(totalRemaining, 90);
}

// ============================================================================
// Clear
// ============================================================================

TEST(SrptHeapTest, Clear) {
  TestHeap heap;
  heap.insert(1, TestEntry(10));
  heap.insert(3, TestEntry(30));
  heap.clear();
  EXPECT_TRUE(heap.empty());
  EXPECT_EQ(heap.size(), 0);
  EXPECT_FALSE(heap.contains(1));
}

// ============================================================================
// 4-ary structure: enough entries to exercise multi-level sifts
// ============================================================================

TEST(SrptHeapTest, MultiLevelSift) {
  TestHeap heap;
  // Insert 20 entries — forces a 3-level 4-ary tree
  // Level 0: 1 node, Level 1: 4 nodes, Level 2: 16 nodes
  for (uint32_t i = 1; i <= 20; ++i) {
    heap.insert(i, TestEntry(i * 10));
  }
  EXPECT_EQ(heap.size(), 20);

  // Min should be stream 1 (10 bytes)
  EXPECT_EQ(heap.peekMinStreamId(), 1);
  EXPECT_EQ(heap.peekMin().remaining, 10);

  // Extract all and verify sorted order
  std::vector<size_t> extracted;
  while (!heap.empty()) {
    extracted.push_back(heap.extractMin().remaining);
  }

  std::vector<size_t> expected(20);
  for (size_t i = 0; i < 20; ++i) {
    expected[i] = (i + 1) * 10;
  }
  EXPECT_EQ(extracted, expected);
}

TEST(SrptHeapTest, LargeScaleInsertExtract) {
  TestHeap heap;
  constexpr uint32_t kCount = 1000;

  // Insert in reverse order — worst case for min-heap
  for (uint32_t i = kCount; i >= 1; --i) {
    heap.insert(i, TestEntry(i));
  }
  EXPECT_EQ(heap.size(), kCount);
  EXPECT_EQ(heap.peekMinStreamId(), 1);

  // Extract all — must come out sorted
  size_t prev = 0;
  for (uint32_t i = 0; i < kCount; ++i) {
    auto val = heap.extractMin();
    EXPECT_GE(val.remaining, prev) << "Heap order violation at i=" << i;
    prev = val.remaining;
  }
  EXPECT_TRUE(heap.empty());
}

TEST(SrptHeapTest, InterleavedInsertEraseUpdate) {
  TestHeap heap;

  // Simulate steady-state: insert streams, flush some, erase done, insert new
  for (uint32_t round = 0; round < 50; ++round) {
    uint32_t baseId = round * 10;

    // Insert 5 streams
    for (uint32_t i = 0; i < 5; ++i) {
      heap.insert(baseId + i, TestEntry((i + 1) * 100));
    }

    // Flush smallest partially
    if (!heap.empty()) {
      heap.peekMin().remaining /= 2;
      heap.update(heap.peekMinStreamId());
    }

    // Erase the first stream in this batch
    heap.erase(baseId);
  }

  // Verify heap property by extracting all
  size_t prev = 0;
  while (!heap.empty()) {
    auto val = heap.extractMin();
    EXPECT_GE(val.remaining, prev);
    prev = val.remaining;
  }
}

// ============================================================================
// Edge cases
// ============================================================================

TEST(SrptHeapTest, EqualPriorities) {
  TestHeap heap;
  heap.insert(1, TestEntry(50));
  heap.insert(3, TestEntry(50));
  heap.insert(5, TestEntry(50));

  // All have same priority — any is a valid min
  EXPECT_EQ(heap.peekMin().remaining, 50);

  auto v1 = heap.extractMin();
  auto v2 = heap.extractMin();
  auto v3 = heap.extractMin();
  EXPECT_EQ(v1.remaining, 50);
  EXPECT_EQ(v2.remaining, 50);
  EXPECT_EQ(v3.remaining, 50);
  EXPECT_TRUE(heap.empty());
}

TEST(SrptHeapTest, InsertAfterClear) {
  TestHeap heap;
  heap.insert(1, TestEntry(10));
  heap.clear();

  heap.insert(3, TestEntry(20));
  EXPECT_EQ(heap.size(), 1);
  EXPECT_EQ(heap.peekMinStreamId(), 3);
}

TEST(SrptHeapTest, ZeroPriority) {
  TestHeap heap;
  heap.insert(1, TestEntry(100));
  heap.insert(3, TestEntry(0));

  EXPECT_EQ(heap.peekMinStreamId(), 3);
  EXPECT_EQ(heap.peekMin().remaining, 0);
}

} // namespace
} // namespace apache::thrift::fast_thrift::frame::write
