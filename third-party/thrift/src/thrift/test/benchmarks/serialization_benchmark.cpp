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

#include <cstdint>
#include <map>
#include <vector>
#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <folly/io/Cursor.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/test/benchmarks/gen-cpp2/serialization_benchmark_types.h>
#include <thrift/test/benchmarks/sbe_generated_flyweights/GroupSizeEncoding.h>
#include <thrift/test/benchmarks/sbe_generated_flyweights/MessageHeader.h>
#include <thrift/test/benchmarks/sbe_generated_flyweights/SBEStruct1.h>
#include <thrift/test/benchmarks/sbe_generated_flyweights/VarStringEncoding.h>
#include <thrift/test/benchmarks/serialization_benchmark_generated.h>

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

struct RawStruct {
  int32_t field_1;
  std::string field_2;
  std::vector<std::string> field_3;
  std::map<std::string, int32_t> field_4;
};

BENCHMARK(RawStructCreationBaseline) {
  RawStruct s;
  s.field_1 = 1;
  s.field_2 = "hello, world";
  s.field_3.reserve(kDefaultListSize);
  for (int i = 0; i < kDefaultListSize; ++i) {
    s.field_3.push_back("item " + std::to_string(i));
  }
  for (int i = 0; i < kDefaultMapSize; ++i) {
    s.field_4["key " + std::to_string(i)] = i;
  }
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

BENCHMARK(FlatBuffersSerialization) {
  // Create a FlatBufferBuilder object
  flatbuffers::FlatBufferBuilder builder(2048);

  // Create a StringToUByteMap objects
  std::vector<flatbuffers::Offset<StringToUByteMap>> field4;
  for (int i = 0; i < 10; i++) {
    auto key = builder.CreateString("key" + std::to_string(i));
    auto value = static_cast<uint8_t>(i);
    auto stringToUByteMap = CreateStringToUByteMap(builder, key, value);
    field4.push_back(stringToUByteMap);
  }

  // Create a FBStruct1 object
  std::vector<flatbuffers::Offset<flatbuffers::String>> field3;
  for (int i = 0; i < 10; i++) {
    auto field3Value = builder.CreateString("item " + std::to_string(i));
    field3.push_back(field3Value);
  }
  auto field3Vector = builder.CreateVector(field3);
  auto field4Vector = builder.CreateVector(field4);

  auto fbStruct1 = CreateFBStruct1(
      builder,
      1,
      builder.CreateString("hello, world"),
      field3Vector,
      field4Vector);

  // Finish the buffer and get a pointer to the root object
  builder.Finish(fbStruct1);
}

BENCHMARK(FlatBuffersDerialization) {
  flatbuffers::FlatBufferBuilder builder(2048);
  void* bufferPointer;
  BENCHMARK_SUSPEND {
    // Create a FlatBufferBuilder object

    // Create a StringToUByteMap objects
    std::vector<flatbuffers::Offset<StringToUByteMap>> field4;
    for (int i = 0; i < 10; i++) {
      auto key = builder.CreateString("key" + std::to_string(i));
      auto value = static_cast<uint8_t>(i);
      auto stringToUByteMap = CreateStringToUByteMap(builder, key, value);
      field4.push_back(stringToUByteMap);
    }

    // Create a FBStruct1 object
    std::vector<flatbuffers::Offset<flatbuffers::String>> field3;
    for (int i = 0; i < 10; i++) {
      auto field3Value = builder.CreateString("item " + std::to_string(i));
      field3.push_back(field3Value);
    }
    auto field3Vector = builder.CreateVector(field3);
    auto field4Vector = builder.CreateVector(field4);

    auto fbStruct1 = CreateFBStruct1(
        builder,
        1,
        builder.CreateString("hello, world"),
        field3Vector,
        field4Vector);

    // Finish the buffer and get a pointer to the root object
    builder.Finish(fbStruct1);
    bufferPointer = builder.GetBufferPointer();
  }

  // Deserialize the buffer
  const auto* fbStruct1Root = flatbuffers::GetRoot<FBStruct1>(bufferPointer);

  // Access the fields of the structs
  fbStruct1Root->field_1();
  fbStruct1Root->field_2()->str();
  // Access the elements of the field_3 vector
  for (flatbuffers::uoffset_t i = 0; i < fbStruct1Root->field_3()->size();
       i++) {
    fbStruct1Root->field_3()->Get(i)->str();
  }

  // Access the elements of the field_4 vector
  for (flatbuffers::uoffset_t i = 0; i < fbStruct1Root->field_4()->size();
       i++) {
    auto mapEntry = fbStruct1Root->field_4()->Get(i);
    mapEntry->key()->str();
    mapEntry->value();
  }
}

BENCHMARK(FlatBuffersRoundTrip) {
  // Create a FlatBufferBuilder object
  flatbuffers::FlatBufferBuilder builder(2048);

  // Create a StringToUByteMap objects
  std::vector<flatbuffers::Offset<StringToUByteMap>> field4;
  for (int i = 0; i < 10; i++) {
    auto key = builder.CreateString("key" + std::to_string(i));
    auto value = static_cast<uint8_t>(i);
    auto stringToUByteMap = CreateStringToUByteMap(builder, key, value);
    field4.push_back(stringToUByteMap);
  }

  // Create a FBStruct1 object
  std::vector<flatbuffers::Offset<flatbuffers::String>> field3;
  for (int i = 0; i < 10; i++) {
    auto field3Value = builder.CreateString("item " + std::to_string(i));
    field3.push_back(field3Value);
  }
  auto field3Vector = builder.CreateVector(field3);
  auto field4Vector = builder.CreateVector(field4);

  auto fbStruct1 = CreateFBStruct1(
      builder,
      1,
      builder.CreateString("hello, world"),
      field3Vector,
      field4Vector);

  // Finish the buffer and get a pointer to the root object
  builder.Finish(fbStruct1);
  auto bufferPointer = builder.GetBufferPointer();

  // Deserialize the buffer
  const auto* fbStruct1Root = flatbuffers::GetRoot<FBStruct1>(bufferPointer);

  // Access the fields of the structs
  fbStruct1Root->field_1();
  fbStruct1Root->field_2()->str();
  // Access the elements of the field_3 vector
  for (flatbuffers::uoffset_t i = 0; i < fbStruct1Root->field_3()->size();
       i++) {
    fbStruct1Root->field_3()->Get(i)->str();
  }

  // Access the elements of the field_4 vector
  for (flatbuffers::uoffset_t i = 0; i < fbStruct1Root->field_4()->size();
       i++) {
    auto mapEntry = fbStruct1Root->field_4()->Get(i);
    mapEntry->key()->str();
    mapEntry->value();
  }
}

BENCHMARK(SBESerializationBenchmark) {
  char buffer[1024] = {};
  auto field_2_str = "hello, world";
  SBEStruct1 struct1;
  struct1.wrapForEncode(buffer, 0, 1024)
      .field_1(1)
      .putField_2(field_2_str, sizeof(field_2_str));

  auto& field_3 = struct1.field_3();
  for (int i = 0; i < kDefaultListSize; i++) {
    field_3.putList_entry("item " + std::to_string(i));
  }

  auto& field_4 = struct1.field_4();
  for (int i = 0; i < kDefaultListSize; i++) {
    field_4.map_value(static_cast<uint8_t>(i));
    field_4.putMap_key("key" + std::to_string(i));
  }
}

BENCHMARK(SBEDeserializationBenchmark) {
  char buffer[1024] = {};
  BENCHMARK_SUSPEND {
    auto field_2_str = "hello, world";
    SBEStruct1 struct1;
    struct1.wrapForEncode(buffer, 0, 1024)
        .field_1(1)
        .putField_2(field_2_str, sizeof(field_2_str));

    auto& field_3 = struct1.field_3();
    for (int i = 0; i < kDefaultListSize; i++) {
      field_3.putList_entry("item " + std::to_string(i));
    }

    auto& field_4 = struct1.field_4();
    for (int i = 0; i < kDefaultListSize; i++) {
      field_4.map_value(static_cast<uint8_t>(i));
      field_4.putMap_key("key" + std::to_string(i));
    }
  }
  SBEStruct1 other;
  other.wrapForDecode(
      buffer,
      0,
      SBEStruct1::sbeBlockLength(),
      SBEStruct1::sbeSchemaVersion(),
      sizeof(buffer));

  (void)other.field_1();
  (void)other.field_2();

  auto& field_3 = other.field_3();
  while (field_3.hasNext()) {
    (void)field_3.next();
  }

  auto& field_4 = other.field_4();
  while (field_4.hasNext()) {
    (void)field_4.next();
  }
}

BENCHMARK(SBERoundTripBenchmark) {
  char buffer[1024] = {};
  {
    auto field_2_str = "hello, world";
    SBEStruct1 struct1;
    struct1.wrapForEncode(buffer, 0, 1024)
        .field_1(1)
        .putField_2(field_2_str, sizeof(field_2_str));

    auto& field_3 = struct1.field_3();
    for (int i = 0; i < kDefaultListSize; i++) {
      field_3.putList_entry("item " + std::to_string(i));
    }

    auto& field_4 = struct1.field_4();
    for (int i = 0; i < kDefaultListSize; i++) {
      field_4.map_value(static_cast<uint8_t>(i));
      field_4.putMap_key("key" + std::to_string(i));
    }
  }
  {
    SBEStruct1 other;
    other.wrapForDecode(
        buffer,
        0,
        SBEStruct1::sbeBlockLength(),
        SBEStruct1::sbeSchemaVersion(),
        sizeof(buffer));

    (void)other.field_1();
    (void)other.field_2();

    auto& field_3 = other.field_3();
    while (field_3.hasNext()) {
      (void)field_3.next();
    }

    auto& field_4 = other.field_4();
    while (field_4.hasNext()) {
      (void)field_4.next();
    }
  }
}

} // namespace apache::thrift::benchmarks

int main(int argc, char** argv) {
  folly::init(&argc, &argv);
  folly::runBenchmarks();
}
