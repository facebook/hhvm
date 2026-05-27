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

#include <thrift/lib/cpp2/fast_thrift/security/FizzHandshakeHelper.h>

#include <sys/socket.h>
#include <array>
#include <chrono>
#include <memory>
#include <string_view>

#include <gtest/gtest.h>

#include <fizz/client/AsyncFizzClient.h>
#include <fizz/client/FizzClientContext.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncSocketException.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <folly/synchronization/Baton.h>

#include <thrift/lib/cpp2/fast_thrift/security/FizzServerCertConfig.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerContextBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/security/ThriftTlsConfig.h>
#include <thrift/lib/cpp2/fast_thrift/security/test/TestCert.h>

namespace apache::thrift::fast_thrift::security::test {

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

// Drives a fizz client handshake on `fd`. Records success/error via callback.
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
  void fizzHandshakeSuccess(
      fizz::client::AsyncFizzClient* /*transport*/) noexcept override {
    if (done_) {
      done_(folly::exception_wrapper());
    }
  }

  void fizzHandshakeError(
      fizz::client::AsyncFizzClient* /*transport*/,
      folly::exception_wrapper ex) noexcept override {
    if (done_) {
      done_(std::move(ex));
    }
  }

  fizz::client::AsyncFizzClient::UniquePtr client_;
  DoneCallback done_;
};

} // namespace

class FizzHandshakeHelperTest : public ::testing::Test {
 protected:
  void SetUp() override {
    evbThread_ = std::make_unique<folly::ScopedEventBaseThread>();
    evb_ = evbThread_->getEventBase();

    FizzServerCertConfig cfg;
    auto cert = makeTestCert();
    cfg.certPem = cert.certPem;
    cfg.keyPem = cert.keyPem;
    // Skip mTLS for handshake-mechanics tests; we cover client auth via the
    // ContextBuilder tests and end-to-end in the integration test.
    cfg.clientAuth = fizz::server::ClientAuthMode::None;
    serverCtx_ = buildFizzServerContext(cfg, ThriftTlsConfig{}).fizzContext;
  }

  void TearDown() override { evbThread_.reset(); }

  std::unique_ptr<folly::ScopedEventBaseThread> evbThread_;
  folly::EventBase* evb_{nullptr};
  std::shared_ptr<const fizz::server::FizzServerContext> serverCtx_;
  // Direct ownership of the in-flight helper, exercised in lieu of the
  // production owner (FizzHandshakeHandler::inFlight_). onTerminal closures
  // reset current_ to mimic the handler unregistering the helper on its
  // terminal callback.
  FizzHandshakeHelper::UniquePtr current_;
  size_t terminalCount_{0};
};

TEST_F(FizzHandshakeHelperTest, HandshakeSuccess) {
  auto sp = makeSocketPair();

  folly::Baton<> serverDone;
  folly::AsyncTransport::UniquePtr negotiated;
  folly::exception_wrapper serverEx;

  evb_->runInEventBaseThreadAndWait([&] {
    auto serverSock = folly::AsyncSocket::newSocket(evb_, sp.server);
    current_.reset(new FizzHandshakeHelper(
        std::move(serverSock),
        serverCtx_,
        /*thriftParams=*/nullptr,
        HandshakeTimeout{std::chrono::seconds{5}},
        [this](FizzHandshakeHelper*) noexcept {
          ++terminalCount_;
          current_.reset();
        },
        [&](fizz::server::AsyncFizzServer::UniquePtr fs,
            std::shared_ptr<apache::thrift::ThriftParametersServerExtension>,
            folly::exception_wrapper ex) noexcept {
          negotiated = folly::AsyncTransport::UniquePtr(fs.release());
          serverEx = std::move(ex);
          serverDone.post();
        }));
    current_->start();
  });

  folly::Baton<> clientDone;
  folly::exception_wrapper clientEx;
  std::unique_ptr<TestFizzClient> client;
  evb_->runInEventBaseThreadAndWait([&] {
    client = std::make_unique<TestFizzClient>(evb_, sp.client);
    client->start([&](folly::exception_wrapper ex) noexcept {
      clientEx = std::move(ex);
      clientDone.post();
    });
  });

  ASSERT_TRUE(serverDone.try_wait_for(std::chrono::seconds{5}));
  ASSERT_TRUE(clientDone.try_wait_for(std::chrono::seconds{5}));

  EXPECT_FALSE(serverEx) << serverEx.what();
  EXPECT_FALSE(clientEx) << clientEx.what();
  EXPECT_NE(negotiated, nullptr);

  // Tear down on the EVB thread that owns these objects. terminalCount_
  // updates from the EVB thread; the user callback posts serverDone before
  // finish() invokes onTerminal, so we must observe terminalCount_ on the
  // EVB thread after that has run.
  evb_->runInEventBaseThreadAndWait([&] {
    EXPECT_EQ(terminalCount_, 1u);
    EXPECT_EQ(current_, nullptr);
    negotiated.reset();
    client.reset();
  });
}

