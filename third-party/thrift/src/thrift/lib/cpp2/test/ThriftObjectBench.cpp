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
#include <thrift/lib/cpp2/test/Structs.h>

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

constexpr std::size_t QUEUE_ALLOC_SIZE = 4 * 1024 * 1024;

folly::IOBufQueue& get_queue() {
  static folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  return queue;
}

template <typename ProtocolWriter, typename T>
std::unique_ptr<folly::IOBuf> serialize(T& s) {
  folly::IOBufQueue iobufQueue;
  ProtocolWriter writer;
  writer.setOutput(&iobufQueue);
  s.write(&writer);
  return iobufQueue.move();
}

template <typename T, typename ProtocolWriter, typename F>
void withSerialized(F&& f) {
  folly::BenchmarkSuspender suspender;
  auto t = create<T>();
  auto buf = serialize<ProtocolWriter>(t);
  buf->coalesce();
  suspender.dismiss();
  const auto res = f(*buf);
  // Destructors & clean-up are excluded from benchmarking
  suspender.rehire();
  folly::doNotOptimizeAway(res);
  get_queue().clearAndTryReuseLargestBuffer();
}

template <typename T, typename ProtocolWriter, typename ParseF, typename F>
void withParsed(ParseF&& parseF, F&& f) {
  folly::BenchmarkSuspender suspender;
  auto t = create<T>();
  auto buf = serialize<ProtocolWriter>(t);
  buf->coalesce();
  const auto obj = parseF(*buf);
  suspender.dismiss();
  const auto res = f(obj);
  // Destructors & clean-up are excluded from benchmarking
  suspender.rehire();
  folly::doNotOptimizeAway(res);
  get_queue().clearAndTryReuseLargestBuffer();
}

template <typename ProtocolWriter, typename ProtocolReader, typename T>
void bench_decode_protocol() {
  withSerialized<T, ProtocolWriter>([](folly::IOBuf& buf) {
    return protocol::parseObject<ProtocolReader>(buf);
  });
}

template <typename ProtocolWriter, typename ProtocolReader, typename T>
void bench_encode_protocol() {
  withParsed<T, ProtocolWriter>(
      [](auto& buf) { return protocol::parseObject<ProtocolReader>(buf); },
      [](const auto& obj) {
        protocol::serializeObject<ProtocolWriter>(obj, get_queue());
        return 0;
      });
}

template <typename ProtocolWriter, typename ProtocolReader, typename T>
void bench_read_all_protocol() {
  withParsed<T, ProtocolWriter>(
      [](auto& buf) { return protocol::parseObject<ProtocolReader>(buf); },
      [](const auto& obj) { return read_all(obj); });
}

template <typename ProtocolWriter, typename ProtocolReader, typename T>
void bench_read_half_protocol() {
  withParsed<T, ProtocolWriter>(
      [](auto& buf) { return protocol::parseObject<ProtocolReader>(buf); },
      [](const auto& obj) { return read_some(SparseAccess::Half, obj); });
}

template <typename ProtocolWriter, typename ProtocolReader, typename T>
void bench_read_first_protocol() {
  withParsed<T, ProtocolWriter>(
      [](auto& buf) { return protocol::parseObject<ProtocolReader>(buf); },
      [](const auto& obj) { return read_some(SparseAccess::First, obj); });
}

#define BENCH_OP(PROT_NAME, OP, WRITER, READER, T) \
  BENCHMARK(PROT_NAME##_protocol_##T##_##OP) {     \
    bench_##OP##_protocol<WRITER, READER, T>();    \
  }

// Define a list of protocols to benchmark each operation + struct
#define FOR_EACH_PROTOCOL(OP, T)                                      \
  BENCH_OP(Binary, OP, BinaryProtocolWriter, BinaryProtocolReader, T) \
  BENCH_OP(Compact, OP, CompactProtocolWriter, CompactProtocolReader, T)

// Define a list of operations to benchmark for each struct
#define BENCH_T(T)                \
  FOR_EACH_PROTOCOL(decode, T)    \
  FOR_EACH_PROTOCOL(encode, T)    \
  FOR_EACH_PROTOCOL(read_all, T)  \
  FOR_EACH_PROTOCOL(read_half, T) \
  FOR_EACH_PROTOCOL(read_first, T)

// Define benchmarks for each struct
BENCH_T(Empty)
BENCH_T(SmallInt)
BENCH_T(BigInt)
BENCH_T(SmallString)
BENCH_T(BigString)
BENCH_T(Mixed)
BENCH_T(MixedInt)
BENCH_T(LargeMixed)
BENCH_T(SmallListInt)
BENCH_T(BigListInt)
BENCH_T(BigListMixed)
BENCH_T(BigListMixedInt)
BENCH_T(LargeListMixed)
BENCH_T(LargeSetInt)
BENCH_T(UnorderedSetInt)
BENCH_T(SortedVecSetInt)
BENCH_T(LargeMapInt)
BENCH_T(LargeMapMixed)
BENCH_T(LargeUnorderedMapMixed)
BENCH_T(LargeSortedVecMapMixed)
BENCH_T(UnorderedMapInt)
BENCH_T(NestedMap)
BENCH_T(SortedVecNestedMap)
BENCH_T(ComplexStruct)

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  get_queue().preallocate(QUEUE_ALLOC_SIZE, QUEUE_ALLOC_SIZE);
  runBenchmarks();
  return 0;
}
