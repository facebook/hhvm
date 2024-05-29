/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/httpserver/HTTPServer.h>
#include <proxygen/httpserver/HTTPServerOptions.h>
#include <proxygen/lib/http/session/HTTPSessionAcceptor.h>

namespace proxygen {

class HTTPServerAcceptor final : public HTTPSessionAcceptor {
 public:
  static AcceptorConfiguration makeConfig(const HTTPServer::IPConfig& ipConfig,
                                          const HTTPServerOptions& opts);

  static std::unique_ptr<HTTPServerAcceptor> make(
      const AcceptorConfiguration& conf,
      const HTTPServerOptions& opts,
      const std::shared_ptr<HTTPCodecFactory>& codecFactory = nullptr);

  /**
   * Invokes the given method when all the connections are drained
   */
  void setCompletionCallback(std::function<void()> f);

  ~HTTPServerAcceptor() override;

 private:
  HTTPServerAcceptor(const AcceptorConfiguration& conf,
                     const std::shared_ptr<HTTPCodecFactory>& codecFactory,
                     std::vector<RequestHandlerFactory*> handlerFactories,
                     const HTTPServerOptions& options);

  // HTTPSessionAcceptor
  HTTPTransaction::Handler* newHandler(HTTPTransaction& txn,
                                       HTTPMessage* msg) noexcept override;

  void onNewConnection(folly::AsyncTransport::UniquePtr sock,
                       const folly::SocketAddress* address,
                       const std::string& nextProtocolName,
                       wangle::SecureTransportType secureTransportType,
                       const wangle::TransportInfo& tinfo) override;

  void onConnectionsDrained() override;

  const HTTPServerOptions& serverOptions_;
  std::function<void()> completionCallback_;
  const std::vector<RequestHandlerFactory*> handlerFactories_{nullptr};
};

} // namespace proxygen