TEST_F(FizzHandshakeHelperTest, HandshakeTimeout) {
  auto sp = makeSocketPair();

  folly::Baton<> done;
  folly::exception_wrapper ex;

  evb_->runInEventBaseThreadAndWait([&] {
    auto serverSock = folly::AsyncSocket::newSocket(evb_, sp.server);
    current_.reset(new FizzHandshakeHelper(
        std::move(serverSock),
        serverCtx_,
        /*thriftParams=*/nullptr,
        // Short timeout — client never sends ClientHello.
        HandshakeTimeout{std::chrono::milliseconds{50}},
        [this](FizzHandshakeHelper*) noexcept {
          ++terminalCount_;
          current_.reset();
        },
        [&](fizz::server::AsyncFizzServer::UniquePtr,
            std::shared_ptr<apache::thrift::ThriftParametersServerExtension>,
            folly::exception_wrapper e) noexcept {
          ex = std::move(e);
          done.post();
        }));
    current_->start();
  });

  ASSERT_TRUE(done.try_wait_for(std::chrono::seconds{2}));
  ASSERT_TRUE(ex);
  // terminalCount_ updates from the EVB thread; check it there.
  evb_->runInEventBaseThreadAndWait([&] {
    EXPECT_EQ(terminalCount_, 1u);
    EXPECT_EQ(current_, nullptr);
  });

  // Close the dangling client fd.
  ::close(sp.client.toFd());
}

TEST_F(FizzHandshakeHelperTest, HandshakeErrorOnGarbageInput) {
  auto sp = makeSocketPair();

  folly::Baton<> done;
  folly::exception_wrapper ex;

  evb_->runInEventBaseThreadAndWait([&] {
    auto serverSock = folly::AsyncSocket::newSocket(evb_, sp.server);
    current_.reset(new FizzHandshakeHelper(
        std::move(serverSock),
        serverCtx_,
        /*thriftParams=*/nullptr,
        HandshakeTimeout{std::chrono::seconds{5}},
        [this](FizzHandshakeHelper*) noexcept {
          ++terminalCount_;
          current_.reset();
        },
        [&](fizz::server::AsyncFizzServer::UniquePtr,
            std::shared_ptr<apache::thrift::ThriftParametersServerExtension>,
            folly::exception_wrapper e) noexcept {
          ex = std::move(e);
          done.post();
        }));
    current_->start();
  });

  // Write non-TLS garbage to the client end. Fizz should reject it.
  constexpr std::string_view garbage =
      "definitely not a TLS ClientHello frame ........";
  ASSERT_EQ(
      ::write(sp.client.toFd(), garbage.data(), garbage.size()),
      ssize_t(garbage.size()));

  ASSERT_TRUE(done.try_wait_for(std::chrono::seconds{2}));
  ASSERT_TRUE(ex);
  evb_->runInEventBaseThreadAndWait([&] {
    EXPECT_EQ(terminalCount_, 1u);
    EXPECT_EQ(current_, nullptr);
  });

  ::close(sp.client.toFd());
}

TEST_F(FizzHandshakeHelperTest, ShutdownCancelsPendingHandshake) {
  auto sp = makeSocketPair();

  std::atomic<bool> callbackFired{false};
  folly::exception_wrapper cancelEx;

  evb_->runInEventBaseThreadAndWait([&] {
    auto serverSock = folly::AsyncSocket::newSocket(evb_, sp.server);
    current_.reset(new FizzHandshakeHelper(
        std::move(serverSock),
        serverCtx_,
        /*thriftParams=*/nullptr,
        HandshakeTimeout{std::chrono::seconds{5}},
        [this](FizzHandshakeHelper*) noexcept {
          ++terminalCount_;
          current_.reset();
        },
        [&](fizz::server::AsyncFizzServer::UniquePtr,
            std::shared_ptr<apache::thrift::ThriftParametersServerExtension>,
            folly::exception_wrapper ex) noexcept {
          cancelEx = std::move(ex);
          callbackFired.store(true);
        }));
    current_->start();
  });

  // No client handshake. Cancel from the EVB thread to mirror real shutdown.
  // cancel() is synchronous: the terminal callback fires and current_
  // resets before cancel() returns.
  evb_->runInEventBaseThreadAndWait([&] {
    folly::DelayedDestruction::DestructorGuard guard(current_.get());
    current_->cancel();
    // Synchronous contract: terminal callback fired, ownership cleared.
    EXPECT_EQ(terminalCount_, 1u);
    EXPECT_EQ(current_, nullptr);
  });

  EXPECT_TRUE(callbackFired.load());
  EXPECT_TRUE(cancelEx);

  ::close(sp.client.toFd());
}

} // namespace apache::thrift::fast_thrift::security::test
