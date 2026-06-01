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
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/FrameParser.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameHeaders.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FrameWriter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientRocketInterfaceHandler.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift::client::handler {

using apache::thrift::fast_thrift::channel_pipeline::erase_and_box;
using apache::thrift::fast_thrift::channel_pipeline::Result;
using apache::thrift::fast_thrift::channel_pipeline::TypeErasedBox;

namespace {

/**
 * MockDuplexContext for testing ThriftClientRocketInterfaceHandler.
 *
 * Captures messages fired via fireWrite()/fireRead() and exceptions.
 */
class MockDuplexContext {
 public:
  Result fireWrite(TypeErasedBox&& msg) noexcept {
    if (returnError_) {
      return Result::Error;
    }
    writeMessages_.push_back(std::move(msg));
    return returnBackpressure_ ? Result::Backpressure : Result::Success;
  }

  Result fireRead(TypeErasedBox&& msg) noexcept {
    if (returnError_) {
      return Result::Error;
    }
    readMessages_.push_back(std::move(msg));
    return returnBackpressure_ ? Result::Backpressure : Result::Success;
  }

  void fireException(folly::exception_wrapper&& e) noexcept {
    exceptions_.push_back(std::move(e));
  }

  void setReturnBackpressure(bool value) { returnBackpressure_ = value; }

  void setReturnError(bool value) { returnError_ = value; }

  std::vector<TypeErasedBox>& writeMessages() { return writeMessages_; }

  std::vector<TypeErasedBox>& readMessages() { return readMessages_; }

  std::vector<folly::exception_wrapper>& exceptions() { return exceptions_; }

  void reset() {
    writeMessages_.clear();
    readMessages_.clear();
    exceptions_.clear();
    returnBackpressure_ = false;
    returnError_ = false;
  }

 private:
  std::vector<TypeErasedBox> writeMessages_;
  std::vector<TypeErasedBox> readMessages_;
  std::vector<folly::exception_wrapper> exceptions_;
  bool returnBackpressure_{false};
  bool returnError_{false};
};

} // namespace

class ThriftClientRocketInterfaceHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override { ctx_.reset(); }

  Result callOnWrite(TypeErasedBox msg) {
    return handler_.onWrite(ctx_, std::move(msg));
  }

  Result callOnRead(TypeErasedBox msg) {
    return handler_.onRead(ctx_, std::move(msg));
  }

  void callOnException(folly::exception_wrapper&& e) {
    handler_.onException(ctx_, std::move(e));
  }

  // Helper to create a ParsedFrame for payload frames
  apache::thrift::fast_thrift::frame::read::ParsedFrame createPayloadFrame(
      uint32_t streamId = 1) {
    auto frame = apache::thrift::fast_thrift::frame::write::serialize(
        apache::thrift::fast_thrift::frame::write::PayloadHeader{
            .streamId = streamId,
            .follows = false,
            .complete = true,
            .next = true,
        },
        nullptr,
        folly::IOBuf::copyBuffer("test payload"));
    return apache::thrift::fast_thrift::frame::read::parseFrame(
        std::move(frame));
  }

  MockDuplexContext ctx_;
  ThriftClientRocketInterfaceHandler handler_;
};

// =============================================================================
// Outbound Write Tests - ThriftRequestMessage → RocketRequestMessage
// =============================================================================

