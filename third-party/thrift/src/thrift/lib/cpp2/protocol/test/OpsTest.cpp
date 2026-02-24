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

#include <thrift/lib/cpp2/protocol/Ops.h>

#include <thrift/lib/cpp2/dynamic/TypeId.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>
#include <thrift/lib/cpp2/protocol/FieldMask.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>
#include <thrift/lib/cpp2/protocol/test/gen-cpp2/OpsTest_types.h>
#include <thrift/lib/cpp2/reflection/testing.h>
#include <thrift/lib/cpp2/type/Any.h>

#include <gtest/gtest.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace facebook::thrift::test;

TEST(OpsTest, filter) {
  Struct s;
  s.bool_field() = true;
  s.int_field() = 42;
  s.struct_field()->int_field() = 43;
  s.struct_field()->string_field() = "foo";
  NestedStruct i1;
  i1.int_field() = 1;
  i1.string_field() = "bar";
  NestedStruct i2;
  i2.int_field() = 2;
  i2.string_field() = "baz";
  s.int_map_field() = {{1, i1}, {2, i2}};
  s.string_map_field() = {{"foo", true}, {"bar", false}};
  s.any_field() = type::AnyData::toAny(*s.struct_field()).toThrift();

  auto wholeBuf = CompactSerializer::serialize<std::string>(s);

  auto filterWithCursor = [&](const auto& mask, const auto& src) {
    auto buf = folly::IOBuf::wrapBuffer(src.data(), src.size());
    return CompactSerializer::deserialize<Struct>(
        protocol::filterSerialized<CompactSerializer>(MaskRef{mask}, buf)
            .get());
  };

  auto staticFilter = [&](const auto& mask, const Struct& src) {
    return protocol::filter(mask, src);
  };

  // Test all fields included.
  MaskBuilder<Struct> mb(noneMask());
  mb.includes<ident::bool_field>();
  mb.includes<ident::int_field>();
  mb.includes<ident::struct_field>();
  mb.includes<ident::int_map_field>();
  mb.includes<ident::string_map_field>();
  mb.includes<ident::any_field>();
  auto wholeMask = mb.toThrift();

  EXPECT_THRIFT_EQ(s, staticFilter(wholeMask, s));
  EXPECT_THRIFT_EQ(s, filterWithCursor(wholeMask, wholeBuf));
  EXPECT_THRIFT_EQ(s, filterWithCursor(allMask(), wholeBuf));

  // Test all fields excluded.
  mb.reset_to_all();
  mb.excludes<ident::bool_field>();
  mb.excludes<ident::int_field>();
  mb.excludes<ident::struct_field>();
  mb.excludes<ident::int_map_field>();
  mb.excludes<ident::string_map_field>();
  mb.excludes<ident::any_field>();
  auto emptyMask = mb.toThrift();

  EXPECT_THRIFT_EQ(Struct{}, staticFilter(emptyMask, s));
  EXPECT_THRIFT_EQ(Struct{}, filterWithCursor(emptyMask, wholeBuf));
  EXPECT_THRIFT_EQ(Struct{}, filterWithCursor(noneMask(), wholeBuf));

  // Test partial inclusion.
  mb.reset_to_none();
  mb.includes<ident::struct_field>(MaskBuilder<NestedStruct>(noneMask())
                                       .includes<ident::int_field>()
                                       .toThrift());
  mb.includes_map_element<ident::int_map_field>(1);
  mb.includes_map_element<ident::string_map_field>("foo");
  mb.includes_type<ident::any_field>(type::struct_t<NestedStruct>{});
  auto partialMask = mb.toThrift();

  Struct filtered = filterWithCursor(partialMask, wholeBuf);
  EXPECT_EQ(*filtered.int_field(), 0);
  EXPECT_EQ(*filtered.struct_field()->int_field(), 43);
  EXPECT_EQ(*filtered.struct_field()->string_field(), "");
  EXPECT_EQ(filtered.int_map_field()->size(), 1);
  EXPECT_EQ(filtered.int_map_field()->at(1).int_field(), 1);
  EXPECT_EQ(filtered.string_map_field()->size(), 1);
  EXPECT_EQ(filtered.string_map_field()->at("foo"), true);
  EXPECT_THRIFT_EQ(*filtered.any_field(), *s.any_field());

  // Test partial exclusion.
  mb.reset_to_all();
  mb.excludes<ident::struct_field>(MaskBuilder<NestedStruct>(noneMask())
                                       .includes<ident::string_field>()
                                       .toThrift());
  mb.excludes_map_element<ident::int_map_field>(2);
  mb.excludes_map_element<ident::string_map_field>("bar");
  mb.excludes_type<ident::any_field>(type::struct_t<NestedStruct>{});
  auto partialMask2 = mb.toThrift();

  filtered = filterWithCursor(partialMask2, wholeBuf);
  EXPECT_EQ(*filtered.int_field(), 42);
  EXPECT_EQ(*filtered.struct_field()->int_field(), 43);
  EXPECT_EQ(*filtered.struct_field()->string_field(), "");
  EXPECT_EQ(filtered.int_map_field()->size(), 1);
  EXPECT_EQ(filtered.int_map_field()->at(1).int_field(), 1);
  EXPECT_EQ(filtered.string_map_field()->size(), 1);
  EXPECT_EQ(filtered.string_map_field()->at("foo"), true);
  EXPECT_THRIFT_EQ(*filtered.any_field(), type::AnyStruct{});

  // Test nested mask.
  mb.reset_to_none();
  mb.includes_map_element<ident::int_map_field>(
      1,
      MaskBuilder<NestedStruct>(noneMask())
          .includes<ident::string_field>()
          .toThrift());
  mb.includes_type<ident::any_field>(
      type::struct_t<NestedStruct>{},
      MaskBuilder<NestedStruct>(noneMask())
          .includes<ident::string_field>()
          .toThrift());
  auto nestedMask = mb.toThrift();

  filtered = filterWithCursor(nestedMask, wholeBuf);
  EXPECT_EQ(filtered.int_map_field()->size(), 1);
  auto nested = filtered.int_map_field()->at(1);
  EXPECT_EQ(nested.int_field(), 0);
  EXPECT_EQ(nested.string_field(), "bar");
  nested = type::AnyData(*filtered.any_field()).get<NestedStruct>();
  EXPECT_EQ(nested.int_field(), 0);
  EXPECT_EQ(nested.string_field(), "foo");
}

