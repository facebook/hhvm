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

#include <memory>
#include <string>
#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/protocol/CursorBasedSerializer.h>
#include <thrift/lib/cpp2/protocol/benchmark/ProtobufTestData.pb.h>
#include <thrift/lib/cpp2/protocol/benchmark/gen-cpp2/ThriftTestData_types.h>
#include <thrift/lib/cpp2/protocol/benchmark/gen/CarbonTestDataMessages.h>

// Test data generation
template <typename TestData>
TestData createThriftTestData();

template <typename TestData>
TestData createProtobufTestData();

template <typename TestData>
TestData createCarbonTestData();

// Serialization
// Thrift
template <typename Protocol, typename TestData>
std::unique_ptr<folly::IOBuf> serializeThrift() {
  TestData testData = createThriftTestData<TestData>();

  folly::IOBufQueue queue;
  Protocol writer;
  writer.setOutput(&queue);
  testData.write(&writer);
  return queue.move();
}

template <typename TestData>
std::unique_ptr<folly::IOBuf> serializeThriftBinary() {
  return serializeThrift<apache::thrift::BinaryProtocolWriter, TestData>();
}

template <typename TestData>
std::unique_ptr<folly::IOBuf> serializeThriftCompact() {
  return serializeThrift<apache::thrift::CompactProtocolWriter, TestData>();
}

// Cursor-based serialization implementations
template <typename TestData>
std::unique_ptr<folly::IOBuf> serializeThriftCurSe();

// Protobuf serialization using SerializeToArray
template <typename TestData>
std::unique_ptr<folly::IOBuf> serializeProtobufArray();

// Protobuf serialization using SerializeAsString
template <typename TestData>
std::unique_ptr<folly::IOBuf> serializeProtobufString();

// Protobuf serialization using ZeroCopyStream
template <typename TestData>
std::unique_ptr<folly::IOBuf> serializeProtobufZeroCopy();

// Protobuf serialization using ZeroCopyStream
template <typename TestData>
std::unique_ptr<folly::IOBuf> serializeProtobufZeroCopy();

template <typename TestData>
std::unique_ptr<folly::IOBuf> serializeCarbonDefault(
    carbon::CarbonQueueAppenderStorage& storage) {
  TestData testData = createCarbonTestData<TestData>();

  carbon::CarbonProtocolWriter writer(storage);
  serialize(testData, writer);

  auto [iovecs, size] = storage.getIovecs();

  // Wrap iovecs into an IOBuf chain
  return folly::IOBuf::wrapIov(iovecs, size);
}

// Deserialization
// Thrift
template <typename Protocol, typename TestData>
TestData deserializeThrift(const std::unique_ptr<folly::IOBuf>& serialized) {
  Protocol reader;
  reader.setInput(folly::io::Cursor(serialized.get()));

  TestData testData;
  testData.read(&reader);
  return testData;
}

template <typename TestData>
TestData deserializeThriftBinary(
    const std::unique_ptr<folly::IOBuf>& serialized) {
  return deserializeThrift<apache::thrift::BinaryProtocolReader, TestData>(
      serialized);
}

template <typename TestData>
TestData deserializeThriftCompact(
    const std::unique_ptr<folly::IOBuf>& serialized) {
  return deserializeThrift<apache::thrift::CompactProtocolReader, TestData>(
      serialized);
}

// Cursor-based deserialization
template <typename TestData>
void deserializeThriftCurSe(std::unique_ptr<folly::IOBuf> serialized);

// Protobuf deserialization using ParseFromArray
template <typename TestData>
TestData deserializeProtobufArray(std::unique_ptr<folly::IOBuf> serialized);

// Protobuf deserialization using ParseFromString
template <typename TestData>
TestData deserializeProtobufString(std::unique_ptr<folly::IOBuf> serialized);

// Protobuf deserialization using ZeroCopyInputStream
template <typename TestData>
TestData deserializeProtobufZeroCopy(std::unique_ptr<folly::IOBuf> serialized);

// Carbon deserialization
template <typename TestData>
TestData deserializeCarbonDefault(
    const std::unique_ptr<folly::IOBuf>& serialized) {
  carbon::CarbonProtocolReader reader{carbon::CarbonCursor(serialized.get())};

  TestData testData;
  deserialize(testData, reader);
  return testData;
}
