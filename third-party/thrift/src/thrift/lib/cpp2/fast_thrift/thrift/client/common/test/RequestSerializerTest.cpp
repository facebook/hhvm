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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <thrift/lib/cpp2/fast_thrift/thrift/client/common/RequestSerializer.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>

namespace apache::thrift::fast_thrift::thrift {

namespace {

struct TestPayload {
  int32_t value{0};
  std::string name;
};

template <typename Writer>
void serializeTestPayload(Writer& writer, const TestPayload& payload) {
  writer.writeStructBegin("TestPayload");
  writer.writeFieldBegin("value", apache::thrift::protocol::T_I32, 1);
  writer.writeI32(payload.value);
  writer.writeFieldEnd();
  writer.writeFieldBegin("name", apache::thrift::protocol::T_STRING, 2);
  writer.writeString(payload.name);
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
}

template <typename Writer>
uint32_t sizeTestPayload(Writer& writer, const TestPayload& payload) {
  uint32_t size = 0;
  size += writer.serializedStructSize("TestPayload");
  size +=
      writer.serializedFieldSize("value", apache::thrift::protocol::T_I32, 1);
  size += writer.serializedSizeI32(payload.value);
  size +=
      writer.serializedFieldSize("name", apache::thrift::protocol::T_STRING, 2);
  size += writer.serializedSizeString(payload.name);
  size += writer.serializedSizeStop();
  return size;
}

template <typename Reader>
TestPayload deserializeTestPayload(Reader& reader) {
  TestPayload result;
  std::string fname;
  apache::thrift::protocol::TType ftype;
  int16_t fid;
  reader.readStructBegin(fname);
  while (true) {
    reader.readFieldBegin(fname, ftype, fid);
    if (ftype == apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid) {
      case 1:
        reader.readI32(result.value);
        break;
      case 2:
        reader.readString(result.name);
        break;
      default:
        reader.skip(ftype);
        break;
    }
    reader.readFieldEnd();
  }
  reader.readStructEnd();
  return result;
}

} // namespace

TEST(RequestSerializerTest, CompactProtocolRoundTripsData) {
  TestPayload input{.value = 42, .name = "hello"};

  auto result = serializeRequest<apache::thrift::CompactProtocolWriter>(
      [&](apache::thrift::CompactProtocolWriter& w) {
        serializeTestPayload(w, input);
      },
      [&](apache::thrift::CompactProtocolWriter& w) {
        return sizeTestPayload(w, input);
      });

  ASSERT_TRUE(result.hasValue());
  ASSERT_NE(result.value(), nullptr);

  apache::thrift::CompactProtocolReader reader;
  reader.setInput(result.value().get());
  auto output = deserializeTestPayload(reader);

  EXPECT_EQ(output.value, 42);
  EXPECT_EQ(output.name, "hello");
}

TEST(RequestSerializerTest, BinaryProtocolRoundTripsData) {
  TestPayload input{.value = 99, .name = "binary"};

  auto result = serializeRequest<apache::thrift::BinaryProtocolWriter>(
      [&](apache::thrift::BinaryProtocolWriter& w) {
        serializeTestPayload(w, input);
      },
      [&](apache::thrift::BinaryProtocolWriter& w) {
        return sizeTestPayload(w, input);
      });

  ASSERT_TRUE(result.hasValue());
  ASSERT_NE(result.value(), nullptr);

  apache::thrift::BinaryProtocolReader reader;
  reader.setInput(result.value().get());
  auto output = deserializeTestPayload(reader);

  EXPECT_EQ(output.value, 99);
  EXPECT_EQ(output.name, "binary");
}

TEST(RequestSerializerTest, SerializationFailureReturnsError) {
  auto throwingSerializeFn = [](apache::thrift::CompactProtocolWriter&) {
    throw std::runtime_error("boom");
  };
  auto sizeFn = [](apache::thrift::CompactProtocolWriter& w) -> uint32_t {
    return w.serializedStructSize("empty") + w.serializedSizeStop();
  };

  auto result = serializeRequest<apache::thrift::CompactProtocolWriter>(
      throwingSerializeFn, sizeFn);

  ASSERT_TRUE(result.hasError());
  EXPECT_THAT(
      result.error()
          .get_exception<apache::thrift::TApplicationException>()
          ->getMessage(),
      ::testing::HasSubstr("data"));
}

} // namespace apache::thrift::fast_thrift::thrift
