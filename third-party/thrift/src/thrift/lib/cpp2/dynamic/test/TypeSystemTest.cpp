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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <thrift/lib/cpp2/dynamic/SerializableTypeSystemBuilder.h>
#include <thrift/lib/cpp2/dynamic/TypeId.h>
#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemTraits.h>

#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace apache::thrift::type_system {

// For conciseness
using def = TypeSystemBuilder::DefinitionHelper;

namespace {

void expectKnownUris(
    const TypeSystem& typeSystem, std::initializer_list<Uri> uris) {
  folly::F14FastSet<Uri> expectedUris{uris.begin(), uris.end()};
  const auto knownUris = *typeSystem.getKnownUris();
  EXPECT_EQ(expectedUris, knownUris);
}

constexpr auto kEmptyStructUri = "meta.com/thrift/test/EmptyStruct";
constexpr auto kEmptyUnionUri = "meta.com/thrift/test/EmptyUnion";
constexpr auto kEmptyEnumUri = "meta.com/thrift/test/EmptyEnum";

std::unique_ptr<TypeSystem> typeSystemWithEmpties() {
  TypeSystemBuilder builder;
  builder.addType(kEmptyStructUri, def::Struct({}));
  builder.addType(kEmptyUnionUri, def::Union({}));
  builder.addType(kEmptyEnumUri, def::Enum({}));

  return std::move(builder).build();
}

} // namespace

// DefinitionRef should be a cheap value type
static_assert(std::is_trivially_copyable_v<DefinitionRef>);

TEST(TypeSystemTest, EmptyStruct) {
  TypeSystemBuilder builder;

  // Define an empty struct
  builder.addType("meta.com/thrift/test/EmptyStruct", def::Struct({}));

  auto typeSystem = std::move(builder).build();

  // Check the empty struct
  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/EmptyStruct"));
  EXPECT_EQ(def.uri(), "meta.com/thrift/test/EmptyStruct");
  EXPECT_EQ(def.asStruct().fields().size(), 0);

  expectKnownUris(*typeSystem, {def.uri()});

  auto stsBuilder =
      SerializableTypeSystemBuilder::withoutSourceInfo(*typeSystem);
  stsBuilder.addDefinition("meta.com/thrift/test/EmptyStruct");
  auto sts = *std::move(stsBuilder).build();
  EXPECT_EQ(sts.types()->size(), 1);
  auto& emptyStructDef =
      *sts.types()["meta.com/thrift/test/EmptyStruct"].definition();
  EXPECT_EQ(emptyStructDef.structDef()->fields()->size(), 0);
}

TEST(TypeSystemTest, EmptyUnion) {
  TypeSystemBuilder builder;

  // Define a union with no fields
  builder.addType("meta.com/thrift/test/EmptyUnion", def::Union({}));

  auto typeSystem = std::move(builder).build();

  // Check the empty union
  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/EmptyUnion"));
  EXPECT_EQ(def.uri(), "meta.com/thrift/test/EmptyUnion");
  EXPECT_EQ(def.asUnion().fields().size(), 0);

  expectKnownUris(*typeSystem, {def.uri()});

  auto stsBuilder =
      SerializableTypeSystemBuilder::withoutSourceInfo(*typeSystem);
  stsBuilder.addDefinition("meta.com/thrift/test/EmptyUnion");
  auto sts = *std::move(stsBuilder).build();
  EXPECT_EQ(sts.types()->size(), 1);
  auto& emptyUnionDef =
      *sts.types()["meta.com/thrift/test/EmptyUnion"].definition();
  EXPECT_EQ(emptyUnionDef.unionDef()->fields()->size(), 0);
}

TEST(TypeSystemTest, NestedStructs) {
  TypeSystemBuilder builder;

  // Define a nested struct
  Uri outerStructUri = "meta.com/thrift/test/OuterStruct";
  Uri innerStructUri = "meta.com/thrift/test/InnerStruct";

  builder.addType(
      outerStructUri,
      def::Struct({
          def::Field(
              def::Identity(1, "innerStruct"),
              def::Optional,
              TypeIds::uri(innerStructUri)),
      }));

  builder.addType(
      innerStructUri,
      def::Struct({
          def::Field(def::Identity(1, "field1"), def::Optional, TypeIds::I32),
      }));

  auto typeSystem = std::move(builder).build();

  // Check the outer struct
  DefinitionRef outerDef =
      typeSystem->getUserDefinedTypeOrThrow(outerStructUri);
  EXPECT_EQ(outerDef.uri(), outerStructUri);
  EXPECT_EQ(outerDef.asStruct().fields().size(), 1);
  const FieldDefinition& innerField = outerDef.asStruct().fields()[0];
  EXPECT_EQ(innerField.identity(), def::Identity(1, "innerStruct"));
  EXPECT_EQ(innerField.presence(), def::Optional);
  EXPECT_EQ(innerField.type().asStruct().fields().size(), 1);
  const FieldDefinition& field1 = innerField.type().asStruct().fields()[0];
  EXPECT_EQ(field1.identity(), def::Identity(1, "field1"));
  EXPECT_EQ(field1.presence(), def::Optional);
  EXPECT_EQ(field1.type().id(), TypeIds::I32);

  expectKnownUris(*typeSystem, {innerStructUri, outerStructUri});

  auto stsBuilder =
      SerializableTypeSystemBuilder::withoutSourceInfo(*typeSystem);
  stsBuilder.addDefinition(outerStructUri);
  auto sts = *std::move(stsBuilder).build();
  EXPECT_EQ(sts.types()->size(), 2);
  auto& innerStructDef = *sts.types()->at(innerStructUri).definition();
  EXPECT_EQ(innerStructDef.structDef()->fields()->size(), 1);
  EXPECT_EQ(
      innerStructDef.structDef()->fields()[0].identity()->id(), FieldId{1});
  EXPECT_EQ(
      innerStructDef.structDef()->fields()[0].identity()->name(), "field1");
  EXPECT_EQ(
      innerStructDef.structDef()->fields()[0].presence(),
      PresenceQualifier::OPTIONAL_);
  EXPECT_EQ(innerStructDef.structDef()->fields()[0].type(), TypeIds::I32);
  auto& outerStructDef = *sts.types()->at(outerStructUri).definition();
  EXPECT_EQ(outerStructDef.structDef()->fields()->size(), 1);
  EXPECT_EQ(
      outerStructDef.structDef()->fields()[0].identity()->id(), FieldId{1});
  EXPECT_EQ(
      outerStructDef.structDef()->fields()[0].identity()->name(),
      "innerStruct");
  EXPECT_EQ(
      outerStructDef.structDef()->fields()[0].presence(),
      PresenceQualifier::OPTIONAL_);
  EXPECT_EQ(
      outerStructDef.structDef()->fields()[0].type(),
      TypeIds::uri(innerStructUri));
}

