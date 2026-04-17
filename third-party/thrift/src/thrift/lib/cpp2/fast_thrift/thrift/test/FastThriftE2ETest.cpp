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

#include <folly/Synchronized.h>
#include <folly/coro/BlockingWait.h>
#include <folly/coro/Collect.h>
#include <folly/coro/Task.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientErrorFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientRequestResponseFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/connection/ConnectionHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/connection/ConnectionManager.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientChannel.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientMetadataPushHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientRocketInterfaceHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/ThriftServerChannel.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/TestFastService.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/TestFastServiceAsyncClient.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/TestServiceAsyncClient.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_constants.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

THRIFT_FLAG_DECLARE_bool(rocket_client_binary_rpc_metadata_encoding);

namespace apache::thrift::fast_thrift::thrift::test {

using apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder;
using apache::thrift::fast_thrift::channel_pipeline::PipelineImpl;
using apache::thrift::fast_thrift::channel_pipeline::SimpleBufferAllocator;

using apache::thrift::fast_thrift::thrift::test::TestFastService;
using apache::thrift::fast_thrift::thrift::test::TestService;

// Client handler tags
HANDLER_TAG(client_frame_length_parser_handler);
HANDLER_TAG(client_frame_length_encoder_handler);
HANDLER_TAG(rocket_client_frame_codec_handler);
HANDLER_TAG(rocket_client_setup_handler);
HANDLER_TAG(rocket_client_request_response_frame_handler);
HANDLER_TAG(rocket_client_error_frame_handler);
HANDLER_TAG(rocket_client_stream_state_handler);
HANDLER_TAG(thrift_client_rocket_interface_handler);
HANDLER_TAG(thrift_client_metadata_push_handler);

// Server handler tags
HANDLER_TAG(server_frame_length_parser_handler);
HANDLER_TAG(server_frame_length_encoder_handler);
HANDLER_TAG(rocket_server_frame_codec_handler);
HANDLER_TAG(rocket_server_setup_frame_handler);
HANDLER_TAG(rocket_server_request_response_frame_handler);
HANDLER_TAG(rocket_server_stream_state_handler);

/**
 * ConnectCallback - Triggers transportHandler->onConnect() when the
 * TCP connection is established.
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

  void connectErr(const folly::AsyncSocketException&) noexcept override {
    connected_ = false;
    baton_.post();
  }

 private:
  apache::thrift::fast_thrift::transport::TransportHandler* transportHandler_;
  folly::Baton<>& baton_;
  bool& connected_;
};

/**
 * TestHandler - Implements TestServiceSvIf.
 */
class TestHandler : public apache::thrift::ServiceHandler<TestService> {
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
};

/**
 * FastThriftE2ETest - End-to-end integration test with both a fast_thrift
 * client and a fast_thrift server.
 *
 * Server pipeline:
 *   TransportHandler -> FrameLengthParserHandler -> FrameLengthEncoderHandler
 *   -> RocketServerFrameCodecHandler -> RocketServerSetupFrameHandler
 *   -> RocketServerRequestResponseFrameHandler ->
 * RocketServerStreamStateHandler
 *   -> ThriftServerChannel
 *
 * Client pipeline:
 *   TransportHandler -> FrameLengthParserHandler -> FrameLengthEncoderHandler
 *   -> RocketClientFrameCodecHandler -> RocketClientSetupFrameHandler
 *   -> RocketClientRequestResponseFrameHandler -> RocketClientErrorFrameHandler
 *   -> RocketClientStreamStateHandler -> ThriftClientRocketInterfaceHandler
 *   -> ThriftClientMetadataHandler -> ThriftClientMetadataPushHandler
 *   -> ThriftClientRequestResponseHandler -> ThriftClientChannel
 */
class FastThriftE2ETest : public ::testing::Test {
 protected:
  struct ThriftConnectionContext {
    std::shared_ptr<thrift::ThriftServerChannel> serverChannel;
    std::unique_ptr<thrift::server::ThriftServerTransportAdapter>
        transportAdapter;
    PipelineImpl::Ptr thriftPipeline;
    std::unique_ptr<SimpleBufferAllocator> thriftAllocator =
        std::make_unique<SimpleBufferAllocator>();
    std::shared_ptr<folly::Baton<>> closedBaton =
        std::make_shared<folly::Baton<>>();
  };

