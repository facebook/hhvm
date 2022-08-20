/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/httpserver/HTTPServer.h>
#include <proxygen/httpserver/samples/hq/HQCommandLine.h>
#include <proxygen/httpserver/samples/hq/HQServer.h>

namespace quic::samples {

class H2Server {
  class SampleHandlerFactory : public proxygen::RequestHandlerFactory {
   public:
    explicit SampleHandlerFactory(
        HTTPTransactionHandlerProvider httpTransactionHandlerProvider);

    void onServerStart(folly::EventBase* /*evb*/) noexcept override;

    void onServerStop() noexcept override;

    proxygen::RequestHandler* onRequest(
        proxygen::RequestHandler* /* handler */,
        proxygen::HTTPMessage* /* msg */) noexcept override;

   private:
    HTTPTransactionHandlerProvider httpTransactionHandlerProvider_;
  }; // SampleHandlerFactory

 public:
  static std::unique_ptr<proxygen::HTTPServerOptions> createServerOptions(
      const HQToolServerParams& params,
      HTTPTransactionHandlerProvider httpTransactionHandlerProvider);
  using AcceptorConfig = std::vector<proxygen::HTTPServer::IPConfig>;
  static std::unique_ptr<AcceptorConfig> createServerAcceptorConfig(
      const HQToolServerParams& /* params */);
  // Starts H2 server in a background thread
  static std::thread run(
      const HQToolServerParams& params,
      HTTPTransactionHandlerProvider httpTransactionHandlerProvider);
};

} // namespace quic::samples
