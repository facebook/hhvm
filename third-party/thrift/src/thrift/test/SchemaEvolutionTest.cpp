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
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/gen-cpp2/schema_evolution_test_for_each_field.h>

using namespace apache::thrift;

template <class T>
struct EvolutionTest : testing::Test {
  using Serializer = T;
};

using Serializers =
    ::testing::Types<BinarySerializer, CompactSerializer, SimpleJSONSerializer>;

TYPED_TEST_CASE(EvolutionTest, Serializers);

TYPED_TEST(EvolutionTest, evolution) {
  using Serializer = typename TestFixture::Serializer;
  cpp2::Old oldObj;
  for_each_field(oldObj, [](auto& meta, auto ref) { ref = *meta.name_ref(); });

  cpp2::New newObj;
  Serializer::deserialize(
      Serializer::template serialize<std::string>(oldObj), newObj);

  EXPECT_EQ(
      newObj.unqualified_to_unqualified(),
      *oldObj.unqualified_to_unqualified());
  EXPECT_EQ(
      newObj.unqualified_to_optional(), *oldObj.unqualified_to_optional());
  EXPECT_EQ(
      newObj.unqualified_to_required(), *oldObj.unqualified_to_required());

  EXPECT_EQ(
      newObj.optional_to_unqualified(), *oldObj.optional_to_unqualified());
  EXPECT_EQ(newObj.optional_to_optional(), *oldObj.optional_to_optional());
  EXPECT_EQ(newObj.optional_to_required(), *oldObj.optional_to_required());

  EXPECT_EQ(
      newObj.required_to_unqualified(), *oldObj.required_to_unqualified());
  EXPECT_EQ(newObj.required_to_optional(), *oldObj.required_to_optional());
  EXPECT_EQ(newObj.required_to_required(), *oldObj.required_to_required());

  if (std::is_same_v<Serializer, SimpleJSONSerializer>) {
    EXPECT_EQ(newObj.unqualified_new(), "");
    EXPECT_EQ(newObj.required_new(), "");

    EXPECT_FALSE(newObj.unqualified_new().is_set());
    EXPECT_FALSE(newObj.optional_new().has_value());
    EXPECT_TRUE(newObj.required_new().has_value());
  } else {
    EXPECT_EQ(newObj.unqualified_new(), *oldObj.unqualified_old());
    EXPECT_EQ(newObj.optional_new(), *oldObj.optional_old());
    EXPECT_EQ(newObj.required_new(), *oldObj.required_old());
  }

  EXPECT_EQ(newObj.unqualified_added(), "");
  EXPECT_EQ(newObj.required_added(), "");

  EXPECT_FALSE(newObj.unqualified_added().is_set());
  EXPECT_FALSE(newObj.optional_added().has_value());
  EXPECT_TRUE(newObj.required_added().has_value());
}

TYPED_TEST(EvolutionTest, union_evolution) {
  using Serializer = typename TestFixture::Serializer;

  cpp2::MyUnion_V2 myUnion_v2;
  myUnion_v2.set_string_field("string_field");

  cpp2::MyUnion_V1 myUnion_v1;
  Serializer::deserialize(
      Serializer::template serialize<std::string>(myUnion_v2), myUnion_v1);

  // We should be able to deserialize the v2 union to v1 union, even if the v2
  // union has a new field that is not present in the v1 union. In this case,
  // we expect the deserialized v1 union to be empty.
  EXPECT_EQ(cpp2::MyUnion_V1::Type::__EMPTY__, myUnion_v1.getType());
}

TYPED_TEST(EvolutionTest, struct_union_evolution) {
  using Serializer = typename TestFixture::Serializer;

  cpp2::MyStruct_V2 myStruct_v2;
  myStruct_v2.i32_field() = 11;
  myStruct_v2.union_field()->set_string_field("string_field");

  cpp2::MyStruct_V1 myStruct_v1;
  Serializer::deserialize(
      Serializer::template serialize<std::string>(myStruct_v2), myStruct_v1);

  EXPECT_EQ(11, myStruct_v1.i32_field());
  // We should be able to deserialize the v2 struct to v1 struct, even if the
  // v2 union has a new field that is not present in the v1 union. In this case,
  // we expect the deserialized v1 struct to have an empty v1 union.
  EXPECT_EQ(
      cpp2::MyUnion_V1::Type::__EMPTY__, myStruct_v1.union_field()->getType());
}