  void SetUp() override {
    THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

    handler_ = std::make_shared<TestHandler>();
    executor_ = std::make_shared<folly::IOThreadPoolExecutor>(1);

    // Server connection factory

    // Server connection factory — called for each accepted connection
    apache::thrift::fast_thrift::rocket::server::connection::ConnectionFactory
        connectionFactory = [this](folly::AsyncSocket::UniquePtr socket)
        -> apache::thrift::fast_thrift::rocket::server::connection::
            RocketServerConnection {
              auto* evb = socket->getEventBase();
              auto transportHandler = apache::thrift::fast_thrift::transport::
                  TransportHandler::create(std::move(socket));

              // Build RocketServerConnection with default appAdapter
              apache::thrift::fast_thrift::rocket::server::connection::
                  RocketServerConnection conn;

              // 1. Build rocket pipeline
              auto rocketPipeline =
                  PipelineBuilder<
                      apache::thrift::fast_thrift::transport::TransportHandler,
                      apache::thrift::fast_thrift::rocket::server::
                          RocketServerAppAdapter,
                      SimpleBufferAllocator>()
                      .setEventBase(evb)
                      .setHead(transportHandler.get())
                      .setTail(conn.appAdapter.get())
                      .setAllocator(&serverRocketAllocator_)
                      .setHeadToTailOp(
                          apache::thrift::fast_thrift::channel_pipeline::
                              HeadToTailOp::Read)
                      .addNextDuplex<
                          apache::thrift::fast_thrift::rocket::server::handler::
                              RocketServerStreamStateHandler>(
                          rocket_server_stream_state_handler_tag)
                      .addNextDuplex<
                          apache::thrift::fast_thrift::rocket::server::handler::
                              RocketServerRequestResponseFrameHandler>(
                          rocket_server_request_response_frame_handler_tag)
                      .addNextDuplex<
                          apache::thrift::fast_thrift::rocket::server::handler::
                              RocketServerSetupFrameHandler>(
                          rocket_server_setup_frame_handler_tag)
                      .addNextDuplex<
                          apache::thrift::fast_thrift::rocket::server::handler::
                              RocketServerFrameCodecHandler>(
                          rocket_server_frame_codec_handler_tag)
                      .addNextOutbound<
                          apache::thrift::fast_thrift::frame::write::handler::
                              FrameLengthEncoderHandler>(
                          server_frame_length_encoder_handler_tag)
                      .addNextInbound<apache::thrift::fast_thrift::frame::read::
                                          handler::FrameLengthParserHandler>(
                          server_frame_length_parser_handler_tag)
                      .build();

              conn.appAdapter->setPipeline(rocketPipeline.get());
              transportHandler->setPipeline(*rocketPipeline);

              conn.transportHandler = std::move(transportHandler);
              conn.pipeline = std::move(rocketPipeline);

              // 2. Build thrift pipeline
              auto serverChannel =
                  std::make_shared<thrift::ThriftServerChannel>(handler_);
              auto transportAdapter = std::make_unique<
                  thrift::server::ThriftServerTransportAdapter>(
                  *conn.appAdapter);

              ThriftConnectionContext ctx;
              auto thriftPipeline =
                  PipelineBuilder<
                      thrift::server::ThriftServerTransportAdapter,
                      thrift::ThriftServerChannel,
                      SimpleBufferAllocator>()
                      .setEventBase(evb)
                      .setHead(transportAdapter.get())
                      .setTail(serverChannel.get())
                      .setAllocator(ctx.thriftAllocator.get())
                      .setHeadToTailOp(
                          apache::thrift::fast_thrift::channel_pipeline::
                              HeadToTailOp::Read)
                      .build();

              transportAdapter->setPipeline(thriftPipeline.get());
              serverChannel->setPipelineRef(*thriftPipeline);
              serverChannel->setWorker(
                  apache::thrift::Cpp2Worker::createDummy(evb));

              ctx.serverChannel = std::move(serverChannel);
              ctx.transportAdapter = std::move(transportAdapter);
              ctx.thriftPipeline = std::move(thriftPipeline);

              ctx.serverChannel->setCloseCallback(
                  [baton = ctx.closedBaton]() { baton->post(); });

              thriftConnections_.withWLock(
                  [&](auto& conns) { conns.push_back(std::move(ctx)); });

              return conn;
            };

    connectionManager_ = apache::thrift::fast_thrift::rocket::server::
        connection::ConnectionManager::create(
            folly::SocketAddress("::1", 0),
            folly::getKeepAliveToken(executor_.get()),
            std::move(connectionFactory));
    connectionManager_->start();

    clientThread_ = std::make_unique<folly::ScopedEventBaseThread>();
  }
  void TearDown() override {
    clientThread_->getEventBase()->runInEventBaseThreadAndWait([&] {
      clientPipeline_.reset();
      clientTransportHandler_.reset();
    });
    clientThread_.reset();
    // Wait for all server connections to close. ThriftServerChannel fires its
    // close callback after the EOF propagates through both pipelines and
    // ThriftServerTransportAdapter::onTransportError() completes. After this,
    // the IO thread will not access ThriftServerTransportAdapter again.
    {
      std::vector<std::shared_ptr<folly::Baton<>>> batons;
      thriftConnections_.withRLock([&](const auto& conns) {
        for (const auto& ctx : conns) {
          batons.push_back(ctx.closedBaton);
        }
      });
      for (auto& baton : batons) {
        baton->wait();
      }
    }
    connectionManager_->stop();
    // Clear thrift contexts here (before executor_.reset()) so Cpp2Worker's
    // IOWorkerContext is destroyed while the event base is alive.
    thriftConnections_.withWLock([](auto& conns) { conns.clear(); });
    connectionManager_.reset();
    executor_->join();
    executor_.reset();
  }

