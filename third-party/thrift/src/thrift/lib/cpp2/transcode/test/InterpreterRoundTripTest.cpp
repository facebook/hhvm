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

#include <thrift/lib/cpp2/transcode/TranscodeInterpreter.h>

#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/transcode/CodecFactory.h>
#include <thrift/lib/cpp2/transcode/TranscodePlan.h>
#include <thrift/lib/cpp2/transcode/WireType.h>
#include <thrift/lib/cpp2/transcode/test/gen-cpp2/InterpreterRoundTripFixtures_types.h>

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace apache::thrift::transcode {
namespace {

namespace fixture = apache::thrift::transcode::test;

using def = type_system::TypeSystemBuilder::DefinitionHelper;
using type_system::TypeIds;

// Raw Thrift Binary-protocol TType bytes, used to hand-encode a sample message.
constexpr uint8_t kTStop = 0;
constexpr uint8_t kTByte = 3;
constexpr uint8_t kTI16 = 6;
constexpr uint8_t kTI32 = 8;
constexpr uint8_t kTString = 11;
constexpr uint8_t kTStruct = 12;
constexpr uint8_t kTList = 15;

void putI16BE(std::vector<uint8_t>& out, int16_t v) {
  auto u = static_cast<uint16_t>(v);
  out.push_back(static_cast<uint8_t>((u >> 8) & 0xFF));
  out.push_back(static_cast<uint8_t>(u & 0xFF));
}

void putI32BE(std::vector<uint8_t>& out, int32_t v) {
  out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
  out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
  out.push_back(static_cast<uint8_t>(v & 0xFF));
}

void putUVarint(std::vector<uint8_t>& out, uint64_t v) {
  while (v >= 0x80) {
    out.push_back(static_cast<uint8_t>((v & 0x7F) | 0x80));
    v >>= 7;
  }
  out.push_back(static_cast<uint8_t>(v));
}

// A struct with a scalar int, a string, and a list<i32>, exercising the
// scalar, length-prefixed-bytes, and sequence executors in one message.
struct InterpreterRoundTripTest : ::testing::Test {
  std::unique_ptr<type_system::TypeSystem> typeSystem;

  InterpreterRoundTripTest() {
    type_system::TypeSystemBuilder builder;
    builder.addType(
        "test.Sample",
        def::Struct({
            def::Field(
                def::Identity(1, "id"), def::AlwaysPresent, TypeIds::I32),
            def::Field(
                def::Identity(2, "name"), def::AlwaysPresent, TypeIds::String),
            def::Field(
                def::Identity(3, "nums"),
                def::AlwaysPresent,
                TypeIds::list(TypeIds::I32)),
        }));
    builder.addType(
        "test.ScalarSample",
        def::Struct({
            def::Field(
                def::Identity(1, "b"), def::AlwaysPresent, TypeIds::Byte),
            def::Field(def::Identity(2, "s"), def::AlwaysPresent, TypeIds::I16),
        }));
    builder.addType(
        "test.Inner",
        def::Struct({
            def::Field(
                def::Identity(1, "id"), def::AlwaysPresent, TypeIds::I32),
        }));
    builder.addType(
        "test.Outer",
        def::Struct({
            def::Field(
                def::Identity(1, "inner"),
                def::AlwaysPresent,
                TypeIds::uri("test.Inner")),
        }));
    builder.addType("test.Color", def::Enum({{"RED", 1}, {"BLUE", 2}}));
    builder.addType(
        "test.EnumSample",
        def::Struct({
            def::Field(
                def::Identity(1, "color"),
                def::AlwaysPresent,
                TypeIds::uri("test.Color")),
        }));
    typeSystem = std::move(builder).build();
  }

  const type_system::StructNode& sampleNode() {
    return typeSystem->getUserDefinedTypeOrThrow("test.Sample").asStruct();
  }

  const type_system::StructNode& scalarNode() {
    return typeSystem->getUserDefinedTypeOrThrow("test.ScalarSample")
        .asStruct();
  }

  const type_system::StructNode& outerNode() {
    return typeSystem->getUserDefinedTypeOrThrow("test.Outer").asStruct();
  }

  const type_system::StructNode& enumNode() {
    return typeSystem->getUserDefinedTypeOrThrow("test.EnumSample").asStruct();
  }

  TranscodePlan fuse(const Codec& source, const Codec& target) {
    auto plan = fuseCodecs(source, target);
    EXPECT_FALSE(plan.hasError()) << plan.error().message;
    return std::move(*plan);
  }
};

// A hand-built Thrift Binary message for
// test.Sample{id=7, name="hi", nums=[10, 20, 30]}, using the raw Binary wire
// format (field type byte, i16 BE id, payload; STOP terminates).
std::vector<uint8_t> sampleBinaryMessage() {
  std::vector<uint8_t> b;

  b.push_back(kTI32); // field 1: i32
  putI16BE(b, 1);
  putI32BE(b, 7);

  b.push_back(kTString); // field 2: string "hi"
  putI16BE(b, 2);
  putI32BE(b, 2);
  b.push_back('h');
  b.push_back('i');

  b.push_back(kTList); // field 3: list<i32> [10, 20, 30]
  putI16BE(b, 3);
  b.push_back(kTI32); // element type
  putI32BE(b, 3); // count
  putI32BE(b, 10);
  putI32BE(b, 20);
  putI32BE(b, 30);

  b.push_back(kTStop);
  return b;
}

std::vector<uint8_t> sampleBinaryMessageOutOfOrder() {
  std::vector<uint8_t> b;

  b.push_back(kTList); // field 3: list<i32> [10, 20, 30]
  putI16BE(b, 3);
  b.push_back(kTI32);
  putI32BE(b, 3);
  putI32BE(b, 10);
  putI32BE(b, 20);
  putI32BE(b, 30);

  b.push_back(kTI32); // field 1: i32
  putI16BE(b, 1);
  putI32BE(b, 7);

  b.push_back(kTString); // field 2: string "hi"
  putI16BE(b, 2);
  putI32BE(b, 2);
  b.push_back('h');
  b.push_back('i');

  b.push_back(kTStop);
  return b;
}

std::vector<uint8_t> scalarBinaryMessage() {
  std::vector<uint8_t> b;

  b.push_back(kTByte); // field 1: byte -2
  putI16BE(b, 1);
  b.push_back(0xFE);

  b.push_back(kTI16); // field 2: i16 -1
  putI16BE(b, 2);
  putI16BE(b, -1);

  b.push_back(kTStop);
  return b;
}

std::vector<uint8_t> binaryWrongTypeMessage() {
  std::vector<uint8_t> b;

  b.push_back(kTString); // field 1 is schema i32, not string
  putI16BE(b, 1);
  putI32BE(b, 0);

  b.push_back(kTStop);
  return b;
}

std::vector<uint8_t> binaryNegativeListCountMessage() {
  std::vector<uint8_t> b;

  b.push_back(kTList);
  putI16BE(b, 3);
  b.push_back(kTI32);
  putI32BE(b, -1);

  b.push_back(kTStop);
  return b;
}

std::vector<uint8_t> binaryWrongListElementTypeMessage() {
  std::vector<uint8_t> b;

  b.push_back(kTList);
  putI16BE(b, 3);
  b.push_back(kTString);
  putI32BE(b, 0);

  b.push_back(kTStop);
  return b;
}

std::vector<uint8_t> compactOversizedListCountMessage() {
  std::vector<uint8_t> b;

  b.push_back(static_cast<uint8_t>((3 << 4) | wire::kCompactList));
  b.push_back(static_cast<uint8_t>(0xF0 | wire::kCompactI32));
  putUVarint(b, static_cast<uint64_t>(std::numeric_limits<int32_t>::max()) + 1);

  b.push_back(wire::kCompactStop);
  return b;
}

std::vector<uint8_t> protobufPackedListLengthOverrunMessage() {
  std::vector<uint8_t> b;

  putUVarint(b, (static_cast<uint64_t>(3) << 3) | 2);
  putUVarint(b, 5);
  putUVarint(b, 10);

  return b;
}

std::vector<uint8_t> protobufNestedStructLengthOverrunMessage() {
  std::vector<uint8_t> b;

  putUVarint(b, (static_cast<uint64_t>(1) << 3) | 2);
  putUVarint(b, 4);
  putUVarint(b, (static_cast<uint64_t>(1) << 3) | 0);

  return b;
}

std::vector<uint8_t> protobufNestedStructMessage() {
  std::vector<uint8_t> b;

  putUVarint(b, (static_cast<uint64_t>(1) << 3) | 2);
  putUVarint(b, 2);
  putUVarint(b, (static_cast<uint64_t>(1) << 3) | 0);
  putUVarint(b, 14);

  return b;
}

std::vector<uint8_t> binaryNestedStructMessage() {
  std::vector<uint8_t> b;

  b.push_back(kTStruct);
  putI16BE(b, 1);
  b.push_back(kTI32);
  putI16BE(b, 1);
  putI32BE(b, 7);
  b.push_back(kTStop);
  b.push_back(kTStop);

  return b;
}

std::string enumBinaryMessage(fixture::Color color) {
  fixture::EnumSample value;
  value.color() = color;
  return BinarySerializer::serialize<std::string>(value);
}

std::vector<uint8_t> toBytes(const folly::IOBuf& buf) {
  auto c = buf.cloneCoalescedAsValue();
  return std::vector<uint8_t>(c.data(), c.data() + c.length());
}

TEST_F(InterpreterRoundTripTest, CompactBinaryRoundTripIsIdentity) {
  auto compact = makeThriftCompactCodec(sampleNode());
  auto binary = makeThriftBinaryCodec(sampleNode());

  TranscodeInterpreter binaryToCompact{fuse(binary, compact)};
  TranscodeInterpreter compactToBinary{fuse(compact, binary)};

  const std::vector<uint8_t> binary0 = sampleBinaryMessage();
  auto binary0Buf =
      folly::IOBuf::wrapBufferAsValue(binary0.data(), binary0.size());

  // Binary -> Compact gives a golden Compact buffer (derived, not hardcoded).
  auto compact0 = binaryToCompact.transcode(binary0Buf);
  ASSERT_FALSE(compact0.hasError()) << compact0.error().message;

  // Compact -> Binary should reproduce the original Binary bytes exactly.
  auto binary1 = compactToBinary.transcode(**compact0);
  ASSERT_FALSE(binary1.hasError()) << binary1.error().message;
  EXPECT_EQ(toBytes(**binary1), binary0);

  // Binary -> Compact again must land back on the golden Compact buffer.
  auto compact1 = binaryToCompact.transcode(**binary1);
  ASSERT_FALSE(compact1.hasError()) << compact1.error().message;
  EXPECT_EQ(toBytes(**compact1), toBytes(**compact0));
}

TEST_F(InterpreterRoundTripTest, DispatchesFieldsByIdWhenWireOrderDiffers) {
  auto compact = makeThriftCompactCodec(sampleNode());
  auto binary = makeThriftBinaryCodec(sampleNode());

  TranscodeInterpreter binaryToCompact{fuse(binary, compact)};
  TranscodeInterpreter compactToBinary{fuse(compact, binary)};

  const std::vector<uint8_t> binary0 = sampleBinaryMessageOutOfOrder();
  auto binary0Buf =
      folly::IOBuf::wrapBufferAsValue(binary0.data(), binary0.size());

  auto compact0 = binaryToCompact.transcode(binary0Buf);
  ASSERT_FALSE(compact0.hasError()) << compact0.error().message;

  auto binary1 = compactToBinary.transcode(**compact0);
  ASSERT_FALSE(binary1.hasError()) << binary1.error().message;
  EXPECT_EQ(toBytes(**binary1), binary0);
}

TEST_F(InterpreterRoundTripTest, TruncatedInputYieldsError) {
  auto compact = makeThriftCompactCodec(sampleNode());
  auto binary = makeThriftBinaryCodec(sampleNode());
  TranscodeInterpreter binaryToCompact{fuse(binary, compact)};

  // Chop the message mid-value: keep field 1's header + only 2 of its 4 i32
  // payload bytes, so the reader runs off the end.
  std::vector<uint8_t> truncated = sampleBinaryMessage();
  truncated.resize(5);
  auto buf =
      folly::IOBuf::wrapBufferAsValue(truncated.data(), truncated.size());

  auto result = binaryToCompact.transcode(buf);
  ASSERT_TRUE(result.hasError());
  EXPECT_TRUE(
      result.error().code == TranscodeErrc::Truncated ||
      result.error().code == TranscodeErrc::Malformed)
      << "unexpected code: " << toString(result.error().code);
}

TEST_F(InterpreterRoundTripTest, WrongFieldTypeYieldsError) {
  auto compact = makeThriftCompactCodec(sampleNode());
  auto binary = makeThriftBinaryCodec(sampleNode());
  TranscodeInterpreter binaryToCompact{fuse(binary, compact)};

  auto input = binaryWrongTypeMessage();
  auto buf = folly::IOBuf::wrapBufferAsValue(input.data(), input.size());

  auto result = binaryToCompact.transcode(buf);
  ASSERT_TRUE(result.hasError());
}

TEST_F(InterpreterRoundTripTest, NegativeBinaryContainerCountYieldsError) {
  auto compact = makeThriftCompactCodec(sampleNode());
  auto binary = makeThriftBinaryCodec(sampleNode());
  TranscodeInterpreter binaryToCompact{fuse(binary, compact)};

  auto input = binaryNegativeListCountMessage();
  auto buf = folly::IOBuf::wrapBufferAsValue(input.data(), input.size());

  auto result = binaryToCompact.transcode(buf);
  ASSERT_TRUE(result.hasError());
}

TEST_F(InterpreterRoundTripTest, WrongBinaryListElementTypeYieldsError) {
  auto compact = makeThriftCompactCodec(sampleNode());
  auto binary = makeThriftBinaryCodec(sampleNode());
  TranscodeInterpreter binaryToCompact{fuse(binary, compact)};

  auto input = binaryWrongListElementTypeMessage();
  auto buf = folly::IOBuf::wrapBufferAsValue(input.data(), input.size());

  auto result = binaryToCompact.transcode(buf);
  ASSERT_TRUE(result.hasError());
}

TEST_F(InterpreterRoundTripTest, OversizedCompactContainerCountYieldsError) {
  auto compact = makeThriftCompactCodec(sampleNode());
  auto binary = makeThriftBinaryCodec(sampleNode());
  TranscodeInterpreter compactToBinary{fuse(compact, binary)};

  auto input = compactOversizedListCountMessage();
  auto buf = folly::IOBuf::wrapBufferAsValue(input.data(), input.size());

  auto result = compactToBinary.transcode(buf);
  ASSERT_TRUE(result.hasError());
}

TEST_F(InterpreterRoundTripTest, ProtobufPackedLengthOverrunYieldsError) {
  auto protobuf = makeProtobufBinaryCodec(sampleNode());
  auto binary = makeThriftBinaryCodec(sampleNode());
  TranscodeInterpreter protobufToBinary{fuse(protobuf, binary)};

  auto input = protobufPackedListLengthOverrunMessage();
  auto buf = folly::IOBuf::wrapBufferAsValue(input.data(), input.size());

  auto result = protobufToBinary.transcode(buf);
  ASSERT_TRUE(result.hasError());
}

TEST_F(InterpreterRoundTripTest, ProtobufNestedLengthOverrunYieldsError) {
  auto protobuf = makeProtobufBinaryCodec(outerNode());
  auto binary = makeThriftBinaryCodec(outerNode());
  TranscodeInterpreter protobufToBinary{fuse(protobuf, binary)};

  auto input = protobufNestedStructLengthOverrunMessage();
  auto buf = folly::IOBuf::wrapBufferAsValue(input.data(), input.size());

  auto result = protobufToBinary.transcode(buf);
  ASSERT_TRUE(result.hasError());
}

TEST_F(InterpreterRoundTripTest, ProtobufNestedStructWritesThriftStop) {
  auto protobuf = makeProtobufBinaryCodec(outerNode());
  auto binary = makeThriftBinaryCodec(outerNode());
  TranscodeInterpreter protobufToBinary{fuse(protobuf, binary)};

  auto input = protobufNestedStructMessage();
  auto buf = folly::IOBuf::wrapBufferAsValue(input.data(), input.size());

  auto result = protobufToBinary.transcode(buf);
  ASSERT_FALSE(result.hasError()) << result.error().message;
  EXPECT_EQ(toBytes(**result), binaryNestedStructMessage());
}

TEST_F(InterpreterRoundTripTest, BinarySignedScalarsTranscodeToJson) {
  auto binary = makeThriftBinaryCodec(scalarNode());
  auto json = makeJsonCodec(scalarNode());
  TranscodeInterpreter binaryToJson{fuse(binary, json)};

  const std::vector<uint8_t> binary0 = scalarBinaryMessage();
  auto binary0Buf =
      folly::IOBuf::wrapBufferAsValue(binary0.data(), binary0.size());

  auto result = binaryToJson.transcode(binary0Buf);
  ASSERT_FALSE(result.hasError()) << result.error().message;

  auto jsonBytes = toBytes(**result);
  EXPECT_EQ(
      std::string(jsonBytes.begin(), jsonBytes.end()), R"({"b":-2,"s":-1})");
}

TEST_F(InterpreterRoundTripTest, BinaryEnumTranscodesToJsonNameWhenKnown) {
  auto binary = makeThriftBinaryCodec(enumNode());
  auto json = makeJsonCodec(enumNode());
  TranscodeInterpreter binaryToJson{fuse(binary, json)};

  auto binary0 = enumBinaryMessage(fixture::Color::RED);
  auto binary0Buf = folly::IOBuf::copyBuffer(binary0.data(), binary0.size());

  auto result = binaryToJson.transcode(*binary0Buf);
  ASSERT_FALSE(result.hasError()) << result.error().message;

  auto jsonBytes = toBytes(**result);
  EXPECT_EQ(
      std::string(jsonBytes.begin(), jsonBytes.end()), R"({"color":"RED"})");
}

TEST_F(InterpreterRoundTripTest, BinaryEnumTranscodesToJsonNumberWhenUnknown) {
  auto binary = makeThriftBinaryCodec(enumNode());
  auto json = makeJsonCodec(enumNode());
  TranscodeInterpreter binaryToJson{fuse(binary, json)};

  auto binary0 = enumBinaryMessage(static_cast<fixture::Color>(99));
  auto binary0Buf = folly::IOBuf::copyBuffer(binary0.data(), binary0.size());

  auto result = binaryToJson.transcode(*binary0Buf);
  ASSERT_FALSE(result.hasError()) << result.error().message;

  auto jsonBytes = toBytes(**result);
  EXPECT_EQ(std::string(jsonBytes.begin(), jsonBytes.end()), R"({"color":99})");
}

} // namespace
} // namespace apache::thrift::transcode
