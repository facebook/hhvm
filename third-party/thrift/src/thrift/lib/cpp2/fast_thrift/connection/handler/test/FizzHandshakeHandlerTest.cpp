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

#include <sys/socket.h>
#include <array>
#include <chrono>

#include <gtest/gtest.h>

#include <fizz/client/AsyncFizzClient.h>
#include <fizz/client/FizzClientContext.h>
#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncSocketException.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/net/NetworkSocket.h>
#include <folly/synchronization/Baton.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/connection/SocketOptions.h>
#include <thrift/lib/cpp2/fast_thrift/connection/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/connection/endpoint/ConnectionListener.h>
#include <thrift/lib/cpp2/fast_thrift/connection/handler/FizzHandshakeHandler.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerCertConfig.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerContextBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/security/ThriftTlsConfig.h>
#include <thrift/lib/cpp2/fast_thrift/security/test/TestCert.h>

namespace apache::thrift::fast_thrift::connection::handler {

using channel_pipeline::PipelineBuilder;
using channel_pipeline::PipelineImpl;
using channel_pipeline::Result;
using channel_pipeline::SimpleBufferAllocator;
using channel_pipeline::TypeErasedBox;
using channel_pipeline::test::MockTailHandler;

HANDLER_TAG(fizz_handler);

namespace {

struct SocketPair {
  folly::NetworkSocket server;
  folly::NetworkSocket client;
};

SocketPair makeSocketPair() {
  std::array<int, 2> fds{};
  PCHECK(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds.data()) == 0);
  return {folly::NetworkSocket(fds[0]), folly::NetworkSocket(fds[1])};
}

// Drives a fizz client handshake on `fd`. Used to satisfy the server-side
// handshake we're really testing.
class TestFizzClient
    : private fizz::client::AsyncFizzClient::HandshakeCallback {
 public:
  using DoneCallback = folly::Function<void(folly::exception_wrapper) noexcept>;

  TestFizzClient(folly::EventBase* evb, folly::NetworkSocket fd) {
    auto sock = folly::AsyncSocket::newSocket(evb, fd);
    auto ctx = std::make_shared<fizz::client::FizzClientContext>();
    client_.reset(new fizz::client::AsyncFizzClient(std::move(sock), ctx));
  }

  void start(DoneCallback done) {
    done_ = std::move(done);
    client_->connect(
        this,
        /*verifier=*/nullptr,
        /*sni=*/folly::none,
        /*pskIdentity=*/folly::none,
        /*echConfigs=*/folly::none);
  }

 private:
  void fizzHandshakeSuccess(fizz::client::AsyncFizzClient*) noexcept override {
    if (done_) {
      done_(folly::exception_wrapper());
    }
  }
  void fizzHandshakeError(
      fizz::client::AsyncFizzClient*,
      folly::exception_wrapper ex) noexcept override {
    if (done_) {
      done_(std::move(ex));
    }
  }

  fizz::client::AsyncFizzClient::UniquePtr client_;
  DoneCallback done_;
};

} // namespace

class FizzHandshakeHandlerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    evbThread_ = std::make_unique<folly::ScopedEventBaseThread>();
    evb_ = evbThread_->getEventBase();

    security::FizzServerCertConfig cfg;
    auto cert = security::test::makeTestCert();
    cfg.certPem = cert.certPem;
    cfg.keyPem = cert.keyPem;
    cfg.clientAuth = fizz::server::ClientAuthMode::None;
    serverCtx_ =
        security::buildFizzServerContext(cfg, security::ThriftTlsConfig{})
            .fizzContext;
  }

  void TearDown() override { evbThread_.reset(); }

  std::unique_ptr<folly::ScopedEventBaseThread> evbThread_;
  folly::EventBase* evb_{nullptr};
  std::shared_ptr<const fizz::server::FizzServerContext> serverCtx_;
  SimpleBufferAllocator allocator_;
  MockTailHandler tail_;
};

