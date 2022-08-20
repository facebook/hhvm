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

#include <thrift/lib/cpp2/protocol/Protocol.h>

#include <folly/String.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/JSONProtocol.h>
#include <thrift/lib/cpp2/protocol/SimpleJSONProtocol.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;

class ProtocolTest : public testing::Test {};

static constexpr size_t kTestingProtocolMaxDepth = 4;

template <typename ProtocolWriter>
void makeNestedWriteInner(
    ProtocolWriter& writer, const size_t levels, const TType& type) {
  switch (type) {
    case TType::T_STRUCT: {
      for (size_t i = 0; i < levels; ++i) {
        writer.writeStructBegin("");
        writer.writeFieldBegin("fn", TType::T_STRUCT, 0);
      }
      writer.writeStructBegin("");
      writer.writeFieldBegin("fn", TType::T_BYTE, 0);
      writer.writeByte(7);
      writer.writeFieldEnd();
      writer.writeFieldStop();
      writer.writeStructEnd();
      for (size_t i = 0; i < levels; ++i) {
        writer.writeFieldEnd();
        writer.writeFieldStop();
        writer.writeStructEnd();
      }
      break;
    }
    case TType::T_LIST: {
      for (size_t i = 0; i < levels; ++i) {
        writer.writeListBegin(TType::T_LIST, 1);
      }
      writer.writeListBegin(TType::T_BYTE, 1);
      writer.writeByte(7);
      writer.writeListEnd();
      for (size_t i = 0; i < levels; ++i) {
        writer.writeListEnd();
      }
      break;
    }
    case TType::T_SET: {
      for (size_t i = 0; i < levels; ++i) {
        writer.writeSetBegin(TType::T_SET, 1);
      }
      writer.writeSetBegin(TType::T_BYTE, 1);
      writer.writeByte(7);
      writer.writeSetEnd();
      for (size_t i = 0; i < levels; ++i) {
        writer.writeSetEnd();
      }
      break;
    }
    case TType::T_MAP: {
      for (size_t i = 0; i < levels; ++i) {
        writer.writeMapBegin(TType::T_BYTE, TType::T_MAP, 1);
        writer.writeByte(7);
      }
      writer.writeMapBegin(TType::T_BYTE, TType::T_BYTE, 1);
      writer.writeByte(7);
      writer.writeByte(7);
      writer.writeMapEnd();
      for (size_t i = 0; i < levels; ++i) {
        writer.writeMapEnd();
      }
      break;
    }
    default:
      break;
  }
}

template <typename ProtocolWriter>
folly::IOBufQueue makeNested(
    const size_t height, const size_t levels, const TType type) {
  CHECK_GE(levels, 3);
  folly::IOBufQueue q;
  ProtocolWriter writer;
  writer.setOutput(&q);
  writer.setHeight(height);
  writer.writeStructBegin("");

  for (size_t j = 0; j < height; ++j) {
    writer.writeFieldBegin("fn", type, 0);
    makeNestedWriteInner(writer, levels - 3, type);
    writer.writeFieldEnd();
  }

  writer.writeFieldBegin("fn", type, 0);
  makeNestedWriteInner(writer, levels - 2, type);
  writer.writeFieldEnd();

  writer.writeFieldStop();
  writer.writeStructEnd();
  return q;
}

template <typename ProtocolReader>
size_t doSkip(const size_t height, const folly::IOBufQueue& input) {
  std::string inputs;
  input.appendToString(inputs);
  VLOG(1) << folly::hexlify<std::string>(inputs);
  ProtocolReader reader;
  reader.setHeight(height);
  reader.setInput(input.front());
  const auto a = reader.getCursorPosition();
  reader.skip(TType::T_STRUCT);
  const auto b = reader.getCursorPosition();
  return b - a;
}

template <typename ProtocolWriter, typename ProtocolReader>
void runSkipCheckDepth(
    folly::tag_t<ProtocolWriter, ProtocolReader>, const TType type) {
  const size_t height = kTestingProtocolMaxDepth;
  {
    const auto q = makeNested<ProtocolWriter>(height, height - 1, type);
    const auto s = doSkip<ProtocolReader>(height, q);
    EXPECT_EQ(q.front()->computeChainDataLength(), s);
  }
  {
    try {
      makeNested<ProtocolWriter>(height, height + 1, type);
      ADD_FAILURE() << "expected TProtocolException";
    } catch (const TProtocolException& e) {
      EXPECT_EQ(TProtocolException::DEPTH_LIMIT, e.getType());
    }
  }
  {
    auto q = makeNested<ProtocolWriter>(height + 1, height + 1, type);
    try {
      doSkip<ProtocolReader>(height, q);
      ADD_FAILURE() << "expected TProtocolException";
    } catch (const TProtocolException& e) {
      EXPECT_EQ(TProtocolException::DEPTH_LIMIT, e.getType());
    }
  }
}

using BinaryProtocol = folly::tag_t<BinaryProtocolWriter, BinaryProtocolReader>;
using CompactProtocol =
    folly::tag_t<CompactProtocolWriter, CompactProtocolReader>;
using JSONProtocol = folly::tag_t<JSONProtocolWriter, JSONProtocolReader>;
using SimpleJSONProtocol =
    folly::tag_t<SimpleJSONProtocolWriter, SimpleJSONProtocolReader>;

TEST_F(ProtocolTest, skip_check_depth_binary) {
  runSkipCheckDepth(BinaryProtocol{}, TType::T_STRUCT);
  runSkipCheckDepth(BinaryProtocol{}, TType::T_LIST);
  runSkipCheckDepth(BinaryProtocol{}, TType::T_SET);
  runSkipCheckDepth(BinaryProtocol{}, TType::T_MAP);
}

TEST_F(ProtocolTest, skip_check_depth_compact) {
  runSkipCheckDepth(CompactProtocol{}, TType::T_STRUCT);
  runSkipCheckDepth(CompactProtocol{}, TType::T_LIST);
  runSkipCheckDepth(CompactProtocol{}, TType::T_SET);
  runSkipCheckDepth(CompactProtocol{}, TType::T_MAP);
}

TEST_F(ProtocolTest, skip_check_depth_json) {
  runSkipCheckDepth(JSONProtocol{}, TType::T_STRUCT);
  runSkipCheckDepth(JSONProtocol{}, TType::T_LIST);
  runSkipCheckDepth(JSONProtocol{}, TType::T_SET);
  runSkipCheckDepth(JSONProtocol{}, TType::T_MAP);
}

TEST_F(ProtocolTest, skip_check_depth_simple_json) {
  runSkipCheckDepth(SimpleJSONProtocol{}, TType::T_STRUCT);
  runSkipCheckDepth(SimpleJSONProtocol{}, TType::T_LIST);
  runSkipCheckDepth(SimpleJSONProtocol{}, TType::T_SET);
  runSkipCheckDepth(SimpleJSONProtocol{}, TType::T_MAP);
}
