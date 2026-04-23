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
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameViews.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>

#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>

using namespace apache::thrift::fast_thrift::frame;
using namespace apache::thrift::fast_thrift::frame::read;
using namespace apache::thrift::fast_thrift::frame::write;

// ============================================================================
// REQUEST_RESPONSE Round-Trip Tests
// ============================================================================

TEST(FrameWriterTest, SerializeRequestResponse) {
  auto metadata = folly::IOBuf::copyBuffer("metadata");
  auto data = folly::IOBuf::copyBuffer("payload");

  auto frame = serialize(
      RequestResponseHeader{.streamId = 1},
      std::move(metadata),
      std::move(data));

  auto parsed = parseFrame(std::move(frame));

  EXPECT_TRUE(parsed.isValid());
  EXPECT_EQ(parsed.type(), FrameType::REQUEST_RESPONSE);
  EXPECT_EQ(parsed.streamId(), 1u);
  EXPECT_TRUE(parsed.hasMetadata());
  EXPECT_EQ(parsed.metadataSize(), 8u);
  EXPECT_EQ(parsed.dataSize(), 7u);
}

TEST(FrameWriterTest, SerializeRequestResponseWithFollows) {
  auto data = folly::IOBuf::copyBuffer("fragment");

  auto frame = serialize(
      RequestResponseHeader{.streamId = 42, .follows = true},
      nullptr,
      std::move(data));

  auto parsed = parseFrame(std::move(frame));

  EXPECT_EQ(parsed.type(), FrameType::REQUEST_RESPONSE);
  EXPECT_EQ(parsed.streamId(), 42u);
  EXPECT_TRUE(parsed.hasFollows());
  EXPECT_FALSE(parsed.hasMetadata());
}

TEST(FrameWriterTest, SerializeRequestResponseNoPayload) {
  auto frame =
      serialize(RequestResponseHeader{.streamId = 100}, nullptr, nullptr);

  auto parsed = parseFrame(std::move(frame));

  EXPECT_EQ(parsed.type(), FrameType::REQUEST_RESPONSE);
  EXPECT_EQ(parsed.streamId(), 100u);
  EXPECT_EQ(parsed.payloadSize(), 0u);
}

// ============================================================================
// REQUEST_FNF Round-Trip Tests
// ============================================================================

TEST(FrameWriterTest, SerializeRequestFnf) {
  auto data = folly::IOBuf::copyBuffer("fire and forget");

  auto frame =
      serialize(RequestFnfHeader{.streamId = 50}, nullptr, std::move(data));

  auto parsed = parseFrame(std::move(frame));

  EXPECT_EQ(parsed.type(), FrameType::REQUEST_FNF);
  EXPECT_EQ(parsed.streamId(), 50u);
  EXPECT_EQ(parsed.dataSize(), 15u);
}

// ============================================================================
// REQUEST_STREAM Round-Trip Tests
// ============================================================================

TEST(FrameWriterTest, SerializeRequestStream) {
  auto metadata = folly::IOBuf::copyBuffer("meta");
  auto data = folly::IOBuf::copyBuffer("request data");

  auto frame = serialize(
      RequestStreamHeader{.streamId = 42, .initialRequestN = 100},
      std::move(metadata),
      std::move(data));

  auto parsed = parseFrame(std::move(frame));

  EXPECT_TRUE(parsed.isValid());
  EXPECT_EQ(parsed.type(), FrameType::REQUEST_STREAM);
  EXPECT_EQ(parsed.streamId(), 42u);
  EXPECT_TRUE(parsed.hasMetadata());

  RequestStreamView view(parsed);
  EXPECT_EQ(view.initialRequestN(), 100u);
}

TEST(FrameWriterTest, SerializeRequestStreamMaxRequestN) {
  auto frame = serialize(
      RequestStreamHeader{.streamId = 1, .initialRequestN = 0xFFFFFFFF},
      nullptr,
      nullptr);

  auto parsed = parseFrame(std::move(frame));

  RequestStreamView view(parsed);
  EXPECT_EQ(view.initialRequestN(), 0xFFFFFFFFu);
}

// ============================================================================
// REQUEST_CHANNEL Round-Trip Tests
// ============================================================================

TEST(FrameWriterTest, SerializeRequestChannel) {
  auto data = folly::IOBuf::copyBuffer("channel data");

  auto frame = serialize(
      RequestChannelHeader{
          .streamId = 51, .initialRequestN = 1000, .complete = true},
      nullptr,
      std::move(data));

  auto parsed = parseFrame(std::move(frame));

  EXPECT_EQ(parsed.type(), FrameType::REQUEST_CHANNEL);
  EXPECT_EQ(parsed.streamId(), 51u);
  EXPECT_TRUE(parsed.isComplete());

  RequestChannelView view(parsed);
  EXPECT_EQ(view.initialRequestN(), 1000u);
}