// Parameterized test for all protocol combinations
enum class SerializerType {
  Binary,
  Compact,
  SimpleJSON,
};

struct ProtocolPair {
  SerializerType source;
  SerializerType dest;
};

class TranscodeTest : public ::testing::TestWithParam<ProtocolPair> {
 protected:
  template <typename Serializer>
  static std::unique_ptr<folly::IOBuf> serializeNestedStruct(
      const NestedStruct& s) {
    return Serializer::template serialize<folly::IOBufQueue>(s).move();
  }

  template <typename Serializer>
  static NestedStruct deserializeNestedStruct(const folly::IOBuf* buf) {
    return Serializer::template deserialize<NestedStruct>(buf);
  }

  template <typename SrcSerializer, typename DstSerializer>
  void testTranscode(const NestedStruct& s) {
    using namespace apache::thrift::type_system;
    using def = TypeSystemBuilder::DefinitionHelper;

    // Build a TypeSystem for NestedStruct with URI
    TypeSystemBuilder builder;
    builder.addType(
        "facebook.com/thrift/type/NestedStruct",
        def::Struct({
            def::Field(
                def::Identity(1, "int_field"), def::Optional, TypeIds::I32),
            def::Field(
                def::Identity(2, "string_field"),
                def::Optional,
                TypeIds::String),
        }));

    auto typeSystem = std::move(builder).build();
    auto structTypeRef =
        typeSystem
            ->getUserDefinedTypeOrThrow("facebook.com/thrift/type/NestedStruct")
            .asStruct()
            .asRef();

    auto srcBuf = serializeNestedStruct<SrcSerializer>(s);
    auto dstBuf = transcodeSerialized<SrcSerializer, DstSerializer>(
        srcBuf, structTypeRef);
    auto result = deserializeNestedStruct<DstSerializer>(dstBuf.get());
    EXPECT_THRIFT_EQ(s, result);
  }
};

