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
#include <thrift/lib/cpp2/protocol/ProtocolReaderStructReadState.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

namespace apache::thrift::test {
namespace {

TEST(ProtocolSkipTest, SkipInt32) {
  IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeI32(INT_MAX);
  auto buf = queue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  reader.skip(TType::T_I32);
}

TEST(ProtocolSkipTest, SkipInt8) {
  IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeByte(CHAR_MAX);
  auto buf = queue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  reader.skip(TType::T_I08);
}

TEST(ProtocolSkipTest, SkipInt64) {
  IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeI64(LLONG_MAX);
  auto buf = queue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  reader.skip(TType::T_I64);
}

TEST(ProtocolSkipTest, SkipUInt64) {
  IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeI64(ULLONG_MAX);
  auto buf = queue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  reader.skip(TType::T_U64);
}

TEST(ProtocolSkipTest, SkipString) {
  IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeString("test");
  auto buf = queue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  reader.skip(TType::T_STRING);
}

TEST(ProtocolSkipTest, SkipUTF8) {
  IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeString("test");
  auto buf = queue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  reader.skip(TType::T_UTF8);
}

TEST(ProtocolSkipTest, SkipUTF16) {
  IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeString("test");
  auto buf = queue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  reader.skip(TType::T_UTF16);
}

TEST(ProtocolSkipTest, SkipStream) {
  IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeFieldStop();
  auto buf = queue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  bool thrown = false;
  try {
    reader.skip(TType::T_STREAM);
  } catch (const TProtocolException& ex) {
    EXPECT_EQ(TProtocolException::INVALID_DATA, ex.getType());
    thrown = true;
  }
  EXPECT_TRUE(thrown);
}

TEST(ProtocolSkipTest, SkipVoid) {
  IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeFieldStop();
  auto buf = queue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  bool thrown = false;
  try {
    reader.skip(TType::T_VOID);
  } catch (const TProtocolException& ex) {
    EXPECT_EQ(TProtocolException::INVALID_DATA, ex.getType());
    thrown = true;
  }
  EXPECT_TRUE(thrown);
}

TEST(ProtocolSkipTest, SkipStop) {
  IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeFieldStop();
  auto buf = queue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  bool thrown = false;
  try {
    reader.skip(TType::T_STOP);
  } catch (const TProtocolException& ex) {
    EXPECT_EQ(TProtocolException::INVALID_DATA, ex.getType());
    thrown = true;
  }
  EXPECT_TRUE(thrown);
}

TEST(ProtocolSkipTest, SkipStopInContainer) {
  IOBufQueue queue;
  CompactProtocolWriter writer;
  writer.setOutput(&queue);
  writer.writeListBegin(TType::T_STOP, 1u << 30);
  auto buf = queue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());
  bool thrown = false;
  try {
    reader.skip(TType::T_LIST);
  } catch (const TProtocolException& ex) {
    EXPECT_EQ(TProtocolException::INVALID_DATA, ex.getType());
    thrown = true;
  }
  EXPECT_TRUE(thrown);
}

} // namespace
} // namespace apache::thrift::test
