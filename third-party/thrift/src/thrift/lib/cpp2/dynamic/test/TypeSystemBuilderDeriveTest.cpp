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
#include <thrift/lib/cpp2/runtime/SchemaRegistry.h>
#include <thrift/lib/cpp2/schema/SyntaxGraph.h>

#include <thrift/lib/cpp2/schema/test/gen-cpp2/syntax_graph_handlers.h>
#include <thrift/lib/cpp2/schema/test/gen-cpp2/syntax_graph_types.h>

#include <memory>

namespace apache::thrift::type_system {

using def = TypeSystemBuilder::DefinitionHelper;

namespace {

// Helper to create a simple programmatic TypeSystem for testing
std::shared_ptr<TypeSystem> makeSimpleTypeSystem() {
  TypeSystemBuilder builder;
  builder.addType(
      "test.com/base/BaseStruct",
      def::Struct({
          def::Field(def::Identity(1, "field1"), def::Optional, TypeIds::I32),
      }),
      def::SourceInfo("base.thrift", "BaseStruct"));
  builder.addType(
      "test.com/base/BaseEnum",
      def::Enum({{"VALUE1", 1}, {"VALUE2", 2}}),
      def::SourceInfo("base.thrift", "BaseEnum"));
  builder.addType(
      "test.com/base/BaseUnion",
      def::Union({
          def::Field(def::Identity(1, "option1"), def::Optional, TypeIds::I32),
          def::Field(
              def::Identity(2, "option2"), def::Optional, TypeIds::String),
      }),
      def::SourceInfo("base.thrift", "BaseUnion"));
  return std::move(builder).build();
}

// Helper to create a SyntaxGraph-backed TypeSystem
std::shared_ptr<const TypeSystem> makeSyntaxGraphTypeSystem() {
  auto schema =
      apache::thrift::ServiceHandler<syntax_graph::test::TestService>()
          .getServiceSchema()
          .value()
          .schema;
  // Use shared_ptr aliasing to own the SyntaxGraph while exposing its
  // TypeSystem
  auto graph = std::make_shared<syntax_graph::SyntaxGraph>(
      syntax_graph::SyntaxGraph::fromSchema(std::move(schema)));
  return std::shared_ptr<const TypeSystem>(graph, &graph->asTypeSystem());
}

} // namespace

// ============================================================================
// Basic Derivation Tests
// ============================================================================

TEST(TypeSystemBuilderDeriveTest, DeriveFromSimpleTypeSystem_ResolveBaseType) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;

  // Add an overlay type
  builder.addType("test.com/overlay/OverlayStruct", def::Struct({}));

  auto overlay = std::move(builder).buildDerivedFrom(base);

  // Should resolve base type
  auto baseRef = overlay->getUserDefinedType("test.com/base/BaseStruct");
  ASSERT_TRUE(baseRef.has_value());
  EXPECT_EQ(baseRef->uri(), "test.com/base/BaseStruct");
  EXPECT_EQ(baseRef->asStruct().fields().size(), 1);

  // Should resolve overlay type
  auto overlayRef =
      overlay->getUserDefinedType("test.com/overlay/OverlayStruct");
  ASSERT_TRUE(overlayRef.has_value());
  EXPECT_EQ(overlayRef->uri(), "test.com/overlay/OverlayStruct");
}

TEST(TypeSystemBuilderDeriveTest, DeriveFromSyntaxGraph_ResolveBaseType) {
  auto base = makeSyntaxGraphTypeSystem();
  TypeSystemBuilder builder;

  // Add an overlay type
  builder.addType("test.com/overlay/OverlayStruct", def::Struct({}));

  auto overlay = std::move(builder).buildDerivedFrom(base);

  // Should resolve base type from SyntaxGraph
  auto baseRef =
      overlay->getUserDefinedType("meta.com/thrift_test/TestRecursiveStruct");
  ASSERT_TRUE(baseRef.has_value());
  EXPECT_EQ(baseRef->uri(), "meta.com/thrift_test/TestRecursiveStruct");

  // Should resolve overlay type
  auto overlayRef =
      overlay->getUserDefinedType("test.com/overlay/OverlayStruct");
  ASSERT_TRUE(overlayRef.has_value());
}

