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

//
// Security regression tests for T264699605:
// Integer underflow in AlignedParserStrategy allows pre-auth crash via
// attacker-controlled frame/metadata lengths.
//
// These tests craft raw malformed Rocket frames to verify that the parser
// rejects undersized frames and oversized metadata claims without underflow.
//

#include <cstring>

#include <gtest/gtest.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/framing/parser/AlignedParserStrategy.h>
#include <thrift/lib/cpp2/transport/rocket/framing/parser/test/TestUtil.h>

namespace apache::thrift::rocket {

using State = detail::aligned_parser::State;

namespace {

// Wire encoding helpers for crafting raw attack frames.
void writeUint24BE(uint8_t* buf, uint32_t val) {
  buf[0] = (val >> 16) & 0xFF;
  buf[1] = (val >> 8) & 0xFF;
  buf[2] = val & 0xFF;
}

void writeUint32BE(uint8_t* buf, uint32_t val) {
  buf[0] = (val >> 24) & 0xFF;
  buf[1] = (val >> 16) & 0xFF;
  buf[2] = (val >> 8) & 0xFF;
  buf[3] = val & 0xFF;
}

void writeUint16BE(uint8_t* buf, uint16_t val) {
  buf[0] = (val >> 8) & 0xFF;
  buf[1] = val & 0xFF;
}

constexpr size_t kHeaderSize = Serializer::kMinimumFrameHeaderLength; // 9
constexpr size_t kMetaLenFieldSize = Serializer::kBytesForFrameOrMetadataLength;
constexpr uint16_t kMetadataFlag = 1 << 8; // Flags::Bits::METADATA

// Craft a raw 9-byte Rocket frame header.
// frameLength: the 3-byte frame length field (content size after the 3-byte
//              length prefix)
// streamId: RSocket stream ID
// frameType: RSocket frame type enum
// flags: 10-bit flag field (e.g., kMetadataFlag for metadata present)
std::array<uint8_t, kHeaderSize> craftHeader(
    uint32_t frameLength,
    uint32_t streamId,
    FrameType frameType,
    uint16_t flags = 0) {
  std::array<uint8_t, kHeaderSize> header{};
  writeUint24BE(header.data(), frameLength);
  writeUint32BE(header.data() + 3, streamId);
  uint16_t frameTypeAndFlags =
      (static_cast<uint8_t>(frameType) << Flags::kBits) | flags;
  writeUint16BE(header.data() + 7, frameTypeAndFlags);
  return header;
}

// Craft a 3-byte metadata length field.
std::array<uint8_t, kMetaLenFieldSize> craftMetadataLength(uint32_t len) {
  std::array<uint8_t, kMetaLenFieldSize> buf{};
  writeUint24BE(buf.data(), len);
  return buf;
}

// Feed exactly len bytes from data into the parser via the
// getReadBuffer/readDataAvailable protocol.
void feedBytes(
    AlignedParserStrategy<FakeOwner>& parser, const uint8_t* data, size_t len) {
  void* buf;
  size_t lenReturn;
  parser.getReadBuffer(&buf, &lenReturn);
  ASSERT_GE(lenReturn, len);
  std::memcpy(buf, data, len);
  parser.readDataAvailable(len);
}

// Feed a complete valid CANCEL frame (no data/metadata, simplest valid frame).
// Used to verify parser recovery after rejecting a malformed frame.
void feedValidCancelFrame(AlignedParserStrategy<FakeOwner>& parser) {
  CancelFrame cancelFrame(StreamId(1));
  auto iobuf = std::move(cancelFrame).serialize();
  const folly::ByteRange frameBuf = iobuf->coalesce();
  feedBytes(parser, frameBuf.data(), kHeaderSize);
}

} // namespace

// ---------------------------------------------------------------------------
// Underflow Site 1: configureAligned() — no metadata path
//   remainingData_ = frameLength_ - kHeaderLength
//   Underflows when frameLength_ < kHeaderLength (6)
// ---------------------------------------------------------------------------

TEST(AlignedParserSecurityTest, ZeroFrameLength_RequestResponse_Throws) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  auto header = craftHeader(
      /*frameLength=*/0,
      /*streamId=*/1,
      FrameType::REQUEST_RESPONSE);
  EXPECT_THROW(
      feedBytes(parser, header.data(), kHeaderSize), std::runtime_error);
  EXPECT_EQ(owner.frames_.size(), 0);
}

