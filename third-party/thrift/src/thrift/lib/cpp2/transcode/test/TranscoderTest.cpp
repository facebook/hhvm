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

#include <thrift/lib/cpp2/transcode/Transcoder.h>

#include <thrift/lib/cpp2/transcode/CodecFactory.h>
#include <thrift/lib/cpp2/transcode/TranscodePlan.h>

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>

#include <cstdint>
#include <string>
#include <vector>

namespace apache::thrift::transcode {
namespace {

using def = type_system::TypeSystemBuilder::DefinitionHelper;
using type_system::TypeIds;

// Raw Thrift Binary-protocol TType bytes, used to hand-encode a sample message.
constexpr uint8_t kTStop = 0;
constexpr uint8_t kTI32 = 8;
constexpr uint8_t kTString = 11;
constexpr uint8_t kTList = 15;

void putI16BE(std::vector<uint8_t>& out, int16_t v) {
  out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
  out.push_back(static_cast<uint8_t>(v & 0xFF));
}

void putI32BE(std::vector<uint8_t>& out, int32_t v) {
  out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
  out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
  out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
  out.push_back(static_cast<uint8_t>(v & 0xFF));
}

struct TranscoderTest : ::testing::Test {
  std::unique_ptr<type_system::TypeSystem> typeSystem;

  TranscoderTest() {
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
    typeSystem = std::move(builder).build();
  }

  const type_system::StructNode& sampleNode() {
    return typeSystem->getUserDefinedTypeOrThrow("test.Sample").asStruct();
  }

