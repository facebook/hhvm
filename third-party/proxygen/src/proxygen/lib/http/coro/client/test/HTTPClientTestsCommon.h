/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPCoroSession.h"
#include "proxygen/lib/http/coro/HTTPError.h"
#include "proxygen/lib/http/coro/HTTPFixedSource.h"
#include "proxygen/lib/http/coro/HTTPHybridSource.h"
#include "proxygen/lib/http/coro/client/HTTPClient.h"
#include "proxygen/lib/http/coro/server/ScopedHTTPServer.h"
#include "proxygen/lib/http/coro/test/HTTPTestSources.h"
#include "proxygen/lib/http/coro/transport/test/HTTPConnectTransportTest.h"
#include <folly/logging/xlog.h>

#include <algorithm>
#include <folly/coro/Baton.h>
#include <folly/io/async/AsyncSocketException.h>
#include <proxygen/lib/http/codec/test/TestUtils.h>
#include <proxygen/lib/http/coro/test/TestUtils.h>
#include <proxygen/lib/utils/TestUtils.h>

#include <folly/portability/GTest.h>

namespace proxygen::coro::test {
using namespace testing;
using folly::AsyncSocketException;

enum class TransportType { TCP, TLS, TLS_FIZZ, QUIC };

inline std::string transportTypeToTestName(
    const testing::TestParamInfo<TransportType>& info) {
  switch (info.param) {
    case TransportType::TLS:
      return "tls";
    case TransportType::TLS_FIZZ:
      return "fizz";
    case TransportType::TCP:
      return "tcp";
    case TransportType::QUIC:
      return "quic";
  }
}

class TestHandler : public HTTPHandler {
 public:
  folly::coro::Task<HTTPSourceHolder> handleRequest(
      folly::EventBase* evb,
      HTTPSessionContextPtr ctx,
      HTTPSourceHolder requestSource) override;

  ConnectHandler connectHandler_;
};

class HTTPClientTests : public TestWithParam<TransportType> {
 public:
  void SetUp() override;

  void TearDown() override {
    testHandler_->connectHandler_.resetExceptionExpected();
    server_.reset();
  }

  folly::DrivableExecutor* getExecutor() {
    return &evb_;
  }

  std::string getURL(const std::string& hostname,
                     uint16_t port,
                     const std::string& path) {
    return folly::to<std::string>("http",
                                  (GetParam() != TransportType::TCP ? "s" : ""),
                                  "://",
                                  hostname,
                                  ":",
                                  port,
                                  path);
  }

  std::string getURL(const folly::SocketAddress& serverAddress,
                     const std::string& path) {
    return getURL(serverAddress.getAddressStr(), serverAddress.getPort(), path);
  }

  std::string getURL(const std::string& path) {
    return getURL(serverAddress_, path);
  }

  bool useQuic() const {
    return GetParam() == TransportType::QUIC;
  }

  static std::unique_ptr<ScopedHTTPServer> constructServer(
      const std::string& ip,
      const uint16_t port,
      const TransportType transportType,
      std::shared_ptr<TestHandler> testHandler);

  std::unique_ptr<ScopedHTTPServer> constructServer(
      const std::string& ip,
      const uint16_t port,
      std::shared_ptr<TestHandler> testHandler =
          std::make_shared<TestHandler>());

 protected:
  static HTTPServer::Config getServerConfig(const std::string& ip,
                                            const uint16_t port,
                                            const TransportType transportType);

  enum class ExceptionType {
    TIMED_OUT,
    SSL_ERROR,
  };
  static void expectException(TransportType ttype,
                              folly::Try<HTTPCoroSession*>& sess,
                              ExceptionType exType) {
    if (ttype == TransportType::QUIC) {
      switch (exType) {
        case ExceptionType::TIMED_OUT: {
          auto ex = sess.tryGetExceptionObject<quic::QuicInternalException>();
          EXPECT_EQ(ex->errorCode(), quic::LocalErrorCode::CONNECT_FAILED);
          break;
        }
        case ExceptionType::SSL_ERROR: {
          auto ex = sess.tryGetExceptionObject<quic::QuicTransportException>();
          bool isCryptoError = uint16_t(ex->errorCode()) &
                               uint16_t(quic::TransportErrorCode::CRYPTO_ERROR);
          EXPECT_TRUE(isCryptoError);
          break;
        }
      }
    } else {
      auto ex = sess.tryGetExceptionObject<AsyncSocketException>();
      folly::AsyncSocketException::AsyncSocketExceptionType aseType;
      switch (exType) {
        case ExceptionType::TIMED_OUT:
          aseType = AsyncSocketException::AsyncSocketExceptionType::TIMED_OUT;
          break;
        case ExceptionType::SSL_ERROR:
          aseType = AsyncSocketException::AsyncSocketExceptionType::SSL_ERROR;
          break;
      }
      EXPECT_EQ(ex->getType(), aseType);
    }
  }

  folly::EventBase evb_;
  std::shared_ptr<TestHandler> testHandler_;
  std::unique_ptr<ScopedHTTPServer> server_;
  folly::SocketAddress serverAddress_;
};

} // namespace proxygen::coro::test
