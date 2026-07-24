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

#include <thrift/lib/cpp2/transcode/test/TranscodeBenchmarkData.h>

#include <thrift/lib/cpp2/dynamic/Serialization.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/runtime/SchemaRegistry.h>
#include <thrift/lib/cpp2/transcode/CodecFactory.h>
#include <thrift/lib/cpp2/transcode/test/gen-cpp2/GoldenFixtures_types.h>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>

#include <array>
#include <cstddef>
#include <cstdint>
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

enum class ProtocolKey : std::size_t {
  Compact,
  Binary,
  Json,
  Count,
};

constexpr auto kCompact = ProtocolKey::Compact;
constexpr auto kBinary = ProtocolKey::Binary;
constexpr auto kJson = ProtocolKey::Json;
constexpr auto kProtocolCount = static_cast<std::size_t>(ProtocolKey::Count);
constexpr std::size_t kOutputBufferSize = 1024 * 1024;

constexpr std::size_t protocolIndex(ProtocolKey protocol) {
  return static_cast<std::size_t>(protocol);
}

constexpr std::size_t pathIndex(ProtocolKey source, ProtocolKey target) {
  return protocolIndex(source) * kProtocolCount + protocolIndex(target);
}

using CodecFactory = Codec (*)(const type_system::StructNode&);

struct ProtocolSpec {
  ProtocolKey key;
  CodecFactory makeCodec;
};

const std::array<ProtocolSpec, kProtocolCount> kProtocolSpecs{{
    {kCompact, makeThriftCompactCodec},
    {kBinary, makeThriftBinaryCodec},
    {kJson, makeJsonCodec},
}};

struct ProtocolPair {
  std::string_view name;
  ProtocolKey source;
  ProtocolKey target;
  void (*benchCpp2Roundtrip)(std::size_t);
  void (*benchDynamicValueRoundtrip)(std::size_t);
};

const std::array<ProtocolPair, 4>& protocolPairs();

TranscoderOptions allowExperimentalProtocols() {
  TranscoderOptions options;
  options.unsupportedPlanPolicy =
      UnsupportedPlanPolicy::AllowExperimentalProtocols;
  return options;
}

TranscodePlan fuseOrThrow(const Codec& source, const Codec& target) {
  auto plan = fuseCodecs(source, target);
  if (plan.hasError()) {
    throw std::runtime_error(plan.error().message);
  }
  return std::move(*plan);
}

std::unique_ptr<ITranscoder> makeInterpreter(
    const Codec& source, const Codec& target) {
  auto transcoder = makeTranscoder(
      fuseOrThrow(source, target),
      Engine::Interpreter,
      allowExperimentalProtocols());
  if (transcoder.hasError()) {
    throw std::runtime_error(transcoder.error().message);
  }
  return std::move(*transcoder);
}

template <typename T>
std::unique_ptr<folly::IOBuf> serializeInput(
    ProtocolKey protocol, const T& value) {
  switch (protocol) {
    case kCompact:
      return folly::IOBuf::copyBuffer(
          CompactSerializer::serialize<std::string>(value));
    case kBinary:
      return folly::IOBuf::copyBuffer(
          BinarySerializer::serialize<std::string>(value));
    case kJson:
      return folly::IOBuf::copyBuffer(
          SimpleJSONSerializer::serialize<std::string>(value));
    case ProtocolKey::Count:
      break;
  }
  throw std::runtime_error("unknown input protocol");
}

template <typename T>
struct BenchFixture {
  explicit BenchFixture(T value)
      : type_(SchemaRegistry::get().getTypeSystemNode<T>().asRef()) {
    const auto& node = type_.asStruct();
    for (const auto& spec : kProtocolSpecs) {
      codecs_[protocolIndex(spec.key)] = spec.makeCodec(node);
      inputs_[protocolIndex(spec.key)] = serializeInput(spec.key, value);
    }
    for (const auto& pair : protocolPairs()) {
      paths_[pathIndex(pair.source, pair.target)] =
          makeInterpreter(codec(pair.source), codec(pair.target));
    }
  }

  type_system::TypeRef type() const { return type_; }

  const folly::IOBuf& input(ProtocolKey protocol) const {
    const auto& value = inputs_[protocolIndex(protocol)];
    if (!value) {
      throw std::runtime_error("missing benchmark input");
    }
    return *value;
  }

  const ITranscoder& transcoder(ProtocolKey source, ProtocolKey target) const {
    const auto& value = paths_[pathIndex(source, target)];
    if (!value) {
      throw std::runtime_error("missing benchmark transcode path");
    }
    return *value;
  }

 private:
  const Codec& codec(ProtocolKey protocol) const {
    const auto& value = codecs_[protocolIndex(protocol)];
    if (!value) {
      throw std::runtime_error("missing benchmark codec");
    }
    return *value;
  }

  type_system::TypeRef type_;
  std::array<std::optional<Codec>, kProtocolCount> codecs_;
  std::array<std::unique_ptr<folly::IOBuf>, kProtocolCount> inputs_;
  std::array<std::unique_ptr<ITranscoder>, kProtocolCount * kProtocolCount>
      paths_;
};

BenchFixture<fixture::GoldenStruct>& goldenFixture() {
  static BenchFixture<fixture::GoldenStruct> fixture{
      bench_data::goldenStruct()};
  return fixture;
}

void reserveOutput(folly::IOBufQueue& queue, std::size_t size) {
  auto [data, capacity] = queue.preallocate(size, size, size);
  if (data == nullptr || capacity < size) {
    throw std::runtime_error("failed to preallocate benchmark output buffer");
  }
}