TEST(TypeSystemBuilderDeriveTest, DeriveFromSchemaRegistry_ResolveBaseType) {
  // Use shared_ptr with no-op deleter to wrap the singleton SchemaRegistry
  auto base = std::shared_ptr<const TypeSystem>(
      &SchemaRegistry::get(), [](const TypeSystem*) { /* no-op */ });
  TypeSystemBuilder builder;

  // Add an overlay type
  builder.addType("test.com/overlay/OverlayStruct", def::Struct({}));

  auto overlay = std::move(builder).buildDerivedFrom(base);

  // Should resolve overlay type
  auto overlayRef =
      overlay->getUserDefinedType("test.com/overlay/OverlayStruct");
  ASSERT_TRUE(overlayRef.has_value());

  // Should be able to resolve types from SchemaRegistry if any are registered
  // (The specific URIs available depend on what's linked into the test binary)
}

// ============================================================================
// Overlay Type References Base Tests
// ============================================================================

TEST(TypeSystemBuilderDeriveTest, OverlayStructReferencesBaseStruct) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;

  // Overlay struct with field referencing base struct
  builder.addType(
      "test.com/overlay/OverlayStruct",
      def::Struct({
          def::Field(
              def::Identity(1, "baseRef"),
              def::Optional,
              TypeIds::uri("test.com/base/BaseStruct")),
      }));

  auto overlay = std::move(builder).buildDerivedFrom(base);

  auto overlayRef =
      overlay->getUserDefinedType("test.com/overlay/OverlayStruct");
  ASSERT_TRUE(overlayRef.has_value());

  // Get the base type directly
  auto baseStructRef = base->getUserDefinedType("test.com/base/BaseStruct");
  ASSERT_TRUE(baseStructRef.has_value());

  const auto& fields = overlayRef->asStruct().fields();
  ASSERT_EQ(fields.size(), 1);
  EXPECT_EQ(fields[0].identity().name(), "baseRef");

  // The field type should resolve to the SAME base struct (pointer equality)
  EXPECT_TRUE(fields[0].type().isStruct());
  EXPECT_EQ(&fields[0].type().asStruct(), &baseStructRef->asStruct());
}

TEST(TypeSystemBuilderDeriveTest, OverlayStructReferencesBaseEnum) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;

  builder.addType(
      "test.com/overlay/OverlayStruct",
      def::Struct({
          def::Field(
              def::Identity(1, "enumField"),
              def::Optional,
              TypeIds::uri("test.com/base/BaseEnum")),
      }));

  auto overlay = std::move(builder).buildDerivedFrom(base);

  auto overlayRef =
      overlay->getUserDefinedType("test.com/overlay/OverlayStruct");
  ASSERT_TRUE(overlayRef.has_value());

  // Get the base enum directly
  auto baseEnumRef = base->getUserDefinedType("test.com/base/BaseEnum");
  ASSERT_TRUE(baseEnumRef.has_value());

  const auto& field = overlayRef->asStruct().fields()[0];
  EXPECT_TRUE(field.type().isEnum());
  // Pointer equality - should resolve to the SAME enum definition
  EXPECT_EQ(&field.type().asEnum(), &baseEnumRef->asEnum());
}

TEST(TypeSystemBuilderDeriveTest, OverlayUnionReferencesBaseType) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;

  builder.addType(
      "test.com/overlay/OverlayUnion",
      def::Union({
          def::Field(
              def::Identity(1, "baseOption"),
              def::Optional,
              TypeIds::uri("test.com/base/BaseStruct")),
      }));

  auto overlay = std::move(builder).buildDerivedFrom(base);

  auto overlayRef =
      overlay->getUserDefinedType("test.com/overlay/OverlayUnion");
  ASSERT_TRUE(overlayRef.has_value());
  EXPECT_TRUE(overlayRef->isUnion());

  // Get the base type directly
  auto baseStructRef = base->getUserDefinedType("test.com/base/BaseStruct");
  ASSERT_TRUE(baseStructRef.has_value());

  const auto& field = overlayRef->asUnion().fields()[0];
  EXPECT_EQ(field.type().asStruct().uri(), "test.com/base/BaseStruct");
  // Pointer equality - should resolve to the SAME base struct
  EXPECT_EQ(&field.type().asStruct(), &baseStructRef->asStruct());
}

