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

#include <atomic>
#include <memory>

#include <folly/coro/BlockingWait.h>
#include <folly/coro/Collect.h>
#include <folly/coro/Task.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/adapter/RocketClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/common/RocketClientConnection.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientErrorFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientRequestResponseHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientChannel.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/adapter/ThriftClientTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientMetadataPushHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/BackwardsCompatibilityTestFastChildService.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/BackwardsCompatibilityTestFastChildServiceAsyncClient.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/BackwardsCompatibilityTestFastService.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/BackwardsCompatibilityTestFastServiceAsyncClient.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/BackwardsCompatibilityTestService.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/BackwardsCompatibilityTestServiceAsyncClient.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/server/PreprocessParams.h>
#include <thrift/lib/cpp2/server/PreprocessResult.h>
#include <thrift/lib/cpp2/server/ThriftServer.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_constants.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift::client::test {

using apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder;
using apache::thrift::fast_thrift::channel_pipeline::PipelineImpl;
using apache::thrift::fast_thrift::channel_pipeline::SimpleBufferAllocator;

using apache::thrift::fast_thrift::thrift::test::
    BackwardsCompatibilityTestFastChildService;
using apache::thrift::fast_thrift::thrift::test::
    BackwardsCompatibilityTestFastService;
using apache::thrift::fast_thrift::thrift::test::
    BackwardsCompatibilityTestService;

// Handler tags for pipeline construction
HANDLER_TAG(frame_length_parser_handler);
HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(rocket_client_frame_codec_handler);
HANDLER_TAG(rocket_client_setup_handler);
HANDLER_TAG(rocket_client_request_response_handler);
HANDLER_TAG(rocket_client_error_frame_handler);
HANDLER_TAG(rocket_client_stream_state_handler);
HANDLER_TAG(thrift_client_metadata_push_handler);

/**
 * ConnectCallback - Callback for socket connection that triggers
 * transportHandler->onConnect() when connection is established.
 */
class ConnectCallback : public folly::AsyncSocket::ConnectCallback {
 public:
  explicit ConnectCallback(
      apache::thrift::fast_thrift::transport::TransportHandler*
          transportHandler,
      folly::Baton<>& baton,
      bool& connected)
      : transportHandler_(transportHandler),
        baton_(baton),
        connected_(connected) {}

  void connectSuccess() noexcept override {
    connected_ = true;
    transportHandler_->onConnect();
    baton_.post();
  }

  void connectErr(const folly::AsyncSocketException& /*ex*/) noexcept override {
    connected_ = false;
    baton_.post();
  }

 private:
  apache::thrift::fast_thrift::transport::TransportHandler* transportHandler_;
  folly::Baton<>& baton_;
  bool& connected_;
};

/**
 * BackwardsCompatibilityTestHandler - Implementation of the generated
 * BackwardsCompatibilityTestServiceSvIf interface.
 *
 * This handler implements the service methods defined in
 * BackwardsCompatibilityTestService.thrift.
 */
class BackwardsCompatibilityTestHandler
    : public apache::thrift::ServiceHandler<BackwardsCompatibilityTestService> {
 public:
  void echo(
      std::string& response, std::unique_ptr<std::string> message) override {
    response = *message;
  }

  int64_t add(int64_t a, int64_t b) override { return a + b; }

  void sendResponse(std::string& response, int64_t size) override {
    response = std::string(static_cast<size_t>(size), 'x');
  }

  void ping() override {}

  void throwError(std::unique_ptr<std::string> message) override {
    throw apache::thrift::TApplicationException(
        apache::thrift::TApplicationException::UNKNOWN, *message);
  }
};

class BackwardsCompatibilityTestFastHandler
    : public apache::thrift::ServiceHandler<
          BackwardsCompatibilityTestFastService> {
 public:
  void echo(
      std::string& response, std::unique_ptr<std::string> message) override {
    response = *message;
  }

  int64_t add(int64_t a, int64_t b) override { return a + b; }

  void sendResponse(std::string& response, int64_t size) override {
    response = std::string(static_cast<size_t>(size), 'x');
  }

  void ping() override {}

  void throwError(std::unique_ptr<std::string> message) override {
    throw apache::thrift::TApplicationException(
        apache::thrift::TApplicationException::UNKNOWN, *message);
  }
};

