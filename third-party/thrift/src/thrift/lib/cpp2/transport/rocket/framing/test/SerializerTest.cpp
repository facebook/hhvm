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

#include <thrift/lib/cpp2/transport/rocket/framing/Serializer.h>

#include <gtest/gtest.h>

using namespace apache::thrift::rocket;

class HeaderSerializerTest : public testing::Test {
 protected:
  static constexpr size_t kMaxBufferSize = 64;
  uint8_t buffer[kMaxBufferSize];
  HeaderSerializer serializer{buffer, kMaxBufferSize};
};

TEST_F(HeaderSerializerTest, WriteBE_Success) {
  uint32_t value = 0x12345678;
  size_t bytesWritten = serializer.writeBE(value);

  EXPECT_EQ(bytesWritten, sizeof(value));
  EXPECT_EQ(serializer.result().size(), sizeof(value));
  EXPECT_EQ(
      *reinterpret_cast<const uint32_t*>(serializer.result().data()),
      folly::Endian::big(value));
}

TEST_F(HeaderSerializerTest, WriteBE_BufferFull) {
  uint64_t value = 0x123456789ABCDEF0;
  // Fill the buffer with some data to make it almost full
  size_t offset = kMaxBufferSize / sizeof(size_t);
  for (size_t i = 0; i < offset; ++i) {
    size_t bytesWritten = serializer.writeBE(i);
    EXPECT_EQ(bytesWritten, sizeof(size_t));
  }

  size_t bytesWritten = serializer.writeBE(value);

  // The buffer is expected to be full, so no bytes should be written
  EXPECT_EQ(bytesWritten, 0);
}

TEST_F(HeaderSerializerTest, WriteFrameOrMetadataSize_Success) {
  size_t nbytes = 0x12345;
  size_t bytesWritten = serializer.writeFrameOrMetadataSize(nbytes);

  EXPECT_EQ(bytesWritten, HeaderSerializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(
      serializer.result().size(),
      HeaderSerializer::kBytesForFrameOrMetadataLength);
  EXPECT_EQ(serializer.result().data()[0], (nbytes >> 16) & 0xFF);
  EXPECT_EQ(serializer.result().data()[1], (nbytes >> 8) & 0xFF);
  EXPECT_EQ(serializer.result().data()[2], nbytes & 0xFF);
}

TEST_F(HeaderSerializerTest, WriteFrameOrMetadataSize_BufferFull) {
  // Fill the buffer with some data to make it almost full
  size_t offset = kMaxBufferSize / sizeof(size_t);
  for (size_t i = 0; i < offset; ++i) {
    size_t bytesWritten = serializer.writeBE(i);
    EXPECT_EQ(bytesWritten, sizeof(size_t));
  }

  size_t nbytes = 0x12345;
  size_t bytesWritten = serializer.writeFrameOrMetadataSize(nbytes);
  EXPECT_EQ(bytesWritten, 0);
}

TEST_F(HeaderSerializerTest, WriteFrameTypeAndFlags_Success) {
  FrameType frameType = FrameType::REQUEST_RESPONSE;
  Flags flags;
  flags.complete();

  size_t bytesWritten = serializer.writeFrameTypeAndFlags(frameType, flags);

  EXPECT_EQ(bytesWritten, sizeof(uint16_t));
  EXPECT_EQ(serializer.result().size(), sizeof(uint16_t));
  uint16_t resultValue =
      *reinterpret_cast<const uint16_t*>(serializer.result().data());
  uint16_t expectedValue = (static_cast<uint16_t>(frameType) << Flags::kBits) |
      static_cast<uint16_t>(flags);
  EXPECT_EQ(resultValue, folly::Endian::big(expectedValue));
}