TEST_P(TranscodeTest, AllProtocolCombinations) {
  auto param = GetParam();

  // Create nested struct for typed transcode
  NestedStruct nested;
  nested.int_field() = 42;
  nested.string_field() = "hello";

  // Test based on protocol combination
  if (param.source == SerializerType::Binary &&
      param.dest == SerializerType::Compact) {
    testTranscode<BinarySerializer, CompactSerializer>(nested);
  } else if (
      param.source == SerializerType::Compact &&
      param.dest == SerializerType::Binary) {
    testTranscode<CompactSerializer, BinarySerializer>(nested);
  } else if (
      param.source == SerializerType::SimpleJSON &&
      param.dest == SerializerType::Binary) {
    testTranscode<SimpleJSONSerializer, BinarySerializer>(nested);
  } else if (
      param.source == SerializerType::SimpleJSON &&
      param.dest == SerializerType::Compact) {
    testTranscode<SimpleJSONSerializer, CompactSerializer>(nested);
  } else if (
      param.source == SerializerType::Binary &&
      param.dest == SerializerType::SimpleJSON) {
    testTranscode<BinarySerializer, SimpleJSONSerializer>(nested);
  } else if (
      param.source == SerializerType::Compact &&
      param.dest == SerializerType::SimpleJSON) {
    testTranscode<CompactSerializer, SimpleJSONSerializer>(nested);
  }
}

INSTANTIATE_TEST_SUITE_P(
    ProtocolCombinations,
    TranscodeTest,
    ::testing::Values(
        ProtocolPair{SerializerType::Binary, SerializerType::Compact},
        ProtocolPair{SerializerType::Compact, SerializerType::Binary},
        ProtocolPair{SerializerType::SimpleJSON, SerializerType::Binary},
        ProtocolPair{SerializerType::SimpleJSON, SerializerType::Compact},
        ProtocolPair{SerializerType::Binary, SerializerType::SimpleJSON},
        ProtocolPair{SerializerType::Compact, SerializerType::SimpleJSON}));

TEST(OpsTest, transcodeWithUnknownFieldsDrop) {
  using namespace apache::thrift::type_system;
  using def = TypeSystemBuilder::DefinitionHelper;

  // Build a TypeSystem with only 2 fields
  TypeSystemBuilder builder;
  builder.addType(
      "facebook.com/thrift/type/PartialStruct",
      def::Struct({
          def::Field(
              def::Identity(1, "int_field"), def::Optional, TypeIds::I32),
          def::Field(
              def::Identity(2, "string_field"), def::Optional, TypeIds::String),
      }));

  auto typeSystem = std::move(builder).build();
  auto structTypeRef =
      typeSystem
          ->getUserDefinedTypeOrThrow("facebook.com/thrift/type/PartialStruct")
          .asStruct()
          .asRef();

  // Manually create SimpleJSON with extra fields not in schema
  folly::IOBufQueue queue;
  SimpleJSONProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeStructBegin("");
  writer.writeFieldBegin("int_field", TType::T_I32, 1);
  writer.writeI32(42);
  writer.writeFieldEnd();
  writer.writeFieldBegin("unknown_field", TType::T_STRING, 2);
  writer.writeString("ignored");
  writer.writeFieldEnd();
  writer.writeFieldBegin("string_field", TType::T_STRING, 3);
  writer.writeString("hello");
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  auto jsonBuf = queue.move();
  LOG(INFO) << "Serialized JSON: " << jsonBuf->toString();

  // Transcode SimpleJSON -> Compact with Drop policy (default)
  auto compactBuf =
      transcodeSerialized<SimpleJSONSerializer, CompactSerializer>(
          jsonBuf, structTypeRef, UnknownFieldIdPolicy::Drop);

  // Deserialize and verify unknown field was dropped
  NestedStruct result =
      CompactSerializer::deserialize<NestedStruct>(compactBuf.get());
  EXPECT_EQ(result.int_field(), 42);
  EXPECT_EQ(result.string_field(), "hello");
}

