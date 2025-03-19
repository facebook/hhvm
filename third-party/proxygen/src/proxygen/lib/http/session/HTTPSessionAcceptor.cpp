/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HTTPSessionAcceptor.h>

#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/codec/HTTP2Codec.h>
#include <proxygen/lib/http/session/HTTPDefaultSessionCodecFactory.h>
#include <proxygen/lib/http/session/HTTPDirectResponseHandler.h>

using folly::SocketAddress;
using std::string;
using std::unique_ptr;

namespace proxygen {

const SocketAddress HTTPSessionAcceptor::unknownSocketAddress_("0.0.0.0", 0);

HTTPSessionAcceptor::HTTPSessionAcceptor(
    std::shared_ptr<const AcceptorConfiguration> accConfig)
    : HTTPSessionAcceptor(std::move(accConfig), nullptr) {
}

HTTPSessionAcceptor::HTTPSessionAcceptor(
    std::shared_ptr<const AcceptorConfiguration> accConfig,
    std::shared_ptr<HTTPCodecFactory> codecFactory)
    : HTTPAcceptor(std::move(accConfig)),
      codecFactory_(codecFactory),
      simpleController_(std::make_shared<SimpleController>(this)) {
  if (!codecFactory_) {
    codecFactory_ =
        std::make_shared<HTTPDefaultSessionCodecFactory>(getConfig());
  }
}

HTTPSessionAcceptor::~HTTPSessionAcceptor() {
}

const HTTPErrorPage* HTTPSessionAcceptor::getErrorPage(
    const SocketAddress& addr) const {
  return defaultErrorPage_.get();
}

void HTTPSessionAcceptor::onNewConnection(folly::AsyncTransport::UniquePtr sock,
                                          const SocketAddress* peerAddress,
                                          const string& nextProtocol,
                                          wangle::SecureTransportType,
                                          const wangle::TransportInfo& tinfo) {

  unique_ptr<HTTPCodec> codec = codecFactory_->getCodec(
      nextProtocol,
      TransportDirection::DOWNSTREAM,
      // we assume if security protocol isn't empty, then it's TLS
      !sock->getSecurityProtocol().empty());

  if (!codec) {
    VLOG(2) << "codecFactory_ failed to provide codec";
    onSessionCreationError(ProxygenError::kErrorUnsupportedScheme);
    return;
  }
  auto egressSettings = codec->getEgressSettings();
  if (egressSettings && setEnableConnectProtocol_) {
    egressSettings->setSetting(SettingsId::ENABLE_CONNECT_PROTOCOL, 1);
  }

  auto controller = getController();
  SocketAddress localAddress;
  try {
    sock->getLocalAddress(&localAddress);
  } catch (...) {
    VLOG(3) << "couldn't get local address for socket";
    localAddress = unknownSocketAddress_;
  }

  // overwrite address if the socket has no IP, e.g. Unix domain socket
  if (!localAddress.isFamilyInet()) {
    if (getConfig()->bindAddress.isFamilyInet()) {
      localAddress = getConfig()->bindAddress;
    } else {
      localAddress = unknownSocketAddress_;
    }
    VLOG(4) << "set localAddress=" << localAddress.describe();
  }

  auto sessionInfoCb = sessionInfoCb_ ? sessionInfoCb_ : this;
  VLOG(4) << "Created new " << nextProtocol << " session for peer "
          << *peerAddress;
  auto codecProtocol = codec->getProtocol();
  HTTPDownstreamSession* session =
      new HTTPDownstreamSession(getTransactionTimeoutSet(),
                                std::move(sock),
                                localAddress,
                                *peerAddress,
                                controller.get(),
                                std::move(codec),
                                tinfo,
                                sessionInfoCb);
  if (getConfig()->maxConcurrentIncomingStreams) {
    session->setMaxConcurrentIncomingStreams(
        getConfig()->maxConcurrentIncomingStreams);
  }
  session->setEgressSettings(getConfig()->egressSettings);

  // set HTTP2 priorities flag on session object
  auto HTTP2PrioritiesEnabled = getHttp2PrioritiesEnabled();
  session->setHTTP2PrioritiesEnabled(HTTP2PrioritiesEnabled);

  // set flow control parameters
  session->setFlowControl(getConfig()->initialReceiveWindow,
                          getConfig()->receiveStreamWindowSize,
                          getConfig()->receiveSessionWindowSize);
  // TODO(@damlaj): support server early resp for http/3
  if (getConfig()->serverEarlyResponseEnabled &&
      codecProtocol == CodecProtocol::HTTP_2) {
    session->enableServerEarlyResponse();
  }
  if (getConfig()->writeBufferLimit > 0) {
    session->setWriteBufferLimit(getConfig()->writeBufferLimit);
  }
  session->setSessionStats(downstreamSessionStats_);
  Acceptor::addConnection(session);
  startSession(*session);
}

size_t HTTPSessionAcceptor::dropIdleConnections(size_t num) {
  // release in batch for more efficiency
  VLOG(6) << "attempt to drop downstream idle connections";
  return downstreamConnectionManager_->dropIdleConnections(num);
}

} // namespace proxygen