TEST(TypeSystemBuilderDeriveTest, OverlayListOfBaseType) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;

  builder.addType(
      "test.com/overlay/OverlayStruct",
      def::Struct({
          def::Field(
              def::Identity(1, "listField"),
              def::Optional,
              TypeIds::list(TypeIds::uri("test.com/base/BaseStruct"))),
      }));

  auto overlay = std::move(builder).buildDerivedFrom(base);

  auto overlayRef =
      overlay->getUserDefinedType("test.com/overlay/OverlayStruct");
  ASSERT_TRUE(overlayRef.has_value());

  // Get the base type directly
  auto baseStructRef = base->getUserDefinedType("test.com/base/BaseStruct");
  ASSERT_TRUE(baseStructRef.has_value());

  const auto& field = overlayRef->asStruct().fields()[0];
  EXPECT_TRUE(field.type().isList());
  EXPECT_EQ(
      field.type().asList().elementType().asStruct().uri(),
      "test.com/base/BaseStruct");
  // Pointer equality - list element should resolve to the SAME base struct
  EXPECT_EQ(
      &field.type().asList().elementType().asStruct(),
      &baseStructRef->asStruct());
}

TEST(TypeSystemBuilderDeriveTest, OverlayMapWithBaseKeyAndValue) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;

  builder.addType(
      "test.com/overlay/OverlayStruct",
      def::Struct({
          def::Field(
              def::Identity(1, "mapField"),
              def::Optional,
              TypeIds::map(
                  TypeIds::uri("test.com/base/BaseEnum"),
                  TypeIds::uri("test.com/base/BaseStruct"))),
      }));

  auto overlay = std::move(builder).buildDerivedFrom(base);

  auto overlayRef =
      overlay->getUserDefinedType("test.com/overlay/OverlayStruct");
  ASSERT_TRUE(overlayRef.has_value());

  // Get the base types directly
  auto baseEnumRef = base->getUserDefinedType("test.com/base/BaseEnum");
  auto baseStructRef = base->getUserDefinedType("test.com/base/BaseStruct");
  ASSERT_TRUE(baseEnumRef.has_value());
  ASSERT_TRUE(baseStructRef.has_value());

  const auto& field = overlayRef->asStruct().fields()[0];
  EXPECT_TRUE(field.type().isMap());
  EXPECT_EQ(
      field.type().asMap().keyType().asEnum().uri(), "test.com/base/BaseEnum");
  EXPECT_EQ(
      field.type().asMap().valueType().asStruct().uri(),
      "test.com/base/BaseStruct");
  // Pointer equality - map key and value should resolve to SAME base types
  EXPECT_EQ(&field.type().asMap().keyType().asEnum(), &baseEnumRef->asEnum());
  EXPECT_EQ(
      &field.type().asMap().valueType().asStruct(), &baseStructRef->asStruct());
}

// ============================================================================
// Conflict Detection Tests
// ============================================================================

TEST(TypeSystemBuilderDeriveTest, ConflictWithBaseType_ThrowsOnBuild) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;

  // Try to add a type with the same URI as base
  builder.addType("test.com/base/BaseStruct", def::Struct({}));

  EXPECT_THROW(
      { std::move(builder).buildDerivedFrom(base); }, InvalidTypeError);
}

TEST(TypeSystemBuilderDeriveTest, ConflictWithBaseEnum_ThrowsOnBuild) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;

  builder.addType("test.com/base/BaseEnum", def::Enum({{"DIFFERENT", 99}}));

  EXPECT_THROW(
      { std::move(builder).buildDerivedFrom(base); }, InvalidTypeError);
}

TEST(TypeSystemBuilderDeriveTest, ConflictWithBaseUnion_ThrowsOnBuild) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;

  builder.addType("test.com/base/BaseUnion", def::Union({}));

  EXPECT_THROW(
      { std::move(builder).buildDerivedFrom(base); }, InvalidTypeError);
}

TEST(
    TypeSystemBuilderDeriveTest,
    ConflictWithBaseSourceIdentifier_ThrowsOnBuild) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;

  // Try to add a type with a different URI but same source location/name as
  // base The base has "BaseStruct" at "base.thrift"
  builder.addType(
      "test.com/overlay/DifferentUri",
      def::Struct({}),
      def::SourceInfo("base.thrift", "BaseStruct"));

  EXPECT_THROW(
      { std::move(builder).buildDerivedFrom(base); }, InvalidTypeError);
}

// ============================================================================
// TypeSystem Interface Method Tests
// ============================================================================

TEST(TypeSystemBuilderDeriveTest, GetKnownUris_MergesBaseAndOverlay) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;
  builder.addType("test.com/overlay/OverlayStruct", def::Struct({}));

  auto overlay = std::move(builder).buildDerivedFrom(base);

  auto knownUris = overlay->getKnownUris();
  ASSERT_TRUE(knownUris.has_value());

  // Should contain both base and overlay URIs
  EXPECT_TRUE(knownUris->contains("test.com/base/BaseStruct"));
  EXPECT_TRUE(knownUris->contains("test.com/base/BaseEnum"));
  EXPECT_TRUE(knownUris->contains("test.com/base/BaseUnion"));
  EXPECT_TRUE(knownUris->contains("test.com/overlay/OverlayStruct"));
  EXPECT_EQ(knownUris->size(), 4);
}