TEST(AlignedParserSecurityTest, ZeroFrameLength_Payload_Throws) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  auto header = craftHeader(
      /*frameLength=*/0,
      /*streamId=*/1,
      FrameType::PAYLOAD);
  EXPECT_THROW(
      feedBytes(parser, header.data(), kHeaderSize), std::runtime_error);
  EXPECT_EQ(owner.frames_.size(), 0);
}

TEST(AlignedParserSecurityTest, FrameLengthOne_Throws) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  auto header = craftHeader(
      /*frameLength=*/1,
      /*streamId=*/1,
      FrameType::REQUEST_RESPONSE);
  EXPECT_THROW(
      feedBytes(parser, header.data(), kHeaderSize), std::runtime_error);
}

TEST(AlignedParserSecurityTest, FrameLengthFive_OneBelow_Minimum_Throws) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  // kHeaderLength is 6; frameLength=5 is one byte short
  auto header = craftHeader(
      /*frameLength=*/5,
      /*streamId=*/1,
      FrameType::REQUEST_RESPONSE);
  EXPECT_THROW(
      feedBytes(parser, header.data(), kHeaderSize), std::runtime_error);
}

// ---------------------------------------------------------------------------
// Underflow Site 2: configuredNonAligned()
//   remainingUnaligned_ = frameLength_ - kHeaderLength
//   Same underflow for non-aligned frame types (SETUP, KEEPALIVE, etc.)
// ---------------------------------------------------------------------------

TEST(AlignedParserSecurityTest, ZeroFrameLength_Setup_NonAlignedPath_Throws) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  auto header = craftHeader(
      /*frameLength=*/0,
      /*streamId=*/0, // SETUP uses stream 0
      FrameType::SETUP);
  EXPECT_THROW(
      feedBytes(parser, header.data(), kHeaderSize), std::runtime_error);
  EXPECT_EQ(owner.frames_.size(), 0);
}

TEST(
    AlignedParserSecurityTest,
    FrameLengthFive_Keepalive_NonAlignedPath_Throws) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  auto header = craftHeader(
      /*frameLength=*/5,
      /*streamId=*/0, // KEEPALIVE uses stream 0
      FrameType::KEEPALIVE);
  EXPECT_THROW(
      feedBytes(parser, header.data(), kHeaderSize), std::runtime_error);
}

TEST(AlignedParserSecurityTest, FrameLengthFive_Error_NonAlignedPath_Throws) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  auto header = craftHeader(
      /*frameLength=*/5,
      /*streamId=*/1,
      FrameType::ERROR);
  EXPECT_THROW(
      feedBytes(parser, header.data(), kHeaderSize), std::runtime_error);
}

// ---------------------------------------------------------------------------
// Underflow Site 3: handleAwaitingMetadataLength()
//   remainingData_ = frameLength_ - kHeaderLength
//                     - kBytesForFrameOrMetadataLength - remainingMetadata_
//   Underflows when metadata length exceeds available budget.
// ---------------------------------------------------------------------------

TEST(
    AlignedParserSecurityTest,
    MetadataFlagSet_FrameTooSmallForMetadataLenField_Throws) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  // frameLength=6 (kHeaderLength): room for header only, not the 3-byte
  // metadata length field
  auto header = craftHeader(
      /*frameLength=*/6,
      /*streamId=*/1,
      FrameType::REQUEST_RESPONSE,
      kMetadataFlag);
  EXPECT_THROW(
      feedBytes(parser, header.data(), kHeaderSize), std::runtime_error);
  EXPECT_EQ(owner.frames_.size(), 0);
}

TEST(
    AlignedParserSecurityTest,
    MetadataFlagSet_FrameOneByteShortForMetadataField_Throws) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  // Minimum for metadata path: kHeaderLength + kMetaLenFieldSize = 6 + 3 = 9
  // So frameLength=8 is one byte short
  auto header = craftHeader(
      /*frameLength=*/8,
      /*streamId=*/1,
      FrameType::REQUEST_RESPONSE,
      kMetadataFlag);
  EXPECT_THROW(
      feedBytes(parser, header.data(), kHeaderSize), std::runtime_error);
}

