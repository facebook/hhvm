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
#include "proxygen/lib/http/coro/client/HTTPCoroSessionPool.h"
#include "proxygen/lib/http/coro/server/HTTPServer.h"
#include <folly/init/Init.h>
#include <folly/logging/xlog.h>
#include <folly/portability/GFlags.h>
#include <memory>

DEFINE_int32(port, 8082, "What port to listen on");
DEFINE_string(cert, "", "Certificate file");
DEFINE_string(key, "", "Key file");
DEFINE_string(plaintext_proto, "", "Plaintext protocol");
DEFINE_bool(quic, false, "Enable QUIC (requires cert/key)");

DEFINE_string(backend_server, "::1", "Backend server address");
DEFINE_int32(backend_port, 80, "Backend server port");
DEFINE_bool(backend_tls, false, "Backend server TLS (y/n)");
DEFINE_bool(fbssl, false, "Add FBSSL: ON");
// TODO: backend QUIC?

using namespace proxygen;
using namespace proxygen::coro;

namespace {
static HTTPCoroSessionPool& getPool(folly::EventBase* evb) {
  if (FLAGS_backend_tls) {
    static auto connParams = HTTPCoroConnector::defaultConnectionParams();
    connParams.fizzContextAndVerifier =
        HTTPCoroConnector::makeFizzClientContext(
            HTTPCoroConnector::defaultTLSParams());
    static HTTPCoroSessionPool pool(evb,
                                    FLAGS_backend_server,
                                    FLAGS_backend_port,
                                    HTTPCoroSessionPool::PoolParams(),
                                    connParams);
    return pool;
  } else {
    static HTTPCoroSessionPool pool(
        evb, FLAGS_backend_server, FLAGS_backend_port);
    return pool;
  }
}

class ProxyHandler : public proxygen::coro::HTTPHandler {
 public:
  folly::coro::Task<HTTPSourceHolder> handleRequest(
      folly::EventBase* evb,
      HTTPSessionContextPtr /*ctx*/,
      HTTPSourceHolder requestSource) override {
    auto headerEvent = co_await co_awaitTry(requestSource.readHeaderEvent());
    if (headerEvent.hasException()) {
      co_return HTTPFixedSource::makeFixedResponse(400);
    }

    auto& pool = getPool(evb);
    XLOG(DBG4) << "Sending request upstream";
    if (FLAGS_fbssl && !FLAGS_backend_tls) {
      headerEvent->headers->getHeaders().add("FBSSL", "ON");
    }
    // any exceptions are propagated
    auto res = co_await co_awaitTry(pool.getSessionWithReservation());
    if (res.hasException()) {
      XLOG(ERR)
          << "Failed to connect err="
          << uint64_t(
                 res.tryGetExceptionObject<HTTPCoroSessionPool::Exception>()
                     ->type);
      co_return HTTPFixedSource::makeFixedResponse(503);
    }
    co_return co_await res->session->sendRequest(
        new HTTPHybridSource(std::move(headerEvent->headers),
                             std::move(requestSource)),
        std::move(res->reservation));
  }
};

} // namespace

int main(int argc, char** argv) {
  folly::init(&argc, &argv);
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
                    std::make_shared<ProxyHandler>());
  server.start();

  return 0;
}