TEST(TypeSystemBuilderDeriveTest, GetUserDefinedType_FallsBackToBase) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;
  auto overlay = std::move(builder).buildDerivedFrom(base);

  // Lookup type that only exists in base
  auto ref = overlay->getUserDefinedType("test.com/base/BaseStruct");
  ASSERT_TRUE(ref.has_value());
  EXPECT_EQ(ref->uri(), "test.com/base/BaseStruct");
}

TEST(TypeSystemBuilderDeriveTest, GetUserDefinedType_OverlayTypeFound) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;
  builder.addType("test.com/overlay/OverlayStruct", def::Struct({}));
  auto overlay = std::move(builder).buildDerivedFrom(base);

  // Lookup type that exists in overlay
  auto ref = overlay->getUserDefinedType("test.com/overlay/OverlayStruct");
  ASSERT_TRUE(ref.has_value());
  EXPECT_EQ(ref->uri(), "test.com/overlay/OverlayStruct");
}

TEST(
    TypeSystemBuilderDeriveTest,
    GetUserDefinedTypeBySourceIdentifier_FallsBackToBase) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;
  builder.addType(
      "test.com/overlay/OverlayStruct",
      def::Struct({}),
      def::SourceInfo("overlay.thrift", "OverlayStruct"));
  auto overlay = std::move(builder).buildDerivedFrom(base);

  // Lookup by source identifier from base
  SourceIdentifierView baseId{"base.thrift", "BaseStruct"};
  auto baseRef = overlay->getUserDefinedTypeBySourceIdentifier(baseId);
  ASSERT_TRUE(baseRef.has_value());
  EXPECT_EQ(baseRef->uri(), "test.com/base/BaseStruct");

  // Lookup by source identifier from overlay
  SourceIdentifierView overlayId{"overlay.thrift", "OverlayStruct"};
  auto overlayRef = overlay->getUserDefinedTypeBySourceIdentifier(overlayId);
  ASSERT_TRUE(overlayRef.has_value());
  EXPECT_EQ(overlayRef->uri(), "test.com/overlay/OverlayStruct");
}

TEST(TypeSystemBuilderDeriveTest, GetUserDefinedTypesAtLocation_MergesBoth) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;
  builder.addType(
      "test.com/overlay/OverlayInBase",
      def::Struct({}),
      def::SourceInfo("base.thrift", "OverlayInBase"));
  auto overlay = std::move(builder).buildDerivedFrom(base);

  // Get types at base.thrift - should have both base and overlay types
  auto types = overlay->getUserDefinedTypesAtLocation("base.thrift");
  EXPECT_TRUE(types.contains("BaseStruct"));
  EXPECT_TRUE(types.contains("BaseEnum"));
  EXPECT_TRUE(types.contains("BaseUnion"));
  EXPECT_TRUE(types.contains("OverlayInBase"));
}

// ============================================================================
// Container Type Tests
// ============================================================================

TEST(TypeSystemBuilderDeriveTest, ContainerOfBaseType_ResolvesCorrectly) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;

  builder.addType(
      "test.com/overlay/WithContainers",
      def::Struct({
          def::Field(
              def::Identity(1, "listOfBase"),
              def::Optional,
              TypeIds::list(TypeIds::uri("test.com/base/BaseStruct"))),
          def::Field(
              def::Identity(2, "setOfBase"),
              def::Optional,
              TypeIds::set(TypeIds::uri("test.com/base/BaseEnum"))),
      }));

  auto overlay = std::move(builder).buildDerivedFrom(base);

  auto ref = overlay->getUserDefinedType("test.com/overlay/WithContainers");
  ASSERT_TRUE(ref.has_value());

  // Get the base types directly
  auto baseStructRef = base->getUserDefinedType("test.com/base/BaseStruct");
  auto baseEnumRef = base->getUserDefinedType("test.com/base/BaseEnum");
  ASSERT_TRUE(baseStructRef.has_value());
  ASSERT_TRUE(baseEnumRef.has_value());

  const auto& fields = ref->asStruct().fields();
  EXPECT_EQ(
      fields[0].type().asList().elementType().asStruct().uri(),
      "test.com/base/BaseStruct");
  EXPECT_EQ(
      fields[1].type().asSet().elementType().asEnum().uri(),
      "test.com/base/BaseEnum");
  // Pointer equality - container elements should resolve to SAME base types
  EXPECT_EQ(
      &fields[0].type().asList().elementType().asStruct(),
      &baseStructRef->asStruct());
  EXPECT_EQ(
      &fields[1].type().asSet().elementType().asEnum(), &baseEnumRef->asEnum());
}

