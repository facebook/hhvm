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

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/BoxedValuePtr.h>

using namespace apache::thrift::detail;

TEST(BoxedValuePtrTest, DefaultConstructor) {
  boxed_value_ptr<int> a;
  EXPECT_FALSE(static_cast<bool>(a));
}

TEST(BoxedValuePtrTest, Constructor) {
  boxed_value_ptr<int> a(5);
  EXPECT_EQ(*a, 5);
}

TEST(BoxedValuePtrTest, CopyConstructor) {
  boxed_value_ptr<int> a(5);
  boxed_value_ptr<int> b(a);
  EXPECT_EQ(*b, 5);
  EXPECT_EQ(*a, 5);
}

TEST(BoxedValuePtrTest, CopyAssignment) {
  boxed_value_ptr<int> a(5);
  boxed_value_ptr<int> b;
  b = a;
  EXPECT_EQ(*b, 5);
  EXPECT_EQ(*a, 5);
}

TEST(BoxedValuePtrTest, MoveConstructor) {
  boxed_value_ptr<int> a(5);
  boxed_value_ptr<int> b(std::move(a));
  EXPECT_EQ(*b, 5);
}

TEST(BoxedValuePtrTest, MoveAssignment) {
  boxed_value_ptr<int> a(5);
  boxed_value_ptr<int> b;
  b = std::move(a);
  EXPECT_EQ(*b, 5);
}

TEST(BoxedValuePtrTest, EmptyAssignment) {
  boxed_value_ptr<int> a;
  boxed_value_ptr<int> b(5);
  EXPECT_EQ(*b, 5);
  b = a;
  EXPECT_FALSE(static_cast<bool>(b));
}

TEST(BoxedValuePtrTest, Emplace) {
  boxed_value_ptr<int> a;
  a.emplace(5);
  EXPECT_EQ(*a, 5);
  a.emplace(7);
  EXPECT_EQ(*a, 7);
}

TEST(LazyPtrRest, Reset) {
  boxed_value_ptr<int> a(6);
  a.reset();
  EXPECT_FALSE(static_cast<bool>(a));
}

TEST(BoxedValuePtrTest, Assignment) {
  boxed_value_ptr<int> a;
  a = 6;
  EXPECT_EQ(*a, 6);
}

TEST(BoxedValuePtrTest, MoveOnlyType) {
  boxed_value_ptr<std::unique_ptr<int>> a;
  a = std::make_unique<int>(5);
  EXPECT_EQ(**a, 5);
  boxed_value_ptr<std::unique_ptr<int>> b(std::move(a));
  EXPECT_EQ(**b, 5);
}

TEST(BoxedValuePtrTest, Swap) {
  boxed_value_ptr<int> a(5);
  boxed_value_ptr<int> b(7);
  std::swap(a, b);
  EXPECT_EQ(*a, 7);
  EXPECT_EQ(*b, 5);
}

TEST(BoxedValuePtrTest, Equal) {
  boxed_value_ptr<int> a;
  boxed_value_ptr<int> b;
  EXPECT_TRUE(a == b);
  a = 5;
  EXPECT_FALSE(a == b);
  b = 5;
  EXPECT_FALSE(a == b);
  b = 7;
  EXPECT_FALSE(a == b);
}
