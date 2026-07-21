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

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/protocol/Json5Protocol.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/runtime/SchemaRegistry.h>
#include <thrift/lib/cpp2/transcode/test/GoldenTestData.h>
#include <thrift/lib/cpp2/transcode/test/gen-cpp2/GoldenFixtures_types.h>
#include <thrift/lib/cpp2/transcode/test/gen-cpp2/GoldenFixtures_types_custom_protocol.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace apache::thrift::transcode {
namespace {

namespace fixture = apache::thrift::transcode::test;
namespace test_data = apache::thrift::transcode::golden_test;

template <typename T>
const type_system::StructNode& structNode() {
  return SchemaRegistry::get().getTypeSystemNode<T>();
}

TranscodePlan fuseOrThrow(const Codec& source, const Codec& target) {
  auto fused = fuseStructOps(
      std::get<StructOp>(source.root), std::get<StructOp>(target.root));
  if (fused.hasError()) {
    throw std::runtime_error(fused.error().message);
  }
  TranscodePlan plan{"golden_transcode", std::move(*fused)};
  plan.sourceProtocol = source.protocol;
  plan.targetProtocol = target.protocol;
  return plan;
}

folly::IOBuf wrap(const std::string& bytes) {
  return folly::IOBuf::wrapBufferAsValue(bytes.data(), bytes.size());
}

std::string toString(const folly::IOBuf& buf) {
  auto coalesced = buf.cloneCoalescedAsValue();
  return std::string(
      reinterpret_cast<const char*>(coalesced.data()), coalesced.length());
}

using TranscodeFn =
    std::function<std::optional<std::string>(const std::string&)>;

TranscodeFn makeWireTranscoder(
    std::string_view name, const Codec& source, const Codec& target) {
  auto interpreter =
      std::make_shared<TranscodeInterpreter>(fuseOrThrow(source, target));
  return [interpreter,
          name](const std::string& input) -> std::optional<std::string> {
    auto inputBuf = wrap(input);
    auto output = interpreter->transcode(inputBuf);
    if (output.hasError()) {
      ADD_FAILURE() << name << ": " << output.error().message;
      return std::nullopt;
    }
    return toString(**output);
  };
}

template <typename T>
struct MaterializedCodec {
  std::string_view name;
  std::function<std::string(const T&)> serialize;
  std::function<T(const std::string&)> deserialize;
};

template <typename T, typename Serializer>
MaterializedCodec<T> makeMaterializedCodec(std::string_view name) {
  return {
      name,
      [](const T& value) {
        return Serializer::template serialize<std::string>(value);
      },
      [](const std::string& bytes) {
        T value;
        auto consumed = Serializer::deserialize(bytes, value);
        EXPECT_EQ(static_cast<std::size_t>(consumed), bytes.size());
        return value;
      }};
}

template <typename T>
MaterializedCodec<T> makeJson5BasicMaterializedCodec() {
  json5::detail::Json5ProtocolWriter::Options options;
  options.enumAsInteger = true;
  options.binaryAsBase64String = true;
  return {
      "json5-basic",
      [options](const T& value) {
        return json5::detail::toJsonImpl<type::infer_tag<T>>(value, options);
      },
      [](const std::string& bytes) {
        return Json5ProtocolUtils::fromJson5<T>(bytes);
      }};
}

template <typename T>
MaterializedCodec<test_data::PreEncodedJson<T>>
makePreEncodedJsonMaterializedCodec() {
  using PreEncodedJson = test_data::PreEncodedJson<T>;
  return {
      "pre-encoded-json",
      [](const PreEncodedJson& value) { return value.bytes; },
      [](const std::string& bytes) {
        return PreEncodedJson{.bytes = bytes, .value = {}};
      }};
}

template <typename Input, typename Output>
struct TranscodePipeline {
  std::string_view name;
  MaterializedCodec<Input> inputCodec;
  TranscodeFn transcode;
  MaterializedCodec<Output> outputCodec;
  std::function<void(const Output&, const Input&)> expectOutput;