void observeOutput(const folly::IOBufQueue& queue) {
  auto length = queue.chainLength();
  const auto* output = queue.front();
  folly::doNotOptimizeAway(length);
  folly::doNotOptimizeAway(output);
}

void benchTranscode(std::size_t iters, ProtocolKey source, ProtocolKey target) {
  const auto& fixture = goldenFixture();
  const auto& transcoder = fixture.transcoder(source, target);
  const auto& input = fixture.input(source);
  std::vector<std::uint8_t> output(kOutputBufferSize);
  while (iters-- > 0) {
    auto written =
        transcoder.transcodeInto(input, output.data(), output.size());
    if (written.hasError()) {
      throw std::runtime_error(written.error().message);
    }
    folly::doNotOptimizeAway(*written);
    folly::doNotOptimizeAway(output.data());
  }
}

template <typename SourceSerializer, typename TargetSerializer>
void benchStructRoundtrip(std::size_t iters, ProtocolKey source) {
  const auto& fixture = goldenFixture();
  const auto& input = fixture.input(source);
  folly::IOBufQueue output(folly::IOBufQueue::cacheChainLength());
  reserveOutput(output, kOutputBufferSize);
  while (iters-- > 0) {
    auto value =
        SourceSerializer::template deserialize<fixture::GoldenStruct>(&input);
    TargetSerializer::serialize(value, &output);
    observeOutput(output);
    output.clearAndTryReuseLargestBuffer();
  }
}

template <
    typename SourceSerializer,
    typename TargetSerializer,
    ProtocolKey Source>
void benchPairRoundtrip(std::size_t iters) {
  benchStructRoundtrip<SourceSerializer, TargetSerializer>(iters, Source);
}

template <typename SourceReader, typename TargetWriter>
void benchDynamicValueRoundtrip(std::size_t iters, ProtocolKey source) {
  const auto& fixture = goldenFixture();
  const auto& input = fixture.input(source);
  const auto type = fixture.type();
  folly::IOBufQueue output(folly::IOBufQueue::cacheChainLength());
  reserveOutput(output, kOutputBufferSize);
  while (iters-- > 0) {
    SourceReader reader;
    reader.setInput(&input);
    auto value = dynamic::deserializeValue(reader, type, nullptr);

    {
      TargetWriter writer;
      writer.setOutput(&output, kOutputBufferSize);
      dynamic::serializeValue(writer, value);
    }
    observeOutput(output);
    output.clearAndTryReuseLargestBuffer();
  }
}

template <typename SourceReader, typename TargetWriter, ProtocolKey Source>
void benchPairDynamicValueRoundtrip(std::size_t iters) {
  benchDynamicValueRoundtrip<SourceReader, TargetWriter>(iters, Source);
}

const std::array<ProtocolPair, 4> kProtocolPairs{{
    {"Golden_JsonToCompact",
     kJson,
     kCompact,
     benchPairRoundtrip<SimpleJSONSerializer, CompactSerializer, kJson>,
     benchPairDynamicValueRoundtrip<
         SimpleJSONProtocolReader,
         CompactProtocolWriter,
         kJson>},
    {"Golden_CompactToJson",
     kCompact,
     kJson,
     benchPairRoundtrip<CompactSerializer, SimpleJSONSerializer, kCompact>,
     benchPairDynamicValueRoundtrip<
         CompactProtocolReader,
         SimpleJSONProtocolWriter,
         kCompact>},
    {"Golden_JsonToBinary",
     kJson,
     kBinary,
     benchPairRoundtrip<SimpleJSONSerializer, BinarySerializer, kJson>,
     benchPairDynamicValueRoundtrip<
         SimpleJSONProtocolReader,
         BinaryProtocolWriter,
         kJson>},
    {"Golden_BinaryToJson",
     kBinary,
     kJson,
     benchPairRoundtrip<BinarySerializer, SimpleJSONSerializer, kBinary>,
     benchPairDynamicValueRoundtrip<
         BinaryProtocolReader,
         SimpleJSONProtocolWriter,
         kBinary>},
}};

const std::array<ProtocolPair, 4>& protocolPairs() {
  return kProtocolPairs;
}

bool registerProtocolPairBenchmarks() {
  for (std::size_t i = 0; i < kProtocolPairs.size(); ++i) {
    const auto& pair = kProtocolPairs[i];
    folly::addBenchmark(
        __FILE__,
        std::string{pair.name} + "_DynamicValueRoundtrip",
        [pair](unsigned iters) {
          pair.benchDynamicValueRoundtrip(iters);
          return iters;
        });
    folly::addBenchmark(
        __FILE__,
        "%" + std::string{pair.name} + "_Cpp2Roundtrip",
        [pair](unsigned iters) {
          pair.benchCpp2Roundtrip(iters);
          return iters;
        });
    folly::addBenchmark(
        __FILE__,
        "%" + std::string{pair.name} + "_Kernel",
        [pair](unsigned iters) {
          benchTranscode(iters, pair.source, pair.target);
          return iters;
        });
    if (i + 1 < kProtocolPairs.size()) {
      folly::addBenchmark(__FILE__, "-", []() -> unsigned { return 0; });
    }
  }
  return true;
}

FOLLY_PUSH_WARNING
FOLLY_CLANG_DISABLE_WARNING("-Wglobal-constructors")
[[maybe_unused]] const bool kRegisteredProtocolPairBenchmarks =
    registerProtocolPairBenchmarks();
FOLLY_POP_WARNING

} // namespace
} // namespace apache::thrift::transcode

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  (void)apache::thrift::transcode::goldenFixture();
  folly::runBenchmarks();
  return 0;
}