TEST(FrameWriterTest, SerializeRequestChannelWithAllFlags) {
  auto frame = serialize(
      RequestChannelHeader{
          .streamId = 52,
          .initialRequestN = 500,
          .follows = true,
          .complete = true},
      nullptr,
      nullptr);

  auto parsed = parseFrame(std::move(frame));

  EXPECT_TRUE(parsed.hasFollows());
  EXPECT_TRUE(parsed.isComplete());
}

// ============================================================================
// REQUEST_N Round-Trip Tests
// ============================================================================

TEST(FrameWriterTest, SerializeRequestN) {
  auto frame = serialize(RequestNHeader{.streamId = 8, .requestN = 50});

  auto parsed = parseFrame(std::move(frame));

  EXPECT_EQ(parsed.type(), FrameType::REQUEST_N);
  EXPECT_EQ(parsed.streamId(), 8u);
  EXPECT_EQ(parsed.payloadSize(), 0u);

  RequestNView view(parsed);
  EXPECT_EQ(view.requestN(), 50u);
}

TEST(FrameWriterTest, SerializeRequestNMaxValue) {
  auto frame = serialize(RequestNHeader{.streamId = 9, .requestN = 0x7FFFFFFF});

  auto parsed = parseFrame(std::move(frame));

  RequestNView view(parsed);
  EXPECT_EQ(view.requestN(), 0x7FFFFFFFu);
}

// ============================================================================
// CANCEL Round-Trip Tests
// ============================================================================

TEST(FrameWriterTest, SerializeCancel) {
  auto frame = serialize(CancelHeader{.streamId = 7});

  auto parsed = parseFrame(std::move(frame));

  EXPECT_EQ(parsed.type(), FrameType::CANCEL);
  EXPECT_EQ(parsed.streamId(), 7u);
  EXPECT_EQ(parsed.payloadSize(), 0u);
}

// ============================================================================
// PAYLOAD Round-Trip Tests
// ============================================================================

TEST(FrameWriterTest, SerializePayload) {
  auto metadata = folly::IOBuf::copyBuffer("meta");
  auto data = folly::IOBuf::copyBuffer("response");

  auto frame = serialize(
      PayloadHeader{.streamId = 5, .complete = true, .next = true},
      std::move(metadata),
      std::move(data));

  auto parsed = parseFrame(std::move(frame));

  EXPECT_EQ(parsed.type(), FrameType::PAYLOAD);
  EXPECT_EQ(parsed.streamId(), 5u);
  EXPECT_TRUE(parsed.isComplete());
  EXPECT_TRUE(parsed.hasNext());
  EXPECT_TRUE(parsed.hasMetadata());
  EXPECT_EQ(parsed.metadataSize(), 4u);
  EXPECT_EQ(parsed.dataSize(), 8u);
}

TEST(FrameWriterTest, SerializePayloadAllFlags) {
  auto frame = serialize(
      PayloadHeader{
          .streamId = 10, .follows = true, .complete = true, .next = true},
      nullptr,
      nullptr);

  auto parsed = parseFrame(std::move(frame));

  EXPECT_TRUE(parsed.hasFollows());
  EXPECT_TRUE(parsed.isComplete());
  EXPECT_TRUE(parsed.hasNext());
}

TEST(FrameWriterTest, SerializePayloadDataOnly) {
  auto data = folly::IOBuf::copyBuffer("just data");

  auto frame = serialize(
      PayloadHeader{.streamId = 11, .next = true}, nullptr, std::move(data));

  auto parsed = parseFrame(std::move(frame));

  EXPECT_FALSE(parsed.hasMetadata());
  EXPECT_EQ(parsed.metadataSize(), 0u);
  EXPECT_EQ(parsed.dataSize(), 9u);

  // Verify data content via cursor
  auto cursor = parsed.dataCursor();
  std::string result;
  result.resize(parsed.dataSize());
  cursor.pull(result.data(), result.size());
  EXPECT_EQ(result, "just data");
}

// ============================================================================
// ERROR Round-Trip Tests
// ============================================================================

TEST(FrameWriterTest, SerializeError) {
  auto errorMsg = folly::IOBuf::copyBuffer("Something went wrong");

  auto frame = serialize(
      ErrorHeader{.streamId = 3, .errorCode = 0x00000201}, // APPLICATION_ERROR
      nullptr,
      std::move(errorMsg));

  auto parsed = parseFrame(std::move(frame));

  EXPECT_EQ(parsed.type(), FrameType::ERROR);
  EXPECT_EQ(parsed.streamId(), 3u);

  ErrorView view(parsed);
  EXPECT_EQ(view.errorCode(), 0x00000201u);
}

