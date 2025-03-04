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

constexpr std::size_t QUEUE_ALLOC_SIZE = 16 * 1024 * 1024;

template <typename ProtocolWriter, typename T>
std::unique_ptr<folly::IOBuf> serialize(T& s) {
  folly::IOBufQueue iobufQueue;
  ProtocolWriter writer;
  writer.setOutput(&iobufQueue);
  s.write(&writer);
  auto buf = iobufQueue.move();
  buf->coalesce();
  return buf;
}

template <typename T>
struct TestingObject {
  T val;
  std::unique_ptr<folly::IOBuf> buf;
  protocol::Object obj;

  template <typename ProtocolWriter, typename ProtocolReader>
  static TestingObject<T> make() {
    T value = create<T>();
    std::unique_ptr<folly::IOBuf> buf = serialize<ProtocolWriter>(value);
    protocol::Object obj = protocol::parseObject<ProtocolReader>(*buf);
    return TestingObject<T>{std::move(value), std::move(buf), std::move(obj)};
  }

 private:
  TestingObject(
      T&& val, std::unique_ptr<folly::IOBuf>&& buf, protocol::Object&& obj)
      : val{std::move(val)}, buf{std::move(buf)}, obj{std::move(obj)} {}
};

folly::IOBufQueue& get_queue() {
  static folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  return queue;
}

// Constructs a serialized buffer for the given type encoded with a given
// Protocol(Writer)
template <typename T, typename ProtocolWriter, typename ProtocolReader>
const TestingObject<T>& get_serde();

template <typename ProtocolWriter, typename ProtocolReader, typename T>
void bench_decode_protocol(const TestingObject<T>& input) {
  const auto obj = protocol::parseObject<ProtocolReader>(*input.buf);
  folly::doNotOptimizeAway(obj);
  BENCHMARK_SUSPEND {
    std::destroy_at(&obj);
  }
}

template <typename ProtocolWriter, typename ProtocolReader, typename T>
void bench_encode_protocol(const TestingObject<T>& input) {
  auto& queue = get_queue();
  protocol::serializeObject<ProtocolWriter>(input.obj, get_queue());
  folly::doNotOptimizeAway(queue);
  BENCHMARK_SUSPEND {
    queue.clearAndTryReuseLargestBuffer();
  }
}

template <typename ProtocolWriter, typename ProtocolReader, typename T>
void bench_read_all_protocol(const TestingObject<T>& input) {
  const auto val = read_all(input.obj);
  folly::doNotOptimizeAway(val);
}

template <typename ProtocolWriter, typename ProtocolReader, typename T>
void bench_read_half_protocol(const TestingObject<T>& input) {
  const auto val = read_some(SparseAccess::Half, input.obj);
  folly::doNotOptimizeAway(val);
}

template <typename ProtocolWriter, typename ProtocolReader, typename T>
void bench_read_first_protocol(const TestingObject<T>& input) {
  const auto val = read_some(SparseAccess::SingleRandom, input.obj);
  folly::doNotOptimizeAway(val);
}

#define BENCH_OP(PROT_NAME, OP, WRITER, READER, T)                            \
  BENCHMARK(PROT_NAME##_protocol_##T##_##OP) {                                \
    bench_##OP##_protocol<WRITER, READER, T>(get_serde<T, WRITER, READER>()); \
  }

// Define prototype serialization & deserialization functions for each struct &
// protocol
#define DEFINE_SERDE(T, WRITER, READER)                                     \
  template <>                                                               \
  const TestingObject<T>& get_serde<T, WRITER, READER>() {                  \
    static TestingObject<T> obj = TestingObject<T>::make<WRITER, READER>(); \
    return obj;                                                             \
  }

// Define a list of operations to benchmark for each struct + protocol
#define FOR_EACH_OP(PROT_NAME, T, WRITER, READER)   \
  DEFINE_SERDE(T, WRITER, READER)                   \
  BENCH_OP(PROT_NAME, decode, WRITER, READER, T)    \
  BENCH_OP(PROT_NAME, encode, WRITER, READER, T)    \
  BENCH_OP(PROT_NAME, read_all, WRITER, READER, T)  \
  BENCH_OP(PROT_NAME, read_half, WRITER, READER, T) \
  BENCH_OP(PROT_NAME, read_first, WRITER, READER, T)

// Define a list of protocols to benchmark for each struct
#define BENCH_T(T)                                                   \
  FOR_EACH_OP(Binary, T, BinaryProtocolWriter, BinaryProtocolReader) \
  FOR_EACH_OP(Compact, T, CompactProtocolWriter, CompactProtocolReader)

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
