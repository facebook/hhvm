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

#include <thrift/lib/cpp2/dynamic/TypeId.h>
#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemTraits.h>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace apache::thrift::type_system {

// For conciseness
namespace {

constexpr inline PresenceQualifier optional = PresenceQualifier::OPTIONAL_;
constexpr inline PresenceQualifier unqualified = PresenceQualifier::UNQUALIFIED;
using RawAnnotationsMap = folly::F14FastMap<Uri, SerializableRecordUnion>;

FieldIdentity identity(std::int16_t id, std::string_view name) {
  return FieldIdentity{FieldId{id}, std::string(name)};
}

SerializableFieldDefinition makeField(
    FieldIdentity identity,
    PresenceQualifier presence,
    TypeId type,
    std::optional<SerializableRecord> customDefault = std::nullopt,
    RawAnnotationsMap annotations = {}) {
  SerializableFieldDefinition def;
  def.identity() = std::move(identity);
  def.presence() = presence;
  def.type() = type;
  if (customDefault.has_value()) {
    def.customDefaultValue() = SerializableRecord::toThrift(*customDefault);
  }
  def.annotations() = std::move(annotations);
  return def;
}

SerializableStructDefinition makeStruct(
    std::vector<SerializableFieldDefinition> fields,
    bool isSealed = false,
    RawAnnotationsMap annotations = {}) {
  SerializableStructDefinition def;
  def.fields() = fields;
  def.isSealed() = isSealed;
  def.annotations() = std::move(annotations);
  return def;
}

SerializableUnionDefinition makeUnion(
    std::vector<SerializableFieldDefinition> fields,
    bool isSealed = false,
    RawAnnotationsMap annotations = {}) {
  SerializableUnionDefinition def;
  def.fields() = fields;
  def.isSealed() = isSealed;
  def.annotations() = std::move(annotations);
  return def;
}

struct EnumValue {
  std::string name;
  std::int32_t value;
  RawAnnotationsMap annotations;

  EnumValue(std::string n, std::int32_t v) : name{std::move(n)}, value{v} {}
  EnumValue(std::string n, std::int32_t v, RawAnnotationsMap m)
      : name{std::move(n)}, value{v}, annotations(std::move(m)) {}
};

SerializableEnumDefinition makeEnum(
    std::vector<EnumValue> values, RawAnnotationsMap annotations = {}) {
  SerializableEnumDefinition enumDef;
  for (auto& [name, value, annotations] : values) {
    SerializableEnumValueDefinition v;
    v.name() = std::move(name);
    v.datum() = value;
    v.annotations() = std::move(annotations);
    enumDef.values()->emplace_back(std::move(v));
  }
  enumDef.annotations() = std::move(annotations);
  return enumDef;
}

SerializableOpaqueAliasDefinition makeOpaqueAlias(
    TypeId targetType, RawAnnotationsMap annotations = {}) {
  SerializableOpaqueAliasDefinition def;
  def.targetType() = targetType;
  def.annotations() = std::move(annotations);
  return def;
}

SerializableThriftSourceInfo makeSourceInfo(
    std::string_view location, std::string_view name) {
  SerializableThriftSourceInfo entry;
  entry.locator() = std::string(location);
  entry.name() = std::string(name);
  return entry;
}

void expectKnownUris(
    const TypeSystem& typeSystem, std::initializer_list<Uri> uris) {
  folly::F14FastSet<Uri> expectedUris{uris.begin(), uris.end()};
  const auto knownUris = typeSystem.getKnownUris();
  EXPECT_EQ(expectedUris, knownUris);
}

constexpr auto kEmptyStructUri = "meta.com/thrift/test/EmptyStruct";
constexpr auto kEmptyUnionUri = "meta.com/thrift/test/EmptyUnion";
constexpr auto kEmptyEnumUri = "meta.com/thrift/test/EmptyEnum";

std::unique_ptr<TypeSystem> typeSystemWithEmpties() {
  TypeSystemBuilder builder;
  builder.addType(kEmptyStructUri, makeStruct({}));
  builder.addType(kEmptyUnionUri, makeUnion({}));
  builder.addType(kEmptyEnumUri, makeEnum({}));

  return std::move(builder).build();
}

} // namespace

