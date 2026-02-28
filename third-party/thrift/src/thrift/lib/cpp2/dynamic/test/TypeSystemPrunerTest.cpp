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

#include <thrift/lib/cpp2/dynamic/SerializableTypeSystemBuilder.h>
#include <thrift/lib/cpp2/dynamic/TypeId.h>
#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>

namespace apache::thrift::type_system {

using def = TypeSystemBuilder::DefinitionHelper;

namespace {

void expectHasType(const TypeSystem& ts, UriView uri) {
  EXPECT_TRUE(ts.getUserDefinedType(uri).has_value()) << uri;
}

void expectNoType(const TypeSystem& ts, UriView uri) {
  EXPECT_FALSE(ts.getUserDefinedType(uri).has_value()) << uri;
}

void expectKnownUris(
    const TypeSystem& typeSystem, std::initializer_list<Uri> uris) {
  folly::F14FastSet<Uri> expectedUris{uris.begin(), uris.end()};
  const auto knownUris = *typeSystem.getKnownUris();
  EXPECT_EQ(expectedUris, knownUris);
}

AnnotationsMap makeAnnot(UriView uri) {
  AnnotationsMap annots;
  annots.emplace(uri, SerializableRecord(SerializableRecord::FieldSet{}));
  return annots;
}

// Builds a TypeSystem with a variety of interconnected types:
//
//   Struct "test/A" { field b: test/B }
//   Struct "test/B" { field c: test/C }
//   Struct "test/C" {}
//   Union  "test/U" { field a: test/A }
//   Enum   "test/E" { VAL=0 }
//   OpaqueAlias "test/Alias" -> i32
//   Struct "test/Unrelated" {}
//   Struct "test/AnnotStruct" {}
//   Struct "test/Annotated" {}  (annotated with @test/AnnotStruct)
//
// All types have source info.
std::unique_ptr<TypeSystem> makeRichTypeSystem() {
  TypeSystemBuilder builder;
  builder.addType(
      "test/A",
      def::Struct({
          def::Field(
              def::Identity(1, "b"),
              def::AlwaysPresent,
              TypeIds::uri("test/B")),
      }),
      def::SourceInfo("test.thrift", "A"));
  builder.addType(
      "test/B",
      def::Struct({
          def::Field(
              def::Identity(1, "c"),
              def::AlwaysPresent,
              TypeIds::uri("test/C")),
      }),
      def::SourceInfo("test.thrift", "B"));
  builder.addType(
      "test/C", def::Struct({}), def::SourceInfo("test.thrift", "C"));
  builder.addType(
      "test/U",
      def::Union({
          def::Field(
              def::Identity(1, "a"), def::Optional, TypeIds::uri("test/A")),
      }),
      def::SourceInfo("test.thrift", "U"));
  builder.addType(
      "test/E",
      def::Enum({def::DefinitionHelper::EnumValue{"VAL", 0}}),
      def::SourceInfo("test.thrift", "E"));
  builder.addType(
      "test/Alias",
      def::OpaqueAlias(TypeIds::I32),
      def::SourceInfo("test.thrift", "Alias"));
  builder.addType(
      "test/Unrelated",
      def::Struct({}),
      def::SourceInfo("test.thrift", "Unrelated"));
  builder.addType(
      "test/AnnotStruct",
      def::Struct({}),
      def::SourceInfo("test.thrift", "AnnotStruct"));
  builder.addType(
      "test/Annotated",
      def::Struct({}, false, makeAnnot("test/AnnotStruct")),
      def::SourceInfo("test.thrift", "Annotated"));
  return std::move(builder).build();
}

} // namespace

TEST(TypeSystemPrunerTest, ExtractSingleType) {
  auto source = makeRichTypeSystem();
  std::vector<UriView> roots{"test/C"};
  auto pruned = TypeSystemBuilder::buildPrunedFrom(*source, roots);

  expectHasType(*pruned, "test/C");
  expectNoType(*pruned, "test/A");
  expectNoType(*pruned, "test/B");
  expectNoType(*pruned, "test/Unrelated");
  expectKnownUris(*pruned, {"test/C"});
}

TEST(TypeSystemPrunerTest, ExtractMultipleTypes) {
  auto source = makeRichTypeSystem();
  std::vector<UriView> roots{"test/C", "test/E"};
  auto pruned = TypeSystemBuilder::buildPrunedFrom(*source, roots);

  expectHasType(*pruned, "test/C");
  expectHasType(*pruned, "test/E");
  expectNoType(*pruned, "test/A");
  expectNoType(*pruned, "test/Unrelated");
}

TEST(TypeSystemPrunerTest, ExtractTransitiveTypes) {
  auto source = makeRichTypeSystem();
  std::vector<UriView> roots{"test/A"};
  auto pruned = TypeSystemBuilder::buildPrunedFrom(*source, roots);

  expectHasType(*pruned, "test/A");
  expectHasType(*pruned, "test/B");
  expectHasType(*pruned, "test/C");
  expectNoType(*pruned, "test/Unrelated");
  expectKnownUris(*pruned, {"test/A", "test/B", "test/C"});

  auto aRef = pruned->getUserDefinedTypeOrThrow("test/A");
  EXPECT_EQ(aRef.asStruct().fields()[0].type().asStruct().uri(), "test/B");
}

TEST(TypeSystemPrunerTest, ExtractContainerTypes) {
  TypeSystemBuilder builder;
  builder.addType(
      "test/WithContainers",
      def::Struct({
          def::Field(
              def::Identity(1, "l"),
              def::AlwaysPresent,
              TypeIds::list(TypeIds::uri("test/X"))),
          def::Field(
              def::Identity(2, "m"),
              def::AlwaysPresent,
              TypeIds::map(TypeIds::I32, TypeIds::uri("test/Y"))),
          def::Field(
              def::Identity(3, "s"),
              def::AlwaysPresent,
              TypeIds::set(TypeIds::uri("test/Z"))),
      }));
  builder.addType("test/X", def::Struct({}));
  builder.addType("test/Y", def::Struct({}));
  builder.addType("test/Z", def::Struct({}));
  builder.addType("test/Other", def::Struct({}));
  auto source = std::move(builder).build();

  std::vector<UriView> roots{"test/WithContainers"};
  auto pruned = TypeSystemBuilder::buildPrunedFrom(*source, roots);

  expectHasType(*pruned, "test/WithContainers");
  expectHasType(*pruned, "test/X");
  expectHasType(*pruned, "test/Y");
  expectHasType(*pruned, "test/Z");
  expectNoType(*pruned, "test/Other");
}

TEST(TypeSystemPrunerTest, ExtractOpaqueAliases) {
  auto source = makeRichTypeSystem();
  std::vector<UriView> roots{"test/Alias"};
  auto pruned = TypeSystemBuilder::buildPrunedFrom(*source, roots);

  auto ref = pruned->getUserDefinedTypeOrThrow("test/Alias");
  EXPECT_TRUE(ref.isOpaqueAlias());
  EXPECT_TRUE(ref.asOpaqueAlias().targetType().isI32());
  expectNoType(*pruned, "test/Unrelated");
}

TEST(TypeSystemPrunerTest, ExtractAnnotations) {
  auto source = makeRichTypeSystem();
  std::vector<UriView> roots{"test/Annotated"};
  auto pruned = TypeSystemBuilder::buildPrunedFrom(*source, roots);

  expectHasType(*pruned, "test/Annotated");
  expectHasType(*pruned, "test/AnnotStruct");
  expectNoType(*pruned, "test/Unrelated");
}

TEST(TypeSystemPrunerTest, ExtractFieldAnnotations) {
  TypeSystemBuilder builder;
  builder.addType("test/FieldAnnot", def::Struct({}));
  builder.addType(
      "test/A",
      def::Struct({
          def::Field(
              def::Identity(1, "f"),
              def::AlwaysPresent,
              TypeIds::I32,
              std::nullopt,
              makeAnnot("test/FieldAnnot")),
      }));
  auto source = std::move(builder).build();

  std::vector<UriView> roots{"test/A"};
  auto pruned = TypeSystemBuilder::buildPrunedFrom(*source, roots);

  expectHasType(*pruned, "test/A");
  expectHasType(*pruned, "test/FieldAnnot");
}

TEST(TypeSystemPrunerTest, ExtractEnumValueAnnotations) {
  TypeSystemBuilder builder;
  builder.addType("test/ValueAnnot", def::Struct({}));
  builder.addType(
      "test/E",
      def::Enum({
          def::DefinitionHelper::EnumValue{
              "VAL", 0, makeAnnot("test/ValueAnnot")},
      }));
  auto source = std::move(builder).build();

  std::vector<UriView> roots{"test/E"};
  auto pruned = TypeSystemBuilder::buildPrunedFrom(*source, roots);

  expectHasType(*pruned, "test/E");
  expectHasType(*pruned, "test/ValueAnnot");
}

TEST(TypeSystemPrunerTest, RecursiveTypes) {
  TypeSystemBuilder builder;
  builder.addType(
      "test/R",
      def::Struct({
          def::Field(
              def::Identity(1, "self"), def::Optional, TypeIds::uri("test/R")),
      }));
  auto source = std::move(builder).build();

  std::vector<UriView> roots{"test/R"};
  auto pruned = TypeSystemBuilder::buildPrunedFrom(*source, roots);

  auto ref = pruned->getUserDefinedTypeOrThrow("test/R");
  EXPECT_EQ(
      std::addressof(ref.asStruct().fields()[0].type().asStruct()),
      std::addressof(ref.asStruct()));
}

TEST(TypeSystemPrunerTest, MutuallyRecursiveTypes) {
  auto source = makeRichTypeSystem();
  // test/U references test/A, test/A references test/B -> test/C
  std::vector<UriView> roots{"test/U"};
  auto pruned = TypeSystemBuilder::buildPrunedFrom(*source, roots);

  expectHasType(*pruned, "test/U");
  expectHasType(*pruned, "test/A");
  expectHasType(*pruned, "test/B");
  expectHasType(*pruned, "test/C");
  expectNoType(*pruned, "test/Unrelated");
}

TEST(TypeSystemPrunerTest, SourceInfoPreserved) {
  auto source = makeRichTypeSystem();
  std::vector<UriView> roots{"test/A"};
  auto pruned =
      TypeSystemBuilder::buildPrunedFrom(*source, roots, PruneOptions{true});

  auto ref = pruned->getUserDefinedTypeOrThrow("test/A");
  auto info = pruned->getSourceIdentiferForUserDefinedType(ref);
  ASSERT_TRUE(info.has_value());
  EXPECT_EQ(info->location, "test.thrift");
  EXPECT_EQ(info->name, "A");

  auto bySource = pruned->getUserDefinedTypeBySourceIdentifier(
      SourceIdentifierView{"test.thrift", "A"});
  ASSERT_TRUE(bySource.has_value());
  EXPECT_EQ(bySource->uri(), "test/A");
}

TEST(TypeSystemPrunerTest, SourceInfoOmitted) {
  auto source = makeRichTypeSystem();
  std::vector<UriView> roots{"test/A"};
  auto pruned =
      TypeSystemBuilder::buildPrunedFrom(*source, roots, PruneOptions{false});

  auto ref = pruned->getUserDefinedTypeOrThrow("test/A");
  EXPECT_FALSE(pruned->getSourceIdentiferForUserDefinedType(ref).has_value());
}

TEST(TypeSystemPrunerTest, PrunedIsIndependentOfSource) {
  std::unique_ptr<TypeSystem> pruned;
  {
    auto source = makeRichTypeSystem();
    std::vector<UriView> roots{"test/A"};
    pruned = TypeSystemBuilder::buildPrunedFrom(*source, roots);
  }
  // Source is destroyed — pruned must still work
  auto ref = pruned->getUserDefinedTypeOrThrow("test/A");
  EXPECT_EQ(ref.asStruct().fields().size(), 1);
}

TEST(TypeSystemPrunerTest, ExtractByDefinitionRef) {
  auto source = makeRichTypeSystem();
  auto aRef = source->getUserDefinedTypeOrThrow("test/A");
  std::vector<DefinitionRef> roots{aRef};
  auto pruned = TypeSystemBuilder::buildPrunedFrom(*source, roots);

  expectHasType(*pruned, "test/A");
  expectHasType(*pruned, "test/B");
  expectNoType(*pruned, "test/Unrelated");
}

TEST(TypeSystemPrunerTest, PruneAsEmpty) {
  auto source = makeRichTypeSystem();
  std::vector<UriView> roots{};
  auto pruned = TypeSystemBuilder::buildPrunedFrom(*source, roots);

  auto knownUris = pruned->getKnownUris();
  ASSERT_TRUE(knownUris.has_value());
  EXPECT_TRUE(knownUris->empty());
}

TEST(TypeSystemPrunerTest, ThrowOnNonExistantType) {
  auto source = makeRichTypeSystem();
  std::vector<UriView> roots{"test/DoesNotExist"};
  EXPECT_THROW(
      TypeSystemBuilder::buildPrunedFrom(*source, roots), InvalidTypeError);
}

TEST(TypeSystemPrunerTest, ExtractIntoSerializableTypeSystem) {
  auto source = makeRichTypeSystem();
  std::vector<UriView> roots{"test/A"};
  auto sts = SerializableTypeSystemBuilder::buildPrunedFrom(*source, roots);

  EXPECT_EQ(sts->types()->size(), 3); // A, B, C
  EXPECT_TRUE(sts->types()->contains("test/A"));
  EXPECT_TRUE(sts->types()->contains("test/B"));
  EXPECT_TRUE(sts->types()->contains("test/C"));
  EXPECT_FALSE(sts->types()->contains("test/Unrelated"));
}

TEST(TypeSystemPrunerTest, ExtractWithSourceInfo) {
  auto source = makeRichTypeSystem();
  std::vector<UriView> roots{"test/A"};

  auto withInfo = SerializableTypeSystemBuilder::buildPrunedFrom(
      *source, roots, PruneOptions{.includeSourceInfo = true});
  ASSERT_TRUE(withInfo->types()->at("test/A").sourceInfo().has_value());
  EXPECT_EQ(
      *withInfo->types()->at("test/A").sourceInfo()->locator(), "test.thrift");

  auto withoutInfo = SerializableTypeSystemBuilder::buildPrunedFrom(
      *source, roots, PruneOptions{.includeSourceInfo = false});
  EXPECT_FALSE(withoutInfo->types()->at("test/A").sourceInfo().has_value());
}

TEST(TypeSystemPrunerTest, ExtractIntoSerializableByDefinitionRef) {
  auto source = makeRichTypeSystem();
  auto aRef = source->getUserDefinedTypeOrThrow("test/A");
  std::vector<DefinitionRef> roots{aRef};
  auto sts = SerializableTypeSystemBuilder::buildPrunedFrom(*source, roots);

  EXPECT_EQ(sts->types()->size(), 3); // A, B, C
  EXPECT_FALSE(sts->types()->contains("test/Unrelated"));
}

} // namespace apache::thrift::type_system
