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

#include <thread>

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/FieldRef.h>

namespace apache::thrift::detail::test {

template <class T>
BitSet<T> makeBitSet([[maybe_unused]] T& storage) {
  return {};
}

template <class>
struct IntTest : ::testing::Test {};

using Ints = ::testing::Types<uint8_t, std::atomic<uint8_t>>;
TYPED_TEST_SUITE(IntTest, Ints);

TYPED_TEST(IntTest, Traits) {
  static_assert(std::is_copy_constructible_v<BitSet<TypeParam>>);
  static_assert(std::is_nothrow_move_constructible_v<BitSet<TypeParam>>);
  static_assert(std::is_trivially_destructible_v<BitSet<TypeParam>>);
  // `BitSet<TypeParam>` will be data member in thrift struct. We
  // need to make sure thrift struct is copy/move assignable.
  static_assert(std::is_copy_assignable_v<BitSet<TypeParam>>);
  static_assert(std::is_move_assignable_v<BitSet<TypeParam>>);
}

TYPED_TEST(IntTest, Basic) {
  TypeParam storage{0};
  auto b = makeBitSet<TypeParam>(storage);
  for (int i = 0; i < 8; i++) {
    b[i] = i % 3;
  }
  EXPECT_FALSE(b[0]);
  EXPECT_TRUE(b[1]);
  EXPECT_TRUE(b[2]);
  EXPECT_FALSE(b[3]);
  EXPECT_TRUE(b[4]);
  EXPECT_TRUE(b[5]);
  EXPECT_FALSE(b[6]);
  EXPECT_TRUE(b[7]);
}

template <class>
struct AtomicIntTest : ::testing::Test {};

using AtomicInts = ::testing::Types<std::atomic<uint8_t>>;
TYPED_TEST_SUITE(AtomicIntTest, AtomicInts);

TYPED_TEST(AtomicIntTest, Basic) {
  TypeParam storage{0};
  auto b = makeBitSet<TypeParam>(storage);
  std::thread t[8];
  for (int i = 0; i < 8; i++) {
    t[i] = std::thread([&b, i] { b[i] = i % 3; });
  }
  for (auto& i : t) {
    i.join();
  }
  EXPECT_FALSE(b[0]);
  EXPECT_TRUE(b[1]);
  EXPECT_TRUE(b[2]);
  EXPECT_FALSE(b[3]);
  EXPECT_TRUE(b[4]);
  EXPECT_TRUE(b[5]);
  EXPECT_FALSE(b[6]);
  EXPECT_TRUE(b[7]);
}

template <class>
struct BitRefTest : ::testing::Test {};

using BitRefs = ::testing::Types<BitRef<true>, BitRef<false>>;
TYPED_TEST_SUITE(BitRefTest, BitRefs);

TYPED_TEST(BitRefTest, Traits) {
  EXPECT_TRUE(std::is_trivially_copy_constructible_v<TypeParam>);
  EXPECT_TRUE(std::is_trivially_move_constructible_v<TypeParam>);
  EXPECT_TRUE(std::is_trivially_destructible_v<TypeParam>);
}

TYPED_TEST(BitRefTest, Get) {
  uint8_t storage = 0b1001001;
  for (int i = 0; i < 8; i++) {
    TypeParam b(storage, i);
    EXPECT_EQ(bool(b), i % 3 == 0);
  }
}

TYPED_TEST(BitRefTest, AtomicGet) {
  std::atomic<uint8_t> storage{0b1001001};
  for (int i = 0; i < 8; i++) {
    TypeParam b(storage, i);
    EXPECT_EQ(bool(b), i % 3 == 0);
  }
}

TEST(BitRefTest, Set) {
  uint8_t storage = 0;
  for (int i = 0; i < 8; i++) {
    BitRef<false> b(storage, i);
    b = i % 3 == 0;
  }
  EXPECT_EQ(storage, 0b1001001);
}

TEST(BitRefTest, AtomicSet) {
  std::atomic<uint8_t> storage{0};
  std::thread a[8];
  for (int i = 0; i < 8; i++) {
    a[i] = std::thread([&storage, i] {
      BitRef<false> b(storage, i);
      b = i % 3 == 0;
    });
  }
  for (auto&& t : a) {
    t.join();
  }
  EXPECT_EQ(storage, 0b1001001);
}

} // namespace apache::thrift::detail::test
