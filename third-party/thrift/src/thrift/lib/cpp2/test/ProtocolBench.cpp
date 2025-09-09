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
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/test/Structs.h>

#include <glog/logging.h>
#include <folly/Benchmark.h>
#include <folly/BenchmarkUtil.h>
#include <folly/Optional.h>
#include <folly/init/Init.h>
#include <folly/portability/GFlags.h>

using namespace apache::thrift;
using namespace thrift::benchmark;

template <class>
struct SerializerTraits;
template <class ReaderType, class WriterType>
struct SerializerTraits<Serializer<ReaderType, WriterType>> {
  using Reader = ReaderType;
  using Writer = WriterType;
};

template <class T>
using GetReader = typename SerializerTraits<T>::Reader;
template <class T>
using GetWriter = typename SerializerTraits<T>::Writer;

struct FrozenSerializer {
  template <class T>
  static void serialize(const T& obj, folly::IOBufQueue* out) {
    out->append(folly::IOBuf::fromString(frozen::freezeToString(obj)));
  }
  template <class T>
  static size_t deserialize(folly::IOBuf* iobuf, T& t) {
    auto view = frozen::mapFrozen<T>(iobuf->coalesce());
    t = view.thaw();
    return 0;
  }
};

enum class SerializerMethod {
  Codegen,
  Object,
};

// The benckmark is to measure single struct use case, the iteration here is
// more like a benchmark artifact, so avoid doing optimizationon iteration
// usecase in this benchmark (e.g. move string definition out of while loop)

template <
    SerializerMethod kSerializerMethod,
    typename Serializer,
    typename Struct,
    typename Counter>
void writeBench(size_t iters, Counter&&) {
  folly::BenchmarkSuspender susp;
  auto strct = create<Struct>();
  protocol::Object obj;
  if constexpr (kSerializerMethod == SerializerMethod::Object) {
    folly::IOBufQueue q;
    Serializer::serialize(strct, &q);
    obj = protocol::parseObject<GetReader<Serializer>>(*q.move());
  }
  susp.dismiss();

  folly::IOBufQueue q;
  while (iters--) {
    if constexpr (kSerializerMethod == SerializerMethod::Object) {
      protocol::serializeObject<GetWriter<Serializer>>(obj, q);
      folly::doNotOptimizeAway(q);
    } else {
      Serializer::serialize(strct, &q);
      folly::doNotOptimizeAway(q);
    }

    // Reuse the queue across iterations to avoid allocating a new buffer for
    // each struct (which would dominate the measurement), but keep only the
    // tail to avoid unbounded growth.
    if (auto head = q.front(); head && head->isChained()) {
      q.append(q.move()->prev()->unlink());
    }
  }
  susp.rehire();
}

template <
    SerializerMethod kSerializerMethod,
    typename Serializer,
    typename Struct,
    typename Counter>
void readBench(size_t iters, Counter&& counter) {
  folly::BenchmarkSuspender susp;
  auto strct = create<Struct>();
  folly::IOBufQueue q;
  Serializer::serialize(strct, &q);
  auto buf = q.move();
  // coalesce the IOBuf chain to test fast path
  buf->coalesce();
  susp.dismiss();

  while (iters--) {
    if constexpr (kSerializerMethod == SerializerMethod::Object) {
      auto obj = protocol::parseObject<GetReader<Serializer>>(*buf);
      folly::doNotOptimizeAway(obj);
    } else {
      Struct data;
      Serializer::deserialize(buf.get(), data);
      folly::doNotOptimizeAway(data);
    }
  }
  susp.rehire();
  counter["serialized_size"] = buf->computeChainDataLength();
}

constexpr SerializerMethod getSerializerMethod(std::string_view prefix) {
  return prefix == "" || prefix == "OpEncode" ? SerializerMethod::Codegen
      : prefix == "Object"
      ? SerializerMethod::Object
      : throw std::invalid_argument(std::string(prefix) + " is invalid");
}

#define X1(Prefix, proto, rdwr, bench)                                   \
  BENCHMARK_COUNTERS(                                                    \
      Prefix##proto##Protocol_##rdwr##_##bench, counter, iters) {        \
    rdwr##Bench<getSerializerMethod(#Prefix), proto##Serializer, bench>( \
        iters, counter);                                                 \
  }

