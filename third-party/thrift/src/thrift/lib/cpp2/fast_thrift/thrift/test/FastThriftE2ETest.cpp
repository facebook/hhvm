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

// End-to-end test of the whole fast_thrift stack: a native FastClient talking
// to a real FastThriftServer. The server runs a FastServiceHandler<Service>
// behind its own internally-built pipeline; the client is a
// FastClient<TestFastService, ThriftClientAppAdapter> whose pipeline is stood
// up by hand (rocket transport + thrift application handlers).

#include <gtest/gtest.h>

#include <folly/coro/BlockingWait.h>
#include <folly/coro/Task.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/async/RpcOptions.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/frame/read/handler/FrameLengthParserHandler.h>
#include <thrift/lib/cpp2/fast_thrift/frame/write/handler/FrameLengthEncoderHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/adapter/RocketClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/common/RocketClientConnection.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientConnectionErrorHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientFrameCodecHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientRequestResponseHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientSetupFrameHandler.h>
#include <thrift/lib/cpp2/fast_thrift/rocket/client/handler/RocketClientStreamStateHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/ThriftClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/adapter/ThriftClientTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientChecksumHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientMetadataPushHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/FastThriftServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/TestFastService.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/TestFastServiceAsyncClient.h>
#include <thrift/lib/cpp2/fast_thrift/transport/TransportHandler.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_constants.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift::fast_thrift::thrift::test {

namespace ftt = ::apache::thrift::fast_thrift::thrift;

using apache::thrift::fast_thrift::channel_pipeline::PipelineBuilder;
using apache::thrift::fast_thrift::channel_pipeline::PipelineImpl;
using apache::thrift::fast_thrift::channel_pipeline::SimpleBufferAllocator;

using apache::thrift::fast_thrift::thrift::test::TestFastService;

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
 * TestFastServiceHandler - identity/arithmetic FastServiceHandler bound into
 * the FastThriftServer.
 */
class TestFastServiceHandler
    : public apache::thrift::FastServiceHandler<TestFastService> {
 public:
  void async_eb_echo(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<std::string>> cb,
      std::unique_ptr<std::string> message) override {
    cb->result(std::move(message));
  }

  void async_eb_add(
      ftt::FastHandlerCallbackPtr<int64_t> cb, int64_t a, int64_t b) override {
    cb->result(a + b);
  }

  void async_eb_sendResponse(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<std::string>> cb,
      int64_t size) override {
    cb->result(std::make_unique<std::string>(static_cast<size_t>(size), 'x'));
  }

  void async_eb_ping(ftt::FastHandlerCallbackPtr<void> cb) override {
    cb->done();
  }
};

/**
 * FastThriftE2ETest - native FastClient against a real FastThriftServer.
 *
 * Server: FastThriftServer serving TestFastServiceHandler through the
 *   pipeline it builds internally.
 * Client (two-pipeline):
 *   Rocket pipeline: RocketClientAppAdapter → [rocket handlers] →
 *     TransportHandler
 *   Thrift pipeline: ThriftClientAppAdapter → ThriftClientMetadataPushHandler
 *     → ThriftClientChecksumHandler → ThriftClientTransportAdapter
 */
class FastThriftE2ETest : public ::testing::Test {
 protected:
  using FastClientType = apache::thrift::
      FastClient<TestFastService, thrift::ThriftClientAppAdapter>;

  void SetUp() override {
    handler_ = std::make_shared<TestFastServiceHandler>();

    ftt::FastThriftServerConfig config;
    config.address = folly::SocketAddress("::1", 0);
    config.numIOThreads = 1;
    // Validate request checksums and echo a response checksum. enableChecksum
    // requires enableRequestContext (the response algorithm rides the
    // per-request ThriftRequestContext).
    config.enableRequestContext = true;
    config.enableChecksum = true;

    server_ = std::make_unique<ftt::FastThriftServer>(std::move(config));
    server_->setInterface(handler_);
    server_->start();

    clientThread_ = std::make_unique<folly::ScopedEventBaseThread>();
  }

  void TearDown() override {
    clientThread_->getEventBase()->runInEventBaseThreadAndWait([&] {
      clientPipeline_.reset();
      clientTransportAdapter_.reset();
    });
    clientThread_.reset();
    server_->stop();
    server_.reset();
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

      // 1. Build rocket pipeline inside RocketClientConnection
      auto connection =
          std::make_unique<rocket::client::RocketClientConnection>();

      connection->transportHandler =
          rocket::client::RocketClientConnection::TransportHandler::create(
              std::move(socket));

      auto* transportHandlerPtr = connection->transportHandler.get();

      connectCallback_ = std::make_unique<ConnectCallback>(
          transportHandlerPtr, connectBaton, connected);
      socketPtr->connect(connectCallback_.get(), server_->getAddress(), 30000);

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

      // 2. Build thrift pipeline: AppAdapter → TransportAdapter. The checksum
      // handler fills request checksums and validates response checksums.
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
              .addNextDuplex<
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

  std::shared_ptr<TestFastServiceHandler> handler_;
  std::unique_ptr<ftt::FastThriftServer> server_;
  std::unique_ptr<folly::ScopedEventBaseThread> clientThread_;
  SimpleBufferAllocator clientAllocator_;
  std::unique_ptr<thrift::client::ThriftClientTransportAdapter>
      clientTransportAdapter_;
  PipelineImpl::Ptr clientPipeline_;
  std::unique_ptr<ConnectCallback> connectCallback_;
};

// =============================================================================
// Test Cases
// =============================================================================

TEST_F(FastThriftE2ETest, Ping) {
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

TEST_F(FastThriftE2ETest, Echo) {
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

TEST_F(FastThriftE2ETest, Add) {
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

TEST_F(FastThriftE2ETest, SendResponse) {
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

// Full both-direction checksum round-trip. A successful call proves the client
// sent a real XXH3 checksum, the server validated the request, the server
// echoed a checksum, and the client validated the echo — any broken link
// surfaces as a CHECKSUM_MISMATCH RPC error.
TEST_F(FastThriftE2ETest, ChecksumRoundTrip) {
  auto client = createFastClient();
  auto* evb = clientThread_->getEventBase();

  apache::thrift::RpcOptions opts;
  opts.setChecksum(apache::thrift::RpcOptions::Checksum::XXH3_64);

  auto result = folly::coro::blockingWait(
      folly::coro::co_withExecutor(evb, client->co_echo(opts, "checksummed")));
  EXPECT_EQ(result, "checksummed");

  destroyFastClientOnEvb(client);
}

} // namespace apache::thrift::fast_thrift::thrift::test
