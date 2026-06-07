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
 * End-to-end tests for ThriftServerCompositeAppAdapter.
 *
 * The composite sits between the thrift pipeline and codegen-generated
 * <Service>AppAdapter children. Synthetic tests (composite unit + composite
 * integration) cover routing/lifecycle in isolation and with hand-built
 * child adapters. These tests exercise the production scenario: real
 * sockets, real Rocket pipelines on both sides, and real codegen-generated
 * server/client stubs for *two distinct* services hosted behind one
 * composite.
 *
 * Modeled on FastThriftFastClientE2ETest. The only structural difference
 * is the thrift pipeline tail: a ThriftServerCompositeAppAdapter wrapping
 * CompositeE2EPrimaryServiceAppAdapter + CompositeE2ESecondaryServiceAppAdapter
 * (both codegen-generated, both bound to their respective FastServiceHandler).
 */

#include <gtest/gtest.h>

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
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientErrorFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientRequestResponseHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/common/RocketServerConnection.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerMessageMarshalHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/security/SSLPolicy.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/adapter/ThriftClientTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientChecksumHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientMetadataPushHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerCompositeAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/CompositeE2EPrimaryService.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/CompositeE2EPrimaryService.tcc>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/CompositeE2ESecondaryService.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/CompositeE2ESecondaryService.tcc>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_constants.h>

THRIFT_FLAG_DECLARE_bool(rocket_client_binary_rpc_metadata_encoding);

namespace {

using namespace apache::thrift::fast_thrift;
using namespace apache::thrift::fast_thrift::thrift::test::composite_e2e;

using channel_pipeline::PipelineBuilder;
using channel_pipeline::PipelineImpl;
using channel_pipeline::SimpleBufferAllocator;

// Server handler tags
HANDLER_TAG(server_frame_length_parser_handler);
HANDLER_TAG(server_frame_length_encoder_handler);
HANDLER_TAG(server_frame_codec_handler);
HANDLER_TAG(server_frame_defragmentation_handler);
HANDLER_TAG(server_frame_fragmentation_handler);
HANDLER_TAG(rocket_server_message_marshal_handler);
HANDLER_TAG(rocket_server_setup_frame_handler);
HANDLER_TAG(rocket_server_stream_state_handler);
HANDLER_TAG(rocket_server_request_response_handler);

// Client handler tags
HANDLER_TAG(client_frame_length_parser_handler);
HANDLER_TAG(client_frame_length_encoder_handler);
HANDLER_TAG(rocket_client_frame_codec_handler);
HANDLER_TAG(rocket_client_setup_handler);
HANDLER_TAG(rocket_client_error_frame_handler);
HANDLER_TAG(rocket_client_stream_state_handler);
HANDLER_TAG(rocket_client_request_response_handler);
HANDLER_TAG(thrift_client_metadata_push_handler);
HANDLER_TAG(thrift_client_checksum_handler);

// =============================================================================
// User-implemented handlers — one per service. Counters are atomic because
// the server runs the handler on its IO thread, the test reads on its own
// thread.
// =============================================================================

class PrimaryHandler
    : public apache::thrift::FastServiceHandler<CompositeE2EPrimaryService> {
 public:
  std::atomic<int> pingCount{0};
  std::atomic<int> echoCount{0};
  std::atomic<int> addCount{0};

  void async_eb_primaryPing(thrift::FastHandlerCallbackPtr<void> cb) override {
    pingCount++;
    cb->done();
  }

  void async_eb_primaryEcho(
      thrift::FastHandlerCallbackPtr<std::unique_ptr<std::string>> cb,
      std::unique_ptr<std::string> message) override {
    echoCount++;
    cb->result(std::make_unique<std::string>(*message + "_primary"));
  }

  void async_eb_primaryAdd(
      thrift::FastHandlerCallbackPtr<int64_t> cb,
      int64_t a,
      int64_t b) override {
    addCount++;
    cb->result(a + b);
  }
};

class SecondaryHandler
    : public apache::thrift::FastServiceHandler<CompositeE2ESecondaryService> {
 public:
  std::atomic<int> pingCount{0};
  std::atomic<int> greetCount{0};
  std::atomic<int> multiplyCount{0};

  void async_eb_secondaryPing(
      thrift::FastHandlerCallbackPtr<void> cb) override {
    pingCount++;
    cb->done();
  }

  void async_eb_secondaryGreet(
      thrift::FastHandlerCallbackPtr<std::unique_ptr<std::string>> cb,
      std::unique_ptr<std::string> name) override {
    greetCount++;
    cb->result(std::make_unique<std::string>("hello, " + *name));
  }

  void async_eb_secondaryMultiply(
      thrift::FastHandlerCallbackPtr<int64_t> cb,
      int64_t a,
      int64_t b) override {
    multiplyCount++;
    cb->result(a * b);
  }
};

// =============================================================================
// ConnectCallback — sets pipeline_'s active state after socket connect.
// Lifted verbatim from FastThriftE2ETest pattern.
// =============================================================================

class ConnectCallback : public folly::AsyncSocket::ConnectCallback {
 public:
  ConnectCallback(
      rocket::client::RocketClientConnection::TransportHandler* handler,
      folly::Baton<>& baton,
      bool& connected)
      : handler_(handler), baton_(baton), connected_(connected) {}