  /**
   * Create a fast_thrift client connected to the fast_thrift server.
   */
  std::unique_ptr<apache::thrift::Client<TestService>> createClient() {
    auto* evb = clientThread_->getEventBase();
    thrift::ThriftClientChannel::UniquePtr channel;
    folly::Baton<> connectBaton;
    bool connected = false;

    evb->runInEventBaseThreadAndWait([&] {
      auto socket = folly::AsyncSocket::newSocket(evb);
      auto* socketPtr = socket.get();

      clientTransportHandler_ =
          apache::thrift::fast_thrift::transport::TransportHandler::create(
              std::move(socket));
      channel = thrift::ThriftClientChannel::newChannel(evb);

      connectCallback_ = std::make_unique<ConnectCallback>(
          clientTransportHandler_.get(), connectBaton, connected);
      socketPtr->connect(
          connectCallback_.get(), connectionManager_->getAddress(), 30000);

      auto setupFactory = []() {
        apache::thrift::RequestSetupMetadata meta;
        meta.minVersion() = 8;
        meta.maxVersion() = 10;
        meta.clientMetadata().ensure().agent() = "fast_thrift_e2e_test";

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

      clientPipeline_ =
          PipelineBuilder<
              thrift::ThriftClientChannel,
              apache::thrift::fast_thrift::transport::TransportHandler,
              SimpleBufferAllocator>()
              .setEventBase(evb)
              .setTail(clientTransportHandler_.get())
              .setHead(channel.get())
              .setAllocator(&clientAllocator_)
              .addNextInbound<apache::thrift::fast_thrift::frame::read::
                                  handler::FrameLengthParserHandler>(
                  client_frame_length_parser_handler_tag)
              .addNextOutbound<apache::thrift::fast_thrift::frame::write::
                                   handler::FrameLengthEncoderHandler>(
                  client_frame_length_encoder_handler_tag)
              .addNextDuplex<apache::thrift::fast_thrift::rocket::client::
                                 handler::RocketClientFrameCodecHandler>(
                  rocket_client_frame_codec_handler_tag)
              .addNextDuplex<apache::thrift::fast_thrift::rocket::client::
                                 handler::RocketClientSetupFrameHandler>(
                  rocket_client_setup_handler_tag, std::move(setupFactory))
              .addNextDuplex<
                  apache::thrift::fast_thrift::rocket::client::handler::
                      RocketClientRequestResponseFrameHandler>(
                  rocket_client_request_response_frame_handler_tag)
              .addNextInbound<apache::thrift::fast_thrift::rocket::client::
                                  handler::RocketClientErrorFrameHandler>(
                  rocket_client_error_frame_handler_tag)
              .addNextDuplex<apache::thrift::fast_thrift::rocket::client::
                                 handler::RocketClientStreamStateHandler>(
                  rocket_client_stream_state_handler_tag)
              .addNextDuplex<
                  thrift::client::handler::ThriftClientRocketInterfaceHandler>(
                  thrift_client_rocket_interface_handler_tag)
              .addNextInbound<
                  thrift::client::handler::ThriftClientMetadataPushHandler>(
                  thrift_client_metadata_push_handler_tag)
              .build();

      channel->setPipeline(clientPipeline_.get());
      clientTransportHandler_->setPipeline(*clientPipeline_);
    });

    connectBaton.wait();

    if (!connected) {
      throw std::runtime_error("Failed to connect to server");
    }

    return std::make_unique<apache::thrift::Client<TestService>>(
        std::move(channel));
  }

  void destroyClientOnEvb(
      std::unique_ptr<apache::thrift::Client<TestService>>& client) {
    clientThread_->getEventBase()->runInEventBaseThreadAndWait(
        [&] { client.reset(); });
  }

  std::shared_ptr<TestHandler> handler_;
  std::shared_ptr<folly::IOThreadPoolExecutor> executor_;
  apache::thrift::fast_thrift::rocket::server::connection::ConnectionManager::
      Ptr connectionManager_;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
  SimpleBufferAllocator clientAllocator_;
  SimpleBufferAllocator serverRocketAllocator_;
  apache::thrift::fast_thrift::transport::TransportHandler::Ptr
      clientTransportHandler_;
  PipelineImpl::Ptr clientPipeline_;
  folly::Synchronized<std::vector<ThriftConnectionContext>> thriftConnections_;
  std::unique_ptr<ConnectCallback> connectCallback_;
};

// =============================================================================
// Test Cases
// =============================================================================

TEST_F(FastThriftE2ETest, Ping) {
  auto client = createClient();

  folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(), client->co_ping()));

