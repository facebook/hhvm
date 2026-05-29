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

#include <thrift/lib/cpp2/fast_thrift/frame/read/DirectStreamMap.h>

#include <folly/portability/GTest.h>

#include <memory>

namespace apache::thrift::fast_thrift::frame::read {
namespace {

// Move-only value type to mimic FragmentState
struct TestValue {
  int data{0};
  std::unique_ptr<int> ptr;

  TestValue() = default;
  explicit TestValue(int d) : data(d), ptr(std::make_unique<int>(d)) {}
  TestValue(TestValue&&) = default;
  TestValue& operator=(TestValue&&) = default;
};

// ============================================================================
// Basic Operations
// ============================================================================

TEST(DirectStreamMapTest, EmptyMap) {
  DirectStreamMap<int> map;
  EXPECT_TRUE(map.empty());
  EXPECT_EQ(map.size(), 0);
  EXPECT_EQ(map.find(1), map.end());
  EXPECT_FALSE(map.contains(1));
}

TEST(DirectStreamMapTest, InsertAndFind) {
  DirectStreamMap<int> map;
  auto [it, inserted] = map.emplace(1, 42);
  EXPECT_TRUE(inserted);
  EXPECT_NE(it, map.end());
  EXPECT_EQ(it->first, 1);
  EXPECT_EQ(it->second, 42);
  EXPECT_EQ(map.size(), 1);
  EXPECT_FALSE(map.empty());
}

TEST(DirectStreamMapTest, InsertDuplicate) {
  DirectStreamMap<int> map;
  map.emplace(1, 42);
  auto [it, inserted] = map.emplace(1, 99);
  EXPECT_FALSE(inserted);
  EXPECT_EQ(it->second, 42);
  EXPECT_EQ(map.size(), 1);
}

TEST(DirectStreamMapTest, FindMissing) {
  DirectStreamMap<int> map;
  map.emplace(1, 42);
  EXPECT_EQ(map.find(2), map.end());
  EXPECT_EQ(map.find(999), map.end());
  EXPECT_FALSE(map.contains(2));
}

TEST(DirectStreamMapTest, EraseByIterator) {
  DirectStreamMap<int> map;
  map.emplace(1, 42);
  auto it = map.find(1);
  map.erase(it);
  EXPECT_EQ(map.size(), 0);
  EXPECT_TRUE(map.empty());
  EXPECT_EQ(map.find(1), map.end());
}

TEST(DirectStreamMapTest, EraseEnd) {
  DirectStreamMap<int> map;
  map.erase(map.end()); // no-op
  EXPECT_EQ(map.size(), 0);
}

TEST(DirectStreamMapTest, Clear) {
  DirectStreamMap<int> map;
  map.emplace(1, 10);
  map.emplace(3, 30);
  map.emplace(5, 50);
  map.clear();
  EXPECT_EQ(map.size(), 0);
  EXPECT_TRUE(map.empty());
  EXPECT_EQ(map.find(1), map.end());
  EXPECT_EQ(map.find(3), map.end());
  EXPECT_EQ(map.find(5), map.end());
}

// ============================================================================
// Move-Only Values
// ============================================================================

TEST(DirectStreamMapTest, MoveOnlyValue) {
  DirectStreamMap<TestValue> map;
  auto [it, ok] = map.emplace(7, TestValue(42));
  EXPECT_TRUE(ok);
  EXPECT_EQ(it->second.data, 42);
  EXPECT_NE(it->second.ptr, nullptr);
  EXPECT_EQ(*it->second.ptr, 42);
}

TEST(DirectStreamMapTest, EraseReleasesResources) {
  DirectStreamMap<TestValue> map;
  map.emplace(7, TestValue(42));
  map.erase(map.find(7));
  EXPECT_EQ(map.find(7), map.end());
  EXPECT_EQ(map.size(), 0);
}

// ============================================================================
// RSocket Stream ID Patterns (increment by 2)
// ============================================================================

TEST(DirectStreamMapTest, OddStreamIds) {
  DirectStreamMap<int> map;
  for (uint32_t id = 1; id <= 21; id += 2) {
    map.emplace(id, static_cast<int>(id * 10));
  }
  EXPECT_EQ(map.size(), 11);
  for (uint32_t id = 1; id <= 21; id += 2) {
    auto it = map.find(id);
    ASSERT_NE(it, map.end()) << "Missing stream ID " << id;
    EXPECT_EQ(it->second, static_cast<int>(id * 10));
  }
}

TEST(DirectStreamMapTest, EvenStreamIds) {
  DirectStreamMap<int> map;
  for (uint32_t id = 2; id <= 22; id += 2) {
    map.emplace(id, static_cast<int>(id * 10));
  }
  EXPECT_EQ(map.size(), 11);
  for (uint32_t id = 2; id <= 22; id += 2) {
    auto it = map.find(id);
    ASSERT_NE(it, map.end()) << "Missing stream ID " << id;
    EXPECT_EQ(it->second, static_cast<int>(id * 10));
  }
}

// ============================================================================
// Linear Probing / Collision Handling
// ============================================================================

TEST(DirectStreamMapTest, ConsecutiveKeysCollide) {
  // Keys 2 and 3 both map to index 1 via (key >> 1).
  // Second insert must linear-probe.
  DirectStreamMap<int> map(16);
  map.emplace(2, 20);
  map.emplace(3, 30);
  EXPECT_EQ(map.size(), 2);
  EXPECT_EQ(map.find(2)->second, 20);
  EXPECT_EQ(map.find(3)->second, 30);
}

TEST(DirectStreamMapTest, ManyCollisions) {
  DirectStreamMap<int> map(16);
  for (uint32_t i = 1; i <= 10; ++i) {
    map.emplace(i, static_cast<int>(i));
  }
  EXPECT_EQ(map.size(), 10);
  for (uint32_t i = 1; i <= 10; ++i) {
    ASSERT_NE(map.find(i), map.end()) << "Missing key " << i;
    EXPECT_EQ(map.find(i)->second, static_cast<int>(i));
  }
}

// ============================================================================
// Backshift Delete Behavior
// ============================================================================

TEST(DirectStreamMapTest, EraseDoesNotBreakProbeChain) {
  DirectStreamMap<int> map(16);
  // 2 and 3 collide at index 1. Insert both, erase 2.
  // Backshift should pull 3 back into 2's slot, keeping it findable.
  map.emplace(2, 20);
  map.emplace(3, 30);
  map.erase(map.find(2));
  EXPECT_EQ(map.find(2), map.end());
  ASSERT_NE(map.find(3), map.end());
  EXPECT_EQ(map.find(3)->second, 30);
}

TEST(DirectStreamMapTest, ReinsertAfterErase) {
  DirectStreamMap<int> map;
  map.emplace(5, 50);
  map.erase(map.find(5));
  EXPECT_EQ(map.find(5), map.end());

  auto [it, ok] = map.emplace(5, 99);
  EXPECT_TRUE(ok);
  EXPECT_EQ(it->second, 99);
  EXPECT_EQ(map.size(), 1);
}

TEST(DirectStreamMapTest, BackshiftChain) {
  // Insert 3 keys that collide: 2, 3, 4 all map to indices 1, 1, 2.
  // Erase the first — backshift should compact the chain.
  DirectStreamMap<int> map(16);
  map.emplace(2, 20); // index 1
  map.emplace(3, 30); // index 1 -> probes to 2
  map.emplace(4, 40); // index 2 -> probes to 3
  EXPECT_EQ(map.size(), 3);

  map.erase(map.find(2));
  EXPECT_EQ(map.size(), 2);
  EXPECT_EQ(map.find(2), map.end());
  ASSERT_NE(map.find(3), map.end());
  EXPECT_EQ(map.find(3)->second, 30);
  ASSERT_NE(map.find(4), map.end());
  EXPECT_EQ(map.find(4)->second, 40);
}

TEST(DirectStreamMapTest, ManyErasesDoNotCorrupt) {
  DirectStreamMap<int> map;
  for (uint32_t i = 1; i <= 100; ++i) {
    map.emplace(i, static_cast<int>(i));
  }
  // Erase all even keys
  for (uint32_t i = 2; i <= 100; i += 2) {
    map.erase(map.find(i));
  }
  EXPECT_EQ(map.size(), 50);
  for (uint32_t i = 1; i <= 100; i += 2) {
    EXPECT_TRUE(map.contains(i)) << "Missing key " << i;
  }
  for (uint32_t i = 2; i <= 100; i += 2) {
    EXPECT_FALSE(map.contains(i)) << "Found erased key " << i;
  }
}

// ============================================================================
// Dynamic Resize
// ============================================================================

TEST(DirectStreamMapTest, ResizeOnHighLoad) {
  DirectStreamMap<int> map(16);
  EXPECT_EQ(map.capacity(), 16);
  // Load factor 7/8: resize triggers when size*8 > cap*7.
  // With cap=16: size=14 -> 112 > 112 false. size=15 -> 120 > 112 true.
  // So the 16th emplace (size_=15 at check) triggers resize.
  for (uint32_t i = 1; i <= 16; ++i) {
    map.emplace(i, static_cast<int>(i));
  }
  EXPECT_GT(map.capacity(), 16u);
  EXPECT_EQ(map.size(), 16);
  for (uint32_t i = 1; i <= 16; ++i) {
    ASSERT_TRUE(map.contains(i)) << "Missing key " << i << " after resize";
    EXPECT_EQ(map.find(i)->second, static_cast<int>(i));
  }
}

TEST(DirectStreamMapTest, ResizePreservesAllEntries) {
  DirectStreamMap<int> map(16);
  for (uint32_t i = 1; i <= 12; ++i) {
    map.emplace(i, static_cast<int>(i));
  }
  // Erase half, then insert enough to trigger resize
  for (uint32_t i = 1; i <= 6; ++i) {
    map.erase(map.find(i));
  }
  EXPECT_EQ(map.size(), 6);
  // Fill up to trigger resize (need size > cap * 7/8)
  for (uint32_t i = 100; i <= 112; ++i) {
    map.emplace(i, static_cast<int>(i));
  }
  // All surviving keys findable after rehash
  for (uint32_t i = 7; i <= 12; ++i) {
    EXPECT_TRUE(map.contains(i));
  }
  for (uint32_t i = 100; i <= 112; ++i) {
    EXPECT_TRUE(map.contains(i));
  }
}

// ============================================================================
// Large Scale
// ============================================================================

TEST(DirectStreamMapTest, TenThousandStreamIds) {
  DirectStreamMap<int> map;
  constexpr uint32_t kCount = 10000;
  for (uint32_t id = 1; id <= kCount * 2; id += 2) {
    map.emplace(id, static_cast<int>(id));
  }
  EXPECT_EQ(map.size(), kCount);
  for (uint32_t id = 1; id <= kCount * 2; id += 2) {
    ASSERT_TRUE(map.contains(id)) << "Missing stream ID " << id;
    EXPECT_EQ(map.find(id)->second, static_cast<int>(id));
  }
}

// ============================================================================
// Steady-State Cycles (simulates handler usage)
// ============================================================================

TEST(DirectStreamMapTest, InsertEraseRepeated) {
  DirectStreamMap<int> map;
  for (uint32_t round = 0; round < 1000; ++round) {
    uint32_t streamId = round * 2 + 1;
    map.emplace(streamId, static_cast<int>(streamId));
    map.erase(map.find(streamId));
  }
  EXPECT_EQ(map.size(), 0);
}

TEST(DirectStreamMapTest, InterleavedInsertErase) {
  DirectStreamMap<int> map;
  constexpr int kStreams = 10;
  for (int round = 0; round < 100; ++round) {
    uint32_t base = static_cast<uint32_t>(round * kStreams * 2 + 1);
    for (int s = 0; s < kStreams; ++s) {
      uint32_t id = base + static_cast<uint32_t>(s * 2);
      map.emplace(id, static_cast<int>(id));
    }
    EXPECT_EQ(map.size(), static_cast<size_t>(kStreams));
    for (int s = 0; s < kStreams; ++s) {
      uint32_t id = base + static_cast<uint32_t>(s * 2);
      map.erase(map.find(id));
    }
    EXPECT_EQ(map.size(), 0);
  }
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(DirectStreamMapTest, SmallCapRoundsUp) {
  DirectStreamMap<int> map(3);
  EXPECT_GE(map.capacity(), 16u);
}

TEST(DirectStreamMapTest, CustomCapacity) {
  DirectStreamMap<int> map(1024);
  EXPECT_GE(map.capacity(), 1024u);
}

TEST(DirectStreamMapTest, ModifyValueInPlace) {
  DirectStreamMap<int> map;
  map.emplace(1, 10);
  map.find(1)->second = 42;
  EXPECT_EQ(map.find(1)->second, 42);
}

} // namespace
} // namespace apache::thrift::fast_thrift::frame::read