TEST(TypeSystemTest, RecursiveStructAndUnion) {
  TypeSystemBuilder builder;

  Uri recursiveStructUri = "meta.com/thrift/test/RecursiveStruct";
  Uri recursiveUnionUri = "meta.com/thrift/test/RecursiveUnion";

  // Define a recursive struct
  builder.addType(
      recursiveStructUri,
      def::Struct({
          def::Field(
              def::Identity(1, "self"),
              def::Optional,
              TypeIds::uri(recursiveStructUri)),
      }));

  // Define a recursive union
  builder.addType(
      recursiveUnionUri,
      def::Union({
          def::Field(
              def::Identity(1, "self"),
              def::Optional,
              TypeIds::uri(recursiveUnionUri)),
      }));

  auto typeSystem = std::move(builder).build();

  // Check the recursive struct
  DefinitionRef structDef =
      typeSystem->getUserDefinedTypeOrThrow(recursiveStructUri);
  EXPECT_EQ(structDef.uri(), recursiveStructUri);
  EXPECT_EQ(structDef.asStruct().fields().size(), 1);
  const FieldDefinition& structField = structDef.asStruct().fields()[0];
  EXPECT_EQ(structField.identity(), def::Identity(1, "self"));
  EXPECT_EQ(structField.presence(), def::Optional);
  EXPECT_EQ(
      std::addressof(structField.type().asStruct()),
      std::addressof(structDef.asStruct()));

  // Check the recursive union
  DefinitionRef unionDef =
      typeSystem->getUserDefinedTypeOrThrow(recursiveUnionUri);
  EXPECT_EQ(unionDef.uri(), recursiveUnionUri);
  EXPECT_EQ(unionDef.asUnion().fields().size(), 1);
  const FieldDefinition& unionField = unionDef.asUnion().fields()[0];
  EXPECT_EQ(unionField.identity(), def::Identity(1, "self"));
  EXPECT_EQ(unionField.presence(), def::Optional);
  EXPECT_EQ(
      std::addressof(unionField.type().asUnion()),
      std::addressof(unionDef.asUnion()));

  expectKnownUris(*typeSystem, {structDef.uri(), unionDef.uri()});

  auto stsBuilder =
      SerializableTypeSystemBuilder::withoutSourceInfo(*typeSystem);
  stsBuilder.addDefinition(recursiveStructUri);
  stsBuilder.addDefinition(recursiveUnionUri);
  auto sts = *std::move(stsBuilder).build();
  EXPECT_EQ(sts.types()->size(), 2);
  auto& recursiveStructDef = *sts.types()->at(recursiveStructUri).definition();
  EXPECT_EQ(recursiveStructDef.structDef()->fields()->size(), 1);
  EXPECT_EQ(
      recursiveStructDef.structDef()->fields()[0].identity()->id(), FieldId{1});
  EXPECT_EQ(
      recursiveStructDef.structDef()->fields()[0].identity()->name(), "self");
  EXPECT_EQ(
      recursiveStructDef.structDef()->fields()[0].presence(),
      PresenceQualifier::OPTIONAL_);
  EXPECT_EQ(
      recursiveStructDef.structDef()->fields()[0].type(),
      TypeIds::uri(recursiveStructUri));
  auto& recursiveUnionDef = *sts.types()->at(recursiveUnionUri).definition();
  EXPECT_EQ(recursiveUnionDef.unionDef()->fields()->size(), 1);
  EXPECT_EQ(
      recursiveUnionDef.unionDef()->fields()[0].identity()->id(), FieldId{1});
  EXPECT_EQ(
      recursiveUnionDef.unionDef()->fields()[0].identity()->name(), "self");
  EXPECT_EQ(
      recursiveUnionDef.unionDef()->fields()[0].presence(),
      PresenceQualifier::OPTIONAL_);
}

TEST(TypeSystemTest, IncompleteTypeSystem) {
  // Missing MyUnion
  TypeSystemBuilder builder;

  builder.addType(
      "meta.com/thrift/test/MyStruct",
      def::Struct({
          def::Field(def::Identity(1, "field1"), def::Optional, TypeIds::I32),
          def::Field(
              def::Identity(2, "field2"),
              def::AlwaysPresent,
              TypeIds::uri("meta.com/thrift/test/MyUnion")),
      }));
  EXPECT_THROW(std::move(builder).build(), InvalidTypeError);
}

TEST(TypeSystemTest, MutuallyRecursiveStructuredTypes) {
  TypeSystemBuilder builder;

  builder.addType(
      "meta.com/thrift/test/MyStruct",
      def::Struct({
          def::Field(def::Identity(1, "field1"), def::Optional, TypeIds::I32),
          def::Field(
              def::Identity(2, "field2"),
              def::AlwaysPresent,
              TypeIds::uri("meta.com/thrift/test/MyUnion")),
      }));
  builder.addType(
      "meta.com/thrift/test/MyUnion",
      def::Union({
          def::Field(def::Identity(1, "int64"), def::Optional, TypeIds::I64),
          def::Field(
              def::Identity(2, "myStruct"),
              def::Optional,
              TypeIds::uri("meta.com/thrift/test/MyStruct")),
      }));

  auto typeSystem = std::move(builder).build();

  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/MyStruct"));

  EXPECT_EQ(def.uri(), "meta.com/thrift/test/MyStruct");
  EXPECT_EQ(def.asStruct().fields().size(), 2);
  {
    const FieldDefinition& field1 = def.asStruct().fields()[0];
    EXPECT_EQ(field1.identity(), def::Identity(1, "field1"));
    EXPECT_EQ(field1.presence(), def::Optional);
    EXPECT_EQ(field1.type().id(), TypeIds::I32);
  }
  {
    const FieldDefinition& field2 = def.asStruct().fields()[1];
    EXPECT_EQ(field2.identity(), def::Identity(2, "field2"));
    EXPECT_EQ(field2.presence(), def::AlwaysPresent);
    EXPECT_EQ(field2.type().id(), TypeIds::uri("meta.com/thrift/test/MyUnion"));
    {
      TypeRef typeRef = field2.type();
      const UnionNode& otherUnion = typeRef.asUnion();
      EXPECT_EQ(otherUnion.fields().size(), 2);
      {
        const FieldDefinition& int64 = otherUnion.fields()[0];
        EXPECT_EQ(int64.identity(), def::Identity(1, "int64"));
        EXPECT_EQ(int64.presence(), def::Optional);
        EXPECT_EQ(int64.type().id(), TypeIds::I64);
      }
      {
        const FieldDefinition& myStruct = otherUnion.fields()[1];
        EXPECT_EQ(myStruct.identity(), def::Identity(2, "myStruct"));
        EXPECT_EQ(myStruct.presence(), def::Optional);
        EXPECT_EQ(
            std::addressof(myStruct.type().asStruct()),
            std::addressof(def.asStruct()));
      }
    }
  }

  expectKnownUris(
      *typeSystem,
      {"meta.com/thrift/test/MyStruct", "meta.com/thrift/test/MyUnion"});

  auto stsBuilder =
      SerializableTypeSystemBuilder::withoutSourceInfo(*typeSystem);
  stsBuilder.addDefinition("meta.com/thrift/test/MyStruct");
  stsBuilder.addDefinition("meta.com/thrift/test/MyUnion");
  auto sts = *std::move(stsBuilder).build();
  EXPECT_EQ(sts.types()->size(), 2);
  auto& myStructDef =
      *sts.types()->at("meta.com/thrift/test/MyStruct").definition();
  EXPECT_EQ(myStructDef.structDef()->fields()->size(), 2);
  EXPECT_EQ(myStructDef.structDef()->fields()[0].identity()->id(), FieldId{1});
  EXPECT_EQ(myStructDef.structDef()->fields()[0].identity()->name(), "field1");
  EXPECT_EQ(
      myStructDef.structDef()->fields()[0].presence(),
      PresenceQualifier::OPTIONAL_);
  EXPECT_EQ(myStructDef.structDef()->fields()[0].type(), TypeIds::I32);
  EXPECT_EQ(myStructDef.structDef()->fields()[1].identity()->id(), FieldId{2});
  EXPECT_EQ(myStructDef.structDef()->fields()[1].identity()->name(), "field2");
  EXPECT_EQ(
      myStructDef.structDef()->fields()[1].presence(),
      PresenceQualifier::UNQUALIFIED);
  EXPECT_EQ(
      myStructDef.structDef()->fields()[1].type(),
      TypeIds::uri("meta.com/thrift/test/MyUnion"));

  auto& myUnionDef =
      *sts.types()->at("meta.com/thrift/test/MyUnion").definition();
  EXPECT_EQ(myUnionDef.unionDef()->fields()->size(), 2);
  EXPECT_EQ(myUnionDef.unionDef()->fields()[0].identity()->id(), FieldId{1});
  EXPECT_EQ(myUnionDef.unionDef()->fields()[0].identity()->name(), "int64");
  EXPECT_EQ(
      myUnionDef.unionDef()->fields()[0].presence(),
      PresenceQualifier::OPTIONAL_);
  EXPECT_EQ(myUnionDef.unionDef()->fields()[0].type(), TypeIds::I64);
  EXPECT_EQ(myUnionDef.unionDef()->fields()[1].identity()->id(), FieldId{2});
  EXPECT_EQ(myUnionDef.unionDef()->fields()[1].identity()->name(), "myStruct");
  EXPECT_EQ(
      myUnionDef.unionDef()->fields()[1].presence(),
      PresenceQualifier::OPTIONAL_);
  EXPECT_EQ(
      myUnionDef.unionDef()->fields()[1].type(),
      TypeIds::uri("meta.com/thrift/test/MyStruct"));
}

