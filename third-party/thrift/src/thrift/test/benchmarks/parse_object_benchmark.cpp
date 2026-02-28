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

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/test/Structs.h>

namespace apache::thrift::protocol {

using ::thrift::benchmark::ComplexStruct;
using ::thrift::benchmark::Mixed;

template <typename Serializer, typename T>
folly::IOBuf getSerializedCoalscedBuf(const T& t) {
  folly::IOBufQueue queue;
  Serializer::serialize(t, &queue);
  auto buf = queue.moveAsValue();
  buf.coalesce();
  return buf;
}

const auto complexStruct = create<ComplexStruct>();
const auto mixed = create<Mixed>();
const auto complexStructCompactBuf =
    getSerializedCoalscedBuf<CompactSerializer>(complexStruct);
const auto complexStructBinaryBuf =
    getSerializedCoalscedBuf<BinarySerializer>(complexStruct);
const auto mixedCompactBuf = getSerializedCoalscedBuf<CompactSerializer>(mixed);
const auto mixedBinaryBuf = getSerializedCoalscedBuf<BinarySerializer>(mixed);
const auto mixedBufMask = []() {
  Mask m;
  m.includes().emplace()[16] = allMask();
  return m;
}();

BENCHMARK(OpDecodeComplexStruct_Compact) {
  ComplexStruct obj;
  CompactProtocolReader reader;
  reader.setInput(&complexStructCompactBuf);
  op::decode<type::struct_t<ComplexStruct>>(reader, obj);
  folly::doNotOptimizeAway(obj);
}

BENCHMARK_RELATIVE(ParseObjectComplexStruct_Compact) {
  auto obj =
      protocol::parseObject<CompactProtocolReader>(complexStructCompactBuf);
  folly::doNotOptimizeAway(obj);
}

BENCHMARK(OpDecodeMixed_Compact) {
  Mixed obj;
  CompactProtocolReader reader;
  reader.setInput(&mixedCompactBuf);
  op::decode<type::struct_t<Mixed>>(reader, obj);
  folly::doNotOptimizeAway(obj);
}

BENCHMARK_RELATIVE(ParseObjectMixed_Compact) {
  auto obj = protocol::parseObject<CompactProtocolReader>(mixedCompactBuf);
  folly::doNotOptimizeAway(obj);
}

BENCHMARK_RELATIVE(ParseObjectMixedWithMaskFromComplexStruct_Compact) {
  auto obj = protocol::parseObject<CompactProtocolReader>(
      complexStructCompactBuf, mixedBufMask);
  folly::doNotOptimizeAway(obj);
}

BENCHMARK(OpDecodeComplexStruct_Binary) {
  ComplexStruct obj;
  BinaryProtocolReader reader;
  reader.setInput(&complexStructBinaryBuf);
  op::decode<type::struct_t<ComplexStruct>>(reader, obj);
  folly::doNotOptimizeAway(obj);
}

BENCHMARK_RELATIVE(ParseObjectComplexStruct_Binary) {
  auto obj =
      protocol::parseObject<BinaryProtocolReader>(complexStructBinaryBuf);
  folly::doNotOptimizeAway(obj);
}

BENCHMARK(OpDecodeMixed_Binary) {
  Mixed obj;
  BinaryProtocolReader reader;
  reader.setInput(&mixedBinaryBuf);
  op::decode<type::struct_t<Mixed>>(reader, obj);
  folly::doNotOptimizeAway(obj);
}

BENCHMARK_RELATIVE(ParseObjectMixed_Binary) {
  auto obj = protocol::parseObject<BinaryProtocolReader>(mixedBinaryBuf);
  folly::doNotOptimizeAway(obj);
}

BENCHMARK_RELATIVE(ParseObjectMixedWithMaskFromComplexStruct_Binary) {
  auto obj = protocol::parseObject<BinaryProtocolReader>(
      complexStructBinaryBuf, mixedBufMask);
  folly::doNotOptimizeAway(obj);
}

} // namespace apache::thrift::protocol

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::runBenchmarks();
}
