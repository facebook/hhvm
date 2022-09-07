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

#include <thrift/lib/cpp2/detail/BoxedIfLarge.h>

#include <folly/portability/GTest.h>

namespace apache::thrift::detail {

template <std::size_t N, std::size_t M>
struct TestStruct {
  char a;
  BoxedIfLarge<std::array<char, N>, M> b;
  char c;
};

TEST(BoxedIfLarge, TestAllocatedOnStack) {
  TestStruct<10, 10> f;
  EXPECT_LT(static_cast<void*>(&f.a), static_cast<void*>(&*f.b));
  EXPECT_GT(static_cast<void*>(&f.c), static_cast<void*>(&*f.b));
  EXPECT_EQ(sizeof(f.b), 10);
  (*f.b)[0] = 42;
}

TEST(BoxedIfLarge, TestAllocatedOnHeap) {
  // We allocate large struct on heap
  TestStruct<11, 10> f;
  EXPECT_TRUE(
      (static_cast<void*>(&f.a) > static_cast<void*>(&*f.b)) ||
      (static_cast<void*>(&f.c) < static_cast<void*>(&*f.b)));
  EXPECT_EQ(sizeof(f.b), sizeof(std::remove_reference_t<decltype(f.b)>*));
  (*f.b)[0] = 42;
}

} // namespace apache::thrift::detail