class BackwardsCompatibilityTestFastChildHandler
    : public apache::thrift::ServiceHandler<
          BackwardsCompatibilityTestFastChildService> {
 public:
  // Inherited from parent.
  void echo(
      std::string& response, std::unique_ptr<std::string> message) override {
    response = *message;
  }

  int64_t add(int64_t a, int64_t b) override { return a + b; }

  void sendResponse(std::string& response, int64_t size) override {
    response = std::string(static_cast<size_t>(size), 'x');
  }

  void ping() override {}

  void throwError(std::unique_ptr<std::string> message) override {
    throw apache::thrift::TApplicationException(
        apache::thrift::TApplicationException::UNKNOWN, *message);
  }

  // Child-only.
  void childEcho(
      std::string& response, std::unique_ptr<std::string> message) override {
    response = "child:" + *message;
  }
};

/**
 * ThriftClientBackwardsCompatibilityE2ETest - E2E integration test for
 * fast_thrift client.
 *
 * Uses a standard ThriftServer (via ScopedServerInterfaceThread) with
 * a fast_thrift client channel to verify end-to-end communication.
 */
class ThriftClientBackwardsCompatibilityE2ETest : public ::testing::Test {
 protected:
  void SetUp() override {
    handler_ = std::make_shared<BackwardsCompatibilityTestHandler>();
    server_ =
        std::make_unique<apache::thrift::ScopedServerInterfaceThread>(handler_);
    clientThread_ = std::make_unique<folly::ScopedEventBaseThread>();
  }

  void TearDown() override {
    clientThread_->getEventBase()->runInEventBaseThreadAndWait([&] {
      clientPipeline_.reset();
      clientTransportAdapter_.reset();
    });
    channel_.reset();
    server_.reset();
    clientThread_.reset();
  }

