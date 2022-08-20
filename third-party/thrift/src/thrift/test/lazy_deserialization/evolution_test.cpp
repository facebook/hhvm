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

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/lazy_deserialization/MemberAccessor.h>
#include <thrift/test/lazy_deserialization/gen-cpp2/evolution_types.h>

namespace apache::thrift::test {

FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field1, StructWithLessFields, field1);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field2, StructWithLessFields, field2);

FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field1, StructWithMoreFields, field1);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field2, StructWithMoreFields, field2);
FBTHRIFT_DEFINE_MEMBER_ACCESSOR(get_field3, StructWithMoreFields, field3);

using Serializers = ::testing::Types<BinarySerializer, CompactSerializer>;

template <typename T>
struct Evolution : testing::Test {};

TYPED_TEST_SUITE(Evolution, Serializers);

TYPED_TEST(Evolution, AddFields) {
  using Serializer = TypeParam;
  StructWithLessFields lessFields;
  lessFields.field1_ref() = {{"field1"}, {"foo"}};
  lessFields.field2_ref() = {{"field2"}, {"bar"}};
  auto moreFields = Serializer::template deserialize<StructWithMoreFields>(
      Serializer::template serialize<std::string>(lessFields));

  EXPECT_TRUE(get_field1(moreFields).empty());
  EXPECT_FALSE(get_field2(moreFields).empty());
  EXPECT_TRUE(get_field3(moreFields).empty());

  EXPECT_EQ(moreFields.field1_ref(), lessFields.field1_ref());
  EXPECT_EQ(moreFields.field2_ref(), lessFields.field2_ref());
  EXPECT_TRUE(moreFields.field3_ref()->empty());
}

TYPED_TEST(Evolution, RemoveFields) {
  using Serializer = TypeParam;
  StructWithMoreFields moreFields;
  moreFields.field1_ref() = {{"field1"}, {"foo"}};
  moreFields.field2_ref() = {{"field2"}, {"bar"}};
  moreFields.field3_ref() = {{"field3"}, {"baz"}};
  auto lessFields = Serializer::template deserialize<StructWithLessFields>(
      Serializer::template serialize<std::string>(moreFields));

  EXPECT_TRUE(get_field1(lessFields).empty());
  EXPECT_FALSE(get_field2(lessFields).empty());

  EXPECT_EQ(moreFields.field1_ref(), lessFields.field1_ref());
  EXPECT_EQ(moreFields.field2_ref(), lessFields.field2_ref());
}

} // namespace apache::thrift::test