  destroyClientOnEvb(client);
}

TEST_F(FastThriftE2ETest, EchoRequestResponse) {
  auto client = createClient();

  auto result = folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(), client->co_echo("hello world")));
  EXPECT_EQ(result, "hello world");

  destroyClientOnEvb(client);
}

TEST_F(FastThriftE2ETest, SequentialRequests) {
  auto client = createClient();
  auto* evb = clientThread_->getEventBase();

  // First request — completes before the second is sent
  auto result1 = folly::coro::blockingWait(
      folly::coro::co_withExecutor(evb, client->co_echo("first")));
  EXPECT_EQ(result1, "first");

  // Second request — sent after the first completes
  auto result2 = folly::coro::blockingWait(
      folly::coro::co_withExecutor(evb, client->co_echo("second")));
  EXPECT_EQ(result2, "second");

  // Third request — validates stream IDs continue advancing
  auto result3 = folly::coro::blockingWait(
      folly::coro::co_withExecutor(evb, client->co_echo("third")));
  EXPECT_EQ(result3, "third");

  destroyClientOnEvb(client);
}

TEST_F(FastThriftE2ETest, MultipleRequests) {
  auto client = createClient();
  auto* evb = clientThread_->getEventBase();

  auto [r1, r2, r3] = folly::coro::blockingWait(
      folly::coro::collectAll(
          folly::coro::co_withExecutor(evb, client->co_echo("first")),
          folly::coro::co_withExecutor(evb, client->co_echo("second")),
          folly::coro::co_withExecutor(evb, client->co_echo("third"))));

  EXPECT_EQ(r1, "first");
  EXPECT_EQ(r2, "second");
  EXPECT_EQ(r3, "third");

  destroyClientOnEvb(client);
}

TEST_F(FastThriftE2ETest, LargeResponse) {
  auto client = createClient();

  constexpr int64_t kResponseSize = 10000;

  auto result = folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(),
          client->co_sendResponse(kResponseSize)));
  EXPECT_EQ(result, std::string(kResponseSize, 'x'));

  destroyClientOnEvb(client);
}

// =============================================================================
// FastThriftFastClientE2ETest
// FastThriftFastClientE2ETest - Generated FastClient with fast_thrift server
// =============================================================================

class FastThriftFastClientE2ETest : public ::testing::Test {
 protected:
  using FastClientType = apache::thrift::
      FastClient<TestFastService, thrift::ThriftClientAppAdapter>;

  struct ThriftConnectionContext {
    std::shared_ptr<thrift::ThriftServerChannel> serverChannel;
    std::unique_ptr<thrift::server::ThriftServerTransportAdapter>
        transportAdapter;
    PipelineImpl::Ptr thriftPipeline;
    std::unique_ptr<SimpleBufferAllocator> thriftAllocator =
        std::make_unique<SimpleBufferAllocator>();
    std::shared_ptr<folly::Baton<>> closedBaton =
        std::make_shared<folly::Baton<>>();
  };

