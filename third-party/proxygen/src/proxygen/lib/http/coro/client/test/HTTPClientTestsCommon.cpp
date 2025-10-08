/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/client/test/HTTPClientTestsCommon.h"

using folly::coro::co_nothrow;

namespace {

// gets the sni sent by client
using namespace proxygen::coro;
std::string getSniOrEmpty(const HTTPSessionContextPtr& ctx) {
  wangle::TransportInfo info;
  ctx->getCurrentTransportInfo(&info, /*includeSetupFields=*/true);
  return info.sslServerName ? *info.sslServerName : "";
}

} // namespace

namespace proxygen::coro::test {

/*static*/ HTTPServer::Config HTTPClientTests::getServerConfig(
    const std::string& ip,
    const uint16_t port,
    const TransportType transportType) {
  auto tlsConfig = HTTPServer::getDefaultTLSConfig();
  tlsConfig.isDefault = true;
  tlsConfig.clientVerification =
      folly::SSLContext::VerifyClientCertificate::DO_NOT_REQUEST;
  tlsConfig.setNextProtocols({"h2", "http/1.1"});
  try {
    const std::string kTestDir = getContainingDirectory(XLOG_FILENAME).str();
    tlsConfig.setCertificate(kTestDir + "certs/test_cert1.pem",
                             kTestDir + "certs/test_key1.pem",
                             "");
  } catch (const std::exception& ex) {
    XLOG(ERR) << "Invalid certificate file or key file: %s" << ex.what();
  }
  HTTPServer::Config serverConfig;
  serverConfig.socketConfig.bindAddress.setFromIpPort(ip, port);
  if (transportType != TransportType::TCP) {
    serverConfig.socketConfig.sslContextConfigs.emplace_back(
        std::move(tlsConfig));
  }
  if (transportType == TransportType::QUIC) {
    serverConfig.quicConfig = HTTPServer::QuicConfig();
  }
  return serverConfig;
}

/*static*/ std::unique_ptr<ScopedHTTPServer> HTTPClientTests::constructServer(
    const std::string& ip,
    const uint16_t port,
    const TransportType transportType,
    std::shared_ptr<TestHandler> testHandler) {
  // Disable cert verification since the server is self-signed
  HTTPClient::setDefaultCAPaths({});
  HTTPClient::setDefaultFizzCertVerifier(
      std::make_shared<InsecureVerifierDangerousDoNotUseInProduction>());
  auto serverConfig = getServerConfig(ip, port, transportType);
  return ScopedHTTPServer::start(std::move(serverConfig), testHandler);
}

std::unique_ptr<ScopedHTTPServer> HTTPClientTests::constructServer(
    const std::string& ip,
    const uint16_t port,
    std::shared_ptr<TestHandler> testHandler) {
  return constructServer(ip, port, GetParam(), std::move(testHandler));
}

void HTTPClientTests::SetUp() {
  testHandler_ = std::make_shared<TestHandler>();
  server_ = constructServer("127.0.0.1", 0, testHandler_);
  serverAddress_ = *server_->address();
}

folly::coro::Task<HTTPSourceHolder> TestHandler::handleRequest(
    folly::EventBase* evb,
    HTTPSessionContextPtr ctx,
    HTTPSourceHolder requestSource) {

  // all connections are made directly with ip; expect empty sni
  auto sni = getSniOrEmpty(ctx);
  EXPECT_TRUE(sni.empty()) << "unexpected sni=" << sni;

  auto headerEvent = co_await co_nothrow(requestSource.readHeaderEvent());

  auto request = headerEvent.headers.get();
  EXPECT_EQ(request->isSecure(), ctx->getSetupTransportInfo().secure);

  if (request->getMethod() == HTTPMethod::CONNECT) {
    // Hack to silence the expect in connectHandler
    request->getHeaders().add("Foo", "Bar");
    auto hybridSource = new HTTPHybridSource(std::move(headerEvent.headers),
                                             requestSource.release());
    hybridSource->setHeapAllocated();
    auto source =
        co_await connectHandler_.handleRequest(evb, ctx, hybridSource);

    co_return source;
  }

  if (request->getPathAsStringPiece() == "/error") {
    co_return HTTPFixedSource::makeFixedResponse(500, "Error");
  }
  if (request->getPathAsStringPiece() == "/abortHeaders") {
    co_yield folly::coro::co_error(HTTPError(HTTPErrorCode::CANCEL, "cancel"));
  }
  if (request->getPathAsStringPiece() == "/trailers") {
    XCHECK(headerEvent.eom);
    auto resp = HTTPFixedSource::makeFixedResponse(200, "Trailers");
    resp->trailers_ = std::make_unique<HTTPHeaders>();
    resp->trailers_->add("Test", "Success");
    co_return resp;
  }
  if (request->getPathAsStringPiece() == "/incompleteBody") {
    HTTPSourceReader reader;
    auto hybridSource = new HTTPHybridSource(std::move(headerEvent.headers),
                                             requestSource.release());
    hybridSource->setHeapAllocated();
    auto resp = co_await folly::coro::co_awaitTry(
        reader.setSource(hybridSource).read());
    if (resp.hasException<HTTPError>()) {
      EXPECT_NE(dynamic_cast<const HTTPError*>(resp.exception().get_exception())
                    ->code,
                HTTPErrorCode::READ_TIMEOUT);
    } else {
      EXPECT_TRUE(false) << "http exception is expected for this handler";
    }

    co_return HTTPFixedSource::makeFixedResponse(500, "failed");
  }
  if (request->getPathAsStringPiece().find("/bodyError") == 0) {
    auto path = request->getPathAsStringPiece();
    auto delimiter = std::find(path.begin(), path.end(), '_');
    co_return new ErrorSource(
        std::string("super long response that can't fit in one frame"),
        false,
        delimiter == path.end()
            ? 0
            : folly::to<uint32_t>(delimiter + 1, path.end()));
  }
  if (request->getPathAsStringPiece().find("/earlyreturn") == 0) {
    auto hybridSource = new HTTPHybridSource(std::move(headerEvent.headers),
                                             requestSource.release());
    hybridSource->setHeapAllocated();
    co_withExecutor(evb,
                    [](HTTPHybridSource* source) -> folly::coro::Task<void> {
                      HTTPSourceReader reader(source);
                      co_await reader.read();
                    }(hybridSource))
        .start();
    co_return HTTPFixedSource::makeFixedResponse(200);
  }
  // echo source
  auto resp = new EchoBodySource(
      std::move(requestSource),
      200,
      headerEvent.eom,
      {{std::string("x-method"), request->getMethodString()},
       {std::string("x-host"),
        request->getHeaders().getSingleOrEmpty(HTTP_HEADER_HOST)},
       {std::string("x-custom-header-count"),
        folly::to<std::string>(
            request->getHeaders().getNumberOfValues("custom"))},
       {std::string("src-port"),
        folly::to<std::string>(ctx->getPeerAddress().getPort())}},
      (request->getPathAsStringPiece() == "/abortBody"));
  co_return resp;
}
} // namespace proxygen::coro::test
