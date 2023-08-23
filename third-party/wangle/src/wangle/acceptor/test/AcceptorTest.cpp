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

#include <folly/io/async/EventBase.h>
#include <folly/io/async/test/AsyncSSLSocketTest.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <glog/logging.h>
#include <wangle/acceptor/AcceptObserver.h>
#include <wangle/acceptor/Acceptor.h>

using namespace folly;
using namespace wangle;
using namespace testing;

class TestConnection : public wangle::ManagedConnection {
 public:
  void timeoutExpired() noexcept override {}
  void describe(std::ostream& /*os*/) const override {}
  bool isBusy() const override {
    return false;
  }
  void notifyPendingShutdown() override {}
  void closeWhenIdle() override {}
  void dropConnection(const std::string& /* errorMsg */ = "") override {
    delete this;
  }
  void dumpConnectionState(uint8_t /*loglevel*/) override {}

  const folly::SocketAddress& getPeerAddress() const noexcept override {
    return dummyAddress;
  }

  folly::SocketAddress dummyAddress;
};

class TestAcceptor : public Acceptor {
 public:
  explicit TestAcceptor(const ServerSocketConfig& accConfig)
      : Acceptor(accConfig) {}

  void onNewConnection(
      folly::AsyncTransportWrapper::UniquePtr /*sock*/,
      const folly::SocketAddress* /*address*/,
      const std::string& /*nextProtocolName*/,
      SecureTransportType /*secureTransportType*/,
      const TransportInfo& /*tinfo*/) override {
    addConnection(new TestConnection);
    getEventBase()->terminateLoopSoon();
  }

  DefaultToFizzPeekingCallback* getFizzPeeker() override {
    return Acceptor::getFizzPeeker();
  }
};

enum class TestSSLConfig { NO_SSL, SSL, SSL_MULTI_CA };

class AcceptorTest : public ::testing::TestWithParam<TestSSLConfig> {
 public:
  AcceptorTest() = default;

  std::shared_ptr<AsyncSocket> connectClientSocket(
      const SocketAddress& serverAddress) {
    TestSSLConfig testConfig = GetParam();
    if (testConfig == TestSSLConfig::SSL ||
        testConfig == TestSSLConfig::SSL_MULTI_CA) {
      auto clientSocket = AsyncSSLSocket::newSocket(getTestSslContext(), &evb_);
      clientSocket->connect(nullptr, serverAddress);
      return clientSocket;
    } else {
      return AsyncSocket::newSocket(&evb_, serverAddress);
    }
  }

  std::tuple<std::shared_ptr<TestAcceptor>, std::shared_ptr<AsyncServerSocket>>
  initTestAcceptorAndSocket() {
    TestSSLConfig testConfig = GetParam();
    ServerSocketConfig config;
    if (testConfig == TestSSLConfig::SSL ||
        testConfig == TestSSLConfig::SSL_MULTI_CA) {
      config.sslContextConfigs.emplace_back(getTestSslContextConfig());
    }
    return initTestAcceptorAndSocket(config);
  }

  std::tuple<std::shared_ptr<TestAcceptor>, std::shared_ptr<AsyncServerSocket>>
  initTestAcceptorAndSocket(ServerSocketConfig config) {
    auto acceptor = std::make_shared<TestAcceptor>(config);
    auto socket = AsyncServerSocket::newSocket(&evb_);
    socket->addAcceptCallback(acceptor.get(), &evb_);
    acceptor->init(socket.get(), &evb_);
    socket->bind(0);
    socket->listen(100);
    socket->startAccepting();
    return std::make_tuple(acceptor, socket);
  }