// DefinitionRef should be a cheap value type
static_assert(std::is_trivially_copyable_v<DefinitionRef>);

TEST(TypeSystemTest, EmptyStruct) {
  TypeSystemBuilder builder;

  // Define an empty struct
  builder.addType("meta.com/thrift/test/EmptyStruct", makeStruct({}));

  auto typeSystem = std::move(builder).build();

  // Check the empty struct
  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/EmptyStruct"));
  EXPECT_EQ(def.uri(), "meta.com/thrift/test/EmptyStruct");
  EXPECT_EQ(def.asStruct().fields().size(), 0);

  expectKnownUris(*typeSystem, {def.uri()});
}

TEST(TypeSystemTest, EmptyUnion) {
  TypeSystemBuilder builder;

  // Define a union with no fields
  builder.addType("meta.com/thrift/test/EmptyUnion", makeUnion({}));

  auto typeSystem = std::move(builder).build();

  // Check the empty union
  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/EmptyUnion"));
  EXPECT_EQ(def.uri(), "meta.com/thrift/test/EmptyUnion");
  EXPECT_EQ(def.asUnion().fields().size(), 0);

  expectKnownUris(*typeSystem, {def.uri()});
}

TEST(TypeSystemTest, NestedStructs) {
  TypeSystemBuilder builder;

  // Define a nested struct
  Uri outerStructUri = "meta.com/thrift/test/OuterStruct";
  Uri innerStructUri = "meta.com/thrift/test/InnerStruct";

  builder.addType(
      outerStructUri,
      makeStruct({
          makeField(
              identity(1, "innerStruct"),
              optional,
              TypeIds::uri(innerStructUri)),
      }));

  builder.addType(
      innerStructUri,
      makeStruct({
          makeField(identity(1, "field1"), optional, TypeIds::I32),
      }));

  auto typeSystem = std::move(builder).build();

  // Check the outer struct
  DefinitionRef outerDef =
      typeSystem->getUserDefinedTypeOrThrow(outerStructUri);
  EXPECT_EQ(outerDef.uri(), outerStructUri);
  EXPECT_EQ(outerDef.asStruct().fields().size(), 1);
  const FieldNode& innerField = outerDef.asStruct().fields()[0];
  EXPECT_EQ(innerField.identity(), identity(1, "innerStruct"));
  EXPECT_EQ(innerField.presence(), optional);
  EXPECT_EQ(innerField.type().asStruct().fields().size(), 1);
  const FieldNode& field1 = innerField.type().asStruct().fields()[0];
  EXPECT_EQ(field1.identity(), identity(1, "field1"));
  EXPECT_EQ(field1.presence(), optional);
  EXPECT_EQ(field1.type().id(), TypeIds::I32);

  expectKnownUris(*typeSystem, {innerStructUri, outerStructUri});
}

TEST(TypeSystemTest, RecursiveStructAndUnion) {
  TypeSystemBuilder builder;

  Uri recursiveStructUri = "meta.com/thrift/test/RecursiveStruct";
  Uri recursiveUnionUri = "meta.com/thrift/test/RecursiveUnion";

  // Define a recursive struct
  builder.addType(
      recursiveStructUri,
      makeStruct({
          makeField(
              identity(1, "self"), optional, TypeIds::uri(recursiveStructUri)),
      }));

  // Define a recursive union
  builder.addType(
      recursiveUnionUri,
      makeUnion({
          makeField(
              identity(1, "self"), optional, TypeIds::uri(recursiveUnionUri)),
      }));

  auto typeSystem = std::move(builder).build();

  // Check the recursive struct
  DefinitionRef structDef =
      typeSystem->getUserDefinedTypeOrThrow(recursiveStructUri);
  EXPECT_EQ(structDef.uri(), recursiveStructUri);
  EXPECT_EQ(structDef.asStruct().fields().size(), 1);
  const FieldNode& structField = structDef.asStruct().fields()[0];
  EXPECT_EQ(structField.identity(), identity(1, "self"));
  EXPECT_EQ(structField.presence(), optional);
  EXPECT_EQ(
      std::addressof(structField.type().asStruct()),
      std::addressof(structDef.asStruct()));

  // Check the recursive union
  DefinitionRef unionDef =
      typeSystem->getUserDefinedTypeOrThrow(recursiveUnionUri);
  EXPECT_EQ(unionDef.uri(), recursiveUnionUri);
  EXPECT_EQ(unionDef.asUnion().fields().size(), 1);
  const FieldNode& unionField = unionDef.asUnion().fields()[0];
  EXPECT_EQ(unionField.identity(), identity(1, "self"));
  EXPECT_EQ(unionField.presence(), optional);
  EXPECT_EQ(
      std::addressof(unionField.type().asUnion()),
      std::addressof(unionDef.asUnion()));

  expectKnownUris(*typeSystem, {structDef.uri(), unionDef.uri()});
}

