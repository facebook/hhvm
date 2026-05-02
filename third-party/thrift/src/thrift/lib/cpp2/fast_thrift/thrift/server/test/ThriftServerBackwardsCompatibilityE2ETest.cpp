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
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/adapter/RocketServerAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/connection/ConnectionHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/connection/ConnectionManager.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerRequestResponseHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/server/handler/RocketServerStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/ThriftServerChannel.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/BackwardsCompatibilityTestService.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/BackwardsCompatibilityTestServiceAsyncClient.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>

THRIFT_FLAG_DECLARE_bool(rocket_client_binary_rpc_metadata_encoding);

namespace apache::thrift::fast_thrift::thrift::server::test {

using apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder;
using apache::thrift::fast_thrift::channel_pipeline::PipelineImpl;
using apache::thrift::fast_thrift::channel_pipeline::SimpleBufferAllocator;

using apache::thrift::fast_thrift::thrift::test::
    BackwardsCompatibilityTestService;

// Handler tags for pipeline construction
HANDLER_TAG(frame_length_parser_handler);
HANDLER_TAG(frame_length_encoder_handler);
HANDLER_TAG(rocket_server_frame_codec_handler);
HANDLER_TAG(server_setup_frame_handler);
HANDLER_TAG(server_request_response_frame_handler);
HANDLER_TAG(server_stream_state_handler);

/**
 * BackwardsCompatibilityTestHandler - Implementation of the generated
 * BackwardsCompatibilityTestServiceSvIf interface.
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
};

/**
 * ThriftServerBackwardsCompatibilityE2ETest - E2E integration test for
 * fast_thrift server.
 *
 * Uses a standard Thrift client (RocketClientChannel) connecting to a
 * fast_thrift server pipeline to verify end-to-end communication.
 *
 * Server architecture (two pipelines):
 *
 * Rocket pipeline (owned by RocketServerConnection):
 *   TransportHandler
 *   -> FrameLengthParserHandler
 *   -> FrameLengthEncoderHandler
 *   -> RocketServerFrameCodecHandler
 *   -> RocketServerSetupFrameHandler
 *   -> RocketServerRequestResponseHandler
 *   -> RocketServerStreamStateHandler
 *   -> RocketServerAppAdapter
 *
 * Thrift pipeline:
 *   ThriftServerTransportAdapter -> ThriftServerChannel
 */
class ThriftServerBackwardsCompatibilityE2ETest : public ::testing::Test {
 protected:
  struct ThriftConnectionContext {
    std::shared_ptr<thrift::ThriftServerChannel> serverChannel;
    std::unique_ptr<thrift::server::ThriftServerTransportAdapter>
        transportAdapter;
    PipelineImpl::Ptr thriftPipeline;
    std::unique_ptr<SimpleBufferAllocator> thriftAllocator =
        std::make_unique<SimpleBufferAllocator>();
  };

  void SetUp() override {
    THRIFT_FLAG_SET_MOCK(rocket_client_binary_rpc_metadata_encoding, true);

    handler_ = std::make_shared<BackwardsCompatibilityTestHandler>();

    executor_ = std::make_shared<folly::IOThreadPoolExecutor>(1);

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
                      .setAllocator(&rocketAllocator_)
                      .addNextInbound<apache::thrift::fast_thrift::frame::read::
                                          handler::FrameLengthParserHandler>(
                          frame_length_parser_handler_tag)
                      .addNextOutbound<
                          apache::thrift::fast_thrift::frame::write::handler::
                              FrameLengthEncoderHandler>(
                          frame_length_encoder_handler_tag)
                      .addNextDuplex<
                          apache::thrift::fast_thrift::rocket::server::handler::
                              RocketServerFrameCodecHandler>(
                          rocket_server_frame_codec_handler_tag)
                      .addNextDuplex<
                          apache::thrift::fast_thrift::rocket::server::handler::
                              RocketServerSetupFrameHandler>(
                          server_setup_frame_handler_tag)
                      .addNextDuplex<
                          apache::thrift::fast_thrift::rocket::server::handler::
                              RocketServerStreamStateHandler>(
                          server_stream_state_handler_tag)
                      .addNextDuplex<
                          apache::thrift::fast_thrift::rocket::server::handler::
                              RocketServerRequestResponseHandler>(
                          server_request_response_frame_handler_tag)
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
                      .build();

              transportAdapter->setPipeline(thriftPipeline.get());
              serverChannel->setPipelineRef(*thriftPipeline);
              serverChannel->setWorker(
                  apache::thrift::Cpp2Worker::createDummy(evb));

              ctx.serverChannel = std::move(serverChannel);
              ctx.transportAdapter = std::move(transportAdapter);
              ctx.thriftPipeline = std::move(thriftPipeline);

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
    clientThread_.reset();
    connectionManager_->stop();
    // connectionManager_->stop() is synchronous: IO thread is done by the time
    // it returns. Clear thrift contexts here (before executor_.reset()) so
    // Cpp2Worker's IOWorkerContext is destroyed while the event base is alive.
    thriftConnections_.withWLock([](auto& conns) { conns.clear(); });
    connectionManager_.reset();
    executor_->join();
    executor_.reset();
  }

