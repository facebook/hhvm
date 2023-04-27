/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/protocol/test/Utilities.h>
#include <fizz/server/AsyncFizzServer.h>
#include <fizz/server/test/Mocks.h>
#include <fizz/server/test/Utils.h>
#include <folly/io/async/AsyncServerSocket.h>
#include <folly/ssl/Init.h>
#include <proxygen/lib/http/HTTPConnectorWithFizz.h>
#include <proxygen/lib/http/session/HTTPUpstreamSession.h>

using namespace proxygen;
using namespace testing;
using namespace fizz::server;

class MockHTTPConnectorCallback : public HTTPConnector::Callback {
 public:
  ~MockHTTPConnectorCallback() override = default;
  MOCK_METHOD(void, connectSuccess, (HTTPUpstreamSession * session));
  MOCK_METHOD(void, connectError, (const folly::AsyncSocketException& ex));
  MOCK_METHOD(void, preConnect, (folly::AsyncTransport * transport));
};

class HTTPConnectorWithFizzTest : public testing::Test {
 public:
  HTTPConnectorWithFizzTest()
      : evb_(true), factory_(&handshakeCb_), server_(evb_, &factory_) {
  }

  void SetUp() override {
    folly::ssl::init();

    timer_ = folly::HHWheelTimer::newTimer(
        &evb_,
        std::chrono::milliseconds(folly::HHWheelTimer::DEFAULT_TICK_INTERVAL),
        folly::AsyncTimeout::InternalEnum::NORMAL,
        std::chrono::milliseconds(5000));

    EXPECT_CALL(cb_, preConnect(_))
        .WillOnce(Invoke([](folly::AsyncTransport* t) {
          EXPECT_NE(t, nullptr);
          EXPECT_NE(t->getUnderlyingTransport<fizz::client::AsyncFizzClient>(),
                    nullptr);
        }));
  }

 protected:
  class DummyCallbackFactory
      : public fizz::server::test::FizzTestServer::CallbackFactory {
   public:
    explicit DummyCallbackFactory(fizz::server::test::MockHandshakeCallback* cb)
        : cb_(cb) {
    }

    AsyncFizzServer::HandshakeCallback* getCallback(
        std::shared_ptr<AsyncFizzServer> srv) override {
      // Keep connection alive
      conn_ = srv;
      return cb_;
    }

   private:
    AsyncFizzServer::HandshakeCallback* cb_;
    std::shared_ptr<AsyncFizzServer> conn_;
  };
  void SetupFailureCallbacks() {
    ON_CALL(handshakeCb_, _fizzHandshakeError(_))
        .WillByDefault(Invoke([&](folly::exception_wrapper ex) {
          evb_.terminateLoopSoon();
          if (ex.what().toStdString().find("readEOF()") == std::string::npos) {
            FAIL() << "Server error handler called: "
                   << ex.what().toStdString();
          }
        }));
    ON_CALL(cb_, connectError(_))
        .WillByDefault(Invoke([&](const folly::AsyncSocketException& ex) {
          evb_.terminateLoopSoon();
          FAIL() << "Client error handler called: " << ex.what();
        }));
  }
  folly::EventBase evb_;
  fizz::server::test::MockHandshakeCallback handshakeCb_;
  DummyCallbackFactory factory_;
  fizz::server::test::FizzTestServer server_;
  folly::HHWheelTimer::UniquePtr timer_;
  MockHTTPConnectorCallback cb_;
};

TEST_F(HTTPConnectorWithFizzTest, TestFizzConnect) {
  SetupFailureCallbacks();
  HTTPConnectorWithFizz connector(&cb_, timer_.get());
  proxygen::HTTPUpstreamSession* session = nullptr;
  EXPECT_CALL(cb_, connectSuccess(_))
      .WillOnce(
          Invoke([&](proxygen::HTTPUpstreamSession* sess) { session = sess; }));

  auto context = std::make_shared<fizz::client::FizzClientContext>();
  connector.connectFizz(&evb_, server_.getAddress(), context, nullptr);
  EXPECT_CALL(handshakeCb_, _fizzHandshakeSuccess()).WillOnce(Invoke([&]() {
    evb_.terminateLoopSoon();
  }));
  evb_.loop();
  if (session) {
    session->dropConnection();
  }
}

TEST_F(HTTPConnectorWithFizzTest, TestFizzConnectFailure) {
  HTTPConnectorWithFizz connector(&cb_, timer_.get());

  auto serverContext = server_.getFizzContext();
  serverContext->setSupportedCiphers(
      {{fizz::CipherSuite::TLS_AES_128_GCM_SHA256}});

  auto context = std::make_shared<fizz::client::FizzClientContext>();
  context->setSupportedCiphers({fizz::CipherSuite::TLS_AES_256_GCM_SHA384});

  connector.connectFizz(&evb_, server_.getAddress(), context, nullptr);
  EXPECT_CALL(handshakeCb_, _fizzHandshakeError(_));
  EXPECT_CALL(cb_, connectError(_)).WillOnce(InvokeWithoutArgs([&]() {
    evb_.terminateLoopSoon();
  }));
  evb_.loop();
}