TEST(TypeSystemBuilderDeriveTest, NestedContainers_MixedSources) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;

  builder.addType(
      "test.com/overlay/OverlayValue",
      def::Struct({
          def::Field(def::Identity(1, "value"), def::Optional, TypeIds::I64),
      }));

  builder.addType(
      "test.com/overlay/WithNestedContainers",
      def::Struct({
          def::Field(
              def::Identity(1, "mapField"),
              def::Optional,
              TypeIds::map(
                  TypeIds::uri("test.com/base/BaseEnum"),
                  TypeIds::list(
                      TypeIds::uri("test.com/overlay/OverlayValue")))),
      }));

  auto overlay = std::move(builder).buildDerivedFrom(base);

  auto ref =
      overlay->getUserDefinedType("test.com/overlay/WithNestedContainers");
  ASSERT_TRUE(ref.has_value());

  const auto& mapField = ref->asStruct().fields()[0];
  EXPECT_TRUE(mapField.type().isMap());
  EXPECT_EQ(
      mapField.type().asMap().keyType().asEnum().uri(),
      "test.com/base/BaseEnum");
  EXPECT_EQ(
      mapField.type()
          .asMap()
          .valueType()
          .asList()
          .elementType()
          .asStruct()
          .uri(),
      "test.com/overlay/OverlayValue");
}

// ============================================================================
// Edge Case Tests
// ============================================================================

TEST(TypeSystemBuilderDeriveTest, EmptyOverlay_DelegatesToBase) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;

  // Build with no overlay types
  auto overlay = std::move(builder).buildDerivedFrom(base);

  // Should still resolve base types
  auto ref = overlay->getUserDefinedType("test.com/base/BaseStruct");
  ASSERT_TRUE(ref.has_value());

  // Known URIs should match base
  auto knownUris = overlay->getKnownUris();
  ASSERT_TRUE(knownUris.has_value());
  EXPECT_EQ(knownUris->size(), 3);
}

TEST(TypeSystemBuilderDeriveTest, BaseTypeSystemLifetime_KeptAlive) {
  std::weak_ptr<TypeSystem> weakBase;
  std::unique_ptr<TypeSystem> overlay;

  {
    auto base = makeSimpleTypeSystem();
    weakBase = base;
    TypeSystemBuilder builder;
    builder.addType("test.com/overlay/OverlayStruct", def::Struct({}));
    overlay = std::move(builder).buildDerivedFrom(base);
  }

  // Base should still be alive (held by overlay)
  EXPECT_FALSE(weakBase.expired());

  // Should still be able to resolve base types
  auto ref = overlay->getUserDefinedType("test.com/base/BaseStruct");
  ASSERT_TRUE(ref.has_value());
}

TEST(TypeSystemBuilderDeriveTest, ChainedDerivation_ThreeLevels) {
  // Level 1: base
  auto level1 = makeSimpleTypeSystem();

  // Level 2: derive from level1
  TypeSystemBuilder builder2;
  builder2.addType(
      "test.com/level2/Level2Struct",
      def::Struct({
          def::Field(
              def::Identity(1, "level1Ref"),
              def::Optional,
              TypeIds::uri("test.com/base/BaseStruct")),
      }));
  auto level2 = std::move(builder2).buildDerivedFrom(level1);

  // Level 3: derive from level2
  TypeSystemBuilder builder3;
  builder3.addType(
      "test.com/level3/Level3Struct",
      def::Struct({
          def::Field(
              def::Identity(1, "level2Ref"),
              def::Optional,
              TypeIds::uri("test.com/level2/Level2Struct")),
          def::Field(
              def::Identity(2, "level1Ref"),
              def::Optional,
              TypeIds::uri("test.com/base/BaseStruct")),
      }));
  auto level3 = std::move(builder3).buildDerivedFrom(
      std::shared_ptr<TypeSystem>(std::move(level2)));

  // Should resolve all levels
  EXPECT_TRUE(
      level3->getUserDefinedType("test.com/base/BaseStruct").has_value());
  EXPECT_TRUE(
      level3->getUserDefinedType("test.com/level2/Level2Struct").has_value());
  EXPECT_TRUE(
      level3->getUserDefinedType("test.com/level3/Level3Struct").has_value());

  // Verify the references work
  auto level3Ref = level3->getUserDefinedType("test.com/level3/Level3Struct");
  ASSERT_TRUE(level3Ref.has_value());
  const auto& fields = level3Ref->asStruct().fields();
  EXPECT_EQ(fields[0].type().asStruct().uri(), "test.com/level2/Level2Struct");
  EXPECT_EQ(fields[1].type().asStruct().uri(), "test.com/base/BaseStruct");
}