TEST(TypeSystemTest, CustomDefaultFieldValues) {
  TypeSystemBuilder builder;

  // Define a struct with a custom default field value
  builder.addType(
      "meta.com/thrift/test/StructWithDefaults",
      def::Struct({
          def::Field(
              def::Identity(1, "fieldWithDefault"),
              def::Optional,
              TypeIds::I32,
              SerializableRecord::Int32(42)),
          def::Field(
              def::Identity(2, "fieldWithoutDefault"),
              def::Optional,
              TypeIds::I64),
      }));

  auto typeSystem = std::move(builder).build();

  // Check the struct with the custom default field value
  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/StructWithDefaults"));
  EXPECT_EQ(def.uri(), "meta.com/thrift/test/StructWithDefaults");
  EXPECT_EQ(def.asStruct().fields().size(), 2);

  const FieldDefinition& fieldWithDefault = def.asStruct().fields()[0];
  EXPECT_EQ(fieldWithDefault.identity(), def::Identity(1, "fieldWithDefault"));
  EXPECT_EQ(fieldWithDefault.presence(), def::Optional);
  EXPECT_EQ(fieldWithDefault.type().id(), TypeIds::I32);
  EXPECT_NE(fieldWithDefault.customDefault(), nullptr);
  EXPECT_EQ(fieldWithDefault.customDefault()->asInt32(), 42);

  const FieldDefinition& fieldWithoutDefault = def.asStruct().fields()[1];
  EXPECT_EQ(
      fieldWithoutDefault.identity(), def::Identity(2, "fieldWithoutDefault"));
  EXPECT_EQ(fieldWithoutDefault.presence(), def::Optional);
  EXPECT_EQ(fieldWithoutDefault.type().id(), TypeIds::I64);
  EXPECT_EQ(fieldWithoutDefault.customDefault(), nullptr);

  auto stsBuilder =
      SerializableTypeSystemBuilder::withoutSourceInfo(*typeSystem);
  stsBuilder.addDefinition("meta.com/thrift/test/StructWithDefaults");
  auto sts = *std::move(stsBuilder).build();
  EXPECT_EQ(sts.types()->size(), 1);
  auto& structWithDefaultsDef =
      *sts.types()->at("meta.com/thrift/test/StructWithDefaults").definition();
  EXPECT_EQ(structWithDefaultsDef.structDef()->fields()->size(), 2);
  EXPECT_EQ(
      structWithDefaultsDef.structDef()->fields()[0].identity()->id(),
      FieldId{1});
  EXPECT_EQ(
      structWithDefaultsDef.structDef()->fields()[0].identity()->name(),
      "fieldWithDefault");
  EXPECT_EQ(
      structWithDefaultsDef.structDef()->fields()[0].presence(),
      PresenceQualifier::OPTIONAL_);
  EXPECT_EQ(
      structWithDefaultsDef.structDef()->fields()[0].type(), TypeIds::I32);
  EXPECT_EQ(
      structWithDefaultsDef.structDef()
          ->fields()[0]
          .customDefaultPartialRecord()
          ->getType(),
      SerializableRecordUnion::Type::int32Datum);
  EXPECT_EQ(
      structWithDefaultsDef.structDef()
          ->fields()[0]
          .customDefaultPartialRecord()
          ->int32Datum(),
      42);
}

TEST(TypeSystemTest, Annotations) {
  TypeSystemBuilder builder;
  builder.addType(
      "meta.com/thrift/test/MyAnnot",
      def::Struct({def::Field(
          def::Identity(1, "field1"), def::Optional, TypeIds::I32)}));

  AnnotationsMap annots = {
      {"meta.com/thrift/test/MyAnnot",
       {SerializableRecord::FieldSet(
           {{FieldId(1), SerializableRecord::Int32(42)}})}}};

  // @MyAnnot{field1=42}
  // struct MyStruct{
  //   @MyAnnot{field1=42}
  //   1: optional i32 field1;
  // }
  builder.addType(
      "meta.com/thrift/test/MyStruct",
      def::Struct(
          {def::Field(
              def::Identity(1, "field1"),
              def::Optional,
              TypeIds::I32,
              std::nullopt,
              annots)},
          false,
          annots));

  // @MyAnnot{field1=42}
  // union MyUnion{
  //   @MyAnnot{field1=42}
  //   1: i32 field1;
  // }
  builder.addType(
      "meta.com/thrift/test/MyUnion",
      def::Union(
          {def::Field(
              def::Identity(1, "int64"),
              def::Optional,
              TypeIds::I64,
              std::nullopt,
              annots)},
          false,
          annots));

  // @MyAnnot{field1=42}
  // typedef i32 MyI32
  builder.addType(
      "meta.com/thrift/test/MyI32", def::OpaqueAlias(TypeIds::I32, annots));

  // @MyAnnot{field1=42}
  // enum MyEnum {
  //   @MyAnnot{field1=42}
  //   VALUE = 1,
  // }
  builder.addType(
      "meta.com/thrift/test/MyEnum",
      def::Enum({{"VALUE1", 1, annots}}, annots));

  auto typeSystem = std::move(builder).build();

  auto checkAnnot = [](const auto& node) {
    const auto* annot =
        node.getAnnotationOrNull("meta.com/thrift/test/MyAnnot");
    ASSERT_TRUE(annot != nullptr);
    EXPECT_EQ(annot->asFieldSet().at(FieldId{1}).asInt32(), 42);
  };

  auto checkAnnotSerializabe = [](const auto& node) {
    const auto& annot = node.annotations()->at("meta.com/thrift/test/MyAnnot");
    EXPECT_EQ(annot.fieldSetDatum()->at(FieldId{1}).int32Datum(), 42);
  };

  checkAnnot(
      typeSystem
          ->getUserDefinedTypeOrThrow(Uri("meta.com/thrift/test/MyStruct"))
          .asStruct());
  checkAnnot(
      typeSystem
          ->getUserDefinedTypeOrThrow(Uri("meta.com/thrift/test/MyStruct"))
          .asStruct()
          .at(FieldId{1}));
  checkAnnot(
      typeSystem->getUserDefinedTypeOrThrow(Uri("meta.com/thrift/test/MyUnion"))
          .asUnion());
  checkAnnot(
      typeSystem->getUserDefinedTypeOrThrow(Uri("meta.com/thrift/test/MyUnion"))
          .asUnion()
          .at(FieldId{1}));
  checkAnnot(
      typeSystem->getUserDefinedTypeOrThrow(Uri("meta.com/thrift/test/MyI32"))
          .asOpaqueAlias());
  checkAnnot(
      typeSystem->getUserDefinedTypeOrThrow(Uri("meta.com/thrift/test/MyEnum"))
          .asEnum());
  checkAnnot(
      typeSystem->getUserDefinedTypeOrThrow(Uri("meta.com/thrift/test/MyEnum"))
          .asEnum()
          .values()[0]);
  {
    auto stsBuilder =
        SerializableTypeSystemBuilder::withoutSourceInfo(*typeSystem);
    stsBuilder.addDefinition("meta.com/thrift/test/MyStruct");
    auto sts = *std::move(stsBuilder).build();
    EXPECT_EQ(sts.types()->size(), 2);
    checkAnnotSerializabe(*sts.types()
                               ->at("meta.com/thrift/test/MyStruct")
                               .definition()
                               ->structDef());
    checkAnnotSerializabe(sts.types()
                              ->at("meta.com/thrift/test/MyStruct")
                              .definition()
                              ->structDef()
                              ->fields()[0]);
  }
  {
    auto stsBuilder =
        SerializableTypeSystemBuilder::withoutSourceInfo(*typeSystem);
    stsBuilder.addDefinition("meta.com/thrift/test/MyUnion");
    auto sts = *std::move(stsBuilder).build();
    EXPECT_EQ(sts.types()->size(), 2);
    checkAnnotSerializabe(*sts.types()
                               ->at("meta.com/thrift/test/MyUnion")
                               .definition()
                               ->unionDef());
    checkAnnotSerializabe(sts.types()
                              ->at("meta.com/thrift/test/MyUnion")
                              .definition()
                              ->unionDef()
                              ->fields()[0]);
  }
  {
    auto stsBuilder =
        SerializableTypeSystemBuilder::withoutSourceInfo(*typeSystem);
    stsBuilder.addDefinition("meta.com/thrift/test/MyI32");
    auto sts = *std::move(stsBuilder).build();
    EXPECT_EQ(sts.types()->size(), 2);
    checkAnnotSerializabe(*sts.types()
                               ->at("meta.com/thrift/test/MyI32")
                               .definition()
                               ->opaqueAliasDef());
  }
  {
    auto stsBuilder =
        SerializableTypeSystemBuilder::withoutSourceInfo(*typeSystem);
    stsBuilder.addDefinition("meta.com/thrift/test/MyEnum");
    auto sts = *std::move(stsBuilder).build();
    EXPECT_EQ(sts.types()->size(), 2);
    checkAnnotSerializabe(*sts.types()
                               ->at("meta.com/thrift/test/MyEnum")
                               .definition()
                               ->enumDef());
    checkAnnotSerializabe(sts.types()
                              ->at("meta.com/thrift/test/MyEnum")
                              .definition()
                              ->enumDef()
                              ->values()[0]);
  }
}