  void SetUp() override {
    THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

    handler_ = std::make_shared<TestHandler>();
    executor_ = std::make_shared<folly::IOThreadPoolExecutor>(1);

    rocket::server::connection::ConnectionFactory connectionFactory =
        [this](folly::AsyncSocket::UniquePtr socket)
        -> rocket::server::connection::RocketServerConnection {
      auto* evb = socket->getEventBase();
      auto transportHandler =
          transport::TransportHandler::create(std::move(socket));

      // Build RocketServerConnection with default appAdapter
      rocket::server::connection::RocketServerConnection conn;

      // 1. Build rocket pipeline
      auto rocketPipeline =
          PipelineBuilder<
              transport::TransportHandler,
              rocket::server::RocketServerAppAdapter,
              SimpleBufferAllocator>()
              .setEventBase(evb)
              .setHead(transportHandler.get())
              .setTail(conn.appAdapter.get())
              .setAllocator(&serverRocketAllocator_)
              .setHeadToTailOp(channel_pipeline::HeadToTailOp::Read)
              .addNextDuplex<
                  rocket::server::handler::RocketServerStreamStateHandler>(
                  rocket_server_stream_state_handler_tag)
              .addNextDuplex<rocket::server::handler::
                                 RocketServerRequestResponseFrameHandler>(
                  rocket_server_request_response_frame_handler_tag)
              .addNextDuplex<
                  rocket::server::handler::RocketServerSetupFrameHandler>(
                  rocket_server_setup_frame_handler_tag)
              .addNextDuplex<
                  rocket::server::handler::RocketServerFrameCodecHandler>(
                  rocket_server_frame_codec_handler_tag)
              .addNextOutbound<
                  frame::write::handler::FrameLengthEncoderHandler>(
                  server_frame_length_encoder_handler_tag)
              .addNextInbound<frame::read::handler::FrameLengthParserHandler>(
                  server_frame_length_parser_handler_tag)
              .build();

      conn.appAdapter->setPipeline(rocketPipeline.get());
      transportHandler->setPipeline(*rocketPipeline);

      conn.transportHandler = std::move(transportHandler);
      conn.pipeline = std::move(rocketPipeline);

      // 2. Build thrift pipeline
      auto serverChannel =
          std::make_shared<thrift::ThriftServerChannel>(handler_);
      auto transportAdapter =
          std::make_unique<thrift::server::ThriftServerTransportAdapter>(
              *conn.appAdapter);

      ThriftConnectionContext ctx;
      auto thriftPipeline =
          PipelineBuilder<
              thrift::server::ThriftServerTransportAdapter,
              thrift::ThriftServerChannel,
              SimpleBufferAllocator>()
              .setEventBase(evb)
              .setHead(transportAdapter.get())
              .setTail(serverChannel.get())
              .setAllocator(ctx.thriftAllocator.get())
              .setHeadToTailOp(channel_pipeline::HeadToTailOp::Read)
              .build();

      transportAdapter->setPipeline(thriftPipeline.get());
      serverChannel->setPipelineRef(*thriftPipeline);
      serverChannel->setWorker(apache::thrift::Cpp2Worker::createDummy(evb));

      ctx.serverChannel = std::move(serverChannel);
      ctx.transportAdapter = std::move(transportAdapter);
      ctx.thriftPipeline = std::move(thriftPipeline);

      ctx.serverChannel->setCloseCallback(
          [baton = ctx.closedBaton]() { baton->post(); });

      thriftConnections_.withWLock(
          [&](auto& conns) { conns.push_back(std::move(ctx)); });

      return conn;
    };

    connectionManager_ = rocket::server::connection::ConnectionManager::create(
        folly::SocketAddress("::1", 0),
        folly::getKeepAliveToken(executor_.get()),
        std::move(connectionFactory));
    connectionManager_->start();

    clientThread_ = std::make_unique<folly::ScopedEventBaseThread>();
  }

