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

#include <thrift/lib/cpp2/dynamic/TypeSystemDigest.h>

#include <gtest/gtest.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/dynamic/SerializableRecord.h>
#include <thrift/lib/cpp2/dynamic/SerializableTypeSystemBuilder.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>

using namespace apache::thrift::type_system;
using def = TypeSystemBuilder::DefinitionHelper;

namespace {

template <typename T>
concept is_ts_hashable = std::invocable<TypeSystemHasher, std::decay_t<T>>;

template <typename T, typename U>
  requires is_ts_hashable<T> && is_ts_hashable<U>
void assertDigestEq(T&& left, U&& right) {
  ASSERT_EQ(TypeSystemHasher{}(left), TypeSystemHasher{}(right));
}

template <typename T, typename U>
  requires is_ts_hashable<T> && is_ts_hashable<U>
void assertDigestNeq(T&& left, U&& right) {
  ASSERT_NE(TypeSystemHasher{}(left), TypeSystemHasher{}(right));
}

/**
 * Helper functions for directly constructing SerializableTypeSystem entries.
 * Use these only for special cases that TypeSystemBuilder can't handle,
 * such as annotation URIs that aren't actual types in the type system.
 */
SerializableTypeDefinitionEntry makeStructEntry(
    SerializableStructDefinition s) {
  SerializableTypeDefinitionEntry entry;
  entry.definition()->structDef_ref() = std::move(s);
  return entry;
}

SerializableTypeDefinitionEntry makeUnionEntry(SerializableUnionDefinition u) {
  SerializableTypeDefinitionEntry entry;
  entry.definition()->unionDef_ref() = std::move(u);
  return entry;
}

} // namespace

TEST(TypeSystemDigestTest, VersionConstantExists) {
  EXPECT_EQ(kTypeSystemDigestVersion, 2);
}

TEST(TypeSystemDigestTest, EmptyTypeSystem) {
  TypeSystemBuilder builder;
  auto ts = std::move(builder).build();
  auto digest = TypeSystemHasher{}(*ts);
  EXPECT_EQ(digest.size(), 32);

  // Verify determinism
  auto digest2 = TypeSystemHasher{}(*ts);
  EXPECT_EQ(digest, digest2);
}

TEST(TypeSystemDigestTest, IgnoresTypeIterationOrder) {
  // Empty struct definitions - order of types doesn't affect digest
  auto emptyStruct = def::Struct({});

  TypeSystemBuilder builder1;
  builder1.addType("meta.com/a", emptyStruct);
  builder1.addType("meta.com/b", emptyStruct);
  builder1.addType("meta.com/c", emptyStruct);
  auto ts1 = std::move(builder1).build();

  TypeSystemBuilder builder2;
  builder2.addType("meta.com/c", emptyStruct);
  builder2.addType("meta.com/a", emptyStruct);
  builder2.addType("meta.com/b", emptyStruct);
  auto ts2 = std::move(builder2).build();

  assertDigestEq(*ts1, *ts2);
}

TEST(TypeSystemDigestTest, IgnoresSourceInfo) {
  // Source info is excluded from digest computation
  auto structDef = def::Struct({});

  // Both type systems have the same definition - they should have the same
  // digest regardless of any source info differences.
  TypeSystemBuilder builder1;
  builder1.addType("meta.com/Foo", structDef);
  auto ts1 = std::move(builder1).build();

  TypeSystemBuilder builder2;
  builder2.addType("meta.com/Foo", structDef);
  auto ts2 = std::move(builder2).build();

  assertDigestEq(*ts1, *ts2);
}

TEST(TypeSystemDigestTest, DifferentUriDifferentDigest) {
  auto emptyStruct = def::Struct({});

  TypeSystemBuilder builder1;
  builder1.addType("meta.com/Bar", emptyStruct);
  auto ts1 = std::move(builder1).build();

  TypeSystemBuilder builder2;
  builder2.addType("meta.com/Baz", emptyStruct);
  auto ts2 = std::move(builder2).build();

  assertDigestNeq(*ts1, *ts2);
}

TEST(TypeSystemDigestTest, StructFieldOrderIndependent) {
  auto field1 =
      def::Field(def::Identity(1, "first"), def::AlwaysPresent, TypeId::I32());
  auto field2 =
      def::Field(def::Identity(2, "second"), def::AlwaysPresent, TypeId::I32());
  auto field3 =
      def::Field(def::Identity(3, "third"), def::AlwaysPresent, TypeId::I32());

  TypeSystemBuilder builder1;
  builder1.addType("meta.com/S", def::Struct({field1, field2, field3}));
  auto ts1 = std::move(builder1).build();

  // Insert in different order
  TypeSystemBuilder builder2;
  builder2.addType("meta.com/S", def::Struct({field3, field1, field2}));
  auto ts2 = std::move(builder2).build();

  assertDigestEq(*ts1, *ts2);
}

TEST(TypeSystemDigestTest, UnionFieldOrderIndependent) {
  auto field1 =
      def::Field(def::Identity(1, "opt1"), def::Optional, TypeId::I32());
  auto field2 =
      def::Field(def::Identity(2, "opt2"), def::Optional, TypeId::I32());

  TypeSystemBuilder builder1;
  builder1.addType("meta.com/U", def::Union({field1, field2}));
  auto ts1 = std::move(builder1).build();

  TypeSystemBuilder builder2;
  builder2.addType("meta.com/U", def::Union({field2, field1}));
  auto ts2 = std::move(builder2).build();

  assertDigestEq(*ts1, *ts2);
}

TEST(TypeSystemDigestTest, EnumValueOrderIndependent) {
  TypeSystemBuilder builder1;
  builder1.addType("meta.com/E", def::Enum({{"A", 1}, {"B", 2}, {"C", 3}}));
  auto ts1 = std::move(builder1).build();

  TypeSystemBuilder builder2;
  builder2.addType("meta.com/E", def::Enum({{"C", 3}, {"A", 1}, {"B", 2}}));
  auto ts2 = std::move(builder2).build();

  assertDigestEq(*ts1, *ts2);
}

TEST(TypeSystemDigestTest, DifferentFieldIdDifferentDigest) {
  auto field1 =
      def::Field(def::Identity(1, "field"), def::AlwaysPresent, TypeId::I32());
  auto field2 =
      def::Field(def::Identity(2, "field"), def::AlwaysPresent, TypeId::I32());

  TypeSystemBuilder builder1;
  builder1.addType("meta.com/S", def::Struct({field1}));
  auto ts1 = std::move(builder1).build();

  TypeSystemBuilder builder2;
  builder2.addType("meta.com/S", def::Struct({field2}));
  auto ts2 = std::move(builder2).build();

  assertDigestNeq(*ts1, *ts2);
}

