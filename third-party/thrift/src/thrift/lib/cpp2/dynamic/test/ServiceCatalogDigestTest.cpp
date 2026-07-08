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

#include <thrift/lib/cpp2/dynamic/ServiceCatalogDigest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include <glog/logging.h>
#include <gtest/gtest.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/dynamic/DynamicValue.h>
#include <thrift/lib/cpp2/dynamic/ServiceDescriptorBuilder.h>
#include <thrift/lib/cpp2/dynamic/ServiceDescriptorSerialization.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>
#include <thrift/lib/cpp2/dynamic/test/gen-cpp2/service_catalog_digest_expected_values_constants.h>

namespace apache::thrift::dynamic {
namespace {

using def = type_system::TypeSystemBuilder::DefinitionHelper;
namespace expected =
    apache::thrift::dynamic::test::service_catalog_digest_expected_values::
        service_catalog_digest_expected_values_constants;

std::string toHex(const ServiceCatalogDigest& digest) {
  constexpr char kHex[] = "0123456789abcdef";
  std::string result;
  result.reserve(digest.size() * 2);
  for (auto byte : digest) {
    const auto value = static_cast<std::uint8_t>(byte);
    result.push_back(kHex[value >> 4]);
    result.push_back(kHex[value & 0x0f]);
  }
  return result;
}

std::string toDigestBytes(const type_system::TypeSystemDigest& digest) {
  return std::string(
      reinterpret_cast<const char*>(digest.data()), digest.size());
}

std::shared_ptr<const type_system::TypeSystem> makeTypeSystem() {
  return std::shared_ptr<const type_system::TypeSystem>(
      type_system::TypeSystemBuilder{}.build());
}

std::shared_ptr<const type_system::TypeSystem> makeAnnotationTypeSystem() {
  type_system::TypeSystemBuilder builder;
  builder.addType(
      "test.com/Annotation",
      def::Struct({
          def::Field(
              def::Identity(1, "label"),
              def::Optional,
              type_system::TypeIds::String),
      }));
  return std::shared_ptr<const type_system::TypeSystem>(
      std::move(builder).build());
}

std::shared_ptr<const type_system::TypeSystem> makeRichAnnotationTypeSystem() {
  type_system::TypeSystemBuilder builder;
  builder.addType(
      "test.com/RichAnnotation",
      def::Struct({
          def::Field(
              def::Identity(1, "label"),
              def::Optional,
              type_system::TypeIds::String),
          def::Field(
              def::Identity(2, "enabled"),
              def::Optional,
              type_system::TypeIds::Bool),
          def::Field(
              def::Identity(3, "byteValue"),
              def::Optional,
              type_system::TypeIds::Byte),
          def::Field(
              def::Identity(4, "i16Value"),
              def::Optional,
              type_system::TypeIds::I16),
          def::Field(
              def::Identity(5, "i32Value"),
              def::Optional,
              type_system::TypeIds::I32),
          def::Field(
              def::Identity(6, "i64Value"),
              def::Optional,
              type_system::TypeIds::I64),
          def::Field(
              def::Identity(7, "floatValue"),
              def::Optional,
              type_system::TypeIds::Float),
          def::Field(
              def::Identity(8, "doubleValue"),
              def::Optional,
              type_system::TypeIds::Double),
          def::Field(
              def::Identity(9, "data"),
              def::Optional,
              type_system::TypeIds::Binary),
          def::Field(
              def::Identity(10, "tags"),
              def::Optional,
              type_system::TypeIds::list(type_system::TypeIds::String)),
          def::Field(
              def::Identity(11, "levels"),
              def::Optional,
              type_system::TypeIds::set(type_system::TypeIds::I32)),
          def::Field(
              def::Identity(12, "weights"),
              def::Optional,
              type_system::TypeIds::map(
                  type_system::TypeIds::String, type_system::TypeIds::I64)),
      }));
  return std::shared_ptr<const type_system::TypeSystem>(
      std::move(builder).build());
}

DynamicValue makeAnnotation(
    const type_system::TypeSystem& typeSystem, std::string_view label) {
  auto value =
      DynamicValue::makeDefault(typeSystem.UserDefined("test.com/Annotation"));
  value.asStruct().setField("label", DynamicValue::makeString(label));
  return value;
}

DynamicValue makeRichAnnotation(const type_system::TypeSystem& typeSystem) {
  auto value = DynamicValue::makeDefault(
      typeSystem.UserDefined("test.com/RichAnnotation"));
  auto& fields = value.asStruct();
  fields.setField(
      "label",
      DynamicValue::makeString(std::string_view{"runtime\0value", 13}));
  fields.setField("enabled", DynamicValue::makeBool(true));
  fields.setField("byteValue", DynamicValue::makeByte(-7));
  fields.setField("i16Value", DynamicValue::makeI16(-1234));
  fields.setField("i32Value", DynamicValue::makeI32(123456));
  fields.setField("i64Value", DynamicValue::makeI64(1234567890123));
  fields.setField("floatValue", DynamicValue::makeFloat(1.25f));
  fields.setField("doubleValue", DynamicValue::makeDouble(-2.5));
  fields.setField(
      "data",
      DynamicValue::makeBinary(
          folly::IOBuf::copyBuffer(std::string_view{"bin\0data", 8})));

  auto tags = DynamicValue::makeDefault(
      typeSystem.ListOf(type_system::TypeSystem::String()));
  tags.asList().push_back(DynamicValue::makeString("alpha"));
  tags.asList().push_back(DynamicValue::makeString("beta"));
  fields.setField("tags", std::move(tags));

  auto levels = DynamicValue::makeDefault(
      typeSystem.SetOf(type_system::TypeSystem::I32()));
  levels.asSet().insert(DynamicValue::makeI32(2));
  levels.asSet().insert(DynamicValue::makeI32(1));
  fields.setField("levels", std::move(levels));

  auto weights = DynamicValue::makeDefault(typeSystem.MapOf(
      type_system::TypeSystem::String(), type_system::TypeSystem::I64()));
  weights.asMap().insert(
      DynamicValue::makeString("left"), DynamicValue::makeI64(10));
  weights.asMap().insert(
      DynamicValue::makeString("right"), DynamicValue::makeI64(20));
  fields.setField("weights", std::move(weights));

  return value;
}

std::unique_ptr<ServiceDescriptor> makeCalculator() {
  ServiceDescriptorBuilder builder(
      makeTypeSystem(), "Calculator", "test.com/Calculator");
  builder.addFunction("subtract")
      .addParam("left", FieldId{1}, type_system::TypeSystem::I32())
      .addParam("right", FieldId{2}, type_system::TypeSystem::I32())
      .setResponseType(type_system::TypeSystem::I32());
  builder.addFunction("add")
      .addParam("left", FieldId{1}, type_system::TypeSystem::I32())
      .addParam("right", FieldId{2}, type_system::TypeSystem::I32())
      .setResponseType(type_system::TypeSystem::I32());
  return builder.build();
}

std::unique_ptr<ServiceDescriptor> makeAnnotatedCalculator(
    std::string_view label) {
  auto typeSystem = makeAnnotationTypeSystem();
  ServiceDescriptorBuilder builder(
      typeSystem, "Calculator", "test.com/Calculator");
  builder.addServiceAnnotation(makeAnnotation(*typeSystem, label));
  builder.addFunction("get")
      .addAnnotation(makeAnnotation(*typeSystem, label))
      .setResponseType(type_system::TypeSystem::I32());
  return builder.build();
}

std::unique_ptr<ServiceDescriptor> makeAnnotatedSessionCalculator(
    std::string_view label) {
  auto typeSystem = makeAnnotationTypeSystem();
  ServiceDescriptorBuilder builder(
      typeSystem, "Calculator", "test.com/Calculator");
  builder.addServiceAnnotation(makeAnnotation(*typeSystem, label));
  builder.addFunction("makeSession")
      .addParam(
          "seed",
          FieldId{1},
          type_system::TypeSystem::I32(),
          {makeAnnotation(*typeSystem, label)})
      .addAnnotation(makeAnnotation(*typeSystem, label))
      .setCreatedInteractionUri("test.com/CalculatorSession");

  auto& interaction =
      builder.addInteraction("CalculatorSession", "test.com/CalculatorSession")
          .addAnnotation(makeAnnotation(*typeSystem, label));
  interaction.addFunction("get")
      .addAnnotation(makeAnnotation(*typeSystem, label))
      .setResponseType(type_system::TypeSystem::I32());
  return builder.build();
}

std::unique_ptr<ServiceDescriptor> makeRichDescriptor() {
  auto typeSystem = makeRichAnnotationTypeSystem();
  ServiceDescriptorBuilder builder(
      typeSystem, "CatalogGolden", "test.com/CatalogGolden");
  builder.addServiceAnnotation(makeRichAnnotation(*typeSystem));
  builder.addFunction("makeSession")
      .addParam(
          "seed",
          FieldId{1},
          type_system::TypeSystem::I32(),
          {makeRichAnnotation(*typeSystem)})
      .addAnnotation(makeRichAnnotation(*typeSystem))
      .setQualifier(FunctionQualifier::Idempotent)
      .setCreatedInteractionUri("test.com/CatalogGoldenSession");
  builder.addFunction("observe")
      .addAnnotation(makeRichAnnotation(*typeSystem))
      .setBidirectionalStream(
          type_system::TypeSystem::I32(), type_system::TypeSystem::String());
  builder.addFunction("upload")
      .setSink(
          type_system::TypeSystem::I32(), type_system::TypeSystem::String())
      .addException("appError", FieldId{1}, type_system::TypeSystem::String());
  builder.addFunction("notify").setOneWay();

  auto& interaction = builder.addInteraction(
      "CatalogGoldenSession", "test.com/CatalogGoldenSession");
  interaction.addAnnotation(makeRichAnnotation(*typeSystem));
  interaction.addFunction("get")
      .addAnnotation(makeRichAnnotation(*typeSystem))
      .setQualifier(FunctionQualifier::ReadOnly)
      .setResponseType(type_system::TypeSystem::I64());
  return builder.build();
}

const ServiceDescriptor& requireDescriptor(
    const std::unique_ptr<ServiceDescriptor>& descriptor) {
  return *CHECK_NOTNULL(descriptor.get());
}

std::string_view requireExpectedDigest(const char* digest) {
  return CHECK_NOTNULL(digest);
}

void expectGoldenDigest(
    const ServiceDescriptor& descriptor,
    std::string_view serviceUri,
    std::string_view expectedDigest,
    type_system::DigestMode mode = type_system::DigestMode::Full) {
  ServiceCatalogHasher hasher{mode};
  auto catalog = toSerializable(descriptor, serviceUri);
  auto outOfBandCatalog = catalog;
  if (mode != type_system::DigestMode::Full) {
    type_system::TypeSystemHasher typeHasher{mode};
    outOfBandCatalog.typesDigest() =
        toDigestBytes(typeHasher(*catalog.types()));
  }
  outOfBandCatalog.types_ref().reset();

  const auto descriptorDigest = hasher(descriptor, serviceUri);
  const auto inlineCatalogDigest = hasher(catalog);
  const auto outOfBandCatalogDigest = hasher(outOfBandCatalog);

  EXPECT_EQ(descriptorDigest, inlineCatalogDigest);
  EXPECT_EQ(descriptorDigest, outOfBandCatalogDigest);
  EXPECT_EQ(toHex(descriptorDigest), expectedDigest);
}

TEST(ServiceCatalogDigestTest, VersionConstantExists) {
  EXPECT_EQ(kServiceCatalogDigestVersion, 1);
}

TEST(ServiceCatalogDigestTest, ToSerializableSetsTypeDigest) {
  auto service = makeCalculator();
  const auto& descriptor = requireDescriptor(service);
  auto catalog = toSerializable(descriptor, "test.com/Calculator");

  type_system::TypeSystemHasher typeHasher;
  const auto expected = typeHasher(*catalog.types());
  const auto& bytes = *catalog.typesDigest();

  ASSERT_EQ(bytes.size(), expected.size());
  EXPECT_EQ(
      bytes,
      std::string(
          reinterpret_cast<const char*>(expected.data()), expected.size()));
}

TEST(ServiceCatalogDigestTest, DescriptorAndSerializedCatalogMatch) {
  auto service = makeCalculator();
  const auto& descriptor = requireDescriptor(service);
  auto catalog = toSerializable(descriptor, "test.com/Calculator");

  EXPECT_EQ(
      ServiceCatalogHasher{}(descriptor, "test.com/Calculator"),
      ServiceCatalogHasher{}(catalog));
}

TEST(ServiceCatalogDigestTest, GoldenCalculatorDigest) {
  auto service = makeCalculator();
  expectGoldenDigest(
      requireDescriptor(service),
      "test.com/Calculator",
      requireExpectedDigest(expected::DIGEST_CALCULATOR()));
}

TEST(
    ServiceCatalogDigestTest,
    DescriptorAndSerializedCatalogMatchWithAnnotationsAndInteractions) {
  auto service = makeAnnotatedSessionCalculator("runtime");
  const auto& descriptor = requireDescriptor(service);
  auto catalog = toSerializable(descriptor, "test.com/Calculator");

  EXPECT_EQ(
      ServiceCatalogHasher{}(descriptor, "test.com/Calculator"),
      ServiceCatalogHasher{}(catalog));
}

TEST(ServiceCatalogDigestTest, GoldenRichDescriptorDigest) {
  auto service = makeRichDescriptor();
  expectGoldenDigest(
      requireDescriptor(service),
      "test.com/CatalogGolden",
      requireExpectedDigest(expected::DIGEST_RICH_DESCRIPTOR()));
}

TEST(ServiceCatalogDigestTest, GoldenRichDescriptorStructuralDigest) {
  auto service = makeRichDescriptor();
  expectGoldenDigest(
      requireDescriptor(service),
      "test.com/CatalogGolden",
      requireExpectedDigest(expected::DIGEST_RICH_DESCRIPTOR_STRUCTURAL()),
      type_system::DigestMode::Structural);
}

TEST(ServiceCatalogDigestTest, InlineAndOutOfBandTypesMatch) {
  auto service = makeCalculator();
  auto inlineCatalog =
      toSerializable(requireDescriptor(service), "test.com/Calculator");
  auto outOfBandCatalog = inlineCatalog;
  outOfBandCatalog.types_ref().reset();

  EXPECT_EQ(
      ServiceCatalogHasher{}(inlineCatalog),
      ServiceCatalogHasher{}(outOfBandCatalog));
}

TEST(ServiceCatalogDigestTest, IgnoresFunctionAndParameterOrder) {
  auto service = makeCalculator();
  auto original =
      toSerializable(requireDescriptor(service), "test.com/Calculator");
  auto reordered = original;

  auto& functions = *reordered.interfaces()
                         ->at("test.com/Calculator")
                         .serviceDef_ref()
                         ->functions();
  std::reverse(functions.begin(), functions.end());
  auto& params = *functions.at(0).params();
  std::reverse(params.begin(), params.end());

  EXPECT_EQ(
      ServiceCatalogHasher{}(original), ServiceCatalogHasher{}(reordered));
}

TEST(ServiceCatalogDigestTest, ChangesWhenFunctionTypeChanges) {
  ServiceDescriptorBuilder i32Builder(
      makeTypeSystem(), "Calculator", "test.com/Calculator");
  i32Builder.addFunction("get").setResponseType(type_system::TypeSystem::I32());

  ServiceDescriptorBuilder i64Builder(
      makeTypeSystem(), "Calculator", "test.com/Calculator");
  i64Builder.addFunction("get").setResponseType(type_system::TypeSystem::I64());

  auto i32Service = i32Builder.build();
  auto i64Service = i64Builder.build();
  EXPECT_NE(
      ServiceCatalogHasher{}(
          requireDescriptor(i32Service), "test.com/Calculator"),
      ServiceCatalogHasher{}(
          requireDescriptor(i64Service), "test.com/Calculator"));
}

TEST(ServiceCatalogDigestTest, ChangesWhenInteractionChanges) {
  ServiceDescriptorBuilder i32Builder(
      makeTypeSystem(), "Calculator", "test.com/Calculator");
  i32Builder.addFunction("makeSession")
      .setCreatedInteractionUri("test.com/CalculatorSession");
  i32Builder.addInteraction("CalculatorSession", "test.com/CalculatorSession")
      .addFunction("get")
      .setResponseType(type_system::TypeSystem::I32());

  ServiceDescriptorBuilder i64Builder(
      makeTypeSystem(), "Calculator", "test.com/Calculator");
  i64Builder.addFunction("makeSession")
      .setCreatedInteractionUri("test.com/CalculatorSession");
  i64Builder.addInteraction("CalculatorSession", "test.com/CalculatorSession")
      .addFunction("get")
      .setResponseType(type_system::TypeSystem::I64());

  auto i32Service = i32Builder.build();
  auto i64Service = i64Builder.build();
  EXPECT_NE(
      ServiceCatalogHasher{}(
          requireDescriptor(i32Service), "test.com/Calculator"),
      ServiceCatalogHasher{}(
          requireDescriptor(i64Service), "test.com/Calculator"));
}

TEST(ServiceCatalogDigestTest, StructuralModeIgnoresAnnotations) {
  auto firstDescriptor = makeAnnotatedCalculator("first");
  auto secondDescriptor = makeAnnotatedCalculator("second");
  const auto& firstService = requireDescriptor(firstDescriptor);
  const auto& secondService = requireDescriptor(secondDescriptor);
  auto first = toSerializable(firstService, "test.com/Calculator");
  auto second = toSerializable(secondService, "test.com/Calculator");

  ServiceCatalogHasher full;
  ServiceCatalogHasher structural{type_system::DigestMode::Structural};

  EXPECT_NE(full(first), full(second));
  EXPECT_EQ(structural(first), structural(second));
  EXPECT_NE(
      full(firstService, "test.com/Calculator"),
      full(secondService, "test.com/Calculator"));
  EXPECT_EQ(
      structural(firstService, "test.com/Calculator"),
      structural(secondService, "test.com/Calculator"));
}

} // namespace
} // namespace apache::thrift::dynamic