TEST(FrameWriterTest, SerializeErrorWithMetadata) {
  auto metadata = folly::IOBuf::copyBuffer("error_meta");
  auto errorMsg = folly::IOBuf::copyBuffer("error message");

  auto frame = serialize(
      ErrorHeader{.streamId = 4, .errorCode = 0x00000101}, // INVALID_SETUP
      std::move(metadata),
      std::move(errorMsg));

  auto parsed = parseFrame(std::move(frame));

  EXPECT_TRUE(parsed.hasMetadata());

  ErrorView view(parsed);
  EXPECT_EQ(view.errorCode(), 0x00000101u);
}

// ============================================================================
// KEEPALIVE Round-Trip Tests
// ============================================================================

TEST(FrameWriterTest, SerializeKeepAlive) {
  auto frame = serialize(
      KeepAliveHeader{.lastReceivedPosition = 12345678, .respond = true});

  auto parsed = parseFrame(std::move(frame));

  EXPECT_EQ(parsed.type(), FrameType::KEEPALIVE);
  EXPECT_EQ(parsed.streamId(), 0u);
  EXPECT_TRUE(parsed.metadata.shouldRespond());

  KeepAliveView view(parsed);
  EXPECT_EQ(view.lastReceivedPosition(), 12345678u);
}

TEST(FrameWriterTest, SerializeKeepAliveNoRespond) {
  auto frame = serialize(KeepAliveHeader{.lastReceivedPosition = 0});

  auto parsed = parseFrame(std::move(frame));

  EXPECT_FALSE(parsed.metadata.shouldRespond());

  KeepAliveView view(parsed);
  EXPECT_EQ(view.lastReceivedPosition(), 0u);
}

TEST(FrameWriterTest, SerializeKeepAliveWithData) {
  auto data = folly::IOBuf::copyBuffer("ping");

  auto frame = serialize(KeepAliveHeader{.respond = true}, std::move(data));

  auto parsed = parseFrame(std::move(frame));

  EXPECT_EQ(parsed.type(), FrameType::KEEPALIVE);
  EXPECT_EQ(parsed.dataSize(), 4u);
}

// ============================================================================
// SETUP Round-Trip Tests
// ============================================================================

TEST(FrameWriterTest, SerializeSetup) {
  auto metadata = folly::IOBuf::copyBuffer("setup_meta");
  auto data = folly::IOBuf::copyBuffer("setup_data");

  auto frame = serialize(
      SetupHeader{
          .majorVersion = 1,
          .minorVersion = 0,
          .keepaliveTime = 30000,
          .maxLifetime = 60000},
      std::move(metadata),
      std::move(data));

  auto parsed = parseFrame(std::move(frame));

  EXPECT_EQ(parsed.type(), FrameType::SETUP);
  EXPECT_EQ(parsed.streamId(), 0u);
  EXPECT_TRUE(parsed.hasMetadata());

  SetupView view(parsed);
  EXPECT_EQ(view.majorVersion(), 1u);
  EXPECT_EQ(view.minorVersion(), 0u);
  EXPECT_EQ(view.keepaliveTime(), 30000u);
  EXPECT_EQ(view.maxLifetime(), 60000u);
}

TEST(FrameWriterTest, SerializeSetupWithLease) {
  auto frame = serialize(
      SetupHeader{
          .majorVersion = 1,
          .minorVersion = 0,
          .keepaliveTime = 10000,
          .maxLifetime = 20000,
          .lease = true},
      nullptr,
      nullptr);

  auto parsed = parseFrame(std::move(frame));

  SetupView view(parsed);
  EXPECT_TRUE(view.hasLease());
}

TEST(FrameWriterTest, SerializeSetupContainsMimeTypes) {
  auto frame = serialize(
      SetupHeader{
          .majorVersion = 1,
          .minorVersion = 0,
          .keepaliveTime = 30000,
          .maxLifetime = 60000},
      nullptr,
      nullptr);

  // Read raw frame bytes to verify MIME types are encoded
  auto cursor = folly::io::Cursor(frame.get());

  // Skip: streamId(4) + typeAndFlags(2) + majorVersion(2) + minorVersion(2)
  //       + keepaliveTime(4) + maxLifetime(4) = 18 bytes
  cursor.skip(18);

  // Read metadata MIME type
  uint8_t metadataMimeLen = cursor.read<uint8_t>();
  EXPECT_EQ(metadataMimeLen, 36u);
  std::string metadataMime;
  metadataMime.resize(metadataMimeLen);
  cursor.pull(metadataMime.data(), metadataMimeLen);
  EXPECT_EQ(metadataMime, "application/x-rocket-metadata+binary");

  // Read data MIME type
  uint8_t dataMimeLen = cursor.read<uint8_t>();
  EXPECT_EQ(dataMimeLen, 28u);
  std::string dataMime;
  dataMime.resize(dataMimeLen);
  cursor.pull(dataMime.data(), dataMimeLen);
  EXPECT_EQ(dataMime, "application/x-rocket-payload");
}

