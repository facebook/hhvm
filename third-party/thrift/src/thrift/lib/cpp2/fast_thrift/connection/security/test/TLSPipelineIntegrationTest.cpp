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

// End-to-end tests for the inner TLS pipeline owned by ConnectionTLSHandler.
// Drives sockets through the full chain (classifier → fizz handshake →
// stoptls) and verifies the ConnectionMessage emitted to the outer pipeline.

#include <sys/socket.h>
#include <array>
#include <chrono>
#include <memory>
#include <string_view>

#include <gtest/gtest.h>

#include <fizz/client/AsyncFizzClient.h>
#include <fizz/client/FizzClientContext.h>
#include <fizz/server/AsyncFizzServer.h>
#include <folly/ExceptionWrapper.h>
#include <folly/Function.h>
#include <folly/SocketAddress.h>
#include <folly/init/Init.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/net/NetworkSocket.h>
#include <folly/observer/SimpleObservable.h>
#include <folly/synchronization/Baton.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/BufferAllocator.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineImpl.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/connection/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/connection/handler/ConnectionTLSHandler.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerCertConfig.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerContextBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/security/SSLPolicy.h>
#include <thrift/lib/cpp2/fast_thrift/security/ThriftTlsConfig.h>
#include <thrift/lib/cpp2/fast_thrift/security/test/TestCert.h>

namespace apache::thrift::fast_thrift::connection::security::test {

namespace fts = ::apache::thrift::fast_thrift::security;
namespace conn = ::apache::thrift::fast_thrift::connection;

// Bring nested test namespace into scope so we can call
// `apache::thrift::fast_thrift::security::test::makeTestCert()` as
// `fts_test::makeTestCert()` without colliding with this file's own
// `connection::security::test` namespace.
namespace fts_test = ::apache::thrift::fast_thrift::security::test;

namespace {

// === Outer-pipeline endpoint mocks ===

// Head: TLS pipeline produces no outbound, so any onWrite here is a bug.
class NoopHead {
 public:
  channel_pipeline::Result onWrite(channel_pipeline::TypeErasedBox&&) noexcept {
    return channel_pipeline::Result::Success;
  }
  void onReadReady() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
};

// Tail: hands the emitted ConnectionMessage to a per-test callback.
class CapturingTail {
 public:
  using OnRead = folly::Function<void(conn::ConnectionMessage&&) noexcept>;

  explicit CapturingTail(OnRead onRead) noexcept : onRead_(std::move(onRead)) {}

  channel_pipeline::Result onRead(
      channel_pipeline::TypeErasedBox&& msg) noexcept {
    auto m = msg.take<conn::ConnectionMessage>();
    if (onRead_) {
      onRead_(std::move(m));
    }
    return channel_pipeline::Result::Success;
  }
  void onException(folly::exception_wrapper&&) noexcept {}
  void onWriteReady() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}

 private:
  OnRead onRead_;
};

// === Client-side fizz driver ===

struct SocketPair {
  folly::NetworkSocket server;
  folly::NetworkSocket client;
};

SocketPair makeSocketPair() {
  std::array<int, 2> fds{};
  PCHECK(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds.data()) == 0);
  return {folly::NetworkSocket(fds[0]), folly::NetworkSocket(fds[1])};
}

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
    client_->connect(this, nullptr, folly::none, folly::none, folly::none);
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

HANDLER_TAG(tls_test_handler);

} // namespace

class TLSPipelineIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    evbThread_ = std::make_unique<folly::ScopedEventBaseThread>();
    evb_ = evbThread_->getEventBase();

    fts::FizzServerCertConfig cfg;
    auto cert = fts_test::makeTestCert();
    cfg.certPem = cert.certPem;
    cfg.keyPem = cert.keyPem;
    cfg.clientAuth = fizz::server::ClientAuthMode::None;

    tlsParams_ = std::make_shared<const fts::TLSParams>(
        fts::buildTLSParams(cfg, fts::ThriftTlsConfig{}));
    tlsParamsObservable_ = std::make_unique<folly::observer::SimpleObservable<
        std::shared_ptr<const fts::TLSParams>>>(tlsParams_);
  }

  void TearDown() override {
    if (pipeline_) {
      evb_->runInEventBaseThreadAndWait([&] {
        pipeline_->deactivate();
        pipeline_.reset();
      });
    }
    evbThread_.reset();
  }

  // Build outer pipeline NoopHead → ConnectionTLSHandler → CapturingTail.
  void buildPipeline(fts::SSLPolicy policy, CapturingTail::OnRead onRead) {
    head_ = std::make_unique<NoopHead>();
    tail_ = std::make_unique<CapturingTail>(std::move(onRead));

    evb_->runInEventBaseThreadAndWait([&] {
      channel_pipeline::PipelineBuilder<
          NoopHead,
          CapturingTail,
          channel_pipeline::SimpleBufferAllocator>
          builder;
      builder.setEventBase(evb_)
          .setHead(head_.get())
          .setTail(tail_.get())
          .setAllocator(&allocator_)
          .addNextDuplex<conn::handler::ConnectionTLSHandler>(
              tls_test_handler_tag,
              *evb_,
              policy,
              tlsParamsObservable_->getObserver(),
              &allocator_);
      pipeline_ = builder.build();
      pipeline_->activate();
    });
  }

  // Feed a server-side socket into the pipeline.
  void feedSocket(folly::NetworkSocket fd) {
    evb_->runInEventBaseThreadAndWait([&] {
      auto sock = folly::AsyncSocket::newSocket(evb_, fd);
      conn::ConnectionMessage msg{
          .transport = folly::AsyncTransport::UniquePtr(sock.release()),
          .clientAddr = folly::SocketAddress{"127.0.0.1", 0},
      };
      (void)pipeline_->fireRead(
          channel_pipeline::erase_and_box(std::move(msg)));
    });
  }

  std::unique_ptr<folly::ScopedEventBaseThread> evbThread_;
  folly::EventBase* evb_{nullptr};
  std::shared_ptr<const fts::TLSParams> tlsParams_;
  std::unique_ptr<
      folly::observer::SimpleObservable<std::shared_ptr<const fts::TLSParams>>>
      tlsParamsObservable_;
  channel_pipeline::SimpleBufferAllocator allocator_;
  std::unique_ptr<NoopHead> head_;
  std::unique_ptr<CapturingTail> tail_;
  channel_pipeline::PipelineImpl::Ptr pipeline_;
};

// REQUIRED + TLS: pipeline = FizzHandshakeHandler → StopTLSV1Handler (no-op).
// Handshake succeeds → tail receives AsyncFizzServer.
TEST_F(TLSPipelineIntegrationTest, RequiredTLSHandshakeSucceeds) {
  folly::Baton<> emitted;
  folly::AsyncTransport::UniquePtr transport;

  buildPipeline(
      fts::SSLPolicy::REQUIRED, [&](conn::ConnectionMessage&& m) noexcept {
        transport = std::move(m.transport);
        emitted.post();
      });

  auto sp = makeSocketPair();
  feedSocket(sp.server);

  std::unique_ptr<TestFizzClient> client;
  folly::Baton<> clientDone;
  folly::exception_wrapper clientEx;
  evb_->runInEventBaseThreadAndWait([&] {
    client = std::make_unique<TestFizzClient>(evb_, sp.client);
    client->start([&](folly::exception_wrapper ex) noexcept {
      clientEx = std::move(ex);
      clientDone.post();
    });
  });

  ASSERT_TRUE(emitted.try_wait_for(std::chrono::seconds{5}));
  ASSERT_TRUE(clientDone.try_wait_for(std::chrono::seconds{5}));
  EXPECT_FALSE(clientEx) << clientEx.what();
  ASSERT_NE(transport, nullptr);
  EXPECT_NE(
      dynamic_cast<fizz::server::AsyncFizzServer*>(transport.get()), nullptr);

  evb_->runInEventBaseThreadAndWait([&] {
    transport.reset();
    client.reset();
  });
}

