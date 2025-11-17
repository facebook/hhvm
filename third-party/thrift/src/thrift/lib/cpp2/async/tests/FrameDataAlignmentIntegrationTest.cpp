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
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/async/tests/gen-cpp2/FrameDataAlignmentIntegrationTestService.h>
#include <thrift/lib/cpp2/protocol/detail/PaddedBinaryAdapter.h>
#include <thrift/lib/cpp2/test/FlagTestUtils.h>
#include <thrift/lib/cpp2/transport/rocket/framing/Frames.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace apache::thrift;
using namespace apache::thrift::rocket;
using namespace apache::thrift::protocol;
using namespace apache::thrift::test;

class FrameDataAlignmentIntegrationTestServiceHandler
    : public virtual ServiceHandler<FrameDataAlignmentIntegrationTestService> {
 public:
  void sendRecvAligned(
      AlignedDataResponse& response,
      std::unique_ptr<AlignedDataRequest> request) override {
    response.data() = request->data()->buf->toString();
    response.padding() = request->data()->paddingBytes;
  }

  void sendRecvWrongEntry(
      AlignedDataResponse& response,
      std::unique_ptr<AlignedDataWrongEntryRequest> request) override {
    response.data() = *request->firstEntry() + request->data()->buf->toString();
    response.padding() = request->data()->paddingBytes;
  }

  void sendRecvWrongParam(
      AlignedDataResponse& response,
      std::unique_ptr<std::string> firstParam,
      std::unique_ptr<AlignedDataRequest> request) override {
    response.data() = *firstParam + request->data()->buf->toString();
    response.padding() = request->data()->paddingBytes;
  }

  void sendRecvBinary(
      std::string& response,
      std::unique_ptr<BinaryDataRequest> request) override {
    response = *request->data();
  }
};

TEST(FrameDataAlignmentIntegration, BasicAlignment) {
  THRIFT_FLAG_MOCK_GUARD(rocket_enable_frame_relative_alignment, true);

  constexpr uint32_t kAlignment = 4;
  auto handler =
      std::make_shared<FrameDataAlignmentIntegrationTestServiceHandler>();
  auto client = makeTestClient(
      handler,
      nullptr /* injectFault */,
      nullptr /* streamInjectFault */,
      protocol::T_BINARY_PROTOCOL);

  // Create test data
  std::string testData = "Test data for basic alignment";

  // Create a request with padded binary data
  AlignedDataRequest request;
  request.data() =
      PaddedBinaryData(kAlignment, folly::IOBuf::copyBuffer(testData));

  // Request alignment via rpc options
  RpcOptions rpcOptions;
  rpcOptions.setFrameRelativeDataAlignment(kAlignment);

  // Send the request and receive the response
  AlignedDataResponse response;
  client->sync_sendRecvAligned(rpcOptions, response, request);

  // Compute the expected padding.
  uint32_t dataOffsetInFrame =
      /* frame length */ rocket::Serializer::kBytesForFrameOrMetadataLength +
      RequestResponseFrame::frameHeaderSize() +
      /* metadata size */ rocket::Serializer::kBytesForFrameOrMetadataLength +
      /* metadata payload */ 26 + /* offset in data payload */ 6 +
      /* data size */ 4 + PaddedBinaryData::kPaddingHeaderBytes;

  uint32_t expectedPadding =
      (kAlignment - (dataOffsetInFrame & (kAlignment - 1))) & (kAlignment - 1);

  // Verify that the response matches the sent data
  EXPECT_EQ(response.data(), testData);
  EXPECT_EQ(*response.padding(), expectedPadding);
}

TEST(FrameDataAlignmentIntegration, LargeAlignment) {
  THRIFT_FLAG_MOCK_GUARD(rocket_enable_frame_relative_alignment, true);

  constexpr uint32_t kAlignment = 4096;
  auto handler =
      std::make_shared<FrameDataAlignmentIntegrationTestServiceHandler>();
  auto client = makeTestClient(
      handler,
      nullptr /* injectFault */,
      nullptr /* streamInjectFault */,
      protocol::T_BINARY_PROTOCOL);

  std::string testData = "Test data for large alignment";

  AlignedDataRequest request;
  request.data() = PaddedBinaryData(4096, folly::IOBuf::copyBuffer(testData));

  // Request alignment via rpc options
  RpcOptions rpcOptions;
  rpcOptions.setFrameRelativeDataAlignment(kAlignment);

  // Send the request and receive the response
  AlignedDataResponse response;
  client->sync_sendRecvAligned(rpcOptions, response, request);

  // Compute the expected padding.
  uint32_t dataOffsetInFrame =
      /* frame length */ rocket::Serializer::kBytesForFrameOrMetadataLength +
      RequestResponseFrame::frameHeaderSize() +
      /* metadata size */ rocket::Serializer::kBytesForFrameOrMetadataLength +
      /* metadata payload */ 26 + /* offset in data payload */ 6 +
      /* data size */ 4 + PaddedBinaryData::kPaddingHeaderBytes;

  uint32_t expectedPadding =
      (kAlignment - (dataOffsetInFrame & (kAlignment - 1))) & (kAlignment - 1);

  // Verify that the response matches the sent data
  EXPECT_EQ(response.data(), testData);
  EXPECT_EQ(*response.padding(), expectedPadding);
}

