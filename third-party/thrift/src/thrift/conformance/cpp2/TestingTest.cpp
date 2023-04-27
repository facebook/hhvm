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

#include <thrift/conformance/cpp2/Testing.h>

#include <type_traits>

#include <folly/portability/GTest.h>

namespace apache::thrift::conformance::test {
namespace {

TEST(TestingTest, CtorTypeV) {
  EXPECT_EQ((ctor_type_v<CtorType::Delete, CtorType::Throw>), CtorType::Delete);
  EXPECT_EQ(
      (ctor_type_v<CtorType::Delete, CtorType::NoThrow>), CtorType::Delete);
  EXPECT_EQ(
      (ctor_type_v<CtorType::Delete, CtorType::Trivial>), CtorType::Delete);

  EXPECT_EQ((ctor_type_v<CtorType::Throw, CtorType::Throw>), CtorType::Throw);
  EXPECT_EQ((ctor_type_v<CtorType::Throw, CtorType::NoThrow>), CtorType::Throw);
  EXPECT_EQ((ctor_type_v<CtorType::Throw, CtorType::Trivial>), CtorType::Throw);

  EXPECT_EQ((ctor_type_v<CtorType::NoThrow, CtorType::Throw>), CtorType::Throw);
  EXPECT_EQ(
      (ctor_type_v<CtorType::NoThrow, CtorType::NoThrow>), CtorType::NoThrow);
  EXPECT_EQ(
      (ctor_type_v<CtorType::NoThrow, CtorType::Trivial>), CtorType::NoThrow);

  EXPECT_EQ((ctor_type_v<CtorType::Trivial, CtorType::Throw>), CtorType::Throw);
  EXPECT_EQ(
      (ctor_type_v<CtorType::Trivial, CtorType::NoThrow>), CtorType::NoThrow);
  EXPECT_EQ(
      (ctor_type_v<CtorType::Trivial, CtorType::Trivial>), CtorType::Trivial);
}

TEST(TestingTest, Presets) {
  staticAssertCtors<
      Copyable,
      /*default=*/CtorType::Trivial,
      /*copy=*/CtorType::Throw,
      /*move=*/CtorType::Delete,
      /*dtor=*/CtorType::Trivial>();

  staticAssertCtors<
      Moveable,
      /*default=*/CtorType::Trivial,
      /*copy=*/CtorType::Delete,
      /*move=*/CtorType::NoThrow,
      /*dtor=*/CtorType::Trivial>();

  staticAssertCtors<
      MoveableThrow,
      /*default=*/CtorType::Trivial,
      /*copy=*/CtorType::Delete,
      /*move=*/CtorType::Throw,
      /*dtor=*/CtorType::Trivial>();

  staticAssertCtors<
      NonMoveable,
      /*default=*/CtorType::Trivial,
      /*copy=*/CtorType::Delete,
      /*move=*/CtorType::Delete,
      /*dtor=*/CtorType::Trivial>();

  staticAssertCtors<
      NoDefault,
      /*default=*/CtorType::Delete,
      /*copy=*/CtorType::Trivial,
      /*move=*/CtorType::Trivial,
      /*dtor=*/CtorType::Trivial>();

  staticAssertAssignable<
      DestructorThrow,
      /*copy=*/CtorType::Trivial,
      /*move=*/CtorType::Trivial>();
  staticAssertConstructible<
      DestructorThrow,
      /*default=*/CtorType::Throw,
      /*copy=*/CtorType::Throw,
      /*move=*/CtorType::Throw,
      /*dtor=*/CtorType::Throw>();
}

} // namespace
} // namespace apache::thrift::conformance::test