TEST(TypeSystemDigestTest, DifferentFieldNameDifferentDigest) {
  auto field1 =
      def::Field(def::Identity(1, "alpha"), def::AlwaysPresent, TypeId::I32());
  auto field2 =
      def::Field(def::Identity(1, "beta"), def::AlwaysPresent, TypeId::I32());

  TypeSystemBuilder builder1;
  builder1.addType("meta.com/S", def::Struct({field1}));
  auto ts1 = std::move(builder1).build();

  TypeSystemBuilder builder2;
  builder2.addType("meta.com/S", def::Struct({field2}));
  auto ts2 = std::move(builder2).build();

  assertDigestNeq(*ts1, *ts2);
}

TEST(TypeSystemDigestTest, DifferentEnumValueDifferentDigest) {
  TypeSystemBuilder builder1;
  builder1.addType("meta.com/E", def::Enum({{"X", 1}}));
  auto ts1 = std::move(builder1).build();

  TypeSystemBuilder builder2;
  builder2.addType("meta.com/E", def::Enum({{"X", 2}}));
  auto ts2 = std::move(builder2).build();

  assertDigestNeq(*ts1, *ts2);
}

TEST(TypeSystemDigestTest, DifferentEnumNameDifferentDigest) {
  TypeSystemBuilder builder1;
  builder1.addType("meta.com/E", def::Enum({{"FOO", 1}}));
  auto ts1 = std::move(builder1).build();

  TypeSystemBuilder builder2;
  builder2.addType("meta.com/E", def::Enum({{"BAR", 1}}));
  auto ts2 = std::move(builder2).build();

  assertDigestNeq(*ts1, *ts2);
}

TEST(TypeSystemDigestTest, StructVsUnionDifferentDigest) {
  // Use direct construction to create struct and union with same field
  // presence to test that the type kind difference alone produces different
  // digests. (TypeSystemBuilder requires union fields to be Optional.)
  auto field = def::Field(def::Identity(1, "f"), def::Optional, TypeId::I32());

  SerializableTypeSystem ts1, ts2;
  ts1.types()->emplace("meta.com/T", makeStructEntry(def::Struct({field})));
  ts2.types()->emplace("meta.com/T", makeUnionEntry(def::Union({field})));

  assertDigestNeq(ts1, ts2);
}

TEST(TypeSystemDigestTest, SealedDifferentDigest) {
  auto field =
      def::Field(def::Identity(1, "f"), def::AlwaysPresent, TypeId::I32());

  TypeSystemBuilder builder1;
  builder1.addType("meta.com/S", def::Struct({field}, /*isSealed=*/false));
  auto ts1 = std::move(builder1).build();

  TypeSystemBuilder builder2;
  builder2.addType("meta.com/S", def::Struct({field}, /*isSealed=*/true));
  auto ts2 = std::move(builder2).build();

  assertDigestNeq(*ts1, *ts2);
}

TEST(TypeSystemDigestTest, PresenceQualifierDifferentDigest) {
  auto field1 =
      def::Field(def::Identity(1, "f"), def::AlwaysPresent, TypeId::I32());
  auto field2 = def::Field(def::Identity(1, "f"), def::Optional, TypeId::I32());

  TypeSystemBuilder builder1;
  builder1.addType("meta.com/S", def::Struct({field1}));
  auto ts1 = std::move(builder1).build();

  TypeSystemBuilder builder2;
  builder2.addType("meta.com/S", def::Struct({field2}));
  auto ts2 = std::move(builder2).build();

  assertDigestNeq(*ts1, *ts2);
}

TEST(TypeSystemDigestTest, AnnotationOrderIndependent) {
  // Annotations use URIs that may not be types in the system,
  // so we use direct construction here.
  AnnotationsMap ann1{
      {"meta.com/Ann1", SerializableRecord::FieldSet({})},
      {"meta.com/Ann2", SerializableRecord::FieldSet({})}};
  AnnotationsMap ann2{
      {"meta.com/Ann2", SerializableRecord::FieldSet({})},
      {"meta.com/Ann1", SerializableRecord::FieldSet({})}};

  auto field1 = def::Field(
      def::Identity(1, "f"), def::AlwaysPresent, TypeId::I32(), {}, ann1);
  auto field2 = def::Field(
      def::Identity(1, "f"), def::AlwaysPresent, TypeId::I32(), {}, ann2);

  SerializableTypeSystem ts1, ts2;
  ts1.types()->emplace("meta.com/S", makeStructEntry(def::Struct({field1})));
  ts2.types()->emplace("meta.com/S", makeStructEntry(def::Struct({field2})));

  assertDigestEq(ts1, ts2);
}

TEST(TypeSystemDigestTest, DifferentAnnotationDifferentDigest) {
  // Annotations use URIs that may not be types in the system,
  // so we use direct construction here.
  AnnotationsMap ann1{{"meta.com/Ann1", SerializableRecord::FieldSet({})}};
  AnnotationsMap ann2{{"meta.com/Ann2", SerializableRecord::FieldSet({})}};

  auto field1 = def::Field(
      def::Identity(1, "f"), def::AlwaysPresent, TypeId::I32(), {}, ann1);
  auto field2 = def::Field(
      def::Identity(1, "f"), def::AlwaysPresent, TypeId::I32(), {}, ann2);

  SerializableTypeSystem ts1, ts2;
  ts1.types()->emplace("meta.com/S", makeStructEntry(def::Struct({field1})));
  ts2.types()->emplace("meta.com/S", makeStructEntry(def::Struct({field2})));

  assertDigestNeq(ts1, ts2);
}

// Tests for TypeRef hashing equivalence with SerializableTypeDefinition

TEST(TypeSystemDigestTest, TypeRefStructMatchesSerializable) {
  auto structDef = def::Struct(
      {def::Field(
           def::Identity(1, "field1"), def::AlwaysPresent, TypeId::I32()),
       def::Field(
           def::Identity(2, "field2"), def::Optional, TypeId::String())});

  // Build runtime TypeSystem
  TypeSystemBuilder builder;
  builder.addType("meta.com/TestStruct", structDef);
  auto typeSystem = std::move(builder).build();

  // Get TypeRef from built TypeSystem
  auto defRef = typeSystem->getUserDefinedTypeOrThrow("meta.com/TestStruct");

  // Build SerializableTypeDefinition
  SerializableTypeDefinition serializableDef;
  serializableDef.structDef_ref() = structDef;

  // Hash both - they should produce the same digest
  TypeSystemHasher hasher;
  auto typeRefDigest = hasher(TypeRef::fromDefinition(defRef));
  auto serializableDigest = hasher(serializableDef);

  EXPECT_EQ(typeRefDigest, serializableDigest);
}

