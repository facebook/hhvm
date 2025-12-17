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

#include "hphp/util/sparse-bitset.h"
#include <folly/portability/GTest.h>
#include <algorithm>
#include <vector>

namespace HPHP {

TEST(SparseBitsetTest, EmptySet) {
  SparseBitset<> bitset;
  EXPECT_TRUE(bitset.none());
  EXPECT_TRUE(bitset.empty());
  EXPECT_FALSE(bitset.any());
  EXPECT_EQ(0, bitset.size());
  EXPECT_FALSE(bitset.contains(0));
  EXPECT_FALSE(bitset.contains(100));
  EXPECT_FALSE(bitset.contains(1000000));
}

TEST(SparseBitsetTest, AddSingleElement) {
  SparseBitset<> bitset;
  bitset.add(42);

  EXPECT_FALSE(bitset.none());
  EXPECT_FALSE(bitset.empty());
  EXPECT_TRUE(bitset.any());
  EXPECT_EQ(1, bitset.size());
  EXPECT_TRUE(bitset.contains(42));
  EXPECT_FALSE(bitset.contains(41));
  EXPECT_FALSE(bitset.contains(43));
}

TEST(SparseBitsetTest, AddMultipleElements) {
  SparseBitset<> bitset;
  bitset.add(10);
  bitset.add(20);
  bitset.add(30);

  EXPECT_EQ(3, bitset.size());
  EXPECT_TRUE(bitset.contains(10));
  EXPECT_TRUE(bitset.contains(20));
  EXPECT_TRUE(bitset.contains(30));
  EXPECT_FALSE(bitset.contains(15));
}

TEST(SparseBitsetTest, AddDuplicates) {
  SparseBitset<> bitset;
  bitset.add(42);
  bitset.add(42);
  bitset.add(42);

  EXPECT_EQ(1, bitset.size());
  EXPECT_TRUE(bitset.contains(42));
}

TEST(SparseBitsetTest, AddSparseElements) {
  SparseBitset<> bitset;
  bitset.add(0);
  bitset.add(1000);
  bitset.add(1000000);
  bitset.add(1000000000);

  EXPECT_EQ(4, bitset.size());
  EXPECT_TRUE(bitset.contains(0));
  EXPECT_TRUE(bitset.contains(1000));
  EXPECT_TRUE(bitset.contains(1000000));
  EXPECT_TRUE(bitset.contains(1000000000));
  EXPECT_FALSE(bitset.contains(500));
}

TEST(SparseBitsetTest, RemoveElement) {
  SparseBitset<> bitset;
  bitset.add(10);
  bitset.add(20);
  bitset.add(30);

  bitset.remove(20);
  EXPECT_EQ(2, bitset.size());
  EXPECT_TRUE(bitset.contains(10));
  EXPECT_FALSE(bitset.contains(20));
  EXPECT_TRUE(bitset.contains(30));
}

TEST(SparseBitsetTest, RemoveNonexistent) {
  SparseBitset<> bitset;
  bitset.add(10);

  bitset.remove(20);  // Should be a no-op
  EXPECT_EQ(1, bitset.size());
  EXPECT_TRUE(bitset.contains(10));
}

TEST(SparseBitsetTest, RemoveLastElement) {
  SparseBitset<> bitset;
  bitset.add(42);
  bitset.remove(42);

  EXPECT_TRUE(bitset.none());
  EXPECT_EQ(0, bitset.size());
  EXPECT_FALSE(bitset.contains(42));
}

TEST(SparseBitsetTest, Clear) {
  SparseBitset<> bitset;
  bitset.add(10);
  bitset.add(20);
  bitset.add(30);

  bitset.clear();
  EXPECT_TRUE(bitset.none());
  EXPECT_TRUE(bitset.empty());
  EXPECT_EQ(0, bitset.size());
  EXPECT_FALSE(bitset.contains(10));
  EXPECT_FALSE(bitset.contains(20));
  EXPECT_FALSE(bitset.contains(30));
}

TEST(SparseBitsetTest, ForEachEmpty) {
  SparseBitset<> bitset;
  bool called = false;
  bitset.forEach([&](uint64_t) { called = true; });
  EXPECT_FALSE(called);
}

TEST(SparseBitsetTest, ForEachSingleElement) {
  SparseBitset<> bitset;
  bitset.add(42);

  std::vector<uint64_t> elements;
  bitset.forEach([&](uint64_t x) { elements.push_back(x); });

  EXPECT_EQ(1, elements.size());
  EXPECT_EQ(42, elements[0]);
}

TEST(SparseBitsetTest, ForEachMultipleElements) {
  SparseBitset<> bitset;
  std::vector<uint64_t> input = {10, 50, 100, 500, 1000};
  for (auto x : input) {
    bitset.add(x);
  }

  std::vector<uint64_t> output;
  bitset.forEach([&](uint64_t x) { output.push_back(x); });

  EXPECT_EQ(input, output);  // Should be in ascending order
}

TEST(SparseBitsetTest, ForEachSparseElements) {
  SparseBitset<> bitset;
  bitset.add(0);
  bitset.add(1000);
  bitset.add(1000000);

  std::vector<uint64_t> elements;
  bitset.forEach([&](uint64_t x) { elements.push_back(x); });

  EXPECT_EQ(3, elements.size());
  EXPECT_EQ(0, elements[0]);
  EXPECT_EQ(1000, elements[1]);
  EXPECT_EQ(1000000, elements[2]);
}

TEST(SparseBitsetTest, Equality) {
  SparseBitset<> a, b;

  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a != b);