  void connectSuccess() noexcept override {
    handler_->onConnect();
    connected_ = true;
    baton_.post();
  }

  void connectErr(const folly::AsyncSocketException&) noexcept override {
    connected_ = false;
    baton_.post();
  }

 private:
  rocket::client::RocketClientConnection::TransportHandler* handler_;
  folly::Baton<>& baton_;
  bool& connected_;
};

} // namespace

// =============================================================================
// Fixture
// =============================================================================

class ThriftServerCompositeE2ETest : public ::testing::Test {
 protected:
  using PrimaryClient = apache::thrift::
      FastClient<CompositeE2EPrimaryService, thrift::ThriftClientAppAdapter>;
  using SecondaryClient = apache::thrift::
      FastClient<CompositeE2ESecondaryService, thrift::ThriftClientAppAdapter>;

  // Per-connection server context — the composite borrows its children, so
  // we have to keep them alive at least as long as the composite + pipelines.
  // Satisfies connection::Connection (close/drain/setCloseCallback) so it
  // can be returned directly from the connection factory; ConnectionManager
  // keeps it alive until the connection closes.
  struct ServerConnectionContext {
    // Field order is destruction order in reverse. Tail handlers (composite,
    // children) must be declared FIRST so they outlive transportAdapter:
    // transportAdapter's destructor releases the last pipeline
    // DestructorGuard, which kicks off PipelineImpl::onDelayedDestroy →
    // callHandlerRemovedImpl → composite->handlerRemoved(). Composite must
    // still be alive at that point. Mirrors FastThriftFastClientE2ETest's
    // ThriftConnectionContext, where serverChannel (the tail) is declared
    // before transportAdapter for the same reason.
    thrift::ThriftServerAppAdapter::Ptr primaryChild;
    thrift::ThriftServerAppAdapter::Ptr secondaryChild;
    thrift::ThriftServerCompositeAppAdapter::Ptr composite;
    std::unique_ptr<thrift::server::ThriftServerTransportAdapter>
        transportAdapter;
    PipelineImpl::Ptr thriftPipeline;
    std::unique_ptr<SimpleBufferAllocator> thriftAllocator =
        std::make_unique<SimpleBufferAllocator>();
    std::unique_ptr<SimpleBufferAllocator> rocketAllocator =
        std::make_unique<SimpleBufferAllocator>();

    std::function<void()> closeCb;
    bool closed{false};

    ServerConnectionContext() = default;
    ServerConnectionContext(ServerConnectionContext&&) noexcept = default;
    ServerConnectionContext& operator=(ServerConnectionContext&&) noexcept =
        default;
    ServerConnectionContext(const ServerConnectionContext&) = delete;
    ServerConnectionContext& operator=(const ServerConnectionContext&) = delete;

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

