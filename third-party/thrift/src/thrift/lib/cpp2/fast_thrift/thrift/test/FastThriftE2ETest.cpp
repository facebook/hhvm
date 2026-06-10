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
#include <thrift/lib/cpp2/fast_thrift/connection/ConnectionHandler.h>
#include <thrift/lib/cpp2/fast_thrift/connection/ConnectionManager.h>
#include <thrift/lib/cpp2/fast_thrift/frame/handler/FrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameDefragmentationHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/FragmentationHandlerConfig.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameFragmentationHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/adapter/RocketClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/common/RocketClientConnection.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientConnectionErrorHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientRequestResponseHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerMessageMarshalHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientChannel.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/adapter/ThriftClientTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientChecksumHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientMetadataPushHandler.h>
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
HANDLER_TAG(rocket_client_request_response_handler);
HANDLER_TAG(rocket_client_connection_error_handler);
HANDLER_TAG(rocket_client_stream_state_handler);
HANDLER_TAG(thrift_client_metadata_push_handler);
HANDLER_TAG(thrift_client_checksum_handler);

// Server handler tags
HANDLER_TAG(server_frame_length_parser_handler);
HANDLER_TAG(server_frame_length_encoder_handler);
HANDLER_TAG(server_frame_codec_handler);
HANDLER_TAG(server_frame_defragmentation_handler);
HANDLER_TAG(server_frame_fragmentation_handler);
HANDLER_TAG(rocket_server_message_marshal_handler);
HANDLER_TAG(rocket_server_setup_frame_handler);
HANDLER_TAG(rocket_server_request_response_handler);
HANDLER_TAG(rocket_server_stream_state_handler);

/**
 * ConnectCallback - Triggers transportHandler->onConnect() when the
 * TCP connection is established.
 */
