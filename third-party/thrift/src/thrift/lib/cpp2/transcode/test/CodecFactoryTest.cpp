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

#include <thrift/lib/cpp2/transcode/CodecFactory.h>
#include <thrift/lib/cpp2/transcode/TranscodePlan.h>

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>

#include <stdexcept>
#include <string>
#include <string_view>

namespace apache::thrift::transcode {
namespace {

using def = type_system::TypeSystemBuilder::DefinitionHelper;
using type_system::TypeId;
using type_system::TypeIds;

template <typename Fn>
void expectInvalidArgumentMessage(Fn fn, std::string_view needle) {
  try {
    fn();
    FAIL() << "expected std::invalid_argument";
  } catch (const std::invalid_argument& ex) {
    EXPECT_NE(std::string(ex.what()).find(needle), std::string::npos)
        << ex.what();
  }
}

// A two-field struct: 1: i32 id (always present), 2: string name (optional).
struct CodecFactoryTest : ::testing::Test {
  std::unique_ptr<type_system::TypeSystem> typeSystem;

  CodecFactoryTest() {
    type_system::TypeSystemBuilder builder;
    builder.addType(
        "test.Person",
        def::Struct({
            def::Field(
                def::Identity(1, "id"), def::AlwaysPresent, TypeIds::I32),
            def::Field(
                def::Identity(2, "name"), def::Optional, TypeIds::String),
        }));
    typeSystem = std::move(builder).build();
  }

