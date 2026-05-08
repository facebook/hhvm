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

#include <algorithm>
#include <random>
#include <vector>

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/frozen/Frozen.h>

using namespace apache::thrift::frozen;

namespace {

std::vector<bool> makeRandomBoolVector(
    size_t size, uint32_t seed, double trueRate) {
  std::mt19937 rng(seed);
  std::bernoulli_distribution dist(trueRate);
  std::vector<bool> v(size);
  for (size_t i = 0; i < size; ++i) {
    v[i] = dist(rng);
  }
  return v;
}

// Verifies a frozen list<bool> matches the original vector element-by-element
// and that popcount() matches std::count(true).
void verifyFrozenBoolList(const std::vector<bool>& original) {
  auto frozen = freeze(original);
  ASSERT_EQ(frozen.size(), original.size());

  for (size_t i = 0; i < original.size(); ++i) {
    EXPECT_EQ(frozen[i], original[i]) << "Mismatch at index " << i;
  }

  size_t expected =
      static_cast<size_t>(std::count(original.begin(), original.end(), true));
  EXPECT_EQ(frozen.popcount(), expected);
}

} // namespace

TEST(FrozenBoolList, TotalPopcount) {
  constexpr size_t kSize = 10000;
  auto original = makeRandomBoolVector(kSize, /*seed=*/42, /*trueRate=*/0.3);
  verifyFrozenBoolList(original);
}

TEST(FrozenBoolList, PrefixPopcount) {
  constexpr size_t kSize = 10000;
  auto original = makeRandomBoolVector(kSize, /*seed=*/42, /*trueRate=*/0.3);

  auto frozen = freeze(original);

  // Test prefix popcount at positions exercising:
  // - bit offset 0 (empty prefix)
  // - single element
  // - within-first-byte boundaries (7, 8, 9)
  // - 64-bit word boundaries (63, 64, 65)
  // - larger ranges (100, 500, 1000, 5000)
  // - last element and full list
  std::vector<size_t> positions = {
      0, 1, 7, 8, 9, 63, 64, 65, 100, 500, 1000, 5000, 9999, kSize};

  for (size_t j : positions) {
    size_t bruteForce = static_cast<size_t>(
        std::count(original.begin(), original.begin() + j, true));
    EXPECT_EQ(frozen.popcount(j), bruteForce)
        << "Prefix popcount mismatch at position " << j;
  }
}

TEST(FrozenBoolList, BoundaryEmpty) {
  verifyFrozenBoolList({});
}

TEST(FrozenBoolList, BoundarySingleTrue) {
  verifyFrozenBoolList({true});
}

TEST(FrozenBoolList, BoundarySingleFalse) {
  // Single false: zero-bit optimization may activate (bits=0, 0 data bytes).
  // popcount() must still return 0 correctly.
  std::vector<bool> v{false};
  auto frozen = freeze(v);
  ASSERT_EQ(frozen.size(), 1);
  EXPECT_EQ(frozen[0], false);
  EXPECT_EQ(frozen.popcount(), 0);
}

TEST(FrozenBoolList, BoundaryAllTrue) {
  constexpr size_t kSize = 200;
  std::vector<bool> allTrue(kSize, true);
  verifyFrozenBoolList(allTrue);
}

TEST(FrozenBoolList, BoundaryAllFalse) {
  constexpr size_t kSize = 200;
  std::vector<bool> allFalse(kSize, false);
  auto frozen = freeze(allFalse);

  EXPECT_EQ(frozen.size(), kSize);
  // Frozen2's zero-bit optimization: all-false lists allocate 0 data bytes.
  // popcount() must handle this without dereferencing a null data pointer.
  EXPECT_EQ(frozen.popcount(), 0);
}

TEST(FrozenBoolList, Boundary7Elements) {
  // Sub-byte: 0 full bytes, 7 remaining bits in the partial byte.
  std::vector<bool> v = {true, false, true, true, false, true, false};
  verifyFrozenBoolList(v);
}

TEST(FrozenBoolList, Boundary8Elements) {
  // Exactly 1 full byte, no partial byte.
  std::vector<bool> v = {true, false, true, true, false, true, false, true};
  verifyFrozenBoolList(v);
}

TEST(FrozenBoolList, Boundary9Elements) {
  // 1 full byte + 1 bit in a partial second byte.
  std::vector<bool> v = {
      true, false, true, true, false, true, false, true, true};
  verifyFrozenBoolList(v);
}

TEST(FrozenBoolList, Boundary64Elements) {
  // Exactly 8 full bytes (64 bits), no partial byte.
  auto v = makeRandomBoolVector(64, /*seed=*/99, /*trueRate=*/0.5);
  verifyFrozenBoolList(v);
}
