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

#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>

#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>

using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::read;

namespace {

// Helper to create a frame buffer from binary data
std::unique_ptr<folly::IOBuf> makeFrameBuffer(std::vector<uint8_t> data) {
  return folly::IOBuf::copyBuffer(data.data(), data.size());
}

// Helper to create frame header bytes
void writeU32BE(std::vector<uint8_t>& buf, uint32_t value) {
  buf.push_back(static_cast<uint8_t>(value >> 24));
  buf.push_back(static_cast<uint8_t>(value >> 16));
  buf.push_back(static_cast<uint8_t>(value >> 8));
  buf.push_back(static_cast<uint8_t>(value));
}

void writeU16BE(std::vector<uint8_t>& buf, uint16_t value) {
  buf.push_back(static_cast<uint8_t>(value >> 8));
  buf.push_back(static_cast<uint8_t>(value));
}

void writeU24BE(std::vector<uint8_t>& buf, uint32_t value) {
  buf.push_back(static_cast<uint8_t>(value >> 16));
  buf.push_back(static_cast<uint8_t>(value >> 8));
  buf.push_back(static_cast<uint8_t>(value));
}

// Build frame typeAndFlags from type and flags
uint16_t makeTypeAndFlags(FrameType type, uint16_t flags) {
  return (static_cast<uint16_t>(type) << 10) | (flags & 0x3FF);
}

} // namespace

// ============================================================================
// Basic Parsing Tests
// ============================================================================

TEST(FrameParserTest, ParseRequestResponseFrame) {
  std::vector<uint8_t> data;

  // Stream ID = 1
  writeU32BE(data, 1);

  // TypeAndFlags: REQUEST_RESPONSE (0x04), no flags
  writeU16BE(data, makeTypeAndFlags(FrameType::REQUEST_RESPONSE, 0));

  // Payload: "hello"
  const std::string payload = "hello";
  data.insert(data.end(), payload.begin(), payload.end());

  auto frame = parseFrame(makeFrameBuffer(data));

  EXPECT_TRUE(frame.isValid());
  EXPECT_EQ(frame.type(), FrameType::REQUEST_RESPONSE);
  EXPECT_EQ(frame.typeName(), std::string("REQUEST_RESPONSE"));
  EXPECT_EQ(frame.streamId(), 1);
  EXPECT_FALSE(frame.hasMetadata());
  EXPECT_EQ(frame.payloadSize(), 5);
  EXPECT_EQ(frame.metadataSize(), 0);
  EXPECT_EQ(frame.dataSize(), 5);
}

TEST(FrameParserTest, ParseRequestStreamFrame) {
  std::vector<uint8_t> data;

  // Stream ID = 42
  writeU32BE(data, 42);

  // TypeAndFlags: REQUEST_STREAM (0x06), no flags
  writeU16BE(data, makeTypeAndFlags(FrameType::REQUEST_STREAM, 0));

  // InitialRequestN = 100
  writeU32BE(data, 100);

  // Payload: "data"
  const std::string payload = "data";
  data.insert(data.end(), payload.begin(), payload.end());

  auto frame = parseFrame(makeFrameBuffer(data));

  EXPECT_TRUE(frame.isValid());
  EXPECT_EQ(frame.type(), FrameType::REQUEST_STREAM);
  EXPECT_EQ(frame.streamId(), 42);
  EXPECT_EQ(frame.payloadSize(), 4);
}

TEST(FrameParserTest, ParsePayloadFrame) {
  std::vector<uint8_t> data;

  // Stream ID = 5
  writeU32BE(data, 5);

  // TypeAndFlags: PAYLOAD (0x0A), complete + next flags
  uint16_t flags = (1 << 6) | (1 << 5); // complete (bit 6), next (bit 5)
  writeU16BE(data, makeTypeAndFlags(FrameType::PAYLOAD, flags));

  // Payload: "response"
  const std::string payload = "response";
  data.insert(data.end(), payload.begin(), payload.end());

  auto frame = parseFrame(makeFrameBuffer(data));

  EXPECT_TRUE(frame.isValid());
  EXPECT_EQ(frame.type(), FrameType::PAYLOAD);
  EXPECT_EQ(frame.streamId(), 5);
  EXPECT_TRUE(frame.isComplete());
  EXPECT_TRUE(frame.hasNext());
  EXPECT_EQ(frame.payloadSize(), 8);
}