  /**
   * Create a fast_thrift client channel connected to the test server.
   *
   * The channel is constructed with the full pipeline:
   * - FrameLengthParserHandler: Parses frames from raw bytes
   * - FrameLengthEncoderHandler: Encodes frames with length prefix
   * - RocketClientFrameCodecHandler: Bidirectional codec for Rocket frames
   * - RocketClientSetupFrameHandler: Handles initial SETUP frame
   * - RocketClientRequestResponseHandler: Handles REQUEST_RESPONSE frames
   * - RocketClientStreamStateHandler: Manages stream state
   * - ThriftClientMetadataHandler: Handles Thrift metadata
   */
  thrift::ThriftClientChannel::UniquePtr createFastThriftChannel() {
    auto* evb = clientThread_->getEventBase();
    thrift::ThriftClientChannel::UniquePtr channel;
    folly::Baton<> connectBaton;
    bool connected = false;

    evb->runInEventBaseThreadAndWait([&] {
      auto socket = folly::AsyncSocket::newSocket(evb);
      auto* socketPtr = socket.get();

      // 1. Build rocket pipeline inside RocketClientConnection
      auto connection = std::make_unique<apache::thrift::fast_thrift::rocket::
                                             client::RocketClientConnection>();

      connection->transportHandler =
          apache::thrift::fast_thrift::transport::TransportHandler::create(
              std::move(socket));

      auto* transportHandlerPtr = connection->transportHandler.get();
      auto setupFactory = []() {
        apache::thrift::RequestSetupMetadata meta;
        // Set version info like RocketClientChannelBase does
        meta.minVersion() = 8;
        meta.maxVersion() = 10;
        // Add client metadata
        auto& clientMetadata = meta.clientMetadata().ensure();
        clientMetadata.agent() = "fast_thrift_test";

        // Serialize using BinaryProtocol
        apache::thrift::BinaryProtocolWriter writer;
        folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
        writer.setOutput(&queue);
        meta.write(&writer);

        // Prepend the rocket protocol key
        folly::IOBufQueue result(folly::IOBufQueue::cacheChainLength());
        const uint32_t protocolKey =
            apache::thrift::RpcMetadata_constants::kRocketProtocolKey();
        folly::io::QueueAppender appender(&result, sizeof(protocolKey));
        appender.writeBE<uint32_t>(protocolKey);
        result.append(queue.move());

        return std::make_pair(result.move(), std::unique_ptr<folly::IOBuf>());
      };

      connection->pipeline =
          PipelineBuilder<
              apache::thrift::fast_thrift::transport::TransportHandler,
              apache::thrift::fast_thrift::rocket::client::
                  RocketClientAppAdapter,
              SimpleBufferAllocator>()
              .setEventBase(evb)
              .setHead(connection->transportHandler.get())
              .setTail(connection->appAdapter.get())
              .setAllocator(&connection->allocator)
              .addNextInbound<apache::thrift::fast_thrift::frame::read::
                                  handler::FrameLengthParserHandler>(
                  frame_length_parser_handler_tag)
              .addNextOutbound<apache::thrift::fast_thrift::frame::write::
                                   handler::FrameLengthEncoderHandler>(
                  frame_length_encoder_handler_tag)
              .addNextDuplex<apache::thrift::fast_thrift::rocket::client::
                                 handler::RocketClientFrameCodecHandler>(
                  rocket_client_frame_codec_handler_tag)
              .addNextDuplex<apache::thrift::fast_thrift::rocket::client::
                                 handler::RocketClientSetupFrameHandler>(
                  rocket_client_setup_handler_tag, std::move(setupFactory))
              .addNextInbound<apache::thrift::fast_thrift::rocket::client::
                                  handler::RocketClientErrorFrameHandler>(
                  rocket_client_error_frame_handler_tag)
              .addNextDuplex<apache::thrift::fast_thrift::rocket::client::
                                 handler::RocketClientStreamStateHandler>(
                  rocket_client_stream_state_handler_tag)
              .addNextInbound<apache::thrift::fast_thrift::rocket::client::
                                  handler::RocketClientRequestResponseHandler>(
                  rocket_client_request_response_handler_tag)
              .build();

      connection->appAdapter->setPipeline(connection->pipeline.get());
      connection->transportHandler->setPipeline(*connection->pipeline);

      // 2. Build thrift pipeline: ThriftClientChannel → TransportAdapter
      channel = thrift::ThriftClientChannel::newChannel(evb);

      clientTransportAdapter_ =
          std::make_unique<thrift::client::ThriftClientTransportAdapter>(
              std::move(connection));

      clientPipeline_ =
          PipelineBuilder<
              thrift::client::ThriftClientTransportAdapter,
              thrift::ThriftClientChannel,
              SimpleBufferAllocator>()
              .setEventBase(evb)
              .setHead(clientTransportAdapter_.get())
              .setTail(channel.get())
              .setAllocator(&allocator_)
              .addNextInbound<
                  thrift::client::handler::ThriftClientMetadataPushHandler>(
                  thrift_client_metadata_push_handler_tag)
              .build();

      channel->setPipeline(clientPipeline_.get());
      clientTransportAdapter_->setPipeline(clientPipeline_.get());

      connectCallback_ = std::make_unique<ConnectCallback>(
          transportHandlerPtr, connectBaton, connected);
      socketPtr->connect(connectCallback_.get(), server_->getAddress(), 30000);
    });

    // Wait for connection to complete
    connectBaton.wait();

    if (!connected) {
      throw std::runtime_error("Failed to connect to server");
    }

    return channel;
  }

  std::shared_ptr<BackwardsCompatibilityTestHandler> handler_;
  std::unique_ptr<apache::thrift::ScopedServerInterfaceThread> server_;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
  thrift::ThriftClientChannel::UniquePtr channel_;
  std::unique_ptr<thrift::client::ThriftClientTransportAdapter>
      clientTransportAdapter_;
  PipelineImpl::Ptr clientPipeline_;
  SimpleBufferAllocator allocator_;
  std::unique_ptr<ConnectCallback> connectCallback_;
};

TEST_F(ThriftClientBackwardsCompatibilityE2ETest, Ping) {
  channel_ = createFastThriftChannel();

  auto client = std::make_unique<
      apache::thrift::Client<BackwardsCompatibilityTestService>>(
      std::move(channel_));

  folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(), client->co_ping()));

  // Destroy the client in the EventBase thread
  clientThread_->getEventBase()->runInEventBaseThreadAndWait(
      [&] { client.reset(); });
}

