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

// Unit tests for ThriftClientChannel construction/identity.
//
// ThriftClientChannel is rocket-direct: it is handed a fully-connected
// RocketClientConnection and drives it via the app adapter. End-to-end
// request/response, error, and multiplexing behavior is covered by
// ThriftClientIntegrationTest (over a TestAsyncTransport connection) and
// ThriftClientBackwardsCompatibilityE2ETest (against a real server); this
// file only covers the channel's own construction surface.

#include <gtest/gtest.h>

#include <folly/io/IOBuf.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/common/RocketClientConnection.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientConnectionErrorHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientRequestResponseHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientChannel.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/test/TestAsyncTransport.h>

namespace apache::thrift::fast_thrift::thrift {

using apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder;
using apache::thrift::fast_thrift::transport::test::TestAsyncTransport;

// Handler tags for the rocket pipeline.
HANDLER_TAG(frame_length_parser_handler);
HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(rocket_client_frame_codec_handler);
HANDLER_TAG(rocket_client_setup_handler);
HANDLER_TAG(rocket_client_connection_error_handler);
HANDLER_TAG(rocket_client_stream_state_handler);
HANDLER_TAG(rocket_client_request_response_handler);

class ThriftClientChannelTest : public ::testing::Test {
 protected:
  // Build a fully-connected rocket connection over a TestAsyncTransport and
  // hand it to a new channel, mirroring how callers wire the channel.
  ThriftClientChannel::UniquePtr makeConnectedChannel(
      uint16_t protocolId = apache::thrift::protocol::T_COMPACT_PROTOCOL) {
    auto transport =
        folly::AsyncTransport::UniquePtr(new TestAsyncTransport(&evb_));
    testTransport_ = static_cast<TestAsyncTransport*>(transport.get());

    auto connection =
        std::make_unique<rocket::client::RocketClientConnection>();
    connection->transportHandler =
        rocket::client::RocketClientConnection::TransportHandler::create(
            std::move(transport));
    auto* transportHandlerPtr = connection->transportHandler.get();

    connection->pipeline =
        PipelineBuilder<
            rocket::client::RocketClientConnection::TransportHandler,
            rocket::client::RocketClientAppAdapter,
            channel_pipeline::SimpleBufferAllocator>()
            .setEventBase(&evb_)
            .setHead(connection->transportHandler.get())
            .setTail(connection->appAdapter.get())
            .setAllocator(&connection->allocator)
            .addNextInbound<frame::read::handler::FrameLengthParserHandler>(
                frame_length_parser_handler_tag)
            .addNextOutbound<frame::write::handler::FrameLengthEncoderHandler>(
                frame_length_encoder_handler_tag)
            .addNextDuplex<
                rocket::client::handler::RocketClientFrameCodecHandler>(
                rocket_client_frame_codec_handler_tag)
            .addNextDuplex<
                rocket::client::handler::RocketClientSetupFrameHandler>(
                rocket_client_setup_handler_tag,
                []() {
                  return std::make_pair(
                      folly::IOBuf::copyBuffer("setup"),
                      std::unique_ptr<folly::IOBuf>());
                })
            .addNextInbound<
                rocket::client::handler::RocketClientConnectionErrorHandler>(
                rocket_client_connection_error_handler_tag)
            .addNextDuplex<
                rocket::client::handler::RocketClientStreamStateHandler>(
                rocket_client_stream_state_handler_tag)
            .addNextInbound<
                rocket::client::handler::RocketClientRequestResponseHandler>(
                rocket_client_request_response_handler_tag)
            .build();

    connection->appAdapter->setPipeline(connection->pipeline.get());
    connection->transportHandler->setPipeline(connection->pipeline.get());

    auto channel =
        ThriftClientChannel::newChannel(std::move(connection), protocolId);
    transportHandlerPtr->onConnect();
    evb_.loopOnce();
    testTransport_->getWrittenData(); // discard SETUP frame
    return channel;
  }

  folly::EventBase evb_;
  TestAsyncTransport* testTransport_{nullptr};
};

TEST_F(ThriftClientChannelTest, NewChannelReturnsValidChannel) {
  auto channel = makeConnectedChannel();
  EXPECT_NE(channel, nullptr);
}

TEST_F(ThriftClientChannelTest, NewChannelSetsDefaultProtocolId) {
  auto channel = makeConnectedChannel();
  EXPECT_EQ(
      channel->getProtocolId(), apache::thrift::protocol::T_COMPACT_PROTOCOL);
}

TEST_F(ThriftClientChannelTest, NewChannelWithCustomProtocolId) {
  auto channel =
      makeConnectedChannel(apache::thrift::protocol::T_BINARY_PROTOCOL);
  EXPECT_EQ(
      channel->getProtocolId(), apache::thrift::protocol::T_BINARY_PROTOCOL);
}

TEST_F(ThriftClientChannelTest, GetEventBaseReturnsCorrectEventBase) {
  auto channel = makeConnectedChannel();
  EXPECT_EQ(channel->getEventBase(), &evb_);
}

} // namespace apache::thrift::fast_thrift::thrift
