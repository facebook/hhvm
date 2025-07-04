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

#include <gtest/gtest.h>
#include <thrift/test/gen-cpp2/FieldRefConstPropagation_types.h>

// Similar to std::as_const but don't delete rvalue reference overload
namespace apache::thrift::test {

// The same function is repeated 6 times in this unit-test.
// The reason is because we want to make sure linter works, thus we can't merge
// them into a single template function

TEST(FieldRefConstPropagation, Unqualified) {
  Unqualified foo;
  foo.msg() = "123";

  // both field_ref and reference are not const
  auto msg_ref = foo.msg();

  // reference is const, but field_ref is not
  const auto cmsg_ref = foo.msg();

  // field_ref is not const, but reference is const
  auto msg_cref = std::as_const(foo).msg();

  // both field_ref and reference are const
  const auto cmsg_cref = std::as_const(foo).msg();

  auto& c1 = msg_ref->at(0);
  auto& c2 = cmsg_ref->at(0); // Only for this case we should have lint warning
  auto& c3 = msg_cref->at(0);
  auto& c4 = cmsg_cref->at(0);

  EXPECT_EQ(c1, '1');
  EXPECT_EQ(c2, '1');
  EXPECT_EQ(c3, '1');
  EXPECT_EQ(c4, '1');

  static_assert(!std::is_const_v<std::remove_reference_t<decltype(c1)>>);
  static_assert(!std::is_const_v<std::remove_reference_t<decltype(c2)>>);
  static_assert(std::is_const_v<std::remove_reference_t<decltype(c3)>>);
  static_assert(std::is_const_v<std::remove_reference_t<decltype(c4)>>);
}

TEST(FieldRefConstPropagation, Optional) {
  Optional foo;
  foo.msg() = "123";

  // both field_ref and reference are not const
  auto msg_ref = foo.msg();

  // reference is const, but field_ref is not
  const auto cmsg_ref = foo.msg();

  // field_ref is not const, but reference is const
  auto msg_cref = std::as_const(foo).msg();

  // both field_ref and reference are const
  const auto cmsg_cref = std::as_const(foo).msg();

  auto& c1 = msg_ref->at(0);
  auto& c2 = cmsg_ref->at(0); // Only for this case we should have lint warning
  auto& c3 = msg_cref->at(0);
  auto& c4 = cmsg_cref->at(0);

  EXPECT_EQ(c1, '1');
  EXPECT_EQ(c2, '1');
  EXPECT_EQ(c3, '1');
  EXPECT_EQ(c4, '1');

  static_assert(!std::is_const_v<std::remove_reference_t<decltype(c1)>>);
  static_assert(!std::is_const_v<std::remove_reference_t<decltype(c2)>>);
  static_assert(std::is_const_v<std::remove_reference_t<decltype(c3)>>);
  static_assert(std::is_const_v<std::remove_reference_t<decltype(c4)>>);
}

TEST(FieldRefConstPropagation, Required) {
  Required foo;
  foo.msg() = "123";

  // both field_ref and reference are not const
  auto msg_ref = foo.msg();

  // reference is const, but field_ref is not
  const auto cmsg_ref = foo.msg();

  // field_ref is not const, but reference is const
  auto msg_cref = std::as_const(foo).msg();

  // both field_ref and reference are const
  const auto cmsg_cref = std::as_const(foo).msg();

  auto& c1 = msg_ref->at(0);
  auto& c2 = cmsg_ref->at(0); // Only for this case we should have lint warning
  auto& c3 = msg_cref->at(0);
  auto& c4 = cmsg_cref->at(0);

  EXPECT_EQ(c1, '1');
  EXPECT_EQ(c2, '1');
  EXPECT_EQ(c3, '1');
  EXPECT_EQ(c4, '1');

  static_assert(!std::is_const_v<std::remove_reference_t<decltype(c1)>>);
  static_assert(!std::is_const_v<std::remove_reference_t<decltype(c2)>>);
  static_assert(std::is_const_v<std::remove_reference_t<decltype(c3)>>);
  static_assert(std::is_const_v<std::remove_reference_t<decltype(c4)>>);
}

TEST(FieldRefConstPropagation, Boxed) {
  Boxed foo;
  foo.msg() = "123";

  // both field_ref and reference are not const
  auto msg_ref = foo.msg();

  // reference is const, but field_ref is not
  const auto cmsg_ref = foo.msg();

  // field_ref is not const, but reference is const
  auto msg_cref = std::as_const(foo).msg();

  // both field_ref and reference are const
  const auto cmsg_cref = std::as_const(foo).msg();

  auto& c1 = msg_ref->at(0);
  auto& c2 = cmsg_ref->at(0); // Only for this case we should have lint warning
  auto& c3 = msg_cref->at(0);
  auto& c4 = cmsg_cref->at(0);

  EXPECT_EQ(c1, '1');
  EXPECT_EQ(c2, '1');
  EXPECT_EQ(c3, '1');
  EXPECT_EQ(c4, '1');

  static_assert(!std::is_const_v<std::remove_reference_t<decltype(c1)>>);
  static_assert(!std::is_const_v<std::remove_reference_t<decltype(c2)>>);
  static_assert(std::is_const_v<std::remove_reference_t<decltype(c3)>>);
  static_assert(std::is_const_v<std::remove_reference_t<decltype(c4)>>);
}

TEST(FieldRefConstPropagation, TerseWrite) {
  TerseWrite foo;
  foo.msg() = "123";

  // both field_ref and reference are not const
  auto msg_ref = foo.msg();

  // reference is const, but field_ref is not
  const auto cmsg_ref = foo.msg();

  // field_ref is not const, but reference is const
  auto msg_cref = std::as_const(foo).msg();

  // both field_ref and reference are const
  const auto cmsg_cref = std::as_const(foo).msg();

  auto& c1 = msg_ref->at(0);
  auto& c2 = cmsg_ref->at(0); // Only for this case we should have lint warning
  auto& c3 = msg_cref->at(0);
  auto& c4 = cmsg_cref->at(0);

  EXPECT_EQ(c1, '1');
  EXPECT_EQ(c2, '1');
  EXPECT_EQ(c3, '1');
  EXPECT_EQ(c4, '1');

  static_assert(!std::is_const_v<std::remove_reference_t<decltype(c1)>>);
  static_assert(!std::is_const_v<std::remove_reference_t<decltype(c2)>>);
  static_assert(std::is_const_v<std::remove_reference_t<decltype(c3)>>);
  static_assert(std::is_const_v<std::remove_reference_t<decltype(c4)>>);
}

TEST(FieldRefConstPropagation, Union) {
  Union foo;
  foo.msg() = "123";

  // both field_ref and reference are not const
  auto msg_ref = foo.msg();

  // reference is const, but field_ref is not
  const auto cmsg_ref = foo.msg();

  // field_ref is not const, but reference is const
  auto msg_cref = std::as_const(foo).msg();

  // both field_ref and reference are const
  const auto cmsg_cref = std::as_const(foo).msg();

  auto& c1 = msg_ref->at(0);
  auto& c2 = cmsg_ref->at(0); // Only for this case we should have lint warning
  auto& c3 = msg_cref->at(0);
  auto& c4 = cmsg_cref->at(0);

  EXPECT_EQ(c1, '1');
  EXPECT_EQ(c2, '1');
  EXPECT_EQ(c3, '1');
  EXPECT_EQ(c4, '1');

  static_assert(!std::is_const_v<std::remove_reference_t<decltype(c1)>>);
  static_assert(!std::is_const_v<std::remove_reference_t<decltype(c2)>>);
  static_assert(std::is_const_v<std::remove_reference_t<decltype(c3)>>);
  static_assert(std::is_const_v<std::remove_reference_t<decltype(c4)>>);
}
} // namespace apache::thrift::test