TEST(TypeSystemDigestTest, TypeRefUnionMatchesSerializable) {
  auto unionDef = def::Union(
      {def::Field(def::Identity(1, "opt1"), def::Optional, TypeId::I32()),
       def::Field(def::Identity(2, "opt2"), def::Optional, TypeId::String())});

  // Build runtime TypeSystem
  TypeSystemBuilder builder;
  builder.addType("meta.com/TestUnion", unionDef);
  auto typeSystem = std::move(builder).build();

  // Get TypeRef from built TypeSystem
  auto defRef = typeSystem->getUserDefinedTypeOrThrow("meta.com/TestUnion");

  // Build SerializableTypeDefinition
  SerializableTypeDefinition serializableDef;
  serializableDef.unionDef_ref() = unionDef;

  // Hash both - they should produce the same digest
  TypeSystemHasher hasher;
  auto typeRefDigest = hasher(TypeRef::fromDefinition(defRef));
  auto serializableDigest = hasher(serializableDef);

  EXPECT_EQ(typeRefDigest, serializableDigest);
}

TEST(TypeSystemDigestTest, TypeRefEnumMatchesSerializable) {
  auto enumDef = def::Enum({{"VALUE_A", 1}, {"VALUE_B", 2}, {"VALUE_C", 3}});

  // Build runtime TypeSystem
  TypeSystemBuilder builder;
  builder.addType("meta.com/TestEnum", enumDef);
  auto typeSystem = std::move(builder).build();

  // Get TypeRef from built TypeSystem
  auto defRef = typeSystem->getUserDefinedTypeOrThrow("meta.com/TestEnum");

  // Build SerializableTypeDefinition
  SerializableTypeDefinition serializableDef;
  serializableDef.enumDef_ref() = enumDef;

  // Hash both - they should produce the same digest
  TypeSystemHasher hasher;
  auto typeRefDigest = hasher(TypeRef::fromDefinition(defRef));
  auto serializableDigest = hasher(serializableDef);

  EXPECT_EQ(typeRefDigest, serializableDigest);
}

TEST(TypeSystemDigestTest, TypeRefOpaqueAliasMatchesSerializable) {
  auto opaqueAliasDef = def::OpaqueAlias(TypeId::I64());

  // Build runtime TypeSystem
  TypeSystemBuilder builder;
  builder.addType("meta.com/TestAlias", opaqueAliasDef);
  auto typeSystem = std::move(builder).build();

  // Get TypeRef from built TypeSystem
  auto defRef = typeSystem->getUserDefinedTypeOrThrow("meta.com/TestAlias");

  // Build SerializableTypeDefinition
  SerializableTypeDefinition serializableDef;
  serializableDef.opaqueAliasDef_ref() = opaqueAliasDef;

  // Hash both - they should produce the same digest
  TypeSystemHasher hasher;
  auto typeRefDigest = hasher(TypeRef::fromDefinition(defRef));
  auto serializableDigest = hasher(serializableDef);

  EXPECT_EQ(typeRefDigest, serializableDigest);
}

TEST(TypeSystemDigestTest, TypeRefPrimitiveThrowsException) {
  TypeSystemHasher hasher;

  // Primitive types should throw an exception
  EXPECT_THROW(hasher(TypeRef(TypeRef::Bool{})), std::invalid_argument);
  EXPECT_THROW(hasher(TypeRef(TypeRef::Byte{})), std::invalid_argument);
  EXPECT_THROW(hasher(TypeRef(TypeRef::I16{})), std::invalid_argument);
  EXPECT_THROW(hasher(TypeRef(TypeRef::I32{})), std::invalid_argument);
  EXPECT_THROW(hasher(TypeRef(TypeRef::I64{})), std::invalid_argument);
  EXPECT_THROW(hasher(TypeRef(TypeRef::Float{})), std::invalid_argument);
  EXPECT_THROW(hasher(TypeRef(TypeRef::Double{})), std::invalid_argument);
  EXPECT_THROW(hasher(TypeRef(TypeRef::String{})), std::invalid_argument);
  EXPECT_THROW(hasher(TypeRef(TypeRef::Binary{})), std::invalid_argument);
  EXPECT_THROW(hasher(TypeRef(TypeRef::Any{})), std::invalid_argument);
}

// Extensive tests for TypeRef hashing equivalence

TEST(TypeSystemDigestTest, TypeRefContainerThrowsException) {
  TypeSystemHasher hasher;

  // Build a type system to get access to container type construction
  TypeSystemBuilder builder;
  builder.addType(
      "meta.com/S",
      def::Struct({def::Field(
          def::Identity(1, "f"), def::AlwaysPresent, TypeId::I32())}));
  auto typeSystem = std::move(builder).build();

  // Container types should throw an exception
  auto listRef = typeSystem->ListOf(TypeRef(TypeRef::I32{}));
  auto setRef = typeSystem->SetOf(TypeRef(TypeRef::I32{}));
  auto mapRef =
      typeSystem->MapOf(TypeRef(TypeRef::String{}), TypeRef(TypeRef::I32{}));

  EXPECT_THROW(hasher(listRef), std::invalid_argument);
  EXPECT_THROW(hasher(setRef), std::invalid_argument);
  EXPECT_THROW(hasher(mapRef), std::invalid_argument);
}

TEST(TypeSystemDigestTest, TypeRefStructWithAnnotationsMatchesSerializable) {
  AnnotationsMap structAnnotations{
      {"facebook.com/thrift/annotation/Deprecated",
       SerializableRecord::Bool{true}}};
  AnnotationsMap fieldAnnotations{
      {"facebook.com/thrift/annotation/Doc",
       SerializableRecord::Text{"A field"}}};

  auto structDef = def::Struct(
      {def::Field(
          def::Identity(1, "annotatedField"),
          def::AlwaysPresent,
          TypeId::I32(),
          std::nullopt,
          fieldAnnotations)},
      false,
      structAnnotations);

  TypeSystemBuilder builder;
  builder.addType("meta.com/AnnotatedStruct", structDef);
  auto typeSystem = std::move(builder).build();

  auto defRef =
      typeSystem->getUserDefinedTypeOrThrow("meta.com/AnnotatedStruct");

  SerializableTypeDefinition serializableDef;
  serializableDef.structDef_ref() = structDef;

  TypeSystemHasher hasher;
  EXPECT_EQ(hasher(TypeRef::fromDefinition(defRef)), hasher(serializableDef));
}

TEST(TypeSystemDigestTest, TypeRefStructWithCustomDefaultMatchesSerializable) {
  auto structDef = def::Struct({def::Field(
      def::Identity(1, "fieldWithDefault"),
      def::AlwaysPresent,
      TypeId::I32(),
      SerializableRecord::Int32{42})});

  TypeSystemBuilder builder;
  builder.addType("meta.com/StructWithDefault", structDef);
  auto typeSystem = std::move(builder).build();

  auto defRef =
      typeSystem->getUserDefinedTypeOrThrow("meta.com/StructWithDefault");

  SerializableTypeDefinition serializableDef;
  serializableDef.structDef_ref() = structDef;

  TypeSystemHasher hasher;
  EXPECT_EQ(hasher(TypeRef::fromDefinition(defRef)), hasher(serializableDef));
}

