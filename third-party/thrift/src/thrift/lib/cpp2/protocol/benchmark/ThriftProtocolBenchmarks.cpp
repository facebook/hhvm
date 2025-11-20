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
#include "ThriftProtocolBenchmarkHelper.h"

using namespace apache::thrift;
/**
 * Serialization Benchmarks
 */
#define BENCHMARK_RUN_SERIALIZE(                                        \
    benchmarkType, testDataSize, rpcType, protocolType)                 \
  benchmarkType(                                                        \
      Serialize##_##rpcType##_##protocolType##_##testDataSize, iters) { \
    for (size_t i = 0; i < iters; ++i) {                                \
      auto serialized = serialize##rpcType##protocolType<               \
          benchmark::rpcType##TestData##testDataSize>();                \
      folly::doNotOptimizeAway(serialized);                             \
    }                                                                   \
  }

#define BENCHMARK_RUN_CARBON_SERIALIZATION(testDataSize)                   \
  BENCHMARK_RELATIVE(Serialize_Carbon_Default_##testDataSize, iters) {     \
    for (size_t i = 0; i < iters; ++i) {                                   \
      carbon::CarbonQueueAppenderStorage storage;                          \
      auto serialized =                                                    \
          serializeCarbonDefault<benchmark::CarbonTestData##testDataSize>( \
              storage);                                                    \
      folly::doNotOptimizeAway(serialized);                                \
    }                                                                      \
  }

#define BENCHMARK_RUN_SERIALIZE_BASELINE(testDataSize, rpcType, protocolType) \
  BENCHMARK_RUN_SERIALIZE(BENCHMARK, testDataSize, rpcType, protocolType)

#define BENCHMARK_RUN_SERIALIZE_RELATIVE(testDataSize, rpcType, protocolType) \
  BENCHMARK_RUN_SERIALIZE(                                                    \
      BENCHMARK_RELATIVE, testDataSize, rpcType, protocolType)

#define BENCHMARK_RUN_THRIFT_SERIALIZATION(testDataSize)          \
  BENCHMARK_RUN_SERIALIZE_BASELINE(testDataSize, Thrift, Binary)  \
  BENCHMARK_RUN_SERIALIZE_RELATIVE(testDataSize, Thrift, Compact) \
  BENCHMARK_RUN_SERIALIZE_RELATIVE(testDataSize, Thrift, CurSe)

#define BENCHMARK_RUN_PROTOBUF_SERIALIZATION(testDataSize)         \
  BENCHMARK_RUN_SERIALIZE_RELATIVE(testDataSize, Protobuf, Array)  \
  BENCHMARK_RUN_SERIALIZE_RELATIVE(testDataSize, Protobuf, String) \
  BENCHMARK_RUN_SERIALIZE_RELATIVE(testDataSize, Protobuf, ZeroCopy)

#define BENCHMARK_COMPARE_PROTOCOLS_SERIALIZATION(testDataSize) \
  BENCHMARK_RUN_THRIFT_SERIALIZATION(testDataSize)              \
  BENCHMARK_RUN_PROTOBUF_SERIALIZATION(testDataSize)            \
  BENCHMARK_RUN_CARBON_SERIALIZATION(testDataSize)

/**
 * Deserialization Benchmarks
 */

#define BENCHMARK_RUN_DESERIALIZE(                                            \
    benchmarkType, testDataSize, rpcType, protocolType)                       \
  benchmarkType(                                                              \
      Deserialize##_##rpcType##_##protocolType##_##testDataSize, iters) {     \
    for (size_t i = 0; i < iters; ++i) {                                      \
      folly::BenchmarkSuspender susp;                                         \
      auto serialized = serialize##rpcType##protocolType<                     \
          benchmark::rpcType##TestData##testDataSize>();                      \
      susp.dismiss();                                                         \
                                                                              \
      auto deserialized = deserialize##rpcType##protocolType<                 \
          benchmark::rpcType##TestData##testDataSize>(std::move(serialized)); \
      folly::doNotOptimizeAway(deserialized);                                 \
    }                                                                         \
  }

#define BENCHMARK_RUN_DESERIALIZE_BASELINE( \
    testDataSize, rpcType, protocolType)    \
  BENCHMARK_RUN_DESERIALIZE(BENCHMARK, testDataSize, rpcType, protocolType)