TEST(TypeSystemTest, WrongAnnotationTypeUri) {
  TypeSystemBuilder builder;

  AnnotationsMap annots = {
      {"meta.com/thrift/test/MyAnnot",
       {SerializableRecord::FieldSet(
           {{FieldId(1), SerializableRecord::Int32(42)}})}}};

  // @MyAnnot{field1=42}
  // struct MyStruct{
  //   @MyAnnot{field1=42}
  //   1: optional i32 field1;
  // }
  builder.addType(
      "meta.com/thrift/test/MyAnnot",
      def::Union({def::Field(
          def::Identity(1, "field1"), def::Optional, TypeIds::I32)}));
  builder.addType(
      "meta.com/thrift/test/MyStruct",
      def::Struct(
          {def::Field(
              def::Identity(1, "field1"),
              def::Optional,
              TypeIds::I32,
              std::nullopt,
              annots)},
          false,
          annots));
  EXPECT_THROW(std::move(builder).build(), InvalidTypeError);
}

TEST(TypeSystemTest, WrongAnnotationTypeValue) {
  TypeSystemBuilder builder;

  // Value is not a FieldSetDatum.
  AnnotationsMap annots = {
      {"meta.com/thrift/test/MyAnnot", {SerializableRecord::Int32(42)}}};

  // @MyAnnot{field1=42}
  // struct MyStruct{
  //   @MyAnnot{field1=42}
  //   1: optional i32 field1;
  // }
  builder.addType(
      "meta.com/thrift/test/MyAnnot",
      def::Struct({def::Field(
          def::Identity(1, "field1"), def::Optional, TypeIds::I32)}));
  builder.addType(
      "meta.com/thrift/test/MyStruct",
      def::Struct(
          {def::Field(
              def::Identity(1, "field1"),
              def::Optional,
              TypeIds::I32,
              std::nullopt,
              annots)},
          false,
          annots));
  EXPECT_THROW(std::move(builder).build(), InvalidTypeError);
}

TEST(TypeSystemTest, MissingAnnotationType) {
  TypeSystemBuilder builder;

  AnnotationsMap annots = {
      {"meta.com/thrift/test/MyAnnot",
       {SerializableRecord::FieldSet(
           {{FieldId(1), SerializableRecord::Int32(42)}})}}};

  // @MyAnnot{field1=42}
  // struct MyStruct{
  //   @MyAnnot{field1=42}
  //   1: optional i32 field1;
  // }
  builder.addType(
      "meta.com/thrift/test/MyStruct",
      def::Struct(
          {def::Field(
              def::Identity(1, "field1"),
              def::Optional,
              TypeIds::I32,
              std::nullopt,
              annots)},
          false,
          annots));
  EXPECT_THROW(std::move(builder).build(), InvalidTypeError);
}

TEST(TypeSystemTest, UnionFieldsMustBeOptional) {
  TypeSystemBuilder builder;

  // Attempt to add a union with a non-optional field
  EXPECT_THROW(
      builder.addType(
          "meta.com/thrift/test/InvalidUnion",
          def::Union({
              def::Field(
                  def::Identity(1, "nonOptionalField"),
                  def::AlwaysPresent,
                  TypeIds::I32),
          })),
      InvalidTypeError);
}

TEST(TypeSystemTest, FieldIdentitiesMustBeUnique) {
  TypeSystemBuilder builder;

  // Attempt to add a struct with duplicate field ids
  EXPECT_THROW(
      builder.addType(
          "meta.com/thrift/test/DuplicateFieldIdStruct",
          def::Struct({
              def::Field(
                  def::Identity(1, "field1"), def::Optional, TypeIds::I32),
              def::Field(
                  def::Identity(1, "field2"), def::Optional, TypeIds::I64),
          })),
      InvalidTypeError);

  // Attempt to add a struct with duplicate field names
  EXPECT_THROW(
      builder.addType(
          "meta.com/thrift/test/DuplicateFieldNameStruct",
          def::Struct({
              def::Field(
                  def::Identity(1, "field1"), def::Optional, TypeIds::I32),
              def::Field(
                  def::Identity(2, "field1"), def::Optional, TypeIds::I64),
          })),
      InvalidTypeError);
}

TEST(TypeSystemTest, OpaqueAliasMustNotBeUserDefinedType) {
  TypeSystemBuilder builder;

  // Attempt to add an opaque alias with a user-defined type as target
  EXPECT_THROW(
      builder.addType(
          "meta.com/thrift/test/InvalidOpaqueAlias",
          def::OpaqueAlias(TypeIds::uri("meta.com/thrift/test/MyStruct"))),
      InvalidTypeError);
}

TEST(TypeSystemTest, EnumMappingsMustBeUnique) {
  TypeSystemBuilder builder;

  // Attempt to add an enum with duplicate names
  EXPECT_THROW(
      builder.addType(
          "meta.com/thrift/test/DuplicateEnumName",
          def::Enum({{"name1", 1}, {"name1", 2}})),
      InvalidTypeError);

  // Attempt to add an enum with duplicate values
  EXPECT_THROW(
      builder.addType(
          "meta.com/thrift/test/DuplicateEnumValue",
          def::Enum({{"name1", 1}, {"name2", 1}})),
      InvalidTypeError);
}

TEST(TypeSystemTest, ListTypeRef) {
  TypeSystemBuilder builder;

  // Define a struct with a list field
  builder.addType(
      "meta.com/thrift/test/ListStruct",
      def::Struct({
          def::Field(
              def::Identity(1, "listField"),
              def::Optional,
              TypeIds::list(TypeIds::I32)),
      }));

  auto typeSystem = std::move(builder).build();

  // Check the struct with the list field
  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/ListStruct"));
  EXPECT_EQ(def.uri(), "meta.com/thrift/test/ListStruct");
  EXPECT_EQ(def.asStruct().fields().size(), 1);
  const FieldDefinition& listField = def.asStruct().fields()[0];
  EXPECT_EQ(listField.identity(), def::Identity(1, "listField"));
  EXPECT_EQ(listField.presence(), def::Optional);
  EXPECT_TRUE(listField.type().isList());
  EXPECT_EQ(listField.type().asList().id(), TypeIds::list(TypeIds::I32));
  EXPECT_EQ(listField.type().id(), TypeRef::List::of(TypeRef::I32()).id());

  auto stsBuilder =
      SerializableTypeSystemBuilder::withoutSourceInfo(*typeSystem);
  stsBuilder.addDefinition("meta.com/thrift/test/ListStruct");
  auto sts = *std::move(stsBuilder).build();
  EXPECT_EQ(sts.types()->size(), 1);
  auto& listStructDef = *sts.types()
                             ->at("meta.com/thrift/test/ListStruct")
                             .definition()
                             ->structDef();
  EXPECT_EQ(listStructDef.fields()->size(), 1);
  EXPECT_EQ(listStructDef.fields()[0].identity()->id(), FieldId{1});
  EXPECT_EQ(listStructDef.fields()[0].identity()->name(), "listField");
  EXPECT_EQ(listStructDef.fields()[0].presence(), PresenceQualifier::OPTIONAL_);
  EXPECT_EQ(listStructDef.fields()[0].type(), TypeIds::list(TypeIds::I32));
}