TEST(TypeSystemBuilderDeriveTest, DeriveFromNullptr_BehavesLikeStandalone) {
  TypeSystemBuilder builder;
  builder.addType("test.com/standalone/Struct", def::Struct({}));

  auto typeSystem = std::move(builder).buildDerivedFrom(nullptr);

  auto ref = typeSystem->getUserDefinedType("test.com/standalone/Struct");
  ASSERT_TRUE(ref.has_value());

  // Should not find base types (there is no base)
  EXPECT_FALSE(
      typeSystem->getUserDefinedType("test.com/base/BaseStruct").has_value());
}

TEST(TypeSystemBuilderDeriveTest, OverlayTypeReferencesOtherOverlayType) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;

  // Add two overlay types where one references the other
  builder.addType(
      "test.com/overlay/Inner",
      def::Struct({
          def::Field(def::Identity(1, "value"), def::Optional, TypeIds::I32),
      }));

  builder.addType(
      "test.com/overlay/Outer",
      def::Struct({
          def::Field(
              def::Identity(1, "inner"),
              def::Optional,
              TypeIds::uri("test.com/overlay/Inner")),
      }));

  auto overlay = std::move(builder).buildDerivedFrom(base);

  auto outerRef = overlay->getUserDefinedType("test.com/overlay/Outer");
  ASSERT_TRUE(outerRef.has_value());

  const auto& field = outerRef->asStruct().fields()[0];
  EXPECT_EQ(field.type().asStruct().uri(), "test.com/overlay/Inner");
}

TEST(
    TypeSystemBuilderDeriveTest,
    OverlayTypeReferencesMissingType_ThrowsOnBuild) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;

  // Reference a type that doesn't exist in base or overlay
  builder.addType(
      "test.com/overlay/BadRef",
      def::Struct({
          def::Field(
              def::Identity(1, "missing"),
              def::Optional,
              TypeIds::uri("test.com/nonexistent/Missing")),
      }));

  EXPECT_THROW(
      { std::move(builder).buildDerivedFrom(base); }, InvalidTypeError);
}

TEST(TypeSystemBuilderDeriveTest, RecursiveOverlayType) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;

  // Add a recursive overlay type
  builder.addType(
      "test.com/overlay/Recursive",
      def::Struct({
          def::Field(
              def::Identity(1, "self"),
              def::Optional,
              TypeIds::uri("test.com/overlay/Recursive")),
          def::Field(
              def::Identity(2, "baseRef"),
              def::Optional,
              TypeIds::uri("test.com/base/BaseStruct")),
      }));

  auto overlay = std::move(builder).buildDerivedFrom(base);

  auto ref = overlay->getUserDefinedType("test.com/overlay/Recursive");
  ASSERT_TRUE(ref.has_value());

  // Get the base type directly
  auto baseStructRef = base->getUserDefinedType("test.com/base/BaseStruct");
  ASSERT_TRUE(baseStructRef.has_value());

  const auto& fields = ref->asStruct().fields();
  EXPECT_EQ(fields[0].type().asStruct().uri(), "test.com/overlay/Recursive");
  // Verify it's actually recursive (same pointer)
  EXPECT_EQ(&fields[0].type().asStruct(), &ref->asStruct());
  // Also verify base reference works with pointer equality
  EXPECT_EQ(fields[1].type().asStruct().uri(), "test.com/base/BaseStruct");
  EXPECT_EQ(&fields[1].type().asStruct(), &baseStructRef->asStruct());
}