TEST(TypeSystemTest, IncompleteTypeSystem) {
  // Missing MyUnion
  TypeSystemBuilder builder;

  builder.addType(
      "meta.com/thrift/test/MyStruct",
      makeStruct({
          makeField(identity(1, "field1"), optional, TypeIds::I32),
          makeField(
              identity(2, "field2"),
              unqualified,
              TypeIds::uri("meta.com/thrift/test/MyUnion")),
      }));
  EXPECT_THROW(std::move(builder).build(), InvalidTypeError);
}

TEST(TypeSystemTest, MutuallyRecursiveStructuredTypes) {
  TypeSystemBuilder builder;

  builder.addType(
      "meta.com/thrift/test/MyStruct",
      makeStruct({
          makeField(identity(1, "field1"), optional, TypeIds::I32),
          makeField(
              identity(2, "field2"),
              unqualified,
              TypeIds::uri("meta.com/thrift/test/MyUnion")),
      }));
  builder.addType(
      "meta.com/thrift/test/MyUnion",
      makeUnion({
          makeField(identity(1, "int64"), optional, TypeIds::I64),
          makeField(
              identity(2, "myStruct"),
              optional,
              TypeIds::uri("meta.com/thrift/test/MyStruct")),
      }));

  auto typeSystem = std::move(builder).build();

  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/MyStruct"));

  EXPECT_EQ(def.uri(), "meta.com/thrift/test/MyStruct");
  EXPECT_EQ(def.asStruct().fields().size(), 2);
  {
    const FieldNode& field1 = def.asStruct().fields()[0];
    EXPECT_EQ(field1.identity(), identity(1, "field1"));
    EXPECT_EQ(field1.presence(), optional);
    EXPECT_EQ(field1.type().id(), TypeIds::I32);
  }
  {
    const FieldNode& field2 = def.asStruct().fields()[1];
    EXPECT_EQ(field2.identity(), identity(2, "field2"));
    EXPECT_EQ(field2.presence(), unqualified);
    EXPECT_EQ(field2.type().id(), TypeIds::uri("meta.com/thrift/test/MyUnion"));
    {
      TypeRef typeRef = field2.type();
      const UnionNode& otherUnion = typeRef.asUnion();
      EXPECT_EQ(otherUnion.fields().size(), 2);
      {
        const FieldNode& int64 = otherUnion.fields()[0];
        EXPECT_EQ(int64.identity(), identity(1, "int64"));
        EXPECT_EQ(int64.presence(), optional);
        EXPECT_EQ(int64.type().id(), TypeIds::I64);
      }
      {
        const FieldNode& myStruct = otherUnion.fields()[1];
        EXPECT_EQ(myStruct.identity(), identity(2, "myStruct"));
        EXPECT_EQ(myStruct.presence(), optional);
        EXPECT_EQ(
            std::addressof(myStruct.type().asStruct()),
            std::addressof(def.asStruct()));
      }
    }
  }

  expectKnownUris(
      *typeSystem,
      {"meta.com/thrift/test/MyStruct", "meta.com/thrift/test/MyUnion"});
}