// Drive a real client through ConnectionListener → FizzHandshakeHandler →
// MockTail. The tail should receive a ConnectionMessage whose transport is
// the upgraded AsyncFizzServer, not the original plain AsyncSocket.
TEST_F(FizzHandshakeHandlerTest, UpgradesTransportOnSuccess) {
  // Build pipeline.
  ConnectionListener::Ptr listener(new ConnectionListener(
      evb_,
      folly::SocketAddress("::1", 0),
      SocketOptions{},
      /*enableReusePortBpfSpread=*/false));
  auto pipeline = PipelineBuilder<
                      ConnectionListener,
                      MockTailHandler,
                      SimpleBufferAllocator>()
                      .setEventBase(evb_)
                      .setHead(listener.get())
                      .setTail(&tail_)
                      .setAllocator(&allocator_)
                      .addNextDuplex<FizzHandshakeHandler>(
                          fizz_handler_tag,
                          serverCtx_,
                          /*thriftParams=*/nullptr,
                          std::chrono::seconds{5})
                      .build();
  listener->setPipeline(pipeline.get());
  evb_->runInEventBaseThreadAndWait([&] { pipeline->activate(); });

  // Capture upgraded message on the tail.
  ConnectionMessage captured;
  folly::Baton<> tailDone;
  tail_.setOnReadCallback([&](TypeErasedBox&& box) noexcept {
    captured = box.take<ConnectionMessage>();
    tailDone.post();
    return Result::Success;
  });

  // Drive the handshake.
  auto sp = makeSocketPair();
  folly::SocketAddress clientAddr("127.0.0.1", 5001);

  std::unique_ptr<TestFizzClient> client;
  folly::Baton<> clientDone;
  folly::exception_wrapper clientEx;
  evb_->runInEventBaseThreadAndWait([&] {
    // Feed the server fd into the pipeline via the listener.
    listener->connectionAccepted(
        sp.server,
        clientAddr,
        folly::AsyncServerSocket::AcceptCallback::AcceptInfo{});

    client = std::make_unique<TestFizzClient>(evb_, sp.client);
    client->start([&](folly::exception_wrapper ex) noexcept {
      clientEx = std::move(ex);
      clientDone.post();
    });
  });

  ASSERT_TRUE(tailDone.try_wait_for(std::chrono::seconds{5}));
  ASSERT_TRUE(clientDone.try_wait_for(std::chrono::seconds{5}));

  EXPECT_FALSE(clientEx) << clientEx.what();
  EXPECT_NE(captured.transport.get(), nullptr);
  EXPECT_EQ(captured.clientAddr, clientAddr);
  // Upgraded transport is the AsyncFizzServer, not a plain AsyncSocket.
  EXPECT_EQ(
      dynamic_cast<folly::AsyncSocket*>(captured.transport.get()), nullptr);

  // Tear down everything on the EVB.
  evb_->runInEventBaseThreadAndWait([&] {
    captured.transport.reset();
    client.reset();
    listener->resetPipeline();
    pipeline.reset();
    listener.reset();
  });
}

// When the client sends garbage instead of a ClientHello the handshake fails
// and the connection is dropped (tail never sees the message).
TEST_F(FizzHandshakeHandlerTest, DropsConnectionOnHandshakeFailure) {
  ConnectionListener::Ptr listener(new ConnectionListener(
      evb_,
      folly::SocketAddress("::1", 0),
      SocketOptions{},
      /*enableReusePortBpfSpread=*/false));
  auto pipeline = PipelineBuilder<
                      ConnectionListener,
                      MockTailHandler,
                      SimpleBufferAllocator>()
                      .setEventBase(evb_)
                      .setHead(listener.get())
                      .setTail(&tail_)
                      .setAllocator(&allocator_)
                      .addNextDuplex<FizzHandshakeHandler>(
                          fizz_handler_tag,
                          serverCtx_,
                          /*thriftParams=*/nullptr,
                          // Short timeout so the test doesn't have to wait 30s.
                          std::chrono::milliseconds{200})
                      .build();
  listener->setPipeline(pipeline.get());
  evb_->runInEventBaseThreadAndWait([&] { pipeline->activate(); });

  auto sp = makeSocketPair();
  folly::SocketAddress clientAddr("127.0.0.1", 5002);

  // ReadCallback on the client side that posts a baton when the server
  // drops its end of the socket — the deterministic signal that the
  // server handler ran its failure path.
  struct EofWatcher : folly::AsyncReader::ReadCallback {
    folly::Baton<> closed;
    char scratch[64]{};
    void getReadBuffer(void** bufReturn, size_t* lenReturn) override {
      *bufReturn = scratch;
      *lenReturn = sizeof(scratch);
    }
    void readDataAvailable(size_t /*len*/) noexcept override {}
    void readEOF() noexcept override { closed.post(); }
    void readErr(const folly::AsyncSocketException& /*ex*/) noexcept override {
      closed.post();
    }
  };
  EofWatcher watcher;
  folly::AsyncSocket::UniquePtr client;
  static constexpr std::string_view kGarbage{"NOT_A_TLS_HANDSHAKE\n"};

  evb_->runInEventBaseThreadAndWait([&] {
    listener->connectionAccepted(
        sp.server,
        clientAddr,
        folly::AsyncServerSocket::AcceptCallback::AcceptInfo{});
    client.reset(new folly::AsyncSocket(evb_, sp.client));
    client->write(/*callback=*/nullptr, kGarbage.data(), kGarbage.size());
    client->setReadCB(&watcher);
  });

  ASSERT_TRUE(watcher.closed.try_wait_for(std::chrono::seconds{5}));

  EXPECT_EQ(tail_.readCount(), 0)
      << "tail must not see the message when handshake fails";

  evb_->runInEventBaseThreadAndWait([&] {
    client.reset();
    listener->resetPipeline();
    pipeline.reset();
    listener.reset();
  });
}

} // namespace apache::thrift::fast_thrift::connection::handler