TEST(TypeSystemDigestTest, TypeRefStructSealedMatchesSerializable) {
  auto sealedStructDef = def::Struct(
      {def::Field(def::Identity(1, "f"), def::AlwaysPresent, TypeId::I32())},
      /*isSealed=*/true);

  TypeSystemBuilder builder;
  builder.addType("meta.com/SealedStruct", sealedStructDef);
  auto typeSystem = std::move(builder).build();

  auto defRef = typeSystem->getUserDefinedTypeOrThrow("meta.com/SealedStruct");

  SerializableTypeDefinition serializableDef;
  serializableDef.structDef_ref() = sealedStructDef;

  TypeSystemHasher hasher;
  EXPECT_EQ(hasher(TypeRef::fromDefinition(defRef)), hasher(serializableDef));
}

TEST(TypeSystemDigestTest, TypeRefStructWithManyFieldsMatchesSerializable) {
  auto structDef = def::Struct({
      def::Field(
          def::Identity(1, "field1"), def::AlwaysPresent, TypeId::Bool()),
      def::Field(def::Identity(2, "field2"), def::Optional, TypeId::Byte()),
      def::Field(def::Identity(3, "field3"), def::AlwaysPresent, TypeId::I16()),
      def::Field(def::Identity(4, "field4"), def::Optional, TypeId::I32()),
      def::Field(def::Identity(5, "field5"), def::AlwaysPresent, TypeId::I64()),
      def::Field(def::Identity(6, "field6"), def::Optional, TypeId::Float()),
      def::Field(
          def::Identity(7, "field7"), def::AlwaysPresent, TypeId::Double()),
      def::Field(def::Identity(8, "field8"), def::Optional, TypeId::String()),
      def::Field(
          def::Identity(9, "field9"), def::AlwaysPresent, TypeId::Binary()),
  });

  TypeSystemBuilder builder;
  builder.addType("meta.com/ManyFieldsStruct", structDef);
  auto typeSystem = std::move(builder).build();

  auto defRef =
      typeSystem->getUserDefinedTypeOrThrow("meta.com/ManyFieldsStruct");

  SerializableTypeDefinition serializableDef;
  serializableDef.structDef_ref() = structDef;

  TypeSystemHasher hasher;
  EXPECT_EQ(hasher(TypeRef::fromDefinition(defRef)), hasher(serializableDef));
}

TEST(TypeSystemDigestTest, TypeRefEnumWithAnnotationsMatchesSerializable) {
  AnnotationsMap enumAnnotations{
      {"facebook.com/thrift/annotation/Doc",
       SerializableRecord::Text{"Color enum"}}};
  AnnotationsMap valueAnnotations{
      {"facebook.com/thrift/annotation/Alias", SerializableRecord::Text{"R"}}};

  auto enumDef = def::Enum(
      {{"RED", 1, valueAnnotations}, {"GREEN", 2}, {"BLUE", 3}},
      enumAnnotations);

  TypeSystemBuilder builder;
  builder.addType("meta.com/AnnotatedEnum", enumDef);
  auto typeSystem = std::move(builder).build();

  auto defRef = typeSystem->getUserDefinedTypeOrThrow("meta.com/AnnotatedEnum");

  SerializableTypeDefinition serializableDef;
  serializableDef.enumDef_ref() = enumDef;

  TypeSystemHasher hasher;
  EXPECT_EQ(hasher(TypeRef::fromDefinition(defRef)), hasher(serializableDef));
}

TEST(TypeSystemDigestTest, TypeRefEnumWithManyValuesMatchesSerializable) {
  auto enumDef = def::Enum({
      {"VALUE_0", 0},
      {"VALUE_1", 1},
      {"VALUE_2", 2},
      {"VALUE_3", 3},
      {"VALUE_4", 4},
      {"VALUE_5", 5},
      {"VALUE_10", 10},
      {"VALUE_100", 100},
      {"VALUE_NEGATIVE", -1},
  });

  TypeSystemBuilder builder;
  builder.addType("meta.com/ManyValuesEnum", enumDef);
  auto typeSystem = std::move(builder).build();

  auto defRef =
      typeSystem->getUserDefinedTypeOrThrow("meta.com/ManyValuesEnum");

  SerializableTypeDefinition serializableDef;
  serializableDef.enumDef_ref() = enumDef;

  TypeSystemHasher hasher;
  EXPECT_EQ(hasher(TypeRef::fromDefinition(defRef)), hasher(serializableDef));
}

TEST(
    TypeSystemDigestTest,
    TypeRefOpaqueAliasWithAnnotationsMatchesSerializable) {
  AnnotationsMap annotations{
      {"facebook.com/thrift/annotation/TypeAlias",
       SerializableRecord::Text{"MyInt64"}}};

  auto opaqueAliasDef = def::OpaqueAlias(TypeId::I64(), annotations);

  TypeSystemBuilder builder;
  builder.addType("meta.com/AnnotatedAlias", opaqueAliasDef);
  auto typeSystem = std::move(builder).build();

  auto defRef =
      typeSystem->getUserDefinedTypeOrThrow("meta.com/AnnotatedAlias");

  SerializableTypeDefinition serializableDef;
  serializableDef.opaqueAliasDef_ref() = opaqueAliasDef;

  TypeSystemHasher hasher;
  EXPECT_EQ(hasher(TypeRef::fromDefinition(defRef)), hasher(serializableDef));
}

TEST(TypeSystemDigestTest, TypeRefOpaqueAliasTargetTypesDiffer) {
  TypeSystemHasher hasher;

  auto i32AliasDef = def::OpaqueAlias(TypeId::I32());
  auto i64AliasDef = def::OpaqueAlias(TypeId::I64());
  auto stringAliasDef = def::OpaqueAlias(TypeId::String());

  TypeSystemBuilder builder;
  builder.addType("meta.com/I32Alias", i32AliasDef);
  builder.addType("meta.com/I64Alias", i64AliasDef);
  builder.addType("meta.com/StringAlias", stringAliasDef);
  auto typeSystem = std::move(builder).build();

  auto i32Ref = typeSystem->getUserDefinedTypeOrThrow("meta.com/I32Alias");
  auto i64Ref = typeSystem->getUserDefinedTypeOrThrow("meta.com/I64Alias");
  auto stringRef =
      typeSystem->getUserDefinedTypeOrThrow("meta.com/StringAlias");

  auto i32Digest = hasher(TypeRef::fromDefinition(i32Ref));
  auto i64Digest = hasher(TypeRef::fromDefinition(i64Ref));
  auto stringDigest = hasher(TypeRef::fromDefinition(stringRef));

  EXPECT_NE(i32Digest, i64Digest);
  EXPECT_NE(i64Digest, stringDigest);
  EXPECT_NE(i32Digest, stringDigest);
}