#define BENCHMARK_RUN_DESERIALIZE_RELATIVE( \
    testDataSize, rpcType, protocolType)    \
  BENCHMARK_RUN_DESERIALIZE(                \
      BENCHMARK_RELATIVE, testDataSize, rpcType, protocolType)

#define BENCHMARK_RUN_CURSE_DESERIALIZATION(testDataSize)                  \
  BENCHMARK_RELATIVE(Deserialize_Thrift_CurSe_##testDataSize, iters) {     \
    for (size_t i = 0; i < iters; ++i) {                                   \
      folly::BenchmarkSuspender susp;                                      \
      auto serialized =                                                    \
          serializeThriftCurSe<benchmark::ThriftTestData##testDataSize>(); \
      susp.dismiss();                                                      \
                                                                           \
      deserializeThriftCurSe<benchmark::ThriftTestData##testDataSize>(     \
          std::move(serialized));                                          \
    }                                                                      \
  }

#define BENCHMARK_RUN_THRIFT_DESERIALIZATION(testDataSize)          \
  BENCHMARK_RUN_DESERIALIZE_BASELINE(testDataSize, Thrift, Binary)  \
  BENCHMARK_RUN_DESERIALIZE_RELATIVE(testDataSize, Thrift, Compact) \
  BENCHMARK_RUN_CURSE_DESERIALIZATION(testDataSize)

#define BENCHMARK_RUN_PROTOBUF_DESERIALIZATION(testDataSize)         \
  BENCHMARK_RUN_DESERIALIZE_RELATIVE(testDataSize, Protobuf, Array)  \
  BENCHMARK_RUN_DESERIALIZE_RELATIVE(testDataSize, Protobuf, String) \
  BENCHMARK_RUN_DESERIALIZE_RELATIVE(testDataSize, Protobuf, ZeroCopy)

#define BENCHMARK_RUN_CARBON_DESERIALIZATION(testDataSize)                   \
  BENCHMARK_RELATIVE(Deserialize_Carbon_Default_##testDataSize, iters) {     \
    for (size_t i = 0; i < iters; ++i) {                                     \
      folly::BenchmarkSuspender susp;                                        \
      carbon::CarbonQueueAppenderStorage storage;                            \
      auto serialized =                                                      \
          serializeCarbonDefault<benchmark::CarbonTestData##testDataSize>(   \
              storage);                                                      \
      susp.dismiss();                                                        \
                                                                             \
      auto deserialized =                                                    \
          deserializeCarbonDefault<benchmark::CarbonTestData##testDataSize>( \
              serialized);                                                   \
      folly::doNotOptimizeAway(deserialized);                                \
    }                                                                        \
  }

#define BENCHMARK_COMPARE_PROTOCOLS_DESERIALIZATION(testDataSize) \
  BENCHMARK_RUN_THRIFT_DESERIALIZATION(testDataSize)              \
  BENCHMARK_RUN_PROTOBUF_DESERIALIZATION(testDataSize)            \
  BENCHMARK_RUN_CARBON_DESERIALIZATION(testDataSize)

/**
 * Compare all protocols
 */
#define BENCHMARK_COMPARE_PROTOCOLS(testDataSize)         \
  BENCHMARK_COMPARE_PROTOCOLS_SERIALIZATION(testDataSize) \
  BENCHMARK_DRAW_LINE();                                  \
  BENCHMARK_COMPARE_PROTOCOLS_DESERIALIZATION(testDataSize)

#define BENCHMARK_COMPARE_ALL_PROTOCOLS() \
  BENCHMARK_COMPARE_PROTOCOLS(Small)      \
  BENCHMARK_DRAW_LINE();                  \
  BENCHMARK_COMPARE_PROTOCOLS(Medium)     \
  BENCHMARK_DRAW_LINE();                  \
  BENCHMARK_COMPARE_PROTOCOLS(Large)

BENCHMARK_COMPARE_ALL_PROTOCOLS()

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}

