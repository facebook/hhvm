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

#include <thrift/lib/cpp2/test/Structs.h>

#include <glog/logging.h>
#include <google/protobuf/arena.h> // @manual
#include <folly/Benchmark.h>
#include <folly/portability/GFlags.h>

using namespace std;
using namespace folly;
using namespace protobuf;

// The benckmark is to measure single struct use case, the iteration here is
// more like a benchmark artifact, so avoid doing optimizationon iteration
// usecase in this benchmark (e.g. move string definition out of while loop)

template <typename Struct>
void writeBench(size_t iters) {
  BenchmarkSuspender susp;
  auto strct = create<Struct>();
  susp.dismiss();

  while (iters--) {
    string s;
    strct.SerializeToString(&s);
  }
  susp.rehire();
}

template <typename Struct>
void writeBenchArena(size_t iters) {
  BenchmarkSuspender susp;
  google::protobuf::Arena arena;
  auto* data = google::protobuf::Arena::CreateMessage<Struct>(&arena);
  *data = create<Struct>();
  susp.dismiss();

  while (iters--) {
    string s;
    data->SerializeToString(&s);
  }
  susp.rehire();
}

template <typename Struct>
void readBench(size_t iters) {
  BenchmarkSuspender susp;
  auto strct = create<Struct>();
  string s;
  strct.SerializeToString(&s);
  susp.dismiss();

  while (iters--) {
    Struct data;
    data.ParseFromString(s);
  }
  susp.rehire();
}

template <typename Struct>
void readBenchArena(size_t iters) {
  BenchmarkSuspender susp;
  auto strct = create<Struct>();
  string s;
  strct.SerializeToString(&s);
  susp.dismiss();

  google::protobuf::Arena arena;
  while (iters--) {
    auto* data = google::protobuf::Arena::CreateMessage<Struct>(&arena);
    data->ParseFromString(s);
    data->release_message();
  }
  susp.rehire();
}

#define BENCHMARK_MACRO(proto, rdwr, bench)    \
  BENCHMARK(proto##_##rdwr##_##bench, iters) { \
    rdwr##Bench<bench>(iters);                 \
  }

#define BENCHMARK_MACRO_RELATIVE(proto, rdwr, bench)          \
  BENCHMARK_RELATIVE(proto##_Arena_##rdwr##_##bench, iters) { \
    rdwr##Bench<bench>(iters);                                \
  }

#define BENCHMARK_RW(proto, bench)             \
  BENCHMARK_MACRO(proto, read, bench)          \
  BENCHMARK_MACRO_RELATIVE(proto, read, bench) \
  BENCHMARK_MACRO(proto, write, bench)         \
  BENCHMARK_MACRO_RELATIVE(proto, write, bench)

#define BENCHMARK_ALL(proto)          \
  BENCHMARK_RW(proto, Empty)          \
  BENCHMARK_RW(proto, SmallInt)       \
  BENCHMARK_RW(proto, BigInt)         \
  BENCHMARK_RW(proto, SmallString)    \
  BENCHMARK_RW(proto, BigString)      \
  BENCHMARK_RW(proto, Mixed)          \
  BENCHMARK_RW(proto, SmallListInt)   \
  BENCHMARK_RW(proto, BigListInt)     \
  BENCHMARK_RW(proto, BigListMixed)   \
  BENCHMARK_RW(proto, LargeListMixed) \
  BENCHMARK_RW(proto, LargeMapInt)    \
  BENCHMARK_RW(proto, NestedMap)      \
  BENCHMARK_RW(proto, LargeMixed)

BENCHMARK_ALL(ProtoBuf)

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  runBenchmarks();

  return 0;
}
