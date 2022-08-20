/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <type_traits>

#include <folly/portability/GTest.h>

#include <thrift/test/gen-cpp2/NoExcept_types.h>

using namespace apache::thrift::test;

TEST(NoExcept, noexcept) {
  EXPECT_TRUE(std::is_nothrow_move_constructible<Simple>::value);
  EXPECT_TRUE(std::is_nothrow_move_assignable<Simple>::value);

  EXPECT_TRUE(std::is_nothrow_move_constructible<SimpleWithString>::value);
  EXPECT_TRUE(std::is_nothrow_move_assignable<SimpleWithString>::value);

  EXPECT_TRUE(std::is_nothrow_move_constructible<List>::value);
  EXPECT_TRUE(std::is_nothrow_move_assignable<List>::value);

  EXPECT_TRUE(std::is_nothrow_move_constructible<Set>::value);
  EXPECT_TRUE(std::is_nothrow_move_assignable<Set>::value);

  EXPECT_TRUE(std::is_nothrow_move_constructible<Map>::value);
  EXPECT_TRUE(std::is_nothrow_move_assignable<Map>::value);

  EXPECT_TRUE(std::is_nothrow_move_constructible<Complex>::value);
  EXPECT_TRUE(std::is_nothrow_move_assignable<Complex>::value);

  EXPECT_TRUE(
      std::is_nothrow_move_constructible<ComplexWithStringAndMap>::value);
  EXPECT_TRUE(std::is_nothrow_move_assignable<ComplexWithStringAndMap>::value);
}