TEST(FrameParserTest, ParseErrorFrame) {
  std::vector<uint8_t> data;

  // Stream ID = 3
  writeU32BE(data, 3);

  // TypeAndFlags: ERROR (0x0B), no flags
  writeU16BE(data, makeTypeAndFlags(FrameType::ERROR, 0));

  // Error code = 0x00000201 (APPLICATION_ERROR)
  writeU32BE(data, 0x00000201);

  // Error message: "error occurred"
  const std::string message = "error occurred";
  data.insert(data.end(), message.begin(), message.end());

  auto frame = parseFrame(makeFrameBuffer(data));

  EXPECT_TRUE(frame.isValid());
  EXPECT_EQ(frame.type(), FrameType::ERROR);
  EXPECT_EQ(frame.streamId(), 3);
  EXPECT_EQ(frame.payloadSize(), 14);
}

TEST(FrameParserTest, ParseCancelFrame) {
  std::vector<uint8_t> data;

  // Stream ID = 7
  writeU32BE(data, 7);

  // TypeAndFlags: CANCEL (0x09), no flags
  writeU16BE(data, makeTypeAndFlags(FrameType::CANCEL, 0));

  auto frame = parseFrame(makeFrameBuffer(data));

  EXPECT_TRUE(frame.isValid());
  EXPECT_EQ(frame.type(), FrameType::CANCEL);
  EXPECT_EQ(frame.streamId(), 7);
  EXPECT_EQ(frame.payloadSize(), 0);
}

TEST(FrameParserTest, ParseRequestNFrame) {
  std::vector<uint8_t> data;

  // Stream ID = 8
  writeU32BE(data, 8);

  // TypeAndFlags: REQUEST_N (0x08), no flags
  writeU16BE(data, makeTypeAndFlags(FrameType::REQUEST_N, 0));

  // RequestN = 50
  writeU32BE(data, 50);

  auto frame = parseFrame(makeFrameBuffer(data));

  EXPECT_TRUE(frame.isValid());
  EXPECT_EQ(frame.type(), FrameType::REQUEST_N);
  EXPECT_EQ(frame.streamId(), 8);
  EXPECT_EQ(frame.payloadSize(), 0);
}

// ============================================================================
// Connection-Level Frame Tests
// ============================================================================

TEST(FrameParserTest, ParseKeepAliveFrame) {
  std::vector<uint8_t> data;

  // Stream ID = 0 (connection-level)
  writeU32BE(data, 0);

  // TypeAndFlags: KEEPALIVE (0x03), respond flag (bit 7)
  uint16_t flags = 1 << 7; // respond flag
  writeU16BE(data, makeTypeAndFlags(FrameType::KEEPALIVE, flags));

  // Last received position = 12345678
  uint64_t lastPos = 12345678;
  data.push_back(static_cast<uint8_t>(lastPos >> 56));
  data.push_back(static_cast<uint8_t>(lastPos >> 48));
  data.push_back(static_cast<uint8_t>(lastPos >> 40));
  data.push_back(static_cast<uint8_t>(lastPos >> 32));
  data.push_back(static_cast<uint8_t>(lastPos >> 24));
  data.push_back(static_cast<uint8_t>(lastPos >> 16));
  data.push_back(static_cast<uint8_t>(lastPos >> 8));
  data.push_back(static_cast<uint8_t>(lastPos));

  auto frame = parseFrame(makeFrameBuffer(data));

  EXPECT_TRUE(frame.isValid());
  EXPECT_EQ(frame.type(), FrameType::KEEPALIVE);
  EXPECT_EQ(frame.streamId(), 0);
  EXPECT_TRUE(frame.metadata.shouldRespond());
}