TEST(AlignedParserSecurityTest, MetadataLengthExceedsFrameBudget_Throws) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  // frameLength=20, budget = 20 - 6 - 3 = 11
  // Set metadata length to 12 (one more than budget)
  auto header = craftHeader(
      /*frameLength=*/20,
      /*streamId=*/1,
      FrameType::REQUEST_RESPONSE,
      kMetadataFlag);
  // Feed the 9-byte header (transitions to AwaitingMetadataLength)
  feedBytes(parser, header.data(), kHeaderSize);
  EXPECT_EQ(parser.state(), State::AwaitingMetadataLength);

  // Feed the 3-byte metadata length field with value 12 (exceeds budget of 11)
  auto metaLen = craftMetadataLength(12);
  EXPECT_THROW(
      feedBytes(parser, metaLen.data(), kMetaLenFieldSize), std::runtime_error);
  EXPECT_EQ(owner.frames_.size(), 0);
}

TEST(AlignedParserSecurityTest, MetadataLengthMaxValue_0xFFFFFF_Throws) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  // frameLength=20 is a valid-looking frame, but metadata claims 0xFFFFFF
  // (16,777,215 bytes). Budget is only 20-6-3=11. This is the researcher's
  // primary attack: a small frame with a massive metadata length that causes
  // SIZE_MAX underflow → crash via memcpy(buf, data, SIZE_MAX).
  auto header = craftHeader(
      /*frameLength=*/20,
      /*streamId=*/1,
      FrameType::REQUEST_RESPONSE,
      kMetadataFlag);
  feedBytes(parser, header.data(), kHeaderSize);
  EXPECT_EQ(parser.state(), State::AwaitingMetadataLength);

  auto metaLen = craftMetadataLength(0xFFFFFF);
  EXPECT_THROW(
      feedBytes(parser, metaLen.data(), kMetaLenFieldSize), std::runtime_error);
  EXPECT_EQ(owner.frames_.size(), 0);
}

TEST(AlignedParserSecurityTest, MetadataLengthExactlyOneBeyondBudget_Throws) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  // frameLength=9, budget = 9-6-3 = 0. Metadata length = 1 → exceeds budget.
  auto header = craftHeader(
      /*frameLength=*/9,
      /*streamId=*/1,
      FrameType::REQUEST_RESPONSE,
      kMetadataFlag);
  feedBytes(parser, header.data(), kHeaderSize);

  auto metaLen = craftMetadataLength(1);
  EXPECT_THROW(
      feedBytes(parser, metaLen.data(), kMetaLenFieldSize), std::runtime_error);
}

TEST(AlignedParserSecurityTest, MetadataLength_Payload_ExceedsBudget_Throws) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  // Same attack but with PAYLOAD frame type (also alignable)
  auto header = craftHeader(
      /*frameLength=*/15,
      /*streamId=*/1,
      FrameType::PAYLOAD,
      kMetadataFlag);
  feedBytes(parser, header.data(), kHeaderSize);
  EXPECT_EQ(parser.state(), State::AwaitingMetadataLength);

  // budget = 15 - 6 - 3 = 6, metadata claims 100
  auto metaLen = craftMetadataLength(100);
  EXPECT_THROW(
      feedBytes(parser, metaLen.data(), kMetaLenFieldSize), std::runtime_error);
}

// ---------------------------------------------------------------------------
// Parser recovery: after rejecting a malformed frame, the parser must reset
// to a clean state and successfully parse subsequent valid frames.
// A corrupt post-rejection state would be a secondary vulnerability.
// ---------------------------------------------------------------------------

TEST(
    AlignedParserSecurityTest,
    RecoveryAfterUndersizedFrame_ParsesValidFrameSuccessfully) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);

  // Send a malformed frame (underflow attempt)
  auto badHeader = craftHeader(
      /*frameLength=*/0,
      /*streamId=*/1,
      FrameType::REQUEST_RESPONSE);
  EXPECT_THROW(
      feedBytes(parser, badHeader.data(), kHeaderSize), std::runtime_error);
  EXPECT_EQ(owner.frames_.size(), 0);

  // Parser should be reset to initial state
  EXPECT_EQ(parser.state(), State::AwaitingHeader);

  // Now send a valid CANCEL frame — parser should handle it cleanly
  feedValidCancelFrame(parser);
  EXPECT_EQ(owner.frames_.size(), 1);
}

