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

#include <map>
#include <vector>
#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/Cursor.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/test/benchmarks/gen-cpp2/serialization_benchmark_types.h>

namespace apache::thrift::benchmarks {

constexpr int kDefaultListSize = 10;
constexpr int kDefaultMapSize = 10;

inline void populate_struct_baseline(
    Struct1& s,
    int list_size = kDefaultListSize,
    int map_size = kDefaultMapSize) {
  s.field_1() = 1;
  s.field_2() = "hello, world";

  for (int i = 0; i < list_size; ++i) {
    s.field_3()->push_back("item " + std::to_string(i));
  }

  for (int i = 0; i < map_size; ++i) {
    s.field_4()["key " + std::to_string(i)] = i;
  }
}

template <class T, class ProtocolWriter>
inline void serialize_with_protocol_writer(T& t, folly::IOBufQueue& queue) {
  ProtocolWriter writer;
  writer.setOutput(&queue);
  t.write(&writer);
}

template <class T, class ProtocolReader>
inline void deserialize_with_protocol_reader(T& t, folly::IOBufQueue& queue) {
  ProtocolReader reader;
  auto iobuf = queue.move();
  auto cursor = folly::io::Cursor(iobuf.get());
  reader.setInput(cursor);
  t.read(&reader);
}

BENCHMARK(TCompactSerializationBaseline) {
  folly::IOBufQueue queue;
  auto s = Struct1{};
  populate_struct_baseline(s);
  serialize_with_protocol_writer<Struct1, CompactProtocolWriter>(s, queue);
}

BENCHMARK(TCompactDeserializationBaseline) {
  folly::IOBufQueue queue;
  BENCHMARK_SUSPEND {
    auto s = Struct1{};
    populate_struct_baseline(s);
    serialize_with_protocol_writer<Struct1, CompactProtocolWriter>(s, queue);
  }

  auto other = Struct1{};
  deserialize_with_protocol_reader<Struct1, CompactProtocolReader>(
      other, queue);
}

BENCHMARK(TCompactRoundTripBaseline) {
  folly::IOBufQueue queue;
  auto s = Struct1{};
  populate_struct_baseline(s);
  serialize_with_protocol_writer<Struct1, CompactProtocolWriter>(s, queue);
  auto other = Struct1{};
  deserialize_with_protocol_reader<Struct1, CompactProtocolReader>(
      other, queue);
}

BENCHMARK(TBinarySerializationBaseline) {
  folly::IOBufQueue queue;
  auto s = Struct1{};
  populate_struct_baseline(s);
  serialize_with_protocol_writer<Struct1, BinaryProtocolWriter>(s, queue);
}

BENCHMARK(TBinaryDeserializationBaseline) {
  folly::IOBufQueue queue;
  BENCHMARK_SUSPEND {
    auto s = Struct1{};
    populate_struct_baseline(s);
    serialize_with_protocol_writer<Struct1, BinaryProtocolWriter>(s, queue);
  }

  auto other = Struct1{};
  deserialize_with_protocol_reader<Struct1, BinaryProtocolReader>(other, queue);
}

BENCHMARK(TBinaryRoundTripBaseline) {
  folly::IOBufQueue queue;
  auto s = Struct1{};
  populate_struct_baseline(s);
  serialize_with_protocol_writer<Struct1, BinaryProtocolWriter>(s, queue);
  auto other = Struct1{};
  deserialize_with_protocol_reader<Struct1, BinaryProtocolReader>(other, queue);
}

} // namespace apache::thrift::benchmarks

int main(int argc, char** argv) {
  folly::init(&argc, &argv);
  folly::runBenchmarks();
}
