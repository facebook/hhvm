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

/**
 * End-to-end tests for the Rocket pipeline.
 *
 * These tests connect a client and server via socket pairs and exercise
 * the full pipeline: request -> transport -> server handlers -> server app
 * adapter, and response -> transport -> client handlers -> client app adapter.
 *
 * Both sides use the production RocketClientAppAdapter and
 * RocketServerAppAdapter with callback-based inbound handling.
 */

#include <gtest/gtest.h>

#include <sys/socket.h>

#include <folly/io/IOBuf.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/frame/FrameType.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/BatchingFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/adapter/RocketClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientErrorFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientRequestResponseFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>

namespace apache::thrift::fast_thrift::rocket::test {

using channel_pipeline::HeadToTailOp;
using channel_pipeline::PipelineBuilder;
using channel_pipeline::PipelineImpl;
using channel_pipeline::Result;
using channel_pipeline::TypeErasedBox;
using channel_pipeline::test::TestAllocator;

// Client handler tags
HANDLER_TAG(c_frame_length_parser);
HANDLER_TAG(c_frame_length_encoder);
HANDLER_TAG(c_frame_codec);
HANDLER_TAG(c_setup);
HANDLER_TAG(c_request_response);
HANDLER_TAG(c_error);
HANDLER_TAG(c_stream_state);

// Server handler tags
HANDLER_TAG(s_frame_length_parser);
HANDLER_TAG(s_batching);
HANDLER_TAG(s_frame_length_encoder);
HANDLER_TAG(s_frame_codec);
HANDLER_TAG(s_setup);
HANDLER_TAG(s_request_response);
HANDLER_TAG(s_stream_state);

class RocketE2ETest : public ::testing::Test {
 protected:
  void SetUp() override {
    clientAllocator_.reset();
    serverAllocator_.reset();
  }

  void TearDown() override {
    evbThread_.getEventBase()->runInEventBaseThreadAndWait([this]() {
      clientPipeline_.reset();
      serverPipeline_.reset();
      if (clientTransport_) {
        clientTransport_->onClose(folly::exception_wrapper{});
        clientTransport_.reset();
      }
      if (serverTransport_) {
        serverTransport_->onClose(folly::exception_wrapper{});
        serverTransport_.reset();
      }
    });
  }

  std::pair<folly::NetworkSocket, folly::NetworkSocket> createSocketPair() {
    std::array<int, 2> sockets{};
    int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, sockets.data());
    EXPECT_EQ(ret, 0) << "Failed to create socket pair";
    return {folly::NetworkSocket(sockets[0]), folly::NetworkSocket(sockets[1])};
  }

  void setupPipelines() {
    auto [clientFd, serverFd] = createSocketPair();
    auto* evb = evbThread_.getEventBase();

    evb->runInEventBaseThreadAndWait([this, evb, clientFd, serverFd]() {
      // Create async sockets
      auto clientSocket =
          folly::AsyncSocket::UniquePtr(new folly::AsyncSocket(evb, clientFd));
      auto serverSocket =
          folly::AsyncSocket::UniquePtr(new folly::AsyncSocket(evb, serverFd));

      // Create transport handlers
      clientTransport_ =
          transport::TransportHandler::create(std::move(clientSocket));
      serverTransport_ =
          transport::TransportHandler::create(std::move(serverSocket));

      // Wire client callbacks
      clientAppAdapter_->setResponseHandlers(
          [this](TypeErasedBox&& msg) noexcept -> Result {
            clientResponseCount_++;
            clientResponses_.push_back(std::move(msg));
            if (clientResponseBaton_) {
              clientResponseBaton_->post();
            }
            return Result::Success;
          },
          [this](folly::exception_wrapper&& /*e*/) noexcept {
            clientErrorCount_++;
          });

      // Wire server callbacks
      serverAppAdapter_->setRequestHandlers(
          [this](TypeErasedBox&& msg) noexcept -> Result {
            serverRequestCount_++;
            serverRequests_.push_back(std::move(msg));

            // Auto-respond if enabled
            if (autoRespond_) {
              auto& req = serverRequests_.back()
                              .get<rocket::server::RocketRequestMessage>();
              server::RocketResponseMessage response{
                  .payload = folly::IOBuf::copyBuffer("e2e-response"),
                  .metadata = nullptr,
                  .streamId = req.streamId,
                  .complete = true,
              };
              (void)serverAppAdapter_->write(std::move(response));
            }

            if (serverRequestBaton_) {
              serverRequestBaton_->post();
            }
            return Result::Success;
          },
          [this](folly::exception_wrapper&& /*e*/) noexcept {
            serverErrorCount_++;
          });

      // Build client pipeline
      clientPipeline_ =
          PipelineBuilder<
              transport::TransportHandler,
              client::RocketClientAppAdapter,
              TestAllocator>()
              .setEventBase(evb)
              .setHead(clientTransport_.get())
              .setTail(clientAppAdapter_.get())
              .setAllocator(&clientAllocator_)
              .addNextInbound<frame::read::handler::FrameLengthParserHandler>(
                  c_frame_length_parser_tag)
              .addNextOutbound<
                  frame::write::handler::FrameLengthEncoderHandler>(
                  c_frame_length_encoder_tag)
              .addNextDuplex<client::handler::RocketClientFrameCodecHandler>(
                  c_frame_codec_tag)
              .addNextDuplex<client::handler::RocketClientSetupFrameHandler>(
                  c_setup_tag,
                  []() {
                    return std::make_pair(
                        folly::IOBuf::copyBuffer("setup"),
                        std::unique_ptr<folly::IOBuf>());
                  })
              .addNextDuplex<
                  client::handler::RocketClientRequestResponseFrameHandler>(
                  c_request_response_tag)
              .addNextInbound<client::handler::RocketClientErrorFrameHandler>(
                  c_error_tag)
              .addNextDuplex<client::handler::RocketClientStreamStateHandler>(
                  c_stream_state_tag)
              .build();

      clientAppAdapter_->setPipeline(clientPipeline_.get());
      clientTransport_->setPipeline(*clientPipeline_);

      // Build server pipeline
      serverPipeline_ =
          PipelineBuilder<
              transport::TransportHandler,
              server::RocketServerAppAdapter,
              TestAllocator>()
              .setEventBase(evb)
              .setHead(serverTransport_.get())
              .setTail(serverAppAdapter_.get())
              .setAllocator(&serverAllocator_)
              .addNextOutbound<frame::write::handler::BatchingFrameHandler>(
                  s_batching_tag)
              .addNextInbound<frame::read::handler::FrameLengthParserHandler>(
                  s_frame_length_parser_tag)
              .addNextOutbound<
                  frame::write::handler::FrameLengthEncoderHandler>(
                  s_frame_length_encoder_tag)
              .addNextDuplex<server::handler::RocketServerFrameCodecHandler>(
                  s_frame_codec_tag)
              .addNextDuplex<server::handler::RocketServerSetupFrameHandler>(
                  s_setup_tag)
              .addNextDuplex<
                  server::handler::RocketServerRequestResponseFrameHandler>(
                  s_request_response_tag)
              .addNextDuplex<server::handler::RocketServerStreamStateHandler>(
                  s_stream_state_tag)
              .build();

      serverAppAdapter_->setPipeline(serverPipeline_.get());
      serverTransport_->setPipeline(*serverPipeline_);

      // Activate both sides — server first so it's ready for the SETUP frame
      serverTransport_->onConnect();
      clientTransport_->onConnect();
    });
  }