TEST_F(ThriftClientBackwardsCompatibilityE2ETest, EchoRequestResponse) {
  channel_ = createFastThriftChannel();

  auto client = std::make_unique<
      apache::thrift::Client<BackwardsCompatibilityTestService>>(
      std::move(channel_));

  auto result = folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(), client->co_echo("hello world")));
  EXPECT_EQ(result, "hello world");

  // Destroy the client in the EventBase thread
  clientThread_->getEventBase()->runInEventBaseThreadAndWait(
      [&] { client.reset(); });
}

TEST_F(ThriftClientBackwardsCompatibilityE2ETest, AddNumbers) {
  channel_ = createFastThriftChannel();

  auto client = std::make_unique<
      apache::thrift::Client<BackwardsCompatibilityTestService>>(
      std::move(channel_));

  auto result = folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(), client->co_add(17, 25)));
  EXPECT_EQ(result, 42);

  // Destroy the client in the EventBase thread
  clientThread_->getEventBase()->runInEventBaseThreadAndWait(
      [&] { client.reset(); });
}

TEST_F(ThriftClientBackwardsCompatibilityE2ETest, MultipleRequests) {
  channel_ = createFastThriftChannel();

  auto client = std::make_unique<
      apache::thrift::Client<BackwardsCompatibilityTestService>>(
      std::move(channel_));
  auto* evb = clientThread_->getEventBase();

  auto [r1, r2, r3] = folly::coro::blockingWait(
      folly::coro::collectAll(
          folly::coro::co_withExecutor(evb, client->co_echo("first")),
          folly::coro::co_withExecutor(evb, client->co_echo("second")),
          folly::coro::co_withExecutor(evb, client->co_echo("third"))));

  EXPECT_EQ(r1, "first");
  EXPECT_EQ(r2, "second");
  EXPECT_EQ(r3, "third");

  // Destroy the client in the EventBase thread
  clientThread_->getEventBase()->runInEventBaseThreadAndWait(
      [&] { client.reset(); });
}

TEST_F(ThriftClientBackwardsCompatibilityE2ETest, LargeResponse) {
  channel_ = createFastThriftChannel();

  auto client = std::make_unique<
      apache::thrift::Client<BackwardsCompatibilityTestService>>(
      std::move(channel_));

  constexpr int64_t kResponseSize = 10000;

  auto result = folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(),
          client->co_sendResponse(kResponseSize)));
  EXPECT_EQ(result.size(), kResponseSize);
  EXPECT_EQ(result, std::string(kResponseSize, 'x'));

  // Destroy the client in the EventBase thread
  clientThread_->getEventBase()->runInEventBaseThreadAndWait(
      [&] { client.reset(); });
}

// =============================================================================
// Error Handling E2E Tests
// =============================================================================

TEST_F(
    ThriftClientBackwardsCompatibilityE2ETest,
    ServerThrowsAppExceptionDeliveredAsException) {
  channel_ = createFastThriftChannel();

  auto client = std::make_unique<
      apache::thrift::Client<BackwardsCompatibilityTestService>>(
      std::move(channel_));

  try {
    folly::coro::blockingWait(
        folly::coro::co_withExecutor(
            clientThread_->getEventBase(),
            client->co_throwError("server error message")));
    FAIL() << "Expected TApplicationException";
  } catch (const apache::thrift::TApplicationException& ex) {
    EXPECT_NE(
        std::string(ex.what()).find("server error message"), std::string::npos);
  }

  // Destroy the client in the EventBase thread
  clientThread_->getEventBase()->runInEventBaseThreadAndWait(
      [&] { client.reset(); });
}