TEST_F(
    ThriftClientRocketInterfaceHandlerTest,
    ConvertThriftRequestToRocketRequest) {
  auto serializedMetadata = folly::IOBuf::copyBuffer("serialized metadata");
  auto data = folly::IOBuf::copyBuffer("request data");

  ThriftRequestMessage request{
      .payload =
          ThriftRequestPayload{
              .metadata = std::move(serializedMetadata),
              .data = std::move(data),
              .initialRequestN = 100,
              .rpcKind =
                  apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
              .complete = true,
          },
      .requestHandle = 42,
  };

  auto result = callOnWrite(erase_and_box(std::move(request)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);

  auto& written =
      ctx_.writeMessages()[0]
          .get<apache::thrift::fast_thrift::rocket::RocketRequestMessage>();
  EXPECT_EQ(written.requestHandle, 42);
  EXPECT_EQ(
      written.frameType,
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);

  // Verify the payload was extracted correctly
  ASSERT_TRUE(
      written.frame
          .is<apache::thrift::fast_thrift::rocket::RocketFramePayload>());
  auto& payload =
      written.frame
          .get<apache::thrift::fast_thrift::rocket::RocketFramePayload>();
  EXPECT_NE(payload.metadata, nullptr);
  EXPECT_NE(payload.data, nullptr);
  EXPECT_EQ(payload.initialRequestN, 100);
  EXPECT_TRUE(payload.complete);
}

TEST_F(ThriftClientRocketInterfaceHandlerTest, RequestHandlePassedThrough) {
  ThriftRequestMessage request{
      .payload =
          ThriftRequestPayload{
              .metadata = folly::IOBuf::copyBuffer("meta"),
              .data = folly::IOBuf::copyBuffer("data"),
              .rpcKind =
                  apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
          },
      .requestHandle = 12345,
  };

  auto result = callOnWrite(erase_and_box(std::move(request)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);

  auto& written =
      ctx_.writeMessages()[0]
          .get<apache::thrift::fast_thrift::rocket::RocketRequestMessage>();
  EXPECT_EQ(written.requestHandle, 12345);
}

TEST_F(ThriftClientRocketInterfaceHandlerTest, RpcKindConvertedToFrameType) {
  ThriftRequestMessage request{
      .payload =
          ThriftRequestPayload{
              .metadata = folly::IOBuf::copyBuffer("meta"),
              .data = folly::IOBuf::copyBuffer("data"),
              .rpcKind =
                  apache::thrift::RpcKind::SINGLE_REQUEST_STREAMING_RESPONSE,
          },
      .requestHandle = 1,
  };

  auto result = callOnWrite(erase_and_box(std::move(request)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.writeMessages().size(), 1);

  auto& written =
      ctx_.writeMessages()[0]
          .get<apache::thrift::fast_thrift::rocket::RocketRequestMessage>();
  EXPECT_EQ(
      written.frameType,
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_STREAM);
}

TEST_F(ThriftClientRocketInterfaceHandlerTest, WriteBackpressurePropagated) {
  ctx_.setReturnBackpressure(true);

  ThriftRequestMessage request{
      .payload =
          ThriftRequestPayload{
              .metadata = folly::IOBuf::copyBuffer("meta"),
              .data = folly::IOBuf::copyBuffer("data"),
              .rpcKind =
                  apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
          },
      .requestHandle = 1,
  };

  auto result = callOnWrite(erase_and_box(std::move(request)));

  EXPECT_EQ(result, Result::Backpressure);
}

TEST_F(ThriftClientRocketInterfaceHandlerTest, WriteErrorPropagated) {
  ctx_.setReturnError(true);

  ThriftRequestMessage request{
      .payload =
          ThriftRequestPayload{
              .metadata = folly::IOBuf::copyBuffer("meta"),
              .data = folly::IOBuf::copyBuffer("data"),
              .rpcKind =
                  apache::thrift::RpcKind::SINGLE_REQUEST_SINGLE_RESPONSE,
          },
      .requestHandle = 1,
  };

  auto result = callOnWrite(erase_and_box(std::move(request)));

  EXPECT_EQ(result, Result::Error);
}

// =============================================================================
// Inbound Read Tests - RocketResponseMessage → ThriftResponseMessage
// =============================================================================

TEST_F(
    ThriftClientRocketInterfaceHandlerTest,
    ConvertRocketResponseToThriftResponse) {
  auto frame = createPayloadFrame(5);

  apache::thrift::fast_thrift::rocket::RocketResponseMessage response{
      .frame = std::move(frame),
      .requestHandle = 99,
      .requestFrameType =
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE,
  };

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);

  auto& read = ctx_.readMessages()[0].get<ThriftResponseMessage>();
  EXPECT_EQ(read.requestHandle, 99);
  EXPECT_EQ(
      read.requestFrameType,
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE);

  // Verify frame is valid
  EXPECT_TRUE(read.frame.isValid());
}

TEST_F(ThriftClientRocketInterfaceHandlerTest, ResponseRequestHandlePreserved) {
  apache::thrift::fast_thrift::rocket::RocketResponseMessage response{
      .frame = createPayloadFrame(),
      .requestHandle = 777,
      .requestFrameType =
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE,
  };

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);

  auto& read = ctx_.readMessages()[0].get<ThriftResponseMessage>();
  EXPECT_EQ(read.requestHandle, 777);
}

TEST_F(
    ThriftClientRocketInterfaceHandlerTest, ResponseRequestFrameTypePreserved) {
  apache::thrift::fast_thrift::rocket::RocketResponseMessage response{
      .frame = createPayloadFrame(),
      .requestHandle = 1,
      .requestFrameType =
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_STREAM,
  };

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Success);
  ASSERT_EQ(ctx_.readMessages().size(), 1);

  auto& read = ctx_.readMessages()[0].get<ThriftResponseMessage>();
  EXPECT_EQ(
      read.requestFrameType,
      apache::thrift::fast_thrift::frame::FrameType::REQUEST_STREAM);
}

TEST_F(ThriftClientRocketInterfaceHandlerTest, ReadBackpressurePropagated) {
  ctx_.setReturnBackpressure(true);

  apache::thrift::fast_thrift::rocket::RocketResponseMessage response{
      .frame = createPayloadFrame(),
      .requestHandle = 1,
      .requestFrameType =
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE,
  };

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Backpressure);
}

TEST_F(ThriftClientRocketInterfaceHandlerTest, ReadErrorPropagated) {
  ctx_.setReturnError(true);

  apache::thrift::fast_thrift::rocket::RocketResponseMessage response{
      .frame = createPayloadFrame(),
      .requestHandle = 1,
      .requestFrameType =
          apache::thrift::fast_thrift::frame::FrameType::REQUEST_RESPONSE,
  };

  auto result = callOnRead(erase_and_box(std::move(response)));

  EXPECT_EQ(result, Result::Error);
}

// =============================================================================
// Exception Forwarding
// =============================================================================

TEST_F(ThriftClientRocketInterfaceHandlerTest, OnExceptionForwardsToContext) {
  auto error = folly::make_exception_wrapper<std::runtime_error>("test error");

  callOnException(std::move(error));

  ASSERT_EQ(ctx_.exceptions().size(), 1);
  EXPECT_TRUE(ctx_.exceptions()[0].is_compatible_with<std::runtime_error>());
}

} // namespace apache::thrift::fast_thrift::thrift::client::handler