TEST(TypeSystemDigestTest, TypeRefDifferentTypeKindsDiffer) {
  TypeSystemHasher hasher;

  // All these have the same field structure but different type kinds
  auto field = def::Field(def::Identity(1, "f"), def::Optional, TypeId::I32());

  auto structDef = def::Struct({field});
  auto unionDef = def::Union({field});

  TypeSystemBuilder builder;
  builder.addType("meta.com/S", structDef);
  builder.addType("meta.com/U", unionDef);
  auto typeSystem = std::move(builder).build();

  auto structRef = typeSystem->getUserDefinedTypeOrThrow("meta.com/S");
  auto unionRef = typeSystem->getUserDefinedTypeOrThrow("meta.com/U");

  auto structDigest = hasher(TypeRef::fromDefinition(structRef));
  auto unionDigest = hasher(TypeRef::fromDefinition(unionRef));

  EXPECT_NE(structDigest, unionDigest);
}

TEST(TypeSystemDigestTest, TypeRefFieldOrderIndependent) {
  TypeSystemHasher hasher;

  auto field1 =
      def::Field(def::Identity(1, "a"), def::AlwaysPresent, TypeId::I32());
  auto field2 =
      def::Field(def::Identity(2, "b"), def::Optional, TypeId::String());
  auto field3 =
      def::Field(def::Identity(3, "c"), def::AlwaysPresent, TypeId::Bool());

  auto structDef1 = def::Struct({field1, field2, field3});
  auto structDef2 = def::Struct({field3, field1, field2});

  TypeSystemBuilder builder1, builder2;
  builder1.addType("meta.com/S", structDef1);
  builder2.addType("meta.com/S", structDef2);
  auto ts1 = std::move(builder1).build();
  auto ts2 = std::move(builder2).build();

  auto ref1 = ts1->getUserDefinedTypeOrThrow("meta.com/S");
  auto ref2 = ts2->getUserDefinedTypeOrThrow("meta.com/S");

  EXPECT_EQ(
      hasher(TypeRef::fromDefinition(ref1)),
      hasher(TypeRef::fromDefinition(ref2)));
}

TEST(TypeSystemDigestTest, TypeRefEnumValueOrderIndependent) {
  TypeSystemHasher hasher;

  auto enumDef1 = def::Enum({{"A", 1}, {"B", 2}, {"C", 3}});
  auto enumDef2 = def::Enum({{"C", 3}, {"A", 1}, {"B", 2}});

  TypeSystemBuilder builder1, builder2;
  builder1.addType("meta.com/E", enumDef1);
  builder2.addType("meta.com/E", enumDef2);
  auto ts1 = std::move(builder1).build();
  auto ts2 = std::move(builder2).build();

  auto ref1 = ts1->getUserDefinedTypeOrThrow("meta.com/E");
  auto ref2 = ts2->getUserDefinedTypeOrThrow("meta.com/E");

  EXPECT_EQ(
      hasher(TypeRef::fromDefinition(ref1)),
      hasher(TypeRef::fromDefinition(ref2)));
}

TEST(TypeSystemDigestTest, TypeRefAnnotationOrderIndependent) {
  TypeSystemHasher hasher;

  AnnotationsMap ann1{
      {"facebook.com/thrift/annotation/Ann1", SerializableRecord::Int32{1}},
      {"facebook.com/thrift/annotation/Ann2", SerializableRecord::Int32{2}},
      {"facebook.com/thrift/annotation/Ann3", SerializableRecord::Int32{3}}};
  AnnotationsMap ann2{
      {"facebook.com/thrift/annotation/Ann3", SerializableRecord::Int32{3}},
      {"facebook.com/thrift/annotation/Ann1", SerializableRecord::Int32{1}},
      {"facebook.com/thrift/annotation/Ann2", SerializableRecord::Int32{2}}};

  auto structDef1 = def::Struct(
      {def::Field(def::Identity(1, "f"), def::AlwaysPresent, TypeId::I32())},
      false,
      ann1);
  auto structDef2 = def::Struct(
      {def::Field(def::Identity(1, "f"), def::AlwaysPresent, TypeId::I32())},
      false,
      ann2);

  TypeSystemBuilder builder1, builder2;
  builder1.addType("meta.com/S", structDef1);
  builder2.addType("meta.com/S", structDef2);
  auto ts1 = std::move(builder1).build();
  auto ts2 = std::move(builder2).build();

  auto ref1 = ts1->getUserDefinedTypeOrThrow("meta.com/S");
  auto ref2 = ts2->getUserDefinedTypeOrThrow("meta.com/S");

  EXPECT_EQ(
      hasher(TypeRef::fromDefinition(ref1)),
      hasher(TypeRef::fromDefinition(ref2)));
}

TEST(TypeSystemDigestTest, TypeRefDifferentFieldTypesDiffer) {
  TypeSystemHasher hasher;

  auto structDef1 = def::Struct(
      {def::Field(def::Identity(1, "f"), def::AlwaysPresent, TypeId::I32())});
  auto structDef2 = def::Struct(
      {def::Field(def::Identity(1, "f"), def::AlwaysPresent, TypeId::I64())});

  TypeSystemBuilder builder1, builder2;
  builder1.addType("meta.com/S", structDef1);
  builder2.addType("meta.com/S", structDef2);
  auto ts1 = std::move(builder1).build();
  auto ts2 = std::move(builder2).build();

  auto ref1 = ts1->getUserDefinedTypeOrThrow("meta.com/S");
  auto ref2 = ts2->getUserDefinedTypeOrThrow("meta.com/S");

  EXPECT_NE(
      hasher(TypeRef::fromDefinition(ref1)),
      hasher(TypeRef::fromDefinition(ref2)));
}

TEST(TypeSystemDigestTest, TypeRefDifferentFieldPresenceDiffer) {
  TypeSystemHasher hasher;

  auto structDef1 = def::Struct(
      {def::Field(def::Identity(1, "f"), def::AlwaysPresent, TypeId::I32())});
  auto structDef2 = def::Struct(
      {def::Field(def::Identity(1, "f"), def::Optional, TypeId::I32())});

  TypeSystemBuilder builder1, builder2;
  builder1.addType("meta.com/S", structDef1);
  builder2.addType("meta.com/S", structDef2);
  auto ts1 = std::move(builder1).build();
  auto ts2 = std::move(builder2).build();

  auto ref1 = ts1->getUserDefinedTypeOrThrow("meta.com/S");
  auto ref2 = ts2->getUserDefinedTypeOrThrow("meta.com/S");

  EXPECT_NE(
      hasher(TypeRef::fromDefinition(ref1)),
      hasher(TypeRef::fromDefinition(ref2)));
}