TEST_F(
    ThriftClientBackwardsCompatibilityE2ETest,
    ServerShutdownCausesTransportError) {
  channel_ = createFastThriftChannel();

  auto client = std::make_unique<
      apache::thrift::Client<BackwardsCompatibilityTestService>>(
      std::move(channel_));

  // Verify connection works first
  folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(), client->co_ping()));

  // Stop the server
  server_.reset();

  // Next request should fail with a transport or app exception
  EXPECT_THROW(
      folly::coro::blockingWait(
          folly::coro::co_withExecutor(
              clientThread_->getEventBase(),
              client->co_echo("after shutdown"))),
      std::exception);

  // Destroy the client in the EventBase thread
  clientThread_->getEventBase()->runInEventBaseThreadAndWait(
      [&] { client.reset(); });
}

TEST_F(ThriftClientBackwardsCompatibilityE2ETest, RequestAfterThrowStillWorks) {
  channel_ = createFastThriftChannel();

  auto client = std::make_unique<
      apache::thrift::Client<BackwardsCompatibilityTestService>>(
      std::move(channel_));

  // First request throws
  EXPECT_THROW(
      folly::coro::blockingWait(
          folly::coro::co_withExecutor(
              clientThread_->getEventBase(),
              client->co_throwError("first error"))),
      apache::thrift::TApplicationException);

  // Connection should still work for subsequent requests
  auto result = folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(), client->co_echo("still alive")));
  EXPECT_EQ(result, "still alive");

  // Destroy the client in the EventBase thread
  clientThread_->getEventBase()->runInEventBaseThreadAndWait(
      [&] { client.reset(); });
}

TEST_F(
    ThriftClientBackwardsCompatibilityE2ETest,
    ServerOverloadRejectsWithLoadshedding) {
  channel_ = createFastThriftChannel();

  auto client = std::make_unique<
      apache::thrift::Client<BackwardsCompatibilityTestService>>(
      std::move(channel_));

  // Verify connection works first
  folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(), client->co_ping()));

  // Make server reject all requests as overloaded.
  // Server will send ERROR frame with REJECTED +
  // ResponseRpcError{APP_OVERLOAD}. This exercises decodeErrorFrameAsResponse()
  // → onResponse(T_EXCEPTION) path.
  server_->getThriftServer().addPreprocessFunc(
      "overload_test",
      [](const apache::thrift::server::PreprocessParams&)
          -> apache::thrift::PreprocessResult {
        return apache::thrift::AppOverloadedException(
            "test", "server overloaded");
      });

  try {
    folly::coro::blockingWait(
        folly::coro::co_withExecutor(
            clientThread_->getEventBase(),
            client->co_echo("should be rejected")));
    FAIL() << "Expected TApplicationException";
  } catch (const apache::thrift::TApplicationException& ex) {
    EXPECT_EQ(ex.getType(), apache::thrift::TApplicationException::LOADSHEDDING)
        << "Overload rejection should map to LOADSHEDDING, got: "
        << ex.getType();
  }

  // Destroy the client in the EventBase thread
  clientThread_->getEventBase()->runInEventBaseThreadAndWait(
      [&] { client.reset(); });
}

TEST_F(
    ThriftClientBackwardsCompatibilityE2ETest,
    OverloadRecoveryAfterServerUnblocks) {
  channel_ = createFastThriftChannel();

  auto client = std::make_unique<
      apache::thrift::Client<BackwardsCompatibilityTestService>>(
      std::move(channel_));

  // Set overloaded via a togglable flag
  auto overloaded = std::make_shared<std::atomic<bool>>(true);
  server_->getThriftServer().addPreprocessFunc(
      "overload_test",
      [overloaded](const apache::thrift::server::PreprocessParams&)
          -> apache::thrift::PreprocessResult {
        if (overloaded->load()) {
          return apache::thrift::AppOverloadedException(
              "test", "server overloaded");
        }
        return {};
      });

  // Request should be rejected
  EXPECT_THROW(
      folly::coro::blockingWait(
          folly::coro::co_withExecutor(
              clientThread_->getEventBase(), client->co_echo("rejected"))),
      apache::thrift::TApplicationException);

  // Unblock the server
  overloaded->store(false);

  // Connection should still work after overload clears
  auto result = folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(), client->co_echo("recovered")));
  EXPECT_EQ(result, "recovered");

  // Destroy the client in the EventBase thread
  clientThread_->getEventBase()->runInEventBaseThreadAndWait(
      [&] { client.reset(); });
}