// PERMITTED + plaintext: classifier peeks, sees non-TLS, fires to tail.
// Bypasses Fizz + StopTLS → tail receives a plaintext AsyncSocket.
TEST_F(TLSPipelineIntegrationTest, PermittedPlaintextBypassesHandshake) {
  folly::Baton<> emitted;
  folly::AsyncTransport::UniquePtr transport;

  buildPipeline(
      fts::SSLPolicy::PERMITTED, [&](conn::ConnectionMessage&& m) noexcept {
        transport = std::move(m.transport);
        emitted.post();
      });

  auto sp = makeSocketPair();
  feedSocket(sp.server);

  // Write 9+ bytes of non-TLS — enough for the classifier to make a decision.
  constexpr std::string_view garbage =
      "definitely not a TLS ClientHello frame ........";
  ASSERT_EQ(
      ::write(sp.client.toFd(), garbage.data(), garbage.size()),
      ssize_t(garbage.size()));

  ASSERT_TRUE(emitted.try_wait_for(std::chrono::seconds{5}));
  ASSERT_NE(transport, nullptr);
  // Plaintext bypass: transport stays AsyncSocket; never wrapped in fizz.
  EXPECT_EQ(
      dynamic_cast<fizz::server::AsyncFizzServer*>(transport.get()), nullptr);
  EXPECT_NE(dynamic_cast<folly::AsyncSocket*>(transport.get()), nullptr);

  evb_->runInEventBaseThreadAndWait([&] { transport.reset(); });
  ::close(sp.client.toFd());
}

// PERMITTED + TLS: classifier peeks, sees TLS, fires through Fizz handshake.
// Handshake completes → tail receives AsyncFizzServer.
TEST_F(TLSPipelineIntegrationTest, PermittedTLSPathCompletesHandshake) {
  folly::Baton<> emitted;
  folly::AsyncTransport::UniquePtr transport;

  buildPipeline(
      fts::SSLPolicy::PERMITTED, [&](conn::ConnectionMessage&& m) noexcept {
        transport = std::move(m.transport);
        emitted.post();
      });

  auto sp = makeSocketPair();
  feedSocket(sp.server);

  std::unique_ptr<TestFizzClient> client;
  folly::Baton<> clientDone;
  folly::exception_wrapper clientEx;
  evb_->runInEventBaseThreadAndWait([&] {
    client = std::make_unique<TestFizzClient>(evb_, sp.client);
    client->start([&](folly::exception_wrapper ex) noexcept {
      clientEx = std::move(ex);
      clientDone.post();
    });
  });

  ASSERT_TRUE(emitted.try_wait_for(std::chrono::seconds{5}));
  ASSERT_TRUE(clientDone.try_wait_for(std::chrono::seconds{5}));
  EXPECT_FALSE(clientEx) << clientEx.what();
  ASSERT_NE(transport, nullptr);
  EXPECT_NE(
      dynamic_cast<fizz::server::AsyncFizzServer*>(transport.get()), nullptr);

  evb_->runInEventBaseThreadAndWait([&] {
    transport.reset();
    client.reset();
  });
}

// REQUIRED + handshake garbage: no message reaches the tail; connection
// is dropped at the FizzHandshakeHandler level.
TEST_F(TLSPipelineIntegrationTest, RequiredGarbageInputDropsConnection) {
  folly::Baton<> emitted;
  buildPipeline(
      fts::SSLPolicy::REQUIRED,
      [&](conn::ConnectionMessage&&) noexcept { emitted.post(); });

  auto sp = makeSocketPair();
  feedSocket(sp.server);

  // Garbage that doesn't parse as a TLS ClientHello.
  constexpr std::string_view garbage =
      "definitely not a TLS ClientHello frame ........";
  ASSERT_EQ(
      ::write(sp.client.toFd(), garbage.data(), garbage.size()),
      ssize_t(garbage.size()));

  // Tail should never fire — handshake fails inside FizzHandshakeHandler
  // and the message is absorbed.
  EXPECT_FALSE(emitted.try_wait_for(std::chrono::milliseconds{500}));

  ::close(sp.client.toFd());
}

} // namespace apache::thrift::fast_thrift::connection::security::test

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  folly::Init init(&argc, &argv);
  return RUN_ALL_TESTS();
}
