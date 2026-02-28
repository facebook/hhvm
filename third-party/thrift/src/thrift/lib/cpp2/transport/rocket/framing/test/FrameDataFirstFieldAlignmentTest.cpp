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

#include <gtest/gtest.h>
#include <folly/io/Cursor.h>
#include <folly/io/IOBuf.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/protocol/detail/PaddedBinaryAdapter.h>
#include <thrift/lib/cpp2/test/FlagTestUtils.h>
#include <thrift/lib/cpp2/transport/rocket/Types.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/transport/rocket/framing/test/gen-cpp2/FrameDataFirstFieldAlignment_types.h>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::rocket;
using namespace apache::thrift::test;

namespace {

template <typename T>
T getRequest(const std::string& data, uint32_t padding) {
  if constexpr (std::is_same_v<T, AlignedDataRequest>) {
    AlignedDataRequest req;
    req.data() = PaddedBinaryData(padding, folly::IOBuf::copyBuffer(data));
    return req;
  } else {
    BinaryDataRequest req;
    req.data() = data;
    return req;
  }
}

template <typename T, typename Protocol>
Payload getSerializedPayload(
    const std::string& data,
    uint32_t padding,
    std::optional<std::string> metadata = std::nullopt) {
  T request = getRequest<T>(data, padding);
  folly::IOBufQueue serializedData;
  Protocol prot;
  prot.setOutput(&serializedData);
  // Start of request struct
  prot.writeStructBegin("");
  // Write the struct type
  prot.writeFieldBegin("", TType::T_STRUCT, 1);
  // Write the request struct params
  request.write(&prot);
  // Field stop for request struct params
  prot.writeFieldStop();
  // End of request struct
  prot.writeStructEnd();

  if (metadata) {
    return Payload::makeFromMetadataAndData(
        folly::IOBuf::copyBuffer(*metadata), serializedData.move());
  } else {
    return Payload::makeFromData(serializedData.move());
  }
}

void verifyDataCommon(
    RequestResponseFrame&& frame,
    uint32_t alignment,
    std::string& testData,
    uintptr_t& frameStartAddress,
    uintptr_t& firstFieldDataStartAddress) {
  auto serializedFrame = [&]() {
    THRIFT_FLAG_MOCK_GUARD(rocket_enable_frame_relative_alignment, true);
    return std::move(frame).serialize();
  }();

  serializedFrame->coalesce();

  frameStartAddress = reinterpret_cast<uintptr_t>(serializedFrame->data());
  serializedFrame->trimStart(3);

  // Deserialize and verify data alignment
  RequestResponseFrame deserializedFrame(std::move(serializedFrame));
  auto payloadData = std::move(deserializedFrame.payload()).data();

  // Deserialize the payload and validate it
  folly::io::Cursor cr(payloadData.get());
  cr.skip(3); // Skip struct metadata
  cr.skip(3); // Skip field metadata
  uint32_t dataSize = cr.readBE<uint32_t>();
  uint64_t magic = cr.readBE<uint64_t>();
  ASSERT_EQ(magic, PaddedBinaryData::kMagic);
  uint32_t paddingBytes = cr.readBE<uint32_t>();
  ASSERT_LT(paddingBytes, alignment);

  uint32_t paddedDataExtraBytes =
      sizeof(magic) + sizeof(paddingBytes) + paddingBytes;
  ASSERT_EQ(dataSize, testData.size() + paddedDataExtraBytes);
  cr.skip(paddingBytes);

  std::string outData(
      reinterpret_cast<const char*>(cr.data()), testData.size());
  EXPECT_EQ(outData, testData);

  firstFieldDataStartAddress = reinterpret_cast<uintptr_t>(cr.data());
}

// Helper function to verify alignment of data field
void verifyDataAligned(
    RequestResponseFrame&& frame, uint32_t alignment, std::string& testData) {
  uintptr_t frameStartAddress = 0x0;
  uintptr_t firstFieldDataStartAddress = 0x0;
  verifyDataCommon(
      std::move(frame),
      alignment,
      testData,
      frameStartAddress,
      firstFieldDataStartAddress);

  uintptr_t offset = firstFieldDataStartAddress - frameStartAddress;
  EXPECT_EQ(offset % alignment, 0)
      << "Data offset=" << offset
      << "b is not aligned to alignment=" << alignment
      << "b. (frameStartAddress=0x" << std::hex << frameStartAddress
      << ", data=0x" << firstFieldDataStartAddress << ")";
}

#ifdef NDEBUG
void verifyDataNotAligned(
    RequestResponseFrame&& frame, uint32_t alignment, std::string& testData) {
  uintptr_t frameStartAddress = 0x0;
  uintptr_t firstFieldDataStartAddress = 0x0;
  verifyDataCommon(
      std::move(frame),
      alignment,
      testData,
      frameStartAddress,
      firstFieldDataStartAddress);

  uintptr_t offset = firstFieldDataStartAddress - frameStartAddress;
  EXPECT_NE(offset % alignment, 0)
      << "Data offset=" << offset << "b is aligned to alignment=" << alignment
      << "b. (frameStartAddress=0x" << std::hex << frameStartAddress
      << ", data=0x" << firstFieldDataStartAddress << ")";
}
#endif

TEST(FrameDataAlignmentTest, BasicAlignment) {
  constexpr uint32_t kAdapterPadding = 4;
  constexpr uint32_t kFrameAlignment = kAdapterPadding;
  std::string testData = "Test data for basic alignment";

  Payload payload =
      getSerializedPayload<AlignedDataRequest, BinaryProtocolWriter>(
          testData, kAdapterPadding);
  payload.setDataFirstFieldAlignment(kFrameAlignment);
  payload.setDataSerializationProtocol(ProtocolType::T_BINARY_PROTOCOL);

  RequestResponseFrame frame(StreamId(1), std::move(payload));

  verifyDataAligned(std::move(frame), kFrameAlignment, testData);
}

TEST(FrameDataAlignmentTest, LargeAlignment) {
  constexpr uint32_t kAdapterPadding = 4096;
  constexpr uint32_t kFrameAlignment = kAdapterPadding;
  std::string testData = "Test data for large alignment";

  Payload payload =
      getSerializedPayload<AlignedDataRequest, BinaryProtocolWriter>(
          testData, kAdapterPadding);
  payload.setDataFirstFieldAlignment(kFrameAlignment);
  payload.setDataSerializationProtocol(ProtocolType::T_BINARY_PROTOCOL);

  RequestResponseFrame frame(StreamId(1), std::move(payload));

  verifyDataAligned(std::move(frame), kFrameAlignment, testData);
}

TEST(FrameDataAlignmentTest, ArbitraryAlignment) {
  constexpr uint32_t kAdapterPadding = 121;
  constexpr uint32_t kFrameAlignment = kAdapterPadding;
  std::string testData = "Test data for arbitrary alignment";

  Payload payload =
      getSerializedPayload<AlignedDataRequest, BinaryProtocolWriter>(
          testData, kAdapterPadding);
  payload.setDataFirstFieldAlignment(kFrameAlignment);
  payload.setDataSerializationProtocol(ProtocolType::T_BINARY_PROTOCOL);

  RequestResponseFrame frame(StreamId(1), std::move(payload));
#ifndef NDEBUG
  ASSERT_DEATH(
      verifyDataAligned(std::move(frame), kFrameAlignment, testData),
      "Alignment is not a power of two");
#else
  verifyDataNotAligned(std::move(frame), kFrameAlignment + 1, testData);
#endif
}

TEST(FrameDataAlignmentTest, PaddedDataWithMetadata) {
  constexpr uint32_t kAdapterPadding = 64;
  constexpr uint32_t kFrameAlignment = kAdapterPadding;
  std::string testData = "Test data for arbitrary alignment";

  Payload payload =
      getSerializedPayload<AlignedDataRequest, BinaryProtocolWriter>(
          testData, kAdapterPadding, "metadata");
  payload.setDataFirstFieldAlignment(kFrameAlignment);
  payload.setDataSerializationProtocol(ProtocolType::T_BINARY_PROTOCOL);

  RequestResponseFrame frame(StreamId(1), std::move(payload));

  verifyDataAligned(std::move(frame), kFrameAlignment, testData);
}

TEST(FrameDataAlignmentTest, EmptyDataAlignment) {
  constexpr uint32_t kAdapterPadding = 64;
  constexpr uint32_t kFrameAlignment = kAdapterPadding;
  std::string testData;

  Payload payload =
      getSerializedPayload<AlignedDataRequest, BinaryProtocolWriter>(
          testData, kAdapterPadding);
  payload.setDataFirstFieldAlignment(kFrameAlignment);
  payload.setDataSerializationProtocol(ProtocolType::T_BINARY_PROTOCOL);

  RequestResponseFrame frame(StreamId(1), std::move(payload));

#ifndef NDEBUG
  ASSERT_DEATH(
      verifyDataAligned(std::move(frame), kFrameAlignment, testData),
      "Not padded");
#else
  auto serializedFrame = [&]() {
    THRIFT_FLAG_MOCK_GUARD(rocket_enable_frame_relative_alignment, true);
    return std::move(frame).serialize();
  }();

  serializedFrame->coalesce();

  // Trim frame length (3 bytes)
  serializedFrame->trimStart(3);

  // Deserialize the frame
  RequestResponseFrame deserializedFrame(std::move(serializedFrame));
  auto payloadData = std::move(deserializedFrame.payload()).data();

  // For an empty payload we just store 12 bytes:
  // 1-2: field begin for struct
  // 3-5: field begin for data
  // 6-9: field data size
  // 10: field stop for data
  // 11: field stop for struct
  ASSERT_EQ(payloadData->computeChainDataLength(), 12);
#endif
}

TEST(FrameDataAlignmentTest, InsufficientPadding) {
  constexpr size_t kAdapterPadding = 64;
  constexpr size_t kFrameAlignment = 128;
  std::string testData = "Test data for insufficient padding";

  Payload payload =
      getSerializedPayload<AlignedDataRequest, BinaryProtocolWriter>(
          testData, kAdapterPadding);
  payload.setDataFirstFieldAlignment(kFrameAlignment);
  payload.setDataSerializationProtocol(ProtocolType::T_BINARY_PROTOCOL);

  RequestResponseFrame frame(StreamId(1), std::move(payload));

#ifndef NDEBUG
  ASSERT_DEATH(
      verifyDataAligned(std::move(frame), kFrameAlignment, testData),
      "Insufficient padding");
#else
  verifyDataNotAligned(std::move(frame), kFrameAlignment, testData);
#endif
}

TEST(FrameDataAlignmentTest, RawBinaryData) {
  constexpr uint32_t kFrameAlignment = 64;
  std::string testData = "Test data for raw binary data";

  Payload payload =
      getSerializedPayload<BinaryDataRequest, BinaryProtocolWriter>(
          testData, kFrameAlignment);
  payload.setDataFirstFieldAlignment(kFrameAlignment);
  payload.setDataSerializationProtocol(ProtocolType::T_BINARY_PROTOCOL);

  RequestResponseFrame frame(StreamId(1), std::move(payload));

#ifndef NDEBUG
  ASSERT_DEATH(
      verifyDataAligned(std::move(frame), kFrameAlignment, testData),
      "Magic mismatch");
#else
  auto serializedFrame = [&]() {
    THRIFT_FLAG_MOCK_GUARD(rocket_enable_frame_relative_alignment, true);
    return std::move(frame).serialize();
  }();
  serializedFrame->coalesce();

  // Trim frame length (3 bytes)
  serializedFrame->trimStart(3);

  // Deserialize the frame
  RequestResponseFrame deserializedFrame(std::move(serializedFrame));
  auto payloadData = std::move(deserializedFrame.payload()).data();

  // Deserialize the payload and validate it
  folly::io::Cursor cr(payloadData.get());
  cr.skip(3); // Skip field metadata
  cr.skip(3); // Skip struct metadata
  uint32_t dataSize = cr.readBE<uint32_t>();
  ASSERT_EQ(dataSize, testData.size());

  // Since the data was not padded, the next bytes should just be data itself
  std::string outData(
      reinterpret_cast<const char*>(cr.data()), testData.size());
  EXPECT_EQ(outData, testData);
#endif
}

TEST(FrameDataAlignmentTest, CoalescedIOBufs) {
  constexpr uint32_t kAdapterPadding = 64;
  constexpr uint32_t kFrameAlignment = kAdapterPadding;
  std::string testData = "Test data for coalesced iobufs";

  AlignedDataRequest request =
      getRequest<AlignedDataRequest>(testData, kAdapterPadding);

  Payload payload =
      getSerializedPayload<AlignedDataRequest, BinaryProtocolWriter>(
          testData, kAdapterPadding);

  // Coalesce the IOBufs and make a new payload from it
  auto coalescedBuf = payload.buffer()->cloneCoalesced();
  Payload coalescedPayload = Payload::makeFromData(std::move(coalescedBuf));
  coalescedPayload.setDataFirstFieldAlignment(kFrameAlignment);
  coalescedPayload.setDataSerializationProtocol(
      ProtocolType::T_BINARY_PROTOCOL);

  RequestResponseFrame frame(StreamId(1), std::move(coalescedPayload));

#ifndef NDEBUG
  ASSERT_DEATH(
      verifyDataAligned(std::move(frame), kFrameAlignment, testData),
      "Padding IOBuf not separate");
#else
  verifyDataNotAligned(std::move(frame), kFrameAlignment + 1, testData);
#endif
}

TEST(FrameDataAlignmentTest, ZeroFrameRelativeAlignment) {
  constexpr uint32_t kAdapterPadding = 64;
  constexpr uint32_t kFrameAlignment = 0;
  std::string testData = "Test data for zero frame relative alignment";

  Payload payload =
      getSerializedPayload<AlignedDataRequest, BinaryProtocolWriter>(
          testData, kAdapterPadding);
  payload.setDataFirstFieldAlignment(kFrameAlignment);
  payload.setDataSerializationProtocol(ProtocolType::T_BINARY_PROTOCOL);

  RequestResponseFrame frame(StreamId(1), std::move(payload));

#ifndef NDEBUG
  ASSERT_DEATH(
      {
        THRIFT_FLAG_MOCK_GUARD(rocket_enable_frame_relative_alignment, true);
        auto serializedFrame = std::move(frame).serialize();
      },
      "Alignment is not a power of two");
#else
  auto serializedFrame = [&]() {
    THRIFT_FLAG_MOCK_GUARD(rocket_enable_frame_relative_alignment, true);
    return std::move(frame).serialize();
  }();

  serializedFrame->coalesce();

  // Trim frame length (3 bytes)
  serializedFrame->trimStart(3);

  // Deserialize and verify data alignment
  RequestResponseFrame deserializedFrame(std::move(serializedFrame));
  auto payloadData = std::move(deserializedFrame.payload()).data();

  // Deserialize the payload and validate it
  folly::io::Cursor cr(payloadData.get());
  cr.skip(3); // Skip field metadata
  cr.skip(3); // Skip struct metadata
  uint32_t dataSize = cr.readBE<uint32_t>();
  uint64_t magic = cr.readBE<uint64_t>();
  ASSERT_EQ(magic, PaddedBinaryData::kMagic);
  uint32_t paddingBytes = cr.readBE<uint32_t>();
  // Padding should not change
  ASSERT_EQ(paddingBytes, kAdapterPadding);

  uint32_t paddedDataExtraBytes =
      sizeof(magic) + sizeof(paddingBytes) + paddingBytes;
  ASSERT_EQ(dataSize, testData.size() + paddedDataExtraBytes);
  cr.skip(paddingBytes);

  std::string outData(
      reinterpret_cast<const char*>(cr.data()), testData.size());
  EXPECT_EQ(outData, testData);
#endif
}

TEST(FrameDataAlignmentTest, CompactProtocol) {
  constexpr uint32_t kAdapterPadding = 64;
  constexpr uint32_t kFrameAlignment = kAdapterPadding;
  std::string testData = "Test data for compact protocol";

  Payload payload =
      getSerializedPayload<BinaryDataRequest, CompactProtocolWriter>(
          testData, kAdapterPadding);
  payload.setDataFirstFieldAlignment(kFrameAlignment);
  payload.setDataSerializationProtocol(ProtocolType::T_COMPACT_PROTOCOL);

  RequestResponseFrame frame(StreamId(1), std::move(payload));

#ifndef NDEBUG
  ASSERT_DEATH(
      verifyDataAligned(std::move(frame), kFrameAlignment, testData),
      "Only binary protocol is supported");
#else
  auto serializedFrame = [&]() {
    THRIFT_FLAG_MOCK_GUARD(rocket_enable_frame_relative_alignment, true);
    return std::move(frame).serialize();
  }();

  serializedFrame->coalesce();

  // Trim frame length (3 bytes)
  serializedFrame->trimStart(3);

  // Deserialize and verify data alignment
  RequestResponseFrame deserializedFrame(std::move(serializedFrame));
  auto payloadData = std::move(deserializedFrame.payload()).data();

  // Deserialize the payload and validate it
  CompactProtocolReader reader;
  reader.setInput(payloadData.get());
  // Skip struct metadata
  TType fieldType;
  int16_t fieldId;
  std::string fieldName;
  reader.readFieldBegin(fieldName, fieldType, fieldId);
  AlignedDataRequest deserializedRequest;
  deserializedRequest.read(&reader);
  EXPECT_EQ(deserializedRequest.data()->buf->to<std::string>(), testData);
#endif
}

TEST(FrameDataAlignmentTest, EmptyProtocol) {
  constexpr uint32_t kAdapterPadding = 64;
  constexpr uint32_t kFrameAlignment = kAdapterPadding;
  std::string testData = "Test data for empty protocol";

  Payload payload =
      getSerializedPayload<AlignedDataRequest, BinaryProtocolWriter>(
          testData, kAdapterPadding);
  payload.setDataFirstFieldAlignment(kFrameAlignment);
  // Do not set the serialization protocol in payload
  RequestResponseFrame frame(StreamId(1), std::move(payload));

#ifndef NDEBUG
  ASSERT_DEATH(
      verifyDataAligned(std::move(frame), kFrameAlignment, testData),
      "No data serialization protocol specified");
#else
  auto serializedFrame = [&]() {
    THRIFT_FLAG_MOCK_GUARD(rocket_enable_frame_relative_alignment, true);
    return std::move(frame).serialize();
  }();

  serializedFrame->coalesce();

  // Trim frame length (3 bytes)
  serializedFrame->trimStart(3);

  // Deserialize and verify data alignment
  RequestResponseFrame deserializedFrame(std::move(serializedFrame));
  auto payloadData = std::move(deserializedFrame.payload()).data();

  // Deserialize the payload and validate it
  BinaryProtocolReader reader;
  reader.setInput(payloadData.get());
  reader.skipBytes(3); // Skip struct metadata
  AlignedDataRequest deserializedRequest;
  deserializedRequest.read(&reader);
  EXPECT_EQ(deserializedRequest.data()->buf->to<std::string>(), testData);
#endif
}

} // namespace