  void operator()(std::string_view caseName, const Input& input) const {
    SCOPED_TRACE(
        ::testing::Message() << name << " / " << inputCodec.name << " -> "
                             << outputCodec.name << " / " << caseName);
    auto inputBytes = inputCodec.serialize(input);
    auto outputBytes = transcode(inputBytes);
    ASSERT_TRUE(outputBytes.has_value());
    auto output = outputCodec.deserialize(*outputBytes);
    expectOutput(output, input);
  }
};

template <typename T>
std::function<void(const T&, const T&)> expectEqual() {
  return [](const T& output, const T& input) { EXPECT_EQ(output, input); };
}

enum class PipelineSupport : uint8_t {
  CompactInput = 1 << 0,
  CompactOutput = 1 << 1,
  BinaryInput = 1 << 2,
  BinaryOutput = 1 << 3,
  JsonInput = 1 << 4,
  JsonOutput = 1 << 5,
};

using PipelineSupportMask = uint8_t;

constexpr PipelineSupportMask supportBit(PipelineSupport support) {
  return static_cast<PipelineSupportMask>(support);
}

constexpr PipelineSupportMask kAllPipelineSupport =
    supportBit(PipelineSupport::CompactInput) |
    supportBit(PipelineSupport::CompactOutput) |
    supportBit(PipelineSupport::BinaryInput) |
    supportBit(PipelineSupport::BinaryOutput) |
    supportBit(PipelineSupport::JsonInput) |
    supportBit(PipelineSupport::JsonOutput);

constexpr bool supports(PipelineSupportMask mask, PipelineSupport support) {
  return (mask & supportBit(support)) != 0;
}

constexpr PipelineSupportMask without(
    PipelineSupportMask mask, PipelineSupport support) {
  return mask & ~supportBit(support);
}

constexpr PipelineSupportMask kJsonInputOutputUnsupported = without(
    without(kAllPipelineSupport, PipelineSupport::JsonInput),
    PipelineSupport::JsonOutput);

constexpr PipelineSupportMask kJsonOutputUnsupported =
    without(kAllPipelineSupport, PipelineSupport::JsonOutput);

template <typename T>
std::vector<TranscodePipeline<T, T>> makeProtocolPipelines(
    const type_system::StructNode& node,
    PipelineSupportMask support = kAllPipelineSupport,
    std::function<void(const T&, const T&)> expectOutput = expectEqual<T>()) {
  auto compactWire = makeThriftCompactCodec(node);
  auto binaryWire = makeThriftBinaryCodec(node);
  auto jsonWire = makeJsonCodec(node);
  auto compactMaterialized =
      makeMaterializedCodec<T, CompactSerializer>("compact");
  auto binaryMaterialized =
      makeMaterializedCodec<T, BinarySerializer>("binary");
  auto jsonMaterialized = makeJson5BasicMaterializedCodec<T>();

  std::vector<TranscodePipeline<T, T>> pipelines;
  auto addPipeline = [&](std::string_view name,
                         PipelineSupport sourceSupport,
                         PipelineSupport targetSupport,
                         const MaterializedCodec<T>& inputCodec,
                         const Codec& sourceWire,
                         const Codec& targetWire,
                         const MaterializedCodec<T>& outputCodec) {
    if (!supports(support, sourceSupport) ||
        !supports(support, targetSupport)) {
      return;
    }
    pipelines.push_back({
        name,
        inputCodec,
        makeWireTranscoder(name, sourceWire, targetWire),
        outputCodec,
        expectOutput,
    });
  };

  addPipeline(
      "compact->binary",
      PipelineSupport::CompactInput,
      PipelineSupport::BinaryOutput,
      compactMaterialized,
      compactWire,
      binaryWire,
      binaryMaterialized);
  addPipeline(
      "binary->compact",
      PipelineSupport::BinaryInput,
      PipelineSupport::CompactOutput,
      binaryMaterialized,
      binaryWire,
      compactWire,
      compactMaterialized);
  addPipeline(
      "json->compact",
      PipelineSupport::JsonInput,
      PipelineSupport::CompactOutput,
      jsonMaterialized,
      jsonWire,
      compactWire,
      compactMaterialized);
  addPipeline(
      "compact->json",
      PipelineSupport::CompactInput,
      PipelineSupport::JsonOutput,
      compactMaterialized,
      compactWire,
      jsonWire,
      jsonMaterialized);
  addPipeline(
      "json->binary",
      PipelineSupport::JsonInput,
      PipelineSupport::BinaryOutput,
      jsonMaterialized,
      jsonWire,
      binaryWire,
      binaryMaterialized);
  addPipeline(
      "binary->json",
      PipelineSupport::BinaryInput,
      PipelineSupport::JsonOutput,
      binaryMaterialized,
      binaryWire,
      jsonWire,
      jsonMaterialized);

  return pipelines;
}

template <typename T>
std::vector<TranscodePipeline<test_data::PreEncodedJson<T>, T>>
makePreEncodedJsonSourcePipelines(
    const type_system::StructNode& node,
    const std::function<void(const T&, const T&)>& expectOutput =
        expectEqual<T>()) {
  using PreEncodedJson = test_data::PreEncodedJson<T>;
  auto jsonWire = makeJsonCodec(node);
  auto compactWire = makeThriftCompactCodec(node);
  auto binaryWire = makeThriftBinaryCodec(node);
  auto jsonMaterialized = makePreEncodedJsonMaterializedCodec<T>();
  auto compactMaterialized =
      makeMaterializedCodec<T, CompactSerializer>("compact");
  auto binaryMaterialized =
      makeMaterializedCodec<T, BinarySerializer>("binary");
  auto expectPreEncodedOutput =
      [expectOutput](const T& output, const PreEncodedJson& input) {
        expectOutput(output, input.value);
      };

  std::vector<TranscodePipeline<PreEncodedJson, T>> pipelines;
  auto addPipeline = [&](std::string_view name,
                         const Codec& targetWire,
                         const MaterializedCodec<T>& outputCodec) {
    pipelines.push_back({
        name,
        jsonMaterialized,
        makeWireTranscoder(name, jsonWire, targetWire),
        outputCodec,
        expectPreEncodedOutput,
    });
  };

  addPipeline("json->compact", compactWire, compactMaterialized);
  addPipeline("json->binary", binaryWire, binaryMaterialized);
  return pipelines;
}

template <typename T>
struct GoldenCase {
  std::string_view name;
  T value;
};

template <typename T>
class GoldenCases {
 public:
  template <typename... Args>
  GoldenCases&& generated(std::string_view name, Args&&... args) && {
    cases_.push_back(
        {name, test_data::generate<T>(std::forward<Args>(args)...)});
    return std::move(*this);
  }