TEST(FrameParserTest, ParseSetupFrame) {
  std::vector<uint8_t> data;

  // Stream ID = 0 (connection-level)
  writeU32BE(data, 0);

  // TypeAndFlags: SETUP (0x01), metadata flag (bit 8)
  uint16_t flags = 1 << 8;
  writeU16BE(data, makeTypeAndFlags(FrameType::SETUP, flags));

  // Version: major=1, minor=0
  writeU16BE(data, 1); // major
  writeU16BE(data, 0); // minor

  // Keepalive: 30000ms
  writeU32BE(data, 30000);

  // Max lifetime: 60000ms
  writeU32BE(data, 60000);

  // Resume token length = 0 (no resume token)
  writeU16BE(data, 0);

  // Metadata size (3 bytes)
  writeU24BE(data, 5);

  // Metadata: "meta1"
  const std::string metadata = "meta1";
  data.insert(data.end(), metadata.begin(), metadata.end());

  // Data: "setup data"
  const std::string payload = "setup data";
  data.insert(data.end(), payload.begin(), payload.end());

  auto frame = parseFrame(makeFrameBuffer(data));

  EXPECT_TRUE(frame.isValid());
  EXPECT_EQ(frame.type(), FrameType::SETUP);
  EXPECT_EQ(frame.typeName(), std::string("SETUP"));
  EXPECT_EQ(frame.streamId(), 0);
  EXPECT_TRUE(frame.hasMetadata());
  EXPECT_EQ(frame.metadataSize(), 5);
  EXPECT_EQ(frame.dataSize(), 10);
}

TEST(FrameParserTest, ParseMetadataPushFrame) {
  std::vector<uint8_t> data;

  // Stream ID = 0 (connection-level)
  writeU32BE(data, 0);

  // TypeAndFlags: METADATA_PUSH (0x0C), metadata flag always set
  uint16_t flags = 1 << 8;
  writeU16BE(data, makeTypeAndFlags(FrameType::METADATA_PUSH, flags));

  // Metadata: "pushed metadata"
  const std::string metadata = "pushed metadata";
  data.insert(data.end(), metadata.begin(), metadata.end());

  auto frame = parseFrame(makeFrameBuffer(data));

  EXPECT_TRUE(frame.isValid());
  EXPECT_EQ(frame.type(), FrameType::METADATA_PUSH);
  EXPECT_EQ(frame.streamId(), 0);
  // Note: METADATA_PUSH doesn't have a metadata size prefix in the payload
  // since the entire payload is metadata
}

// ============================================================================
// Metadata Handling Tests
// ============================================================================

TEST(FrameParserTest, ParseFrameWithMetadata) {
  std::vector<uint8_t> data;

  // Stream ID = 10
  writeU32BE(data, 10);

  // TypeAndFlags: REQUEST_RESPONSE (0x04), metadata flag (bit 8)
  uint16_t flags = 1 << 8;
  writeU16BE(data, makeTypeAndFlags(FrameType::REQUEST_RESPONSE, flags));

  // Metadata size (3 bytes) = 12
  writeU24BE(data, 12);

  // Metadata: "metametameta"
  const std::string metadata = "metametameta";
  data.insert(data.end(), metadata.begin(), metadata.end());

  // Data: "actual data"
  const std::string payload = "actual data";
  data.insert(data.end(), payload.begin(), payload.end());

  auto frame = parseFrame(makeFrameBuffer(data));

  EXPECT_TRUE(frame.isValid());
  EXPECT_TRUE(frame.hasMetadata());
  EXPECT_EQ(frame.metadataSize(), 12);
  EXPECT_EQ(frame.payloadSize(), 23); // 12 + 11
  EXPECT_EQ(frame.dataSize(), 11);
}