  class ServerConnectionFactory {
   public:
    using BuildFn = std::function<ServerConnectionContext(
        folly::AsyncTransport::UniquePtr)>;

    explicit ServerConnectionFactory(BuildFn build) noexcept
        : build_(std::move(build)) {}

    ServerConnectionContext getConnection(
        folly::AsyncTransport::UniquePtr socket) {
      return build_(std::move(socket));
    }

   private:
    BuildFn build_;
  };

  void SetUp() override {
    THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

    primaryHandler_ = std::make_shared<PrimaryHandler>();
    secondaryHandler_ = std::make_shared<SecondaryHandler>();
    executor_ = std::make_shared<folly::IOThreadPoolExecutor>(1);

    connectionManager_ = connection::ConnectionManager::create(
        folly::SocketAddress("::1", 0),
        folly::getKeepAliveToken(executor_.get()),
        security::SSLPolicy::DISABLED,
        /*tlsParams=*/nullptr,
        connection::SocketOptions{});
    connectionManager_->setConnectionFactory(
        ServerConnectionFactory{
            [this](folly::AsyncTransport::UniquePtr socket) {
              return buildServerConnection(std::move(socket));
            }});
    connectionManager_->start();

    clientThread_ = std::make_unique<folly::ScopedEventBaseThread>();
  }

  ServerConnectionContext buildServerConnection(
      folly::AsyncTransport::UniquePtr socket) {
    auto* evb = socket->getEventBase();

    // Rocket-layer pieces live inside a rocket::server::RocketServerConnection
    // so the thrift transport adapter can take ownership of the whole bundle.
    auto rocketConn =
        std::make_unique<rocket::server::RocketServerConnection>();
    rocketConn->transportHandler =
        transport::TransportHandler::create(std::move(socket));

    ServerConnectionContext ctx;

    // 1. Rocket pipeline (same as bare-server E2E)
    rocketConn->pipeline =
        PipelineBuilder<
            transport::TransportHandler,
            rocket::server::RocketServerAppAdapter,
            SimpleBufferAllocator>()
            .setEventBase(evb)
            .setHead(rocketConn->transportHandler.get())
            .setTail(rocketConn->appAdapter.get())
            .setAllocator(ctx.rocketAllocator.get())
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

    // 2. Thrift pipeline — composite wraps two codegen children.
    ctx.primaryChild.reset(
        new CompositeE2EPrimaryServiceAppAdapter(primaryHandler_));
    ctx.secondaryChild.reset(
        new CompositeE2ESecondaryServiceAppAdapter(secondaryHandler_));

    ctx.composite.reset(new thrift::ThriftServerCompositeAppAdapter());
    ctx.composite->addChild(ctx.primaryChild.get());
    ctx.composite->addChild(ctx.secondaryChild.get());

    ctx.transportAdapter =
        std::make_unique<thrift::server::ThriftServerTransportAdapter>(
            std::move(rocketConn));

    ctx.thriftPipeline = PipelineBuilder<
                             thrift::server::ThriftServerTransportAdapter,
                             thrift::ThriftServerCompositeAppAdapter,
                             SimpleBufferAllocator>()
                             .setEventBase(evb)
                             .setHead(ctx.transportAdapter.get())
                             .setTail(ctx.composite.get())
                             .setAllocator(ctx.thriftAllocator.get())
                             .build();

    ctx.transportAdapter->setPipeline(ctx.thriftPipeline.get());
    // composite's setPipeline fans out to both children so their
    // writeResponse fires through the same thrift pipeline.
    ctx.composite->setPipeline(ctx.thriftPipeline.get());
    // Activate so composite's onPipelineActive fans out to children
    // (Ready -> Open). Without this, child onRead rejects with
    // Result::Error because base state-checks state == Open.
    ctx.thriftPipeline->activate();
    // Connection is inert; ConnectionHandler's installer lambda calls
    // start() after registering the entry, which fires onConnect().
    return ctx;
  }

