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

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameMetadata.h>

#include <gtest/gtest.h>

namespace apache::thrift::fast_thrift::frame::read {
namespace {

TEST(FrameMetadataTest, SizeIs32Bytes) {
  // FrameMetadata must be exactly 32 bytes to fit in TypeErasedBox inline
  // storage
  EXPECT_EQ(sizeof(FrameMetadata), 32);
}

TEST(FrameMetadataTest, FitsInTypeErasedBoxInline) {
  // Verify FrameMetadata can be stored inline in TypeErasedBox
  EXPECT_TRUE(
      apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox::fits_inline<
          FrameMetadata>());
}

TEST(FrameMetadataTest, DefaultConstruction) {
  FrameMetadata metadata;

  EXPECT_EQ(metadata.descriptor, nullptr);
  EXPECT_EQ(metadata.streamId, 0);
  EXPECT_EQ(metadata.flags_, 0);
  EXPECT_EQ(metadata.metadataSize, 0);
  EXPECT_EQ(metadata.payloadOffset, 0);
  EXPECT_EQ(metadata.payloadSize, 0);
}

TEST(FrameMetadataTest, TypeAccessorWithNullDescriptor) {
  FrameMetadata metadata;
  EXPECT_EQ(metadata.type(), FrameType::RESERVED);
  EXPECT_STREQ(metadata.typeName(), "UNKNOWN");
}

TEST(FrameMetadataTest, TypeAccessorWithValidDescriptor) {
  FrameMetadata metadata;
  metadata.descriptor = &getDescriptor(FrameType::REQUEST_RESPONSE);

  EXPECT_EQ(metadata.type(), FrameType::REQUEST_RESPONSE);
  EXPECT_STREQ(metadata.typeName(), "REQUEST_RESPONSE");
}

TEST(FrameMetadataTest, HasMetadataFlag) {
  FrameMetadata metadata;
  EXPECT_FALSE(metadata.hasMetadata());

  metadata.flags_ |= ::apache::thrift::fast_thrift::frame::detail::kMetadataBit;
  EXPECT_TRUE(metadata.hasMetadata());
}

TEST(FrameMetadataTest, DataSizeCalculation) {
  FrameMetadata metadata;
  metadata.payloadSize = 100;
  metadata.metadataSize = 20;

  EXPECT_EQ(metadata.dataSize(), 80);
}

TEST(FrameMetadataTest, DataSizeWithZeroMetadata) {
  FrameMetadata metadata;
  metadata.payloadSize = 100;
  metadata.metadataSize = 0;

  EXPECT_EQ(metadata.dataSize(), 100);
}

TEST(FrameMetadataTest, DataSizeUnderflow) {
  // Edge case: metadataSize > payloadSize should return 0
  FrameMetadata metadata;
  metadata.payloadSize = 10;
  metadata.metadataSize = 20;

  EXPECT_EQ(metadata.dataSize(), 0);
}

TEST(FrameMetadataTest, IsRequestFrame) {
  FrameMetadata metadata;

  metadata.descriptor = &getDescriptor(FrameType::REQUEST_RESPONSE);
  EXPECT_TRUE(metadata.isRequestFrame());

  metadata.descriptor = &getDescriptor(FrameType::PAYLOAD);
  EXPECT_FALSE(metadata.isRequestFrame());
}

TEST(FrameMetadataTest, IsConnectionFrame) {
  FrameMetadata metadata;

  metadata.descriptor = &getDescriptor(FrameType::SETUP);
  EXPECT_TRUE(metadata.isConnectionFrame());

  metadata.descriptor = &getDescriptor(FrameType::KEEPALIVE);
  EXPECT_TRUE(metadata.isConnectionFrame());

  metadata.descriptor = &getDescriptor(FrameType::REQUEST_RESPONSE);
  EXPECT_FALSE(metadata.isConnectionFrame());
}

TEST(FrameMetadataTest, CanStoreInTypeErasedBox) {
  // Create a FrameMetadata and store it in TypeErasedBox
  FrameMetadata metadata;
  metadata.descriptor = &getDescriptor(FrameType::REQUEST_STREAM);
  metadata.streamId = 42;
  metadata.flags_ |= ::apache::thrift::fast_thrift::frame::detail::kMetadataBit;
  metadata.metadataSize = 10;
  metadata.payloadOffset = 20;
  metadata.payloadSize = 100;

  apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox box(metadata);

  EXPECT_FALSE(box.empty());

  auto& retrieved = box.get<FrameMetadata>();
  EXPECT_EQ(retrieved.streamId, 42);
  EXPECT_EQ(retrieved.type(), FrameType::REQUEST_STREAM);
  EXPECT_TRUE(retrieved.hasMetadata());
  EXPECT_EQ(retrieved.metadataSize, 10);
  EXPECT_EQ(retrieved.payloadOffset, 20);
  EXPECT_EQ(retrieved.payloadSize, 100);
}

} // namespace
} // namespace apache::thrift::fast_thrift::frame::read