TEST(FrameDataAlignmentIntegration, CompactProtocol) {
  constexpr uint32_t kAlignment = 4;
  // Create test data
  std::string testData = "Test data for basic alignment";

  // Create a request with padded binary data
  AlignedDataRequest request;
  request.data() =
      PaddedBinaryData(kAlignment, folly::IOBuf::copyBuffer(testData));

  // Request alignment via rpc options
  RpcOptions rpcOptions;
  rpcOptions.setFrameRelativeDataAlignment(kAlignment);

#ifndef NDEBUG
  // Call echo on the client
  EXPECT_DEBUG_DEATH(
      {
        THRIFT_FLAG_MOCK_GUARD(rocket_enable_frame_relative_alignment, true);
        auto handler =
            std::make_shared<FrameDataAlignmentIntegrationTestServiceHandler>();
        auto client = makeTestClient(
            handler,
            nullptr /* injectFault */,
            nullptr /* streamInjectFault */,
            T_COMPACT_PROTOCOL);

        // Send the request and receive the response
        AlignedDataResponse response;
        client->sync_sendRecvAligned(rpcOptions, response, request);
      },
      "Only binary protocol is supported");
#else
  THRIFT_FLAG_MOCK_GUARD(rocket_enable_frame_relative_alignment, true);

  auto handler =
      std::make_shared<FrameDataAlignmentIntegrationTestServiceHandler>();
  auto client = makeTestClient(
      handler,
      nullptr /* injectFault */,
      nullptr /* streamInjectFault */,
      T_COMPACT_PROTOCOL);

  // Send the request and receive the response
  AlignedDataResponse response;
  client->sync_sendRecvAligned(rpcOptions, response, request);

  // Verify that the response matches the sent data
  EXPECT_EQ(response.data(), testData);
  EXPECT_EQ(*response.padding(), 0);
#endif
}

TEST(FrameDataAlignmentIntegration, WrongParam) {
  constexpr uint32_t kAlignment = 4;

  // Create test data
  std::string testData = "Test data for basic alignment";

  // Create a request with padded binary data
  AlignedDataRequest request;
  request.data() =
      PaddedBinaryData(kAlignment, folly::IOBuf::copyBuffer(testData));

  std::string firstParam = "first param";

  // Request alignment via rpc options
  RpcOptions rpcOptions;
  rpcOptions.setFrameRelativeDataAlignment(kAlignment);

#ifndef NDEBUG
  ASSERT_DEATH(
      {
        THRIFT_FLAG_MOCK_GUARD(rocket_enable_frame_relative_alignment, true);

        auto handler =
            std::make_shared<FrameDataAlignmentIntegrationTestServiceHandler>();
        auto client = makeTestClient(
            handler,
            nullptr /* injectFault */,
            nullptr /* streamInjectFault */,
            protocol::T_BINARY_PROTOCOL);

        // Send the request and receive the response
        AlignedDataResponse response;
        client->sync_sendRecvWrongParam(
            rpcOptions, response, firstParam, request);
      },
      "Cannot find entry in the first param with fieldId=1");
#else
  THRIFT_FLAG_MOCK_GUARD(rocket_enable_frame_relative_alignment, true);

  auto handler =
      std::make_shared<FrameDataAlignmentIntegrationTestServiceHandler>();
  auto client = makeTestClient(
      handler,
      nullptr /* injectFault */,
      nullptr /* streamInjectFault */,
      protocol::T_BINARY_PROTOCOL);

  // Send the request and receive the response
  AlignedDataResponse response;
  client->sync_sendRecvWrongParam(rpcOptions, response, firstParam, request);

  // Verify that the response matches the sent data
  EXPECT_EQ(response.data(), firstParam + testData);
  EXPECT_EQ(*response.padding(), kAlignment);
#endif
}

