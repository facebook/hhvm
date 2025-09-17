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

#include <vector>

#include <fmt/format.h>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <google/protobuf/arena.h> // @manual
#include <jemalloc/jemalloc.h>

#include <folly/Benchmark.h>
#include <folly/Format.h>
#include <folly/init/Init.h>
#include <folly/memory/MallctlHelper.h>
#include <folly/portability/GFlags.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/test/ProtoBufBenchData.pb.h>
#include <thrift/lib/cpp2/test/Structs.h>
#include <thrift/lib/cpp2/test/gen-cpp2/ProtocolBenchData_types.h>

using namespace std;
using namespace folly;
using namespace apache::thrift;
using namespace thrift::benchmark;

DEFINE_bool(
    memory_request,
    false,
    "If true, running memory request count. If false, running benchmark");

static uint64_t getMemoryRequestsCounter() {
  size_t narenas = MALLCTL_ARENAS_ALL;
  std::string keySmall =
      folly::sformat("stats.arenas.{}.small.nrequests", narenas);
  std::string keyLarge =
      folly::sformat("stats.arenas.{}.large.nrequests", narenas);

  folly::mallctlCall("thread.tcache.flush");
  folly::mallctlWrite<uint64_t>("epoch", 1);

  uint64_t smallCount = 0, largeCount = 0;
  folly::mallctlRead(keySmall.c_str(), &smallCount);
  folly::mallctlRead(keyLarge.c_str(), &largeCount);
  return smallCount + largeCount;
}

// The benckmark is to measure single struct use case, the iteration here is
// more like a benchmark artifact, so avoid doing optimizationon iteration
// usecase in this benchmark (e.g. move string definition out of while loop)

/************************ Thrift write(serialize) *****************************/

template <typename Serializer, typename Struct>
void writeBench(size_t iter) {
  BenchmarkSuspender susp;
  auto strct = create<Struct>();
  susp.dismiss();
  while (iter--) {
    IOBufQueue q;
    Serializer::serialize(strct, &q);
    folly::doNotOptimizeAway(q);
  }
  susp.rehire();
}

template <typename Serializer, typename Struct>
static int writeBenchMemory(size_t iters) {
  auto strct = create<Struct>();
  auto before = getMemoryRequestsCounter();

  while (iters--) {
    IOBufQueue q;
    Serializer::serialize(strct, &q);
  }

  auto after = getMemoryRequestsCounter();

  return after - before;
}

/************************ Protobuf write(serialize) ***************************/

template <typename Struct>
static void writeBench(size_t iter) {
  BenchmarkSuspender susp;
  auto strct = create<Struct>();
  susp.dismiss();

  string s;
  while (iter--) {
    strct.SerializeToString(&s);
    s.clear();
  }

  susp.rehire();
}

template <typename Struct>
static int writeBenchMemory(size_t iters) {
  auto strct = create<Struct>();
  auto before = getMemoryRequestsCounter();

  string s;
  while (iters--) {
    strct.SerializeToString(&s);
    s.clear();
  }

  auto after = getMemoryRequestsCounter();
  return after - before;
}

template <typename Struct>
static int writeBenchArenaMemory(size_t iters) {
  google::protobuf::Arena arena;
  auto* data = google::protobuf::Arena::CreateMessage<Struct>(&arena);
  *data = create<Struct>();

  auto before = getMemoryRequestsCounter();

  string s;
  while (iters--) {
    data->SerializeToString(&s);
    s.clear();
  }

  auto after = getMemoryRequestsCounter();
  return after - before;
}

/************************ Thrift read(deserialize) **************************/

template <typename Serializer, typename Struct>
static void readBench(size_t iter) {
  BenchmarkSuspender susp;
  auto strct = create<Struct>();
  IOBufQueue q;
  Serializer::serialize(strct, &q);
  auto buf = q.move();
  susp.dismiss();

  while (iter--) {
    Struct data;
    Serializer::deserialize(buf.get(), data);
    folly::doNotOptimizeAway(data);
  }

  susp.rehire();
}

template <typename Serializer, typename Struct>
static int readBenchMemory(size_t iters) {
  auto strct = create<Struct>();
  IOBufQueue q;
  Serializer::serialize(strct, &q);
  auto buf = q.move();

  auto before = getMemoryRequestsCounter();

  while (iters--) {
    Struct data;
    Serializer::deserialize(buf.get(), data);
  }

  auto after = getMemoryRequestsCounter();
  return after - before;
}