  /**
   * Create a standard Thrift client connected to the fast_thrift server.
   */
  std::unique_ptr<apache::thrift::Client<BackwardsCompatibilityTestService>>
  createStandardThriftClient() {
    auto* evb = clientThread_->getEventBase();
    std::unique_ptr<apache::thrift::Client<BackwardsCompatibilityTestService>>
        client;

    evb->runInEventBaseThreadAndWait([&] {
      auto socket =
          folly::AsyncSocket::newSocket(evb, connectionManager_->getAddress());
      auto channel =
          apache::thrift::RocketClientChannel::newChannel(std::move(socket));
      client = std::make_unique<
          apache::thrift::Client<BackwardsCompatibilityTestService>>(
          std::move(channel));
    });

    return client;
  }

  void destroyClientOnEvb(
      std::unique_ptr<
          apache::thrift::Client<BackwardsCompatibilityTestService>>& client) {
    clientThread_->getEventBase()->runInEventBaseThreadAndWait(
        [&] { client.reset(); });
  }

  std::shared_ptr<BackwardsCompatibilityTestHandler> handler_;
  std::shared_ptr<folly::IOThreadPoolExecutor> executor_;
  apache::thrift::fast_thrift::rocket::server::connection::ConnectionManager::
      Ptr connectionManager_;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
  SimpleBufferAllocator rocketAllocator_;
  folly::Synchronized<std::vector<ThriftConnectionContext>> thriftConnections_;
};

TEST_F(ThriftServerBackwardsCompatibilityE2ETest, Ping) {
  auto client = createStandardThriftClient();

  folly::Baton<> baton;
  bool success = false;

  clientThread_->getEventBase()->runInEventBaseThread([&] {
    client->semifuture_ping()
        .via(clientThread_->getEventBase())
        .thenValue([&](auto&&) {
          success = true;
          baton.post();
        })
        .thenError([&](const folly::exception_wrapper& ew) {
          LOG(ERROR) << "ping failed: " << folly::exceptionStr(ew);
          baton.post();
        });
  });

  baton.wait();
  EXPECT_TRUE(success);

  destroyClientOnEvb(client);
}

TEST_F(ThriftServerBackwardsCompatibilityE2ETest, EchoRequestResponse) {
  auto client = createStandardThriftClient();

  folly::Baton<> baton;
  std::string result;

  clientThread_->getEventBase()->runInEventBaseThread([&] {
    client->semifuture_echo("hello world")
        .via(clientThread_->getEventBase())
        .thenValue([&](std::string response) {
          result = std::move(response);
          baton.post();
        })
        .thenError([&](const folly::exception_wrapper& ew) {
          LOG(ERROR) << "echo failed: " << folly::exceptionStr(ew);
          baton.post();
        });
  });

  baton.wait();
  EXPECT_EQ(result, "hello world");

  destroyClientOnEvb(client);
}

TEST_F(ThriftServerBackwardsCompatibilityE2ETest, AddNumbers) {
  auto client = createStandardThriftClient();

  folly::Baton<> baton;
  int64_t result = 0;

  clientThread_->getEventBase()->runInEventBaseThread([&] {
    client->semifuture_add(17, 25)
        .via(clientThread_->getEventBase())
        .thenValue([&](int64_t sum) {
          result = sum;
          baton.post();
        })
        .thenError([&](folly::exception_wrapper) { baton.post(); });
  });

  baton.wait();
  EXPECT_EQ(result, 42);

  destroyClientOnEvb(client);
}

TEST_F(ThriftServerBackwardsCompatibilityE2ETest, MultipleRequests) {
  auto client = createStandardThriftClient();

  folly::Baton<> baton1, baton2, baton3;
  std::string echo1, echo2, echo3;

  clientThread_->getEventBase()->runInEventBaseThread([&] {
    client->semifuture_echo("first")
        .via(clientThread_->getEventBase())
        .thenValue([&](std::string response) {
          echo1 = std::move(response);
          baton1.post();
        })
        .thenError([&](folly::exception_wrapper) { baton1.post(); });

    client->semifuture_echo("second")
        .via(clientThread_->getEventBase())
        .thenValue([&](std::string response) {
          echo2 = std::move(response);
          baton2.post();
        })
        .thenError([&](folly::exception_wrapper) { baton2.post(); });

    client->semifuture_echo("third")
        .via(clientThread_->getEventBase())
        .thenValue([&](std::string response) {
          echo3 = std::move(response);
          baton3.post();
        })
        .thenError([&](folly::exception_wrapper) { baton3.post(); });
  });

  baton1.wait();
  baton2.wait();
  baton3.wait();

  EXPECT_EQ(echo1, "first");
  EXPECT_EQ(echo2, "second");
  EXPECT_EQ(echo3, "third");

  destroyClientOnEvb(client);
}

TEST_F(ThriftServerBackwardsCompatibilityE2ETest, LargeResponse) {
  auto client = createStandardThriftClient();

  constexpr int64_t kResponseSize = 10000;

  folly::Baton<> baton;
  std::string result;

  clientThread_->getEventBase()->runInEventBaseThread([&] {
    client->semifuture_sendResponse(kResponseSize)
        .via(clientThread_->getEventBase())
        .thenValue([&](std::string response) {
          result = std::move(response);
          baton.post();
        })
        .thenError([&](folly::exception_wrapper) { baton.post(); });
  });

  baton.wait();
  EXPECT_EQ(result, std::string(kResponseSize, 'x'));

  destroyClientOnEvb(client);
}

} // namespace apache::thrift::fast_thrift::thrift::server::test