TEST(FrameDataAlignmentIntegration, WrongEntry) {
  constexpr uint32_t kAlignment = 4;

  // Create test data
  std::string testData = "Test data for wrong entry";
  std::string firstEntry = "first entry";

  // Create a request with padded binary data
  AlignedDataWrongEntryRequest request;
  request.firstEntry() = firstEntry;
  request.data() =
      PaddedBinaryData(kAlignment, folly::IOBuf::copyBuffer(testData));

  // Request alignment via rpc options
  RpcOptions rpcOptions;
  rpcOptions.setFrameRelativeDataAlignment(kAlignment);

#ifndef NDEBUG
  ASSERT_DEATH(
      {
        THRIFT_FLAG_MOCK_GUARD(rocket_enable_frame_relative_alignment, true);

        auto handler =
            std::make_shared<FrameDataAlignmentIntegrationTestServiceHandler>();
        auto client = makeTestClient(
            handler,
            nullptr /* injectFault */,
            nullptr /* streamInjectFault */,
            protocol::T_BINARY_PROTOCOL);

        // Send the request and receive the response
        AlignedDataResponse response;
        client->sync_sendRecvWrongEntry(rpcOptions, response, request);
      },
      "Not padded");
#else
  THRIFT_FLAG_MOCK_GUARD(rocket_enable_frame_relative_alignment, true);

  // Verify that the response matches the sent data
  auto handler =
      std::make_shared<FrameDataAlignmentIntegrationTestServiceHandler>();
  auto client = makeTestClient(
      handler,
      nullptr /* injectFault */,
      nullptr /* streamInjectFault */,
      protocol::T_BINARY_PROTOCOL);

  // Send the request and receive the response
  AlignedDataResponse response;
  client->sync_sendRecvWrongEntry(rpcOptions, response, request);

  // Verify that the response matches the sent data
  EXPECT_EQ(response.data(), firstEntry + testData);
  EXPECT_EQ(*response.padding(), kAlignment);
#endif
}

TEST(FrameDataAlignmentIntegration, RawBinary) {
  constexpr uint32_t kAlignment = 4;

  // Create test data
  std::string testData = "Test data for basic alignment";

  // Create a request with padded binary data
  BinaryDataRequest request;
  request.data() = testData;

  // Request alignment via rpc options
  RpcOptions rpcOptions;
  rpcOptions.setFrameRelativeDataAlignment(kAlignment);

#ifndef NDEBUG
  ASSERT_DEATH(
      {
        THRIFT_FLAG_MOCK_GUARD(rocket_enable_frame_relative_alignment, true);

        auto handler =
            std::make_shared<FrameDataAlignmentIntegrationTestServiceHandler>();
        auto client = makeTestClient(
            handler,
            nullptr /* injectFault */,
            nullptr /* streamInjectFault */,
            protocol::T_BINARY_PROTOCOL);

        // Send the request and receive the response
        std::string response;
        client->sync_sendRecvBinary(rpcOptions, response, request);
      },
      "Magic mismatch");
#else
  THRIFT_FLAG_MOCK_GUARD(rocket_enable_frame_relative_alignment, true);

  auto handler =
      std::make_shared<FrameDataAlignmentIntegrationTestServiceHandler>();
  auto client = makeTestClient(
      handler,
      nullptr /* injectFault */,
      nullptr /* streamInjectFault */,
      protocol::T_BINARY_PROTOCOL);

  // Send the request and receive the response
  std::string response;
  client->sync_sendRecvBinary(rpcOptions, response, request);

  // Verify that the response matches the sent data
  EXPECT_EQ(response, testData);
#endif
}

TEST(FrameDataAlignmentIntegration, LargePayload) {
  THRIFT_FLAG_MOCK_GUARD(rocket_enable_frame_relative_alignment, true);

  constexpr uint32_t kAlignment = 4;
  auto handler =
      std::make_shared<FrameDataAlignmentIntegrationTestServiceHandler>();
  auto client = makeTestClient(
      handler,
      nullptr /* injectFault */,
      nullptr /* streamInjectFault */,
      protocol::T_BINARY_PROTOCOL);

  // Create test data
  std::string testData(4096, 'a');

  // Create a request with padded binary data
  AlignedDataRequest request;
  request.data() =
      PaddedBinaryData(kAlignment, folly::IOBuf::copyBuffer(testData));

  // Request alignment via rpc options
  RpcOptions rpcOptions;
  rpcOptions.setFrameRelativeDataAlignment(kAlignment);

  // Send the request and receive the response
  AlignedDataResponse response;
  client->sync_sendRecvAligned(rpcOptions, response, request);

  // Compute the expected padding.
  uint32_t dataOffsetInFrame =
      /* frame length */ rocket::Serializer::kBytesForFrameOrMetadataLength +
      RequestResponseFrame::frameHeaderSize() +
      /* metadata size */ rocket::Serializer::kBytesForFrameOrMetadataLength +
      /* metadata payload */ 26 + /* offset in data payload */ 6 +
      /* data size */ 4 + PaddedBinaryData::kPaddingHeaderBytes;

  uint32_t expectedPadding =
      (kAlignment - (dataOffsetInFrame & (kAlignment - 1))) & (kAlignment - 1);

  // Verify that the response matches the sent data
  EXPECT_EQ(response.data(), testData);
  EXPECT_EQ(*response.padding(), expectedPadding);
}

