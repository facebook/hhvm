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
#include <vector>

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
  channel_pipeline::Result onWrite(
      channel_pipeline::detail::ContextImpl&,
      channel_pipeline::TypeErasedBox&&) noexcept {
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
      channel_pipeline::detail::ContextImpl&,
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

  // Replace the params the pipeline will snapshot with ones carrying no fizz
  // context — a server configured for TLS that has nothing to handshake with.
  // Must be called before buildPipeline, which captures the observer.
  void useTlsParamsWithoutFizzContext() {
    tlsParams_ = std::make_shared<const fts::TLSParams>();
    tlsParamsObservable_ = std::make_unique<folly::observer::SimpleObservable<
        std::shared_ptr<const fts::TLSParams>>>(tlsParams_);
  }

  // Build outer pipeline NoopHead → ConnectionTLSHandler → CapturingTail.
  // maxPending of zero leaves the pending-connection limit off.
  void buildPipeline(
      fts::SSLPolicy policy,
      CapturingTail::OnRead onRead,
      uint32_t maxPending) {
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
              &allocator_,
              maxPending);
      pipeline_ = builder.build();
      pipeline_->activate();
    });
  }

  // Feed a server-side socket into the pipeline. Admission is synchronous, so
  // the returned Result and any pending-count change are settled on return.
  channel_pipeline::Result feedSocket(folly::NetworkSocket fd) {
    auto result = channel_pipeline::Result::Success;
    evb_->runInEventBaseThreadAndWait([&] {
      auto sock = folly::AsyncSocket::newSocket(evb_, fd);
      conn::ConnectionMessage msg{
          .transport = folly::AsyncTransport::UniquePtr(sock.release()),
          .clientAddr = folly::SocketAddress{"127.0.0.1", 0},
          .peerSecurity = nullptr,
      };
      result =
          pipeline_->fireRead(channel_pipeline::erase_and_box(std::move(msg)));
    });
    return result;
  }

  // Keep offering connections until one is admitted.
  //
  // A parked connection is given up on asynchronous terminal paths — handshake
  // failure, peek timeout — that deliberately notify nobody, so there is no
  // event to wait on. The peer's socket close is not a usable substitute
  // either: fizz can close on error before the stage lets go of the
  // connection. Retrying the accept is the observation, and each attempt round
  // trips through the EventBase, yielding to the socket event that ends the
  // failing handshake.
  bool waitForAdmission() {
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds{5};
    do {
      auto sp = makeSocketPair();
      const auto result = feedSocket(sp.server);
      ::close(sp.client.toFd());
      if (result == channel_pipeline::Result::Success) {
        return true;
      }
    } while (std::chrono::steady_clock::now() < deadline);
    return false;
  }

  // A peer that opens a connection and then says nothing occupies whichever
  // stage parked it until it gives up. Once the limit is reached the next
  // connection is refused rather than queued behind it.
  void expectSilentPeerRefusedAtLimit(fts::SSLPolicy policy) {
    buildPipeline(
        policy, [](conn::ConnectionMessage&&) noexcept {}, /*maxPending=*/1);

    auto first = makeSocketPair();
    EXPECT_EQ(feedSocket(first.server), channel_pipeline::Result::Success);

    auto second = makeSocketPair();
    EXPECT_EQ(feedSocket(second.server), channel_pipeline::Result::Error);

    ::close(first.client.toFd());
    ::close(second.client.toFd());
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

// A server configured for TLS with no fizz context cannot handshake, so the
// connection is dropped on admission. It must be reported as such: returning
// Success would tell the caller a connection it no longer holds is still on
// its way through the pipeline, and nothing would ever arrive at the tail.
TEST_F(TLSPipelineIntegrationTest, MissingFizzContextReportsError) {
  useTlsParamsWithoutFizzContext();

  bool reachedTail = false;
  buildPipeline(
      fts::SSLPolicy::REQUIRED,
      [&](conn::ConnectionMessage&&) noexcept { reachedTail = true; },
      /*maxPending=*/0);

  auto sp = makeSocketPair();
  EXPECT_EQ(feedSocket(sp.server), channel_pipeline::Result::Error);
  EXPECT_FALSE(reachedTail);

  ::close(sp.client.toFd());
}

// REQUIRED + TLS: pipeline = FizzHandshakeHandler → StopTLSV1Handler (no-op).
// Handshake succeeds → tail receives AsyncFizzServer, and what the peer proved
// rides out alongside it.
TEST_F(TLSPipelineIntegrationTest, RequiredTLSHandshakeSucceeds) {
  folly::Baton<> emitted;
  folly::AsyncTransport::UniquePtr transport;
  std::shared_ptr<const conn::PeerSecurityInfo> peerSecurity;

  buildPipeline(
      fts::SSLPolicy::REQUIRED,
      [&](conn::ConnectionMessage&& m) noexcept {
        transport = std::move(m.transport);
        peerSecurity = std::move(m.peerSecurity);
        emitted.post();
      },
      /*maxPending=*/0);

  auto sp = makeSocketPair();
  ASSERT_EQ(feedSocket(sp.server), channel_pipeline::Result::Success);

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
  // Snapshotted at handshake completion, so it rides the message the outer
  // pipeline sees rather than being asked of the transport later.
  ASSERT_NE(peerSecurity, nullptr);
  EXPECT_FALSE(peerSecurity->securityProtocol.empty());

  evb_->runInEventBaseThreadAndWait([&] {
    transport.reset();
    client.reset();
  });
}

// PERMITTED + plaintext: classifier peeks, sees non-TLS, fires to tail.
// Bypasses Fizz + StopTLS → tail receives a plaintext AsyncSocket, and nothing
// was proved about the peer.
TEST_F(TLSPipelineIntegrationTest, PermittedPlaintextBypassesHandshake) {
  folly::Baton<> emitted;
  folly::AsyncTransport::UniquePtr transport;
  std::shared_ptr<const conn::PeerSecurityInfo> peerSecurity;

  buildPipeline(
      fts::SSLPolicy::PERMITTED,
      [&](conn::ConnectionMessage&& m) noexcept {
        transport = std::move(m.transport);
        peerSecurity = std::move(m.peerSecurity);
        emitted.post();
      },
      /*maxPending=*/0);

  auto sp = makeSocketPair();
  ASSERT_EQ(feedSocket(sp.server), channel_pipeline::Result::Success);

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
  EXPECT_EQ(peerSecurity, nullptr);

  evb_->runInEventBaseThreadAndWait([&] { transport.reset(); });
  ::close(sp.client.toFd());
}

// PERMITTED + TLS: classifier peeks, sees TLS, fires through Fizz handshake.
// Handshake completes → tail receives AsyncFizzServer.
TEST_F(TLSPipelineIntegrationTest, PermittedTLSPathCompletesHandshake) {
  folly::Baton<> emitted;
  folly::AsyncTransport::UniquePtr transport;

  buildPipeline(
      fts::SSLPolicy::PERMITTED,
      [&](conn::ConnectionMessage&& m) noexcept {
        transport = std::move(m.transport);
        emitted.post();
      },
      /*maxPending=*/0);

  auto sp = makeSocketPair();
  ASSERT_EQ(feedSocket(sp.server), channel_pipeline::Result::Success);

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
      [&](conn::ConnectionMessage&&) noexcept { emitted.post(); },
      /*maxPending=*/0);

  auto sp = makeSocketPair();
  ASSERT_EQ(feedSocket(sp.server), channel_pipeline::Result::Success);

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

// Under REQUIRED a silent peer sits in the handshake stage.
TEST_F(TLSPipelineIntegrationTest, RefusesConnectionsBeyondPendingLimit) {
  expectSilentPeerRefusedAtLimit(fts::SSLPolicy::REQUIRED);
}

// Capacity must come back when a handshake fails, not only when it succeeds.
// Failures are absorbed inside FizzHandshakeHandler without any downstream
// notification, so a limit that relied on an explicit release would strand
// capacity on exactly the input an attacker controls.
TEST_F(TLSPipelineIntegrationTest, FailedHandshakeFreesPendingCapacity) {
  buildPipeline(
      fts::SSLPolicy::REQUIRED,
      [&](conn::ConnectionMessage&&) noexcept {},
      /*maxPending=*/1);

  auto doomed = makeSocketPair();
  ASSERT_EQ(feedSocket(doomed.server), channel_pipeline::Result::Success);

  auto spare = makeSocketPair();
  ASSERT_EQ(feedSocket(spare.server), channel_pipeline::Result::Error);
  ::close(spare.client.toFd());

  constexpr std::string_view garbage =
      "definitely not a TLS ClientHello frame ........";
  ASSERT_EQ(
      ::write(doomed.client.toFd(), garbage.data(), garbage.size()),
      ssize_t(garbage.size()));

  EXPECT_TRUE(waitForAdmission());

  ::close(doomed.client.toFd());
}

// Under PERMITTED it sits one stage earlier, in the classifier, waiting for
// enough bytes to classify — the same exposure, capped the same way.
TEST_F(TLSPipelineIntegrationTest, PeekAwaitingClassificationHoldsPendingSlot) {
  expectSilentPeerRefusedAtLimit(fts::SSLPolicy::PERMITTED);
}

// The classifier releases its slot on a different path from the handshake
// stage — its helper reports terminal before the callback runs rather than
// after — and peek failures are absorbed with no downstream notification. So
// the release contract needs proving on this stage too, not just on fizz.
TEST_F(TLSPipelineIntegrationTest, FailedPeekFreesPendingCapacity) {
  buildPipeline(
      fts::SSLPolicy::PERMITTED,
      [&](conn::ConnectionMessage&&) noexcept {},
      /*maxPending=*/1);

  auto doomed = makeSocketPair();
  ASSERT_EQ(feedSocket(doomed.server), channel_pipeline::Result::Success);

  auto spare = makeSocketPair();
  ASSERT_EQ(feedSocket(spare.server), channel_pipeline::Result::Error);
  ::close(spare.client.toFd());

  // EOF before the classifier has its 9-byte prefix ends the peek in error.
  ::close(doomed.client.toFd());

  EXPECT_TRUE(waitForAdmission());
}

// A zero limit is the off switch: silent peers pile up and none are refused.
TEST_F(TLSPipelineIntegrationTest, ZeroLimitLeavesAcceptsUnbounded) {
  buildPipeline(
      fts::SSLPolicy::REQUIRED,
      [&](conn::ConnectionMessage&&) noexcept {},
      /*maxPending=*/0);

  constexpr size_t kConnections = 4;
  std::vector<SocketPair> pairs;
  for (size_t i = 0; i < kConnections; ++i) {
    pairs.push_back(makeSocketPair());
    EXPECT_EQ(
        feedSocket(pairs.back().server), channel_pipeline::Result::Success)
        << "connection " << i;
  }

  for (auto& pair : pairs) {
    ::close(pair.client.toFd());
  }
}

} // namespace apache::thrift::fast_thrift::connection::security::test

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  folly::Init init(&argc, &argv);
  return RUN_ALL_TESTS();
}