// =============================================================================
// FastClient E2E Tests (using ThriftClientAppAdapter + generated FastClient)
// =============================================================================

class BackwardsCompatibilityFastClientE2ETest : public ::testing::Test {
 protected:
  void SetUp() override {
    handler_ = std::make_shared<BackwardsCompatibilityTestFastHandler>();
    server_ =
        std::make_unique<apache::thrift::ScopedServerInterfaceThread>(handler_);
    clientThread_ = std::make_unique<folly::ScopedEventBaseThread>();
  }

  void TearDown() override {
    clientThread_->getEventBase()->runInEventBaseThreadAndWait([&] {
      thriftPipeline_.reset();
      transportAdapter_.reset();
    });
    server_.reset();
    clientThread_.reset();
  }

  template <typename Service = BackwardsCompatibilityTestFastService>
  std::unique_ptr<
      apache::thrift::FastClient<Service, thrift::ThriftClientAppAdapter>>
  createFastClient() {
    auto* evb = clientThread_->getEventBase();
    folly::Baton<> connectBaton;
    bool connected = false;

    thrift::ThriftClientAppAdapter::Ptr adapter(
        new thrift::ThriftClientAppAdapter(
            static_cast<uint16_t>(
                apache::thrift::protocol::T_COMPACT_PROTOCOL)));

    evb->runInEventBaseThreadAndWait([&] {
      auto socket = folly::AsyncSocket::newSocket(evb);
      auto* socketPtr = socket.get();

      // 1. Build rocket pipeline inside RocketClientConnection
      auto connection =
          std::make_unique<rocket::client::RocketClientConnection>();

      connection->transportHandler =
          transport::TransportHandler::create(std::move(socket));

      auto* transportHandlerPtr = connection->transportHandler.get();

      connectCallback_ = std::make_unique<ConnectCallback>(
          transportHandlerPtr, connectBaton, connected);

      socketPtr->connect(connectCallback_.get(), server_->getAddress(), 30000);

      auto setupFactory = []() {
        apache::thrift::RequestSetupMetadata meta;
        meta.minVersion() = 8;
        meta.maxVersion() = 10;
        meta.clientMetadata().ensure().agent() = "fast_thrift_fast_client_test";

        apache::thrift::BinaryProtocolWriter writer;
        folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
        writer.setOutput(&queue);
        meta.write(&writer);

        folly::IOBufQueue result(folly::IOBufQueue::cacheChainLength());
        const uint32_t protocolKey =
            apache::thrift::RpcMetadata_constants::kRocketProtocolKey();
        folly::io::QueueAppender appender(&result, sizeof(protocolKey));
        appender.writeBE<uint32_t>(protocolKey);
        result.append(queue.move());

        return std::make_pair(result.move(), std::unique_ptr<folly::IOBuf>());
      };

      connection->pipeline =
          PipelineBuilder<
              transport::TransportHandler,
              rocket::client::RocketClientAppAdapter,
              SimpleBufferAllocator>()
              .setEventBase(evb)
              .setHead(connection->transportHandler.get())
              .setTail(connection->appAdapter.get())
              .setAllocator(&connection->allocator)
              .addNextInbound<frame::read::handler::FrameLengthParserHandler>(
                  frame_length_parser_handler_tag)
              .addNextOutbound<
                  frame::write::handler::FrameLengthEncoderHandler>(
                  frame_length_encoder_handler_tag)
              .addNextDuplex<
                  rocket::client::handler::RocketClientFrameCodecHandler>(
                  rocket_client_frame_codec_handler_tag)
              .addNextDuplex<
                  rocket::client::handler::RocketClientSetupFrameHandler>(
                  rocket_client_setup_handler_tag, std::move(setupFactory))
              .template addNextInbound<
                  rocket::client::handler::RocketClientErrorFrameHandler>(
                  rocket_client_error_frame_handler_tag)
              .template addNextDuplex<
                  rocket::client::handler::RocketClientStreamStateHandler>(
                  rocket_client_stream_state_handler_tag)
              .template addNextInbound<
                  rocket::client::handler::RocketClientRequestResponseHandler>(
                  rocket_client_request_response_handler_tag)
              .build();

      connection->appAdapter->setPipeline(connection->pipeline.get());
      connection->transportHandler->setPipeline(*connection->pipeline);

      // 2. Build thrift pipeline: AppAdapter → TransportAdapter
      transportAdapter_ =
          std::make_unique<thrift::client::ThriftClientTransportAdapter>(
              std::move(connection));

      thriftPipeline_ =
          PipelineBuilder<
              thrift::client::ThriftClientTransportAdapter,
              thrift::ThriftClientAppAdapter,
              SimpleBufferAllocator>()
              .setEventBase(evb)
              .setHead(transportAdapter_.get())
              .setTail(adapter.get())
              .setAllocator(&allocator_)
              .addNextInbound<
                  thrift::client::handler::ThriftClientMetadataPushHandler>(
                  thrift_client_metadata_push_handler_tag)
              .build();

      adapter->setPipeline(thriftPipeline_.get());
      transportAdapter_->setPipeline(thriftPipeline_.get());
    });

    connectBaton.wait();
    if (!connected) {
      throw std::runtime_error("Failed to connect to server");
    }

    return std::make_unique<
        apache::thrift::FastClient<Service, thrift::ThriftClientAppAdapter>>(
        std::move(adapter));
  }