TEST(
    AlignedParserSecurityTest,
    RecoveryAfterBadMetadataLength_ParsesValidFrameSuccessfully) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);

  // Send a frame with oversized metadata
  auto header = craftHeader(
      /*frameLength=*/20,
      /*streamId=*/1,
      FrameType::REQUEST_RESPONSE,
      kMetadataFlag);
  feedBytes(parser, header.data(), kHeaderSize);

  auto metaLen = craftMetadataLength(0xFFFFFF);
  EXPECT_THROW(
      feedBytes(parser, metaLen.data(), kMetaLenFieldSize), std::runtime_error);
  EXPECT_EQ(owner.frames_.size(), 0);
  EXPECT_EQ(parser.state(), State::AwaitingHeader);

  // Recover: valid frame should parse
  feedValidCancelFrame(parser);
  EXPECT_EQ(owner.frames_.size(), 1);
}

TEST(AlignedParserSecurityTest, RepeatedMalformedFrames_ParserNeverCorrupts) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);

  // Hammer the parser with 100 malformed frames — it must never corrupt
  for (int i = 0; i < 100; ++i) {
    auto badHeader = craftHeader(
        /*frameLength=*/i % 6, // 0 through 5, all < kHeaderLength
        /*streamId=*/1,
        FrameType::REQUEST_RESPONSE);
    EXPECT_THROW(
        feedBytes(parser, badHeader.data(), kHeaderSize), std::runtime_error);
    EXPECT_EQ(parser.state(), State::AwaitingHeader);
    EXPECT_EQ(owner.frames_.size(), 0);
  }

  // Still recovers after sustained attack
  feedValidCancelFrame(parser);
  EXPECT_EQ(owner.frames_.size(), 1);
}

// ---------------------------------------------------------------------------
// Boundary tests
// Boundary tests: valid frames at the exact minimum sizes.
// Confirm the fix doesn't reject legitimate traffic.
// ---------------------------------------------------------------------------

TEST(AlignedParserSecurityTest, ExactMinimumFrameLength_Cancel_Succeeds) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  // CANCEL has frameLength=6 (kHeaderLength) with no data — this is the
  // minimum valid frame.
  CancelFrame cancelFrame(StreamId(1));
  auto iobuf = std::move(cancelFrame).serialize();
  const folly::ByteRange frameBuf = iobuf->coalesce();
  feedBytes(parser, frameBuf.data(), kHeaderSize);
  EXPECT_EQ(owner.frames_.size(), 1);
  EXPECT_EQ(parser.state(), State::AwaitingHeader);
}

TEST(
    AlignedParserSecurityTest,
    ExactMinimumFrameLength_RequestResponseNoMetadata_Succeeds) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  // REQUEST_RESPONSE with no metadata: minimum frameLength is kHeaderLength=6
  // (0 data bytes). This exercises the boundary case where
  // remainingData_ = 6 - 6 = 0.
  auto header = craftHeader(
      /*frameLength=*/6,
      /*streamId=*/1,
      FrameType::REQUEST_RESPONSE);

  // Should not throw — this is the exact boundary
  EXPECT_NO_THROW(feedBytes(parser, header.data(), kHeaderSize));
  // Parser transitions to AwaitingData with 0 remaining bytes.
  // It awaits a readDataAvailable(0) call which would trigger submitFrame.
  EXPECT_EQ(parser.state(), State::AwaitingData);
  EXPECT_EQ(parser.remainingData(), 0);
}

TEST(AlignedParserSecurityTest, MetadataLengthExactlyEqualsBudget_Succeeds) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  // frameLength=14, budget = 14 - 6 - 3 = 5
  // Metadata length = 5 (exactly equals budget), data = 0
  auto header = craftHeader(
      /*frameLength=*/14,
      /*streamId=*/1,
      FrameType::REQUEST_RESPONSE,
      kMetadataFlag);
  feedBytes(parser, header.data(), kHeaderSize);
  EXPECT_EQ(parser.state(), State::AwaitingMetadataLength);

  auto metaLen = craftMetadataLength(5);
  EXPECT_NO_THROW(feedBytes(parser, metaLen.data(), kMetaLenFieldSize));
  EXPECT_EQ(parser.state(), State::AwaitingMetadata);
  EXPECT_EQ(parser.remainingMetadata(), 5);
  EXPECT_EQ(parser.remainingData(), 0);
}