TEST(TypeSystemTest, SetTypeRef) {
  TypeSystemBuilder builder;

  // Define a struct with a set field
  builder.addType(
      "meta.com/thrift/test/SetStruct",
      def::Struct({
          def::Field(
              def::Identity(1, "setField"),
              def::Optional,
              TypeIds::set(TypeIds::I32)),
      }));

  auto typeSystem = std::move(builder).build();

  // Check the struct with the set field
  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/SetStruct"));
  EXPECT_EQ(def.uri(), "meta.com/thrift/test/SetStruct");
  EXPECT_EQ(def.asStruct().fields().size(), 1);
  const FieldDefinition& setField = def.asStruct().fields()[0];
  EXPECT_EQ(setField.identity(), def::Identity(1, "setField"));
  EXPECT_EQ(setField.presence(), def::Optional);
  EXPECT_TRUE(setField.type().isSet());
  EXPECT_EQ(setField.type().asSet().id(), TypeIds::set(TypeIds::I32));
  EXPECT_EQ(setField.type().id(), TypeRef::Set::of(TypeRef::I32()).id());

  auto stsBuilder =
      SerializableTypeSystemBuilder::withoutSourceInfo(*typeSystem);
  stsBuilder.addDefinition("meta.com/thrift/test/SetStruct");
  auto sts = *std::move(stsBuilder).build();
  EXPECT_EQ(sts.types()->size(), 1);
  auto& setStructDef = *sts.types()
                            ->at("meta.com/thrift/test/SetStruct")
                            .definition()
                            ->structDef();
  EXPECT_EQ(setStructDef.fields()->size(), 1);
  EXPECT_EQ(setStructDef.fields()[0].identity()->id(), FieldId{1});
  EXPECT_EQ(setStructDef.fields()[0].identity()->name(), "setField");
  EXPECT_EQ(setStructDef.fields()[0].presence(), PresenceQualifier::OPTIONAL_);
  EXPECT_EQ(setStructDef.fields()[0].type(), TypeIds::set(TypeIds::I32));
}

TEST(TypeSystemTest, MapTypeRef) {
  TypeSystemBuilder builder;

  // Define a struct with a map field
  builder.addType(
      "meta.com/thrift/test/MapStruct",
      def::Struct({
          def::Field(
              def::Identity(1, "mapField"),
              def::Optional,
              TypeIds::map(TypeIds::I32, TypeIds::String)),
      }));

  auto typeSystem = std::move(builder).build();

  // Check the struct with the map field
  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/MapStruct"));
  EXPECT_EQ(def.uri(), "meta.com/thrift/test/MapStruct");
  EXPECT_EQ(def.asStruct().fields().size(), 1);
  const FieldDefinition& mapField = def.asStruct().fields()[0];
  EXPECT_EQ(mapField.identity(), def::Identity(1, "mapField"));
  EXPECT_EQ(mapField.presence(), def::Optional);
  EXPECT_TRUE(mapField.type().isMap());
  EXPECT_EQ(
      mapField.type().asMap().id(),
      TypeIds::map(TypeIds::I32, TypeIds::String));
  EXPECT_EQ(
      mapField.type().id(),
      TypeRef::Map::of(TypeRef::I32(), TypeRef::String()).id());

  auto stsBuilder =
      SerializableTypeSystemBuilder::withoutSourceInfo(*typeSystem);
  stsBuilder.addDefinition("meta.com/thrift/test/MapStruct");
  auto sts = *std::move(stsBuilder).build();

  EXPECT_EQ(sts.types()->size(), 1);
  auto& mapStructDef = *sts.types()
                            ->at("meta.com/thrift/test/MapStruct")
                            .definition()
                            ->structDef();
  EXPECT_EQ(mapStructDef.fields()->size(), 1);
  EXPECT_EQ(mapStructDef.fields()[0].identity()->id(), FieldId{1});
  EXPECT_EQ(mapStructDef.fields()[0].identity()->name(), "mapField");
  EXPECT_EQ(mapStructDef.fields()[0].presence(), PresenceQualifier::OPTIONAL_);
  EXPECT_EQ(
      mapStructDef.fields()[0].type(),
      TypeIds::map(TypeIds::I32, TypeIds::String));
}

TEST(TypeSystemTest, OpaqueAliasTypeRef) {
  TypeSystemBuilder builder;

  // Define an opaque alias
  builder.addType(
      "meta.com/thrift/test/OpaqueAlias", def::OpaqueAlias(TypeIds::I32));

  auto typeSystem = std::move(builder).build();

  // Check the opaque alias
  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/OpaqueAlias"));
  EXPECT_EQ(def.uri(), "meta.com/thrift/test/OpaqueAlias");
  EXPECT_TRUE(def.isOpaqueAlias());
  EXPECT_EQ(def.asOpaqueAlias().targetType().id(), TypeIds::I32);

  auto stsBuilder =
      SerializableTypeSystemBuilder::withoutSourceInfo(*typeSystem);
  stsBuilder.addDefinition("meta.com/thrift/test/OpaqueAlias");
  auto sts = *std::move(stsBuilder).build();
  EXPECT_EQ(sts.types()->size(), 1);
  auto& opaqueAliasDef = *sts.types()
                              ->at("meta.com/thrift/test/OpaqueAlias")
                              .definition()
                              ->opaqueAliasDef();
  EXPECT_EQ(opaqueAliasDef.targetType(), TypeIds::I32);
}

TEST(TypeSystemTest, EnumTypeRef) {
  TypeSystemBuilder builder;

  // Define an enum
  builder.addType(
      "meta.com/thrift/test/SimpleEnum",
      def::Enum({{"VALUE1", 1}, {"VALUE2", 2}}));

  auto typeSystem = std::move(builder).build();

  // Check the enum
  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/SimpleEnum"));
  EXPECT_EQ(def.uri(), "meta.com/thrift/test/SimpleEnum");
  EXPECT_TRUE(def.isEnum());
  EXPECT_EQ(def.asEnum().values().size(), 2);
  EXPECT_EQ(def.asEnum().values()[0].name, "VALUE1");
  EXPECT_EQ(def.asEnum().values()[0].i32, 1);
  EXPECT_EQ(def.asEnum().values()[1].name, "VALUE2");
  EXPECT_EQ(def.asEnum().values()[1].i32, 2);

  auto stsBuilder =
      SerializableTypeSystemBuilder::withoutSourceInfo(*typeSystem);
  stsBuilder.addDefinition("meta.com/thrift/test/SimpleEnum");
  auto sts = *std::move(stsBuilder).build();
  EXPECT_EQ(sts.types()->size(), 1);
  auto& enumDef = *sts.types()
                       ->at("meta.com/thrift/test/SimpleEnum")
                       .definition()
                       ->enumDef();
  EXPECT_EQ(enumDef.values()->size(), 2);
  EXPECT_EQ(enumDef.values()[0].name(), "VALUE1");
  EXPECT_EQ(enumDef.values()[0].datum(), 1);
  EXPECT_EQ(enumDef.values()[1].name(), "VALUE2");
  EXPECT_EQ(enumDef.values()[1].datum(), 2);
}