TEST(FrameDataAlignmentIntegration, LargePayloadWithLargeAlignment) {
  THRIFT_FLAG_MOCK_GUARD(rocket_enable_frame_relative_alignment, true);

  constexpr uint32_t kAlignment = 4096;
  auto handler =
      std::make_shared<FrameDataAlignmentIntegrationTestServiceHandler>();
  auto client = makeTestClient(
      handler,
      nullptr /* injectFault */,
      nullptr /* streamInjectFault */,
      protocol::T_BINARY_PROTOCOL);

  // Create test data
  std::string testData(4096, 'a');

  // Create a request with padded binary data
  AlignedDataRequest request;
  request.data() =
      PaddedBinaryData(kAlignment, folly::IOBuf::copyBuffer(testData));

  // Request alignment via rpc options
  RpcOptions rpcOptions;
  rpcOptions.setFrameRelativeDataAlignment(kAlignment);

  // Send the request and receive the response
  AlignedDataResponse response;
  client->sync_sendRecvAligned(rpcOptions, response, request);

  // Compute the expected padding.
  uint32_t dataOffsetInFrame =
      /* frame length */ rocket::Serializer::kBytesForFrameOrMetadataLength +
      RequestResponseFrame::frameHeaderSize() +
      /* metadata size */ rocket::Serializer::kBytesForFrameOrMetadataLength +
      /* metadata payload */ 26 + /* offset in data payload */ 6 +
      /* data size */ 4 + PaddedBinaryData::kPaddingHeaderBytes;

  uint32_t expectedPadding =
      (kAlignment - (dataOffsetInFrame & (kAlignment - 1))) & (kAlignment - 1);

  // Verify that the response matches the sent data
  EXPECT_EQ(response.data(), testData);
  EXPECT_EQ(*response.padding(), expectedPadding);
}

TEST(FrameDataAlignmentIntegration, FragmentedPayload) {
  constexpr uint32_t kAlignment = 256;

  // Create test data
  std::string testData(
      2 * apache::thrift::rocket::kMaxFragmentedPayloadSize + 1, 'a');

  // Create a request with padded binary data
  AlignedDataRequest request;
  request.data() =
      PaddedBinaryData(kAlignment, folly::IOBuf::copyBuffer(testData));

  // Request alignment via rpc options
  RpcOptions rpcOptions;
  rpcOptions.setFrameRelativeDataAlignment(kAlignment);

#ifndef NDEBUG
  ASSERT_DEATH(
      {
        THRIFT_FLAG_MOCK_GUARD(rocket_enable_frame_relative_alignment, true);

        auto handler =
            std::make_shared<FrameDataAlignmentIntegrationTestServiceHandler>();
        auto client = makeTestClient(
            handler,
            nullptr /* injectFault */,
            nullptr /* streamInjectFault */,
            protocol::T_BINARY_PROTOCOL);

        // Send the request and receive the response
        AlignedDataResponse response;
        client->sync_sendRecvAligned(rpcOptions, response, request);
      },
      "Fragmented payload");
#else
  THRIFT_FLAG_MOCK_GUARD(rocket_enable_frame_relative_alignment, true);

  auto handler =
      std::make_shared<FrameDataAlignmentIntegrationTestServiceHandler>();
  auto client = makeTestClient(
      handler,
      nullptr /* injectFault */,
      nullptr /* streamInjectFault */,
      protocol::T_BINARY_PROTOCOL);

  // Send the request and receive the response
  AlignedDataResponse response;
  client->sync_sendRecvAligned(rpcOptions, response, request);

  // Verify that the response matches the sent data
  EXPECT_EQ(response.data(), testData);
  EXPECT_EQ(*response.padding(), kAlignment);
#endif
}