  a.add(10);
  EXPECT_FALSE(a == b);
  EXPECT_TRUE(a != b);

  b.add(10);
  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a != b);

  a.add(20);
  b.add(30);
  EXPECT_FALSE(a == b);
  EXPECT_TRUE(a != b);
}

TEST(SparseBitsetTest, IntersectionEmpty) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);

  auto result = a & b;
  EXPECT_TRUE(result.none());
  EXPECT_EQ(0, result.size());
}

TEST(SparseBitsetTest, IntersectionDisjoint) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);
  b.add(30);
  b.add(40);

  auto result = a & b;
  EXPECT_TRUE(result.none());
  EXPECT_EQ(0, result.size());
}

TEST(SparseBitsetTest, IntersectionOverlap) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);
  a.add(30);
  b.add(20);
  b.add(30);
  b.add(40);

  auto result = a & b;
  EXPECT_EQ(2, result.size());
  EXPECT_FALSE(result.contains(10));
  EXPECT_TRUE(result.contains(20));
  EXPECT_TRUE(result.contains(30));
  EXPECT_FALSE(result.contains(40));
}

TEST(SparseBitsetTest, IntersectionIdentical) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);
  b.add(10);
  b.add(20);

  auto result = a & b;
  EXPECT_EQ(a, result);
  EXPECT_EQ(b, result);
}

TEST(SparseBitsetTest, UnionEmpty) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);

  auto result = a | b;
  EXPECT_EQ(a, result);
}

TEST(SparseBitsetTest, UnionDisjoint) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);
  b.add(30);
  b.add(40);

  auto result = a | b;
  EXPECT_EQ(4, result.size());
  EXPECT_TRUE(result.contains(10));
  EXPECT_TRUE(result.contains(20));
  EXPECT_TRUE(result.contains(30));
  EXPECT_TRUE(result.contains(40));
}

TEST(SparseBitsetTest, UnionOverlap) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);
  a.add(30);
  b.add(20);
  b.add(30);
  b.add(40);

  auto result = a | b;
  EXPECT_EQ(4, result.size());
  EXPECT_TRUE(result.contains(10));
  EXPECT_TRUE(result.contains(20));
  EXPECT_TRUE(result.contains(30));
  EXPECT_TRUE(result.contains(40));
}

TEST(SparseBitsetTest, UnionIdentical) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);
  b.add(10);
  b.add(20);

  auto result = a | b;
  EXPECT_EQ(a, result);
}

TEST(SparseBitsetTest, IntersectionAssignEmpty) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);

  a &= b;
  EXPECT_TRUE(a.none());
  EXPECT_EQ(0, a.size());
}

TEST(SparseBitsetTest, IntersectionAssignOverlap) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);
  a.add(30);
  b.add(20);
  b.add(30);
  b.add(40);

  a &= b;
  EXPECT_EQ(2, a.size());
  EXPECT_FALSE(a.contains(10));
  EXPECT_TRUE(a.contains(20));
  EXPECT_TRUE(a.contains(30));
  EXPECT_FALSE(a.contains(40));
}