TEST(TypeSystemTest, ComplexTypeReferences) {
  TypeSystemBuilder builder;

  // Define a struct with complex type references
  builder.addType(
      "meta.com/thrift/test/ComplexStruct",
      def::Struct({
          def::Field(
              def::Identity(1, "listOfStructs"),
              def::Optional,
              TypeIds::list(TypeIds::uri("meta.com/thrift/test/SimpleStruct"))),
          def::Field(
              def::Identity(2, "mapOfUnions"),
              def::Optional,
              TypeIds::map(
                  TypeIds::String,
                  TypeIds::uri("meta.com/thrift/test/SimpleUnion"))),
      }));
  builder.addType(
      "meta.com/thrift/test/SimpleStruct",
      def::Struct({
          def::Field(def::Identity(1, "field1"), def::Optional, TypeIds::I32),
      }));
  builder.addType(
      "meta.com/thrift/test/SimpleUnion",
      def::Union({
          def::Field(def::Identity(1, "field1"), def::Optional, TypeIds::I64),
      }));

  auto typeSystem = std::move(builder).build();

  // Check the complex struct
  DefinitionRef complexDef = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/ComplexStruct"));
  EXPECT_EQ(complexDef.uri(), "meta.com/thrift/test/ComplexStruct");
  EXPECT_EQ(complexDef.asStruct().fields().size(), 2);

  const FieldDefinition& listField = complexDef.asStruct().fields()[0];
  EXPECT_EQ(listField.identity(), def::Identity(1, "listOfStructs"));
  EXPECT_EQ(listField.presence(), def::Optional);
  EXPECT_TRUE(listField.type().isList());
  EXPECT_EQ(
      listField.type().asList().elementType().asStruct().fields().size(), 1);

  const FieldDefinition& mapField = complexDef.asStruct().fields()[1];
  EXPECT_EQ(mapField.identity(), def::Identity(2, "mapOfUnions"));
  EXPECT_EQ(mapField.presence(), def::Optional);
  EXPECT_TRUE(mapField.type().isMap());
  EXPECT_EQ(mapField.type().asMap().keyType().id(), TypeIds::String);
  EXPECT_EQ(mapField.type().asMap().valueType().asUnion().fields().size(), 1);

  auto stsBuilder =
      SerializableTypeSystemBuilder::withoutSourceInfo(*typeSystem);
  stsBuilder.addDefinition("meta.com/thrift/test/ComplexStruct");
  auto sts = *std::move(stsBuilder).build();
  EXPECT_EQ(sts.types()->size(), 3);
  auto& complexStructDef = *sts.types()
                                ->at("meta.com/thrift/test/ComplexStruct")
                                .definition()
                                ->structDef();
  EXPECT_EQ(complexStructDef.fields()->size(), 2);
  EXPECT_EQ(complexStructDef.fields()[0].identity()->id(), FieldId{1});
  EXPECT_EQ(complexStructDef.fields()[0].identity()->name(), "listOfStructs");
  EXPECT_EQ(
      complexStructDef.fields()[0].presence(), PresenceQualifier::OPTIONAL_);
  EXPECT_EQ(
      complexStructDef.fields()[0].type(),
      TypeIds::list(TypeIds::uri("meta.com/thrift/test/SimpleStruct")));
  EXPECT_EQ(complexStructDef.fields()[1].identity()->id(), FieldId{2});
  EXPECT_EQ(complexStructDef.fields()[1].identity()->name(), "mapOfUnions");
  EXPECT_EQ(
      complexStructDef.fields()[1].presence(), PresenceQualifier::OPTIONAL_);
  EXPECT_EQ(
      complexStructDef.fields()[1].type(),
      TypeIds::map(
          TypeIds::String, TypeIds::uri("meta.com/thrift/test/SimpleUnion")));
}

TEST(TypeSystemTest, NestedContainers) {
  TypeSystemBuilder builder;

  // Define a struct with nested containers
  builder.addType(
      "meta.com/thrift/test/NestedContainerStruct",
      def::Struct({
          def::Field(
              def::Identity(1, "listOfMaps"),
              def::Optional,
              TypeIds::list(TypeIds::map(TypeIds::String, TypeIds::I32))),
          def::Field(
              def::Identity(2, "setOfLists"),
              def::Optional,
              TypeIds::set(TypeIds::list(TypeIds::I64))),
      }));

  auto typeSystem = std::move(builder).build();

  // Check the struct with nested containers
  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/NestedContainerStruct"));
  EXPECT_EQ(def.uri(), "meta.com/thrift/test/NestedContainerStruct");
  EXPECT_EQ(def.asStruct().fields().size(), 2);

  const FieldDefinition& listOfMapsField = def.asStruct().fields()[0];
  EXPECT_EQ(listOfMapsField.identity(), def::Identity(1, "listOfMaps"));
  EXPECT_EQ(listOfMapsField.presence(), def::Optional);
  EXPECT_TRUE(listOfMapsField.type().isList());
  EXPECT_TRUE(listOfMapsField.type().asList().elementType().isMap());
  EXPECT_EQ(
      listOfMapsField.type().asList().elementType().asMap().keyType().id(),
      TypeIds::String);
  EXPECT_EQ(
      listOfMapsField.type().asList().elementType().asMap().valueType().id(),
      TypeIds::I32);

  const FieldDefinition& setOfListsField = def.asStruct().fields()[1];
  EXPECT_EQ(setOfListsField.identity(), def::Identity(2, "setOfLists"));
  EXPECT_EQ(setOfListsField.presence(), def::Optional);
  EXPECT_TRUE(setOfListsField.type().isSet());
  EXPECT_TRUE(setOfListsField.type().asSet().elementType().isList());
  EXPECT_EQ(
      setOfListsField.type().asSet().elementType().asList().elementType().id(),
      TypeIds::I64);

  auto stsBuilder =
      SerializableTypeSystemBuilder::withoutSourceInfo(*typeSystem);
  stsBuilder.addDefinition("meta.com/thrift/test/NestedContainerStruct");
  auto sts = *std::move(stsBuilder).build();
  EXPECT_EQ(sts.types()->size(), 1);
  auto& nestedContainerStructDef =
      *sts.types()
           ->at("meta.com/thrift/test/NestedContainerStruct")
           .definition()
           ->structDef();
  EXPECT_EQ(nestedContainerStructDef.fields()->size(), 2);
  EXPECT_EQ(nestedContainerStructDef.fields()[0].identity()->id(), FieldId{1});
  EXPECT_EQ(
      nestedContainerStructDef.fields()[0].identity()->name(), "listOfMaps");
  EXPECT_EQ(
      nestedContainerStructDef.fields()[0].presence(),
      PresenceQualifier::OPTIONAL_);
  EXPECT_EQ(
      nestedContainerStructDef.fields()[0].type(),
      TypeIds::list(TypeIds::map(TypeIds::String, TypeIds::I32)));
  EXPECT_EQ(nestedContainerStructDef.fields()[1].identity()->id(), FieldId{2});
  EXPECT_EQ(
      nestedContainerStructDef.fields()[1].identity()->name(), "setOfLists");
  EXPECT_EQ(
      nestedContainerStructDef.fields()[1].presence(),
      PresenceQualifier::OPTIONAL_);
  EXPECT_EQ(
      nestedContainerStructDef.fields()[1].type(),
      TypeIds::set(TypeIds::list(TypeIds::I64)));
}

TEST(TypeSystemTest, StructWithNegativeFieldId) {
  TypeSystemBuilder builder;

  // Define a struct with the maximum field id
  builder.addType(
      "meta.com/thrift/test/NegativeFieldId",
      def::Struct({
          def::Field(
              def::Identity(-1, "negativeFieldId"),
              def::AlwaysPresent,
              TypeIds::I32),
      }));

  auto typeSystem = std::move(builder).build();

  // Check the struct with the maximum field id
  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/NegativeFieldId"));
  EXPECT_EQ(def.uri(), "meta.com/thrift/test/NegativeFieldId");
  EXPECT_EQ(def.asStruct().fields().size(), 1);
  const FieldDefinition& negativeFieldId = def.asStruct().fields()[0];
  EXPECT_EQ(negativeFieldId.identity().id(), FieldId(-1));
  EXPECT_EQ(negativeFieldId.identity().name(), "negativeFieldId");
  EXPECT_EQ(negativeFieldId.presence(), def::AlwaysPresent);
  EXPECT_EQ(negativeFieldId.type().id(), TypeIds::I32);
}

