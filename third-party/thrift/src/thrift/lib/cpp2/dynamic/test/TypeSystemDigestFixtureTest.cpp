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

// Cross-language fixture tests using compiled schemas loaded via SchemaRegistry
// and shared expected digest constants from digest_expected_values.thrift.
// These tests are separated from TypeSystemDigestTest.cpp because they depend
// on resourcesFbcode, which is incompatible with macOS (introduces xplat folly
// that conflicts with fbcode folly).

#include <thrift/lib/cpp2/dynamic/TypeSystemDigest.h>

#include <fstream>

#include <gtest/gtest.h>
#include <tools/cxx/Resources.h>
#include <folly/compression/Compression.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/dynamic/SerializableRecord.h>
#include <thrift/lib/cpp2/dynamic/SerializableTypeSystemBuilder.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>
#include <thrift/lib/cpp2/dynamic/test/gen-cpp2/digest_expected_values_constants.h>
#include <thrift/lib/cpp2/dynamic/test/gen-cpp2/digest_fixture_enum_types.h>
#include <thrift/lib/cpp2/dynamic/test/gen-cpp2/digest_fixture_multiple_types_types.h>
#include <thrift/lib/cpp2/dynamic/test/gen-cpp2/digest_fixture_single_empty_struct_types.h>
#include <thrift/lib/cpp2/dynamic/test/gen-cpp2/digest_fixture_struct_with_fields_types.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/runtime/SchemaRegistry.h>
#include <thrift/lib/cpp2/schema/SyntaxGraph.h>

using namespace apache::thrift::type_system;
using def = TypeSystemBuilder::DefinitionHelper;

namespace expected = apache::thrift::type_system::test::digest_expected_values::
    digest_expected_values_constants;

