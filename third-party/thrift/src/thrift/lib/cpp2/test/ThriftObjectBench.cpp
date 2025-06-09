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

#include <thrift/lib/cpp2/frozen/FrozenUtil.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/test/ObjectBenchUtils.h>

#include <glog/logging.h>
#include <folly/Benchmark.h>
#include <folly/BenchmarkUtil.h>
#include <folly/init/Init.h>
#include <folly/portability/GFlags.h>

using namespace std;
using namespace folly;
using namespace apache::thrift;
using namespace thrift::benchmark;
using namespace apache::thrift::test::utils;
namespace experimental = apache::thrift::protocol::experimental;

constexpr std::size_t QUEUE_ALLOC_SIZE = 16 * 1024 * 1024;

FBTHRIFT_GEN_SERDE()

#define BENCH_OP(PROT_NAME, OP, WRITER, READER, T)               \
  BENCHMARK(PROT_NAME##_protocol_##T##_##OP) {                   \
    OP<WRITER, READER>(get_serde<T, WRITER, READER>());          \
  }                                                              \
  BENCHMARK_RELATIVE(PROT_NAME##_native_##T##_##OP) {            \
    OP##_native<WRITER, READER>(get_serde<T, WRITER, READER>()); \
  }

FBTHRIFT_GEN_PROTOCOL_BENCHMARKS(BENCH_OP)

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  get_queue().preallocate(QUEUE_ALLOC_SIZE, QUEUE_ALLOC_SIZE);
  runBenchmarks();
  return 0;
}