TEST(TypeSystemTest, EnumWithNegativeValues) {
  TypeSystemBuilder builder;

  // Define an enum with negative values
  builder.addType(
      "meta.com/thrift/test/NegativeEnum",
      def::Enum({{"NEGATIVE_ONE", -1}, {"NEGATIVE_TWO", -2}}));

  auto typeSystem = std::move(builder).build();

  // Check the enum with negative values
  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/NegativeEnum"));
  EXPECT_EQ(def.uri(), "meta.com/thrift/test/NegativeEnum");
  EXPECT_EQ(def.asEnum().values().size(), 2);
  EXPECT_EQ(def.asEnum().values()[0].name, "NEGATIVE_ONE");
  EXPECT_EQ(def.asEnum().values()[0].i32, -1);
  EXPECT_EQ(def.asEnum().values()[1].name, "NEGATIVE_TWO");
  EXPECT_EQ(def.asEnum().values()[1].i32, -2);
}

TEST(TypeSystemTest, TypeRefIsEqualIdentityTo) {
  const auto makeTypeSystem = []() -> std::unique_ptr<TypeSystem> {
    TypeSystemBuilder builder;

    builder.addType(
        "meta.com/thrift/test/OuterStruct",
        def::Struct({
            def::Field(
                def::Identity(1, "innerStruct"),
                def::Optional,
                TypeIds::uri("meta.com/thrift/test/InnerStruct")),
        }));
    builder.addType(
        "meta.com/thrift/test/InnerStruct",
        def::Struct({
            def::Field(def::Identity(1, "field1"), def::Optional, TypeIds::I32),
        }));
    builder.addType(
        "meta.com/thrift/test/Alias",
        def::OpaqueAlias(
            TypeIds::list(TypeIds::uri("meta.com/thrift/test/OuterStruct"))));

    return std::move(builder).build();
  };

  auto ts1 = makeTypeSystem();
  auto ts2 = makeTypeSystem();

  {
    auto lhs = TypeRef::fromDefinition(
        ts1->getUserDefinedTypeOrThrow("meta.com/thrift/test/Alias"));
    auto rhs = TypeRef::fromDefinition(
        ts2->getUserDefinedTypeOrThrow("meta.com/thrift/test/Alias"));

    EXPECT_TRUE(lhs.isEqualIdentityTo(lhs));
    EXPECT_TRUE(lhs.isEqualIdentityTo(rhs));
  }

  {
    auto lhs = TypeRef::fromDefinition(
        ts1->getUserDefinedTypeOrThrow("meta.com/thrift/test/OuterStruct"));
    auto rhs = TypeRef::fromDefinition(
        ts2->getUserDefinedTypeOrThrow("meta.com/thrift/test/OuterStruct"));

    EXPECT_TRUE(lhs.isEqualIdentityTo(lhs));
    EXPECT_TRUE(lhs.isEqualIdentityTo(rhs));
  }

  {
    auto lhs = TypeRef::fromDefinition(
        ts1->getUserDefinedTypeOrThrow("meta.com/thrift/test/OuterStruct"));
    auto rhs = TypeRef::fromDefinition(
        ts1->getUserDefinedTypeOrThrow("meta.com/thrift/test/Alias"));

    EXPECT_FALSE(lhs.isEqualIdentityTo(rhs));
    EXPECT_FALSE(lhs.isEqualIdentityTo(rhs));
  }
}

TEST(TypeSystemTest, ToTType) {
  EXPECT_EQ(ToTTypeFn{}(TypeRef::Bool()), TType::T_BOOL);
  EXPECT_EQ(ToTTypeFn{}(TypeRef::Byte()), TType::T_BYTE);
  EXPECT_EQ(ToTTypeFn{}(TypeRef::I16()), TType::T_I16);
  EXPECT_EQ(ToTTypeFn{}(TypeRef::I32()), TType::T_I32);
  EXPECT_EQ(ToTTypeFn{}(TypeRef::I64()), TType::T_I64);
  EXPECT_EQ(ToTTypeFn{}(TypeRef::Double()), TType::T_DOUBLE);
  EXPECT_EQ(ToTTypeFn{}(TypeRef::Float()), TType::T_FLOAT);
  EXPECT_EQ(ToTTypeFn{}(TypeRef::String()), TType::T_STRING);
  EXPECT_EQ(ToTTypeFn{}(TypeRef::Binary()), TType::T_STRING);

  EXPECT_EQ(ToTTypeFn{}(TypeRef::List::of(TypeRef::I32())), TType::T_LIST);
  EXPECT_EQ(ToTTypeFn{}(TypeRef::List::of(TypeRef::String())), TType::T_LIST);

  EXPECT_EQ(ToTTypeFn{}(TypeRef::Set::of(TypeRef::I32())), TType::T_SET);
  EXPECT_EQ(ToTTypeFn{}(TypeRef::Set::of(TypeRef::Any())), TType::T_SET);

  EXPECT_EQ(
      ToTTypeFn{}(TypeRef::Map::of(TypeRef::I32(), TypeRef::I32())),
      TType::T_MAP);
  EXPECT_EQ(
      ToTTypeFn{}(TypeRef::Map::of(TypeRef::I32(), TypeRef::String())),
      TType::T_MAP);

  auto typeSystem = typeSystemWithEmpties();

  EXPECT_EQ(
      ToTTypeFn{}(typeSystem
                      ->getUserDefinedTypeOrThrow(
                          Uri("meta.com/thrift/test/EmptyStruct"))
                      .asStruct()),
      TType::T_STRUCT);
  EXPECT_EQ(
      ToTTypeFn{}(typeSystem
                      ->getUserDefinedTypeOrThrow(
                          Uri("meta.com/thrift/test/EmptyUnion"))
                      .asUnion()),
      TType::T_STRUCT);
  EXPECT_EQ(
      ToTTypeFn{}(
          typeSystem
              ->getUserDefinedTypeOrThrow(Uri("meta.com/thrift/test/EmptyEnum"))
              .asEnum()),
      TType::T_I32);
}

TEST(TypeSystemTest, TypeResolution) {
  auto ts = typeSystemWithEmpties();

  auto echoTest = [&](const auto& typeId) {
    SCOPED_TRACE(fmt::format("echoTest({})", typeId));
    EXPECT_EQ(typeId, ts->resolveTypeId(typeId).id());
  };
  auto echoTests = [&](const auto&... typeIds) { (echoTest(typeIds), ...); };

  echoTests(
      TypeIds::Bool,
      TypeIds::Byte,
      TypeIds::I16,
      TypeIds::I32,
      TypeIds::I64,
      TypeIds::Double,
      TypeIds::Float,
      TypeIds::String,
      TypeIds::Binary,
      TypeIds::Any,
      TypeIds::uri(kEmptyStructUri),
      TypeIds::uri(kEmptyUnionUri),
      TypeIds::uri(kEmptyEnumUri),
      TypeIds::list(TypeIds::I32),
      TypeIds::list(TypeIds::uri(kEmptyStructUri)),
      TypeIds::set(TypeIds::I64),
      TypeIds::set(TypeIds::uri(kEmptyStructUri)),
      TypeIds::map(TypeIds::I32, TypeIds::uri(kEmptyEnumUri)));
}

