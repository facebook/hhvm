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

#include <thrift/lib/cpp2/fast_thrift/frame/FrameDescriptor.h>

#include <gtest/gtest.h>

namespace apache::thrift::fast_thrift::frame {
namespace {

TEST(FrameDescriptorTest, GetDescriptorReturnsCorrectType) {
  EXPECT_EQ(
      getDescriptor(FrameType::REQUEST_RESPONSE).type,
      FrameType::REQUEST_RESPONSE);
  EXPECT_EQ(
      getDescriptor(FrameType::REQUEST_STREAM).type, FrameType::REQUEST_STREAM);
  EXPECT_EQ(getDescriptor(FrameType::PAYLOAD).type, FrameType::PAYLOAD);
  EXPECT_EQ(getDescriptor(FrameType::ERROR).type, FrameType::ERROR);
  EXPECT_EQ(getDescriptor(FrameType::KEEPALIVE).type, FrameType::KEEPALIVE);
}

TEST(FrameDescriptorTest, HeaderSizesAreCorrect) {
  // Base header size: streamId (4) + typeAndFlags (2) = 6
  EXPECT_EQ(getDescriptor(FrameType::REQUEST_RESPONSE).headerSize, 6);
  EXPECT_EQ(getDescriptor(FrameType::PAYLOAD).headerSize, 6);
  EXPECT_EQ(getDescriptor(FrameType::CANCEL).headerSize, 6);

  // Extended header sizes
  EXPECT_EQ(
      getDescriptor(FrameType::REQUEST_STREAM).headerSize,
      10); // +4 for initialRequestN
  EXPECT_EQ(
      getDescriptor(FrameType::REQUEST_CHANNEL).headerSize,
      10); // +4 for initialRequestN
  EXPECT_EQ(
      getDescriptor(FrameType::REQUEST_N).headerSize, 10); // +4 for requestN
  EXPECT_EQ(getDescriptor(FrameType::ERROR).headerSize, 10); // +4 for errorCode
  EXPECT_EQ(
      getDescriptor(FrameType::KEEPALIVE).headerSize,
      14); // +8 for lastPosition
}

TEST(FrameDescriptorTest, RequestFrameFlags) {
  EXPECT_TRUE(getDescriptor(FrameType::REQUEST_RESPONSE).isRequestFrame);
  EXPECT_TRUE(getDescriptor(FrameType::REQUEST_FNF).isRequestFrame);
  EXPECT_TRUE(getDescriptor(FrameType::REQUEST_STREAM).isRequestFrame);
  EXPECT_TRUE(getDescriptor(FrameType::REQUEST_CHANNEL).isRequestFrame);

  EXPECT_FALSE(getDescriptor(FrameType::PAYLOAD).isRequestFrame);
  EXPECT_FALSE(getDescriptor(FrameType::CANCEL).isRequestFrame);
  EXPECT_FALSE(getDescriptor(FrameType::ERROR).isRequestFrame);
  EXPECT_FALSE(getDescriptor(FrameType::KEEPALIVE).isRequestFrame);
}

TEST(FrameDescriptorTest, StreamZeroFrames) {
  EXPECT_TRUE(getDescriptor(FrameType::SETUP).isStreamZeroOnly);
  EXPECT_TRUE(getDescriptor(FrameType::KEEPALIVE).isStreamZeroOnly);
  EXPECT_TRUE(getDescriptor(FrameType::METADATA_PUSH).isStreamZeroOnly);

  EXPECT_FALSE(getDescriptor(FrameType::REQUEST_RESPONSE).isStreamZeroOnly);
  EXPECT_FALSE(getDescriptor(FrameType::PAYLOAD).isStreamZeroOnly);
}

TEST(FrameDescriptorTest, IsValidFrameType) {
  EXPECT_TRUE(isValidFrameType(FrameType::REQUEST_RESPONSE));
  EXPECT_TRUE(isValidFrameType(FrameType::PAYLOAD));
  EXPECT_TRUE(isValidFrameType(FrameType::ERROR));
  EXPECT_TRUE(isValidFrameType(FrameType::KEEPALIVE));
  EXPECT_TRUE(isValidFrameType(FrameType::EXT));

  EXPECT_FALSE(isValidFrameType(FrameType::RESERVED));
}

TEST(FrameDescriptorTest, DescriptorNames) {
  EXPECT_STREQ(
      getDescriptor(FrameType::REQUEST_RESPONSE).name, "REQUEST_RESPONSE");
  EXPECT_STREQ(getDescriptor(FrameType::PAYLOAD).name, "PAYLOAD");
  EXPECT_STREQ(getDescriptor(FrameType::ERROR).name, "ERROR");
}

// ============================================================================
// Flag Constants Tests (internal encoding details)
// ============================================================================

TEST(FlagConstantsTest, FlagBitsAndMask) {
  // Flags use 10 bits
  EXPECT_EQ(frame::detail::kFlagsBits, 10);
  EXPECT_EQ(frame::detail::kFlagsMask, 0x3FF);
}

TEST(FlagConstantsTest, CommonFlagBitPositions) {
  // Common flags
  EXPECT_EQ(frame::detail::kMetadataBit, 1 << 8);
  EXPECT_EQ(frame::detail::kFollowsBit, 1 << 7);
  EXPECT_EQ(frame::detail::kCompleteBit, 1 << 6);
  EXPECT_EQ(frame::detail::kNextBit, 1 << 5);
}

TEST(FlagConstantsTest, ContextSpecificFlagBitPositions) {
  // Context-specific flags (same bit positions, different meanings)
  EXPECT_EQ(frame::detail::kRespondBit, 1 << 7); // KEEPALIVE
  EXPECT_EQ(frame::detail::kIgnoreBit, 1 << 9); // EXT
  EXPECT_EQ(frame::detail::kLeaseBit, 1 << 6); // SETUP
  EXPECT_EQ(frame::detail::kResumeTokenBit, 1 << 7); // SETUP
}

TEST(FlagConstantsTest, FlagMaskingPreservesOnlyTenBits) {
  // Verify masking works correctly
  uint16_t allBits = 0xFFFF;
  uint16_t masked = allBits & frame::detail::kFlagsMask;
  EXPECT_EQ(masked, 0x3FF);
}

} // namespace
} // namespace apache::thrift::fast_thrift::frame