/************************ Protobuf read(deserialize) **************************/

template <typename Struct>
static void readBench(size_t iter) {
  BenchmarkSuspender susp;
  auto strct = create<Struct>();
  string s;
  strct.SerializeToString(&s);

  susp.dismiss();

  while (iter--) {
    Struct data;
    data.ParseFromString(s);
    folly::doNotOptimizeAway(data);
  }
  susp.rehire();
}

template <typename Struct>
static int readBenchMemory(size_t iters) {
  auto strct = create<Struct>();
  string s;
  strct.SerializeToString(&s);

  auto before = getMemoryRequestsCounter();

  while (iters--) {
    Struct data;
    data.ParseFromString(s);
  }

  auto after = getMemoryRequestsCounter();

  return after - before;
}

template <typename Struct>
static int readBenchArenaMemory(size_t iters) {
  auto strct = create<Struct>();
  string s;
  strct.SerializeToString(&s);

  auto before = getMemoryRequestsCounter();

  while (iters--) {
    google::protobuf::Arena arena;
    auto* data = google::protobuf::Arena::CreateMessage<Struct>(&arena);
    data->ParseFromString(s);
  }

  auto after = getMemoryRequestsCounter();

  return after - before;
}

#define BENCHMARK_MACRO_RELATIVE(proto, rdwr, bench)  \
  BENCHMARK_RELATIVE(Thrift_##rdwr##_##bench, iter) { \
    rdwr##Bench<proto##Serializer, bench>(iter);      \
  }

#define BENCHMARK_MACRO_NORM(proto, rdwr, bench) \
  BENCHMARK(proto##_##rdwr##_##bench, iter) {    \
    rdwr##Bench<p##bench>(iter);                 \
  }

#define BENCHMARK_MACRO(bench)                    \
  BENCHMARK_MACRO_NORM(ProtoBuf, read, bench)     \
  BENCHMARK_MACRO_RELATIVE(Compact, read, bench)  \
  BENCHMARK_MACRO_NORM(ProtoBuf, write, bench)    \
  BENCHMARK_MACRO_RELATIVE(Compact, write, bench) \
  BENCHMARK_DRAW_LINE();

using pEmpty = protobuf::Empty;
BENCHMARK_MACRO(Empty)
using pSmallInt = protobuf::SmallInt;
BENCHMARK_MACRO(SmallInt)
using pBigInt = protobuf::BigInt;
BENCHMARK_MACRO(BigInt)
using pMixedInt = protobuf::MixedInt;
BENCHMARK_MACRO(MixedInt)
using pSmallString = protobuf::SmallString;
BENCHMARK_MACRO(SmallString)
using pBigString = protobuf::BigString;
BENCHMARK_MACRO(BigString)
using pMixed = protobuf::Mixed;
BENCHMARK_MACRO(Mixed)
using pLargeMixed = protobuf::LargeMixed;
BENCHMARK_MACRO(LargeMixed)
using pSmallListInt = protobuf::SmallListInt;
BENCHMARK_MACRO(SmallListInt)
using pBigListInt = protobuf::BigListInt;
BENCHMARK_MACRO(BigListInt)
using pBigListMixed = protobuf::BigListMixed;
BENCHMARK_MACRO(BigListMixed)
using pLargeListMixed = protobuf::LargeListMixed;
BENCHMARK_MACRO(LargeListMixed)
using pLargeMapInt = protobuf::LargeMapInt;
BENCHMARK_MACRO(LargeMapInt)
using pNestedMap = protobuf::NestedMap;
BENCHMARK_MACRO(NestedMap)
using pComplexStruct = protobuf::ComplexStruct;
BENCHMARK_MACRO(ComplexStruct)

// some special map types for Thrift
BENCHMARK_MACRO_NORM(ProtoBuf1, read, NestedMap)
BENCHMARK_MACRO_RELATIVE(Compact, read, SortedVecNestedMapRaw)
BENCHMARK_MACRO_NORM(ProtoBuf1, write, NestedMap)
BENCHMARK_MACRO_RELATIVE(Compact, write, SortedVecNestedMapRaw)

static std::string outputFormat(const std::string& name, int a, int b, int c) {
  return fmt::format(
      "{:20} Thrift: {:<20} Protobuf: {:<20} "
      "Protobuf with Arena: {:<20}",
      name,
      a,
      b,
      c);
}

