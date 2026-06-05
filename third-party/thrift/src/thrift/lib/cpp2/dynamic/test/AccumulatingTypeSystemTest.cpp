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

#include <thrift/lib/cpp2/dynamic/AccumulatingTypeSystem.h>

#include <thrift/lib/cpp2/dynamic/TypeId.h>
#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>

#include <utility>

namespace apache::thrift::type_system {
namespace {

using def = TypeSystemBuilder::DefinitionHelper;

// Builds a single-struct SerializableTypeSystem with i32 fields.
SerializableTypeSystem makeTs(
    const std::string& uri,
    const std::vector<std::pair<int16_t, std::string>>& fieldSpecs) {
  std::vector<SerializableFieldDefinition> fields;
  fields.reserve(fieldSpecs.size());
  for (const auto& [id, name] : fieldSpecs) {
    fields.push_back(
        def::Field(def::Identity(id, name), def::AlwaysPresent, TypeIds::I32));
  }

  SerializableTypeSystem ts;
  SerializableTypeDefinitionEntry entry;
  SerializableTypeDefinition typeDef;
  typeDef.structDef_ref() = def::Struct(std::move(fields));
  entry.definition() = std::move(typeDef);
  ts.types()[uri] = std::move(entry);
  return ts;
}

// As makeTs, but adds a struct-level annotation (changes the full digest but
// not the shape digest).
SerializableTypeSystem makeTsWithAnnotation(
    const std::string& uri,
    const std::vector<std::pair<int16_t, std::string>>& fieldSpecs,
    const std::string& annUri) {
  auto ts = makeTs(uri, fieldSpecs);
  auto& structDef = *ts.types()->at(uri).definition()->structDef_ref();
  SerializableRecordUnion rec;
  rec.boolDatum_ref() = true;
  structDef.annotations()[annUri] = std::move(rec);
  return ts;
}

// A single struct whose field references another user-defined type by URI.
SerializableTypeSystem makeStructReferencing(
    const std::string& uri,
    const std::string& fieldName,
    const std::string& targetUri) {
  SerializableTypeSystem ts;
  SerializableTypeDefinitionEntry entry;
  SerializableTypeDefinition typeDef;
  typeDef.structDef_ref() = def::Struct({def::Field(
      def::Identity(1, fieldName), def::Optional, TypeIds::uri(targetUri))});
  entry.definition() = std::move(typeDef);
  ts.types()[uri] = std::move(entry);
  return ts;
}

TEST(AccumulatingTypeSystemTest, EmptyTypeSystem) {
  AccumulatingTypeSystem ts;
  EXPECT_EQ(ts.size(), 0);
  EXPECT_TRUE(ts.empty());
  EXPECT_FALSE(ts.getUserDefinedType("test.thrift/Foo").has_value());
}

TEST(AccumulatingTypeSystemTest, SingleTypeSystem) {
  AccumulatingTypeSystem ts;
  ts.addTypes(makeTs("test.thrift/Foo", {{1, "a"}, {2, "b"}}));
  EXPECT_EQ(ts.size(), 1);

  auto fooRef = ts.getUserDefinedType("test.thrift/Foo");
  ASSERT_TRUE(fooRef.has_value());
  EXPECT_EQ(fooRef->asStruct().fields().size(), 2);
}

TEST(AccumulatingTypeSystemTest, DisjointTypeSystems) {
  AccumulatingTypeSystem ts;
  ts.addTypes(makeTs("test.thrift/Foo", {{1, "a"}}));
  ts.addTypes(makeTs("test.thrift/Bar", {{1, "x"}}));
  EXPECT_EQ(ts.size(), 2);
  EXPECT_TRUE(ts.getUserDefinedType("test.thrift/Foo").has_value());
  EXPECT_TRUE(ts.getUserDefinedType("test.thrift/Bar").has_value());
}

TEST(AccumulatingTypeSystemTest, IdenticalDefinitionsAreDeduplicated) {
  AccumulatingTypeSystem ts;
  ts.addTypes(makeTs("test.thrift/Foo", {{1, "a"}, {2, "b"}}));
  ts.addTypes(makeTs("test.thrift/Foo", {{1, "a"}, {2, "b"}}));
  EXPECT_EQ(ts.size(), 1);
  EXPECT_TRUE(ts.getUserDefinedType("test.thrift/Foo").has_value());
}

TEST(AccumulatingTypeSystemTest, UnresolvableReferenceLeavesInstanceUnchanged) {
  AccumulatingTypeSystem ts;
  // Foo references a URI that exists nowhere. Structural validation passes, so
  // the failure surfaces only during materialization in insertDefinitions,
  // after Foo's stub has been inserted.
  EXPECT_THROW(
      ts.addTypes(
          makeStructReferencing("test.thrift/Foo", "f", "test.thrift/Missing")),
      InvalidTypeError);

  // The half-built stub must be rolled back: the instance is unchanged.
  EXPECT_EQ(ts.size(), 0);
  EXPECT_TRUE(ts.empty());
  EXPECT_FALSE(ts.getUserDefinedType("test.thrift/Foo").has_value());

  // Re-adding once the dependency exists works (no leaked-stub corruption).
  ts.addTypes(makeTs("test.thrift/Missing", {{1, "x"}}));
  ts.addTypes(
      makeStructReferencing("test.thrift/Foo", "f", "test.thrift/Missing"));
  EXPECT_EQ(ts.size(), 2);
  EXPECT_TRUE(ts.getUserDefinedType("test.thrift/Foo").has_value());
}

TEST(
    AccumulatingTypeSystemTest,
    DuplicateSourceIdentifierLeavesInstanceUnchanged) {
  // Two distinct URIs sharing one source identifier: insertDefinitions stubs
  // both, indexes the first, then throws while indexing the second.
  SerializableTypeSystem batch;
  for (const std::string uri : {"test.thrift/A", "test.thrift/B"}) {
    SerializableTypeDefinitionEntry entry;
    SerializableTypeDefinition typeDef;
    typeDef.structDef_ref() = def::Struct(
        {def::Field(def::Identity(1, "a"), def::AlwaysPresent, TypeIds::I32)});
    entry.definition() = std::move(typeDef);
    SerializableThriftSourceInfo sourceInfo;
    sourceInfo.locator() = "loc.thrift";
    sourceInfo.name() = "Dup";
    entry.sourceInfo() = std::move(sourceInfo);
    batch.types()[uri] = std::move(entry);
  }

  AccumulatingTypeSystem ts;
  EXPECT_THROW(ts.addTypes(batch), InvalidTypeError);

  // Both stubs AND the first definition's source-index entry are rolled back.
  EXPECT_EQ(ts.size(), 0);
  EXPECT_FALSE(ts.getUserDefinedType("test.thrift/A").has_value());
  EXPECT_FALSE(ts.getUserDefinedType("test.thrift/B").has_value());
  EXPECT_TRUE(ts.getUserDefinedTypesAtLocation("loc.thrift").empty());

  // A subsequent valid add still works (no source-index or precondition
  // corruption).
  ts.addTypes(makeTs("test.thrift/A", {{1, "a"}}));
  EXPECT_EQ(ts.size(), 1);
}

// The central guarantee: a DefinitionRef obtained before further additions is
// still valid (same node address) afterwards. A rebuild-on-add implementation
// would fail this.
TEST(AccumulatingTypeSystemTest, DefinitionRefStableAcrossAddTypes) {
  AccumulatingTypeSystem ts;
  ts.addTypes(makeTs("test.thrift/Foo", {{1, "a"}}));

  auto fooRef = ts.getUserDefinedType("test.thrift/Foo");
  ASSERT_TRUE(fooRef.has_value());
  const StructNode* foo = &fooRef->asStruct();

  // Subsequent additions must not invalidate or relocate the existing node.
  ts.addTypes(makeTs("test.thrift/Bar", {{1, "x"}}));
  ts.addTypes(makeTs("test.thrift/Baz", {{1, "y"}}));

  auto fooRef2 = ts.getUserDefinedType("test.thrift/Foo");
  ASSERT_TRUE(fooRef2.has_value());
  EXPECT_EQ(&fooRef2->asStruct(), foo);

  // The originally-obtained pointer is still valid and intact.
  EXPECT_EQ(foo->uri(), "test.thrift/Foo");
  ASSERT_EQ(foo->fields().size(), 1);
  EXPECT_EQ(foo->fields()[0].identity().name(), "a");
}

// A type added in a later batch can reference a type added earlier, resolving
// to the very same node, and the earlier reference remains valid.
TEST(AccumulatingTypeSystemTest, LaterTypeReferencesEarlierType) {
  AccumulatingTypeSystem ts;
  ts.addTypes(makeTs("test.thrift/Inner", {{1, "a"}}));

  auto innerRef = ts.getUserDefinedType("test.thrift/Inner");
  ASSERT_TRUE(innerRef.has_value());
  const StructNode* inner = &innerRef->asStruct();

  ts.addTypes(
      makeStructReferencing("test.thrift/Outer", "inner", "test.thrift/Inner"));

  auto outerRef = ts.getUserDefinedType("test.thrift/Outer");
  ASSERT_TRUE(outerRef.has_value());
  const auto& field = outerRef->asStruct().fields()[0];
  ASSERT_TRUE(field.type().isStruct());
  // Field resolves to the SAME Inner node, and the earlier ref is still valid.
  EXPECT_EQ(&field.type().asStruct(), inner);
}

TEST(AccumulatingTypeSystemTest, DigestConflictThrowsAndLeavesStateUnchanged) {
  AccumulatingTypeSystem ts({DigestMode::Full, MergeResolution::Error});
  ts.addTypes(makeTs("test.thrift/Foo", {{1, "a"}}));

  EXPECT_THROW(
      ts.addTypes(makeTs("test.thrift/Foo", {{1, "a"}, {2, "b"}})),
      MergeConflictError);

  // Strong exception safety: the conflicting batch did not mutate the instance.
  EXPECT_EQ(ts.size(), 1);
  auto fooRef = ts.getUserDefinedType("test.thrift/Foo");
  ASSERT_TRUE(fooRef.has_value());
  EXPECT_EQ(fooRef->asStruct().fields().size(), 1);
}

TEST(AccumulatingTypeSystemTest, DigestConflictTakeFirst) {
  AccumulatingTypeSystem ts({DigestMode::Full, MergeResolution::TakeFirst});
  ts.addTypes(makeTs("test.thrift/Foo", {{1, "a"}}));
  ts.addTypes(makeTs("test.thrift/Foo", {{1, "a"}, {2, "b"}}));
  EXPECT_EQ(ts.size(), 1);

  auto fooRef = ts.getUserDefinedType("test.thrift/Foo");
  ASSERT_TRUE(fooRef.has_value());
  // TakeFirst keeps the 1-field version.
  EXPECT_EQ(fooRef->asStruct().fields().size(), 1);
}

TEST(AccumulatingTypeSystemTest, StructuralComparisonIgnoresAnnotations) {
  auto ts1 = makeTs("test.thrift/Foo", {{1, "a"}});
  auto ts2 = makeTsWithAnnotation("test.thrift/Foo", {{1, "a"}}, "my.ann");

  // Full-digest comparison: the annotation makes them differ -> conflict.
  {
    AccumulatingTypeSystem ts({DigestMode::Full, MergeResolution::Error});
    ts.addTypes(ts1);
    EXPECT_THROW(ts.addTypes(ts2), MergeConflictError);
  }

  // Structural comparison: annotations ignored -> deduplicated.
  {
    AccumulatingTypeSystem ts({DigestMode::Structural, MergeResolution::Error});
    ts.addTypes(ts1);
    EXPECT_NO_THROW(ts.addTypes(ts2));
    EXPECT_EQ(ts.size(), 1);
  }
}

TEST(AccumulatingTypeSystemTest, StructuralComparisonDetectsFieldDifference) {
  AccumulatingTypeSystem ts({DigestMode::Structural, MergeResolution::Error});
  ts.addTypes(makeTs("test.thrift/Foo", {{1, "a"}}));
  EXPECT_THROW(
      ts.addTypes(makeTs("test.thrift/Foo", {{1, "different_name"}})),
      MergeConflictError);
}

TEST(AccumulatingTypeSystemTest, WithBaseTypeSystem) {
  auto baseTs = makeTs("test.thrift/Foo", {{1, "a"}});
  TypeSystemBuilder builder;
  builder.addTypes(std::move(baseTs));
  std::shared_ptr<const TypeSystem> base = std::move(builder).build();

  AccumulatingTypeSystem ts({}, base);

  // Foo is in the base, so re-adding it deduplicates (not added to working
  // set).
  ts.addTypes(makeTs("test.thrift/Foo", {{1, "a"}}));
  EXPECT_EQ(ts.size(), 0);

  // Bar is new.
  ts.addTypes(makeTs("test.thrift/Bar", {{1, "x"}}));
  EXPECT_EQ(ts.size(), 1);

  // Both base and working-set types are visible.
  EXPECT_TRUE(ts.getUserDefinedType("test.thrift/Foo").has_value());
  EXPECT_TRUE(ts.getUserDefinedType("test.thrift/Bar").has_value());
}

TEST(AccumulatingTypeSystemTest, MergeConflictErrorCapturesUri) {
  AccumulatingTypeSystem ts;
  ts.addTypes(makeTs("test.thrift/Foo", {{1, "a"}}));
  try {
    // Re-adding Foo with different fields conflicts under the default policy.
    ts.addTypes(makeTs("test.thrift/Foo", {{1, "a"}, {2, "b"}}));
    FAIL() << "expected MergeConflictError";
  } catch (const MergeConflictError& e) {
    EXPECT_EQ(e.uri(), "test.thrift/Foo");
  }
}

TEST(AccumulatingTypeSystemTest, StructuralComparisonAppliesToBaseTypes) {
  // Base contains Foo with no annotations.
  auto baseTs = makeTs("test.thrift/Foo", {{1, "a"}});
  TypeSystemBuilder builder;
  builder.addTypes(std::move(baseTs));
  std::shared_ptr<const TypeSystem> base = std::move(builder).build();

  // Structural policy: re-adding Foo with an extra annotation is structurally
  // identical to the base, so it deduplicates against the base (the base is
  // compared with the same mode as the working set), not conflicts.
  {
    AccumulatingTypeSystem ts(
        {DigestMode::Structural, MergeResolution::Error}, base);
    EXPECT_NO_THROW(ts.addTypes(
        makeTsWithAnnotation("test.thrift/Foo", {{1, "a"}}, "my.ann")));
    EXPECT_EQ(ts.size(), 0);
  }

  // Full policy: the annotation difference is a conflict against the base.
  {
    AccumulatingTypeSystem ts({DigestMode::Full, MergeResolution::Error}, base);
    EXPECT_THROW(
        ts.addTypes(
            makeTsWithAnnotation("test.thrift/Foo", {{1, "a"}}, "my.ann")),
        MergeConflictError);
  }
}

TEST(AccumulatingTypeSystemTest, WorkingSetTypeReferencesBaseType) {
  auto baseTs = makeTs("test.thrift/Base", {{1, "a"}});
  TypeSystemBuilder builder;
  builder.addTypes(std::move(baseTs));
  std::shared_ptr<const TypeSystem> base = std::move(builder).build();
  auto baseRef = base->getUserDefinedType("test.thrift/Base");
  ASSERT_TRUE(baseRef.has_value());

  AccumulatingTypeSystem ts({}, base);
  ts.addTypes(
      makeStructReferencing("test.thrift/Ref", "b", "test.thrift/Base"));

  auto refType = ts.getUserDefinedType("test.thrift/Ref");
  ASSERT_TRUE(refType.has_value());
  const auto& field = refType->asStruct().fields()[0];
  ASSERT_TRUE(field.type().isStruct());
  // Resolves to the SAME base node (pointer equality).
  EXPECT_EQ(&field.type().asStruct(), &baseRef->asStruct());
}

TEST(AccumulatingTypeSystemTest, CanAddAfterUse) {
  AccumulatingTypeSystem ts;
  ts.addTypes(makeTs("test.thrift/Foo", {{1, "a"}}));
  // Force materialization via a lookup.
  EXPECT_TRUE(ts.getUserDefinedType("test.thrift/Foo").has_value());

  // Continued extension after use is supported.
  EXPECT_NO_THROW(ts.addTypes(makeTs("test.thrift/Bar", {{1, "x"}})));
  EXPECT_TRUE(ts.getUserDefinedType("test.thrift/Foo").has_value());
  EXPECT_TRUE(ts.getUserDefinedType("test.thrift/Bar").has_value());
}

TEST(AccumulatingTypeSystemTest, AddTypeSingleEntry) {
  auto sts = makeTs("test.thrift/Foo", {{1, "a"}});
  SerializableTypeDefinitionEntry entry = sts.types()->at("test.thrift/Foo");

  AccumulatingTypeSystem ts;
  ts.addType("test.thrift/Foo", entry);
  EXPECT_EQ(ts.size(), 1);
  EXPECT_TRUE(ts.getUserDefinedType("test.thrift/Foo").has_value());
}

TEST(AccumulatingTypeSystemTest, AddTypeFromRuntimeDefinition) {
  // Source TypeSystem where Foo references Bar.
  TypeSystemBuilder builder;
  builder.addTypes(makeTs("test.thrift/Bar", {{1, "x"}}));
  builder.addTypes(
      makeStructReferencing("test.thrift/Foo", "b", "test.thrift/Bar"));
  std::shared_ptr<const TypeSystem> source = std::move(builder).build();

  auto fooDef = source->getUserDefinedType("test.thrift/Foo");
  ASSERT_TRUE(fooDef.has_value());

  // Adding just Foo deep-copies it and its transitive closure (Bar).
  AccumulatingTypeSystem ts;
  ts.addType(*source, *fooDef);

  EXPECT_EQ(ts.size(), 2);
  EXPECT_TRUE(ts.getUserDefinedType("test.thrift/Foo").has_value());
  EXPECT_TRUE(ts.getUserDefinedType("test.thrift/Bar").has_value());
}

TEST(AccumulatingTypeSystemTest, PreservesSourceInfo) {
  // A type carrying source info (locator + name).
  SerializableTypeSystem sts = makeTs("test.thrift/Foo", {{1, "a"}});
  SerializableThriftSourceInfo sourceInfo;
  sourceInfo.locator() = "foo.thrift";
  sourceInfo.name() = "Foo";
  sts.types()->at("test.thrift/Foo").sourceInfo() = std::move(sourceInfo);

  AccumulatingTypeSystem ts;
  ts.addTypes(sts);

  // The source identifier round-trips both ways.
  auto bySource = ts.getUserDefinedTypeBySourceIdentifier(
      SourceIdentifierView{"foo.thrift", "Foo"});
  ASSERT_TRUE(bySource.has_value());
  EXPECT_EQ(bySource->uri(), "test.thrift/Foo");

  auto fooRef = ts.getUserDefinedType("test.thrift/Foo");
  ASSERT_TRUE(fooRef.has_value());
  auto sid = ts.getSourceIdentiferForUserDefinedType(*fooRef);
  ASSERT_TRUE(sid.has_value());
  EXPECT_EQ(sid->location, "foo.thrift");
  EXPECT_EQ(sid->name, "Foo");
}

// As PreservesSourceInfo, but exercises the runtime addTypes(const TypeSystem&)
// path, which captures source info via SerializableTypeSystemBuilder::
// withSourceInfo() instead of reading it off a SerializableTypeSystem.
TEST(AccumulatingTypeSystemTest, PreservesSourceInfoFromRuntime) {
  // A runtime TypeSystem whose type carries source info (locator + name).
  TypeSystemBuilder builder;
  builder.addType(
      "test.thrift/Foo",
      def::Struct({def::Field(
          def::Identity(1, "a"), def::AlwaysPresent, TypeIds::I32)}),
      def::SourceInfo("foo.thrift", "Foo"));
  std::shared_ptr<const TypeSystem> source = std::move(builder).build();

  // Sanity: the source itself exposes the source identifier we attached.
  auto sourceFoo = source->getUserDefinedType("test.thrift/Foo");
  ASSERT_TRUE(sourceFoo.has_value());
  ASSERT_TRUE(
      source->getSourceIdentiferForUserDefinedType(*sourceFoo).has_value());

  AccumulatingTypeSystem ts;
  ts.addTypes(*source);

  // The source identifier round-trips both ways after the runtime deep copy.
  auto bySource = ts.getUserDefinedTypeBySourceIdentifier(
      SourceIdentifierView{"foo.thrift", "Foo"});
  ASSERT_TRUE(bySource.has_value());
  EXPECT_EQ(bySource->uri(), "test.thrift/Foo");

  auto fooRef = ts.getUserDefinedType("test.thrift/Foo");
  ASSERT_TRUE(fooRef.has_value());
  auto sid = ts.getSourceIdentiferForUserDefinedType(*fooRef);
  ASSERT_TRUE(sid.has_value());
  EXPECT_EQ(sid->location, "foo.thrift");
  EXPECT_EQ(sid->name, "Foo");
}

TEST(AccumulatingTypeSystemTest, AddTypesFromRuntimeTypeSystem) {
  // A runtime TypeSystem (not a SerializableTypeSystem) as the input source.
  TypeSystemBuilder builder;
  builder.addTypes(makeTs("test.thrift/Foo", {{1, "a"}, {2, "b"}}));
  builder.addTypes(makeTs("test.thrift/Bar", {{1, "x"}}));
  std::shared_ptr<const TypeSystem> source = std::move(builder).build();

  AccumulatingTypeSystem ts;
  ts.addTypes(*source);

  EXPECT_EQ(ts.size(), 2);
  auto fooRef = ts.getUserDefinedType("test.thrift/Foo");
  ASSERT_TRUE(fooRef.has_value());
  EXPECT_EQ(fooRef->asStruct().fields().size(), 2);
  EXPECT_TRUE(ts.getUserDefinedType("test.thrift/Bar").has_value());

  // Re-adding the same runtime types deduplicates.
  ts.addTypes(*source);
  EXPECT_EQ(ts.size(), 2);
}

// A runtime struct that references another runtime type resolves, after the
// deep copy, to this instance's own copy of that referenced type.
TEST(AccumulatingTypeSystemTest, AddTypesFromRuntimePreservesReferences) {
  TypeSystemBuilder builder;
  builder.addTypes(makeTs("test.thrift/Inner", {{1, "a"}}));
  builder.addTypes(
      makeStructReferencing("test.thrift/Outer", "inner", "test.thrift/Inner"));
  std::shared_ptr<const TypeSystem> source = std::move(builder).build();

  AccumulatingTypeSystem ts;
  ts.addTypes(*source);
  EXPECT_EQ(ts.size(), 2);

  auto innerRef = ts.getUserDefinedType("test.thrift/Inner");
  auto outerRef = ts.getUserDefinedType("test.thrift/Outer");
  ASSERT_TRUE(innerRef.has_value());
  ASSERT_TRUE(outerRef.has_value());
  const auto& field = outerRef->asStruct().fields()[0];
  ASSERT_TRUE(field.type().isStruct());
  // Resolves to this instance's Inner node, not the source's.
  EXPECT_EQ(&field.type().asStruct(), &innerRef->asStruct());
}

// "Deep copy": the materialized nodes are owned by this instance and outlive
// the source TypeSystem they were copied from.
TEST(AccumulatingTypeSystemTest, AddTypesFromRuntimeIsDeepCopy) {
  AccumulatingTypeSystem ts;
  const void* sourceFooAddr = nullptr;
  {
    TypeSystemBuilder builder;
    builder.addTypes(makeTs("test.thrift/Foo", {{1, "a"}}));
    std::shared_ptr<const TypeSystem> source = std::move(builder).build();
    auto sourceFoo = source->getUserDefinedType("test.thrift/Foo");
    ASSERT_TRUE(sourceFoo.has_value());
    sourceFooAddr = &sourceFoo->asStruct();
    ts.addTypes(*source);
    // `source` is destroyed at the end of this scope.
  }

  // The copied node survives destruction of the source and lives at a distinct
  // address (it is owned by this instance, not aliased into the source).
  auto fooRef = ts.getUserDefinedType("test.thrift/Foo");
  ASSERT_TRUE(fooRef.has_value());
  EXPECT_NE(static_cast<const void*>(&fooRef->asStruct()), sourceFooAddr);
  EXPECT_EQ(fooRef->asStruct().uri(), "test.thrift/Foo");
  ASSERT_EQ(fooRef->asStruct().fields().size(), 1);
  EXPECT_EQ(fooRef->asStruct().fields()[0].identity().name(), "a");
}

TEST(AccumulatingTypeSystemTest, GetKnownUrisIncludesBaseAndWorkingSet) {
  auto baseTs = makeTs("test.thrift/Base", {{1, "a"}});
  TypeSystemBuilder builder;
  builder.addTypes(std::move(baseTs));
  std::shared_ptr<const TypeSystem> base = std::move(builder).build();

  AccumulatingTypeSystem ts({}, base);
  ts.addTypes(makeTs("test.thrift/Local", {{1, "a"}}));

  auto knownUris = ts.getKnownUris();
  ASSERT_TRUE(knownUris.has_value());
  EXPECT_TRUE(knownUris->contains("test.thrift/Base"));
  EXPECT_TRUE(knownUris->contains("test.thrift/Local"));
  EXPECT_EQ(knownUris->size(), 2);
}

} // namespace
} // namespace apache::thrift::type_system