TEST(TypeSystemTest, CustomDefaultFieldValues) {
  TypeSystemBuilder builder;

  // Define a struct with a custom default field value
  builder.addType(
      "meta.com/thrift/test/StructWithDefaults",
      makeStruct({
          makeField(
              identity(1, "fieldWithDefault"),
              optional,
              TypeIds::I32,
              SerializableRecord::Int32(42)),
          makeField(identity(2, "fieldWithoutDefault"), optional, TypeIds::I64),
      }));

  auto typeSystem = std::move(builder).build();

  // Check the struct with the custom default field value
  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/StructWithDefaults"));
  EXPECT_EQ(def.uri(), "meta.com/thrift/test/StructWithDefaults");
  EXPECT_EQ(def.asStruct().fields().size(), 2);

  const FieldNode& fieldWithDefault = def.asStruct().fields()[0];
  EXPECT_EQ(fieldWithDefault.identity(), identity(1, "fieldWithDefault"));
  EXPECT_EQ(fieldWithDefault.presence(), optional);
  EXPECT_EQ(fieldWithDefault.type().id(), TypeIds::I32);
  EXPECT_NE(fieldWithDefault.customDefault(), nullptr);
  EXPECT_EQ(fieldWithDefault.customDefault()->asInt32(), 42);

  const FieldNode& fieldWithoutDefault = def.asStruct().fields()[1];
  EXPECT_EQ(fieldWithoutDefault.identity(), identity(2, "fieldWithoutDefault"));
  EXPECT_EQ(fieldWithoutDefault.presence(), optional);
  EXPECT_EQ(fieldWithoutDefault.type().id(), TypeIds::I64);
  EXPECT_EQ(fieldWithoutDefault.customDefault(), nullptr);
}

