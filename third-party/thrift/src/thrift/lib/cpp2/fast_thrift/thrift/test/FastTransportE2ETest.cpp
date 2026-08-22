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

// Exercises the fast_thrift transport with a channel-based client and a
// channel-based server: only the framing/rocket transport is fast_thrift; the
// application layer on both ends is the legacy channel path
// (ThriftClientChannel driving Client<TestService> and ThriftServerChannel
// dispatching to ServiceHandler<TestService>).

#include <gtest/gtest.h>

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
#include <thrift/lib/cpp2/fast_thrift/rocket/common/RocketStreamContext.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerMessageMarshalHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientChannel.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/ThriftServerChannel.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/ThriftServerSetupHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/TestService.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/TestServiceAsyncClient.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>

#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_constants.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

THRIFT_FLAG_DECLARE_bool(rocket_client_binary_rpc_metadata_encoding);

namespace apache::thrift::fast_thrift::thrift::test {

using apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder;
using apache::thrift::fast_thrift::channel_pipeline::PipelineImpl;
using apache::thrift::fast_thrift::channel_pipeline::SimpleBufferAllocator;

using apache::thrift::fast_thrift::thrift::test::TestService;

// Client handler tags
HANDLER_TAG(client_frame_length_parser_handler);
HANDLER_TAG(client_frame_length_encoder_handler);
HANDLER_TAG(rocket_client_frame_codec_handler);
HANDLER_TAG(rocket_client_setup_handler);
HANDLER_TAG(rocket_client_request_response_handler);
HANDLER_TAG(rocket_client_connection_error_handler);
HANDLER_TAG(rocket_client_stream_state_handler);

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
HANDLER_TAG(thrift_server_setup_handler);

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
 * FastTransportE2ETest - End-to-end integration test where only the transport
 * is fast_thrift. The client is a channel-based Client<TestService> driven by
 * ThriftClientChannel; the server dispatches through ThriftServerChannel.
 *
 * Server (two-pipeline):
 *   Rocket pipeline: TransportHandler -> ... -> RocketServerAppAdapter
 *   Thrift pipeline: ThriftServerTransportAdapter -> ThriftServerChannel
 *
 * Client (single-pipeline):
 *   Rocket pipeline: RocketClientAppAdapter -> ... -> TransportHandler
 *   The connected rocket connection is handed to ThriftClientChannel, which
 *   drives it directly.
 */
class FastTransportE2ETest : public ::testing::Test {
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
    explicit ServerConnectionFactory(FastTransportE2ETest* fixture) noexcept
        : fixture_(fixture) {}
    ServerConnection getConnection(
        folly::AsyncTransport::UniquePtr socket,
        const folly::SocketAddress& /*clientAddr*/,
        const std::shared_ptr<const connection::PeerSecurityInfo>&
        /*peerSecurity*/) {
      return fixture_->buildServerConnection(std::move(socket));
    }

   private:
    FastTransportE2ETest* fixture_;
  };

  ServerConnection buildServerConnection(
      folly::AsyncTransport::UniquePtr socket) {
    auto* evb = socket->getEventBase();

    // Build rocket-layer pieces inside a rocket::server::RocketServerConnection
    // so ThriftServerTransportAdapter can take ownership of the whole bundle.
    auto rocketConn = std::make_unique<
        apache::thrift::fast_thrift::rocket::server::RocketServerConnection>();
    rocketConn->transportHandler = apache::thrift::fast_thrift::rocket::server::
        RocketServerTransportHandler::create(std::move(socket));

    rocketConn->pipeline =
        PipelineBuilder<
            apache::thrift::fast_thrift::rocket::server::
                RocketServerTransportHandler,
            apache::thrift::fast_thrift::rocket::server::RocketServerAppAdapter,
            SimpleBufferAllocator>()
            .setEventBase(evb)
            .setHead(rocketConn->transportHandler.get())
            .setTail(rocketConn->appAdapter.get())
            .setAllocator(&serverRocketAllocator_)
            .addState<
                apache::thrift::fast_thrift::rocket::RocketStreamContexts>()
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
    conn.thriftPipeline =
        PipelineBuilder<
            thrift::server::ThriftServerTransportAdapter,
            thrift::ThriftServerChannel,
            SimpleBufferAllocator>()
            .setEventBase(evb)
            .setHead(transportAdapter.get())
            .setTail(serverChannel.get())
            .setAllocator(&conn.thriftAllocator)
            .template addNextDuplex<thrift::ThriftServerSetupHandler<
                channel_pipeline::detail::ContextImpl>>(
                thrift_server_setup_handler_tag)
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
   * Create a channel-based client connected to the fast_thrift server.
   *
   * Builds only the rocket pipeline and hands the connected rocket connection
   * to ThriftClientChannel, which drives it directly (no thrift pipeline /
   * transport adapter).
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
        meta.clientMetadata().ensure().agent() = "fast_transport_e2e_test";

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
              .addState<apache::thrift::fast_thrift::rocket::client::
                            RocketClientStreamContexts>()
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

      // 2. Hand the connected rocket connection to the channel, which drives
      // it directly (no thrift pipeline / transport adapter).
      channel = thrift::ThriftClientChannel::newChannel(std::move(connection));

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
    // The client owns the channel, which owns the rocket connection; reset on
    // the evb thread.
    clientThread_->getEventBase()->runInEventBaseThreadAndWait(
        [&] { client.reset(); });
  }

  std::shared_ptr<TestHandler> handler_;
  std::shared_ptr<folly::IOThreadPoolExecutor> executor_;
  apache::thrift::fast_thrift::connection::ConnectionManager::Ptr
      connectionManager_;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
  SimpleBufferAllocator serverRocketAllocator_;
  std::unique_ptr<ConnectCallback> connectCallback_;
};

// =============================================================================
// Test Cases
// =============================================================================

TEST_F(FastTransportE2ETest, Ping) {
  auto client = createClient();

  folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(), client->co_ping()));

  destroyClientOnEvb(client);
}

TEST_F(FastTransportE2ETest, EchoRequestResponse) {
  auto client = createClient();

  auto result = folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(), client->co_echo("hello world")));
  EXPECT_EQ(result, "hello world");

  destroyClientOnEvb(client);
}

TEST_F(FastTransportE2ETest, SequentialRequests) {
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

TEST_F(FastTransportE2ETest, MultipleRequests) {
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

TEST_F(FastTransportE2ETest, LargeResponse) {
  auto client = createClient();

  constexpr int64_t kResponseSize = 10000;

  auto result = folly::coro::blockingWait(
      folly::coro::co_withExecutor(
          clientThread_->getEventBase(),
          client->co_sendResponse(kResponseSize)));
  EXPECT_EQ(result, std::string(kResponseSize, 'x'));

  destroyClientOnEvb(client);
}

} // namespace apache::thrift::fast_thrift::thrift::test
