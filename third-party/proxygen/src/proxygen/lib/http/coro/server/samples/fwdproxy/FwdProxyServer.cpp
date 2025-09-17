/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/HTTPCoroSession.h"
#include "proxygen/lib/http/coro/HTTPFixedSource.h"
#include "proxygen/lib/http/coro/HTTPHybridSource.h"
#include "proxygen/lib/http/coro/client/CoroDNSResolver.h"
#include "proxygen/lib/http/coro/client/HTTPClientConnectionCache.h"
#include "proxygen/lib/http/coro/server/HTTPServer.h"
#include "proxygen/lib/http/coro/server/samples/fwdproxy/ConnectSource.h"
#include <folly/init/Init.h>
#include <folly/io/async/EventBaseLocal.h>
#include <folly/logging/xlog.h>
#include <folly/portability/GFlags.h>
#include <memory>

DEFINE_int32(port, 8082, "What port to listen on");
DEFINE_string(cert, "", "Certificate file");
DEFINE_string(key, "", "Key file");
DEFINE_string(plaintext_proto, "", "Plaintext protocol");
DEFINE_bool(quic, false, "Enable QUIC (requires cert/key)");
DEFINE_int32(timeout, 5000, "Uber timeout");

using namespace proxygen;
using namespace proxygen::coro;

namespace {

class FwdProxyHandler : public proxygen::coro::HTTPHandler {
 public:
  folly::coro::Task<HTTPSourceHolder> handleRequest(
      folly::EventBase* evb,
      HTTPSessionContextPtr /*ctx*/,
      HTTPSourceHolder requestSource) override {
    auto headerEvent = co_await co_awaitTry(requestSource.readHeaderEvent());
    if (headerEvent.hasException()) {
      co_return HTTPFixedSource::makeFixedResponse(400);
    }
    auto& request = headerEvent->headers;
    auto parseURL = ParseURL::parseURL(request->getURL());
    if (!parseURL->valid() || !parseURL->hasHost()) {
      XLOG(ERR) << "Invalid url=" << request->getURL();
      co_return HTTPFixedSource::makeFixedResponse(400);
    }

    // Handle connect separately
    if (request->getMethod() == HTTPMethod::CONNECT) {
      if (parseURL->port() == 0) {
        XLOG(ERR) << "Invalid url=" << request->getURL();
        co_return HTTPFixedSource::makeFixedResponse(400);
      }
      co_return co_await handleConnect(evb,
                                       parseURL->host().str(),
                                       parseURL->port(),
                                       std::move(*headerEvent),
                                       std::move(requestSource));
    }

    if (parseURL->scheme().empty()) {
      XLOG(ERR) << "Invalid url=" << request->getURL();
      co_return HTTPFixedSource::makeFixedResponse(400);
    }

    XLOG(DBG4) << "Sending request upstream";
    // any exceptions are propagated
    auto& connCache = connCache_.try_emplace(*evb, *evb);
    auto res = co_await co_awaitTry(connCache.getSessionWithReservation(
        request->getURL(), std::chrono::milliseconds(FLAGS_timeout)));
    if (res.hasException()) {
      XLOG(ERR) << "Failed to connect err=" << res.exception().what();
      co_return HTTPFixedSource::makeFixedResponse(503);
    }
    URL reqURL(std::move(*parseURL));
    request->setURL(reqURL.makeRelativeURL());
    request->getHeaders().set(HTTP_HEADER_HOST,
                              reqURL.getHostAndPortOmitDefault());
    auto newSource = new HTTPHybridSource(std::move(headerEvent->headers),
                                          std::move(requestSource));
    newSource->setHeapAllocated();
    co_return co_await res->session->sendRequest(newSource,
                                                 std::move(res->reservation));
  }

  folly::coro::Task<HTTPSourceHolder> handleConnect(
      folly::EventBase* evb,
      std::string host,
      uint16_t port,
      HTTPHeaderEvent headerEvent,
      HTTPSourceHolder requestSource) {
    auto serverAddresses = co_await co_awaitTry(CoroDNSResolver::resolveHost(
        evb, host, std::chrono::milliseconds(FLAGS_timeout)));
    if (serverAddresses.hasException()) {
      XLOG(ERR) << "DNS error: " << serverAddresses.exception().what();
      co_return HTTPFixedSource::makeFixedResponse(503);
    }
    serverAddresses->primary.setPort(port);
    auto transport =
        co_await co_awaitTry(folly::coro::Transport::newConnectedSocket(
            evb,
            serverAddresses->primary,
            std::chrono::milliseconds(FLAGS_timeout)));
    if (transport.hasException()) {
      XLOG(ERR) << "Connect error: " << transport.exception().what();
      co_return HTTPFixedSource::makeFixedResponse(503);
    }
    auto connectSource = new ConnectSource(
        std::make_unique<folly::coro::Transport>(std::move(*transport)),
        std::move(requestSource));
    connectSource->setWriteTimeout(std::chrono::milliseconds(FLAGS_timeout));
    if (!headerEvent.eom) {
      co_withExecutor(evb, connectSource->readRequestSendUpstream()).start();
    }
    co_return connectSource;
  }

 private:
  folly::EventBaseLocal<HTTPClientConnectionCache> connCache_;
};

} // namespace

int main(int argc, char** argv) {
  const folly::Init init(&argc, &argv);
  ::gflags::ParseCommandLineFlags(&argc, &argv, false);

  HTTPServer::Config httpServerConfig;
  httpServerConfig.socketConfig.bindAddress.setFromLocalPort(FLAGS_port);
  httpServerConfig.plaintextProtocol = FLAGS_plaintext_proto;

  if (!FLAGS_cert.empty()) {
    auto tlsConfig = HTTPServer::getDefaultTLSConfig();
    try {
      tlsConfig.setCertificate(FLAGS_cert, FLAGS_key, "");
    } catch (const std::exception& ex) {
      XLOG(ERR) << "Invalid certificate file or key file: %s" << ex.what();
    }
    httpServerConfig.socketConfig.sslContextConfigs.emplace_back(
        std::move(tlsConfig));
    if (FLAGS_quic) {
      httpServerConfig.quicConfig = HTTPServer::QuicConfig();
    }
  } else if (FLAGS_quic) {
    XLOG(ERR) << "QUIC requires a cert and key";
    return 1;
  }
  HTTPServer server(std::move(httpServerConfig),
                    std::make_shared<FwdProxyHandler>());
  server.start();

  return 0;
}