TEST(TypeSystemBuilderDeriveTest, MutuallyRecursiveOverlayTypes) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;

  // Add mutually recursive overlay types
  builder.addType(
      "test.com/overlay/TypeA",
      def::Struct({
          def::Field(
              def::Identity(1, "toB"),
              def::Optional,
              TypeIds::uri("test.com/overlay/TypeB")),
      }));

  builder.addType(
      "test.com/overlay/TypeB",
      def::Struct({
          def::Field(
              def::Identity(1, "toA"),
              def::Optional,
              TypeIds::uri("test.com/overlay/TypeA")),
      }));

  auto overlay = std::move(builder).buildDerivedFrom(base);

  auto refA = overlay->getUserDefinedType("test.com/overlay/TypeA");
  auto refB = overlay->getUserDefinedType("test.com/overlay/TypeB");
  ASSERT_TRUE(refA.has_value());
  ASSERT_TRUE(refB.has_value());

  EXPECT_EQ(
      refA->asStruct().fields()[0].type().asStruct().uri(),
      "test.com/overlay/TypeB");
  EXPECT_EQ(
      refB->asStruct().fields()[0].type().asStruct().uri(),
      "test.com/overlay/TypeA");
}

TEST(
    TypeSystemBuilderDeriveTest,
    GetUserDefinedType_NonExistentUri_ReturnsNullopt) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;
  builder.addType("test.com/overlay/OverlayStruct", def::Struct({}));
  auto overlay = std::move(builder).buildDerivedFrom(base);

  // Lookup non-existent type
  auto ref = overlay->getUserDefinedType("test.com/nonexistent/Type");
  EXPECT_FALSE(ref.has_value());
}

TEST(
    TypeSystemBuilderDeriveTest,
    GetSourceIdentifierForUserDefinedType_Overlay) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;
  builder.addType(
      "test.com/overlay/OverlayStruct",
      def::Struct({}),
      def::SourceInfo("overlay.thrift", "OverlayStruct"));
  auto overlay = std::move(builder).buildDerivedFrom(base);

  auto ref = overlay->getUserDefinedType("test.com/overlay/OverlayStruct");
  ASSERT_TRUE(ref.has_value());

  auto sourceId = overlay->getSourceIdentiferForUserDefinedType(*ref);
  ASSERT_TRUE(sourceId.has_value());
  EXPECT_EQ(sourceId->location, "overlay.thrift");
  EXPECT_EQ(sourceId->name, "OverlayStruct");
}

TEST(TypeSystemBuilderDeriveTest, GetSourceIdentifierForUserDefinedType_Base) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;
  auto overlay = std::move(builder).buildDerivedFrom(base);

  auto ref = overlay->getUserDefinedType("test.com/base/BaseStruct");
  ASSERT_TRUE(ref.has_value());

  auto sourceId = overlay->getSourceIdentiferForUserDefinedType(*ref);
  ASSERT_TRUE(sourceId.has_value());
  EXPECT_EQ(sourceId->location, "base.thrift");
  EXPECT_EQ(sourceId->name, "BaseStruct");
}

TEST(TypeSystemBuilderDeriveTest, OverlayOpaqueAlias) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;

  builder.addType(
      "test.com/overlay/MyAlias",
      def::OpaqueAlias(TypeIds::I64),
      def::SourceInfo("overlay.thrift", "MyAlias"));

  auto overlay = std::move(builder).buildDerivedFrom(base);

  auto ref = overlay->getUserDefinedType("test.com/overlay/MyAlias");
  ASSERT_TRUE(ref.has_value());
  EXPECT_TRUE(ref->isOpaqueAlias());
  EXPECT_EQ(ref->asOpaqueAlias().targetType().id(), TypeIds::I64);
}

TEST(TypeSystemBuilderDeriveTest, MultipleOverlayTypesWithPrimitiveFields) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;

  builder.addType(
      "test.com/overlay/AllPrimitives",
      def::Struct({
          def::Field(
              def::Identity(1, "boolField"), def::Optional, TypeIds::Bool),
          def::Field(
              def::Identity(2, "byteField"), def::Optional, TypeIds::Byte),
          def::Field(def::Identity(3, "i16Field"), def::Optional, TypeIds::I16),
          def::Field(def::Identity(4, "i32Field"), def::Optional, TypeIds::I32),
          def::Field(def::Identity(5, "i64Field"), def::Optional, TypeIds::I64),
          def::Field(
              def::Identity(6, "floatField"), def::Optional, TypeIds::Float),
          def::Field(
              def::Identity(7, "doubleField"), def::Optional, TypeIds::Double),
          def::Field(
              def::Identity(8, "stringField"), def::Optional, TypeIds::String),
          def::Field(
              def::Identity(9, "binaryField"), def::Optional, TypeIds::Binary),
      }));

  auto overlay = std::move(builder).buildDerivedFrom(base);

  auto ref = overlay->getUserDefinedType("test.com/overlay/AllPrimitives");
  ASSERT_TRUE(ref.has_value());
  EXPECT_EQ(ref->asStruct().fields().size(), 9);
}