TEST(FrameParserTest, ParseFrameWithoutMetadata) {
  std::vector<uint8_t> data;

  // Stream ID = 11
  writeU32BE(data, 11);

  // TypeAndFlags: REQUEST_RESPONSE (0x04), no metadata flag
  writeU16BE(data, makeTypeAndFlags(FrameType::REQUEST_RESPONSE, 0));

  // Data only: "pure data"
  const std::string payload = "pure data";
  data.insert(data.end(), payload.begin(), payload.end());

  auto frame = parseFrame(makeFrameBuffer(data));

  EXPECT_TRUE(frame.isValid());
  EXPECT_FALSE(frame.hasMetadata());
  EXPECT_EQ(frame.metadataSize(), 0);
  EXPECT_EQ(frame.payloadSize(), 9);
  EXPECT_EQ(frame.dataSize(), 9);
}

// ============================================================================
// Flag Tests
// ============================================================================

TEST(FrameParserTest, FollowsFlag) {
  std::vector<uint8_t> data;

  // Stream ID = 20
  writeU32BE(data, 20);

  // TypeAndFlags: REQUEST_RESPONSE (0x04), follows flag (bit 7)
  uint16_t flags = 1 << 7;
  writeU16BE(data, makeTypeAndFlags(FrameType::REQUEST_RESPONSE, flags));

  // Fragment data
  const std::string fragment = "fragment1";
  data.insert(data.end(), fragment.begin(), fragment.end());

  auto frame = parseFrame(makeFrameBuffer(data));

  EXPECT_TRUE(frame.isValid());
  EXPECT_TRUE(frame.hasFollows());
}

TEST(FrameParserTest, CompleteAndNextFlags) {
  std::vector<uint8_t> data;

  // Stream ID = 21
  writeU32BE(data, 21);

  // TypeAndFlags: PAYLOAD (0x0A), complete (bit 6) + next (bit 5)
  uint16_t flags = (1 << 6) | (1 << 5);
  writeU16BE(data, makeTypeAndFlags(FrameType::PAYLOAD, flags));

  // Payload data
  const std::string payload = "final payload";
  data.insert(data.end(), payload.begin(), payload.end());

  auto frame = parseFrame(makeFrameBuffer(data));

  EXPECT_TRUE(frame.isValid());
  EXPECT_TRUE(frame.isComplete());
  EXPECT_TRUE(frame.hasNext());
}

// ============================================================================
// Cursor Access Tests
// ============================================================================

TEST(FrameParserTest, PayloadCursorReadsCorrectData) {
  std::vector<uint8_t> data;

  // Stream ID = 30
  writeU32BE(data, 30);

  // TypeAndFlags: REQUEST_RESPONSE (0x04), no flags
  writeU16BE(data, makeTypeAndFlags(FrameType::REQUEST_RESPONSE, 0));

  // Payload: "test payload"
  const std::string payload = "test payload";
  data.insert(data.end(), payload.begin(), payload.end());

  auto frame = parseFrame(makeFrameBuffer(data));
  auto cursor = frame.payloadCursor();

  std::string result;
  result.resize(frame.payloadSize());
  cursor.pull(result.data(), result.size());

  EXPECT_EQ(result, payload);
}

TEST(FrameParserTest, DataCursorSkipsMetadata) {
  std::vector<uint8_t> data;

  // Stream ID = 31
  writeU32BE(data, 31);

  // TypeAndFlags: REQUEST_RESPONSE (0x04), metadata flag (bit 8)
  uint16_t flags = 1 << 8;
  writeU16BE(data, makeTypeAndFlags(FrameType::REQUEST_RESPONSE, flags));

  // Metadata size (3 bytes) = 8
  writeU24BE(data, 8);

  // Metadata: "metadata"
  const std::string metadata = "metadata";
  data.insert(data.end(), metadata.begin(), metadata.end());

  // Data: "actual"
  const std::string dataStr = "actual";
  data.insert(data.end(), dataStr.begin(), dataStr.end());

  auto frame = parseFrame(makeFrameBuffer(data));
  auto cursor = frame.dataCursor();

  std::string result;
  result.resize(frame.dataSize());
  cursor.pull(result.data(), result.size());

  EXPECT_EQ(result, dataStr);
}

