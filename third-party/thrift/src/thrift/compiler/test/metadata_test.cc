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
#include <thrift/lib/cpp2/gen/module_metadata_h.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>

namespace {

template <typename T>
using StructMetadata = apache::thrift::detail::md::StructMetadata<T>;
using apache::thrift::metadata::ThriftEnum;
using apache::thrift::metadata::ThriftField;
using apache::thrift::metadata::ThriftMetadata;
using apache::thrift::metadata::ThriftPrimitiveType;
using apache::thrift::metadata::ThriftStruct;
using apache::thrift::metadata::ThriftType;

class MetadataCodegenTest : public testing::Test {
 protected:
  ThriftStruct getStruct(const std::string& name) {
    ThriftStruct s = metadata_.structs()->at(name);
    EXPECT_EQ(*s.name(), name);
    return s;
  }
  ThriftType getResolvedType(const ThriftType& type) {
    ThriftType ret = type;
    while (ret.getType() == ThriftType::Type::t_typedef) {
      auto underlyingType =
          std::move(ret.mutable_t_typedef().underlyingType_ref());
      EXPECT_NE(underlyingType.get(), nullptr);
      ret = std::move(*underlyingType);
    }
    return ret;
  }
  ThriftType checkField(
      const ThriftField& field, int key, const std::string& name) {
    EXPECT_EQ(*field.name(), name);
    EXPECT_EQ(*field.id(), key);
    return getResolvedType(*field.type());
  }
  void checkFieldUnion(
      const ThriftField& field,
      int key,
      const std::string& name,
      const std::string& typeName,
      bool optional = true) {
    auto type = checkField(field, key, name);
    EXPECT_EQ(*field.is_optional(), optional);
    EXPECT_EQ(*type.get_t_union().name(), typeName);
  }
  void checkFieldString(
      const ThriftField& field, int key, const std::string& name) {
    auto type = checkField(field, key, name);
    EXPECT_FALSE(*field.is_optional());
    EXPECT_EQ(type.get_t_primitive(), ThriftPrimitiveType::THRIFT_STRING_TYPE);
  }
  ThriftMetadata metadata_;
};

TEST_F(MetadataCodegenTest, structTest) {
  StructMetadata<ThriftField>::gen(metadata_);

  EXPECT_EQ(metadata_.enums()->size(), 1);
  std::string enumName{"metadata.ThriftPrimitiveType"};
  ThriftEnum e = metadata_.enums()->at(enumName);
  EXPECT_EQ(*e.name(), enumName);
  EXPECT_EQ(e.elements()->size(), 10);
  EXPECT_EQ(e.elements()->at(1), "THRIFT_BOOL_TYPE");
  EXPECT_EQ(e.elements()->at(2), "THRIFT_BYTE_TYPE");
  EXPECT_EQ(e.elements()->at(3), "THRIFT_I16_TYPE");
  EXPECT_EQ(e.elements()->at(4), "THRIFT_I32_TYPE");
  EXPECT_EQ(e.elements()->at(5), "THRIFT_I64_TYPE");
  EXPECT_EQ(e.elements()->at(6), "THRIFT_FLOAT_TYPE");
  EXPECT_EQ(e.elements()->at(8), "THRIFT_BINARY_TYPE");
  EXPECT_EQ(e.elements()->at(9), "THRIFT_STRING_TYPE");
  EXPECT_EQ(e.elements()->at(10), "THRIFT_VOID_TYPE");

  ThriftStruct s;
  s = getStruct("metadata.ThriftListType");
  EXPECT_EQ(s.fields()->size(), 1);
  checkFieldUnion(s.fields()[0], 1, "valueType", "metadata.ThriftType");

  s = getStruct("metadata.ThriftMapType");
  EXPECT_EQ(s.fields()->size(), 2);
  checkFieldUnion(s.fields()[0], 1, "keyType", "metadata.ThriftType");
  checkFieldUnion(s.fields()[1], 2, "valueType", "metadata.ThriftType");

  s = getStruct("metadata.ThriftEnumType");
  EXPECT_EQ(s.fields()->size(), 1);
  checkFieldString(s.fields()[0], 1, "name");

  s = getStruct("metadata.ThriftTypedefType");
  EXPECT_EQ(s.fields()->size(), 3);
  checkFieldString(s.fields()[0], 1, "name");
  checkFieldUnion(s.fields()[1], 2, "underlyingType", "metadata.ThriftType");

  auto td = s.fields()[2];
  EXPECT_EQ(td.name(), "structured_annotations");
  EXPECT_EQ(td.type()->getType(), ThriftType::Type::t_list);
  EXPECT_EQ(
      td.type()->t_list_ref()->valueType_ref()->getType(),
      ThriftType::Type::t_struct);
  EXPECT_EQ(
      td.type()->t_list_ref()->valueType_ref()->t_struct_ref()->name(),
      "metadata.ThriftConstStruct");
}
} // namespace
