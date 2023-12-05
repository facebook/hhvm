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

#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/ProtocolReaderStructReadState.h>
#include <thrift/lib/cpp2/protocol/detail/protocol_methods.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::detail::pm;

namespace {

template <class ProtocolWriter>
std::unique_ptr<folly::IOBuf> createTestInput() {
  folly::IOBufQueue queue;
  ProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeStructBegin("");
  writer.writeFieldBegin("", T_BYTE, 13);
  writer.writeByte(24);
  writer.writeFieldEnd();
  writer.writeFieldBegin("", T_BYTE, 23);
  writer.writeByte(123);
  writer.writeFieldEnd();
  // In compact, this would be encoded as a delta in CompactProtocol
  writer.writeFieldBegin("", T_I64, 38);
  writer.writeI64(123456789);
  writer.writeFieldEnd();
  // In compact, this would be encoded as a delta + bool in one byte.
  writer.writeFieldBegin("", T_BOOL, 39);
  writer.writeBool(true);
  writer.writeFieldEnd();
  // In compact, this would be encoded as a type header + 1 byte for field.
  writer.writeFieldBegin("", T_BOOL, 63);
  writer.writeBool(false);
  writer.writeFieldEnd();
  // In compact, this would be encoded as a type header + 2 byte for field.
  writer.writeFieldBegin("", T_I32, -8192);
  writer.writeI32(5678910);
  writer.writeFieldEnd();
  // In compact, this would be encoded as a type header + 3 byte for field.
  writer.writeFieldBegin("", T_I16, 16381);
  writer.writeI16(12345);
  writer.writeFieldEnd();

  writer.writeFieldStop();
  writer.writeStructEnd();
  return queue.move();
}

template <typename Protocol, typename State, typename T>
void readFieldContents(Protocol& protocol, State& state, T& out) {
  protocol_methods<type_class::integral, T>::readWithContext(
      protocol, out, state);
}

template <class ProtocolReader>
void testAdvanceToNextFieldSuccess() {
  auto input = createTestInput<typename ProtocolReader::ProtocolWriter>();

  typename ProtocolReader::StructReadState state;
  ProtocolReader reader;
  reader.setInput(input.get());
  state.readStructBegin(&reader);
  EXPECT_TRUE(state.advanceToNextField(&reader, 0, 13, T_BYTE));
  {
    int8_t value;
    readFieldContents(reader, state, value);
    EXPECT_EQ(value, 24);
  }
  EXPECT_TRUE(state.advanceToNextField(&reader, 13, 23, T_BYTE));
  {
    int8_t value;
    readFieldContents(reader, state, value);
    EXPECT_EQ(value, 123);
  }
  EXPECT_TRUE(state.advanceToNextField(&reader, 23, 38, T_I64));
  {
    int64_t value;
    readFieldContents(reader, state, value);
    EXPECT_EQ(value, 123456789);
  }
  EXPECT_TRUE(state.advanceToNextField(&reader, 38, 39, T_BOOL));
  {
    bool value;
    readFieldContents(reader, state, value);
    EXPECT_EQ(value, true);
  }
  EXPECT_TRUE(state.advanceToNextField(&reader, 39, 63, T_BOOL));
  {
    bool value;
    readFieldContents(reader, state, value);
    EXPECT_EQ(value, false);
  }
  EXPECT_TRUE(state.advanceToNextField(&reader, 63, -8192, T_I32));
  {
    int32_t value;
    readFieldContents(reader, state, value);
    EXPECT_EQ(value, 5678910);
  }
  EXPECT_TRUE(state.advanceToNextField(&reader, -8192, 16381, T_I16));
  {
    int16_t value;
    readFieldContents(reader, state, value);
    EXPECT_EQ(value, 12345);
  }
  EXPECT_TRUE(state.advanceToNextField(&reader, 16381, 0, T_STOP));
  state.readStructEnd(&reader);
  EXPECT_TRUE(reader.getCursor().isAtEnd());
}

template <class ProtocolReader>
void testAdvanceToNextFieldFail() {
  auto input = createTestInput<typename ProtocolReader::ProtocolWriter>();

  typename ProtocolReader::StructReadState state;
  ProtocolReader reader;
  reader.setInput(input.get());
  state.readStructBegin(&reader);
  // Test missmatching type.
  EXPECT_FALSE(state.advanceToNextField(&reader, 0, 13, T_I32));
  // We failed to transition, state should reflect what was in the buffer.
  EXPECT_EQ(state.fieldId, 13);
  EXPECT_EQ(state.fieldType, T_BYTE);
  reader.skip(T_BYTE);
  // We should still be able to resume.
  EXPECT_TRUE(state.advanceToNextField(&reader, 13, 23, T_BYTE));
  {
    int8_t value;
    readFieldContents(reader, state, value);
    EXPECT_EQ(value, 123);
  }
  // Test mismatched field id.
  EXPECT_FALSE(state.advanceToNextField(&reader, 23, 37, T_I64));
  EXPECT_EQ(state.fieldId, 38);
  EXPECT_EQ(state.fieldType, T_I64);
  reader.skip(T_I64);

  EXPECT_FALSE(state.advanceToNextField(&reader, 38, 40, T_I32));
  EXPECT_EQ(state.fieldId, 39);
  EXPECT_EQ(state.fieldType, T_BOOL);
  reader.skip(T_BOOL);

  EXPECT_FALSE(state.advanceToNextField(&reader, 40, 65, T_BOOL));
  EXPECT_EQ(state.fieldId, 63);
  EXPECT_EQ(state.fieldType, T_BOOL);
  reader.skip(T_BOOL);

  // Fail to find T_STOP.
  EXPECT_FALSE(state.advanceToNextField(&reader, 63, 0, T_STOP));
  EXPECT_EQ(state.fieldId, -8192);
  EXPECT_EQ(state.fieldType, T_I32);
  reader.skip(T_I32);

  EXPECT_FALSE(state.advanceToNextField(&reader, -8192, 16380, T_STRING));
  EXPECT_EQ(state.fieldId, 16381);
  EXPECT_EQ(state.fieldType, T_I16);
  reader.skip(T_I16);

  // No such field in the buffer.
  EXPECT_FALSE(state.advanceToNextField(&reader, 16381, 123, T_STRING));
  EXPECT_EQ(state.fieldType, T_STOP);
  state.readStructEnd(&reader);
  EXPECT_TRUE(reader.getCursor().isAtEnd());
}

TEST(BinaryProtocol, advanceToNextFieldSuccess) {
  testAdvanceToNextFieldSuccess<BinaryProtocolReader>();
}

TEST(CompactProtocol, advanceToNextFieldSuccess) {
  testAdvanceToNextFieldSuccess<CompactProtocolReader>();
}

TEST(BinaryProtocol, advanceToNextFieldFail) {
  testAdvanceToNextFieldSuccess<BinaryProtocolReader>();
}

TEST(CompactProtocol, advanceToNextFieldFail) {
  testAdvanceToNextFieldSuccess<CompactProtocolReader>();
}

} // anonymous namespace