  GoldenCases&& caseValue(std::string_view name, T value) && {
    cases_.push_back({name, std::move(value)});
    return std::move(*this);
  }

  std::vector<GoldenCase<T>> take() && { return std::move(cases_); }

 private:
  std::vector<GoldenCase<T>> cases_;
};

template <typename T>
GoldenCases<T> cases() {
  return {};
}

template <typename T, typename Output>
void runCases(
    const std::vector<GoldenCase<T>>& cases,
    const std::vector<TranscodePipeline<T, Output>>& pipelines) {
  for (const auto& pipeline : pipelines) {
    for (const auto& testCase : cases) {
      pipeline(testCase.name, testCase.value);
    }
  }
}

template <typename T, typename ExpectOutput>
void runProtocolCases(
    GoldenCases<T> cases,
    PipelineSupportMask support,
    ExpectOutput expectOutput) {
  auto ownedCases = std::move(cases).take();
  runCases(
      ownedCases,
      makeProtocolPipelines<T>(
          structNode<T>(),
          support,
          std::function<void(const T&, const T&)>(std::move(expectOutput))));
}

template <typename T>
void runProtocolCases(
    GoldenCases<T> cases, PipelineSupportMask support = kAllPipelineSupport) {
  runProtocolCases(std::move(cases), support, expectEqual<T>());
}

template <typename T, typename ExpectOutput>
void runProtocolCases(GoldenCases<T> cases, ExpectOutput expectOutput) {
  runProtocolCases(
      std::move(cases), kAllPipelineSupport, std::move(expectOutput));
}

template <typename T, typename ExpectOutput>
void runPreEncodedJsonSourceCases(
    GoldenCases<test_data::PreEncodedJson<T>> cases,
    ExpectOutput expectOutput) {
  auto ownedCases = std::move(cases).take();
  runCases(
      ownedCases,
      makePreEncodedJsonSourcePipelines<T>(
          structNode<T>(),
          std::function<void(const T&, const T&)>(std::move(expectOutput))));
}

template <typename T>
void runPreEncodedJsonSourceCases(
    GoldenCases<test_data::PreEncodedJson<T>> cases) {
  runPreEncodedJsonSourceCases(std::move(cases), expectEqual<T>());
}

void expectJsonSourceRejected(
    std::string_view caseName,
    const type_system::StructNode& node,
    std::string_view jsonInput) {
  SCOPED_TRACE(::testing::Message() << "json reject / " << caseName);
  auto jsonWire = makeJsonCodec(node);
  auto compactWire = makeThriftCompactCodec(node);
  TranscodeInterpreter interpreter{fuseOrThrow(jsonWire, compactWire)};

  std::string input{jsonInput};
  auto result = interpreter.transcode(wrap(input));
  EXPECT_TRUE(result.hasError());
}

GoldenCases<test_data::PreEncodedJson<fixture::MapShapes>>
preEncodedJsonMapCases() {
  using PreEncodedMap = test_data::PreEncodedJson<fixture::MapShapes>;

  fixture::MapShapes objectFormValue;
  objectFormValue.empty_string_map() = {};
  objectFormValue.string_map() = {{"", 0}, {"minus", -1}, {"one", 1}};
  objectFormValue.int_map() = {};
  objectFormValue.enum_map() = {
      {fixture::GoldenEnum::Unknown, "unknown"},
      {fixture::GoldenEnum::Active, "active"},
  };

  fixture::MapShapes enumNameObjectFormValue;
  enumNameObjectFormValue.empty_string_map() = {};
  enumNameObjectFormValue.string_map() = {};
  enumNameObjectFormValue.int_map() = {};
  enumNameObjectFormValue.enum_map() = {
      {fixture::GoldenEnum::Unknown, "unknown"},
      {fixture::GoldenEnum::Active, "active"},
  };

  fixture::MapShapes keyValueArrayValue;
  keyValueArrayValue.empty_string_map() = {};
  keyValueArrayValue.string_map() = {};
  keyValueArrayValue.int_map() = {{-1, "minus"}, {0, "zero"}, {42, "answer"}};
  keyValueArrayValue.enum_map() = {};

  return cases<PreEncodedMap>()
      .caseValue(
          "object-form-string-and-enum-keys",
          PreEncodedMap{
              .bytes =
                  R"json({"empty_string_map":{},"string_map":{"":0,"minus":-1,"one":1},"int_map":[],"enum_map":{"0":"unknown","1":"active"}})json",
              .value = std::move(objectFormValue),
          })
      .caseValue(
          "object-form-enum-name-keys",
          PreEncodedMap{
              .bytes =
                  R"json({"empty_string_map":{},"string_map":{},"int_map":[],"enum_map":{"Unknown":"unknown","Active":"active"}})json",
              .value = std::move(enumNameObjectFormValue),
          })
      .caseValue(
          "key-value-array-i32-keys",
          PreEncodedMap{
              .bytes =
                  R"json({"empty_string_map":{},"string_map":{},"int_map":[{"key":-1,"value":"minus"},{"value":"zero","key":0},{"key":42,"value":"answer"}],"enum_map":{}})json",
              .value = std::move(keyValueArrayValue),
          });
}

TEST(GoldenTranscodeTest, ScalarShapesRoundTripSupportedProtocols) {
  runProtocolCases(
      cases<fixture::ScalarShapes>().generated("scalar-boundaries"),
      test_data::expectScalarShapes);
}

TEST(GoldenTranscodeTest, NonUtf8StringShapesPassThroughSupportedProtocols) {
  runProtocolCases(
      cases<fixture::NonUtf8StringShapes>().generated("non-utf8-string"),
      kJsonInputOutputUnsupported);

  using NonUtf8Json = test_data::PreEncodedJson<fixture::NonUtf8StringShapes>;
  runPreEncodedJsonSourceCases(
      cases<NonUtf8Json>().generated("pre-encoded-json-non-utf8-string"));
}

TEST(GoldenTranscodeTest, SpecialFloatShapesRoundTripSupportedSources) {
  runProtocolCases(
      cases<fixture::SpecialFloatShapes>().generated("nan-and-infinity"),
      kJsonOutputUnsupported,
      test_data::expectSpecialFloatShapes);
}

TEST(GoldenTranscodeTest, NegativeFieldIdShapesRoundTripSupportedProtocols) {
  runProtocolCases(
      cases<fixture::NegativeFieldIdShapes>().generated("negative-field-ids"));
}

TEST(GoldenTranscodeTest, PresenceShapesRoundTripSupportedProtocols) {
  runProtocolCases(
      cases<fixture::PresenceShapes>()
          .generated("optional-present", true)
          .generated("optional-absent", false));
}

TEST(GoldenTranscodeTest, SequenceShapesRoundTripSupportedProtocols) {
  runProtocolCases(
      cases<fixture::SequenceShapes>().generated("sequence-shapes"),
      kJsonOutputUnsupported);
}

TEST(GoldenTranscodeTest, MapShapesRoundTripSupportedSources) {
  runProtocolCases(
      cases<fixture::MapShapes>().generated("map-shapes"),
      kJsonOutputUnsupported);
  runPreEncodedJsonSourceCases(preEncodedJsonMapCases());
}

TEST(GoldenTranscodeTest, NestedShapesRoundTripSupportedProtocols) {
  runProtocolCases(
      cases<fixture::NestedShapes>().generated("nested-shapes"),
      kJsonOutputUnsupported);
}

TEST(GoldenTranscodeTest, UnionShapesRoundTripSupportedProtocols) {
  runProtocolCases(cases<fixture::UnionShapes>().generated("union-shapes"));
}

TEST(GoldenTranscodeTest, ExceptionShapesRoundTripSupportedProtocols) {
  runProtocolCases(
      cases<fixture::ExceptionShapes>().generated("exception-shapes"));
}

TEST(GoldenTranscodeTest, GeneratedFixtureRoundTripSupportedSources) {
  runProtocolCases(
      cases<fixture::GoldenStruct>()
          .generated("optional-present", true)
          .generated("optional-absent", false),
      kJsonOutputUnsupported);
}

TEST(GoldenTranscodeTest, JsonRejectsDuplicateKnownFields) {
  expectJsonSourceRejected(
      "duplicate-required-scalar",
      structNode<fixture::PresenceShapes>(),
      R"({"unqualified_i32":1,"unqualified_i32":2})");
  expectJsonSourceRejected(
      "duplicate-optional-null",
      structNode<fixture::PresenceShapes>(),
      R"({"maybe_i32":null,"maybe_i32":1})");
  expectJsonSourceRejected(
      "duplicate-nested-field",
      structNode<fixture::NestedShapes>(),
      R"({"inner":{"n":1,"n":2,"label":"x"},"matrix":[],"inner_groups":[]})");
}

TEST(GoldenTranscodeTest, DISABLED_JsonMapTargetsFollowSpecObjectAndPairForms) {
  GTEST_SKIP() << "TODO: JSON map targets need string/enum key object form and "
                  "non-string key array-of-key-value form.";
}

TEST(GoldenTranscodeTest, DISABLED_JsonRejectsDuplicateSetAndMapEntries) {
  GTEST_SKIP()
      << "TODO: JSON set/map reads need duplicate element and key rejection.";
}

TEST(GoldenTranscodeTest, DISABLED_JsonRejectsUnknownFieldsByDefault) {
  GTEST_SKIP() << "TODO: strict JSON mode should reject unknown field names.";
}

TEST(
    GoldenTranscodeTest, DISABLED_NonThriftEndpointsRejectRequiredAndDefaults) {
  GTEST_SKIP()
      << "TODO: non-Thrift endpoints need rejection coverage for required, "
         "custom-default, and terse fields.";
}

TEST(GoldenTranscodeTest, DISABLED_JsonSpecialFloatStringsRoundTrip) {
  GTEST_SKIP()
      << "TODO: JSON float writing needs NaN and infinity string coverage.";
}

TEST(GoldenTranscodeTest, DISABLED_ProtobufGoldenRoundTripSupportedProtocols) {
  GTEST_SKIP()
      << "TODO: enable Protobuf golden coverage once Protobuf protocol support "
         "is no longer gated.";
}

TEST(GoldenTranscodeTest, DISABLED_HeterogeneousDynamicMaterializedRoundTrip) {
  GTEST_SKIP()
      << "TODO: add materialized folly::dynamic and DynamicValue output codecs "
         "with custom equality.";
}

} // namespace
} // namespace apache::thrift::transcode