TEST(SparseBitsetTest, IntersectionAssignSelf) {
  SparseBitset<> a;
  a.add(10);
  a.add(20);

  auto original = a;
  a &= a;
  EXPECT_EQ(original, a);
}

TEST(SparseBitsetTest, UnionAssignEmpty) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);

  auto original = a;
  a |= b;
  EXPECT_EQ(original, a);
}

TEST(SparseBitsetTest, UnionAssignDisjoint) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);
  b.add(30);
  b.add(40);

  a |= b;
  EXPECT_EQ(4, a.size());
  EXPECT_TRUE(a.contains(10));
  EXPECT_TRUE(a.contains(20));
  EXPECT_TRUE(a.contains(30));
  EXPECT_TRUE(a.contains(40));
}

TEST(SparseBitsetTest, UnionAssignSelf) {
  SparseBitset<> a;
  a.add(10);
  a.add(20);

  auto original = a;
  a |= a;
  EXPECT_EQ(original, a);
}

TEST(SparseBitsetTest, IsSubsetOfEmpty) {
  SparseBitset<> a, b;
  EXPECT_TRUE(a.isSubsetOf(b));

  b.add(10);
  EXPECT_TRUE(a.isSubsetOf(b));

  a.add(20);
  EXPECT_FALSE(a.isSubsetOf(b));
}

TEST(SparseBitsetTest, IsSubsetOfIdentical) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);
  b.add(10);
  b.add(20);

  EXPECT_TRUE(a.isSubsetOf(b));
  EXPECT_TRUE(b.isSubsetOf(a));
}

TEST(SparseBitsetTest, IsSubsetOfTrue) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);
  b.add(10);
  b.add(20);
  b.add(30);

  EXPECT_TRUE(a.isSubsetOf(b));
  EXPECT_FALSE(b.isSubsetOf(a));
}

TEST(SparseBitsetTest, IsSubsetOfFalse) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);
  a.add(30);
  b.add(10);
  b.add(20);

  EXPECT_FALSE(a.isSubsetOf(b));
}

TEST(SparseBitsetTest, IsSubsetOfPartialOverlap) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);
  b.add(20);
  b.add(30);

  EXPECT_FALSE(a.isSubsetOf(b));
  EXPECT_FALSE(b.isSubsetOf(a));
}

TEST(SparseBitsetTest, ForEachIsectEmpty) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);

  bool called = false;
  a.forEachIsect(b, [&](uint64_t) { called = true; });
  EXPECT_FALSE(called);
}

TEST(SparseBitsetTest, ForEachIsectDisjoint) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);
  b.add(30);
  b.add(40);

  bool called = false;
  a.forEachIsect(b, [&](uint64_t) { called = true; });
  EXPECT_FALSE(called);
}

TEST(SparseBitsetTest, ForEachIsectOverlap) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);
  a.add(30);
  b.add(20);
  b.add(30);
  b.add(40);

  std::vector<uint64_t> elements;
  a.forEachIsect(b, [&](uint64_t x) { elements.push_back(x); });

  EXPECT_EQ(2, elements.size());
  EXPECT_EQ(20, elements[0]);
  EXPECT_EQ(30, elements[1]);
}

TEST(SparseBitsetTest, SwapMember) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);
  b.add(30);
  b.add(40);

  auto a_copy = a;
  auto b_copy = b;

  a.swap(b);
  EXPECT_EQ(b_copy, a);
  EXPECT_EQ(a_copy, b);
}

TEST(SparseBitsetTest, SwapStd) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);
  b.add(30);
  b.add(40);

  auto a_copy = a;
  auto b_copy = b;

  std::swap(a, b);
  EXPECT_EQ(b_copy, a);
  EXPECT_EQ(a_copy, b);
}

TEST(SparseBitsetTest, CopyConstructor) {
  SparseBitset<> a;
  a.add(10);
  a.add(20);
  a.add(30);

  SparseBitset<> b = a;
  EXPECT_EQ(a, b);

  // Verify deep copy
  a.add(40);
  EXPECT_NE(a, b);
  EXPECT_FALSE(b.contains(40));
}

TEST(SparseBitsetTest, MoveConstructor) {
  SparseBitset<> a;
  a.add(10);
  a.add(20);
  a.add(30);

  auto a_copy = a;
  SparseBitset<> b = std::move(a);
  EXPECT_EQ(a_copy, b);
}

