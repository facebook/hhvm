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

#include <thrift/lib/cpp2/fast_thrift/connection/security/util/StopTLSHelper.h>

#include <sys/socket.h>
#include <array>
#include <memory>

#include <gtest/gtest.h>

#include <fizz/server/AsyncFizzServer.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/ScopedEventBaseThread.h>

#include <thrift/lib/cpp2/fast_thrift/security/FizzServerCertConfig.h>
#include <thrift/lib/cpp2/fast_thrift/security/FizzServerContextBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/security/ThriftTlsConfig.h>
#include <thrift/lib/cpp2/fast_thrift/security/test/TestCert.h>

namespace apache::thrift::fast_thrift::connection::security::util::test {

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

} // namespace

class StopTLSHelperTest : public ::testing::Test {
 protected:
  void SetUp() override {
    evbThread_ = std::make_unique<folly::ScopedEventBaseThread>();
    evb_ = evbThread_->getEventBase();

    apache::thrift::fast_thrift::security::FizzServerCertConfig cfg;
    auto cert = apache::thrift::fast_thrift::security::test::makeTestCert();
    cfg.certPem = cert.certPem;
    cfg.keyPem = cert.keyPem;
    cfg.clientAuth = fizz::server::ClientAuthMode::None;
    serverCtx_ =
        apache::thrift::fast_thrift::security::buildTLSParams(
            cfg, apache::thrift::fast_thrift::security::ThriftTlsConfig{})
            .fizzContext;
  }

  void TearDown() override { evbThread_.reset(); }

  // Builds an AsyncFizzServer-wrapped server socket without driving the
  // handshake. Sufficient for cancel-path tests; real StopTLS end-to-end is
  // exercised by the handler-level integration tests.
  fizz::server::AsyncFizzServer::UniquePtr makeFizzServer(
      folly::NetworkSocket fd) {
    auto sock = folly::AsyncSocket::newSocket(evb_, fd);
    return fizz::server::AsyncFizzServer::UniquePtr(
        new fizz::server::AsyncFizzServer(
            folly::AsyncTransport::UniquePtr(sock.release()),
            serverCtx_,
            /*extensions=*/nullptr));
  }

  std::unique_ptr<folly::ScopedEventBaseThread> evbThread_;
  folly::EventBase* evb_{nullptr};
  std::shared_ptr<const fizz::server::FizzServerContext> serverCtx_;
  StopTLSHelper::UniquePtr current_;
  size_t terminalCount_{0};
};

TEST_F(StopTLSHelperTest, CancelBeforeStart) {
  auto sp = makeSocketPair();
  std::atomic<bool> callbackFired{false};
  folly::exception_wrapper cancelEx;

  evb_->runInEventBaseThreadAndWait([&] {
    auto fizzServer = makeFizzServer(sp.server);
    current_.reset(new StopTLSHelper(
        std::move(fizzServer),
        [this](StopTLSHelper*) noexcept {
          ++terminalCount_;
          current_.reset();
        },
        [&](folly::AsyncTransport::UniquePtr,
            folly::exception_wrapper ex) noexcept {
          cancelEx = std::move(ex);
          callbackFired.store(true);
        }));

    // Cancel without ever calling start(). The synchronous contract still
    // holds: the terminal callback fires and current_ resets within cancel().
    folly::DelayedDestruction::DestructorGuard guard(current_.get());
    current_->cancel();
    EXPECT_EQ(terminalCount_, 1u);
    EXPECT_EQ(current_, nullptr);
  });

  EXPECT_TRUE(callbackFired.load());
  EXPECT_TRUE(cancelEx);

  ::close(sp.client.toFd());
}

} // namespace apache::thrift::fast_thrift::connection::security::util::test