TEST(TypeSystemBuilderDeriveTest, OverlaySetOfBaseType) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;

  builder.addType(
      "test.com/overlay/WithSet",
      def::Struct({
          def::Field(
              def::Identity(1, "setField"),
              def::Optional,
              TypeIds::set(TypeIds::uri("test.com/base/BaseEnum"))),
      }));

  auto overlay = std::move(builder).buildDerivedFrom(base);

  auto ref = overlay->getUserDefinedType("test.com/overlay/WithSet");
  ASSERT_TRUE(ref.has_value());

  // Get the base type directly
  auto baseEnumRef = base->getUserDefinedType("test.com/base/BaseEnum");
  ASSERT_TRUE(baseEnumRef.has_value());

  const auto& field = ref->asStruct().fields()[0];
  EXPECT_TRUE(field.type().isSet());
  EXPECT_EQ(
      field.type().asSet().elementType().asEnum().uri(),
      "test.com/base/BaseEnum");
  // Pointer equality - set element should resolve to SAME base enum
  EXPECT_EQ(
      &field.type().asSet().elementType().asEnum(), &baseEnumRef->asEnum());
}

TEST(TypeSystemBuilderDeriveTest, DeriveFromSyntaxGraph_OverlayReferencesBase) {
  auto base = makeSyntaxGraphTypeSystem();
  TypeSystemBuilder builder;

  // Overlay type that references a type from the SyntaxGraph base
  builder.addType(
      "test.com/overlay/RefToSyntaxGraph",
      def::Struct({
          def::Field(
              def::Identity(1, "sgRef"),
              def::Optional,
              TypeIds::uri("meta.com/thrift_test/TestRecursiveStruct")),
      }));

  auto overlay = std::move(builder).buildDerivedFrom(base);

  auto ref = overlay->getUserDefinedType("test.com/overlay/RefToSyntaxGraph");
  ASSERT_TRUE(ref.has_value());

  // Get the base type directly from the base TypeSystem
  auto baseTypeRef =
      base->getUserDefinedType("meta.com/thrift_test/TestRecursiveStruct");
  ASSERT_TRUE(baseTypeRef.has_value());

  const auto& field = ref->asStruct().fields()[0];

  // Verify the field type resolves to the SAME definition as in base
  // (pointer equality - the overlay should delegate to base, not copy)
  EXPECT_EQ(field.type().asStruct().uri(), baseTypeRef->uri());
  EXPECT_EQ(&field.type().asStruct(), &baseTypeRef->asStruct());
}

TEST(TypeSystemBuilderDeriveTest, DeepNestedContainers) {
  auto base = makeSimpleTypeSystem();
  TypeSystemBuilder builder;

  // list<map<BaseEnum, set<BaseStruct>>>
  builder.addType(
      "test.com/overlay/DeepNested",
      def::Struct({
          def::Field(
              def::Identity(1, "deep"),
              def::Optional,
              TypeIds::list(
                  TypeIds::map(
                      TypeIds::uri("test.com/base/BaseEnum"),
                      TypeIds::set(TypeIds::uri("test.com/base/BaseStruct"))))),
      }));

  auto overlay = std::move(builder).buildDerivedFrom(base);

  auto ref = overlay->getUserDefinedType("test.com/overlay/DeepNested");
  ASSERT_TRUE(ref.has_value());

  // Get the base types directly
  auto baseEnumRef = base->getUserDefinedType("test.com/base/BaseEnum");
  auto baseStructRef = base->getUserDefinedType("test.com/base/BaseStruct");
  ASSERT_TRUE(baseEnumRef.has_value());
  ASSERT_TRUE(baseStructRef.has_value());

  const auto& field = ref->asStruct().fields()[0];
  EXPECT_TRUE(field.type().isList());

  const auto& mapType = field.type().asList().elementType().asMap();
  EXPECT_EQ(mapType.keyType().asEnum().uri(), "test.com/base/BaseEnum");
  // Pointer equality for map key
  EXPECT_EQ(&mapType.keyType().asEnum(), &baseEnumRef->asEnum());

  const auto& setType = mapType.valueType().asSet();
  EXPECT_EQ(setType.elementType().asStruct().uri(), "test.com/base/BaseStruct");
  // Pointer equality for set element
  EXPECT_EQ(&setType.elementType().asStruct(), &baseStructRef->asStruct());
}

} // namespace apache::thrift::type_system