TEST(FrameParserTest, FrameCursorReadsEntireFrame) {
  std::vector<uint8_t> data;

  // Stream ID = 32
  writeU32BE(data, 32);

  // TypeAndFlags: REQUEST_N (0x08), no flags
  writeU16BE(data, makeTypeAndFlags(FrameType::REQUEST_N, 0));

  // RequestN = 999
  writeU32BE(data, 999);

  auto frame = parseFrame(makeFrameBuffer(data));
  auto cursor = frame.frameCursor();

  // Read back stream ID
  EXPECT_EQ(cursor.readBE<uint32_t>(), 32u);

  // Read back typeAndFlags
  uint16_t typeAndFlags = cursor.readBE<uint16_t>();
  EXPECT_EQ(typeAndFlags >> 10, static_cast<uint16_t>(FrameType::REQUEST_N));

  // Read back requestN
  EXPECT_EQ(cursor.readBE<uint32_t>(), 999u);
}

// ============================================================================
// Edge Cases and Error Handling
// ============================================================================

TEST(FrameParserTest, TryParseFrameWithNullBuffer) {
  auto frame = tryParseFrame(nullptr);

  EXPECT_FALSE(frame.isValid());
  EXPECT_TRUE(frame.empty());
}

TEST(FrameParserTest, TryParseFrameWithTooSmallBuffer) {
  // Only 4 bytes, need at least 6
  std::vector<uint8_t> data = {0x00, 0x00, 0x00, 0x01};
  auto frame = tryParseFrame(makeFrameBuffer(data));

  EXPECT_FALSE(frame.isValid());
}

TEST(FrameParserTest, TryParseFrameWithValidBuffer) {
  std::vector<uint8_t> data;

  // Stream ID = 1
  writeU32BE(data, 1);

  // TypeAndFlags: CANCEL (0x09), no flags
  writeU16BE(data, makeTypeAndFlags(FrameType::CANCEL, 0));

  auto frame = tryParseFrame(makeFrameBuffer(data));

  EXPECT_TRUE(frame.isValid());
  EXPECT_EQ(frame.type(), FrameType::CANCEL);
}

TEST(FrameParserTest, ParseEmptyPayloadFrame) {
  std::vector<uint8_t> data;

  // Stream ID = 40
  writeU32BE(data, 40);

  // TypeAndFlags: REQUEST_RESPONSE (0x04), no flags
  writeU16BE(data, makeTypeAndFlags(FrameType::REQUEST_RESPONSE, 0));

  // No payload

  auto frame = parseFrame(makeFrameBuffer(data));

  EXPECT_TRUE(frame.isValid());
  EXPECT_EQ(frame.payloadSize(), 0);
  EXPECT_EQ(frame.dataSize(), 0);
}

TEST(FrameParserTest, ParseFrameWithLargeStreamId) {
  std::vector<uint8_t> data;

  // Stream ID = max uint32
  writeU32BE(data, 0xFFFFFFFF);

  // TypeAndFlags: CANCEL (0x09), no flags
  writeU16BE(data, makeTypeAndFlags(FrameType::CANCEL, 0));

  auto frame = parseFrame(makeFrameBuffer(data));

  EXPECT_TRUE(frame.isValid());
  EXPECT_EQ(frame.streamId(), 0xFFFFFFFF);
}

// ============================================================================
// Request Frames Tests (REQUEST_FNF, REQUEST_CHANNEL)
// ============================================================================