/**
 * Results for last run: 2025-11-18
 * buck2 run @fbcode//mode/opt-clang-lto
 * fbcode//thrift/lib/cpp2/protocol/benchmark:thrift_protocol_benchmarks
 * ============================================================================
 * [...]enchmark/ThriftProtocolBenchmarks.cpp     relative  time/iter   iters/s
 * ============================================================================
 * Serialize_Thrift_Binary_Small                             380.11ns     2.63M
 * Serialize_Thrift_Compact_Small                  96.143%   395.36ns     2.53M
 * Serialize_Thrift_CurSe_Small                    164.66%   230.85ns     4.33M
 * Serialize_Protobuf_Array_Small                  40.194%   945.69ns     1.06M
 * Serialize_Protobuf_String_Small                 45.670%   832.30ns     1.20M
 * Serialize_Protobuf_ZeroCopy_Small               38.084%   998.08ns     1.00M
 * Serialize_Carbon_Default_Small                  66.659%   570.23ns     1.75M
 * ----------------------------------------------------------------------------
 * Deserialize_Thrift_Binary_Small                           396.13ns     2.52M
 * Deserialize_Thrift_Compact_Small                93.348%   424.35ns     2.36M
 * Deserialize_Thrift_CurSe_Small                  92.639%   427.60ns     2.34M
 * Deserialize_Protobuf_Array_Small                59.960%   660.66ns     1.51M
 * Deserialize_Protobuf_String_Small               59.950%   660.76ns     1.51M
 * Deserialize_Protobuf_ZeroCopy_Small             58.161%   681.08ns     1.47M
 * Deserialize_Carbon_Default_Small                59.696%   663.57ns     1.51M
 * ----------------------------------------------------------------------------
 * Serialize_Thrift_Binary_Medium                            936.16ns     1.07M
 * Serialize_Thrift_Compact_Medium                 98.710%   948.40ns     1.05M
 * Serialize_Thrift_CurSe_Medium                   205.26%   456.09ns     2.19M
 * Serialize_Protobuf_Array_Medium                 31.792%     2.94us   339.60K
 * Serialize_Protobuf_String_Medium                36.678%     2.55us   391.79K
 * Serialize_Protobuf_ZeroCopy_Medium              30.784%     3.04us   328.83K
 * Serialize_Carbon_Default_Medium                 55.541%     1.69us   593.28K
 * ----------------------------------------------------------------------------
 * Deserialize_Thrift_Binary_Medium                            1.06us   939.75K
 * Deserialize_Thrift_Compact_Medium               96.496%     1.10us   906.82K
 * Deserialize_Thrift_CurSe_Medium                 95.684%     1.11us   899.19K
 * Deserialize_Protobuf_Array_Medium               49.946%     2.13us   469.36K
 * Deserialize_Protobuf_String_Medium              51.209%     2.08us   481.24K
 * Deserialize_Protobuf_ZeroCopy_Medium            48.797%     2.18us   458.57K
 * Deserialize_Carbon_Default_Medium               54.886%     1.94us   515.79K
 * ----------------------------------------------------------------------------
 * Serialize_Thrift_Binary_Large                               2.72us   367.88K
 * Serialize_Thrift_Compact_Large                  98.904%     2.75us   363.84K
 * Serialize_Thrift_CurSe_Large                    221.03%     1.23us   813.13K
 * Serialize_Protobuf_Array_Large                  28.509%     9.54us   104.88K
 * Serialize_Protobuf_String_Large                 32.661%     8.32us   120.15K
 * Serialize_Protobuf_ZeroCopy_Large               28.314%     9.60us   104.16K
 * Serialize_Carbon_Default_Large                  49.710%     5.47us   182.87K
 * ----------------------------------------------------------------------------
 * Deserialize_Thrift_Binary_Large                             3.15us   317.08K
 * Deserialize_Thrift_Compact_Large                95.730%     3.29us   303.54K
 * Deserialize_Thrift_CurSe_Large                  94.523%     3.34us   299.71K
 * Deserialize_Protobuf_Array_Large                47.429%     6.65us   150.39K
 * Deserialize_Protobuf_String_Large               47.185%     6.68us   149.61K
 * Deserialize_Protobuf_ZeroCopy_Large             47.922%     6.58us   151.95K
 * Deserialize_Carbon_Default_Large                55.853%     5.65us   177.10K
 */
