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

#pragma once

#include <cstdint>

#include <folly/Benchmark.h>
#include <folly/BenchmarkUtil.h>

#include <thrift/lib/cpp2/protocol/NativeObject.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/test/Structs.h>

namespace apache::thrift::test::utils {

namespace experimental = ::apache::thrift::protocol::experimental;

using NativeObject = ::apache::thrift::protocol::experimental::NativeObject;

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

struct TestingObject {
  std::unique_ptr<folly::IOBuf> buf;
  protocol::Object obj;
  experimental::NativeObject native_obj;

  template <typename T, typename ProtocolWriter, typename ProtocolReader>
  static TestingObject make() {
    folly::BenchmarkSuspender suspender{};
    T value = create<T>();
    std::unique_ptr<folly::IOBuf> buf = serialize<ProtocolWriter>(value);
    protocol::Object obj = protocol::parseObject<ProtocolReader>(*buf);
    experimental::NativeObject native_obj =
        experimental::parseObject<ProtocolReader>(*buf);
    return TestingObject{std::move(buf), std::move(obj), std::move(native_obj)};
  }

 private:
  TestingObject(
      std::unique_ptr<folly::IOBuf>&& buf,
      protocol::Object&& obj,
      NativeObject&& native_obj)
      : buf{std::move(buf)},
        obj{std::move(obj)},
        native_obj{std::move(native_obj)} {}
};

// Constructs a serialized buffer for the given type encoded with a given
// Protocol(Writer)
template <typename T, typename ProtocolWriter, typename ProtocolReader>
const TestingObject& get_serde();

// ----- Access 'all' data within a thrift hierarchy ---- //

std::size_t read_all(const bool& b);
std::size_t read_all(const std::int8_t& i);
std::size_t read_all(const std::int16_t& i);
std::size_t read_all(const std::int32_t& i);
std::size_t read_all(const std::int64_t& i);
std::size_t read_all(const float& f);
std::size_t read_all(const double& d);
std::size_t read_all(const std::string& s);
std::size_t read_all(const folly::IOBuf& b);
std::size_t read_all(const std::vector<protocol::detail::Value>& l);
std::size_t read_all(const folly::F14FastSet<protocol::detail::Value>& s);
std::size_t read_all(
    const folly::F14FastMap<protocol::detail::Value, protocol::detail::Value>&
        m);
std::size_t read_all(const ::apache::thrift::protocol::Object& obj);
std::size_t read_all(const ::apache::thrift::protocol::detail::Value& val);

std::size_t read_all(const experimental::Bytes& s);
std::size_t read_all(const std::monostate&);
std::size_t read_all(const NativeObject& obj);
std::size_t read_all(const experimental::NativeList& l);
std::size_t read_all(const experimental::NativeMap& m);
std::size_t read_all(const experimental::NativeSet& s);
std::size_t read_all(const experimental::NativeObject& s);
std::size_t read_all(const experimental::ValueHolder& s);

// ----- Benchmark operations ----- //

inline folly::IOBufQueue& get_queue() {
  static folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
  return queue;
}

template <typename ProtocolWriter, typename ProtocolReader>
inline void decode(const TestingObject& input) {
  const auto obj = protocol::parseObject<ProtocolReader>(*input.buf);
  folly::doNotOptimizeAway(obj);
  BENCHMARK_SUSPEND {
    std::destroy_at(&obj);
  }
}

template <typename ProtocolWriter, typename ProtocolReader>
inline void encode(const TestingObject& input) {
  protocol::serializeObject<ProtocolWriter>(input.obj, get_queue());
  BENCHMARK_SUSPEND {
    get_queue().clearAndTryReuseLargestBuffer();
  }
}

template <typename ProtocolWriter, typename ProtocolReader>
inline void roundtrip(const TestingObject& input) {
  auto obj = protocol::parseObject<ProtocolReader>(*input.buf);
  folly::doNotOptimizeAway(obj);
  protocol::serializeObject<ProtocolWriter>(obj, get_queue());
  BENCHMARK_SUSPEND {
    get_queue().clearAndTryReuseLargestBuffer();
  }
  // Include Object dtor in timing
}

template <typename ProtocolWriter, typename ProtocolReader>
inline void read_all(const TestingObject& input) {
  const auto val = read_all(input.obj);
  folly::doNotOptimizeAway(val);
}

template <typename ProtocolWriter, typename ProtocolReader>
void decode_native(const TestingObject& input) {
  auto obj = experimental::parseObject<ProtocolReader>(*input.buf);
  folly::doNotOptimizeAway(obj);
  BENCHMARK_SUSPEND {
    std::destroy_at(&obj);
  }
}

template <typename ProtocolWriter, typename ProtocolReader>
void read_all_native(const TestingObject& input) {
  const auto res = read_all(input.native_obj);
  folly::doNotOptimizeAway(res);
}

template <typename ProtocolWriter, typename ProtocolReader>
void encode_native(const TestingObject& input) {
  auto& queue = get_queue();
  experimental::serializeObject<ProtocolWriter>(input.native_obj, queue);
  BENCHMARK_SUSPEND {
    queue.clearAndTryReuseLargestBuffer();
  }
}

template <typename ProtocolWriter, typename ProtocolReader>
void roundtrip_native(const TestingObject& input) {
  auto& queue = get_queue();
  auto obj = protocol::parseObject<ProtocolReader>(*input.buf);
  folly::doNotOptimizeAway(obj);
  experimental::serializeObject<ProtocolWriter>(input.native_obj, queue);
  BENCHMARK_SUSPEND {
    queue.clearAndTryReuseLargestBuffer();
  }
  // Include Object dtor in timing
}

} // namespace apache::thrift::test::utils