TEST(TypeSystemTest, Annotations) {
  TypeSystemBuilder builder;
  builder.addType(
      "meta.com/thrift/test/MyAnnot",
      makeStruct({makeField(identity(1, "field1"), optional, TypeIds::I32)}));

  folly::F14FastMap<Uri, SerializableRecordUnion> annots = {
      {"meta.com/thrift/test/MyAnnot",
       SerializableRecord::toThrift({SerializableRecord::FieldSet(
           {{FieldId(1), SerializableRecord::Int32(42)}})})}};

  // @MyAnnot{field1=42}
  // struct MyStruct{
  //   @MyAnnot{field1=42}
  //   1: optional i32 field1;
  // }
  builder.addType(
      "meta.com/thrift/test/MyStruct",
      makeStruct(
          {makeField(
              identity(1, "field1"),
              optional,
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
      makeUnion(
          {makeField(
              identity(1, "int64"),
              optional,
              TypeIds::I64,
              std::nullopt,
              annots)},
          false,
          annots));

  // @MyAnnot{field1=42}
  // typedef i32 MyI32
  builder.addType(
      "meta.com/thrift/test/MyI32", makeOpaqueAlias(TypeIds::I32, annots));

  // @MyAnnot{field1=42}
  // enum MyEnum {
  //   @MyAnnot{field1=42}
  //   VALUE = 1,
  // }
  builder.addType(
      "meta.com/thrift/test/MyEnum", makeEnum({{"VALUE1", 1, annots}}, annots));

  auto typeSystem = std::move(builder).build();

  auto checkAnnot = [](const auto& node) {
    const auto* annot =
        node.getAnnotationOrNull("meta.com/thrift/test/MyAnnot");
    ASSERT_TRUE(annot != nullptr);
    EXPECT_EQ(annot->asFieldSet().at(FieldId{1}).asInt32(), 42);
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
}

TEST(TypeSystemTest, WrongAnnotationTypeUri) {
  TypeSystemBuilder builder;

  folly::F14FastMap<Uri, SerializableRecordUnion> annots = {
      {"meta.com/thrift/test/MyAnnot",
       SerializableRecord::toThrift({SerializableRecord::FieldSet(
           {{FieldId(1), SerializableRecord::Int32(42)}})})}};

  // @MyAnnot{field1=42}
  // struct MyStruct{
  //   @MyAnnot{field1=42}
  //   1: optional i32 field1;
  // }
  builder.addType(
      "meta.com/thrift/test/MyAnnot",
      makeUnion({makeField(identity(1, "field1"), optional, TypeIds::I32)}));
  builder.addType(
      "meta.com/thrift/test/MyStruct",
      makeStruct(
          {makeField(
              identity(1, "field1"),
              optional,
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
  folly::F14FastMap<Uri, SerializableRecordUnion> annots = {
      {"meta.com/thrift/test/MyAnnot",
       SerializableRecord::toThrift({SerializableRecord::Int32(42)})}};

  // @MyAnnot{field1=42}
  // struct MyStruct{
  //   @MyAnnot{field1=42}
  //   1: optional i32 field1;
  // }
  builder.addType(
      "meta.com/thrift/test/MyAnnot",
      makeStruct({makeField(identity(1, "field1"), optional, TypeIds::I32)}));
  builder.addType(
      "meta.com/thrift/test/MyStruct",
      makeStruct(
          {makeField(
              identity(1, "field1"),
              optional,
              TypeIds::I32,
              std::nullopt,
              annots)},
          false,
          annots));
  EXPECT_THROW(std::move(builder).build(), InvalidTypeError);
}

TEST(TypeSystemTest, MissingAnnotationType) {
  TypeSystemBuilder builder;

  folly::F14FastMap<Uri, SerializableRecordUnion> annots = {
      {"meta.com/thrift/test/MyAnnot",
       SerializableRecord::toThrift({SerializableRecord::FieldSet(
           {{FieldId(1), SerializableRecord::Int32(42)}})})}};

  // @MyAnnot{field1=42}
  // struct MyStruct{
  //   @MyAnnot{field1=42}
  //   1: optional i32 field1;
  // }
  builder.addType(
      "meta.com/thrift/test/MyStruct",
      makeStruct(
          {makeField(
              identity(1, "field1"),
              optional,
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
          makeUnion({
              makeField(
                  identity(1, "nonOptionalField"), unqualified, TypeIds::I32),
          })),
      InvalidTypeError);
}

TEST(TypeSystemTest, FieldIdentitiesMustBeUnique) {
  TypeSystemBuilder builder;

  // Attempt to add a struct with duplicate field ids
  EXPECT_THROW(
      builder.addType(
          "meta.com/thrift/test/DuplicateFieldIdStruct",
          makeStruct({
              makeField(identity(1, "field1"), optional, TypeIds::I32),
              makeField(identity(1, "field2"), optional, TypeIds::I64),
          })),
      InvalidTypeError);

  // Attempt to add a struct with duplicate field names
  EXPECT_THROW(
      builder.addType(
          "meta.com/thrift/test/DuplicateFieldNameStruct",
          makeStruct({
              makeField(identity(1, "field1"), optional, TypeIds::I32),
              makeField(identity(2, "field1"), optional, TypeIds::I64),
          })),
      InvalidTypeError);
}

TEST(TypeSystemTest, OpaqueAliasMustNotBeUserDefinedType) {
  TypeSystemBuilder builder;

  // Attempt to add an opaque alias with a user-defined type as target
  EXPECT_THROW(
      builder.addType(
          "meta.com/thrift/test/InvalidOpaqueAlias",
          makeOpaqueAlias(TypeIds::uri("meta.com/thrift/test/MyStruct"))),
      InvalidTypeError);
}

TEST(TypeSystemTest, EnumMappingsMustBeUnique) {
  TypeSystemBuilder builder;

  // Attempt to add an enum with duplicate names
  EXPECT_THROW(
      builder.addType(
          "meta.com/thrift/test/DuplicateEnumName",
          makeEnum({{"name1", 1}, {"name1", 2}})),
      InvalidTypeError);

  // Attempt to add an enum with duplicate values
  EXPECT_THROW(
      builder.addType(
          "meta.com/thrift/test/DuplicateEnumValue",
          makeEnum({{"name1", 1}, {"name2", 1}})),
      InvalidTypeError);
}

TEST(TypeSystemTest, ListTypeRef) {
  TypeSystemBuilder builder;

  // Define a struct with a list field
  builder.addType(
      "meta.com/thrift/test/ListStruct",
      makeStruct({
          makeField(
              identity(1, "listField"), optional, TypeIds::list(TypeIds::I32)),
      }));

  auto typeSystem = std::move(builder).build();

  // Check the struct with the list field
  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/ListStruct"));
  EXPECT_EQ(def.uri(), "meta.com/thrift/test/ListStruct");
  EXPECT_EQ(def.asStruct().fields().size(), 1);
  const FieldNode& listField = def.asStruct().fields()[0];
  EXPECT_EQ(listField.identity(), identity(1, "listField"));
  EXPECT_EQ(listField.presence(), optional);
  EXPECT_TRUE(listField.type().isList());
  EXPECT_EQ(listField.type().asList().id(), TypeIds::list(TypeIds::I32));
  EXPECT_EQ(listField.type().id(), TypeRef::List::of(TypeRef::I32()).id());
}

TEST(TypeSystemTest, SetTypeRef) {
  TypeSystemBuilder builder;

  // Define a struct with a set field
  builder.addType(
      "meta.com/thrift/test/SetStruct",
      makeStruct({
          makeField(
              identity(1, "setField"), optional, TypeIds::set(TypeIds::I32)),
      }));

  auto typeSystem = std::move(builder).build();

  // Check the struct with the set field
  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/SetStruct"));
  EXPECT_EQ(def.uri(), "meta.com/thrift/test/SetStruct");
  EXPECT_EQ(def.asStruct().fields().size(), 1);
  const FieldNode& setField = def.asStruct().fields()[0];
  EXPECT_EQ(setField.identity(), identity(1, "setField"));
  EXPECT_EQ(setField.presence(), optional);
  EXPECT_TRUE(setField.type().isSet());
  EXPECT_EQ(setField.type().asSet().id(), TypeIds::set(TypeIds::I32));
  EXPECT_EQ(setField.type().id(), TypeRef::Set::of(TypeRef::I32()).id());
}

TEST(TypeSystemTest, MapTypeRef) {
  TypeSystemBuilder builder;

  // Define a struct with a map field
  builder.addType(
      "meta.com/thrift/test/MapStruct",
      makeStruct({
          makeField(
              identity(1, "mapField"),
              optional,
              TypeIds::map(TypeIds::I32, TypeIds::String)),
      }));

  auto typeSystem = std::move(builder).build();

  // Check the struct with the map field
  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/MapStruct"));
  EXPECT_EQ(def.uri(), "meta.com/thrift/test/MapStruct");
  EXPECT_EQ(def.asStruct().fields().size(), 1);
  const FieldNode& mapField = def.asStruct().fields()[0];
  EXPECT_EQ(mapField.identity(), identity(1, "mapField"));
  EXPECT_EQ(mapField.presence(), optional);
  EXPECT_TRUE(mapField.type().isMap());
  EXPECT_EQ(
      mapField.type().asMap().id(),
      TypeIds::map(TypeIds::I32, TypeIds::String));
  EXPECT_EQ(
      mapField.type().id(),
      TypeRef::Map::of(TypeRef::I32(), TypeRef::String()).id());
}

TEST(TypeSystemTest, OpaqueAliasTypeRef) {
  TypeSystemBuilder builder;

  // Define an opaque alias
  builder.addType(
      "meta.com/thrift/test/OpaqueAlias", makeOpaqueAlias(TypeIds::I32));

  auto typeSystem = std::move(builder).build();

  // Check the opaque alias
  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/OpaqueAlias"));
  EXPECT_EQ(def.uri(), "meta.com/thrift/test/OpaqueAlias");
  EXPECT_TRUE(def.isOpaqueAlias());
  EXPECT_EQ(def.asOpaqueAlias().targetType().id(), TypeIds::I32);
}

TEST(TypeSystemTest, EnumTypeRef) {
  TypeSystemBuilder builder;

  // Define an enum
  builder.addType(
      "meta.com/thrift/test/SimpleEnum",
      makeEnum({{"VALUE1", 1}, {"VALUE2", 2}}));

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
}

TEST(TypeSystemTest, ComplexTypeReferences) {
  TypeSystemBuilder builder;

  // Define a struct with complex type references
  builder.addType(
      "meta.com/thrift/test/ComplexStruct",
      makeStruct({
          makeField(
              identity(1, "listOfStructs"),
              optional,
              TypeIds::list(TypeIds::uri("meta.com/thrift/test/SimpleStruct"))),
          makeField(
              identity(2, "mapOfUnions"),
              optional,
              TypeIds::map(
                  TypeIds::String,
                  TypeIds::uri("meta.com/thrift/test/SimpleUnion"))),
      }));
  builder.addType(
      "meta.com/thrift/test/SimpleStruct",
      makeStruct({
          makeField(identity(1, "field1"), optional, TypeIds::I32),
      }));
  builder.addType(
      "meta.com/thrift/test/SimpleUnion",
      makeUnion({
          makeField(identity(1, "field1"), optional, TypeIds::I64),
      }));

  auto typeSystem = std::move(builder).build();

  // Check the complex struct
  DefinitionRef complexDef = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/ComplexStruct"));
  EXPECT_EQ(complexDef.uri(), "meta.com/thrift/test/ComplexStruct");
  EXPECT_EQ(complexDef.asStruct().fields().size(), 2);

  const FieldNode& listField = complexDef.asStruct().fields()[0];
  EXPECT_EQ(listField.identity(), identity(1, "listOfStructs"));
  EXPECT_EQ(listField.presence(), optional);
  EXPECT_TRUE(listField.type().isList());
  EXPECT_EQ(
      listField.type().asList().elementType().asStruct().fields().size(), 1);

  const FieldNode& mapField = complexDef.asStruct().fields()[1];
  EXPECT_EQ(mapField.identity(), identity(2, "mapOfUnions"));
  EXPECT_EQ(mapField.presence(), optional);
  EXPECT_TRUE(mapField.type().isMap());
  EXPECT_EQ(mapField.type().asMap().keyType().id(), TypeIds::String);
  EXPECT_EQ(mapField.type().asMap().valueType().asUnion().fields().size(), 1);
}

TEST(TypeSystemTest, NestedContainers) {
  TypeSystemBuilder builder;

  // Define a struct with nested containers
  builder.addType(
      "meta.com/thrift/test/NestedContainerStruct",
      makeStruct({
          makeField(
              identity(1, "listOfMaps"),
              optional,
              TypeIds::list(TypeIds::map(TypeIds::String, TypeIds::I32))),
          makeField(
              identity(2, "setOfLists"),
              optional,
              TypeIds::set(TypeIds::list(TypeIds::I64))),
      }));

  auto typeSystem = std::move(builder).build();

  // Check the struct with nested containers
  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/NestedContainerStruct"));
  EXPECT_EQ(def.uri(), "meta.com/thrift/test/NestedContainerStruct");
  EXPECT_EQ(def.asStruct().fields().size(), 2);

  const FieldNode& listOfMapsField = def.asStruct().fields()[0];
  EXPECT_EQ(listOfMapsField.identity(), identity(1, "listOfMaps"));
  EXPECT_EQ(listOfMapsField.presence(), optional);
  EXPECT_TRUE(listOfMapsField.type().isList());
  EXPECT_TRUE(listOfMapsField.type().asList().elementType().isMap());
  EXPECT_EQ(
      listOfMapsField.type().asList().elementType().asMap().keyType().id(),
      TypeIds::String);
  EXPECT_EQ(
      listOfMapsField.type().asList().elementType().asMap().valueType().id(),
      TypeIds::I32);

  const FieldNode& setOfListsField = def.asStruct().fields()[1];
  EXPECT_EQ(setOfListsField.identity(), identity(2, "setOfLists"));
  EXPECT_EQ(setOfListsField.presence(), optional);
  EXPECT_TRUE(setOfListsField.type().isSet());
  EXPECT_TRUE(setOfListsField.type().asSet().elementType().isList());
  EXPECT_EQ(
      setOfListsField.type().asSet().elementType().asList().elementType().id(),
      TypeIds::I64);
}

TEST(TypeSystemTest, StructWithNegativeFieldId) {
  TypeSystemBuilder builder;

  // Define a struct with the maximum field id
  builder.addType(
      "meta.com/thrift/test/NegativeFieldId",
      makeStruct({
          makeField(identity(-1, "negativeFieldId"), unqualified, TypeIds::I32),
      }));

  auto typeSystem = std::move(builder).build();

  // Check the struct with the maximum field id
  DefinitionRef def = typeSystem->getUserDefinedTypeOrThrow(
      Uri("meta.com/thrift/test/NegativeFieldId"));
  EXPECT_EQ(def.uri(), "meta.com/thrift/test/NegativeFieldId");
  EXPECT_EQ(def.asStruct().fields().size(), 1);
  const FieldNode& negativeFieldId = def.asStruct().fields()[0];
  EXPECT_EQ(negativeFieldId.identity().id(), FieldId(-1));
  EXPECT_EQ(negativeFieldId.identity().name(), "negativeFieldId");
  EXPECT_EQ(negativeFieldId.presence(), unqualified);
  EXPECT_EQ(negativeFieldId.type().id(), TypeIds::I32);
}

TEST(TypeSystemTest, EnumWithNegativeValues) {
  TypeSystemBuilder builder;

  // Define an enum with negative values
  builder.addType(
      "meta.com/thrift/test/NegativeEnum",
      makeEnum({{"NEGATIVE_ONE", -1}, {"NEGATIVE_TWO", -2}}));

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
        makeStruct({
            makeField(
                identity(1, "innerStruct"),
                optional,
                TypeIds::uri("meta.com/thrift/test/InnerStruct")),
        }));
    builder.addType(
        "meta.com/thrift/test/InnerStruct",
        makeStruct({
            makeField(identity(1, "field1"), optional, TypeIds::I32),
        }));
    builder.addType(
        "meta.com/thrift/test/Alias",
        makeOpaqueAlias(
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
      makeStruct({
          makeField(identity(1, "field1"), optional, TypeIds::I32),
      }),
      makeSourceInfo("file://foo/bar.thrift", "StructWithI32Field"));

  builder.addType(
      "meta.com/thrift/test/Enum",
      makeEnum({{"VALUE1", 1}, {"VALUE2", 2}}),
      makeSourceInfo("file://foo/bar.thrift", "Enum"));

  builder.addType(
      "meta.com/thrift/test/UnionWithI32Field",
      makeUnion({
          makeField(identity(1, "field1"), optional, TypeIds::I32),
      }),
      makeSourceInfo("file://foo/other.thrift", "UnionWithI32Field"));

  builder.addType(
      "meta.com/thrift/test/OpaqueAlias",
      makeOpaqueAlias(TypeIds::I32),
      makeSourceInfo("file://foo/other.thrift", "OpaqueAlias"));

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
}

TEST(TypeSystemTest, SourceIndexedTypeSystemWithDuplicateEntries) {
  TypeSystemBuilder builder;

  builder.addType(
      "meta.com/thrift/test/StructWithI32Field",
      makeStruct({
          makeField(identity(1, "field1"), optional, TypeIds::I32),
      }),
      makeSourceInfo("file://foo/bar.thrift", "StructWithI32Field"));

  builder.addType(
      "meta.com/thrift/test/StructWithI32Field2",
      makeStruct({
          makeField(identity(1, "field1"), optional, TypeIds::I32),
      }),
      makeSourceInfo("file://foo/bar.thrift", "StructWithI32Field"));

  EXPECT_THAT(
      [&] { std::move(builder).build(); },
      testing::ThrowsMessage<InvalidTypeError>(
          testing::HasSubstr("Duplicate source identifier")));
}

} // namespace apache::thrift::type_system
