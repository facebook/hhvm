/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/httpserver/HTTPTransactionHandlerAdaptor.h>
#include <proxygen/httpserver/samples/hq/FizzContext.h>
#include <proxygen/httpserver/samples/hq/H2Server.h>

namespace quic::samples {

using namespace proxygen;

H2Server::SampleHandlerFactory::SampleHandlerFactory(
    HTTPTransactionHandlerProvider httpTransactionHandlerProvider)
    : httpTransactionHandlerProvider_(
          std::move(httpTransactionHandlerProvider)) {
}

void H2Server::SampleHandlerFactory::onServerStart(
    folly::EventBase* /*evb*/) noexcept {
}

void H2Server::SampleHandlerFactory::onServerStop() noexcept {
}

RequestHandler* H2Server::SampleHandlerFactory::onRequest(
    RequestHandler*, HTTPMessage* msg) noexcept {
  return new HTTPTransactionHandlerAdaptor(
      httpTransactionHandlerProvider_(msg));
}

std::unique_ptr<proxygen::HTTPServerOptions> H2Server::createServerOptions(
    const HQToolServerParams& params,
    HTTPTransactionHandlerProvider httpTransactionHandlerProvider) {
  auto serverOptions = std::make_unique<proxygen::HTTPServerOptions>();

  serverOptions->threads = params.httpServerThreads;
  serverOptions->idleTimeout = params.httpServerIdleTimeout;
  serverOptions->shutdownOn = params.httpServerShutdownOn;
  serverOptions->enableContentCompression =
      params.httpServerEnableContentCompression;
  serverOptions->initialReceiveWindow =
      params.transportSettings
          .advertisedInitialBidiLocalStreamFlowControlWindow;
  serverOptions->receiveStreamWindowSize =
      params.transportSettings
          .advertisedInitialBidiLocalStreamFlowControlWindow;
  serverOptions->receiveSessionWindowSize =
      params.transportSettings.advertisedInitialConnectionFlowControlWindow;
  serverOptions->handlerFactories =
      proxygen::RequestHandlerChain()
          .addThen<SampleHandlerFactory>(
              std::move(httpTransactionHandlerProvider))
          .build();
  return serverOptions;
}

std::unique_ptr<H2Server::AcceptorConfig> H2Server::createServerAcceptorConfig(
    const HQToolServerParams& params) {
  auto acceptorConfig = std::make_unique<AcceptorConfig>();
  proxygen::HTTPServer::IPConfig ipConfig(
      params.localH2Address.value(), proxygen::HTTPServer::Protocol::HTTP2);
  ipConfig.sslConfigs.emplace_back(createSSLContext(params));
  acceptorConfig->push_back(ipConfig);
  return acceptorConfig;
}

std::thread H2Server::run(
    const HQToolServerParams& params,
    HTTPTransactionHandlerProvider httpTransactionHandlerProvider) {

  // Start HTTPServer mainloop in a separate thread
  std::thread t([params = folly::copy(params),
                 httpTransactionHandlerProvider =
                     std::move(httpTransactionHandlerProvider)]() mutable {
    {
      auto acceptorConfig = createServerAcceptorConfig(params);
      auto serverOptions = createServerOptions(
          params, std::move(httpTransactionHandlerProvider));
      proxygen::HTTPServer server(std::move(*serverOptions));
      server.bind(std::move(*acceptorConfig));
      server.start();
    }
    // HTTPServer traps the SIGINT.  resignal HQServer
    raise(SIGINT);
  });

  return t;
}

} // namespace quic::samples