  std::shared_ptr<BackwardsCompatibilityTestFastHandler> handler_;
  std::unique_ptr<apache::thrift::ScopedServerInterfaceThread> server_;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
  std::unique_ptr<thrift::client::ThriftClientTransportAdapter>
      transportAdapter_;
  channel_pipeline::PipelineImpl::Ptr thriftPipeline_;
  SimpleBufferAllocator allocator_;
  std::unique_ptr<ConnectCallback> connectCallback_;
};

TEST_F(BackwardsCompatibilityFastClientE2ETest, Ping) {
  auto client = createFastClient();

  folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(), client->co_ping()));
}

TEST_F(BackwardsCompatibilityFastClientE2ETest, EchoRequestResponse) {
  auto client = createFastClient();

  auto result = folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(), client->co_echo("hello world")));
  EXPECT_EQ(result, "hello world");
}

TEST_F(BackwardsCompatibilityFastClientE2ETest, AddNumbers) {
  auto client = createFastClient();

  auto result = folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(), client->co_add(17, 25)));
  EXPECT_EQ(result, 42);
}

TEST_F(BackwardsCompatibilityFastClientE2ETest, MultipleRequests) {
  auto client = createFastClient();
  auto* evb = clientThread_->getEventBase();

  auto [r1, r2, r3] = folly::coro::blockingWait(
      folly::coro::collectAll(
          folly::coro::co_withExecutor(evb, client->co_echo("first")),
          folly::coro::co_withExecutor(evb, client->co_echo("second")),
          folly::coro::co_withExecutor(evb, client->co_echo("third"))));

  EXPECT_EQ(r1, "first");
  EXPECT_EQ(r2, "second");
  EXPECT_EQ(r3, "third");
}

TEST_F(BackwardsCompatibilityFastClientE2ETest, LargeResponse) {
  auto client = createFastClient();

  constexpr int64_t kResponseSize = 10000;

  auto result = folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(),
          client->co_sendResponse(kResponseSize)));
  EXPECT_EQ(result.size(), kResponseSize);
  EXPECT_EQ(result, std::string(kResponseSize, 'x'));
}

// =============================================================================
// FastClient Error Handling E2E Tests
// =============================================================================

TEST_F(
    BackwardsCompatibilityFastClientE2ETest,
    ServerThrowsAppExceptionDeliveredAsException) {
  auto client = createFastClient();

  try {
    folly::coro::blockingWait(
        folly::coro::co_withExecutor(
            clientThread_->getEventBase(),
            client->co_throwError("fast client error")));
    FAIL() << "Expected exception";
  } catch (const apache::thrift::TApplicationException& ex) {
    EXPECT_NE(
        std::string(ex.what()).find("fast client error"), std::string::npos);
  }
}