  static std::shared_ptr<folly::SSLContext> getTestSslContext() {
    auto sslContext = std::make_shared<folly::SSLContext>(
        folly::SSLContext::SSLVersion::TLSv1_2);
    TestSSLConfig testConfig = GetParam();
    if (testConfig == TestSSLConfig::SSL) {
      sslContext->loadCertKeyPairFromFiles(
          folly::test::kTestCert, folly::test::kTestKey);
    } else if (testConfig == TestSSLConfig::SSL_MULTI_CA) {
      // Use a different cert.
      sslContext->loadCertKeyPairFromFiles(
          folly::test::kClientTestCert, folly::test::kClientTestKey);
    }
    sslContext->setOptions(SSL_OP_NO_TICKET);
    sslContext->ciphers("ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
    return sslContext;
  }

  static wangle::SSLContextConfig getTestSslContextConfig() {
    wangle::SSLContextConfig sslCtxConfig;
    TestSSLConfig testConfig = GetParam();
    sslCtxConfig.setCertificate(
        folly::test::kTestCert, folly::test::kTestKey, "");
    if (testConfig == TestSSLConfig::SSL_MULTI_CA) {
      sslCtxConfig.clientCAFiles = std::vector<std::string>{
          folly::test::kTestCA, folly::test::kClientTestCA};
    } else {
      sslCtxConfig.clientCAFile = folly::test::kTestCA;
    }
    sslCtxConfig.sessionContext = "AcceptorTest";
    sslCtxConfig.isDefault = true;
    sslCtxConfig.clientVerification =
        folly::SSLContext::VerifyClientCertificate::ALWAYS;
    sslCtxConfig.sessionCacheEnabled = false;
    return sslCtxConfig;
  }

 protected:
  EventBase evb_;
};

INSTANTIATE_TEST_CASE_P(
    NoSSLAndSSLTests,
    AcceptorTest,
    ::testing::Values(
        TestSSLConfig::NO_SSL,
        TestSSLConfig::SSL,
        TestSSLConfig::SSL_MULTI_CA));

TEST_P(AcceptorTest, Basic) {
  auto [acceptor, serverSocket] = initTestAcceptorAndSocket();
  SocketAddress serverAddress;
  serverSocket->getAddress(&serverAddress);
  auto clientSocket = connectClientSocket(serverAddress);

  evb_.loopForever();

  CHECK_EQ(acceptor->getNumConnections(), 1);
  CHECK(acceptor->getState() == Acceptor::State::kRunning);
  acceptor->forceStop();
  serverSocket->stopAccepting();
  evb_.loop();
}

class MockAcceptObserver : public AcceptObserver {
 public:
  MOCK_METHOD(void, accept, (folly::AsyncTransport* const), (noexcept));
  MOCK_METHOD(void, ready, (folly::AsyncTransport* const), (noexcept));
  MOCK_METHOD(void, acceptorDestroy, (Acceptor* const), (noexcept));
  MOCK_METHOD(void, observerAttach, (Acceptor* const), (noexcept));
  MOCK_METHOD(void, observerDetach, (Acceptor* const), (noexcept));
};

class MockAsyncSocketLifecycleObserver
    : public AsyncSocket::LegacyLifecycleObserver {
 public:
  MOCK_METHOD(void, observerAttach, (AsyncSocket*), (noexcept));
  MOCK_METHOD(void, observerDetach, (AsyncSocket*), (noexcept));
  MOCK_METHOD(void, destroy, (AsyncSocket*), (noexcept));
  MOCK_METHOD(void, close, (AsyncSocket*), (noexcept));
  MOCK_METHOD(void, connect, (AsyncSocket*), (noexcept));
  MOCK_METHOD(void, fdDetach, (AsyncSocket*), (noexcept));
  MOCK_METHOD(void, move, (AsyncSocket*, AsyncSocket*), (noexcept));
  MOCK_METHOD(void, evbAttach, (AsyncSocket*, EventBase*), (noexcept));
  MOCK_METHOD(void, evbDetach, (AsyncSocket*, EventBase*), (noexcept));
};

class MockFizzLoggingCallback : public FizzLoggingCallback {
 public:
  MOCK_METHOD(
      void,
      logFizzHandshakeSuccess,
      (const fizz::server::AsyncFizzServer&, const wangle::TransportInfo&),
      (noexcept));