TEST(TypeSystemTest, TagResolution) {
  auto ts = typeSystemWithEmpties();

  auto testFn = [&](const auto& expectedRef, auto tag) {
    EXPECT_TRUE(TypeRef(expectedRef).isEqualIdentityTo(resolveTag(*ts, tag)));
  };

  testFn(TypeRef::Bool{}, type::bool_t{});
  testFn(TypeRef::Byte{}, type::byte_t{});
  testFn(TypeRef::I16{}, type::i16_t{});
  testFn(TypeRef::I32{}, type::i32_t{});
  testFn(TypeRef::I64{}, type::i64_t{});
  testFn(TypeRef::Double{}, type::double_t{});
  testFn(TypeRef::Float{}, type::float_t{});
  testFn(TypeRef::String{}, type::string_t{});
  testFn(TypeRef::Binary{}, type::binary_t{});

  testFn(TypeRef::List::of(TypeRef::I32{}), type::list<type::i32_t>{});
  testFn(TypeRef::Set::of(TypeRef::I64{}), type::set<type::i64_t>{});
  testFn(
      TypeRef::Map::of(TypeRef::I32{}, TypeRef::String{}),
      type::map<type::i32_t, type::string_t>{});
}
TEST(TypeSystemTest, SourceIndexedTypeSystem) {
  TypeSystemBuilder builder;

  builder.addType(
      "meta.com/thrift/test/StructWithI32Field",
      def::Struct({
          def::Field(def::Identity(1, "field1"), def::Optional, TypeIds::I32),
      }),
      def::SourceInfo("file://foo/bar.thrift", "StructWithI32Field"));

  builder.addType(
      "meta.com/thrift/test/Enum",
      def::Enum({{"VALUE1", 1}, {"VALUE2", 2}}),
      def::SourceInfo("file://foo/bar.thrift", "Enum"));

  builder.addType(
      "meta.com/thrift/test/UnionWithI32Field",
      def::Union({
          def::Field(def::Identity(1, "field1"), def::Optional, TypeIds::I32),
      }),
      def::SourceInfo("file://foo/other.thrift", "UnionWithI32Field"));

  builder.addType(
      "meta.com/thrift/test/OpaqueAlias",
      def::OpaqueAlias(TypeIds::I32),
      def::SourceInfo("file://foo/other.thrift", "OpaqueAlias"));

  auto typeSystem = std::move(builder).build();
  const auto& sym = dynamic_cast<const SourceIndexedTypeSystem&>(*typeSystem);

  EXPECT_EQ(
      &typeSystem->getUserDefinedType("meta.com/thrift/test/StructWithI32Field")
           ->asStruct(),
      &sym.getUserDefinedTypeBySourceIdentifier(
              {"file://foo/bar.thrift", "StructWithI32Field"})
           ->asStruct());

  EXPECT_EQ(
      sym.getUserDefinedTypeBySourceIdentifier(
          {"file://does-not-exist.thrift", "StructWithI32Field"}),
      std::nullopt);
  EXPECT_EQ(
      sym.getUserDefinedTypeBySourceIdentifier(
          {"file://foo/bar.thrift", "DoesNotExist"}),
      std::nullopt);

  {
    auto types = sym.getUserDefinedTypesAtLocation("file://foo/bar.thrift");
    EXPECT_EQ(types.size(), 2);
    EXPECT_EQ(
        &types.find("StructWithI32Field")->second.asStruct(),
        &typeSystem
             ->getUserDefinedType("meta.com/thrift/test/StructWithI32Field")
             ->asStruct());
    EXPECT_EQ(
        &types.find("Enum")->second.asEnum(),
        &typeSystem->getUserDefinedType("meta.com/thrift/test/Enum")->asEnum());
  }

  {
    auto types = sym.getUserDefinedTypesAtLocation("file://foo/other.thrift");
    EXPECT_EQ(types.size(), 2);
    EXPECT_EQ(
        &types.find("UnionWithI32Field")->second.asUnion(),
        &typeSystem
             ->getUserDefinedType("meta.com/thrift/test/UnionWithI32Field")
             ->asUnion());
    EXPECT_EQ(
        &types.find("OpaqueAlias")->second.asOpaqueAlias(),
        &typeSystem->getUserDefinedType("meta.com/thrift/test/OpaqueAlias")
             ->asOpaqueAlias());
  }

  auto stsBuilder = SerializableTypeSystemBuilder::withSourceInfo(sym);
  stsBuilder.addDefinition("meta.com/thrift/test/StructWithI32Field");
  stsBuilder.addDefinition("meta.com/thrift/test/UnionWithI32Field");
  stsBuilder.addDefinition("meta.com/thrift/test/OpaqueAlias");
  stsBuilder.addDefinition("meta.com/thrift/test/Enum");
  auto sts = *std::move(stsBuilder).build();
  EXPECT_EQ(sts.types()->size(), 4);
  auto& structWithI32FieldEntry =
      sts.types()->at("meta.com/thrift/test/StructWithI32Field");
  EXPECT_EQ(
      structWithI32FieldEntry.sourceInfo()->locator(), "file://foo/bar.thrift");
  EXPECT_EQ(structWithI32FieldEntry.sourceInfo()->name(), "StructWithI32Field");

  auto& enumEntry = sts.types()->at("meta.com/thrift/test/Enum");
  EXPECT_EQ(enumEntry.sourceInfo()->locator(), "file://foo/bar.thrift");
  EXPECT_EQ(enumEntry.sourceInfo()->name(), "Enum");

  auto& unionWithI32FieldEntry =
      sts.types()->at("meta.com/thrift/test/UnionWithI32Field");
  EXPECT_EQ(
      unionWithI32FieldEntry.sourceInfo()->locator(),
      "file://foo/other.thrift");
  EXPECT_EQ(unionWithI32FieldEntry.sourceInfo()->name(), "UnionWithI32Field");

  auto& opaqueAliasEntry = sts.types()->at("meta.com/thrift/test/OpaqueAlias");
  EXPECT_EQ(
      opaqueAliasEntry.sourceInfo()->locator(), "file://foo/other.thrift");
  EXPECT_EQ(opaqueAliasEntry.sourceInfo()->name(), "OpaqueAlias");
}

TEST(TypeSystemTest, SourceIndexedTypeSystemWithDuplicateEntries) {
  TypeSystemBuilder builder;

  builder.addType(
      "meta.com/thrift/test/StructWithI32Field",
      def::Struct({
          def::Field(def::Identity(1, "field1"), def::Optional, TypeIds::I32),
      }),
      def::SourceInfo("file://foo/bar.thrift", "StructWithI32Field"));

  builder.addType(
      "meta.com/thrift/test/StructWithI32Field2",
      def::Struct({
          def::Field(def::Identity(1, "field1"), def::Optional, TypeIds::I32),
      }),
      def::SourceInfo("file://foo/bar.thrift", "StructWithI32Field"));

  EXPECT_THAT(
      [&] { std::move(builder).build(); },
      testing::ThrowsMessage<InvalidTypeError>(
          testing::HasSubstr("Duplicate source identifier")));
}

TEST(TypeSystemTest, SourceIndexedTypeSystemLookupByDefinition) {
  TypeSystemBuilder builder;

  builder.addType(
      "meta.com/thrift/test/StructWithI32Field",
      def::Struct({
          def::Field(def::Identity(1, "field1"), def::Optional, TypeIds::I32),
      }),
      def::SourceInfo("file://foo/bar.thrift", "StructWithI32Field"));

  builder.addType(
      "meta.com/thrift/test/Enum",
      def::Enum({{"VALUE1", 1}, {"VALUE2", 2}}),
      def::SourceInfo("file://foo/bar.thrift", "Enum"));

  builder.addType(
      "meta.com/thrift/test/UnionWithI32Field",
      def::Union({
          def::Field(def::Identity(1, "field1"), def::Optional, TypeIds::I32),
      }),
      std::nullopt /* sourceInfo */);

  auto typeSystem = std::move(builder).build();
  const auto& sym = dynamic_cast<const SourceIndexedTypeSystem&>(*typeSystem);

  const StructNode& structWithI32FieldDefinition =
      sym.getUserDefinedTypeBySourceIdentifier(
             {"file://foo/bar.thrift", "StructWithI32Field"})
          ->asStruct();
  EXPECT_EQ(
      sym.getSourceIdentiferForUserDefinedType(
             DefinitionRef(&structWithI32FieldDefinition))
          .value(),
      (SourceIdentifier{"file://foo/bar.thrift", "StructWithI32Field"}));

  const EnumNode& enumNode =
      sym.getUserDefinedType("meta.com/thrift/test/Enum")->asEnum();
  EXPECT_EQ(
      sym.getSourceIdentiferForUserDefinedType(DefinitionRef(&enumNode))
          .value(),
      (SourceIdentifier{"file://foo/bar.thrift", "Enum"}));

  const UnionNode& unionWithI32FieldDefinition =
      sym.getUserDefinedType("meta.com/thrift/test/UnionWithI32Field")
          ->asUnion();
  EXPECT_EQ(
      sym.getSourceIdentiferForUserDefinedType(
          DefinitionRef(&unionWithI32FieldDefinition)),
      std::nullopt);
}

} // namespace apache::thrift::type_system