class ConnectCallback : public folly::AsyncSocket::ConnectCallback {
 public:
  explicit ConnectCallback(
      apache::thrift::fast_thrift::rocket::client::RocketClientConnection::
          TransportHandler* transportHandler,
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
  apache::thrift::fast_thrift::rocket::client::RocketClientConnection::
      TransportHandler* transportHandler_;
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
 * Server (two-pipeline):
 *   Rocket pipeline: TransportHandler -> ... -> RocketServerAppAdapter
 *   Thrift pipeline: ThriftServerTransportAdapter -> ThriftServerChannel
 *
 * Client (two-pipeline):
 *   Rocket pipeline: RocketClientAppAdapter -> ... -> TransportHandler
 *   Thrift pipeline: ThriftClientChannel -> ThriftClientMetadataPushHandler
 *     -> ThriftClientTransportAdapter
 */
class FastThriftE2ETest : public ::testing::Test {
 protected:
  // Per-accepted-client server-side state. Owns the thrift pipeline (which
  // owns the rocket connection via the transport adapter). Satisfies the
  // connection::Connection concept.
  struct ServerConnection {
    std::shared_ptr<thrift::ThriftServerChannel> serverChannel;
    SimpleBufferAllocator thriftAllocator;
    std::unique_ptr<thrift::server::ThriftServerTransportAdapter>
        transportAdapter;
    PipelineImpl::Ptr thriftPipeline;
    std::function<void()> closeCb;
    bool closed{false};

    ServerConnection() = default;
    ServerConnection(ServerConnection&&) noexcept = default;
    ServerConnection& operator=(ServerConnection&&) noexcept = default;
    ServerConnection(const ServerConnection&) = delete;
    ServerConnection& operator=(const ServerConnection&) = delete;

    void start() noexcept {
      transportAdapter->rocketConnection().transportHandler->onConnect();
    }
    void close() noexcept {
      if (closed) {
        return;
      }
      closed = true;
      if (thriftPipeline) {
        thriftPipeline->close();
        thriftPipeline.reset();
      }
      transportAdapter.reset();
      if (closeCb) {
        auto cb = std::move(closeCb);
        cb();
      }
    }
    void drain() noexcept { close(); }
    void setCloseCallback(std::function<void()> cb) { closeCb = std::move(cb); }
  };

  // connection::ConnectionFactory: builds a ServerConnection per accept.
  class ServerConnectionFactory {
   public:
    explicit ServerConnectionFactory(FastThriftE2ETest* fixture) noexcept
        : fixture_(fixture) {}
    ServerConnection getConnection(folly::AsyncTransport::UniquePtr socket) {
      return fixture_->buildServerConnection(std::move(socket));
    }

   private:
    FastThriftE2ETest* fixture_;
  };

  ServerConnection buildServerConnection(
      folly::AsyncTransport::UniquePtr socket) {
    auto* evb = socket->getEventBase();

    // Build rocket-layer pieces inside a rocket::server::RocketServerConnection
    // so ThriftServerTransportAdapter can take ownership of the whole bundle.
    auto rocketConn = std::make_unique<
        apache::thrift::fast_thrift::rocket::server::RocketServerConnection>();
    rocketConn->transportHandler =
        apache::thrift::fast_thrift::transport::TransportHandler::create(
            std::move(socket));

    rocketConn->pipeline =
        PipelineBuilder<
            apache::thrift::fast_thrift::transport::TransportHandler,
            apache::thrift::fast_thrift::rocket::server::RocketServerAppAdapter,
            SimpleBufferAllocator>()
            .setEventBase(evb)
            .setHead(rocketConn->transportHandler.get())
            .setTail(rocketConn->appAdapter.get())
            .setAllocator(&serverRocketAllocator_)
            .addNextInbound<apache::thrift::fast_thrift::frame::read::handler::
                                FrameLengthParserHandler>(
                server_frame_length_parser_handler_tag)
            .addNextOutbound<apache::thrift::fast_thrift::frame::write::
                                 handler::FrameLengthEncoderHandler>(
                server_frame_length_encoder_handler_tag)
            .addNextDuplex<
                apache::thrift::fast_thrift::frame::handler::FrameCodecHandler>(
                server_frame_codec_handler_tag)
            .addNextInbound<apache::thrift::fast_thrift::frame::read::handler::
                                FrameDefragmentationHandler>(
                server_frame_defragmentation_handler_tag)
            .addNextOutbound<apache::thrift::fast_thrift::frame::write::
                                 handler::FrameFragmentationHandler>(
                server_frame_fragmentation_handler_tag,
                apache::thrift::fast_thrift::frame::write::
                    FragmentationHandlerConfig{})
            .addNextDuplex<apache::thrift::fast_thrift::rocket::server::
                               handler::RocketServerMessageMarshalHandler>(
                rocket_server_message_marshal_handler_tag)
            .addNextDuplex<apache::thrift::fast_thrift::rocket::server::
                               handler::RocketServerSetupFrameHandler>(
                rocket_server_setup_frame_handler_tag)
            .addNextDuplex<apache::thrift::fast_thrift::rocket::server::
                               handler::RocketServerStreamStateHandler>(
                rocket_server_stream_state_handler_tag)
            .addNextDuplex<apache::thrift::fast_thrift::rocket::server::
                               handler::RocketServerRequestResponseHandler>(
                rocket_server_request_response_handler_tag)
            .build();
    rocketConn->appAdapter->setPipeline(rocketConn->pipeline.get());
    rocketConn->transportHandler->setPipeline(rocketConn->pipeline.get());

    auto serverChannel =
        std::make_shared<thrift::ThriftServerChannel>(handler_);
    auto transportAdapter =
        std::make_unique<thrift::server::ThriftServerTransportAdapter>(
            std::move(rocketConn));

    ServerConnection conn;
    conn.serverChannel = serverChannel;
    conn.thriftPipeline = PipelineBuilder<
                              thrift::server::ThriftServerTransportAdapter,
                              thrift::ThriftServerChannel,
                              SimpleBufferAllocator>()
                              .setEventBase(evb)
                              .setHead(transportAdapter.get())
                              .setTail(serverChannel.get())
                              .setAllocator(&conn.thriftAllocator)
                              .build();

    transportAdapter->setPipeline(conn.thriftPipeline.get());
    serverChannel->setPipelineRef(*conn.thriftPipeline);
    serverChannel->setWorker(apache::thrift::Cpp2Worker::createDummy(evb));
    conn.transportAdapter = std::move(transportAdapter);
    // Connection is inert; ConnectionHandler's installer lambda calls
    // start() after registering the entry, which fires onConnect().
    return conn;
  }

  void SetUp() override {
    THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

    handler_ = std::make_shared<TestHandler>();
    executor_ = std::make_shared<folly::IOThreadPoolExecutor>(1);

    connectionManager_ =
        apache::thrift::fast_thrift::connection::ConnectionManager::create(
            folly::SocketAddress("::1", 0),
            folly::getKeepAliveToken(executor_.get()),
            apache::thrift::fast_thrift::security::SSLPolicy::DISABLED,
            /*tlsParams=*/nullptr,
            apache::thrift::fast_thrift::connection::SocketOptions{});
    connectionManager_->setConnectionFactory(ServerConnectionFactory{this});
    connectionManager_->start();

    clientThread_ = std::make_unique<folly::ScopedEventBaseThread>();
  }

  void TearDown() override {
    clientThread_->getEventBase()->runInEventBaseThreadAndWait([&] {
      clientPipeline_.reset();
      clientTransportAdapter_.reset();
    });
    clientThread_.reset();
    // ConnectionManager::stop() drains all in-flight server connections
    // before returning, then force-closes any stragglers. After this returns,
    // the IO threads will not touch our connections again.
    connectionManager_->stop();
    connectionManager_.reset();
    executor_->join();
    executor_.reset();
  }

  /**
   * Create a fast_thrift client connected to the fast_thrift server.
   *
   * Uses two-pipeline architecture:
   *   Rocket pipeline: RocketClientAppAdapter → [rocket handlers] →
   *     TransportHandler
   *   Thrift pipeline: ThriftClientChannel → ThriftClientMetadataPushHandler →
   *     ThriftClientTransportAdapter
   */
  std::unique_ptr<apache::thrift::Client<TestService>> createClient() {
    auto* evb = clientThread_->getEventBase();
    thrift::ThriftClientChannel::UniquePtr channel;
    folly::Baton<> connectBaton;
    bool connected = false;

    evb->runInEventBaseThreadAndWait([&] {
      auto socket = folly::AsyncSocket::newSocket(evb);
      auto* socketPtr = socket.get();

      // 1. Build rocket pipeline inside RocketClientConnection
      auto connection =
          std::make_unique<rocket::client::RocketClientConnection>();

      connection->transportHandler =
          apache::thrift::fast_thrift::rocket::client::RocketClientConnection::
              TransportHandler::create(std::move(socket));

      auto* transportHandlerPtr = connection->transportHandler.get();

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

      connection->pipeline =
          PipelineBuilder<
              apache::thrift::fast_thrift::rocket::client::
                  RocketClientConnection::TransportHandler,
              apache::thrift::fast_thrift::rocket::client::
                  RocketClientAppAdapter,
              SimpleBufferAllocator>()
              .setEventBase(evb)
              .setHead(connection->transportHandler.get())
              .setTail(connection->appAdapter.get())
              .setAllocator(&connection->allocator)
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
              .addNextInbound<apache::thrift::fast_thrift::rocket::client::
                                  handler::RocketClientConnectionErrorHandler>(
                  rocket_client_connection_error_handler_tag)
              .addNextDuplex<apache::thrift::fast_thrift::rocket::client::
                                 handler::RocketClientStreamStateHandler>(
                  rocket_client_stream_state_handler_tag)
              .addNextInbound<apache::thrift::fast_thrift::rocket::client::
                                  handler::RocketClientRequestResponseHandler>(
                  rocket_client_request_response_handler_tag)
              .build();

      connection->appAdapter->setPipeline(connection->pipeline.get());
      connection->transportHandler->setPipeline(connection->pipeline.get());

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
              .setAllocator(&clientAllocator_)
              .addNextInbound<
                  thrift::client::handler::ThriftClientMetadataPushHandler>(
                  thrift_client_metadata_push_handler_tag)
              .addNextOutbound<
                  thrift::client::handler::ThriftClientChecksumHandler>(
                  thrift_client_checksum_handler_tag)
              .build();

      channel->setPipeline(clientPipeline_.get());
      clientTransportAdapter_->setPipeline(clientPipeline_.get());

      connectCallback_ = std::make_unique<ConnectCallback>(
          transportHandlerPtr, connectBaton, connected);
      socketPtr->connect(
          connectCallback_.get(), connectionManager_->getAddress(), 30000);
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
    clientThread_->getEventBase()->runInEventBaseThreadAndWait([&] {
      if (clientPipeline_) {
        clientPipeline_->deactivate();
        clientPipeline_->close();
      }
      if (clientTransportAdapter_) {
        clientTransportAdapter_->resetPipeline();
      }
      client.reset();
    });
  }

  std::shared_ptr<TestHandler> handler_;
  std::shared_ptr<folly::IOThreadPoolExecutor> executor_;
  apache::thrift::fast_thrift::connection::ConnectionManager::Ptr
      connectionManager_;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
  SimpleBufferAllocator clientAllocator_;
  SimpleBufferAllocator serverRocketAllocator_;
  std::unique_ptr<thrift::client::ThriftClientTransportAdapter>
      clientTransportAdapter_;
  PipelineImpl::Ptr clientPipeline_;
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

  // Per-accepted-client server-side state for this fixture's FastClient
  // path. Owns the thrift pipeline (which owns the rocket connection via
  // the transport adapter). Satisfies connection::Connection.
  struct ServerConnection {
    std::shared_ptr<thrift::ThriftServerChannel> serverChannel;
    SimpleBufferAllocator thriftAllocator;
    std::unique_ptr<thrift::server::ThriftServerTransportAdapter>
        transportAdapter;
    PipelineImpl::Ptr thriftPipeline;
    std::function<void()> closeCb;
    bool closed{false};

    ServerConnection() = default;
    ServerConnection(ServerConnection&&) noexcept = default;
    ServerConnection& operator=(ServerConnection&&) noexcept = default;
    ServerConnection(const ServerConnection&) = delete;
    ServerConnection& operator=(const ServerConnection&) = delete;

    void start() noexcept {
      transportAdapter->rocketConnection().transportHandler->onConnect();
    }
    void close() noexcept {
      if (closed) {
        return;
      }
      closed = true;
      if (thriftPipeline) {
        thriftPipeline->close();
        thriftPipeline.reset();
      }
      transportAdapter.reset();
      if (closeCb) {
        auto cb = std::move(closeCb);
        cb();
      }
    }
    void drain() noexcept { close(); }
    void setCloseCallback(std::function<void()> cb) { closeCb = std::move(cb); }
  };

  class ServerConnectionFactory;

  ServerConnection buildServerConnection(
      folly::AsyncTransport::UniquePtr socket) {
    auto* evb = socket->getEventBase();

    auto rocketConn =
        std::make_unique<rocket::server::RocketServerConnection>();
    rocketConn->transportHandler =
        transport::TransportHandler::create(std::move(socket));

    rocketConn->pipeline =
        PipelineBuilder<
            transport::TransportHandler,
            rocket::server::RocketServerAppAdapter,
            SimpleBufferAllocator>()
            .setEventBase(evb)
            .setHead(rocketConn->transportHandler.get())
            .setTail(rocketConn->appAdapter.get())
            .setAllocator(&serverRocketAllocator_)
            .addNextInbound<frame::read::handler::FrameLengthParserHandler>(
                server_frame_length_parser_handler_tag)
            .addNextOutbound<frame::write::handler::FrameLengthEncoderHandler>(
                server_frame_length_encoder_handler_tag)
            .addNextDuplex<frame::handler::FrameCodecHandler>(
                server_frame_codec_handler_tag)
            .addNextInbound<frame::read::handler::FrameDefragmentationHandler>(
                server_frame_defragmentation_handler_tag)
            .addNextOutbound<frame::write::handler::FrameFragmentationHandler>(
                server_frame_fragmentation_handler_tag,
                frame::write::FragmentationHandlerConfig{})
            .addNextDuplex<
                rocket::server::handler::RocketServerMessageMarshalHandler>(
                rocket_server_message_marshal_handler_tag)
            .addNextDuplex<
                rocket::server::handler::RocketServerSetupFrameHandler>(
                rocket_server_setup_frame_handler_tag)
            .addNextDuplex<
                rocket::server::handler::RocketServerStreamStateHandler>(
                rocket_server_stream_state_handler_tag)
            .addNextDuplex<
                rocket::server::handler::RocketServerRequestResponseHandler>(
                rocket_server_request_response_handler_tag)
            .build();
    rocketConn->appAdapter->setPipeline(rocketConn->pipeline.get());
    rocketConn->transportHandler->setPipeline(rocketConn->pipeline.get());

    auto serverChannel =
        std::make_shared<thrift::ThriftServerChannel>(handler_);
    auto transportAdapter =
        std::make_unique<thrift::server::ThriftServerTransportAdapter>(
            std::move(rocketConn));

    ServerConnection conn;
    conn.serverChannel = serverChannel;
    conn.thriftPipeline = PipelineBuilder<
                              thrift::server::ThriftServerTransportAdapter,
                              thrift::ThriftServerChannel,
                              SimpleBufferAllocator>()
                              .setEventBase(evb)
                              .setHead(transportAdapter.get())
                              .setTail(serverChannel.get())
                              .setAllocator(&conn.thriftAllocator)
                              .build();

    transportAdapter->setPipeline(conn.thriftPipeline.get());
    serverChannel->setPipelineRef(*conn.thriftPipeline);
    serverChannel->setWorker(apache::thrift::Cpp2Worker::createDummy(evb));
    conn.transportAdapter = std::move(transportAdapter);
    // Connection is inert; ConnectionHandler's installer lambda calls
    // start() after registering the entry, which fires onConnect().
    return conn;
  }

  void SetUp() override {
    THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

    handler_ = std::make_shared<TestHandler>();
    executor_ = std::make_shared<folly::IOThreadPoolExecutor>(1);

    connectionManager_ = connection::ConnectionManager::create(
        folly::SocketAddress("::1", 0),
        folly::getKeepAliveToken(executor_.get()),
        security::SSLPolicy::DISABLED,
        /*tlsParams=*/nullptr,
        connection::SocketOptions{});
    // Factory wraps a back-pointer to the fixture; defined out-of-line
    // below so it can refer to buildServerConnection.
    connectionManager_->setConnectionFactory(ServerConnectionFactory{this});
    connectionManager_->start();

    clientThread_ = std::make_unique<folly::ScopedEventBaseThread>();
  }

  void TearDown() override {
    clientThread_->getEventBase()->runInEventBaseThreadAndWait([&] {
      clientPipeline_.reset();
      clientTransportAdapter_.reset();
    });
    clientThread_.reset();
    connectionManager_->stop();
    connectionManager_.reset();
    executor_->join();
    executor_.reset();
  }

  class ServerConnectionFactory {
   public:
    explicit ServerConnectionFactory(FastThriftFastClientE2ETest* fixture)
        : fixture_(fixture) {}
    ServerConnection getConnection(folly::AsyncTransport::UniquePtr socket) {
      return fixture_->buildServerConnection(std::move(socket));
    }

   private:
    FastThriftFastClientE2ETest* fixture_;
  };

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

      // 1. Build rocket pipeline inside RocketClientConnection
      auto connection =
          std::make_unique<rocket::client::RocketClientConnection>();

      connection->transportHandler =
          rocket::client::RocketClientConnection::TransportHandler::create(
              std::move(socket));

      auto* transportHandlerPtr = connection->transportHandler.get();

      connectCallback_ = std::make_unique<ConnectCallback>(
          transportHandlerPtr, connectBaton, connected);
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

      connection->pipeline =
          PipelineBuilder<
              rocket::client::RocketClientConnection::TransportHandler,
              rocket::client::RocketClientAppAdapter,
              SimpleBufferAllocator>()
              .setEventBase(evb)
              .setHead(connection->transportHandler.get())
              .setTail(connection->appAdapter.get())
              .setAllocator(&connection->allocator)
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

      // 2. Build thrift pipeline: AppAdapter → TransportAdapter
      clientTransportAdapter_ =
          std::make_unique<thrift::client::ThriftClientTransportAdapter>(
              std::move(connection));

      clientPipeline_ =
          PipelineBuilder<
              thrift::client::ThriftClientTransportAdapter,
              thrift::ThriftClientAppAdapter,
              SimpleBufferAllocator>()
              .setEventBase(evb)
              .setHead(clientTransportAdapter_.get())
              .setTail(appAdapter.get())
              .setAllocator(&clientAllocator_)
              .addNextInbound<
                  thrift::client::handler::ThriftClientMetadataPushHandler>(
                  thrift_client_metadata_push_handler_tag)
              .addNextOutbound<
                  thrift::client::handler::ThriftClientChecksumHandler>(
                  thrift_client_checksum_handler_tag)
              .build();

      appAdapter->setPipeline(clientPipeline_.get());
      clientTransportAdapter_->setPipeline(clientPipeline_.get());
    });

    connectBaton.wait();
    if (!connected) {
      throw std::runtime_error("Failed to connect to server");
    }

    return std::make_unique<FastClientType>(std::move(appAdapter));
  }

  void destroyFastClientOnEvb(std::unique_ptr<FastClientType>& client) {
    clientThread_->getEventBase()->runInEventBaseThreadAndWait([&] {
      if (clientPipeline_) {
        clientPipeline_->deactivate();
        clientPipeline_->close();
      }
      if (clientTransportAdapter_) {
        clientTransportAdapter_->resetPipeline();
      }
      client.reset();
    });
  }

  std::shared_ptr<TestHandler> handler_;
  std::shared_ptr<folly::IOThreadPoolExecutor> executor_;
  connection::ConnectionManager::Ptr connectionManager_;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
  SimpleBufferAllocator clientAllocator_;
  SimpleBufferAllocator serverRocketAllocator_;
  std::unique_ptr<thrift::client::ThriftClientTransportAdapter>
      clientTransportAdapter_;
  PipelineImpl::Ptr clientPipeline_;
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

  destroyFastClientOnEvb(client);
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
              std::string result;
              FastClientType::recv_echo(result, state);
              cbPromise.setValue(std::move(result));
            } catch (...) {
              cbPromise.setException(
                  folly::exception_wrapper(std::current_exception()));
            }
          }),
      "hello callback");
  EXPECT_EQ(std::move(cbFuture).get(), "hello callback");

  destroyFastClientOnEvb(client);
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

  destroyFastClientOnEvb(client);
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
              std::string result;
              FastClientType::recv_sendResponse(result, state);
              cbPromise.setValue(std::move(result));
            } catch (...) {
              cbPromise.setException(
                  folly::exception_wrapper(std::current_exception()));
            }
          }),
      kResponseSize);
  auto cbResult = std::move(cbFuture).get();
  EXPECT_EQ(cbResult.size(), kResponseSize);
  EXPECT_EQ(cbResult, std::string(kResponseSize, 'x'));

  destroyFastClientOnEvb(client);
}

} // namespace apache::thrift::fast_thrift::thrift::test