namespace {

std::string toHex(const TypeSystemDigest& d) {
  std::string result;
  result.reserve(d.size() * 2);
  for (auto byte : d) {
    result += fmt::format("{:02x}", byte);
  }
  return result;
}

SerializableTypeSystem buildStsFromRegistry(
    std::initializer_list<std::string_view> uris) {
  auto& registry = apache::thrift::SchemaRegistry::get();
  auto stsBuilder = SerializableTypeSystemBuilder::withoutSourceInfo(registry);
  for (auto uri : uris) {
    stsBuilder.addDefinition(uri);
  }
  return *std::move(stsBuilder).build();
}

apache::thrift::syntax_graph::SyntaxGraph loadFixtureSyntaxGraph(
    std::string_view resourceKey, std::string_view astFileName) {
  auto resourceDir = build::getResourcePath(
      fmt::format("thrift/lib/cpp2/dynamic/test/{}", resourceKey));
  // The -schema target produces a directory; the .ast file is nested inside.
  auto path = resourceDir / "thrift" / "lib" / "cpp2" / "dynamic" / "test" /
      "gen-ast" / std::string(astFileName);
  std::string data;
  {
    std::ifstream file(path.string(), std::ios::binary);
    if (!file) {
      throw std::runtime_error(
          fmt::format("Failed to read .ast file at: {}", path.string()));
    }
    data.assign(
        std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
  }
  // The .ast file may be zstd-compressed or raw compact protocol.
  std::string decompressed;
  try {
    decompressed =
        folly::compression::getCodec(folly::compression::CodecType::ZSTD)
            ->uncompress(std::string_view(data));
  } catch (const std::runtime_error&) {
    decompressed = std::move(data);
  }
  auto schema = apache::thrift::CompactSerializer::deserialize<
      apache::thrift::type::Schema>(decompressed);
  return apache::thrift::syntax_graph::SyntaxGraph::fromSchema(
      std::move(schema));
}

} // namespace

TEST(TypeSystemDigestFixtureTest, Empty) {
  TypeSystemHasher hasher;
  SerializableTypeSystem sts;
  EXPECT_EQ(toHex(hasher(sts)), expected::DIGEST_EMPTY());
}

TEST(TypeSystemDigestFixtureTest, SingleEmptyStruct) {
  auto sts = buildStsFromRegistry({"meta.com/test/Empty"});
  TypeSystemHasher hasher;
  EXPECT_EQ(toHex(hasher(sts)), expected::DIGEST_SINGLE_EMPTY_STRUCT());

  // Verify the per-fixture SyntaxGraph TypeSystem digest matches when pruned
  // to just the requested types (the full SyntaxGraph includes transitive
  // annotation types).
  auto graph = loadFixtureSyntaxGraph(
      "single_empty_struct_schema", "digest_fixture_single_empty_struct.ast");
  auto graphStsBuilder =
      SerializableTypeSystemBuilder::withoutSourceInfo(graph.asTypeSystem());
  graphStsBuilder.addDefinition("meta.com/test/Empty");
  auto graphSts = *std::move(graphStsBuilder).build();
  EXPECT_EQ(hasher(graphSts), hasher(sts));
}

TEST(TypeSystemDigestFixtureTest, StructWithFields) {
  auto sts = buildStsFromRegistry({"meta.com/test/Person"});
  TypeSystemHasher hasher;
  EXPECT_EQ(toHex(hasher(sts)), expected::DIGEST_STRUCT_WITH_FIELDS());

  auto graph = loadFixtureSyntaxGraph(
      "struct_with_fields_schema", "digest_fixture_struct_with_fields.ast");
  auto graphStsBuilder =
      SerializableTypeSystemBuilder::withoutSourceInfo(graph.asTypeSystem());
  graphStsBuilder.addDefinition("meta.com/test/Person");
  auto graphSts = *std::move(graphStsBuilder).build();
  EXPECT_EQ(hasher(graphSts), hasher(sts));
}

TEST(TypeSystemDigestFixtureTest, Enum) {
  auto sts = buildStsFromRegistry({"meta.com/test/Status"});
  TypeSystemHasher hasher;
  EXPECT_EQ(toHex(hasher(sts)), expected::DIGEST_ENUM());

  auto graph = loadFixtureSyntaxGraph("enum_schema", "digest_fixture_enum.ast");
  auto graphStsBuilder =
      SerializableTypeSystemBuilder::withoutSourceInfo(graph.asTypeSystem());
  graphStsBuilder.addDefinition("meta.com/test/Status");
  auto graphSts = *std::move(graphStsBuilder).build();
  EXPECT_EQ(hasher(graphSts), hasher(sts));
}

TEST(TypeSystemDigestFixtureTest, MultipleTypes) {
  // The fixture .thrift defines Person + Status. UserId (opaque alias) is not
  // expressible in IDL, so we load the compiled types and add it manually.
  TypeSystemBuilder builder;
  builder.addTypes(buildStsFromRegistry(
      {"meta.com/test/multi/Person", "meta.com/test/multi/Status"}));
  builder.addType(
      "meta.com/test/multi/UserId", def::OpaqueAlias(TypeId::I64()));
  auto ts = std::move(builder).build();

  auto stsBuilder = SerializableTypeSystemBuilder::withoutSourceInfo(*ts);
  stsBuilder.addDefinition("meta.com/test/multi/Person");
  stsBuilder.addDefinition("meta.com/test/multi/Status");
  stsBuilder.addDefinition("meta.com/test/multi/UserId");
  auto sts = *std::move(stsBuilder).build();

  TypeSystemHasher hasher;
  EXPECT_EQ(toHex(hasher(sts)), expected::DIGEST_MULTIPLE_TYPES());
  EXPECT_EQ(hasher(*ts), hasher(sts));
}

TEST(TypeSystemDigestFixtureTest, TypeIdBool) {
  TypeSystemHasher hasher;
  EXPECT_EQ(toHex(hasher(TypeId::Bool())), expected::DIGEST_TYPE_ID_BOOL());
}

TEST(TypeSystemDigestFixtureTest, TypeIdI32) {
  TypeSystemHasher hasher;
  EXPECT_EQ(toHex(hasher(TypeId::I32())), expected::DIGEST_TYPE_ID_I32());
}

TEST(TypeSystemDigestFixtureTest, TypeIdString) {
  TypeSystemHasher hasher;
  EXPECT_EQ(toHex(hasher(TypeId::String())), expected::DIGEST_TYPE_ID_STRING());
}

TEST(TypeSystemDigestFixtureTest, TypeIdUri) {
  TypeSystemHasher hasher;
  EXPECT_EQ(
      toHex(hasher(TypeIds::uri("meta.com/test/MyStruct"))),
      expected::DIGEST_TYPE_ID_URI());
}
