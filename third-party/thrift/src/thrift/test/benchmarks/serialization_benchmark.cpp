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
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/benchmarks/gen-cpp2/serialization_benchmark_types.h>
#include <thrift/test/benchmarks/serialization_benchmark_generated.h>

namespace apache::thrift::benchmarks {

constexpr int kDefaultListSize = 10;
constexpr int kDefaultMapSize = 10;
constexpr size_t kMinAllocBytes = 2048;

Struct1 populate_struct_baseline() {
  Struct1 s;
  s.field_1() = 1;
  s.field_2() = "hello, world";

  for (int i = 0; i < kDefaultListSize; ++i) {
    s.field_3()->push_back("item " + std::to_string(i));
  }

  for (int i = 0; i < kDefaultMapSize; ++i) {
    s.field_4()["key " + std::to_string(i)] = i;
  }
  return s;
}

template <class ProtocolWriter, class T>
folly::IOBufQueue serialize_with_protocol_writer(T&& t) {
  folly::IOBufQueue queue;
  ProtocolWriter writer;
  BENCHMARK_SUSPEND {
    auto buf = folly::IOBuf::create(kMinAllocBytes);
    queue.append(std::move(buf));
  }
  writer.setOutput(&queue);
  std::forward<T>(t).write(&writer);
  return queue;
}

template <class T, class ProtocolReader>
T deserialize_with_protocol_reader(const std::unique_ptr<folly::IOBuf>& iobuf) {
  T t;
  ProtocolReader reader;
  reader.setInput(folly::io::Cursor(iobuf.get()));
  t.read(&reader);
  return t;
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

template <class ProtocolWriter>
folly::IOBufQueue populate_and_serialize() {
  return serialize_with_protocol_writer<ProtocolWriter>(
      populate_struct_baseline());
}

template <class ProtocolWriter>
folly::IOBufQueue populate_wrapper_and_serialize() {
  auto queue = populate_and_serialize<ProtocolWriter>();
  Wrapper wrapper;
  wrapper.data() = queue.move();
  return serialize_with_protocol_writer<ProtocolWriter>(wrapper);
}

BENCHMARK(TCompactSerializationBaseline) {
  populate_and_serialize<CompactProtocolWriter>();
}

BENCHMARK_RELATIVE(TCompactSerializationWithBinaryField) {
  populate_wrapper_and_serialize<CompactProtocolWriter>();
}

BENCHMARK(TCompactDeserializationBaseline) {
  folly::BenchmarkSuspender susp;
  auto queue = populate_and_serialize<CompactProtocolWriter>();
  susp.dismiss();
  deserialize_with_protocol_reader<Struct1, CompactProtocolReader>(
      queue.move());
}

BENCHMARK_RELATIVE(TCompactDeserializationWithBinaryField) {
  folly::BenchmarkSuspender susp;
  auto queue = populate_wrapper_and_serialize<CompactProtocolWriter>();
  susp.dismiss();
  auto wrapper =
      deserialize_with_protocol_reader<Wrapper, CompactProtocolReader>(
          queue.move());
  deserialize_with_protocol_reader<Struct1, CompactProtocolReader>(
      *wrapper.data());
}

BENCHMARK(TCompactRoundTripBaseline) {
  auto queue = populate_and_serialize<CompactProtocolWriter>();
  deserialize_with_protocol_reader<Struct1, CompactProtocolReader>(
      queue.move());
}

BENCHMARK_RELATIVE(TCompactRoundTripWithBinaryField) {
  auto queue = populate_wrapper_and_serialize<CompactProtocolWriter>();
  auto wrapper =
      deserialize_with_protocol_reader<Wrapper, CompactProtocolReader>(
          queue.move());
  deserialize_with_protocol_reader<Struct1, CompactProtocolReader>(
      *wrapper.data());
}

BENCHMARK(TBinarySerializationBaseline) {
  populate_and_serialize<BinaryProtocolWriter>();
}

BENCHMARK_RELATIVE(TBinarySerializationWithBinaryField) {
  populate_wrapper_and_serialize<BinaryProtocolWriter>();
}

BENCHMARK(TBinaryDeserializationBaseline) {
  folly::BenchmarkSuspender susp;
  auto queue = populate_and_serialize<BinaryProtocolWriter>();
  susp.dismiss();
  deserialize_with_protocol_reader<Struct1, BinaryProtocolReader>(queue.move());
}

BENCHMARK_RELATIVE(TBinaryDeserializationWithBinaryField) {
  folly::BenchmarkSuspender susp;
  auto queue = populate_wrapper_and_serialize<BinaryProtocolWriter>();
  susp.dismiss();
  auto wrapper =
      deserialize_with_protocol_reader<Wrapper, BinaryProtocolReader>(
          queue.move());
  deserialize_with_protocol_reader<Struct1, BinaryProtocolReader>(
      *wrapper.data());
}

BENCHMARK(TBinaryRoundTripBaseline) {
  auto queue = populate_and_serialize<BinaryProtocolWriter>();
  deserialize_with_protocol_reader<Struct1, BinaryProtocolReader>(queue.move());
}

BENCHMARK_RELATIVE(TBinaryRoundTripWithBinaryField) {
  auto queue = populate_wrapper_and_serialize<BinaryProtocolWriter>();
  auto wrapper =
      deserialize_with_protocol_reader<Wrapper, BinaryProtocolReader>(
          queue.move());
  deserialize_with_protocol_reader<Struct1, BinaryProtocolReader>(
      *wrapper.data());
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

} // namespace apache::thrift::benchmarks

int main(int argc, char** argv) {
  const folly::Init init(&argc, &argv);
  folly::runBenchmarks();
}