TEST(FrameParserTest, ParseRequestFnfFrame) {
  std::vector<uint8_t> data;

  // Stream ID = 50
  writeU32BE(data, 50);

  // TypeAndFlags: REQUEST_FNF (0x05), no flags
  writeU16BE(data, makeTypeAndFlags(FrameType::REQUEST_FNF, 0));

  // Payload: "fire and forget"
  const std::string payload = "fire and forget";
  data.insert(data.end(), payload.begin(), payload.end());

  auto frame = parseFrame(makeFrameBuffer(data));

  EXPECT_TRUE(frame.isValid());
  EXPECT_EQ(frame.type(), FrameType::REQUEST_FNF);
  EXPECT_EQ(frame.streamId(), 50);
  EXPECT_TRUE(frame.metadata.isRequestFrame());
  EXPECT_EQ(frame.payloadSize(), 15);
}

TEST(FrameParserTest, ParseRequestChannelFrame) {
  std::vector<uint8_t> data;

  // Stream ID = 51
  writeU32BE(data, 51);

  // TypeAndFlags: REQUEST_CHANNEL (0x07), complete flag (bit 6)
  uint16_t flags = 1 << 6;
  writeU16BE(data, makeTypeAndFlags(FrameType::REQUEST_CHANNEL, flags));

  // InitialRequestN = 1000
  writeU32BE(data, 1000);

  // Payload: "channel data"
  const std::string payload = "channel data";
  data.insert(data.end(), payload.begin(), payload.end());

  auto frame = parseFrame(makeFrameBuffer(data));

  EXPECT_TRUE(frame.isValid());
  EXPECT_EQ(frame.type(), FrameType::REQUEST_CHANNEL);
  EXPECT_EQ(frame.streamId(), 51);
  EXPECT_TRUE(frame.isComplete());
  EXPECT_TRUE(frame.metadata.isRequestFrame());
}

// ============================================================================
// EXT Frame Test
// ============================================================================

TEST(FrameParserTest, ParseExtFrame) {
  std::vector<uint8_t> data;

  // Stream ID = 60
  writeU32BE(data, 60);

  // TypeAndFlags: EXT (0x3F), ignore flag (bit 9)
  uint16_t flags = 1 << 9;
  writeU16BE(data, makeTypeAndFlags(FrameType::EXT, flags));

  // Extended type = 0x00000001
  writeU32BE(data, 0x00000001);

  // Payload: "ext payload"
  const std::string payload = "ext payload";
  data.insert(data.end(), payload.begin(), payload.end());

  auto frame = parseFrame(makeFrameBuffer(data));

  EXPECT_TRUE(frame.isValid());
  EXPECT_EQ(frame.type(), FrameType::EXT);
  EXPECT_EQ(frame.streamId(), 60);
  EXPECT_TRUE(frame.metadata.shouldIgnore());
}

// ============================================================================
// Descriptor Validation Tests
// ============================================================================

TEST(FrameParserTest, ConnectionFrameHasZeroStreamId) {
  std::vector<uint8_t> data;

  // Stream ID = 0 (connection-level)
  writeU32BE(data, 0);

  // TypeAndFlags: KEEPALIVE (0x03), no flags
  writeU16BE(data, makeTypeAndFlags(FrameType::KEEPALIVE, 0));

  // Last received position
  uint64_t lastPos = 0;
  for (int i = 0; i < 8; ++i) {
    data.push_back(static_cast<uint8_t>(lastPos >> (56 - i * 8)));
  }

  auto frame = parseFrame(makeFrameBuffer(data));

  EXPECT_TRUE(frame.isValid());
  EXPECT_TRUE(frame.metadata.isConnectionFrame());
  EXPECT_EQ(frame.streamId(), 0);
}

TEST(FrameParserTest, StreamFrameHasNonZeroStreamId) {
  std::vector<uint8_t> data;

  // Stream ID = 100
  writeU32BE(data, 100);

  // TypeAndFlags: REQUEST_RESPONSE (0x04), no flags
  writeU16BE(data, makeTypeAndFlags(FrameType::REQUEST_RESPONSE, 0));

  auto frame = parseFrame(makeFrameBuffer(data));

  EXPECT_TRUE(frame.isValid());
  EXPECT_FALSE(frame.metadata.isConnectionFrame());
  EXPECT_EQ(frame.streamId(), 100);
}