  // Per-client connection resources. Each FastClient owns its appAdapter
  // (FastClientBase ctor takes Ptr by value), so we cannot share one
  // connection across multiple FastClient instances. Each createClient()
  // call stands up a fresh pipeline; teardown clears the vector under
  // the client EVB.
  struct ClientConnectionResources {
    std::unique_ptr<thrift::client::ThriftClientTransportAdapter>
        transportAdapter;
    PipelineImpl::Ptr pipeline;
  };

  void TearDown() override {
    clientThread_->getEventBase()->runInEventBaseThreadAndWait([&] {
      // Tear down in the correct order:
      // 1. Close pipelines so they stop referencing their tail (appAdapter).
      // 2. Drop transport adapter's pipeline ref.
      // 3. Destroy FastClients (which own the appAdapter Ptrs).
      // 4. Destroy the pipelines themselves.
      for (auto& conn : clientConnections_) {
        if (conn.pipeline) {
          conn.pipeline->deactivate();
          conn.pipeline->close();
        }
        if (conn.transportAdapter) {
          conn.transportAdapter->resetPipeline();
        }
      }
      for (auto& destroy : clientDestroyers_) {
        destroy();
      }
      clientDestroyers_.clear();
      clientConnections_.clear();
    });
    clientThread_.reset();
    // stop() drains every live connection, waits for the drain, then
    // force-closes any stragglers. By the time it returns, every
    // ServerConnectionContext has been destroyed on the IO thread, so the
    // composite + children cannot UAF.
    connectionManager_->stop();
    connectionManager_.reset();
    executor_->join();
    executor_.reset();
  }

  // Stand up a client connection and a typed FastClient on top of it.
  // The fixture takes ownership of the FastClient so we can sequence its
  // destruction (must happen after the pipeline closes, before the
  // pipeline itself is destroyed). Returns a non-owning raw pointer the
  // test uses for the duration of the test body.
  template <typename ClientType>
  ClientType* createClient() {
    auto appAdapter = connectClientPipeline();
    auto client = std::make_shared<ClientType>(std::move(appAdapter));
    ClientType* raw = client.get();
    clientDestroyers_.push_back([client]() mutable { client.reset(); });
    return raw;
  }

  // Stand up a client pipeline + connect to the server. Returns the
  // ThriftClientAppAdapter::Ptr which the caller passes to a FastClient
  // (the FastClient takes ownership of the Ptr).
  thrift::ThriftClientAppAdapter::Ptr connectClientPipeline() {
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

      auto connection =
          std::make_unique<rocket::client::RocketClientConnection>();

      connection->transportHandler =
          rocket::client::RocketClientConnection::TransportHandler::create(
              std::move(socket));

      auto* transportHandlerPtr = connection->transportHandler.get();

      auto connectCallback = std::make_unique<ConnectCallback>(
          transportHandlerPtr, connectBaton, connected);
      auto* connectCallbackPtr = connectCallback.get();
      connectCallbacks_.push_back(std::move(connectCallback));
      socketPtr->connect(
          connectCallbackPtr, connectionManager_->getAddress(), 30000);

      auto setupFactory = []() {
        apache::thrift::RequestSetupMetadata meta;
        meta.minVersion() = 8;
        meta.maxVersion() = 10;
        meta.clientMetadata().ensure().agent() =
            "thrift_server_composite_e2e_test";

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
                  rocket::client::handler::RocketClientErrorFrameHandler>(
                  rocket_client_error_frame_handler_tag)
              .addNextDuplex<
                  rocket::client::handler::RocketClientStreamStateHandler>(
                  rocket_client_stream_state_handler_tag)
              .addNextInbound<
                  rocket::client::handler::RocketClientRequestResponseHandler>(
                  rocket_client_request_response_handler_tag)
              .build();

      connection->appAdapter->setPipeline(connection->pipeline.get());
      connection->transportHandler->setPipeline(connection->pipeline.get());

      ClientConnectionResources resources;
      resources.transportAdapter =
          std::make_unique<thrift::client::ThriftClientTransportAdapter>(
              std::move(connection));

      resources.pipeline =
          PipelineBuilder<
              thrift::client::ThriftClientTransportAdapter,
              thrift::ThriftClientAppAdapter,
              SimpleBufferAllocator>()
              .setEventBase(evb)
              .setHead(resources.transportAdapter.get())
              .setTail(appAdapter.get())
              .setAllocator(&clientAllocator_)
              .addNextInbound<
                  thrift::client::handler::ThriftClientMetadataPushHandler>(
                  thrift_client_metadata_push_handler_tag)
              .addNextOutbound<
                  thrift::client::handler::ThriftClientChecksumHandler>(
                  thrift_client_checksum_handler_tag)
              .build();

      appAdapter->setPipeline(resources.pipeline.get());
      resources.transportAdapter->setPipeline(resources.pipeline.get());

      clientConnections_.push_back(std::move(resources));
    });