TEST_F(BackwardsCompatibilityFastClientE2ETest, RequestAfterThrowStillWorks) {
  auto client = createFastClient();

  EXPECT_THROW(
      folly::coro::blockingWait(
          folly::coro::co_withExecutor(
              clientThread_->getEventBase(),
              client->co_throwError("first error"))),
      apache::thrift::TApplicationException);

  // Connection should still work
  auto result = folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(), client->co_echo("still alive")));
  EXPECT_EQ(result, "still alive");
}

TEST_F(
    BackwardsCompatibilityFastClientE2ETest,
    ServerOverloadRejectsWithLoadshedding) {
  auto client = createFastClient();

  // Verify connection works first
  folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(), client->co_ping()));

  // Make server reject all requests as overloaded
  server_->getThriftServer().addPreprocessFunc(
      "overload_test",
      [](const apache::thrift::server::PreprocessParams&)
          -> apache::thrift::PreprocessResult {
        return apache::thrift::AppOverloadedException(
            "test", "server overloaded");
      });

  try {
    folly::coro::blockingWait(
        folly::coro::co_withExecutor(
            clientThread_->getEventBase(),
            client->co_echo("should be rejected")));
    FAIL() << "Expected TApplicationException";
  } catch (const apache::thrift::TApplicationException& ex) {
    EXPECT_EQ(ex.getType(), apache::thrift::TApplicationException::LOADSHEDDING)
        << "Overload rejection should map to LOADSHEDDING, got: "
        << ex.getType();
  }
}

TEST_F(
    BackwardsCompatibilityFastClientE2ETest,
    OverloadRecoveryAfterServerUnblocks) {
  auto client = createFastClient();

  // Set overloaded via a togglable flag
  auto overloaded = std::make_shared<std::atomic<bool>>(true);
  server_->getThriftServer().addPreprocessFunc(
      "overload_test",
      [overloaded](const apache::thrift::server::PreprocessParams&)
          -> apache::thrift::PreprocessResult {
        if (overloaded->load()) {
          return apache::thrift::AppOverloadedException(
              "test", "server overloaded");
        }
        return {};
      });

  EXPECT_THROW(
      folly::coro::blockingWait(
          folly::coro::co_withExecutor(
              clientThread_->getEventBase(), client->co_echo("rejected"))),
      apache::thrift::TApplicationException);

  // Unblock
  overloaded->store(false);

  // Connection should still work
  auto result = folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(), client->co_echo("recovered")));
  EXPECT_EQ(result, "recovered");
}

// =============================================================================
// FastClient `extends` E2E Tests — verifies that a FastClient<Child> can call
// both inherited (parent-defined) and child-defined methods at runtime.
// =============================================================================

class BackwardsCompatibilityFastChildClientE2ETest
    : public BackwardsCompatibilityFastClientE2ETest {
 protected:
  void SetUp() override {
    childHandler_ =
        std::make_shared<BackwardsCompatibilityTestFastChildHandler>();
    server_ = std::make_unique<apache::thrift::ScopedServerInterfaceThread>(
        childHandler_);
    clientThread_ = std::make_unique<folly::ScopedEventBaseThread>();
  }

  std::shared_ptr<BackwardsCompatibilityTestFastChildHandler> childHandler_;
};

TEST_F(
    BackwardsCompatibilityFastChildClientE2ETest, InheritedPingThroughChild) {
  auto client = createFastClient<BackwardsCompatibilityTestFastChildService>();

  EXPECT_NO_THROW(
      folly::coro::blockingWait(
          folly::coro::co_withExecutor(
              clientThread_->getEventBase(), client->co_ping())));
}

TEST_F(
    BackwardsCompatibilityFastChildClientE2ETest, InheritedEchoThroughChild) {
  auto client = createFastClient<BackwardsCompatibilityTestFastChildService>();

  auto result = folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(), client->co_echo("inherited")));
  EXPECT_EQ(result, "inherited");
}

TEST_F(BackwardsCompatibilityFastChildClientE2ETest, ChildOwnMethod) {
  auto client = createFastClient<BackwardsCompatibilityTestFastChildService>();

  auto result = folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(), client->co_childEcho("hi")));
  EXPECT_EQ(result, "child:hi");
}

} // namespace apache::thrift::fast_thrift::thrift::client::test