  MOCK_METHOD(
      void,
      logFallbackHandshakeSuccess,
      (const folly::AsyncSSLSocket&, const wangle::TransportInfo&),
      (noexcept));

  MOCK_METHOD(
      void,
      logFizzHandshakeFallback,
      (const fizz::server::AsyncFizzServer&, const wangle::TransportInfo&),
      (noexcept));

  MOCK_METHOD(
      void,
      logFizzHandshakeError,
      (const fizz::server::AsyncFizzServer&, const folly::exception_wrapper&),
      (noexcept));

  MOCK_METHOD(
      void,
      logFallbackHandshakeError,
      (const folly::AsyncSSLSocket&, const folly::AsyncSocketException&),
      (noexcept));
};

TEST_P(AcceptorTest, AcceptObserver) {
  auto [acceptor, serverSocket] = initTestAcceptorAndSocket();
  SocketAddress serverAddress;
  serverSocket->getAddress(&serverAddress);

  auto cb = std::make_unique<StrictMock<MockAcceptObserver>>();
  EXPECT_CALL(*cb, observerAttach(acceptor.get()));
  acceptor->addAcceptObserver(cb.get());

  // add first connection, expect callbacks
  auto clientSocket1 = connectClientSocket(serverAddress);
  {
    InSequence s;
    EXPECT_CALL(*cb, accept(_));
    EXPECT_CALL(*cb, ready(_));
  }
  evb_.loopForever();
  Mock::VerifyAndClearExpectations(cb.get());
  CHECK_EQ(acceptor->getNumConnections(), 1);
  CHECK(acceptor->getState() == Acceptor::State::kRunning);

  // add second connection, expect callbacks
  auto clientSocket2 = connectClientSocket(serverAddress);
  {
    InSequence s;
    EXPECT_CALL(*cb, accept(_));
    EXPECT_CALL(*cb, ready(_));
  }
  evb_.loopForever();
  Mock::VerifyAndClearExpectations(cb.get());
  CHECK_EQ(acceptor->getNumConnections(), 2);
  CHECK(acceptor->getState() == Acceptor::State::kRunning);

  // remove AcceptObserver
  EXPECT_CALL(*cb, observerDetach(acceptor.get()));
  EXPECT_TRUE(acceptor->removeAcceptObserver(cb.get()));
  Mock::VerifyAndClearExpectations(cb.get());

  // add third connection, no callbacks
  auto clientSocket3 = connectClientSocket(serverAddress);
  evb_.loopForever();
  Mock::VerifyAndClearExpectations(cb.get());
  CHECK_EQ(acceptor->getNumConnections(), 3);
  CHECK(acceptor->getState() == Acceptor::State::kRunning);

  // stop the acceptor
  acceptor->forceStop();
  serverSocket->stopAccepting();
  evb_.loop();
}

TEST_P(AcceptorTest, AcceptObserverRemove) {
  auto [acceptor, serverSocket] = initTestAcceptorAndSocket();
  auto cb = std::make_unique<StrictMock<MockAcceptObserver>>();
  EXPECT_CALL(*cb, observerAttach(acceptor.get()));
  acceptor->addAcceptObserver(cb.get());
  Mock::VerifyAndClearExpectations(cb.get());

  EXPECT_CALL(*cb, observerDetach(acceptor.get()));
  EXPECT_TRUE(acceptor->removeAcceptObserver(cb.get()));
  Mock::VerifyAndClearExpectations(cb.get());

  // cleanup
  acceptor->forceStop();
  serverSocket->stopAccepting();
  evb_.loop();
}

TEST_P(AcceptorTest, AcceptObserverRemoveMissing) {
  auto [acceptor, serverSocket] = initTestAcceptorAndSocket();
  auto cb = std::make_unique<StrictMock<MockAcceptObserver>>();
  EXPECT_FALSE(acceptor->removeAcceptObserver(cb.get()));

  // cleanup
  acceptor->forceStop();
  serverSocket->stopAccepting();
  evb_.loop();
}

TEST_P(AcceptorTest, AcceptObserverAcceptorDestroyed) {
  auto [acceptor, serverSocket] = initTestAcceptorAndSocket();
  auto cb = std::make_unique<StrictMock<MockAcceptObserver>>();
  EXPECT_CALL(*cb, observerAttach(acceptor.get()));
  acceptor->addAcceptObserver(cb.get());
  Mock::VerifyAndClearExpectations(cb.get());

  // stop the acceptor
  acceptor->forceStop();
  serverSocket->stopAccepting();
  evb_.loop();

  // destroy the acceptor while the AcceptObserver is installed
  EXPECT_CALL(*cb, acceptorDestroy(acceptor.get()));
  acceptor = nullptr;
  Mock::VerifyAndClearExpectations(cb.get());
}

TEST_P(AcceptorTest, AcceptObserverMultipleRemove) {
  auto [acceptor, serverSocket] = initTestAcceptorAndSocket();
  auto cb1 = std::make_unique<StrictMock<MockAcceptObserver>>();
  EXPECT_CALL(*cb1, observerAttach(acceptor.get()));
  acceptor->addAcceptObserver(cb1.get());
  Mock::VerifyAndClearExpectations(cb1.get());

  auto cb2 = std::make_unique<StrictMock<MockAcceptObserver>>();
  EXPECT_CALL(*cb2, observerAttach(acceptor.get()));
  acceptor->addAcceptObserver(cb2.get());
  Mock::VerifyAndClearExpectations(cb2.get());

  EXPECT_CALL(*cb1, observerDetach(acceptor.get()));
  EXPECT_TRUE(acceptor->removeAcceptObserver(cb1.get()));
  Mock::VerifyAndClearExpectations(cb1.get());

  EXPECT_CALL(*cb2, observerDetach(acceptor.get()));
  EXPECT_TRUE(acceptor->removeAcceptObserver(cb2.get()));
  Mock::VerifyAndClearExpectations(cb2.get());

  // cleanup
  acceptor->forceStop();
  serverSocket->stopAccepting();
  evb_.loop();
}

TEST_P(AcceptorTest, AcceptObserverMultipleRemoveReverse) {
  auto [acceptor, serverSocket] = initTestAcceptorAndSocket();
  auto cb1 = std::make_unique<StrictMock<MockAcceptObserver>>();
  EXPECT_CALL(*cb1, observerAttach(acceptor.get()));
  acceptor->addAcceptObserver(cb1.get());
  Mock::VerifyAndClearExpectations(cb1.get());

  auto cb2 = std::make_unique<StrictMock<MockAcceptObserver>>();
  EXPECT_CALL(*cb2, observerAttach(acceptor.get()));
  acceptor->addAcceptObserver(cb2.get());
  Mock::VerifyAndClearExpectations(cb2.get());

  EXPECT_CALL(*cb2, observerDetach(acceptor.get()));
  EXPECT_TRUE(acceptor->removeAcceptObserver(cb2.get()));
  Mock::VerifyAndClearExpectations(cb2.get());

  EXPECT_CALL(*cb1, observerDetach(acceptor.get()));
  EXPECT_TRUE(acceptor->removeAcceptObserver(cb1.get()));
  Mock::VerifyAndClearExpectations(cb1.get());

  // cleanup
  acceptor->forceStop();
  serverSocket->stopAccepting();
  evb_.loop();
}

TEST_P(AcceptorTest, AcceptObserverMultipleAcceptorDestroyed) {
  auto [acceptor, serverSocket] = initTestAcceptorAndSocket();
  auto cb1 = std::make_unique<StrictMock<MockAcceptObserver>>();
  EXPECT_CALL(*cb1, observerAttach(acceptor.get()));
  acceptor->addAcceptObserver(cb1.get());
  Mock::VerifyAndClearExpectations(cb1.get());

  auto cb2 = std::make_unique<StrictMock<MockAcceptObserver>>();
  EXPECT_CALL(*cb2, observerAttach(acceptor.get()));
  acceptor->addAcceptObserver(cb2.get());
  Mock::VerifyAndClearExpectations(cb2.get());

  // stop the acceptor
  acceptor->forceStop();
  serverSocket->stopAccepting();
  evb_.loop();

  // destroy the acceptor while the AcceptObserver is installed
  EXPECT_CALL(*cb1, acceptorDestroy(acceptor.get()));
  EXPECT_CALL(*cb2, acceptorDestroy(acceptor.get()));
  acceptor = nullptr;
  Mock::VerifyAndClearExpectations(cb1.get());
  Mock::VerifyAndClearExpectations(cb2.get());
}

TEST_P(AcceptorTest, AcceptObserverRemoveCallbackThenStopAcceptor) {
  auto [acceptor, serverSocket] = initTestAcceptorAndSocket();
  auto cb = std::make_unique<StrictMock<MockAcceptObserver>>();
  EXPECT_CALL(*cb, observerAttach(acceptor.get()));
  acceptor->addAcceptObserver(cb.get());
  Mock::VerifyAndClearExpectations(cb.get());

  EXPECT_CALL(*cb, observerDetach(acceptor.get()));
  EXPECT_TRUE(acceptor->removeAcceptObserver(cb.get()));
  Mock::VerifyAndClearExpectations(cb.get());

  acceptor->forceStop();
  serverSocket->stopAccepting();
  evb_.loop();
}

TEST_P(AcceptorTest, AcceptObserverStopAcceptorThenRemoveCallback) {
  auto [acceptor, serverSocket] = initTestAcceptorAndSocket();
  auto cb = std::make_unique<StrictMock<MockAcceptObserver>>();
  EXPECT_CALL(*cb, observerAttach(acceptor.get()));
  acceptor->addAcceptObserver(cb.get());
  Mock::VerifyAndClearExpectations(cb.get());

  acceptor->forceStop();
  serverSocket->stopAccepting();
  evb_.loop();

  EXPECT_CALL(*cb, observerDetach(acceptor.get()));
  EXPECT_TRUE(acceptor->removeAcceptObserver(cb.get()));
  Mock::VerifyAndClearExpectations(cb.get());
}

/**
 * Test if AsyncSocket::LegacyLifecycleObserver can track socket during SSL
 * accept.
 *
 * With Fizz support, the accept process involves transforming the AsyncSocket
 * to an AsyncFizzServer. Then, if Fizz falls back to OpenSSL, the
 * AsyncFizzServer will be transformed into an AsyncSSLSocket.
 *
 * During each transformation, the LegacyLifecycleObserver::move callback must
 * be triggered so that the observer can unsubscribe from events on the old
 * socket and subscribe to events on the new socket. This requires Wangle and
 * Fizz to use the AsyncSocket(AsyncSocket* oldSocket) constructor when
 * performing the transformation.
 *
 * This test ensures that even in the worst case, where two transformations
 * occur, that the observer will be able to track the socket through to the
 * completion of the accept, when ready() is triggered.
 */
TEST_P(
    AcceptorTest,
    AcceptObserverInstallSocketObserverThenFizzThenFallbackToSSL) {
  auto [acceptor, serverSocket] = initTestAcceptorAndSocket();
  auto onAcceptCb = std::make_unique<StrictMock<MockAcceptObserver>>();
  auto lifecycleCb =
      std::make_unique<StrictMock<MockAsyncSocketLifecycleObserver>>();
  auto fizzLoggingCb = std::make_unique<StrictMock<MockFizzLoggingCallback>>();
  acceptor->getFizzPeeker()->options().setLoggingCallback(fizzLoggingCb.get());

  EXPECT_CALL(*onAcceptCb, observerAttach(acceptor.get()));
  acceptor->addAcceptObserver(onAcceptCb.get());
  Mock::VerifyAndClearExpectations(onAcceptCb.get());

  // add connection, expect callbacks
  SocketAddress serverAddress;
  serverSocket->getAddress(&serverAddress);

  AsyncSocket::UniquePtr clientSocket;
  TestSSLConfig testConfig = GetParam();
  if (testConfig == TestSSLConfig::SSL ||
      testConfig == TestSSLConfig::SSL_MULTI_CA) {
    auto sslContext = getTestSslContext();
    // fallback from Fizz only occurs with TLS 1.2
    sslContext->disableTLS13();
    clientSocket = AsyncSSLSocket::newSocket(std::move(sslContext), &evb_);
    clientSocket->connect(nullptr, serverAddress);
  } else {
    clientSocket = AsyncSocket::newSocket(&evb_, serverAddress);
  }

  folly::AsyncTransport* remoteSocket = nullptr;

  // we have to EXPECT_EQ below instead of matchers because the remoteSocket
  // will change; ByRef() does not work here, due to const AsyncSocket*
  Sequence s1;
  Sequence s2;
  EXPECT_CALL(*onAcceptCb, accept(_))
      .InSequence(s1)
      .WillOnce(Invoke([&lifecycleCb, &remoteSocket](auto socket) {
        remoteSocket = socket;
        if (auto asyncSocket =
                socket->template getUnderlyingTransport<folly::AsyncSocket>()) {
          EXPECT_CALL(*lifecycleCb, observerAttach(asyncSocket));
          const_cast<folly::AsyncSocket*>(asyncSocket)
              ->addLifecycleObserver(lifecycleCb.get());
        }
      }));

  if (testConfig == TestSSLConfig::SSL ||
      testConfig == TestSSLConfig::SSL_MULTI_CA) {
    // AsyncSocket -> AsyncFizzServer
    EXPECT_CALL(*lifecycleCb, fdDetach(_))
        .InSequence(s1)
        .WillOnce(Invoke([&remoteSocket](folly::AsyncTransport* socket) {
          EXPECT_EQ(remoteSocket, socket);
        }));
    EXPECT_CALL(*lifecycleCb, move(_, _))
        .InSequence(s1)
        .WillOnce(Invoke([&lifecycleCb, &remoteSocket, &s2](
                             folly::AsyncSocket* oldSocket,
                             folly::AsyncSocket* newSocket) {
          EXPECT_EQ(remoteSocket, oldSocket);
          EXPECT_NE(remoteSocket, newSocket);

          // remove LifecycleCallback from old socket
          EXPECT_CALL(*lifecycleCb, observerDetach(_))
              .InSequence(s2)
              .WillOnce(Invoke([&remoteSocket](folly::AsyncTransport* socket) {
                EXPECT_EQ(remoteSocket, socket);
              }));
          EXPECT_TRUE(oldSocket->removeLifecycleObserver(lifecycleCb.get()));
          EXPECT_THAT(oldSocket->getLifecycleObservers(), IsEmpty());

          // add LifecycleCallback to new socket
          EXPECT_THAT(newSocket->getLifecycleObservers(), IsEmpty());
          EXPECT_CALL(*lifecycleCb, observerAttach(_));
          newSocket->addLifecycleObserver(lifecycleCb.get());
          EXPECT_THAT(
              newSocket->getLifecycleObservers(),
              UnorderedElementsAre(lifecycleCb.get()));

          // update remoteSocket
          remoteSocket = newSocket;
        }));

    EXPECT_CALL(*fizzLoggingCb, logFallbackHandshakeSuccess(_, _));

    // AsyncFizzServer -> AsyncSSLSocket
    // use logFizzHandshakeFallback to verify that fallback occurred
    EXPECT_CALL(*fizzLoggingCb, logFizzHandshakeFallback(_, _))
        .InSequence(s1)
        .WillOnce(Invoke([&remoteSocket](
                             const fizz::server::AsyncFizzServer& transport,
                             const wangle::TransportInfo& /* tinfo */) {
          EXPECT_EQ(
              remoteSocket,
              transport.getUnderlyingTransport<folly::AsyncSocket>());
        }));
    EXPECT_CALL(*lifecycleCb, fdDetach(_))
        .InSequence(s1)
        .WillOnce(Invoke([&remoteSocket](folly::AsyncSocket* socket) {
          EXPECT_EQ(remoteSocket, socket);
        }));
    EXPECT_CALL(*lifecycleCb, move(_, _))
        .InSequence(s1)
        .WillOnce(Invoke([&lifecycleCb, &remoteSocket, &s2](
                             folly::AsyncSocket* oldSocket,
                             folly::AsyncSocket* newSocket) {
          EXPECT_EQ(remoteSocket, oldSocket);
          EXPECT_NE(remoteSocket, newSocket);

          // remove LifecycleCallback from old socket
          EXPECT_THAT(
              oldSocket->getLifecycleObservers(),
              UnorderedElementsAre(lifecycleCb.get()));
          EXPECT_CALL(*lifecycleCb, observerDetach(_))
              .InSequence(s2)
              .WillOnce(Invoke([&remoteSocket](folly::AsyncTransport* socket) {
                EXPECT_EQ(remoteSocket, socket);
              }));
          EXPECT_TRUE(oldSocket->removeLifecycleObserver(lifecycleCb.get()));
          EXPECT_THAT(oldSocket->getLifecycleObservers(), IsEmpty());

          // add LifecycleCallback to new socket
          EXPECT_THAT(newSocket->getLifecycleObservers(), IsEmpty());
          EXPECT_CALL(*lifecycleCb, observerAttach(newSocket));
          newSocket->addLifecycleObserver(lifecycleCb.get());
          EXPECT_THAT(
              newSocket->getLifecycleObservers(),
              UnorderedElementsAre(lifecycleCb.get()));

          // update remoteSocket
          remoteSocket = newSocket;
        }));
  }

  // the socket will be ready, and then immediately close
  EXPECT_CALL(*onAcceptCb, ready(_))
      .InSequence(s1)
      .WillOnce(Invoke([&lifecycleCb,
                        &remoteSocket](const auto* const& socket) {
        EXPECT_EQ(remoteSocket, socket);
        if (auto asyncSocket =
                socket->template getUnderlyingTransport<folly::AsyncSocket>()) {
          EXPECT_THAT(
              asyncSocket->getLifecycleObservers(),
              UnorderedElementsAre(lifecycleCb.get()));
        }
      }));
  EXPECT_CALL(*lifecycleCb, close(_))
      .InSequence(s1)
      .WillOnce(Invoke([&remoteSocket](const auto* const& socket) {
        EXPECT_EQ(remoteSocket, socket);
      }));
  EXPECT_CALL(*lifecycleCb, destroy(_))
      .InSequence(s1)
      .WillOnce(Invoke([&remoteSocket](const auto* const& socket) {
        EXPECT_EQ(remoteSocket, socket);
      }));

  evb_.loopForever();
  Mock::VerifyAndClearExpectations(onAcceptCb.get());
  Mock::VerifyAndClearExpectations(fizzLoggingCb.get());
  Mock::VerifyAndClearExpectations(lifecycleCb.get());
  CHECK_EQ(acceptor->getNumConnections(), 1);
  CHECK(acceptor->getState() == Acceptor::State::kRunning);

  acceptor->forceStop();
  serverSocket->stopAccepting();
  evb_.loop();

  EXPECT_CALL(*onAcceptCb, observerDetach(acceptor.get()));
  EXPECT_TRUE(acceptor->removeAcceptObserver(onAcceptCb.get()));
  acceptor = nullptr;
  Mock::VerifyAndClearExpectations(onAcceptCb.get());
}
