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

#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/test/Structs.h>

#include <folly/Benchmark.h>
#include <folly/init/Init.h>

namespace apache::thrift::protocol {

using ::thrift::benchmark::ComplexStruct;
using Tag = type::struct_t<ComplexStruct>;

const auto staticObj = create<ComplexStruct>();
const auto serializedObj =
    BinarySerializer::serialize<folly::IOBufQueue>(staticObj).move();
const auto obj = asValueStruct<Tag>(staticObj).as_object();

BENCHMARK(ProtocolSerialization) {
  auto buf = serializeObject<apache::thrift::BinaryProtocolWriter>(obj);
  folly::doNotOptimizeAway(buf);
}

BENCHMARK_RELATIVE(StaticSerialization) {
  auto buf = BinarySerializer::serialize<folly::IOBufQueue>(staticObj).move();
  folly::doNotOptimizeAway(buf);
}

BENCHMARK(ProtocolDeserialization) {
  auto val = parseObject<apache::thrift::BinaryProtocolReader>(*serializedObj);
  folly::doNotOptimizeAway(val);
}

BENCHMARK_RELATIVE(StaticDeserialization) {
  auto val = BinarySerializer::deserialize<ComplexStruct>(serializedObj.get());
  folly::doNotOptimizeAway(val);
}

BENCHMARK(FromThriftStruct) {
  auto val = asValueStruct<Tag>(staticObj);
  folly::doNotOptimizeAway(val);
}

BENCHMARK(ToThriftStruct) {
  auto val = fromObjectStruct<Tag>(obj);
  folly::doNotOptimizeAway(val);
}

} // namespace apache::thrift::protocol

/**
buck run @mode/opt //thrift/test/benchmarks:protocol_object_benchmark
============================================================================
[...]chmarks/protocol_object_benchmark.cpp     relative  time/iter   iters/s
============================================================================
ProtocolSerialization                                     100.33ms      9.97
StaticSerialization                             151.68%    66.15ms     15.12
ProtocolDeserialization                                   433.52ms      2.31
StaticDeserialization                           452.01%    95.91ms     10.43
FromThriftStruct                                          652.32ms      1.53
ToThriftStruct                                            128.89ms      7.76
 */
int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::runBenchmarks();
}
