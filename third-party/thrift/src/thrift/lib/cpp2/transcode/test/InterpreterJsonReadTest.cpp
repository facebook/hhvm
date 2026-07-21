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

#include <thrift/lib/cpp2/transcode/CodecFactory.h>
#include <thrift/lib/cpp2/transcode/TranscodePlan.h>
#include <thrift/lib/cpp2/transcode/Transcoder.h>

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace apache::thrift::transcode {
namespace {

using def = type_system::TypeSystemBuilder::DefinitionHelper;
using type_system::TypeIds;

std::vector<uint8_t> toBytes(const folly::IOBuf& buf) {
  auto c = buf.cloneCoalescedAsValue();
  return std::vector<uint8_t>(c.data(), c.data() + c.length());
}

std::string toStr(const folly::IOBuf& buf) {
  auto c = buf.cloneCoalescedAsValue();
  return std::string(reinterpret_cast<const char*>(c.data()), c.length());
}

folly::IOBuf wrap(const std::string& s) {
  return folly::IOBuf::wrapBufferAsValue(s.data(), s.size());
}

TranscoderOptions allowExperimentalProtocols() {
  TranscoderOptions options;
  options.unsupportedPlanPolicy =
      UnsupportedPlanPolicy::AllowExperimentalProtocols;
  return options;
}

// Sample schema: scalars (i32/string/bool), a nested struct, and a list<i32> —
// exercises the JSON object reader, the deferred Compact-bool field, the nested
// object reader, and the JSON array reader in one message.
struct InterpreterJsonReadTest : ::testing::Test {
  std::unique_ptr<type_system::TypeSystem> typeSystem;

  InterpreterJsonReadTest() {
    type_system::TypeSystemBuilder builder;
    builder.addType(
        "test.Nested",
        def::Struct({
            def::Field(def::Identity(1, "n"), def::AlwaysPresent, TypeIds::I32),
        }));
    builder.addType(
        "test.Sample",
        def::Struct({
            def::Field(
                def::Identity(1, "id"), def::AlwaysPresent, TypeIds::I32),
            def::Field(
                def::Identity(2, "name"), def::AlwaysPresent, TypeIds::String),
            def::Field(
                def::Identity(3, "flag"), def::AlwaysPresent, TypeIds::Bool),
            def::Field(
                def::Identity(4, "nested"),
                def::AlwaysPresent,
                TypeIds::uri("test.Nested")),
            def::Field(
                def::Identity(5, "nums"),
                def::AlwaysPresent,
                TypeIds::list(TypeIds::I32)),
        }));
    typeSystem = std::move(builder).build();
  }

  const type_system::StructNode& sampleNode() {
    return typeSystem->getUserDefinedTypeOrThrow("test.Sample").asStruct();
  }

  TranscodePlan fuse(const Codec& source, const Codec& target) {
    auto fused = fuseStructOps(
        std::get<StructOp>(source.root), std::get<StructOp>(target.root));
    EXPECT_FALSE(fused.hasError()) << fused.error().message;
    TranscodePlan plan{"plan", std::move(*fused)};
    plan.sourceProtocol = source.protocol;
    plan.targetProtocol = target.protocol;
    return plan;
  }
};

TEST_F(InterpreterJsonReadTest, JsonToCompactProducesStableBytes) {
  auto compact = makeThriftCompactCodec(sampleNode());
  auto json = makeJsonCodec(sampleNode());

  TranscodeInterpreter jsonToCompact{fuse(json, compact)};

  const std::string json0 = R"({"id": 7, "name": "hi", "flag": true, )"
                            R"("nested": {"n": 99}, "nums": [10, 20, 30]})";
  const std::string canonicalJson =
      R"({"id":7,"name":"hi","flag":true,"nested":{"n":99},"nums":[10,20,30]})";

  auto compact0 = jsonToCompact.transcode(wrap(json0));
  ASSERT_FALSE(compact0.hasError()) << compact0.error().message;

  auto compact1 = jsonToCompact.transcode(wrap(canonicalJson));
  ASSERT_FALSE(compact1.hasError()) << compact1.error().message;
  EXPECT_EQ(toBytes(**compact0), toBytes(**compact1));
}

// An empty JSON array must produce an empty Compact list (count 0).
TEST_F(InterpreterJsonReadTest, EmptyArrayProducesStableBytes) {
  auto compact = makeThriftCompactCodec(sampleNode());
  auto json = makeJsonCodec(sampleNode());
  TranscodeInterpreter jsonToCompact{fuse(json, compact)};

  const std::string json0 =
      R"({"id": 1, "name": "x", "flag": false, "nested": {"n": 0}, "nums": []})";
  const std::string canonicalJson =
      R"({"id":1,"name":"x","flag":false,"nested":{"n":0},"nums":[]})";

  auto compact0 = jsonToCompact.transcode(wrap(json0));
  ASSERT_FALSE(compact0.hasError()) << compact0.error().message;
  auto compact1 = jsonToCompact.transcode(wrap(canonicalJson));
  ASSERT_FALSE(compact1.hasError()) << compact1.error().message;
  EXPECT_EQ(toBytes(**compact0), toBytes(**compact1));
}

// An extra unknown key must be skipped via skip_json_value and not disturb the
// surrounding fields (unknown object value nested here to exercise recursion).
TEST_F(InterpreterJsonReadTest, UnknownFieldIsSkipped) {
  auto compact = makeThriftCompactCodec(sampleNode());
  auto json = makeJsonCodec(sampleNode());
  TranscodeInterpreter jsonToCompact{fuse(json, compact)};

  const std::string withUnknown =
      R"({"id": 7, "extra": {"a": [1, 2], "b": "skip me"}, "name": "hi", )"
      R"("flag": true, "nested": {"n": 99}, "nums": [10, 20, 30]})";
  const std::string withoutUnknown =
      R"({"id":7,"name":"hi","flag":true,"nested":{"n":99},"nums":[10,20,30]})";

  auto compact0 = jsonToCompact.transcode(wrap(withUnknown));
  ASSERT_FALSE(compact0.hasError()) << compact0.error().message;
  auto compact1 = jsonToCompact.transcode(wrap(withoutUnknown));
  ASSERT_FALSE(compact1.hasError()) << compact1.error().message;
  EXPECT_EQ(toBytes(**compact0), toBytes(**compact1));
}

// A string value with \n and \uXXXX escapes must decode to the right bytes
// (the JSON reader uses the escape-aware string intrinsic).
TEST_F(InterpreterJsonReadTest, EscapedStringValueRoundTrips) {
  auto compact = makeThriftCompactCodec(sampleNode());
  auto json = makeJsonCodec(sampleNode());
  TranscodeInterpreter jsonToCompact{fuse(json, compact)};

  // Non-raw literal so the JSON reader gets real JSON escapes: a 6-char unicode
  // escape (U+0041, the letter A) next to the newline escape. A bare
  // backslash-u in a normal C++ literal would form a universal-character-name
  // and collapse to the letter A before the reader ever sees an escape. The
  // name decodes to the bytes a, LF, b, A.
  const std::string json0 =
      "{\"id\": 1, \"name\": \"a\\nb\\u0041\", \"flag\": false, "
      "\"nested\": {\"n\": 0}, \"nums\": []}";

  auto compact0 = jsonToCompact.transcode(wrap(json0));
  ASSERT_FALSE(compact0.hasError()) << compact0.error().message;

  // The Compact string bytes must be the unescaped form.
  auto bytes = toBytes(**compact0);
  const std::string needle = "a\nbA";
  bool found = false;
  for (size_t i = 0; i + needle.size() <= bytes.size(); ++i) {
    if (std::memcmp(bytes.data() + i, needle.data(), needle.size()) == 0) {
      found = true;
      break;
    }
  }
  EXPECT_TRUE(found) << "unescaped string not found in Compact output";
}

// A JSON object missing its closing brace must error, not crash or read OOB.
TEST_F(InterpreterJsonReadTest, MalformedMissingCloseBraceErrors) {
  auto compact = makeThriftCompactCodec(sampleNode());
  auto json = makeJsonCodec(sampleNode());
  TranscodeInterpreter jsonToCompact{fuse(json, compact)};

  const std::string bad =
      R"({"id": 7, "name": "hi", "flag": true, "nested": {"n": 99}, )"
      R"("nums": [10, 20, 30])"; // no closing '}'

  auto result = jsonToCompact.transcode(wrap(bad));
  EXPECT_TRUE(result.hasError());
}

// A bad separator (missing ':') must error rather than misparse.
TEST_F(InterpreterJsonReadTest, MalformedMissingColonErrors) {
  auto compact = makeThriftCompactCodec(sampleNode());
  auto json = makeJsonCodec(sampleNode());
  TranscodeInterpreter jsonToCompact{fuse(json, compact)};

  const std::string bad = R"({"id" 7})"; // missing ':'

  auto result = jsonToCompact.transcode(wrap(bad));
  EXPECT_TRUE(result.hasError());
}

TEST_F(InterpreterJsonReadTest, TrailingRootTokenErrors) {
  auto compact = makeThriftCompactCodec(sampleNode());
  auto json = makeJsonCodec(sampleNode());
  TranscodeInterpreter jsonToCompact{fuse(json, compact)};

  const std::string bad =
      R"({"id": 1, "name": "x", "flag": false, "nested": {"n": 0}, "nums": []} true)";

  auto result = jsonToCompact.transcode(wrap(bad));
  EXPECT_TRUE(result.hasError());
}

TEST_F(InterpreterJsonReadTest, TrailingRootWhitespaceIsAccepted) {
  auto compact = makeThriftCompactCodec(sampleNode());
  auto json = makeJsonCodec(sampleNode());
  TranscodeInterpreter jsonToCompact{fuse(json, compact)};

  const std::string input =
      R"({"id": 1, "name": "x", "flag": false, "nested": {"n": 0}, "nums": []}  )";

  auto result = jsonToCompact.transcode(wrap(input));
  EXPECT_FALSE(result.hasError()) << result.error().message;
}

TEST_F(InterpreterJsonReadTest, BinaryFieldUsesBase64JsonString) {
  type_system::TypeSystemBuilder builder;
  builder.addType(
      "test.WithBinary",
      def::Struct({
          def::Field(
              def::Identity(1, "data"), def::AlwaysPresent, TypeIds::Binary),
      }));
  auto ts = std::move(builder).build();
  const auto& node =
      ts->getUserDefinedTypeOrThrow("test.WithBinary").asStruct();

  auto compact = makeThriftCompactCodec(node);
  auto json = makeJsonCodec(node);
  TranscodeInterpreter jsonToCompact{fuse(json, compact)};

  auto compact0 = jsonToCompact.transcode(wrap(R"({"data":"aGk="})"));
  ASSERT_FALSE(compact0.hasError()) << compact0.error().message;

  auto bytes = toBytes(**compact0);
  const std::string needle = "hi";
  bool found = false;
  for (size_t i = 0; i + needle.size() <= bytes.size(); ++i) {
    if (std::memcmp(bytes.data() + i, needle.data(), needle.size()) == 0) {
      found = true;
      break;
    }
  }
  EXPECT_TRUE(found) << "decoded binary not found in Compact output";
}

TEST_F(InterpreterJsonReadTest, OptionalFieldNullIsOmitted) {
  type_system::TypeSystemBuilder builder;
  builder.addType(
      "test.WithOptional",
      def::Struct({
          def::Field(def::Identity(1, "id"), def::AlwaysPresent, TypeIds::I32),
          def::Field(def::Identity(2, "name"), def::Optional, TypeIds::String),
      }));
  auto ts = std::move(builder).build();
  const auto& node =
      ts->getUserDefinedTypeOrThrow("test.WithOptional").asStruct();

  auto compact = makeThriftCompactCodec(node);
  auto json = makeJsonCodec(node);
  TranscodeInterpreter jsonToCompact{fuse(json, compact)};

  auto compact0 = jsonToCompact.transcode(wrap(R"({"id":7,"name":null})"));
  ASSERT_FALSE(compact0.hasError()) << compact0.error().message;

  auto compact1 = jsonToCompact.transcode(wrap(R"({"id":7})"));
  ASSERT_FALSE(compact1.hasError()) << compact1.error().message;
  EXPECT_EQ(toBytes(**compact0), toBytes(**compact1));
}

TEST_F(InterpreterJsonReadTest, NullForAlwaysPresentFieldErrors) {
  auto compact = makeThriftCompactCodec(sampleNode());
  auto json = makeJsonCodec(sampleNode());
  TranscodeInterpreter jsonToCompact{fuse(json, compact)};

  auto result = jsonToCompact.transcode(wrap(R"({"id":null})"));
  EXPECT_TRUE(result.hasError());
}

TEST_F(InterpreterJsonReadTest, MissingAlwaysPresentFieldsUseStandardDefaults) {
  auto compact = makeThriftCompactCodec(sampleNode());
  auto binary = makeThriftBinaryCodec(sampleNode());
  auto json = makeJsonCodec(sampleNode());
  TranscodeInterpreter jsonToCompact{fuse(json, compact)};
  TranscodeInterpreter compactToBinary{fuse(compact, binary)};

  auto compact0 = jsonToCompact.transcode(wrap("{}"));
  ASSERT_FALSE(compact0.hasError()) << compact0.error().message;

  auto compact1 = jsonToCompact.transcode(
      wrap(R"({"id":0,"name":"","flag":false,"nested":{"n":0},"nums":[]})"));
  ASSERT_FALSE(compact1.hasError()) << compact1.error().message;

  auto binary0 = compactToBinary.transcode(**compact0);
  ASSERT_FALSE(binary0.hasError()) << binary0.error().message;
  auto binary1 = compactToBinary.transcode(**compact1);
  ASSERT_FALSE(binary1.hasError()) << binary1.error().message;
  EXPECT_EQ(toBytes(**binary0), toBytes(**binary1));
}

TEST_F(InterpreterJsonReadTest, NarrowIntegerOutOfRangeErrors) {
  type_system::TypeSystemBuilder builder;
  builder.addType(
      "test.WithByte",
      def::Struct({
          def::Field(def::Identity(1, "b"), def::AlwaysPresent, TypeIds::Byte),
      }));
  auto ts = std::move(builder).build();
  const auto& node = ts->getUserDefinedTypeOrThrow("test.WithByte").asStruct();

  auto compact = makeThriftCompactCodec(node);
  auto json = makeJsonCodec(node);
  TranscodeInterpreter jsonToCompact{fuse(json, compact)};

  auto result = jsonToCompact.transcode(wrap(R"({"b":128})"));
  EXPECT_TRUE(result.hasError());
}

TEST_F(InterpreterJsonReadTest, JsonUnionRequiresExactlyOneKnownMember) {
  type_system::TypeSystemBuilder builder;
  builder.addType(
      "test.Choice",
      def::Union({
          def::Field(def::Identity(1, "id"), def::Optional, TypeIds::I32),
          def::Field(def::Identity(2, "name"), def::Optional, TypeIds::String),
      }));
  auto ts = std::move(builder).build();
  const auto& node = ts->getUserDefinedTypeOrThrow("test.Choice").asUnion();

  auto compact = makeThriftCompactCodec(node);
  auto json = makeJsonCodec(node);
  TranscodeInterpreter jsonToCompact{fuse(json, compact)};

  auto compact0 = jsonToCompact.transcode(wrap(R"({"id":7})"));
  ASSERT_FALSE(compact0.hasError()) << compact0.error().message;

  EXPECT_TRUE(jsonToCompact.transcode(wrap("{}")).hasError());
  EXPECT_TRUE(
      jsonToCompact.transcode(wrap(R"({"id":7,"name":"x"})")).hasError());
}

TEST_F(InterpreterJsonReadTest, JsonMapSourceTranscodesWithOptIn) {
  type_system::TypeSystemBuilder builder;
  builder.addType(
      "test.WithMap",
      def::Struct({
          def::Field(
              def::Identity(1, "m"),
              def::AlwaysPresent,
              TypeIds::map(TypeIds::String, TypeIds::I32)),
      }));
  auto ts = std::move(builder).build();
  const auto& node = ts->getUserDefinedTypeOrThrow("test.WithMap").asStruct();

  auto json = makeJsonCodec(node);
  auto compact = makeThriftCompactCodec(node);

  auto transcoder = makeTranscoder(
      fuse(json, compact), Engine::Interpreter, allowExperimentalProtocols());
  ASSERT_FALSE(transcoder.hasError()) << transcoder.error().message;

  auto output = (*transcoder)->transcode(wrap(R"({"m":{"one":1,"two":2}})"));
  EXPECT_FALSE(output.hasError()) << output.error().message;
}

TEST_F(InterpreterJsonReadTest, JsonMapKeyValueArrayAcceptsValueBeforeKey) {
  type_system::TypeSystemBuilder builder;
  builder.addType(
      "test.WithMap",
      def::Struct({
          def::Field(
              def::Identity(1, "m"),
              def::AlwaysPresent,
              TypeIds::map(TypeIds::I32, TypeIds::String)),
      }));
  auto ts = std::move(builder).build();
  const auto& node = ts->getUserDefinedTypeOrThrow("test.WithMap").asStruct();

  auto json = makeJsonCodec(node);
  auto compact = makeThriftCompactCodec(node);
  TranscodeInterpreter jsonToCompact{fuse(json, compact)};

  auto expected = jsonToCompact.transcode(
      wrap(R"({"m":[{"key":1,"value":"one"},{"key":2,"value":"two"}]})"));
  ASSERT_FALSE(expected.hasError()) << expected.error().message;

  auto actual = jsonToCompact.transcode(
      wrap(R"({"m":[{"value":"one","key":1},{"value":"two","key":2}]})"));
  ASSERT_FALSE(actual.hasError()) << actual.error().message;
  EXPECT_EQ(toBytes(**actual), toBytes(**expected));
}

TEST_F(
    InterpreterJsonReadTest,
    JsonMapKeyValueArrayUsesLastKeyValueAndSkipsUnknownMembers) {
  type_system::TypeSystemBuilder builder;
  builder.addType(
      "test.WithMap",
      def::Struct({
          def::Field(
              def::Identity(1, "m"),
              def::AlwaysPresent,
              TypeIds::map(TypeIds::I32, TypeIds::String)),
      }));
  auto ts = std::move(builder).build();
  const auto& node = ts->getUserDefinedTypeOrThrow("test.WithMap").asStruct();

  auto json = makeJsonCodec(node);
  auto compact = makeThriftCompactCodec(node);
  TranscodeInterpreter jsonToCompact{fuse(json, compact)};

  auto expected =
      jsonToCompact.transcode(wrap(R"({"m":[{"key":2,"value":"two"}]})"));
  ASSERT_FALSE(expected.hasError()) << expected.error().message;

  auto actual = jsonToCompact.transcode(wrap(
      R"({"m":[{"extra":{"ignored":[1,2]},"key":1,"value":"one","key":2,"value":"two"}]})"));
  ASSERT_FALSE(actual.hasError()) << actual.error().message;
  EXPECT_EQ(toBytes(**actual), toBytes(**expected));
}

TEST_F(InterpreterJsonReadTest, JsonEnumMapObjectKeyAcceptsNameOrId) {
  type_system::TypeSystemBuilder builder;
  builder.addType("test.Color", def::Enum({{"RED", 1}, {"BLUE", 2}}));
  builder.addType(
      "test.WithMap",
      def::Struct({
          def::Field(
              def::Identity(1, "m"),
              def::AlwaysPresent,
              TypeIds::map(TypeIds::uri("test.Color"), TypeIds::I32)),
      }));
  auto ts = std::move(builder).build();
  const auto& node = ts->getUserDefinedTypeOrThrow("test.WithMap").asStruct();

  auto json = makeJsonCodec(node);
  auto compact = makeThriftCompactCodec(node);
  TranscodeInterpreter jsonToCompact{fuse(json, compact)};

  EXPECT_FALSE(jsonToCompact.transcode(wrap(R"({"m":{"RED":7}})")).hasError());
  EXPECT_FALSE(jsonToCompact.transcode(wrap(R"({"m":{"1":7}})")).hasError());
  EXPECT_TRUE(jsonToCompact.transcode(wrap(R"JSON({"m":{"RED(1)":7}})JSON"))
                  .hasError());
  EXPECT_TRUE(jsonToCompact.transcode(wrap(R"JSON({"m":{"RED (1)":7}})JSON"))
                  .hasError());
}

TEST_F(InterpreterJsonReadTest, JsonEnumValueAcceptsNameOrId) {
  type_system::TypeSystemBuilder builder;
  builder.addType("test.Color", def::Enum({{"RED", 1}, {"BLUE", 2}}));
  builder.addType(
      "test.WithEnum",
      def::Struct({
          def::Field(
              def::Identity(1, "color"),
              def::AlwaysPresent,
              TypeIds::uri("test.Color")),
      }));
  auto ts = std::move(builder).build();
  const auto& node = ts->getUserDefinedTypeOrThrow("test.WithEnum").asStruct();

  auto json = makeJsonCodec(node);
  auto compact = makeThriftCompactCodec(node);
  TranscodeInterpreter jsonToCompact{fuse(json, compact)};

  auto byName = jsonToCompact.transcode(wrap(R"({"color":"RED"})"));
  ASSERT_FALSE(byName.hasError()) << byName.error().message;

  auto byNumber = jsonToCompact.transcode(wrap(R"({"color":1})"));
  ASSERT_FALSE(byNumber.hasError()) << byNumber.error().message;
  EXPECT_EQ(toBytes(**byName), toBytes(**byNumber));

  EXPECT_TRUE(jsonToCompact.transcode(wrap(R"JSON({"color":"RED(1)"})JSON"))
                  .hasError());
  EXPECT_TRUE(jsonToCompact.transcode(wrap(R"({"color":"1"})")).hasError());
}

TEST_F(InterpreterJsonReadTest, JsonMapTargetWritesSpecForms) {
  type_system::TypeSystemBuilder builder;
  builder.addType("test.Color", def::Enum({{"RED", 1}, {"BLUE", 2}}));
  builder.addType(
      "test.WithMaps",
      def::Struct({
          def::Field(
              def::Identity(1, "string_map"),
              def::AlwaysPresent,
              TypeIds::map(TypeIds::String, TypeIds::I32)),
          def::Field(
              def::Identity(2, "int_map"),
              def::AlwaysPresent,
              TypeIds::map(TypeIds::I32, TypeIds::String)),
          def::Field(
              def::Identity(3, "enum_map"),
              def::AlwaysPresent,
              TypeIds::map(TypeIds::uri("test.Color"), TypeIds::String)),
      }));
  auto ts = std::move(builder).build();
  const auto& node = ts->getUserDefinedTypeOrThrow("test.WithMaps").asStruct();

  auto compact = makeThriftCompactCodec(node);
  auto json = makeJsonCodec(node);
  TranscodeInterpreter encoder{fuse(json, compact)};

  auto compactInput = encoder.transcode(wrap(
      R"({"string_map":{"one":1},"int_map":[{"value":"two","key":2}],"enum_map":{"1":"red","BLUE":"blue","99":"unknown"}})"));
  ASSERT_FALSE(compactInput.hasError()) << compactInput.error().message;

  auto transcoder = makeTranscoder(
      fuse(compact, json), Engine::Interpreter, allowExperimentalProtocols());
  ASSERT_FALSE(transcoder.hasError()) << transcoder.error().message;

  auto output = (*transcoder)->transcode(**compactInput);
  ASSERT_FALSE(output.hasError()) << output.error().message;
  EXPECT_EQ(
      toStr(**output),
      R"({"string_map":{"one":1},"int_map":[{"key":2,"value":"two"}],"enum_map":{"RED":"red","BLUE":"blue","99":"unknown"}})");
}

TEST_F(InterpreterJsonReadTest, PackedProtobufSequenceWritesJsonArray) {
  type_system::TypeSystemBuilder builder;
  builder.addType(
      "test.WithList",
      def::Struct({
          def::Field(
              def::Identity(1, "nums"),
              def::AlwaysPresent,
              TypeIds::list(TypeIds::I32)),
      }));
  auto ts = std::move(builder).build();
  const auto& node = ts->getUserDefinedTypeOrThrow("test.WithList").asStruct();

  auto protobuf = makeProtobufBinaryCodec(node);
  auto json = makeJsonCodec(node);
  auto transcoder = makeTranscoder(
      fuse(protobuf, json), Engine::Interpreter, allowExperimentalProtocols());
  ASSERT_FALSE(transcoder.hasError()) << transcoder.error().message;

  const std::vector<uint8_t> input = {
      0x0A, // field 1, length-delimited
      0x03, // packed payload length
      0x14, // zigzag i32 10
      0x28, // zigzag i32 20
      0x3C, // zigzag i32 30
  };
  auto inputBuf = folly::IOBuf::wrapBufferAsValue(input.data(), input.size());

  auto output = (*transcoder)->transcode(inputBuf);
  ASSERT_FALSE(output.hasError()) << output.error().message;
  EXPECT_EQ(toStr(**output), R"({"nums":[10,20,30]})");
}

} // namespace
} // namespace apache::thrift::transcode