#define OpEncodeX1(Prefix, proto, rdwr, bench)                               \
  BENCHMARK_COUNTERS(                                                        \
      Prefix##proto##Protocol_##rdwr##_##bench, counter, iters) {            \
    rdwr##Bench<getSerializerMethod(#Prefix), proto##Serializer, Op##bench>( \
        iters, counter);                                                     \
  }

// clang-format off
#define X2(Prefix, proto, bench)  \
  X1(Prefix, proto, write, bench) \
  X1(Prefix, proto, read, bench)

#define OpEncodeX2(Prefix, proto, bench)  \
  OpEncodeX1(Prefix, proto, write, bench) \
  OpEncodeX1(Prefix, proto, read, bench)

#define X(Prefix, proto)                \
  X2(Prefix, proto, Empty)              \
  X2(Prefix, proto, SmallInt)           \
  X2(Prefix, proto, BigInt)             \
  X2(Prefix, proto, SmallString)        \
  X2(Prefix, proto, BigString)          \
  X2(Prefix, proto, BigBinary)          \
  X2(Prefix, proto, LargeBinary)        \
  X2(Prefix, proto, Mixed)              \
  X2(Prefix, proto, MixedInt)           \
  X2(Prefix, proto, LargeMixed)         \
  X2(Prefix, proto, SmallListInt)       \
  X2(Prefix, proto, BigListByte)        \
  X2(Prefix, proto, BigListShort)       \
  X2(Prefix, proto, BigListInt)         \
  X2(Prefix, proto, BigListBigInt)      \
  X2(Prefix, proto, BigListFloat)       \
  X2(Prefix, proto, BigListDouble)      \
  X2(Prefix, proto, BigListMixed)       \
  X2(Prefix, proto, BigListMixedInt)    \
  X2(Prefix, proto, LargeListMixed)     \
  X2(Prefix, proto, LargeSetInt)        \
  X2(Prefix, proto, UnorderedSetInt)    \
  X2(Prefix, proto, SortedVecSetInt)    \
  X2(Prefix, proto, LargeMapInt)        \
  X2(Prefix, proto, LargeMapMixed)      \
  X2(Prefix, proto, LargeUnorderedMapMixed)        \
  X2(Prefix, proto, LargeSortedVecMapMixed)        \
  X2(Prefix, proto, UnorderedMapInt)    \
  X2(Prefix, proto, NestedMap)          \
  X2(Prefix, proto, SortedVecNestedMap) \
  X2(Prefix, proto, ComplexStruct)

#define OpEncodeX(Prefix, proto)                \
  OpEncodeX2(Prefix, proto, Empty)              \
  OpEncodeX2(Prefix, proto, SmallInt)           \
  OpEncodeX2(Prefix, proto, BigInt)             \
  OpEncodeX2(Prefix, proto, SmallString)        \
  OpEncodeX2(Prefix, proto, BigString)          \
  OpEncodeX2(Prefix, proto, Mixed)              \
  OpEncodeX2(Prefix, proto, MixedInt)           \
  OpEncodeX2(Prefix, proto, LargeMixed)         \
  OpEncodeX2(Prefix, proto, SmallListInt)       \
  OpEncodeX2(Prefix, proto, BigListInt)         \
  OpEncodeX2(Prefix, proto, BigListMixed)       \
  OpEncodeX2(Prefix, proto, BigListMixedInt)    \
  OpEncodeX2(Prefix, proto, LargeListMixed)     \
  OpEncodeX2(Prefix, proto, LargeSetInt)        \
  OpEncodeX2(Prefix, proto, UnorderedSetInt)    \
  OpEncodeX2(Prefix, proto, SortedVecSetInt)    \
  OpEncodeX2(Prefix, proto, LargeMapInt)        \
  OpEncodeX2(Prefix, proto, LargeMapMixed)      \
  OpEncodeX2(Prefix, proto, LargeUnorderedMapMixed)        \
  OpEncodeX2(Prefix, proto, LargeSortedVecMapMixed)        \
  OpEncodeX2(Prefix, proto, UnorderedMapInt)    \
  OpEncodeX2(Prefix, proto, NestedMap)          \
  OpEncodeX2(Prefix, proto, SortedVecNestedMap) \
  OpEncodeX2(Prefix, proto, ComplexStruct)

X(, Binary)
X(, Compact)
X(, SimpleJSON)
X(, JSON)
X(, Frozen)
X(Object, Binary)
X(Object, Compact)
OpEncodeX(OpEncode, Binary)
OpEncodeX(OpEncode, Compact)

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
// clang-format on