TEST(TypeSystemDigestTest, TypeRefDifferentCustomDefaultsDiffer) {
  TypeSystemHasher hasher;

  auto structDef1 = def::Struct({def::Field(
      def::Identity(1, "f"),
      def::AlwaysPresent,
      TypeId::I32(),
      SerializableRecord::Int32{42})});
  auto structDef2 = def::Struct({def::Field(
      def::Identity(1, "f"),
      def::AlwaysPresent,
      TypeId::I32(),
      SerializableRecord::Int32{99})});

  TypeSystemBuilder builder1, builder2;
  builder1.addType("meta.com/S", structDef1);
  builder2.addType("meta.com/S", structDef2);
  auto ts1 = std::move(builder1).build();
  auto ts2 = std::move(builder2).build();

  auto ref1 = ts1->getUserDefinedTypeOrThrow("meta.com/S");
  auto ref2 = ts2->getUserDefinedTypeOrThrow("meta.com/S");

  EXPECT_NE(
      hasher(TypeRef::fromDefinition(ref1)),
      hasher(TypeRef::fromDefinition(ref2)));
}

TEST(TypeSystemDigestTest, TypeRefWithVsWithoutCustomDefaultDiffer) {
  TypeSystemHasher hasher;

  auto structDef1 = def::Struct({def::Field(
      def::Identity(1, "f"),
      def::AlwaysPresent,
      TypeId::I32(),
      SerializableRecord::Int32{0})});
  auto structDef2 = def::Struct(
      {def::Field(def::Identity(1, "f"), def::AlwaysPresent, TypeId::I32())});

  TypeSystemBuilder builder1, builder2;
  builder1.addType("meta.com/S", structDef1);
  builder2.addType("meta.com/S", structDef2);
  auto ts1 = std::move(builder1).build();
  auto ts2 = std::move(builder2).build();

  auto ref1 = ts1->getUserDefinedTypeOrThrow("meta.com/S");
  auto ref2 = ts2->getUserDefinedTypeOrThrow("meta.com/S");

  EXPECT_NE(
      hasher(TypeRef::fromDefinition(ref1)),
      hasher(TypeRef::fromDefinition(ref2)));
}

TEST(TypeSystemDigestTest, TypeRefDeterministic) {
  TypeSystemHasher hasher;

  auto structDef = def::Struct({
      def::Field(def::Identity(1, "a"), def::AlwaysPresent, TypeId::I32()),
      def::Field(def::Identity(2, "b"), def::Optional, TypeId::String()),
  });

  TypeSystemBuilder builder;
  builder.addType("meta.com/S", structDef);
  auto typeSystem = std::move(builder).build();

  auto defRef = typeSystem->getUserDefinedTypeOrThrow("meta.com/S");
  auto typeRef = TypeRef::fromDefinition(defRef);

  // Hashing should be deterministic
  {
    auto digest1 = hasher(typeRef);
    auto digest2 = hasher(typeRef);
    EXPECT_EQ(digest1, digest2);
  }

  // Hash of the TypeRef should match the serialized type system entry
  {
    auto stsBuilder =
        SerializableTypeSystemBuilder::withoutSourceInfo(*typeSystem);
    stsBuilder.addDefinition("meta.com/S");
    auto sts = *std::move(stsBuilder).build();
    auto it = sts.types()->find("meta.com/S");
    ASSERT_NE(it, sts.types()->end());
    const auto& serializableDef = *it->second.definition();

    auto typeRefDigest = hasher(typeRef);
    auto serializableDefDigest = hasher(serializableDef);
    EXPECT_EQ(typeRefDigest, serializableDefDigest);
  }
}

/**
 * Test that runtime TypeId and serializable TypeIdUnion produce the same
 * digest for all type kinds: primitives, user-defined, and containers.
 *
 * For each TypeId, we compare:
 *   1. The runtime TypeId hash
 *   2. The serializable TypeIdUnion hash
 *
 * These must match to ensure hash equivalence between runtime and serialized
 * representations.
 */
TEST(TypeSystemDigestTest, TypeIdHashingEquivalence) {
  auto assertTypeIdHashEquivalence = [](const TypeId& typeId) {
    const TypeIdUnion& typeIdUnion = typeId.toThrift();
    // Verify that kind and getType have the same underlying value
    ASSERT_EQ(
        static_cast<int>(typeId.kind()),
        static_cast<int>(typeIdUnion.getType()))
        << "TypeId name: " << typeId.name();
    assertDigestEq(typeId, typeIdUnion);
  };

  // --- Primitives ---
  {
    assertTypeIdHashEquivalence(TypeId::Bool());
    assertTypeIdHashEquivalence(TypeId::Byte());
    assertTypeIdHashEquivalence(TypeId::I16());
    assertTypeIdHashEquivalence(TypeId::I32());
    assertTypeIdHashEquivalence(TypeId::I64());
    assertTypeIdHashEquivalence(TypeId::Float());
    assertTypeIdHashEquivalence(TypeId::Double());
    assertTypeIdHashEquivalence(TypeId::String());
    assertTypeIdHashEquivalence(TypeId::Binary());
    assertTypeIdHashEquivalence(TypeId::Any());
  }

  // --- User-defined types (URI) ---
  {
    assertTypeIdHashEquivalence(TypeIds::uri("meta.com/MyStruct"));
    assertTypeIdHashEquivalence(TypeIds::uri("meta.com/OtherStruct"));

    // Different URIs should have different digests
    assertDigestNeq(
        TypeIds::uri("meta.com/MyStruct"),
        TypeIds::uri("meta.com/OtherStruct"));
  }

  // --- Containers with primitive element types ---
  {
    assertTypeIdHashEquivalence(TypeIds::list(TypeId::I32()));
    assertTypeIdHashEquivalence(TypeIds::set(TypeId::String()));
    assertTypeIdHashEquivalence(TypeIds::map(TypeId::String(), TypeId::I64()));
  }

  // --- Containers with user-defined element types ---
  {
    assertTypeIdHashEquivalence(
        TypeIds::list(TypeIds::uri("meta.com/MyStruct")));
    assertTypeIdHashEquivalence(TypeIds::set(TypeIds::uri("meta.com/MyEnum")));
    assertTypeIdHashEquivalence(
        TypeIds::map(
            TypeIds::uri("meta.com/MyKey"), TypeIds::uri("meta.com/MyValue")));
  }

  // --- Nested containers ---
  {
    assertTypeIdHashEquivalence(TypeIds::list(TypeIds::list(TypeId::I32())));
    assertTypeIdHashEquivalence(
        TypeIds::map(TypeId::String(), TypeIds::list(TypeId::I64())));
    assertTypeIdHashEquivalence(
        TypeIds::set(
            TypeIds::map(TypeId::I32(), TypeIds::uri("meta.com/Value"))));
  }

  // --- All primitives are different from each other ---
  {
    assertDigestNeq(TypeId::Bool(), TypeId::Byte());
    assertDigestNeq(TypeId::Byte(), TypeId::I16());
    assertDigestNeq(TypeId::I16(), TypeId::I32());
    assertDigestNeq(TypeId::I32(), TypeId::I64());
    assertDigestNeq(TypeId::I64(), TypeId::Float());
    assertDigestNeq(TypeId::Float(), TypeId::Double());
    assertDigestNeq(TypeId::Double(), TypeId::String());
    assertDigestNeq(TypeId::String(), TypeId::Binary());
    assertDigestNeq(TypeId::Binary(), TypeId::Any());
  }

  // --- Container types are different from primitives ---
  {
    assertDigestNeq(TypeId::I32(), TypeIds::list(TypeId::I32()));
    assertDigestNeq(TypeId::I32(), TypeIds::set(TypeId::I32()));
    assertDigestNeq(TypeId::I32(), TypeIds::map(TypeId::I32(), TypeId::I32()));
  }

  // --- Different container element types produce different digests ---
  {
    assertDigestNeq(TypeIds::list(TypeId::I32()), TypeIds::list(TypeId::I64()));
    assertDigestNeq(
        TypeIds::set(TypeId::I32()), TypeIds::set(TypeId::String()));
    assertDigestNeq(
        TypeIds::map(TypeId::I32(), TypeId::I32()),
        TypeIds::map(TypeId::I64(), TypeId::I32()));
    assertDigestNeq(
        TypeIds::map(TypeId::I32(), TypeId::I32()),
        TypeIds::map(TypeId::I32(), TypeId::I64()));
  }

  // --- Any type in containers ---
  {
    assertTypeIdHashEquivalence(TypeIds::list(TypeId::Any()));
    assertTypeIdHashEquivalence(TypeIds::set(TypeId::Any()));
    assertTypeIdHashEquivalence(TypeIds::map(TypeId::String(), TypeId::Any()));
    assertTypeIdHashEquivalence(TypeIds::map(TypeId::Any(), TypeId::I32()));

    // Any in containers should differ from other element types
    assertDigestNeq(TypeIds::list(TypeId::Any()), TypeIds::list(TypeId::I32()));
    assertDigestNeq(
        TypeIds::set(TypeId::Any()), TypeIds::set(TypeId::String()));
    assertDigestNeq(
        TypeIds::map(TypeId::String(), TypeId::Any()),
        TypeIds::map(TypeId::String(), TypeId::I64()));
  }

  // --- Any is distinct from user-defined types ---
  {
    // Any should differ from any URI-based type
    assertDigestNeq(TypeId::Any(), TypeIds::uri("facebook.com/thrift/Any"));
    assertDigestNeq(TypeId::Any(), TypeIds::uri("apache.thrift.type.Any"));
  }
}

