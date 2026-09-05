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

#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>
#include <thrift/lib/cpp2/dynamic/ValueConversion.h>
#include <thrift/lib/cpp2/dynamic/test/gen-cpp2/service_descriptor_test_types.h>

#include <gtest/gtest.h>

#include <memory>

namespace apache::thrift::dynamic {
namespace {

using def = type_system::TypeSystemBuilder::DefinitionHelper;

TEST(ValueConversionTest, ConvertsNumericValueWithMatchingType) {
  const auto value =
      toDynamicValue(int64_t{42}, type_system::TypeSystem::I64());

  EXPECT_TRUE(value.type().isI64());
  EXPECT_EQ(value.asI64(), 42);
  EXPECT_EQ(fromDynamicValue<int64_t>(value), 42);
}

TEST(ValueConversionTest, RejectsMismatchedNumericType) {
  EXPECT_THROW(
      (void)toDynamicValue(int32_t{42}, type_system::TypeSystem::I64()),
      std::invalid_argument);
  EXPECT_THROW(
      (void)toDynamicValue(int64_t{42}, type_system::TypeSystem::I32()),
      std::invalid_argument);
}

TEST(ValueConversionTest, ConvertsStringToBinary) {
  const auto value = toDynamicValue("hello", type_system::TypeSystem::Binary());

  EXPECT_TRUE(value.type().isBinary());
  EXPECT_EQ(fromDynamicValue<std::string>(value), "hello");
}

TEST(ValueConversionTest, ConvertsEmptyBinary) {
  const auto value =
      DynamicValue::makeDefault(type_system::TypeSystem::Binary());

  EXPECT_EQ(fromDynamicValue<std::string>(value), "");
}

TEST(ValueConversionTest, PreservesOpaqueAliasType) {
  type_system::TypeSystemBuilder builder;
  builder.addType(
      "facebook.com/thrift/test/IntAlias",
      def::OpaqueAlias(type_system::TypeIds::I32));
  const std::shared_ptr<const type_system::TypeSystem> typeSystem =
      std::move(builder).build();
  const auto aliasType = type_system::TypeRef(
      typeSystem->getUserDefinedTypeOrThrow("facebook.com/thrift/test/IntAlias")
          .asOpaqueAlias());

  const auto value = toDynamicValue(42, aliasType);

  EXPECT_TRUE(value.type().isEqualIdentityTo(aliasType));
  EXPECT_EQ(fromDynamicValue<int32_t>(value), 42);
}

TEST(ValueConversionTest, CopiesMatchingDynamicValue) {
  const auto input = DynamicValue::makeI32(42);

  const auto value = toDynamicValue(input, type_system::TypeSystem::I32());

  EXPECT_EQ(value, input);
}

TEST(ValueConversionTest, ConvertsAnyStruct) {
  const auto expected = type::AnyData::toAny<type::i32_t>(42).toThrift();

  const auto value = toDynamicValue(expected, type_system::TypeSystem::Any());

  EXPECT_EQ(fromDynamicValue<type::AnyStruct>(value), expected);
}

TEST(ValueConversionTest, TraversesStructuredValues) {
  constexpr auto kColorUri =
      "facebook.com/thrift/service_descriptor_test/Color";
  constexpr auto kStructUri =
      "facebook.com/thrift/service_descriptor_test/AnnotationWithFields";
  type_system::TypeSystemBuilder builder;
  builder.addType(
      kColorUri, def::Enum({{"Unknown", 0}, {"Red", 1}, {"Blue", 2}}));
  builder.addType(
      kStructUri,
      def::Struct({
          def::Field(
              def::Identity(1, "label"),
              def::AlwaysPresent,
              type_system::TypeIds::String),
          def::Field(
              def::Identity(2, "number"),
              def::AlwaysPresent,
              type_system::TypeIds::I32),
          def::Field(
              def::Identity(3, "tags"),
              def::AlwaysPresent,
              type_system::TypeIds::list(type_system::TypeIds::String)),
          def::Field(
              def::Identity(4, "color"),
              def::AlwaysPresent,
              type_system::TypeIds::uri(kColorUri)),
          def::Field(
              def::Identity(5, "scores"),
              def::AlwaysPresent,
              type_system::TypeIds::map(
                  type_system::TypeIds::String, type_system::TypeIds::I32)),
      }));
  const std::shared_ptr<const type_system::TypeSystem> typeSystem =
      std::move(builder).build();
  const auto structType = type_system::TypeRef(
      typeSystem->getUserDefinedTypeOrThrow(kStructUri).asStruct());

  facebook::thrift::service_descriptor_test::AnnotationWithFields expected;
  expected.label() = "label";
  expected.number() = 7;
  expected.tags() = {"a", "b"};
  expected.color() = facebook::thrift::service_descriptor_test::Color::Blue;
  expected.scores() = {{"a", 1}, {"b", 2}};

  const auto value = toDynamicValue(expected, structType);

  EXPECT_EQ(
      fromDynamicValue<
          facebook::thrift::service_descriptor_test::AnnotationWithFields>(
          value),
      expected);
}

TEST(ValueConversionTest, IgnoresFieldsMissingFromRuntimeSchema) {
  constexpr auto kStructUri =
      "facebook.com/thrift/service_descriptor_test/AnnotationWithFields";
  type_system::TypeSystemBuilder builder;
  builder.addType(
      kStructUri,
      def::Struct({def::Field(
          def::Identity(1, "label"),
          def::AlwaysPresent,
          type_system::TypeIds::String)}));
  const std::shared_ptr<const type_system::TypeSystem> typeSystem =
      std::move(builder).build();
  const auto structType = type_system::TypeRef(
      typeSystem->getUserDefinedTypeOrThrow(kStructUri).asStruct());
  facebook::thrift::service_descriptor_test::AnnotationWithFields input;
  input.label() = "label";
  input.number() = 7;

  const auto value = toDynamicValue(input, structType);

  EXPECT_EQ(value.asStruct().getField("label")->asString().view(), "label");
  EXPECT_EQ(value.type().asStruct().fields().size(), 1);
}

TEST(ValueConversionTest, RejectsMismatchedDynamicValue) {
  EXPECT_THROW(
      (void)toDynamicValue(
          DynamicValue::makeString("42"), type_system::TypeSystem::I32()),
      std::invalid_argument);
  EXPECT_THROW(
      (void)toDynamicValue(
          DynamicValue::makeI32(42), type_system::TypeSystem::I64()),
      std::invalid_argument);
  EXPECT_THROW(
      (void)toDynamicValue(42, type_system::TypeSystem::Any()),
      std::invalid_argument);
  EXPECT_THROW(
      (void)fromDynamicValue<int32_t>(DynamicValue::makeI64(42)),
      std::invalid_argument);
}

} // namespace
} // namespace apache::thrift::dynamic