#define BENCHMARK_MEM(rdwr, bench, iter)                        \
  void bench##_##rdwr() {                                       \
    auto a = rdwr##BenchMemory<CompactSerializer, bench>(iter); \
    auto b = rdwr##BenchMemory<p##bench>(iter);                 \
    auto c = rdwr##BenchArenaMemory<p##bench>(iter);            \
    LOG(INFO) << outputFormat(__FUNCTION__, a, b, c);           \
  }

BENCHMARK_MEM(read, MixedInt, 10000)
BENCHMARK_MEM(write, MixedInt, 10000)
BENCHMARK_MEM(read, Empty, 10000)
BENCHMARK_MEM(write, Empty, 10000)
BENCHMARK_MEM(read, SmallInt, 10000)
BENCHMARK_MEM(write, SmallInt, 10000)
BENCHMARK_MEM(read, BigInt, 10000)
BENCHMARK_MEM(write, BigInt, 10000)
BENCHMARK_MEM(read, SmallString, 10000)
BENCHMARK_MEM(write, SmallString, 10000)
BENCHMARK_MEM(read, BigString, 10000)
BENCHMARK_MEM(write, BigString, 10000)
BENCHMARK_MEM(read, Mixed, 10000)
BENCHMARK_MEM(write, Mixed, 10000)
BENCHMARK_MEM(read, LargeMixed, 10000)
BENCHMARK_MEM(write, LargeMixed, 10000)

// container
BENCHMARK_MEM(read, SmallListInt, 1000)
BENCHMARK_MEM(write, SmallListInt, 1000)
BENCHMARK_MEM(read, BigListInt, 1)
BENCHMARK_MEM(write, BigListInt, 1)
BENCHMARK_MEM(read, BigListMixed, 1)
BENCHMARK_MEM(write, BigListMixed, 1)
BENCHMARK_MEM(read, LargeMapInt, 1)
BENCHMARK_MEM(write, LargeMapInt, 1)
BENCHMARK_MEM(read, NestedMap, 1)
BENCHMARK_MEM(write, NestedMap, 1)
BENCHMARK_MEM(read, ComplexStruct, 1)
BENCHMARK_MEM(write, ComplexStruct, 1)

int main(int argc, char** argv) {
  const folly::Init init(&argc, &argv);
  if (FLAGS_memory_request) {
    if (folly::kIsSanitize) {
      LOG(ERROR) << "cannot use jemalloc with sanitizer, abort!";
      return 1;
    }

    LOG(INFO)
        << "number of memeory allocation request called during benchmarking";
    LOG(INFO) << "10000 iterations:";
    Empty_read();
    Empty_write();
    MixedInt_read();
    MixedInt_write();
    SmallInt_read();
    SmallInt_write();
    BigInt_read();
    BigInt_write();
    SmallString_read();
    SmallString_write();
    BigString_read();
    BigString_write();
    Mixed_read();
    Mixed_write();
    LargeMixed_read();
    LargeMixed_write();
    LOG(INFO) << "iter * container size = 10000:";
    SmallListInt_read();
    SmallListInt_write();
    BigListInt_read();
    BigListInt_write();
    BigListMixed_read();
    BigListMixed_write();
    LOG(INFO) << "container size = 1000000:";
    LargeMapInt_read();
    LargeMapInt_write();
    ComplexStruct_read();
    ComplexStruct_write();
    LOG(INFO) << "nested map size = 100000:";
    auto a1 = readBenchMemory<CompactSerializer, NestedMap>(1);
    auto b1 = writeBenchMemory<CompactSerializer, NestedMap>(1);
    auto a2 = readBenchMemory<pNestedMap>(1);
    auto b2 = writeBenchMemory<pNestedMap>(1);
    auto a3 = readBenchMemory<CompactSerializer, SortedVecNestedMapRaw>(1);
    auto b3 = writeBenchMemory<CompactSerializer, SortedVecNestedMapRaw>(1);
    auto arena1 = readBenchArenaMemory<pNestedMap>(1);
    auto arena2 = writeBenchArenaMemory<pNestedMap>(1);
    LOG(INFO) << fmt::format(
        "{} sorted_vec_nested_map: {}",
        outputFormat("NestedMap_read", a1, a2, arena1),
        a3);
    LOG(INFO) << fmt::format(
        "{} sorted_vec_nested_map: {}",
        outputFormat("NestedMap_write", b1, b2, arena2),
        b3);
  } else {
    runBenchmarks();
  }

  return 0;
}