/**
 * Test that runtime TypeSystem and SerializableTypeSystem produce the same
 * digest for equivalent type systems.
 */
TEST(TypeSystemDigestTest, TypeSystemMatchesSerializableTypeSystem) {
  // Build a runtime TypeSystem with multiple types
  TypeSystemBuilder builder;
  builder.addType(
      "meta.com/Person",
      def::Struct({
          def::Field(
              def::Identity(1, "name"), def::AlwaysPresent, TypeId::String()),
          def::Field(def::Identity(2, "age"), def::Optional, TypeId::I32()),
      }));
  builder.addType(
      "meta.com/Status",
      def::Enum({{"ACTIVE", 1}, {"INACTIVE", 2}, {"PENDING", 3}}));
  builder.addType("meta.com/UserId", def::OpaqueAlias(TypeId::I64()));
  auto typeSystem = std::move(builder).build();

  // Convert to SerializableTypeSystem
  auto stsBuilder =
      SerializableTypeSystemBuilder::withoutSourceInfo(*typeSystem);
  stsBuilder.addDefinition("meta.com/Person");
  stsBuilder.addDefinition("meta.com/Status");
  stsBuilder.addDefinition("meta.com/UserId");
  auto sts = *std::move(stsBuilder).build();

  TypeSystemHasher hasher;
  EXPECT_EQ(hasher(*typeSystem), hasher(sts));
}

/**
 * Test SerializableRecord hash equivalence through field custom defaults.
 * This exercises all record kinds via the internal hashing path.
 */
TEST(TypeSystemDigestTest, FieldDefaultRecordKindsMatchSerializable) {
  auto testDefaultEquivalence = [](SerializableRecord defaultValue,
                                   std::string_view testName) {
    auto structDef = def::Struct({def::Field(
        def::Identity(1, "f"),
        def::AlwaysPresent,
        TypeId::I32(), // Type doesn't matter for this test
        std::move(defaultValue))});

    TypeSystemBuilder builder;
    builder.addType("meta.com/S", structDef);
    auto typeSystem = std::move(builder).build();

    auto defRef = typeSystem->getUserDefinedTypeOrThrow("meta.com/S");
    SerializableTypeDefinition serializableDef;
    serializableDef.structDef_ref() = structDef;

    TypeSystemHasher hasher;
    EXPECT_EQ(hasher(TypeRef::fromDefinition(defRef)), hasher(serializableDef))
        << "Failed for: " << testName;
  };

  // Primitives
  testDefaultEquivalence(SerializableRecord::Bool{true}, "Bool");
  testDefaultEquivalence(SerializableRecord::Int8{42}, "Int8");
  testDefaultEquivalence(SerializableRecord::Int16{1000}, "Int16");
  testDefaultEquivalence(SerializableRecord::Int32{100000}, "Int32");
  testDefaultEquivalence(SerializableRecord::Int64{9999999999LL}, "Int64");
  testDefaultEquivalence(SerializableRecord::Float32{3.14f}, "Float32");
  testDefaultEquivalence(SerializableRecord::Float64{2.71828}, "Float64");
  testDefaultEquivalence(SerializableRecord::Text{"hello"}, "Text");

  // ByteArray
  testDefaultEquivalence(
      SerializableRecord::ByteArray{folly::IOBuf::copyBuffer("binary")},
      "ByteArray");

  // FieldSet (nested struct-like)
  testDefaultEquivalence(
      SerializableRecord::FieldSet{
          {FieldId{1}, SerializableRecord::Int32{1}},
          {FieldId{2}, SerializableRecord::Text{"nested"}}},
      "FieldSet");

  // List
  testDefaultEquivalence(
      SerializableRecord::List{
          SerializableRecord::Int32{1},
          SerializableRecord::Int32{2},
          SerializableRecord::Int32{3}},
      "List");

  // Set
  testDefaultEquivalence(
      SerializableRecord::Set{
          SerializableRecord::Text{"a"}, SerializableRecord::Text{"b"}},
      "Set");

  // Map
  testDefaultEquivalence(
      SerializableRecord::Map{
          {SerializableRecord::Text{"key1"}, SerializableRecord::Int32{100}},
          {SerializableRecord::Text{"key2"}, SerializableRecord::Int32{200}}},
      "Map");

  // Deeply nested
  testDefaultEquivalence(
      SerializableRecord::List{SerializableRecord::FieldSet{
          {FieldId{1}, SerializableRecord::Text{"nested"}}}},
      "NestedListOfFieldSet");
}

