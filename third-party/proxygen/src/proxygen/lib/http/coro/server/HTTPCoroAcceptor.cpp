/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/server/HTTPCoroAcceptor.h"
#include "proxygen/lib/http/coro/HTTPCoroSession.h"
#include <folly/logging/xlog.h>

#include <proxygen/lib/http/session/HTTPDefaultSessionCodecFactory.h>
#include <quic/api/QuicSocket.h>
#include <quic/common/events/FollyQuicEventBase.h>

namespace proxygen::coro {

HTTPCoroAcceptor::HTTPCoroAcceptor(
    std::shared_ptr<const AcceptorConfiguration> accConfig,
    std::shared_ptr<HTTPHandler> handler,
    NewConnectionFilter* newConnFilter,
    std::shared_ptr<HTTPCodecFactory> codecFactory)
    : Acceptor(accConfig),
      factory_(std::move(accConfig), std::move(codecFactory)),
      handler_(std::move(handler)),
      newConnectionFilter_(newConnFilter) {
}

HTTPCoroDownstreamSessionFactory::HTTPCoroDownstreamSessionFactory(
    std::shared_ptr<const AcceptorConfiguration> accConfig,
    std::shared_ptr<HTTPCodecFactory> codecFactory,
    Config config)
    : accConfig_(std::move(accConfig)),
      codecFactory_(std::move(codecFactory)),
      config_(config) {
  if (!codecFactory_) {
    codecFactory_ =
        std::make_shared<HTTPDefaultSessionCodecFactory>(accConfig_);
  }
}

void HTTPCoroDownstreamSessionFactory::applySettingsToCodec(HTTPCodec& codec) {
  auto settings = codec.getEgressSettings();
  if (!settings) {
    return;
  }
  if (accConfig_->maxConcurrentIncomingStreams) {
    settings->setSetting(SettingsId::MAX_CONCURRENT_STREAMS,
                         accConfig_->maxConcurrentIncomingStreams);
  }
  settings->setSetting(SettingsId::INITIAL_WINDOW_SIZE,
                       std::max(accConfig_->initialReceiveWindow,
                                accConfig_->receiveStreamWindowSize));
  for (auto& setting : accConfig_->egressSettings) {
    settings->setSetting(setting.id, setting.value);
  }
  if (config_.headerCodecStats) {
    codec.setHeaderCodecStats(config_.headerCodecStats);
  }
}

void HTTPCoroDownstreamSessionFactory::applySettingsToSession(
    HTTPCoroSession& session) {
  // PrioritiesEnabled

  session.setConnectionFlowControl(accConfig_->receiveSessionWindowSize);
  if (config_.sessionLifecycleCb) {
    session.addLifecycleObserver(config_.sessionLifecycleCb);
  }
  session.setConnectionReadTimeout(accConfig_->connectionIdleTimeout);
  session.setStreamReadTimeout(accConfig_->transactionIdleTimeout);
  session.setWriteTimeout(accConfig_->transactionIdleTimeout);
  // accConfig_->writeBufferLimit
  //
  if (config_.sessionStats) {
    session.setSessionStats(config_.sessionStats);
  }
}

HTTPCoroSession* HTTPCoroDownstreamSessionFactory::makeUniplexSession(
    folly::AsyncTransport::UniquePtr transport,
    const folly::SocketAddress* peerAddress,
    const std::string& nextProtocol,
    wangle::SecureTransportType transportType,
    const wangle::TransportInfo& tinfo,
    std::shared_ptr<HTTPHandler> handler) {
  // we assume if security protocol isn't empty, then it's TLS
  bool isTLS = !transport->getSecurityProtocol().empty();
  std::string attemptNextProtocol =
      isTLS ? nextProtocol : accConfig_->plaintextProtocol;
  auto codec = codecFactory_->getCodec(
      attemptNextProtocol, TransportDirection::DOWNSTREAM, isTLS);
  if (!codec) {
    XLOG(DBG2) << "codecFactory_ failed to provide codec for proto="
               << attemptNextProtocol;
    return nullptr;
  }

  applySettingsToCodec(*codec);
  XLOG(DBG4) << "Created new " << attemptNextProtocol << " session for peer "
             << *peerAddress;
  auto eventBase = transport->getEventBase();
  auto coroTransport =
      std::make_unique<folly::coro::Transport>(eventBase, std::move(transport));
  HTTPCoroSession* session = HTTPCoroSession::makeDownstreamCoroSession(
      std::move(coroTransport), std::move(handler), std::move(codec), tinfo);
  applySettingsToSession(*session);
  // readBufferLimit is only read for http/1.1 sessions
  session->setReadBufferLimit(accConfig_->receiveStreamWindowSize);
  return session;
}

HTTPCoroSession* HTTPCoroDownstreamSessionFactory::makeQuicSession(
    std::shared_ptr<quic::QuicSocket> quicSocket,
    wangle::TransportInfo tinfo,
    std::shared_ptr<HTTPHandler> handler,
    bool /*strictValidation*/) {
  auto codec =
      hq::HQMultiCodec::Factory::getCodec(TransportDirection::DOWNSTREAM,
                                          codecFactory_->useStrictValidation(),
                                          accConfig_->headerIndexingStrategy);
  applySettingsToCodec(*codec);
  XLOG(DBG4) << "Created new " << *tinfo.appProtocol << " session for peer "
             << quicSocket->getPeerAddress();
  HTTPCoroSession* session =
      HTTPCoroSession::makeDownstreamCoroSession(std::move(quicSocket),
                                                 std::move(handler),
                                                 std::move(codec),
                                                 std::move(tinfo));
  applySettingsToSession(*session);
  session->setReadBufferLimit(accConfig_->receiveStreamWindowSize);
  return session;
}

void HTTPCoroAcceptor::onNewConnection(
    folly::AsyncTransport::UniquePtr transport,
    const folly::SocketAddress* peerAddress,
    const std::string& nextProtocol,
    wangle::SecureTransportType transportType,
    const wangle::TransportInfo& tinfo) {
  if (newConnectionFilter_ && *newConnectionFilter_) {
    auto pass = folly::makeTryWith([&] {
      return (*newConnectionFilter_)(peerAddress,
                                     transport->getPeerCertificate(),
                                     nextProtocol,
                                     transportType,
                                     tinfo);
    });
    if (pass.hasException() || !*pass) {
      transport->closeWithReset();
      return;
    }
  }

  auto eventBase = transport->getEventBase();
  auto session = factory_.makeUniplexSession(std::move(transport),
                                             peerAddress,
                                             nextProtocol,
                                             transportType,
                                             tinfo,
                                             handler_);
  if (session) {
    onSessionReady(eventBase, session);
  } else {
    onSessionCreationError(ProxygenError::kErrorUnsupportedScheme);
  }
}

void HTTPCoroAcceptor::onSessionReady(folly::EventBase* eventBase,
                                      HTTPCoroSession* session) {
  Acceptor::addConnection(session);

  // Start the coro
  session->run().start();
}

void HTTPCoroAcceptor::onNewConnection(
    std::shared_ptr<quic::QuicSocket> quicSocket, wangle::TransportInfo tinfo) {
  if (newConnectionFilter_ && *newConnectionFilter_) {
    auto nextProtocol = quicSocket->getAppProtocol().value_or("");
    auto peerAddr = quicSocket->getPeerAddress();
    auto pass = folly::makeTryWith([&] {
      return (*newConnectionFilter_)(&peerAddr,
                                     quicSocket->getPeerCertificate().get(),
                                     nextProtocol,
                                     wangle::SecureTransportType::TLS,
                                     tinfo);
    });
    if (pass.hasException() || !*pass) {
      quicSocket->closeNow(
          quic::QuicError(quic::TransportErrorCode::INTERNAL_ERROR,
                          (pass.hasException() ? pass.exception().what().c_str()
                                               : "Connection Filtered")));
      return;
    }
  }
  auto eventBase = quicSocket->getEventBase()
                       ->getTypedEventBase<quic::FollyQuicEventBase>()
                       ->getBackingEventBase();
  auto session = factory_.makeQuicSession(std::move(quicSocket),
                                          std::move(tinfo),
                                          handler_,
                                          /*strictValidation=*/true);
  XCHECK(session);
  onSessionReady(eventBase, session);
}

} // namespace proxygen::coro
