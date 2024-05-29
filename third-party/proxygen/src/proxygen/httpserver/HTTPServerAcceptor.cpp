/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/httpserver/HTTPServerAcceptor.h>

#include <folly/ExceptionString.h>
#include <proxygen/httpserver/RequestHandlerAdaptor.h>
#include <proxygen/httpserver/RequestHandlerFactory.h>
#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/codec/HTTP2Constants.h>
#include <proxygen/lib/http/session/HTTPDownstreamSession.h>

using folly::SocketAddress;

namespace proxygen {

AcceptorConfiguration HTTPServerAcceptor::makeConfig(
    const HTTPServer::IPConfig& ipConfig, const HTTPServerOptions& opts) {

  AcceptorConfiguration conf;
  conf.bindAddress = ipConfig.address;
  conf.connectionIdleTimeout = opts.idleTimeout;
  conf.transactionIdleTimeout = opts.idleTimeout;
  conf.initialReceiveWindow = opts.initialReceiveWindow;
  conf.receiveStreamWindowSize = opts.receiveStreamWindowSize;
  conf.receiveSessionWindowSize = opts.receiveSessionWindowSize;
  conf.acceptBacklog = opts.listenBacklog;
  conf.maxConcurrentIncomingStreams = opts.maxConcurrentIncomingStreams;

  if (opts.enableExHeaders) {
    conf.egressSettings.push_back(
        HTTPSetting(SettingsId::ENABLE_EX_HEADERS, 1));
  }

  if (ipConfig.protocol == HTTPServer::Protocol::SPDY) {
    conf.plaintextProtocol = "spdy/3.1";
  } else if (ipConfig.protocol == HTTPServer::Protocol::HTTP2) {
    conf.plaintextProtocol = http2::kProtocolCleartextString;
  } else if (opts.h2cEnabled) {
    conf.allowedPlaintextUpgradeProtocols = {http2::kProtocolCleartextString};
  }

  conf.sslContextConfigs = ipConfig.sslConfigs;
  conf.strictSSL = ipConfig.strictSSL;
  conf.allowInsecureConnectionsOnSecureServer =
      ipConfig.allowInsecureConnectionsOnSecureServer;
  conf.enableTCPFastOpen = ipConfig.enableTCPFastOpen;
  conf.fastOpenQueueSize = ipConfig.fastOpenQueueSize;
  if (ipConfig.ticketSeeds) {
    conf.initialTicketSeeds = *ipConfig.ticketSeeds;
  }
  if (ipConfig.acceptorSocketOptions.has_value()) {
    conf.setSocketOptions(ipConfig.acceptorSocketOptions.value());
    auto it = ipConfig.acceptorSocketOptions->find({SOL_SOCKET, SO_REUSEPORT});
    if (it != ipConfig.acceptorSocketOptions->end() && it->second != 0) {
      conf.reusePort = true;
    }
  }
  return conf;
}

std::unique_ptr<HTTPServerAcceptor> HTTPServerAcceptor::make(
    const AcceptorConfiguration& conf,
    const HTTPServerOptions& opts,
    const std::shared_ptr<HTTPCodecFactory>& codecFactory) {
  // Create a copy of the filter chain in reverse order since we need to create
  // Handlers in that order.
  std::vector<RequestHandlerFactory*> handlerFactories;
  for (auto& f : opts.handlerFactories) {
    handlerFactories.push_back(f.get());
  }
  std::reverse(handlerFactories.begin(), handlerFactories.end());

  return std::unique_ptr<HTTPServerAcceptor>(
      new HTTPServerAcceptor(conf, codecFactory, handlerFactories, opts));
}

HTTPServerAcceptor::HTTPServerAcceptor(
    const AcceptorConfiguration& conf,
    const std::shared_ptr<HTTPCodecFactory>& codecFactory,
    std::vector<RequestHandlerFactory*> handlerFactories,
    const HTTPServerOptions& options)
    : HTTPSessionAcceptor(conf, codecFactory),
      serverOptions_(options),
      handlerFactories_(handlerFactories) {
}

void HTTPServerAcceptor::setCompletionCallback(std::function<void()> f) {
  completionCallback_ = f;
}

HTTPServerAcceptor::~HTTPServerAcceptor() {
}

HTTPTransactionHandler* HTTPServerAcceptor::newHandler(
    HTTPTransaction& txn, HTTPMessage* msg) noexcept {

  SocketAddress clientAddr, vipAddr;
  txn.getPeerAddress(clientAddr);
  txn.getLocalAddress(vipAddr);
  msg->setClientAddress(clientAddr);
  msg->setDstAddress(vipAddr);

  // Create filters chain
  RequestHandler* h = nullptr;
  for (auto& factory : handlerFactories_) {
    h = factory->onRequest(h, msg);
  }

  return new RequestHandlerAdaptor(h);
}

void HTTPServerAcceptor::onNewConnection(
    folly::AsyncTransport::UniquePtr sock,
    const SocketAddress* address,
    const std::string& nextProtocolName,
    SecureTransportType secureTransportType,
    const wangle::TransportInfo& tinfo) {
  auto& filter = serverOptions_.newConnectionFilter;
  if (filter) {
    try {
      filter(sock.get(), address, nextProtocolName, secureTransportType, tinfo);
    } catch (const std::exception& e) {
      sock->closeWithReset();
      LOG(INFO) << "Exception filtering new socket: " << folly::exceptionStr(e);
      return;
    }
  }

  const auto& func = serverOptions_.zeroCopyEnableFunc;
  if (func && sock) {
    sock->setZeroCopy(true);
    sock->setZeroCopyEnableFunc(func);
  }

  HTTPSessionAcceptor::onNewConnection(
      std::move(sock), address, nextProtocolName, secureTransportType, tinfo);
}

void HTTPServerAcceptor::onConnectionsDrained() {
  if (completionCallback_) {
    completionCallback_();
  }
}

} // namespace proxygen