  const type_system::StructNode& personNode() {
    return typeSystem->getUserDefinedTypeOrThrow("test.Person").asStruct();
  }
};

TEST_F(CodecFactoryTest, ProducesStructRootForEachProtocol) {
  const auto& node = personNode();

  // Every factory should hand back a struct-rooted codec for a struct schema.
  EXPECT_TRUE(
      std::holds_alternative<StructOp>(makeThriftCompactCodec(node).root));
  EXPECT_TRUE(
      std::holds_alternative<StructOp>(makeThriftBinaryCodec(node).root));
  EXPECT_TRUE(
      std::holds_alternative<StructOp>(makeProtobufBinaryCodec(node).root));
  EXPECT_TRUE(std::holds_alternative<StructOp>(makeJsonCodec(node).root));
}

TEST_F(CodecFactoryTest, CompactCodecCarriesBothSchemaFields) {
  auto codec = makeThriftCompactCodec(personNode());
  const auto& root = std::get<StructOp>(codec.root);

  ASSERT_EQ(root.fields.size(), 2u);
  EXPECT_EQ(root.fields[0].fieldId, 1);
  EXPECT_EQ(root.fields[0].fieldName, "id");
  EXPECT_TRUE(root.fields[0].required);
  EXPECT_EQ(root.fields[1].fieldId, 2);
  EXPECT_EQ(root.fields[1].fieldName, "name");
  EXPECT_FALSE(root.fields[1].required);
}

TEST(CodecFactoryStandaloneTest, ProtobufSignedIntegersUseZigzagVarints) {
  type_system::TypeSystemBuilder builder;
  builder.addType(
      "test.SignedNumbers",
      def::Struct({
          def::Field(def::Identity(1, "b"), def::AlwaysPresent, TypeIds::Byte),
          def::Field(def::Identity(2, "s"), def::AlwaysPresent, TypeIds::I16),
          def::Field(def::Identity(3, "i"), def::AlwaysPresent, TypeIds::I32),
          def::Field(def::Identity(4, "l"), def::AlwaysPresent, TypeIds::I64),
      }));
  auto ts = std::move(builder).build();
  ASSERT_NE(ts, nullptr);
  const auto& node =
      ts->getUserDefinedTypeOrThrow("test.SignedNumbers").asStruct();

  auto codec = makeProtobufBinaryCodec(node);
  const auto& root = std::get<StructOp>(codec.root);
  ASSERT_EQ(root.fields.size(), 4u);

  for (const auto& field : root.fields) {
    ASSERT_NE(field.command, nullptr);
    const auto& command = *field.command;
    ASSERT_TRUE(std::holds_alternative<ScalarOp>(command));
    const auto& scalar = std::get<ScalarOp>(command);
    EXPECT_EQ(scalar.readFn, ReadFn::ZigzagVarint);
    EXPECT_EQ(scalar.writeFn, WriteFn::ZigzagVarint);
  }
}

TEST(CodecFactoryStandaloneTest, JsonBinaryUsesBase64String) {
  type_system::TypeSystemBuilder builder;
  builder.addType(
      "test.WithBinary",
      def::Struct({
          def::Field(
              def::Identity(1, "data"), def::AlwaysPresent, TypeIds::Binary),
      }));
  auto ts = std::move(builder).build();
  ASSERT_NE(ts, nullptr);
  const auto& node =
      ts->getUserDefinedTypeOrThrow("test.WithBinary").asStruct();

  auto codec = makeJsonCodec(node);
  const auto& root = std::get<StructOp>(codec.root);
  ASSERT_EQ(root.fields.size(), 1u);
  ASSERT_NE(root.fields[0].command, nullptr);
  const auto& command = *root.fields[0].command;
  ASSERT_TRUE(std::holds_alternative<ScalarOp>(command));

  const auto& scalar = std::get<ScalarOp>(command);
  EXPECT_EQ(scalar.readFn, ReadFn::ParseBase64String);
  EXPECT_EQ(scalar.writeFn, WriteFn::WriteBase64String);
}

TEST(CodecFactoryStandaloneTest, OpaqueAliasUsesTargetScalarCodec) {
  type_system::TypeSystemBuilder builder;
  builder.addType("test.UserId", def::OpaqueAlias(TypeIds::I64));
  builder.addType(
      "test.WithAlias",
      def::Struct({
          def::Field(
              def::Identity(1, "id"),
              def::AlwaysPresent,
              TypeIds::uri("test.UserId")),
      }));
  auto ts = std::move(builder).build();
  ASSERT_NE(ts, nullptr);
  const auto& node = ts->getUserDefinedTypeOrThrow("test.WithAlias").asStruct();

  auto codec = makeThriftBinaryCodec(node);
  const auto& root = std::get<StructOp>(codec.root);
  ASSERT_EQ(root.fields.size(), 1u);
  ASSERT_NE(root.fields[0].command, nullptr);
  const auto& command = *root.fields[0].command;
  ASSERT_TRUE(std::holds_alternative<ScalarOp>(command));

  const auto& scalar = std::get<ScalarOp>(command);
  EXPECT_EQ(scalar.valueKind, ValueKind::I64);
  EXPECT_EQ(scalar.readFn, ReadFn::Fixed64BE);
  EXPECT_EQ(scalar.writeFn, WriteFn::Fixed64BE);
}

TEST(CodecFactoryStandaloneTest, UnsupportedAnyTypeThrowsForEachProtocol) {
  type_system::TypeSystemBuilder builder;
  builder.addType(
      "test.WithAny",
      def::Struct({
          def::Field(def::Identity(1, "payload"), def::Optional, TypeId::Any()),
      }));
  auto ts = std::move(builder).build();
  ASSERT_NE(ts, nullptr);
  const auto& node = ts->getUserDefinedTypeOrThrow("test.WithAny").asStruct();

  EXPECT_THROW({ (void)makeThriftCompactCodec(node); }, std::invalid_argument);
  EXPECT_THROW({ (void)makeThriftBinaryCodec(node); }, std::invalid_argument);
  EXPECT_THROW({ (void)makeProtobufBinaryCodec(node); }, std::invalid_argument);
  EXPECT_THROW({ (void)makeJsonCodec(node); }, std::invalid_argument);
}

TEST(
    CodecFactoryStandaloneTest,
    NonThriftCodecsRejectCustomDefaultsDuringConstruction) {
  type_system::TypeSystemBuilder builder;
  builder.addType(
      "test.WithDefault",
      def::Struct({
          def::Field(
              def::Identity(1, "id"),
              def::AlwaysPresent,
              TypeIds::I32,
              type_system::SerializableRecord::Int32(42)),
      }));
  auto ts = std::move(builder).build();
  ASSERT_NE(ts, nullptr);
  const auto& node =
      ts->getUserDefinedTypeOrThrow("test.WithDefault").asStruct();

  EXPECT_NO_THROW({ (void)makeThriftCompactCodec(node); });
  EXPECT_NO_THROW({ (void)makeThriftBinaryCodec(node); });
  expectInvalidArgumentMessage(
      [&] { (void)makeProtobufBinaryCodec(node); }, "custom defaults");
  expectInvalidArgumentMessage(
      [&] { (void)makeJsonCodec(node); }, "custom defaults");
}

TEST(
    CodecFactoryStandaloneTest,
    NonThriftCodecsRejectTerseFieldsDuringConstruction) {
  type_system::TypeSystemBuilder builder;
  builder.addType(
      "test.WithTerse",
      def::Struct({
          def::Field(
              def::Identity(1, "id"),
              type_system::PresenceQualifier::TERSE,
              TypeIds::I32),
      }));
  auto ts = std::move(builder).build();
  ASSERT_NE(ts, nullptr);
  const auto& node = ts->getUserDefinedTypeOrThrow("test.WithTerse").asStruct();

  EXPECT_NO_THROW({ (void)makeThriftCompactCodec(node); });
  EXPECT_NO_THROW({ (void)makeThriftBinaryCodec(node); });
  expectInvalidArgumentMessage(
      [&] { (void)makeProtobufBinaryCodec(node); }, "terse");
  expectInvalidArgumentMessage([&] { (void)makeJsonCodec(node); }, "terse");
}

TEST(CodecFactoryStandaloneTest, ProtobufMapFieldIsRepeatedEntry) {
  type_system::TypeSystemBuilder builder;
  builder.addType(
      "test.Scoreboard",
      def::Struct({
          def::Field(
              def::Identity(1, "scores"),
              def::Optional,
              TypeIds::map(TypeIds::String, TypeIds::I32)),
      }));
  auto ts = std::move(builder).build();
  ASSERT_NE(ts, nullptr);
  const auto& node =
      ts->getUserDefinedTypeOrThrow("test.Scoreboard").asStruct();

  auto codec = makeProtobufBinaryCodec(node);
  const auto& root = std::get<StructOp>(codec.root);
  ASSERT_EQ(root.fields.size(), 1u);
  EXPECT_TRUE(root.fields[0].isRepeated);
  ASSERT_NE(root.fields[0].command, nullptr);
  const auto& command = *root.fields[0].command;
  ASSERT_TRUE(std::holds_alternative<MapOp>(command));

  const auto& map = std::get<MapOp>(command);
  EXPECT_EQ(map.readFraming, ContainerFraming::None);
  EXPECT_EQ(map.writeFraming, ContainerFraming::None);
  EXPECT_TRUE(map.readEntryIsSubmessage);
  EXPECT_TRUE(map.writeEntryIsSubmessage);
}

TEST(CodecFactoryStandaloneTest, ProtobufUnionCodecCarriesOptionalFields) {
  type_system::TypeSystemBuilder builder;
  builder.addType(
      "test.Choice",
      def::Union({
          def::Field(def::Identity(1, "id"), def::Optional, TypeIds::I32),
          def::Field(def::Identity(2, "name"), def::Optional, TypeIds::String),
      }));
  auto ts = std::move(builder).build();
  ASSERT_NE(ts, nullptr);
  const auto& node = ts->getUserDefinedTypeOrThrow("test.Choice").asUnion();

  auto codec = makeProtobufBinaryCodec(node);
  const auto& root = std::get<StructOp>(codec.root);
  ASSERT_EQ(root.fields.size(), 2u);

  for (const auto& field : root.fields) {
    EXPECT_TRUE(field.optional);
    EXPECT_FALSE(field.required);
  }
}

TEST_F(CodecFactoryTest, FuseJsonSourceWithCompactTarget) {
  // REST -> Thrift direction: read JSON, write Compact.
  auto jsonCodec = makeJsonCodec(personNode());
  auto compactCodec = makeThriftCompactCodec(personNode());

  const auto& jsonRoot = std::get<StructOp>(jsonCodec.root);
  const auto& compactRoot = std::get<StructOp>(compactCodec.root);

  auto fused = fuseStructOps(jsonRoot, compactRoot);
  ASSERT_FALSE(fused.hasError()) << fused.error().message;
  ASSERT_TRUE(std::holds_alternative<StructOp>(*fused));
  const auto& root = std::get<StructOp>(*fused);

  // The fused struct reads with JSON field identity (by name) and writes with
  // Compact field identity (by id).
  EXPECT_EQ(root.fieldIdent, FieldIdent::ByName);
  EXPECT_EQ(root.writeFieldIdent, FieldIdent::ById);

  // Both fields survive the fuse, matched by id.
  ASSERT_EQ(root.fields.size(), 2u);
  EXPECT_EQ(root.fields[0].fieldId, 1);
  EXPECT_EQ(root.fields[0].fieldName, "id");
  EXPECT_EQ(root.fields[1].fieldId, 2);
  EXPECT_EQ(root.fields[1].fieldName, "name");

  // The id field fuses to a scalar; its write type byte comes from the Compact
  // target, matching what the Compact codec assigns to i32.
  ASSERT_TRUE(std::holds_alternative<ScalarOp>(*root.fields[0].command));
  EXPECT_EQ(root.fields[0].writeTypeInfo, compactRoot.fields[0].writeTypeInfo);
}

} // namespace
} // namespace apache::thrift::transcode