TEST(OpsTest, transcodeWithUnknownFieldsThrow) {
  using namespace apache::thrift::type_system;
  using def = TypeSystemBuilder::DefinitionHelper;

  // Build a TypeSystem with only 2 fields
  TypeSystemBuilder builder;
  builder.addType(
      "facebook.com/thrift/type/PartialStruct",
      def::Struct({
          def::Field(
              def::Identity(1, "int_field"), def::Optional, TypeIds::I32),
          def::Field(
              def::Identity(2, "string_field"), def::Optional, TypeIds::String),
      }));

  auto typeSystem = std::move(builder).build();
  auto structTypeRef =
      typeSystem
          ->getUserDefinedTypeOrThrow("facebook.com/thrift/type/PartialStruct")
          .asStruct()
          .asRef();

  // Manually create SimpleJSON with extra fields not in schema
  folly::IOBufQueue queue;
  SimpleJSONProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeStructBegin("");
  writer.writeFieldBegin("int_field", TType::T_I32, 1);
  writer.writeI32(42);
  writer.writeFieldEnd();
  writer.writeFieldBegin("unknown_field", TType::T_STRING, 2);
  writer.writeString("ignored");
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  auto jsonBuf = queue.move();

  // Transcode SimpleJSON -> Compact with Throw policy should throw
  bool threwException = false;
  try {
    transcodeSerialized<SimpleJSONSerializer, CompactSerializer>(
        jsonBuf, structTypeRef, UnknownFieldIdPolicy::Throw);
  } catch (const std::runtime_error&) {
    threwException = true;
  }
  EXPECT_TRUE(threwException);
}

TEST(OpsTest, transcodeFieldIdProtocolPreservesUnknownFields) {
  // Manually create Compact data with fields 1, 2, and 99 (not in schema).
  // Field ID-based protocols encode field IDs on the wire, so unknown fields
  // are always preserved during transcoding regardless of UnknownFieldIdPolicy.
  folly::IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeStructBegin("");
  writer.writeFieldBegin("", TType::T_I32, 1);
  writer.writeI32(42);
  writer.writeFieldEnd();
  writer.writeFieldBegin("", TType::T_STRING, 2);
  writer.writeString("hello");
  writer.writeFieldEnd();
  writer.writeFieldBegin("", TType::T_STRING, 99);
  writer.writeString("unknown_value");
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  auto compactBuf = queue.move();

  // Transcode Compact -> Binary even with Throw policy — field ID-based
  // protocols never trigger the unknown field check.
  auto binaryBuf = transcodeSerialized<CompactSerializer, BinarySerializer>(
      compactBuf, std::nullopt, UnknownFieldIdPolicy::Throw);

  // Read back with BinaryProtocolReader and verify all fields are preserved
  BinaryProtocolReader reader;
  reader.setInput(binaryBuf.get());
  std::string name;
  TType fieldType;
  int16_t fieldId;

  reader.readStructBegin(name);

  reader.readFieldBegin(name, fieldType, fieldId);
  EXPECT_EQ(fieldId, 1);
  EXPECT_EQ(fieldType, TType::T_I32);
  int32_t intVal;
  reader.readI32(intVal);
  EXPECT_EQ(intVal, 42);
  reader.readFieldEnd();

  reader.readFieldBegin(name, fieldType, fieldId);
  EXPECT_EQ(fieldId, 2);
  EXPECT_EQ(fieldType, TType::T_STRING);
  std::string strVal;
  reader.readString(strVal);
  EXPECT_EQ(strVal, "hello");
  reader.readFieldEnd();

  reader.readFieldBegin(name, fieldType, fieldId);
  EXPECT_EQ(fieldId, 99);
  EXPECT_EQ(fieldType, TType::T_STRING);
  reader.readString(strVal);
  EXPECT_EQ(strVal, "unknown_value");
  reader.readFieldEnd();

  reader.readFieldBegin(name, fieldType, fieldId);
  EXPECT_EQ(fieldType, TType::T_STOP);
  reader.readStructEnd();
}