TEST(AlignedParserSecurityTest, MetadataLengthZero_WithMetadataFlag_Succeeds) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  // frameLength=12, metadata flag set but metadata length = 0.
  // budget = 12 - 6 - 3 = 3, metadata = 0, data = 3
  auto header = craftHeader(
      /*frameLength=*/12,
      /*streamId=*/1,
      FrameType::REQUEST_RESPONSE,
      kMetadataFlag);
  feedBytes(parser, header.data(), kHeaderSize);

  auto metaLen = craftMetadataLength(0);
  EXPECT_NO_THROW(feedBytes(parser, metaLen.data(), kMetaLenFieldSize));
  // With 0 metadata, should skip to data
  EXPECT_EQ(parser.remainingMetadata(), 0);
  EXPECT_EQ(parser.remainingData(), 3);
}

TEST(
    AlignedParserSecurityTest,
    NonAlignedPath_ExactMinimumFrameLength_Succeeds) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  // SETUP with frameLength=6 (exact minimum): remainingUnaligned = 6 - 6 = 0
  auto header = craftHeader(
      /*frameLength=*/6,
      /*streamId=*/0,
      FrameType::SETUP);
  EXPECT_NO_THROW(feedBytes(parser, header.data(), kHeaderSize));
  // Transitions to AwaitingNonAligned with 0 remaining bytes.
  EXPECT_EQ(parser.state(), State::AwaitingNonAligned);
  EXPECT_EQ(parser.remainingUnaligned(), 0);
}

// ---------------------------------------------------------------------------
// Adversarial edge cases: values designed to probe for off-by-one errors
// or alternative underflow paths.
// ---------------------------------------------------------------------------

TEST(AlignedParserSecurityTest, FrameLengthMaxUint24_NoMetadata_NoUnderflow) {
  FakeOwner owner;
  AlignedParserStrategy<FakeOwner> parser(owner);
  // Maximum 3-byte frame length: 0xFFFFFF (16,777,215)
  // This is a huge frame but the subtraction won't underflow.
  // The parser should accept the header and transition to AwaitingData
  // with remainingData_ = 0xFFFFFF - 6 = 16,777,209.
  auto header = craftHeader(
      /*frameLength=*/0xFFFFFF,
      /*streamId=*/1,
      FrameType::REQUEST_RESPONSE);
  EXPECT_NO_THROW(feedBytes(parser, header.data(), kHeaderSize));
  EXPECT_EQ(parser.state(), State::AwaitingData);
  EXPECT_EQ(parser.remainingData(), 0xFFFFFF - 6);
}

TEST(AlignedParserSecurityTest, AllNonAlignedFrameTypes_ZeroLength_AllThrow) {
  // Exhaustively verify that ALL non-aligned frame types reject zero-length
  // frames. This prevents a new frame type from accidentally bypassing checks.
  const std::vector<std::pair<FrameType, uint32_t>> nonAlignedTypes = {
      {FrameType::SETUP, 0},
      {FrameType::KEEPALIVE, 0},
      {FrameType::REQUEST_FNF, 1},
      {FrameType::REQUEST_STREAM, 1},
      {FrameType::REQUEST_CHANNEL, 1},
      {FrameType::REQUEST_N, 1},
      {FrameType::ERROR, 1},
      {FrameType::METADATA_PUSH, 0},
      {FrameType::EXT, 1},
  };

  for (auto [frameType, streamId] : nonAlignedTypes) {
    FakeOwner owner;
    AlignedParserStrategy<FakeOwner> parser(owner);
    auto header = craftHeader(/*frameLength=*/0, streamId, frameType);
    EXPECT_THROW(
        feedBytes(parser, header.data(), kHeaderSize), std::runtime_error)
        << "Expected throw for frame type " << static_cast<int>(frameType)
        << " with zero length";
    EXPECT_EQ(owner.frames_.size(), 0);
  }
}

TEST(AlignedParserSecurityTest, AllAlignableFrameTypes_ZeroLength_AllThrow) {
  // Verify both alignable types reject zero-length frames
  for (auto frameType : {FrameType::REQUEST_RESPONSE, FrameType::PAYLOAD}) {
    FakeOwner owner;
    AlignedParserStrategy<FakeOwner> parser(owner);
    auto header = craftHeader(
        /*frameLength=*/0, /*streamId=*/1, frameType);
    EXPECT_THROW(
        feedBytes(parser, header.data(), kHeaderSize), std::runtime_error)
        << "Expected throw for frame type " << static_cast<int>(frameType);
  }
}

} // namespace apache::thrift::rocket
