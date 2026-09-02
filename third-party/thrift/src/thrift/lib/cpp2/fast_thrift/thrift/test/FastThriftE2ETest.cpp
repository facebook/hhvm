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

#include <folly/Executor.h>
#include <folly/coro/BlockingWait.h>
#include <folly/coro/Task.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/task_queue/BlockingQueue.h>
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
#include <thrift/lib/cpp2/fast_thrift/thrift/client/adapter/BorrowedClientAppAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/adapter/ThriftClientTransportAdapter.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientChecksumHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/client/handler/ThriftClientMetadataPushHandler.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/FastThriftServer.h>
#include <thrift/lib/cpp2/fast_thrift/thrift/server/handler/ThriftServerSetupHandler.h>
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

// Server handler tags
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
 * TestFastServiceHandler - identity/arithmetic FastServiceHandler bound into
 * the FastThriftServer.
 */
class TestFastServiceHandler
    : public apache::thrift::FastServiceHandler<TestFastService> {
 public:
  // When set, echo hands the callback to this executor instead of completing
  // inline, so the callback is released on a thread that is neither the
  // connection's EventBase nor the thread the method was dispatched on.
  void setEchoCompletionExecutor(folly::Executor* executor) {
    echoExecutor_ = executor;
  }

  // When set, echo parks the callback instead of completing it, so a test can
  // tear the connection down with a request still outstanding and complete it
  // afterwards.
  void parkNextEcho() { parkEcho_ = true; }

  // Blocks until async_tm_echo has parked a callback. Returns false on timeout.
  bool waitParkedEcho(std::chrono::milliseconds timeout) {
    return parkedBaton_.try_wait_for(timeout);
  }

  // Completes a parked callback, if any. Returns false when none was parked.
  bool releaseParkedEcho() {
    ftt::FastHandlerCallbackPtr<std::unique_ptr<std::string>> parked;
    {
      std::lock_guard<std::mutex> lock(parkedMutex_);
      parked = std::move(parkedEcho_);
    }
    if (!parked) {
      return false;
    }
    parked->result(std::make_unique<std::string>("parked"));
    return true;
  }

  void async_tm_echo(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<std::string>> cb,
      std::unique_ptr<std::string> message) override {
    if (parkEcho_.exchange(false)) {
      {
        std::lock_guard<std::mutex> lock(parkedMutex_);
        parkedEcho_ = std::move(cb);
      }
      parkedBaton_.post();
      return;
    }
    if (auto* executor = echoExecutor_.load()) {
      executor->add(
          [cb = std::move(cb), message = std::move(message)]() mutable {
            cb->result(std::move(message));
          });
      return;
    }
    cb->result(std::move(message));
  }

  void async_tm_add(
      ftt::FastHandlerCallbackPtr<int64_t> cb, int64_t a, int64_t b) override {
    cb->result(a + b);
  }

  void async_tm_sendResponse(
      ftt::FastHandlerCallbackPtr<std::unique_ptr<std::string>> cb,
      int64_t size) override {
    cb->result(std::make_unique<std::string>(static_cast<size_t>(size), 'x'));
  }

  void async_tm_ping(ftt::FastHandlerCallbackPtr<void> cb) override {
    cb->done();
  }

  void async_tm_ranOnEventBase(ftt::FastHandlerCallbackPtr<bool> cb) override {
    const bool onEventBase = cb->getEventBase()->isInEventBaseThread();
    cb->result(onEventBase);
  }

  void async_eb_ebRanOnEventBase(
      ftt::FastHandlerCallbackPtr<bool> cb) override {
    const bool onEventBase = cb->getEventBase()->isInEventBaseThread();
    cb->result(onEventBase);
  }

 private:
  std::atomic<folly::Executor*> echoExecutor_{nullptr};
  std::atomic<bool> parkEcho_{false};
  std::mutex parkedMutex_;
  ftt::FastHandlerCallbackPtr<std::unique_ptr<std::string>> parkedEcho_;
  folly::Baton<> parkedBaton_;
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
// Parameterized on numCPUThreads so every case below runs twice: once with
// handlers inline on the IO thread, once dispatched to a CPU pool. The two
// configurations must be indistinguishable from the client's side.
class FastThriftE2ETest : public ::testing::TestWithParam<uint32_t> {
 protected:
  using FastClientType = apache::thrift::
      FastClient<TestFastService, thrift::ThriftClientAppAdapter>;

  bool usingCPUPool() const { return GetParam() > 0; }

  // Adjust the server between setInterface() and start().
  virtual void configureServer(ftt::FastThriftServer&) {}

  void SetUp() override {
    handler_ = std::make_shared<TestFastServiceHandler>();

    ftt::FastThriftServerConfig config;
    config.address = folly::SocketAddress("::1", 0);
    config.numIOThreads = 1;
    config.numCPUThreads = GetParam();
    // Validate request checksums and echo a response checksum. enableChecksum
    // requires enableRequestContext (the response algorithm rides the
    // per-request ThriftRequestContext).
    config.enableRequestContext = true;
    config.enableChecksum = true;

    server_ = std::make_unique<ftt::FastThriftServer>(std::move(config));
    server_->setInterface(handler_);
    configureServer(*server_);
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

  std::unique_ptr<FastClientType> createFastClient(
      thrift::ThriftClientAppAdapter** appAdapterOut = nullptr) {
    auto* evb = clientThread_->getEventBase();
    folly::Baton<> connectBaton;
    bool connected = false;

    thrift::ThriftClientAppAdapter::Ptr appAdapter(
        new thrift::ThriftClientAppAdapter(
            static_cast<uint16_t>(
                apache::thrift::protocol::T_COMPACT_PROTOCOL)));
    if (appAdapterOut != nullptr) {
      *appAdapterOut = appAdapter.get();
    }

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
              .addState<rocket::client::RocketClientStreamContexts>()
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

TEST_P(FastThriftE2ETest, ReuseAppAdapterWithNewClient) {
  using BorrowedAppAdapter =
      thrift::client::BorrowedClientAppAdapter<thrift::ThriftClientAppAdapter>;
  using ReusedFastClientType =
      apache::thrift::FastClient<TestFastService, BorrowedAppAdapter>;

  auto client = createFastClient();
  BorrowedAppAdapter::Ptr appAdapter(
      new BorrowedAppAdapter(client->getAppAdapter()));
  auto reusedClient =
      std::make_unique<ReusedFastClientType>(std::move(appAdapter));

  reusedClient->sync_ping();

  reusedClient.reset();
  destroyFastClientOnEvb(client);
}

TEST_P(FastThriftE2ETest, Ping) {
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

TEST_P(FastThriftE2ETest, Echo) {
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

TEST_P(FastThriftE2ETest, Add) {
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

TEST_P(FastThriftE2ETest, SendResponse) {
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
TEST_P(FastThriftE2ETest, ChecksumRoundTrip) {
  auto client = createFastClient();
  auto* evb = clientThread_->getEventBase();

  apache::thrift::RpcOptions opts;
  opts.setChecksum(apache::thrift::RpcOptions::Checksum::XXH3_64);

  auto result = folly::coro::blockingWait(
      folly::coro::co_withExecutor(evb, client->co_echo(opts, "checksummed")));
  EXPECT_EQ(result, "checksummed");

  destroyFastClientOnEvb(client);
}

// Many requests in flight on a *single* connection. Every one of them
// acquires and releases a DestructorGuard on the same adapter, and with a CPU
// pool those happen off the connection's EventBase. The adapter's guard
// counter is a plain uint32_t, so a lost increment or decrement here is a
// use-after-free or a leak. Meaningful under TSAN above all — that is what
// distinguishes this from the single-request cases above.
TEST_P(FastThriftE2ETest, ConcurrentRequestsOnOneConnection) {
  constexpr int kRequests = 200;

  auto client = createFastClient();
  auto* evb = clientThread_->getEventBase();

  std::vector<folly::SemiFuture<int64_t>> futures;
  futures.reserve(kRequests);
  for (int i = 0; i < kRequests; ++i) {
    futures.push_back(
        folly::coro::co_withExecutor(evb, client->co_add(i, i)).start());
  }

  for (int i = 0; i < kRequests; ++i) {
    EXPECT_EQ(std::move(futures[i]).get(), int64_t{2} * i);
  }

  destroyFastClientOnEvb(client);
}

// The handler completes on a thread that is neither the connection's
// EventBase nor the thread the method was dispatched on, so the callback —
// and with it the adapter guard and the request context's ThriftConnContext
// reference — is released from a third thread.
//
// Runs in both configurations. Ownership of the callback is unique and simply
// travels to whichever thread completes it, so there is no count to race; the
// only requirement is that destruction lands back on the EventBase, which
// holds regardless of whether a CPU pool is configured.
TEST_P(FastThriftE2ETest, HandlerCompletesOnForeignThread) {
  folly::CPUThreadPoolExecutor completionPool(2);
  handler_->setEchoCompletionExecutor(&completionPool);

  auto client = createFastClient();
  auto* evb = clientThread_->getEventBase();

  constexpr int kRequests = 50;
  // co_echo takes the message by const reference, so it must outlive the
  // deferred coroutines below — a temporary would dangle past .start().
  const std::string message = "foreign";
  std::vector<folly::SemiFuture<std::string>> futures;
  futures.reserve(kRequests);
  for (int i = 0; i < kRequests; ++i) {
    futures.push_back(
        folly::coro::co_withExecutor(evb, client->co_echo(message)).start());
  }
  for (auto& future : futures) {
    EXPECT_EQ(std::move(future).get(), "foreign");
  }

  destroyFastClientOnEvb(client);
  handler_->setEchoCompletionExecutor(nullptr);
  completionPool.join();
}

// A request outlives its connection: the handler parks the callback, the
// client goes away, and only then is the callback completed — from a foreign
// thread. The adapter has to survive until that straggler is released, and
// the write it attempts has to be dropped rather than touching a dead
// pipeline. This is the teardown ordering the whole lifetime design exists
// for, and it must hold whether or not a CPU pool is in play.
TEST_P(FastThriftE2ETest, StragglerCompletesAfterConnectionClosed) {
  auto client = createFastClient();
  auto* evb = clientThread_->getEventBase();

  handler_->parkNextEcho();

  // Fire and forget: the server parks this one, so it never completes. The
  // message must outlive the deferred coroutine — co_echo binds it by
  // reference.
  const std::string message = "parked";
  auto parked =
      folly::coro::co_withExecutor(evb, client->co_echo(message)).start();

  // Wait for the server to actually park it before tearing anything down,
  // otherwise the request may not have reached the handler yet.
  ASSERT_TRUE(handler_->waitParkedEcho(std::chrono::seconds(10)));

  destroyFastClientOnEvb(client);

  // Straggler completes on a foreign thread with the connection already gone.
  std::thread releaser([&] { EXPECT_TRUE(handler_->releaseParkedEcho()); });
  releaser.join();

  // The response cannot arrive — the connection is gone — so this either
  // stays unfulfilled or fails. Either is fine; what matters is that the
  // server did not crash and the adapter was torn down cleanly, which
  // ASAN/TSAN check when the fixture stops the server.
  std::move(parked).wait(std::chrono::seconds(1));
}

// @cpp.ProcessInEbThreadUnsafe is resolved at IDL compile time: the generated
// dispatcher for an annotated method has no offload path at all, so it stays
// on the connection's EventBase even when the server owns a CPU pool. An
// unannotated method on the same service must still offload, which is what
// makes this an A/B rather than an assertion that the pool exists.
TEST_P(FastThriftE2ETest, EventBaseMethodsBypassCPUPool) {
  auto client = createFastClient();
  auto* evb = clientThread_->getEventBase();

  EXPECT_TRUE(
      folly::coro::blockingWait(
          folly::coro::co_withExecutor(evb, client->co_ebRanOnEventBase())));

  EXPECT_EQ(
      folly::coro::blockingWait(
          folly::coro::co_withExecutor(evb, client->co_ranOnEventBase())),
      !usingCPUPool());

  destroyFastClientOnEvb(client);
}

namespace {
constexpr const char* kRejectionReason = "cpu queue refused the task";

// Refuses every task, standing in for a bounded CPU queue at its limit.
class RejectingExecutor : public folly::Executor {
 public:
  void add(folly::Func) override {
    throw folly::QueueFullException(kRejectionReason);
  }
};
} // namespace

// A CPU executor that refuses the request still has to answer the client, and
// the answer has to name the refusal. The dispatcher hands the task a raw
// pointer and keeps the owning handle until the enqueue commits, so a
// rejection leaves the callback completable rather than unwinding through the
// task — which would report only that the callback went uncompleted.
class FastThriftRejectedDispatchTest : public FastThriftE2ETest {
 protected:
  void configureServer(ftt::FastThriftServer& server) override {
    server.setCPUExecutor(folly::getKeepAliveToken(&executor_));
  }

 private:
  RejectingExecutor executor_;
};

TEST_P(FastThriftRejectedDispatchTest, RejectedEnqueueTellsClientWhy) {
  auto client = createFastClient();
  auto* evb = clientThread_->getEventBase();

  std::string message;
  try {
    folly::coro::blockingWait(
        folly::coro::co_withExecutor(evb, client->co_add(1, 2)));
    FAIL() << "a refused request must not look like success";
  } catch (const std::exception& ex) {
    message = ex.what();
  }

  EXPECT_NE(message.find(kRejectionReason), std::string::npos) << message;
  EXPECT_EQ(message.find("not completed"), std::string::npos) << message;

  destroyFastClientOnEvb(client);
}

INSTANTIATE_TEST_SUITE_P(
    RejectingCPUExecutor,
    FastThriftRejectedDispatchTest,
    ::testing::Values(uint32_t{0}),
    [](const auto&) { return "RejectingCPUExecutor"; });

INSTANTIATE_TEST_SUITE_P(
    Inline, FastThriftE2ETest, ::testing::Values(uint32_t{0}), [](const auto&) {
      return "InlineOnIOThread";
    });

INSTANTIATE_TEST_SUITE_P(
    CPUPool,
    FastThriftE2ETest,
    ::testing::Values(uint32_t{4}),
    [](const auto&) { return "CPUThreadPool"; });

} // namespace apache::thrift::fast_thrift::thrift::test
