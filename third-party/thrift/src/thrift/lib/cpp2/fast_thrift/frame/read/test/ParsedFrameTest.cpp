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

#include <thrift/lib/cpp2/fast_thrift/frame/read/ParsedFrame.h>

#include <folly/io/IOBuf.h>

#include <gtest/gtest.h>

namespace apache::thrift::fast_thrift::frame::read {
namespace {

TEST(ParsedFrameTest, DefaultConstruction) {
  ParsedFrame frame;

  EXPECT_EQ(frame.metadata.descriptor, nullptr);
  EXPECT_EQ(frame.metadata.streamId, 0);
  EXPECT_EQ(frame.buffer, nullptr);
  EXPECT_FALSE(frame.isValid());
  EXPECT_TRUE(frame.empty());
  EXPECT_FALSE(static_cast<bool>(frame));
}

TEST(ParsedFrameTest, ConvenienceAccessorsDelegateToMetadata) {
  ParsedFrame frame;
  frame.metadata.descriptor = &getDescriptor(FrameType::REQUEST_RESPONSE);
  frame.metadata.streamId = 42;
  frame.metadata.flags_ |=
      ::apache::thrift::fast_thrift::frame::detail::kMetadataBit;
  frame.metadata.flags_ |=
      ::apache::thrift::fast_thrift::frame::detail::kCompleteBit;
  frame.metadata.metadataSize = 10;
  frame.metadata.payloadSize = 100;

  EXPECT_EQ(frame.type(), FrameType::REQUEST_RESPONSE);
  EXPECT_STREQ(frame.typeName(), "REQUEST_RESPONSE");
  EXPECT_EQ(frame.streamId(), 42);
  EXPECT_TRUE(frame.hasMetadata());
  EXPECT_TRUE(frame.isComplete());
  EXPECT_EQ(frame.metadataSize(), 10);
  EXPECT_EQ(frame.payloadSize(), 100);
  EXPECT_EQ(frame.dataSize(), 90);
}

TEST(ParsedFrameTest, IsValidRequiresBothBufferAndDescriptor) {
  ParsedFrame frame;

  // Neither buffer nor descriptor
  EXPECT_FALSE(frame.isValid());

  // Only descriptor
  frame.metadata.descriptor = &getDescriptor(FrameType::PAYLOAD);
  EXPECT_FALSE(frame.isValid());

  // Only buffer
  frame.metadata.descriptor = nullptr;
  frame.buffer = folly::IOBuf::create(64);
  EXPECT_FALSE(frame.isValid());

  // Both buffer and valid descriptor
  frame.metadata.descriptor = &getDescriptor(FrameType::PAYLOAD);
  EXPECT_TRUE(frame.isValid());
  EXPECT_FALSE(frame.empty());
  EXPECT_TRUE(static_cast<bool>(frame));
}

TEST(ParsedFrameTest, ReservedFrameTypeIsNotValid) {
  ParsedFrame frame;
  frame.buffer = folly::IOBuf::create(64);
  frame.metadata.descriptor = &getDescriptor(FrameType::RESERVED);

  EXPECT_FALSE(frame.isValid());
}

TEST(ParsedFrameTest, FrameCursorStartsAtBeginning) {
  // Create a buffer with some test data
  auto buf = folly::IOBuf::create(20);
  buf->append(20);
  memset(buf->writableData(), 0, 20);
  buf->writableData()[0] = 0xAB;
  buf->writableData()[1] = 0xCD;

  ParsedFrame frame;
  frame.buffer = std::move(buf);

  auto cursor = frame.frameCursor();
  EXPECT_EQ(cursor.read<uint8_t>(), 0xAB);
  EXPECT_EQ(cursor.read<uint8_t>(), 0xCD);
}

TEST(ParsedFrameTest, PayloadCursorSkipsToPayloadOffset) {
  // Create a buffer simulating frame header + payload
  auto buf = folly::IOBuf::create(20);
  buf->append(20);
  memset(buf->writableData(), 0, 20);
  // Put marker bytes at offset 10 (simulating payload start)
  buf->writableData()[10] = 0xDE;
  buf->writableData()[11] = 0xAD;

  ParsedFrame frame;
  frame.buffer = std::move(buf);
  frame.metadata.payloadOffset = 10;

  auto cursor = frame.payloadCursor();
  EXPECT_EQ(cursor.read<uint8_t>(), 0xDE);
  EXPECT_EQ(cursor.read<uint8_t>(), 0xAD);
}

TEST(ParsedFrameTest, MetadataCursorSameAsPayloadCursor) {
  auto buf = folly::IOBuf::create(20);
  buf->append(20);
  memset(buf->writableData(), 0, 20);
  buf->writableData()[5] = 0x12;

  ParsedFrame frame;
  frame.buffer = std::move(buf);
  frame.metadata.payloadOffset = 5;

  auto payloadCursor = frame.payloadCursor();
  auto metadataCursor = frame.metadataCursor();

  EXPECT_EQ(payloadCursor.read<uint8_t>(), 0x12);
  EXPECT_EQ(metadataCursor.read<uint8_t>(), 0x12);
}

TEST(ParsedFrameTest, DataCursorSkipsPastMetadata) {
  // Create a buffer with metadata followed by data
  auto buf = folly::IOBuf::create(30);
  buf->append(30);
  memset(buf->writableData(), 0, 30);
  // Payload starts at offset 10
  // Metadata is 5 bytes at offset 10-14
  // Data starts at offset 15
  buf->writableData()[15] = 0xBE;
  buf->writableData()[16] = 0xEF;

  ParsedFrame frame;
  frame.buffer = std::move(buf);
  frame.metadata.payloadOffset = 10;
  frame.metadata.metadataSize = 5;
  frame.metadata.flags_ |=
      ::apache::thrift::fast_thrift::frame::detail::kMetadataBit;

  auto cursor = frame.dataCursor();
  EXPECT_EQ(cursor.read<uint8_t>(), 0xBE);
  EXPECT_EQ(cursor.read<uint8_t>(), 0xEF);
}

TEST(ParsedFrameTest, DataCursorWithNoMetadata) {
  auto buf = folly::IOBuf::create(20);
  buf->append(20);
  memset(buf->writableData(), 0, 20);
  buf->writableData()[8] = 0xCA;
  buf->writableData()[9] = 0xFE;

  ParsedFrame frame;
  frame.buffer = std::move(buf);
  frame.metadata.payloadOffset = 8;
  frame.metadata.metadataSize = 0; // No metadata

  // With no metadata, data cursor should be at payload offset
  auto dataCursor = frame.dataCursor();
  auto payloadCursor = frame.payloadCursor();

  EXPECT_EQ(dataCursor.read<uint8_t>(), 0xCA);
  EXPECT_EQ(payloadCursor.read<uint8_t>(), 0xCA);
}

TEST(ParsedFrameTest, MoveSemantics) {
  ParsedFrame frame1;
  frame1.metadata.descriptor = &getDescriptor(FrameType::REQUEST_STREAM);
  frame1.metadata.streamId = 123;
  frame1.buffer = folly::IOBuf::create(32);

  // Move construct
  ParsedFrame frame2 = std::move(frame1);

  EXPECT_EQ(frame2.metadata.streamId, 123);
  EXPECT_EQ(frame2.type(), FrameType::REQUEST_STREAM);
  EXPECT_NE(frame2.buffer, nullptr);

  // Original should have null buffer
  EXPECT_EQ(frame1.buffer, nullptr); // NOLINT(bugprone-use-after-move)
}

TEST(ParsedFrameTest, IsConnectionFrameReturnsTrueForStreamIdZero) {
  ParsedFrame frame;
  frame.metadata.descriptor = &getDescriptor(FrameType::KEEPALIVE);
  frame.metadata.streamId = kConnectionStreamId;
  frame.buffer = folly::IOBuf::create(32);

  EXPECT_TRUE(frame.isConnectionFrame());
  EXPECT_EQ(frame.streamId(), 0);
}

TEST(ParsedFrameTest, IsConnectionFrameReturnsFalseForNonZeroStreamId) {
  ParsedFrame frame;
  frame.metadata.descriptor = &getDescriptor(FrameType::PAYLOAD);
  frame.metadata.streamId = 1;
  frame.buffer = folly::IOBuf::create(32);

  EXPECT_FALSE(frame.isConnectionFrame());

  // Also test with other stream IDs
  frame.metadata.streamId = 42;
  EXPECT_FALSE(frame.isConnectionFrame());

  frame.metadata.streamId = 999;
  EXPECT_FALSE(frame.isConnectionFrame());
}

// =============================================================================
// isTerminalFrame Tests
// =============================================================================

TEST(ParsedFrameTest, IsTerminalFrameReturnsTrueForErrorFrame) {
  ParsedFrame frame;
  frame.metadata.descriptor = &getDescriptor(FrameType::ERROR);
  frame.metadata.streamId = 1;
  frame.buffer = folly::IOBuf::create(32);

  EXPECT_TRUE(frame.isTerminalFrame());

  // ERROR is terminal even without complete flag
  frame.metadata.flags_ = 0;
  EXPECT_TRUE(frame.isTerminalFrame());
}

TEST(ParsedFrameTest, IsTerminalFrameReturnsTrueForCancelFrame) {
  ParsedFrame frame;
  frame.metadata.descriptor = &getDescriptor(FrameType::CANCEL);
  frame.metadata.streamId = 1;
  frame.buffer = folly::IOBuf::create(32);

  EXPECT_TRUE(frame.isTerminalFrame());

  // CANCEL is terminal even without complete flag
  frame.metadata.flags_ = 0;
  EXPECT_TRUE(frame.isTerminalFrame());
}

TEST(ParsedFrameTest, IsTerminalFrameReturnsTrueForPayloadWithCompleteFlag) {
  ParsedFrame frame;
  frame.metadata.descriptor = &getDescriptor(FrameType::PAYLOAD);
  frame.metadata.streamId = 1;
  frame.metadata.flags_ =
      ::apache::thrift::fast_thrift::frame::detail::kCompleteBit;
  frame.buffer = folly::IOBuf::create(32);

  EXPECT_TRUE(frame.isTerminalFrame());
}

TEST(
    ParsedFrameTest, IsTerminalFrameReturnsFalseForPayloadWithoutCompleteFlag) {
  ParsedFrame frame;
  frame.metadata.descriptor = &getDescriptor(FrameType::PAYLOAD);
  frame.metadata.streamId = 1;
  frame.metadata.flags_ = 0; // No complete flag
  frame.buffer = folly::IOBuf::create(32);

  EXPECT_FALSE(frame.isTerminalFrame());

  // Even with other flags, still not terminal without complete
  frame.metadata.flags_ =
      ::apache::thrift::fast_thrift::frame::detail::kNextBit |
      ::apache::thrift::fast_thrift::frame::detail::kMetadataBit;
  EXPECT_FALSE(frame.isTerminalFrame());
}

TEST(ParsedFrameTest, IsTerminalFrameReturnsFalseForNonTerminalFrameTypes) {
  ParsedFrame frame;
  frame.metadata.streamId = 1;
  frame.buffer = folly::IOBuf::create(32);

  // REQUEST_N is never terminal
  frame.metadata.descriptor = &getDescriptor(FrameType::REQUEST_N);
  EXPECT_FALSE(frame.isTerminalFrame());

  // REQUEST_RESPONSE is never terminal (it's a request frame)
  frame.metadata.descriptor = &getDescriptor(FrameType::REQUEST_RESPONSE);
  EXPECT_FALSE(frame.isTerminalFrame());

  // REQUEST_STREAM is never terminal
  frame.metadata.descriptor = &getDescriptor(FrameType::REQUEST_STREAM);
  EXPECT_FALSE(frame.isTerminalFrame());

  // KEEPALIVE is never terminal
  frame.metadata.descriptor = &getDescriptor(FrameType::KEEPALIVE);
  EXPECT_FALSE(frame.isTerminalFrame());

  // SETUP is never terminal
  frame.metadata.descriptor = &getDescriptor(FrameType::SETUP);
  EXPECT_FALSE(frame.isTerminalFrame());
}

// =============================================================================
// extractData Tests
// =============================================================================

TEST(ParsedFrameTest, ExtractDataReturnsOnlyDataPortion) {
  // Buffer layout: [header: 6 bytes][metadata: 5 bytes][data: 10 bytes]
  auto buf = folly::IOBuf::create(21);
  buf->append(21);
  memset(buf->writableData(), 0xAA, 6); // header
  memset(buf->writableData() + 6, 0xBB, 5); // metadata
  buf->writableData()[11] = 0xDE; // data marker
  buf->writableData()[12] = 0xAD;

  ParsedFrame frame;
  frame.metadata.descriptor = &getDescriptor(FrameType::PAYLOAD);
  frame.metadata.payloadOffset = 6;
  frame.metadata.metadataSize = 5;
  frame.metadata.payloadSize = 15;
  frame.buffer = std::move(buf);

  auto data = std::move(frame).extractData();

  ASSERT_NE(data, nullptr);
  EXPECT_EQ(data->computeChainDataLength(), 10);
  EXPECT_EQ(data->data()[0], 0xDE);
  EXPECT_EQ(data->data()[1], 0xAD);
}

TEST(ParsedFrameTest, ExtractDataWithNoMetadata) {
  auto buf = folly::IOBuf::create(16);
  buf->append(16);
  memset(buf->writableData(), 0xAA, 6); // header
  buf->writableData()[6] = 0xCA; // data starts immediately
  buf->writableData()[7] = 0xFE;

  ParsedFrame frame;
  frame.metadata.descriptor = &getDescriptor(FrameType::PAYLOAD);
  frame.metadata.payloadOffset = 6;
  frame.metadata.metadataSize = 0;
  frame.metadata.payloadSize = 10;
  frame.buffer = std::move(buf);

  auto data = std::move(frame).extractData();

  ASSERT_NE(data, nullptr);
  EXPECT_EQ(data->computeChainDataLength(), 10);
  EXPECT_EQ(data->data()[0], 0xCA);
}

TEST(ParsedFrameTest, ExtractDataIsZeroCopy) {
  auto buf = folly::IOBuf::create(20);
  buf->append(20);
  buf->writableData()[10] = 0x42;

  const uint8_t* expectedDataPtr = buf->data() + 10;

  ParsedFrame frame;
  frame.metadata.descriptor = &getDescriptor(FrameType::PAYLOAD);
  frame.metadata.payloadOffset = 6;
  frame.metadata.metadataSize = 4;
  frame.metadata.payloadSize = 14;
  frame.buffer = std::move(buf);

  auto data = std::move(frame).extractData();

  // Same underlying memory, no copy
  EXPECT_EQ(data->data(), expectedDataPtr);
  EXPECT_EQ(data->data()[0], 0x42);
}

TEST(ParsedFrameTest, ExtractDataWithChainedIOBuf) {
  // Simulate a frame serialized as a chain: [header] -> [metadata] -> [data]
  // This is how serialize() actually creates frames when metadata/data are
  // passed as separate IOBufs.

  // Header: 9 bytes (6 base + 3 metadata length field)
  auto header = folly::IOBuf::create(9);
  header->append(9);
  memset(header->writableData(), 0xAA, 9);

  // Metadata: 6 bytes
  auto metadata = folly::IOBuf::create(6);
  metadata->append(6);
  memset(metadata->writableData(), 0xBB, 6);

  // Data: "test_data" (9 bytes)
  auto data = folly::IOBuf::copyBuffer("test_data");

  // Chain them together: header -> metadata -> data
  header->appendToChain(std::move(metadata));
  header->appendToChain(std::move(data));

  ASSERT_TRUE(header->isChained());
  EXPECT_EQ(header->computeChainDataLength(), 24); // 9 + 6 + 9

  ParsedFrame frame;
  frame.metadata.descriptor = &getDescriptor(FrameType::PAYLOAD);
  frame.metadata.payloadOffset = 9; // header size
  frame.metadata.metadataSize = 6; // metadata size
  frame.metadata.payloadSize = 15; // metadata + data
  frame.buffer = std::move(header);

  auto extractedData = std::move(frame).extractData();

  ASSERT_NE(extractedData, nullptr);
  EXPECT_EQ(extractedData->computeChainDataLength(), 9);

  // Verify the content is exactly "test_data"
  std::string actualData(
      reinterpret_cast<const char*>(extractedData->data()),
      extractedData->length());
  EXPECT_EQ(actualData, "test_data");
}

TEST(ParsedFrameTest, ExtractDataWithChainedIOBufExactBoundary) {
  // Edge case: trim boundary exactly matches buffer boundary
  // This specifically tests the >= vs > condition in extractData()

  // Header: 6 bytes (exact payloadOffset)
  auto header = folly::IOBuf::create(6);
  header->append(6);
  memset(header->writableData(), 0xAA, 6);

  // Metadata: 10 bytes (exact metadataSize)
  auto metadata = folly::IOBuf::create(10);
  metadata->append(10);
  memset(metadata->writableData(), 0xBB, 10);

  // Data: "exact_boundary" (14 bytes)
  auto data = folly::IOBuf::copyBuffer("exact_boundary");

  header->appendToChain(std::move(metadata));
  header->appendToChain(std::move(data));

  ParsedFrame frame;
  frame.metadata.descriptor = &getDescriptor(FrameType::PAYLOAD);
  frame.metadata.payloadOffset = 6; // Exactly matches header length
  frame.metadata.metadataSize = 10; // Exactly matches metadata length
  frame.metadata.payloadSize = 24; // metadata + data
  frame.buffer = std::move(header);

  auto extractedData = std::move(frame).extractData();

  ASSERT_NE(extractedData, nullptr);
  EXPECT_EQ(extractedData->computeChainDataLength(), 14);

  std::string actualData(
      reinterpret_cast<const char*>(extractedData->data()),
      extractedData->length());
  EXPECT_EQ(actualData, "exact_boundary");
}

} // namespace
} // namespace apache::thrift::fast_thrift::frame::read
