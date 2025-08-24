/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/HTTPFixedSource.h"
#include "proxygen/lib/http/coro/server/HTTPServer.h"
#include <folly/logging/xlog.h>
// TODO: move handler to it's own file?
#include "proxygen/lib/http/coro/HTTPCoroSession.h"
#include <quic/QuicConstants.h>

#include <folly/init/Init.h>
#include <folly/portability/GFlags.h>

DEFINE_int32(port, 80, "What port to listen on");
DEFINE_string(cert, "", "Certificate file");
DEFINE_string(key, "", "Key file");
DEFINE_string(plaintext_proto, "", "Plaintext protocol");
DEFINE_bool(quic, false, "Enable QUIC (requires cert/key)");
DEFINE_uint32(quic_batching_mode,
              static_cast<uint32_t>(quic::QuicBatchingMode::BATCHING_MODE_NONE),
              "QUIC batching mode");
DEFINE_uint32(quic_batch_size,
              quic::kDefaultQuicMaxBatchSize,
              "Maximum number of packets that can be batched in Quic");
DEFINE_int32(threads, 1, "Number of HTTPServer IO threads");

using namespace proxygen::coro;
using namespace proxygen;

namespace {

class EchoResponse : public HTTPSource {
 public:
  explicit EchoResponse(HTTPSourceHolder request)
      : request_(std::move(request)) {
    setHeapAllocated(); // EchoResponse is always heap allocated
  }

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override {
    auto headerEvent = co_await co_awaitTry(request_.readHeaderEvent());
    auto guard = folly::makeGuard(lifetime(headerEvent));
    if (headerEvent.hasException()) {
      co_yield folly::coro::co_error(
          HTTPError(HTTPErrorCode::CANCEL, "input error"));
    }
    auto path = headerEvent->headers->getPathAsStringPiece();
    if (path == "/push") {
      if (headerEvent->eom) {
        XLOG(ERR) << "Can only push in reply to POST";
      }
      pendingPush_ = true;
    }
    auto msg = std::make_unique<HTTPMessage>();
    msg->setStatusCode(200);
    headerEvent->headers->getHeaders().forEach(
        [&msg](std::string& name, std::string& value) {
          msg->getHeaders().set(folly::to<std::string>("x-echo-", name), value);
        });
    co_return HTTPHeaderEvent(std::move(msg), headerEvent->eom);
  }

  folly::coro::Task<HTTPBodyEvent> readBodyEvent(uint32_t max) override {
    if (pendingPush_) {
      pendingPush_ = false;
      auto promise = std::make_unique<HTTPMessage>();
      promise->setURL("/");
      promise->getHeaders().set(HTTP_HEADER_HOST, "foo.com");
      co_return HTTPBodyEvent(
          std::move(promise),
          HTTPFixedSource::makeFixedResponse(
              200, folly::IOBuf::copyBuffer("push it real good")),
          false);
    }
    auto bodyEvent = co_await co_awaitTry(request_.readBodyEvent(max));
    auto guard = folly::makeGuard(lifetime(bodyEvent));
    if (bodyEvent.hasException()) {
      co_yield folly::coro::co_error(
          HTTPError(HTTPErrorCode::CANCEL, "input error"));
    }
    co_return std::move(*bodyEvent);
  }

  void stopReading(
      folly::Optional<const proxygen::coro::HTTPErrorCode>) override {
    // This could run into trouble if there is an active guard on the stack?
    delete this;
  }

 private:
  HTTPSourceHolder request_;
  bool pendingPush_{false};
};

class EchoHandler
    : public HTTPHandler
    , public HTTPServer::Observer {
 public:
  folly::coro::Task<HTTPSourceHolder> handleRequest(
      folly::EventBase* /*evb*/,
      HTTPSessionContextPtr /*ctx*/,
      HTTPSourceHolder requestSource) override {
    co_return new EchoResponse(std::move(requestSource));
  }
  void onThreadStart(folly::EventBase* /*evb*/) override {
    XLOG(DBG2) << "Thread started";
  }
  void onThreadStop(folly::EventBase* /*evb*/) override {
    XLOG(DBG2) << "Thread stopped";
  }
};

} // namespace

int main(int argc, char** argv) {
  folly::init(&argc, &argv);
  ::gflags::ParseCommandLineFlags(&argc, &argv, false);

  HTTPServer::Config httpServerConfig;
  httpServerConfig.socketConfig.bindAddress.setFromLocalPort(FLAGS_port);
  httpServerConfig.plaintextProtocol = FLAGS_plaintext_proto;
  httpServerConfig.numIOThreads = FLAGS_threads;

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
      httpServerConfig.quicConfig->transportSettings.batchingMode =
          quic::getQuicBatchingMode(FLAGS_quic_batching_mode);
      httpServerConfig.quicConfig->transportSettings.maxBatchSize =
          FLAGS_quic_batch_size;
    }
  } else if (FLAGS_quic) {
    XLOG(ERR) << "QUIC requires a cert and key";
    return 1;
  }
  auto handler = std::make_shared<EchoHandler>();
  HTTPServer server(std::move(httpServerConfig), handler);
  server.addObserver(handler.get());
  server.start();

  return 0;
}