#ifndef FBTHRIFT_DO_NOTHING_HANDLER
#define FBTHRIFT_DO_NOTHING_HANDLER(...)
#endif

// Define a list of operations to benchmark for each struct + protocol
// + invoke a callback handler for each struct + protocol
#ifndef FBTHRIFT_FOR_EACH_OP
#define FBTHRIFT_FOR_EACH_OP(                               \
    HANDLER, EACH_TY_HANDLER, PROT_NAME, T, WRITER, READER) \
  EACH_TY_HANDLER(T, WRITER, READER)                        \
  HANDLER(PROT_NAME, decode, WRITER, READER, T)             \
  HANDLER(PROT_NAME, encode, WRITER, READER, T)             \
  HANDLER(PROT_NAME, roundtrip, WRITER, READER, T)          \
  HANDLER(PROT_NAME, read_all, WRITER, READER, T)
#endif

// Define a list of protocols to benchmark for each struct
#ifndef FBTHRIFT_FOR_EACH_PROT
#define FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, T) \
  FBTHRIFT_FOR_EACH_OP(                                     \
      HANDLER,                                              \
      EACH_TY_HANDLER,                                      \
      Binary,                                               \
      T,                                                    \
      ::apache::thrift::BinaryProtocolWriter,               \
      ::apache::thrift::BinaryProtocolReader)               \
  FBTHRIFT_FOR_EACH_OP(                                     \
      HANDLER,                                              \
      EACH_TY_HANDLER,                                      \
      Compact,                                              \
      T,                                                    \
      ::apache::thrift::CompactProtocolWriter,              \
      ::apache::thrift::CompactProtocolReader)
#endif

#define FBTHRIFT_FOR_EACH_UNIQUE_TYPE(HANDLER) \
  HANDLER(Empty)                               \
  HANDLER(SmallInt)                            \
  HANDLER(BigInt)                              \
  HANDLER(SmallString)                         \
  HANDLER(BigString)                           \
  HANDLER(Mixed)                               \
  HANDLER(MixedInt)                            \
  HANDLER(LargeMixed)                          \
  HANDLER(SmallListInt)                        \
  HANDLER(BigListInt)                          \
  HANDLER(BigListMixed)                        \
  HANDLER(BigListMixedInt)                     \
  HANDLER(LargeListMixed)                      \
  HANDLER(LargeSetInt)                         \
  HANDLER(UnorderedSetInt)                     \
  HANDLER(SortedVecSetInt)                     \
  HANDLER(LargeMapInt)                         \
  HANDLER(LargeMapMixed)                       \
  HANDLER(LargeUnorderedMapMixed)              \
  HANDLER(LargeSortedVecMapMixed)              \
  HANDLER(UnorderedMapInt)                     \
  HANDLER(NestedMap)                           \
  HANDLER(SortedVecNestedMap)                  \
  HANDLER(ComplexStruct)

#define FBTHRIFT_FOR_EACH_UNIQUE_OPERATION(HANDLER) \
  HANDLER(decode)                                   \
  HANDLER(encode)                                   \
  HANDLER(roundtrip)                                \
  HANDLER(read_all)

#define FBTHRIFT_FOR_EACH_UNIQUE_PROTOCOL(HANDLER)            \
  HANDLER(Binary, BinaryProtocolWriter, BinaryProtocolReader) \
  HANDLER(Compact, CompactProtocolWriter, CompactProtocolReader)

// Define benchmarks for each struct
#ifndef FBTHRIFT_FOR_EACH_TYPE
#define FBTHRIFT_FOR_EACH_TYPE(HANDLER, EACH_TY_HANDLER)                   \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, Empty)                  \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, SmallInt)               \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, BigInt)                 \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, SmallString)            \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, BigString)              \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, Mixed)                  \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, MixedInt)               \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, LargeMixed)             \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, SmallListInt)           \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, BigListInt)             \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, BigListMixed)           \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, BigListMixedInt)        \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, LargeListMixed)         \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, LargeSetInt)            \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, UnorderedSetInt)        \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, SortedVecSetInt)        \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, LargeMapInt)            \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, LargeMapMixed)          \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, LargeUnorderedMapMixed) \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, LargeSortedVecMapMixed) \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, UnorderedMapInt)        \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, NestedMap)              \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, SortedVecNestedMap)     \
  FBTHRIFT_FOR_EACH_PROT(HANDLER, EACH_TY_HANDLER, ComplexStruct)
#endif // FBTHRIFT_FOR_EACH_TYPE

#ifndef FBTHRIFT_GEN_PROTOCOL_BENCHMARKS
#define FBTHRIFT_GEN_PROTOCOL_BENCHMARKS(F) \
  FBTHRIFT_FOR_EACH_TYPE(F, FBTHRIFT_DO_NOTHING_HANDLER)
#endif

// Define prototype serialization & deserialization functions for each struct &
// protocol
#ifndef FBTHRIFT_DEFINE_SERDE
#define FBTHRIFT_DEFINE_SERDE(T, WRITER, READER)                               \
  template <>                                                                  \
  const ::apache::thrift::test::utils::TestingObject& ::apache::thrift::test:: \
      utils::get_serde<T, WRITER, READER>() {                                  \
    static ::apache::thrift::test::utils::TestingObject obj = ::apache::       \
        thrift::test::utils::TestingObject::make<T, WRITER, READER>();         \
    return obj;                                                                \
  }
#endif

#define FBTHRIFT_GEN_SERDE(...) \
  FBTHRIFT_FOR_EACH_TYPE(FBTHRIFT_DO_NOTHING_HANDLER, FBTHRIFT_DEFINE_SERDE)