  TranscodePlan fuse(const Codec& source, const Codec& target) {
    auto plan = fuseCodecs(source, target);
    EXPECT_FALSE(plan.hasError()) << plan.error().message;
    return std::move(*plan);
  }
};

// A hand-built Thrift Binary message for
// test.Sample{id=7, name="hi", nums=[10, 20, 30]}.
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

std::vector<uint8_t> toBytes(const folly::IOBuf& buf) {
  auto c = buf.cloneCoalescedAsValue();
  return std::vector<uint8_t>(c.data(), c.data() + c.length());
}

TranscoderOptions allowExperimentalProtocols() {
  TranscoderOptions options;
  options.unsupportedPlanPolicy =
      UnsupportedPlanPolicy::AllowExperimentalProtocols;
  return options;
}

TEST_F(TranscoderTest, InterpreterTranscodesCompactToBinary) {
  auto compact = makeThriftCompactCodec(sampleNode());
  auto binary = makeThriftBinaryCodec(sampleNode());

  // Derive a golden Compact buffer from the hand-built Binary message.
  auto seed = makeTranscoder(fuse(binary, compact), Engine::Interpreter);
  ASSERT_FALSE(seed.hasError()) << seed.error().message;
  const std::vector<uint8_t> binary0 = sampleBinaryMessage();
  auto binary0Buf =
      folly::IOBuf::wrapBufferAsValue(binary0.data(), binary0.size());
  auto compact0 = (*seed)->transcode(binary0Buf);
  ASSERT_FALSE(compact0.hasError()) << compact0.error().message;

  auto transcoder = makeTranscoder(fuse(compact, binary), Engine::Interpreter);
  ASSERT_FALSE(transcoder.hasError()) << transcoder.error().message;
  EXPECT_EQ((*transcoder)->engine(), Engine::Interpreter);

  // Compact -> Binary through the engine must reproduce the original bytes.
  auto binary1 = (*transcoder)->transcode(**compact0);
  ASSERT_FALSE(binary1.hasError()) << binary1.error().message;
  EXPECT_EQ(toBytes(**binary1), binary0);
}

TEST_F(TranscoderTest, JitEngineUnlinkedReturnsCompileError) {
  auto compact = makeThriftCompactCodec(sampleNode());
  auto binary = makeThriftBinaryCodec(sampleNode());

  auto transcoder = makeTranscoder(fuse(compact, binary), Engine::Jit);
  ASSERT_TRUE(transcoder.hasError());
  EXPECT_NE(transcoder.error().message.find("JIT"), std::string::npos)
      << transcoder.error().message;
}

TEST_F(TranscoderTest, JsonTargetDoesNotRequireExperimentalProtocolOptIn) {
  auto compact = makeThriftCompactCodec(sampleNode());
  auto json = makeJsonCodec(sampleNode());

  auto transcoder = makeTranscoder(fuse(compact, json), Engine::Interpreter);
  ASSERT_FALSE(transcoder.hasError()) << transcoder.error().message;
  EXPECT_EQ((*transcoder)->engine(), Engine::Interpreter);
}

TEST_F(
    TranscoderTest, JsonObjectSourceDoesNotRequireExperimentalProtocolOptIn) {
  auto json = makeJsonCodec(sampleNode());
  auto compact = makeThriftCompactCodec(sampleNode());

  auto transcoder = makeTranscoder(fuse(json, compact), Engine::Interpreter);
  ASSERT_FALSE(transcoder.hasError()) << transcoder.error().message;
  EXPECT_EQ((*transcoder)->engine(), Engine::Interpreter);
}

TEST_F(TranscoderTest, JsonToJsonRejected) {
  auto json = makeJsonCodec(sampleNode());

  auto transcoder = makeTranscoder(fuse(json, json), Engine::Interpreter);
  ASSERT_TRUE(transcoder.hasError());
  EXPECT_NE(transcoder.error().message.find("JSON-to-JSON"), std::string::npos)
      << transcoder.error().message;
}

TEST_F(TranscoderTest, ProtobufRequiresIncompleteProtocolOptIn) {
  auto protobuf = makeProtobufBinaryCodec(sampleNode());
  auto compact = makeThriftCompactCodec(sampleNode());

  auto defaultTranscoder =
      makeTranscoder(fuse(protobuf, compact), Engine::Interpreter);
  ASSERT_TRUE(defaultTranscoder.hasError());
  EXPECT_NE(
      defaultTranscoder.error().message.find("Protobuf"), std::string::npos)
      << defaultTranscoder.error().message;

  auto transcoder = makeTranscoder(
      fuse(protobuf, compact),
      Engine::Interpreter,
      allowExperimentalProtocols());
  ASSERT_FALSE(transcoder.hasError()) << transcoder.error().message;
  EXPECT_EQ((*transcoder)->engine(), Engine::Interpreter);
}

TEST_F(TranscoderTest, JitEngineRejectedBeforeProtocolGate) {
  auto compact = makeThriftCompactCodec(sampleNode());
  auto json = makeJsonCodec(sampleNode());

  auto defaultTranscoder = makeTranscoder(fuse(compact, json), Engine::Jit);
  ASSERT_TRUE(defaultTranscoder.hasError());
  EXPECT_EQ(
      defaultTranscoder.error().message.find("AllowExperimentalProtocols"),
      std::string::npos)
      << defaultTranscoder.error().message;
  EXPECT_NE(defaultTranscoder.error().message.find("JIT"), std::string::npos)
      << defaultTranscoder.error().message;

  auto transcoder = makeTranscoder(
      fuse(compact, json), Engine::Jit, allowExperimentalProtocols());
  ASSERT_TRUE(transcoder.hasError());
  EXPECT_NE(transcoder.error().message.find("JIT"), std::string::npos)
      << transcoder.error().message;
}

TEST_F(TranscoderTest, JsonProtobufRequiresIncompleteProtocolOptIn) {
  auto json = makeJsonCodec(sampleNode());
  auto protobuf = makeProtobufBinaryCodec(sampleNode());

  auto defaultTranscoder =
      makeTranscoder(fuse(json, protobuf), Engine::Interpreter);
  ASSERT_TRUE(defaultTranscoder.hasError());
  EXPECT_NE(
      defaultTranscoder.error().message.find("Protobuf"), std::string::npos)
      << defaultTranscoder.error().message;
}

TEST_F(TranscoderTest, MissingProtocolMetadataRejected) {
  auto json = makeJsonCodec(sampleNode());
  auto compact = makeThriftCompactCodec(sampleNode());

  auto fused = fuseStructOps(
      std::get<StructOp>(json.root), std::get<StructOp>(compact.root));
  ASSERT_FALSE(fused.hasError()) << fused.error().message;
  auto missingMetadata = makeTranscoder(
      TranscodePlan{"plan", std::move(*fused)}, Engine::Interpreter);
  ASSERT_TRUE(missingMetadata.hasError());
  EXPECT_NE(
      missingMetadata.error().message.find("protocol metadata"),
      std::string::npos)
      << missingMetadata.error().message;
}

} // namespace
} // namespace apache::thrift::transcode