    connectBaton.wait();
    if (!connected) {
      throw std::runtime_error("Failed to connect to server");
    }
    return appAdapter;
  }

  std::shared_ptr<PrimaryHandler> primaryHandler_;
  std::shared_ptr<SecondaryHandler> secondaryHandler_;
  std::shared_ptr<folly::IOThreadPoolExecutor> executor_;
  connection::ConnectionManager::Ptr connectionManager_;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
  SimpleBufferAllocator clientAllocator_;
  std::vector<ClientConnectionResources> clientConnections_;
  std::vector<std::function<void()>> clientDestroyers_;
  std::vector<std::unique_ptr<ConnectCallback>> connectCallbacks_;
};

// =============================================================================
// Tests
// =============================================================================

TEST_F(ThriftServerCompositeE2ETest, PrimaryPingRoutesToPrimaryChild) {
  auto* primary = createClient<PrimaryClient>();

  folly::coro::blockingWait(primary->co_primaryPing());

  EXPECT_EQ(primaryHandler_->pingCount.load(), 1);
  EXPECT_EQ(secondaryHandler_->pingCount.load(), 0);
}

TEST_F(ThriftServerCompositeE2ETest, SecondaryGreetRoutesToSecondaryChild) {
  auto* secondary = createClient<SecondaryClient>();

  auto response =
      folly::coro::blockingWait(secondary->co_secondaryGreet("ankit"));

  EXPECT_EQ(response, "hello, ankit");
  EXPECT_EQ(secondaryHandler_->greetCount.load(), 1);
  EXPECT_EQ(primaryHandler_->echoCount.load(), 0);
}

TEST_F(ThriftServerCompositeE2ETest, BothServicesRouteCorrectly) {
  // Each FastClient owns its appAdapter, so we stand up two separate
  // connections — but each server connection's composite still has BOTH
  // children, so routing fan-out per-composite is what's under test. The
  // handler counters at the end prove each child served only its own
  // methods.
  auto* primary = createClient<PrimaryClient>();
  auto* secondary = createClient<SecondaryClient>();

  auto echoed = folly::coro::blockingWait(primary->co_primaryEcho("ping"));
  auto greeted =
      folly::coro::blockingWait(secondary->co_secondaryGreet("ankit"));
  auto sum = folly::coro::blockingWait(primary->co_primaryAdd(7, 35));
  auto product =
      folly::coro::blockingWait(secondary->co_secondaryMultiply(6, 7));

  EXPECT_EQ(echoed, "ping_primary");
  EXPECT_EQ(greeted, "hello, ankit");
  EXPECT_EQ(sum, 42);
  EXPECT_EQ(product, 42);

  EXPECT_EQ(primaryHandler_->echoCount.load(), 1);
  EXPECT_EQ(primaryHandler_->addCount.load(), 1);
  EXPECT_EQ(secondaryHandler_->greetCount.load(), 1);
  EXPECT_EQ(secondaryHandler_->multiplyCount.load(), 1);

  // Explicit cross-pollination check — neither child must have served the
  // other's methods.
  EXPECT_EQ(primaryHandler_->pingCount.load(), 0);
  EXPECT_EQ(secondaryHandler_->pingCount.load(), 0);
}