/**
 * Test that different SerializableRecord values produce different digests.
 */
TEST(TypeSystemDigestTest, DifferentFieldDefaultsProduceDifferentDigests) {
  auto makeStructWithDefault = [](SerializableRecord defaultValue) {
    return def::Struct({def::Field(
        def::Identity(1, "f"),
        def::AlwaysPresent,
        TypeId::I32(),
        defaultValue)});
  };

  auto struct1 = makeStructWithDefault(SerializableRecord::Int32{1});
  auto struct2 = makeStructWithDefault(SerializableRecord::Int32{2});
  assertDigestNeq(struct1, struct2);

  auto structBool = makeStructWithDefault(SerializableRecord::Bool{true});
  auto structInt = makeStructWithDefault(SerializableRecord::Int32{1});
  assertDigestNeq(structBool, structInt);

  auto structEmpty = makeStructWithDefault(SerializableRecord::List{});
  auto structOne = makeStructWithDefault(
      SerializableRecord::List{SerializableRecord::Int32{1}});
  assertDigestNeq(structEmpty, structOne);
}

/**
 * Test empty TypeId behavior.
 */
TEST(TypeSystemDigestTest, EmptyTypeIdProducesConsistentDigest) {
  TypeIdUnion emptyUnion1, emptyUnion2;
  ASSERT_EQ(emptyUnion1.getType(), TypeIdUnion::Type::__EMPTY__);

  TypeSystemHasher hasher;
  // Empty unions should hash consistently
  EXPECT_EQ(hasher(emptyUnion1), hasher(emptyUnion2));

  // Empty should differ from any valid type
  TypeIdUnion boolUnion = TypeIds::Bool.toThrift();
  EXPECT_NE(hasher(emptyUnion1), hasher(boolUnion));
}

/**
 * Test Any type as field type in struct definitions.
 * Ensures Any is properly handled in type system context.
 */
TEST(TypeSystemDigestTest, AnyTypeInStructFieldMatchesSerializable) {
  // Build a struct with an Any field
  auto structDef = def::Struct({
      def::Field(
          def::Identity(1, "payload"), def::AlwaysPresent, TypeId::Any()),
      def::Field(
          def::Identity(2, "metadata"),
          def::Optional,
          TypeIds::map(TypeId::String(), TypeId::Any())),
  });

  TypeSystemBuilder builder;
  builder.addType("meta.com/Container", structDef);
  auto typeSystem = std::move(builder).build();

  auto defRef = typeSystem->getUserDefinedTypeOrThrow("meta.com/Container");
  SerializableTypeDefinition serializableDef;
  serializableDef.structDef_ref() = structDef;

  TypeSystemHasher hasher;
  EXPECT_EQ(hasher(TypeRef::fromDefinition(defRef)), hasher(serializableDef));

  // Any field should differ from other field types
  auto structWithI32 = def::Struct({
      def::Field(
          def::Identity(1, "payload"), def::AlwaysPresent, TypeId::I32()),
  });
  assertDigestNeq(structDef, structWithI32);

  // Any field should differ from user-defined type field
  auto structWithUri = def::Struct({
      def::Field(
          def::Identity(1, "payload"),
          def::AlwaysPresent,
          TypeIds::uri("meta.com/Payload")),
  });
  assertDigestNeq(structDef, structWithUri);
}

/**
 * Test that an opaque alias (typedef) produces a different digest than its
 * target type. This ensures type aliases are not conflated with their
 * underlying types.
 */
TEST(TypeSystemDigestTest, OpaqueAliasDiffersFromTargetType) {
  TypeSystemHasher hasher;

  // Create an opaque alias targeting I64
  auto aliasDef = def::OpaqueAlias(TypeId::I64());

  TypeSystemBuilder builder;
  builder.addType("meta.com/UserId", aliasDef);
  auto typeSystem = std::move(builder).build();

  auto aliasRef = typeSystem->getUserDefinedTypeOrThrow("meta.com/UserId");

  // Hash the opaque alias definition
  auto aliasDigest = hasher(TypeRef::fromDefinition(aliasRef));

  // Hash the raw I64 TypeId
  auto i64TypeIdDigest = hasher(TypeId::I64());

  // They should differ - an alias is not the same as its target
  EXPECT_NE(aliasDigest, i64TypeIdDigest);

  // Also verify different opaque aliases with same target type differ by URI
  TypeSystemBuilder builder2;
  builder2.addType("meta.com/AccountId", aliasDef);
  auto typeSystem2 = std::move(builder2).build();
  auto accountIdRef =
      typeSystem2->getUserDefinedTypeOrThrow("meta.com/AccountId");

  // Note: TypeRef hashing doesn't include the URI, only the definition content
  // So two aliases with the same target type should have the same definition
  // digest This tests that the alias definition itself is hashed correctly
  SerializableTypeDefinition serializableDef;
  serializableDef.opaqueAliasDef_ref() = aliasDef;
  EXPECT_EQ(hasher(TypeRef::fromDefinition(aliasRef)), hasher(serializableDef));
  EXPECT_EQ(
      hasher(TypeRef::fromDefinition(accountIdRef)), hasher(serializableDef));
}

/**
 * Test that a field with a default value hashes differently from a field
 * without a default value, even when using the serializable definition
 * directly.
 */
TEST(TypeSystemDigestTest, SerializableFieldWithVsWithoutDefaultDiffer) {
  // Field with default
  auto structWithDefault = def::Struct({def::Field(
      def::Identity(1, "value"),
      def::AlwaysPresent,
      TypeId::I32(),
      SerializableRecord::Int32{42})});

  // Field without default
  auto structWithoutDefault = def::Struct({def::Field(
      def::Identity(1, "value"), def::AlwaysPresent, TypeId::I32())});

  // Verify they produce different digests
  assertDigestNeq(structWithDefault, structWithoutDefault);

  // Also verify via SerializableTypeDefinition
  SerializableTypeDefinition defWithDefault, defWithoutDefault;
  defWithDefault.structDef_ref() = structWithDefault;
  defWithoutDefault.structDef_ref() = structWithoutDefault;

  TypeSystemHasher hasher;
  EXPECT_NE(hasher(defWithDefault), hasher(defWithoutDefault));

  // Test with different presence qualifiers too
  auto structOptionalWithDefault = def::Struct({def::Field(
      def::Identity(1, "value"),
      def::Optional,
      TypeId::String(),
      SerializableRecord::Text{"default"})});

  auto structOptionalWithoutDefault = def::Struct(
      {def::Field(def::Identity(1, "value"), def::Optional, TypeId::String())});

  assertDigestNeq(structOptionalWithDefault, structOptionalWithoutDefault);
}