// ============================================================================
// METADATA_PUSH Round-Trip Tests
// ============================================================================

TEST(FrameWriterTest, SerializeMetadataPush) {
  auto metadata = folly::IOBuf::copyBuffer("pushed metadata content");

  auto frame = serialize(MetadataPushHeader{}, std::move(metadata));

  auto parsed = parseFrame(std::move(frame));

  EXPECT_EQ(parsed.type(), FrameType::METADATA_PUSH);
  EXPECT_EQ(parsed.streamId(), 0u);
  EXPECT_TRUE(parsed.hasMetadata());
  EXPECT_EQ(parsed.metadataSize(), 23);
  EXPECT_EQ(parsed.payloadSize(), 23);
  EXPECT_EQ(parsed.dataSize(), 0);

  auto cursor = parsed.payloadCursor();
  std::string result;
  result.resize(parsed.payloadSize());
  cursor.pull(result.data(), result.size());
  EXPECT_EQ(result, "pushed metadata content");
}

// ============================================================================
// EXT Round-Trip Tests
// ============================================================================

TEST(FrameWriterTest, SerializeExt) {
  auto data = folly::IOBuf::copyBuffer("extension data");

  auto frame = serialize(
      ExtHeader{.streamId = 60, .extendedType = 0x00000001, .ignore = true},
      nullptr,
      std::move(data));

  auto parsed = parseFrame(std::move(frame));

  EXPECT_EQ(parsed.type(), FrameType::EXT);
  EXPECT_EQ(parsed.streamId(), 60u);
  EXPECT_TRUE(parsed.metadata.shouldIgnore());

  ExtView view(parsed);
  EXPECT_EQ(view.extendedType(), 0x00000001u);
}

TEST(FrameWriterTest, SerializeExtWithMetadata) {
  auto metadata = folly::IOBuf::copyBuffer("ext_meta");
  auto data = folly::IOBuf::copyBuffer("ext_data");

  auto frame = serialize(
      ExtHeader{.streamId = 61, .extendedType = 0x00000002},
      std::move(metadata),
      std::move(data));

  auto parsed = parseFrame(std::move(frame));

  EXPECT_TRUE(parsed.hasMetadata());
  EXPECT_FALSE(parsed.metadata.shouldIgnore());
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST(FrameWriterTest, SerializeWithLargeStreamId) {
  auto frame = serialize(CancelHeader{.streamId = 0xFFFFFFFF});

  auto parsed = parseFrame(std::move(frame));

  EXPECT_EQ(parsed.streamId(), 0xFFFFFFFFu);
}

TEST(FrameWriterTest, SerializeEmptyMetadata) {
  auto metadata = folly::IOBuf::create(0); // Empty buffer
  auto data = folly::IOBuf::copyBuffer("data");

  auto frame = serialize(
      RequestResponseHeader{.streamId = 1},
      std::move(metadata),
      std::move(data));

  auto parsed = parseFrame(std::move(frame));

  // Empty metadata should not set the metadata flag
  EXPECT_FALSE(parsed.hasMetadata());
}

TEST(FrameWriterTest, RoundTripPreservesPayloadContent) {
  auto metadata = folly::IOBuf::copyBuffer("test metadata content");
  auto data = folly::IOBuf::copyBuffer("test data content");

  auto frame = serialize(
      RequestResponseHeader{.streamId = 123},
      std::move(metadata),
      std::move(data));

  auto parsed = parseFrame(std::move(frame));

  // Read back metadata
  auto metaCursor = parsed.metadataCursor();
  std::string metaResult;
  metaResult.resize(parsed.metadataSize());
  metaCursor.pull(metaResult.data(), metaResult.size());
  EXPECT_EQ(metaResult, "test metadata content");

  // Read back data
  auto dataCursor = parsed.dataCursor();
  std::string dataResult;
  dataResult.resize(parsed.dataSize());
  dataCursor.pull(dataResult.data(), dataResult.size());
  EXPECT_EQ(dataResult, "test data content");
}

// ============================================================================
// IOBuf Chain Tests
// ============================================================================

TEST(FrameWriterTest, SerializeWithChainedData) {
  auto data1 = folly::IOBuf::copyBuffer("part1");
  auto data2 = folly::IOBuf::copyBuffer("part2");
  data1->appendToChain(std::move(data2));

  auto frame = serialize(
      PayloadHeader{.streamId = 1, .next = true}, nullptr, std::move(data1));

  auto parsed = parseFrame(std::move(frame));

  EXPECT_EQ(parsed.dataSize(), 10u); // "part1" + "part2"

  auto cursor = parsed.dataCursor();
  std::string result;
  result.resize(10);
  cursor.pull(result.data(), result.size());
  EXPECT_EQ(result, "part1part2");
}