  void TearDown() override {
    clientThread_->getEventBase()->runInEventBaseThreadAndWait([&] {
      clientPipeline_.reset();
      clientTransportHandler_.reset();
    });
    clientThread_.reset();
    // Wait for all server connections to close. ThriftServerChannel fires its
    // close callback after the EOF propagates through both pipelines and
    // ThriftServerTransportAdapter::onTransportError() completes. After this,
    // the IO thread will not access ThriftServerTransportAdapter again.
    {
      std::vector<std::shared_ptr<folly::Baton<>>> batons;
      thriftConnections_.withRLock([&](const auto& conns) {
        for (const auto& ctx : conns) {
          batons.push_back(ctx.closedBaton);
        }
      });
      for (auto& baton : batons) {
        baton->wait();
      }
    }
    connectionManager_->stop();
    // Clear thrift contexts here (before executor_.reset()) so Cpp2Worker's
    // IOWorkerContext is destroyed while the event base is alive.
    thriftConnections_.withWLock([](auto& conns) { conns.clear(); });
    connectionManager_.reset();
    executor_->join();
    executor_.reset();
  }

  std::unique_ptr<FastClientType> createFastClient() {
    auto* evb = clientThread_->getEventBase();
    folly::Baton<> connectBaton;
    bool connected = false;

    thrift::ThriftClientAppAdapter::Ptr appAdapter(
        new thrift::ThriftClientAppAdapter(
            static_cast<uint16_t>(
                apache::thrift::protocol::T_COMPACT_PROTOCOL)));

    evb->runInEventBaseThreadAndWait([&] {
      auto socket = folly::AsyncSocket::newSocket(evb);
      auto* socketPtr = socket.get();

      clientTransportHandler_ =
          transport::TransportHandler::create(std::move(socket));

      connectCallback_ = std::make_unique<ConnectCallback>(
          clientTransportHandler_.get(), connectBaton, connected);
      socketPtr->connect(
          connectCallback_.get(), connectionManager_->getAddress(), 30000);

      auto setupFactory = []() {
        apache::thrift::RequestSetupMetadata meta;
        meta.minVersion() = 8;
        meta.maxVersion() = 10;
        meta.clientMetadata().ensure().agent() =
            "fast_thrift_fast_client_e2e_test";

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

      clientPipeline_ =
          PipelineBuilder<
              thrift::ThriftClientAppAdapter,
              transport::TransportHandler,
              SimpleBufferAllocator>()
              .setEventBase(evb)
              .setTail(clientTransportHandler_.get())
              .setHead(appAdapter.get())
              .setAllocator(&clientAllocator_)
              .addNextInbound<frame::read::handler::FrameLengthParserHandler>(
                  client_frame_length_parser_handler_tag)
              .addNextOutbound<
                  frame::write::handler::FrameLengthEncoderHandler>(
                  client_frame_length_encoder_handler_tag)
              .addNextDuplex<
                  rocket::client::handler::RocketClientFrameCodecHandler>(
                  rocket_client_frame_codec_handler_tag)
              .addNextDuplex<
                  rocket::client::handler::RocketClientSetupFrameHandler>(
                  rocket_client_setup_handler_tag, std::move(setupFactory))
              .addNextDuplex<rocket::client::handler::
                                 RocketClientRequestResponseFrameHandler>(
                  rocket_client_request_response_frame_handler_tag)
              .addNextInbound<
                  rocket::client::handler::RocketClientErrorFrameHandler>(
                  rocket_client_error_frame_handler_tag)
              .addNextDuplex<
                  rocket::client::handler::RocketClientStreamStateHandler>(
                  rocket_client_stream_state_handler_tag)
              .addNextDuplex<
                  thrift::client::handler::ThriftClientRocketInterfaceHandler>(
                  thrift_client_rocket_interface_handler_tag)
              .addNextInbound<
                  thrift::client::handler::ThriftClientMetadataPushHandler>(
                  thrift_client_metadata_push_handler_tag)
              .build();

      clientTransportHandler_->setPipeline(*clientPipeline_);
      appAdapter->setPipeline(clientPipeline_.get());
    });

    connectBaton.wait();
    if (!connected) {
      throw std::runtime_error("Failed to connect to server");
    }

    return std::make_unique<FastClientType>(std::move(appAdapter));
  }

  std::shared_ptr<TestHandler> handler_;
  std::shared_ptr<folly::IOThreadPoolExecutor> executor_;
  rocket::server::connection::ConnectionManager::Ptr connectionManager_;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
  SimpleBufferAllocator clientAllocator_;
  SimpleBufferAllocator serverRocketAllocator_;
  transport::TransportHandler::Ptr clientTransportHandler_;
  PipelineImpl::Ptr clientPipeline_;
  folly::Synchronized<std::vector<ThriftConnectionContext>> thriftConnections_;
  std::unique_ptr<ConnectCallback> connectCallback_;
};

TEST_F(FastThriftFastClientE2ETest, Ping) {
  auto client = createFastClient();
  auto* evb = clientThread_->getEventBase();

  // coro
  folly::coro::blockingWait(
      folly::coro::co_withExecutor(evb, client->co_ping()));

  // sync
  client->sync_ping();

  // callback
  folly::Promise<folly::Unit> cbPromise;
  auto cbFuture = cbPromise.getSemiFuture();
  client->ping(
      std::make_unique<apache::thrift::FunctionReplyCallback>(
          [&cbPromise](apache::thrift::ClientReceiveState&& state) {
            auto ew = FastClientType::recv_wrapped_ping(state);
            if (ew) {
              cbPromise.setException(std::move(ew));
            } else {
              cbPromise.setValue(folly::Unit{});
            }
          }));
  std::move(cbFuture).get();
}

TEST_F(FastThriftFastClientE2ETest, Echo) {
  auto client = createFastClient();
  auto* evb = clientThread_->getEventBase();

  // coro
  auto coroResult = folly::coro::blockingWait(
      folly::coro::co_withExecutor(evb, client->co_echo("hello coro")));
  EXPECT_EQ(coroResult, "hello coro");

  // sync
  std::string syncResult;
  client->sync_echo(syncResult, "hello sync");
  EXPECT_EQ(syncResult, "hello sync");

  // callback
  folly::Promise<std::string> cbPromise;
  auto cbFuture = cbPromise.getSemiFuture();
  client->echo(
      std::make_unique<apache::thrift::FunctionReplyCallback>(
          [&cbPromise](apache::thrift::ClientReceiveState&& state) {
            try {
              cbPromise.setValue(FastClientType::recv_echo(state));
            } catch (...) {
              cbPromise.setException(
                  folly::exception_wrapper(std::current_exception()));
            }
          }),
      "hello callback");
  EXPECT_EQ(std::move(cbFuture).get(), "hello callback");
}

TEST_F(FastThriftFastClientE2ETest, Add) {
  auto client = createFastClient();
  auto* evb = clientThread_->getEventBase();

  // coro
  auto coroResult = folly::coro::blockingWait(
      folly::coro::co_withExecutor(evb, client->co_add(17, 25)));
  EXPECT_EQ(coroResult, 42);

  // sync
  EXPECT_EQ(client->sync_add(100, 200), 300);

  // callback
  folly::Promise<int64_t> cbPromise;
  auto cbFuture = cbPromise.getSemiFuture();
  client->add(
      std::make_unique<apache::thrift::FunctionReplyCallback>(
          [&cbPromise](apache::thrift::ClientReceiveState&& state) {
            try {
              cbPromise.setValue(FastClientType::recv_add(state));
            } catch (...) {
              cbPromise.setException(
                  folly::exception_wrapper(std::current_exception()));
            }
          }),
      3,
      7);
  EXPECT_EQ(std::move(cbFuture).get(), 10);
}

TEST_F(FastThriftFastClientE2ETest, SendResponse) {
  auto client = createFastClient();
  auto* evb = clientThread_->getEventBase();
  constexpr int64_t kResponseSize = 10000;

  // coro
  auto coroResult = folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          evb, client->co_sendResponse(kResponseSize)));
  EXPECT_EQ(coroResult.size(), kResponseSize);
  EXPECT_EQ(coroResult, std::string(kResponseSize, 'x'));

  // sync
  std::string syncResult;
  client->sync_sendResponse(syncResult, kResponseSize);
  EXPECT_EQ(syncResult.size(), kResponseSize);
  EXPECT_EQ(syncResult, std::string(kResponseSize, 'x'));

  // callback
  folly::Promise<std::string> cbPromise;
  auto cbFuture = cbPromise.getSemiFuture();
  client->sendResponse(
      std::make_unique<apache::thrift::FunctionReplyCallback>(
          [&cbPromise](apache::thrift::ClientReceiveState&& state) {
            try {
              cbPromise.setValue(FastClientType::recv_sendResponse(state));
            } catch (...) {
              cbPromise.setException(
                  folly::exception_wrapper(std::current_exception()));
            }
          }),
      kResponseSize);
  auto cbResult = std::move(cbFuture).get();
  EXPECT_EQ(cbResult.size(), kResponseSize);
  EXPECT_EQ(cbResult, std::string(kResponseSize, 'x'));
}

} // namespace apache::thrift::fast_thrift::thrift::test
