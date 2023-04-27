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
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/gen-cpp2/inject_metadata_fields_types.h>

namespace apache::thrift::test {

template <typename ThriftStruct, typename EnumType, typename StructType>
void test_serialize_and_deserialize() {
  ThriftStruct obj;
  StructType s;
  s.int_field() = 1;

  obj.bool_field() = true;
  obj.byte_field() = 1;
  obj.short_field() = 2;
  obj.int_field() = 3;
  obj.long_field() = 4;
  obj.float_field() = 5;
  obj.double_field() = 6;
  obj.string_field() = "7";
  obj.binary_field() = "8";
  obj.enum_field() = EnumType::ME1;
  obj.list_field() = {1};
  obj.set_field() = {1};
  obj.map_field() = {{1, 1}};
  obj.struct_field() = s;

  EXPECT_EQ(obj.bool_field(), true);
  EXPECT_EQ(obj.byte_field(), 1);
  EXPECT_EQ(obj.short_field(), 2);
  EXPECT_EQ(obj.int_field(), 3);
  EXPECT_EQ(obj.long_field(), 4);
  EXPECT_EQ(obj.float_field(), 5);
  EXPECT_EQ(obj.double_field(), 6);
  EXPECT_EQ(obj.string_field(), "7");
  EXPECT_EQ(obj.binary_field(), "8");
  EXPECT_EQ(obj.enum_field(), EnumType::ME1);
  EXPECT_FALSE(obj.list_field()->empty());
  EXPECT_FALSE(obj.set_field()->empty());
  EXPECT_FALSE(obj.map_field()->empty());
  EXPECT_EQ(obj.struct_field(), s);

  auto objs = CompactSerializer::serialize<std::string>(obj);
  ThriftStruct objd;
  CompactSerializer::deserialize(objs, objd);

  EXPECT_EQ(objd.bool_field(), true);
  EXPECT_EQ(objd.byte_field(), 1);
  EXPECT_EQ(objd.short_field(), 2);
  EXPECT_EQ(objd.int_field(), 3);
  EXPECT_EQ(objd.long_field(), 4);
  EXPECT_EQ(objd.float_field(), 5);
  EXPECT_EQ(objd.double_field(), 6);
  EXPECT_EQ(objd.string_field(), "7");
  EXPECT_EQ(objd.binary_field(), "8");
  EXPECT_EQ(objd.enum_field(), EnumType::ME1);
  EXPECT_FALSE(objd.list_field()->empty());
  EXPECT_FALSE(objd.set_field()->empty());
  EXPECT_FALSE(objd.map_field()->empty());
  EXPECT_EQ(objd.struct_field(), s);
}

TEST(InjectMetadataFields, SameProgram) {
  test_serialize_and_deserialize<
      inject_metadata_fields::InjectedEmptyStruct1,
      inject_metadata_fields::MyEnum,
      inject_metadata_fields::MyStruct>();
}

TEST(InjectMetadataFields, DifferentProgram) {
  test_serialize_and_deserialize<
      inject_metadata_fields::InjectedEmptyStruct2,
      MyEnum,
      MyStruct>();
}

TEST(InjectMetadataFields, FieldsWithAnnotation1) {
  inject_metadata_fields::InjectedEmptyStruct3 obj;

  obj.structured_boxed_field() = 1;
  obj.unstructured_boxed_field() = 2;

  auto objs = CompactSerializer::serialize<std::string>(obj);
  inject_metadata_fields::InjectedEmptyStruct3 objd;
  CompactSerializer::deserialize(objs, objd);

  objd.structured_boxed_field() = 1;
  objd.unstructured_boxed_field() = 2;
}

TEST(InjectMetadataFields, FieldsWithAnnotation2) {
  inject_metadata_fields::InjectedEmptyStruct4 obj;

  obj.structured_boxed_field() = 1;
  obj.unstructured_boxed_field() = 2;

  auto objs = CompactSerializer::serialize<std::string>(obj);
  inject_metadata_fields::InjectedEmptyStruct4 objd;
  CompactSerializer::deserialize(objs, objd);

  objd.structured_boxed_field() = 1;
  objd.unstructured_boxed_field() = 2;
}

} // namespace apache::thrift::test