TEST(SparseBitsetTest, CopyAssignment) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);
  b.add(30);

  b = a;
  EXPECT_EQ(a, b);

  // Verify deep copy
  a.add(40);
  EXPECT_NE(a, b);
  EXPECT_FALSE(b.contains(40));
}

TEST(SparseBitsetTest, MoveAssignment) {
  SparseBitset<> a, b;
  a.add(10);
  a.add(20);
  b.add(30);

  auto a_copy = a;
  b = std::move(a);
  EXPECT_EQ(a_copy, b);
}

TEST(SparseBitsetTest, DifferentChunkSizes) {
  SparseBitset<64> small;
  SparseBitset<512> large;

  small.add(100);
  large.add(100);

  EXPECT_TRUE(small.contains(100));
  EXPECT_TRUE(large.contains(100));
  EXPECT_EQ(1, small.size());
  EXPECT_EQ(1, large.size());
}

TEST(SparseBitsetTest, SameChunkMultipleElements) {
  SparseBitset<256> bitset;
  // Add elements within the same chunk (0-255)
  bitset.add(10);
  bitset.add(20);
  bitset.add(100);
  bitset.add(200);

  EXPECT_EQ(4, bitset.size());
  EXPECT_TRUE(bitset.contains(10));
  EXPECT_TRUE(bitset.contains(20));
  EXPECT_TRUE(bitset.contains(100));
  EXPECT_TRUE(bitset.contains(200));
}

TEST(SparseBitsetTest, MultipleChunks) {
  SparseBitset<256> bitset;
  // Add elements in different chunks
  bitset.add(10);    // Chunk 0
  bitset.add(300);   // Chunk 1
  bitset.add(600);   // Chunk 2
  bitset.add(1000);  // Chunk 3

  EXPECT_EQ(4, bitset.size());
  std::vector<uint64_t> elements;
  bitset.forEach([&](uint64_t x) { elements.push_back(x); });

  EXPECT_EQ(4, elements.size());
  EXPECT_EQ(10, elements[0]);
  EXPECT_EQ(300, elements[1]);
  EXPECT_EQ(600, elements[2]);
  EXPECT_EQ(1000, elements[3]);
}

TEST(SparseBitsetTest, ChunkBoundaries) {
  SparseBitset<256> bitset;
  // Test elements at chunk boundaries
  bitset.add(0);     // First element of chunk 0
  bitset.add(255);   // Last element of chunk 0
  bitset.add(256);   // First element of chunk 1
  bitset.add(511);   // Last element of chunk 1

  EXPECT_EQ(4, bitset.size());
  EXPECT_TRUE(bitset.contains(0));
  EXPECT_TRUE(bitset.contains(255));
  EXPECT_TRUE(bitset.contains(256));
  EXPECT_TRUE(bitset.contains(511));
  EXPECT_FALSE(bitset.contains(1));
  EXPECT_FALSE(bitset.contains(254));
  EXPECT_FALSE(bitset.contains(257));
}

TEST(SparseBitsetTest, LargeIndices) {
  SparseBitset<> bitset;
  uint64_t large1 = 1ULL << 32;  // 4 billion
  uint64_t large2 = 1ULL << 40;  // 1 trillion

  bitset.add(large1);
  bitset.add(large2);

  EXPECT_EQ(2, bitset.size());
  EXPECT_TRUE(bitset.contains(large1));
  EXPECT_TRUE(bitset.contains(large2));
  EXPECT_FALSE(bitset.contains(large1 - 1));
  EXPECT_FALSE(bitset.contains(large1 + 1));
}

TEST(SparseBitsetTest, StressTestManyElements) {
  SparseBitset<> bitset;
  const int count = 10000;

  // Add many elements
  for (int i = 0; i < count; ++i) {
    bitset.add(i * 100);
  }

  EXPECT_EQ(count, bitset.size());

  // Verify all are present
  for (int i = 0; i < count; ++i) {
    EXPECT_TRUE(bitset.contains(i * 100));
  }

  // Verify forEach visits all elements in order
  int visited = 0;
  bitset.forEach([&](uint64_t x) {
    EXPECT_EQ(visited * 100, x);
    ++visited;
  });
  EXPECT_EQ(count, visited);
}

}  // namespace HPHP