  RocketRequestMessage createRequest(std::unique_ptr<folly::IOBuf> data) {
    return RocketRequestMessage{
        .frame =
            RocketFramePayload{
                .metadata = nullptr,
                .data = std::move(data),
            },
        .frameType = frame::FrameType::REQUEST_RESPONSE,
    };
  }

  folly::ScopedEventBaseThread evbThread_;
  client::RocketClientAppAdapter::Ptr clientAppAdapter_{
      new client::RocketClientAppAdapter()};
  server::RocketServerAppAdapter::Ptr serverAppAdapter_{
      new server::RocketServerAppAdapter()};
  transport::TransportHandler::Ptr clientTransport_;
  transport::TransportHandler::Ptr serverTransport_;
  PipelineImpl::Ptr clientPipeline_;
  PipelineImpl::Ptr serverPipeline_;
  TestAllocator clientAllocator_;
  TestAllocator serverAllocator_;

  // Client-side counters
  std::atomic<int> clientResponseCount_{0};
  int clientErrorCount_{0};
  std::vector<TypeErasedBox> clientResponses_;

  // Server-side counters
  std::atomic<int> serverRequestCount_{0};
  int serverErrorCount_{0};
  std::vector<TypeErasedBox> serverRequests_;

  // Synchronization batons (set per-test as needed)
  folly::Baton<>* serverRequestBaton_{nullptr};
  folly::Baton<>* clientResponseBaton_{nullptr};

  // Auto-respond flag
  bool autoRespond_{false};
};

// =============================================================================
// E2E: Client -> Server
// =============================================================================

TEST_F(RocketE2ETest, ClientRequestReachesServer) {
  folly::Baton<> requestReceived;
  serverRequestBaton_ = &requestReceived;

  setupPipelines();

  evbThread_.getEventBase()->runInEventBaseThread([this]() {
    auto request = createRequest(folly::IOBuf::copyBuffer("hello server"));
    (void)clientAppAdapter_->write(std::move(request));
  });

  ASSERT_TRUE(requestReceived.try_wait_for(std::chrono::seconds(5)))
      << "Timed out waiting for server to receive request";
  EXPECT_EQ(serverRequestCount_.load(), 1);
}

// =============================================================================
// E2E: Client -> Server -> Client (round trip)
// =============================================================================

TEST_F(RocketE2ETest, RoundTripRequestResponse) {
  folly::Baton<> responseReceived;
  clientResponseBaton_ = &responseReceived;
  autoRespond_ = true;

  setupPipelines();

  evbThread_.getEventBase()->runInEventBaseThread([this]() {
    auto request = createRequest(folly::IOBuf::copyBuffer("ping"));
    (void)clientAppAdapter_->write(std::move(request));
  });

  ASSERT_TRUE(responseReceived.try_wait_for(std::chrono::seconds(5)))
      << "Timed out waiting for round-trip response";
  EXPECT_EQ(serverRequestCount_.load(), 1);
  EXPECT_EQ(clientResponseCount_.load(), 1);
}

TEST_F(RocketE2ETest, MultipleRoundTrips) {
  autoRespond_ = true;
  setupPipelines();

  constexpr int kNumRequests = 5;
  for (int i = 0; i < kNumRequests; ++i) {
    folly::Baton<> responseReceived;
    clientResponseBaton_ = &responseReceived;

    evbThread_.getEventBase()->runInEventBaseThread([this, i]() {
      auto request = createRequest(
          folly::IOBuf::copyBuffer("request-" + std::to_string(i)));
      (void)clientAppAdapter_->write(std::move(request));
    });

    ASSERT_TRUE(responseReceived.try_wait_for(std::chrono::seconds(5)))
        << "Timed out on request " << i;
  }

  EXPECT_EQ(serverRequestCount_.load(), kNumRequests);
  EXPECT_EQ(clientResponseCount_.load(), kNumRequests);
}

} // namespace apache::thrift::fast_thrift::rocket::test
