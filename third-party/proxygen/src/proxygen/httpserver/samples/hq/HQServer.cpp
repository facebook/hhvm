/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/httpserver/samples/hq/HQServer.h>

#include <ostream>
#include <quic/common/udpsocket/FollyQuicAsyncUDPSocket.h>
#include <string>

#include <folly/io/async/EventBaseLocal.h>
#include <proxygen/httpserver/samples/hq/FizzContext.h>
#include <proxygen/httpserver/samples/hq/H1QDownstreamSession.h>
#include <proxygen/httpserver/samples/hq/HQLoggerHelper.h>
#include <proxygen/lib/http/session/HQDownstreamSession.h>
#include <proxygen/lib/http/session/HTTPDownstreamSession.h>
#include <quic/api/QuicStreamAsyncTransport.h>
#include <quic/server/QuicSharedUDPSocketFactory.h>

using fizz::server::FizzServerContext;
using quic::QuicServerTransport;

namespace {

using namespace quic::samples;
using namespace proxygen;

/**
 * HQSessionController creates new HQSession objects
 *
 * Each HQSessionController object can create only a single session
 * object. TODO: consider changing it.
 *
 * Once the created session object finishes (and detaches), the
 * associated HQController is destroyed. There is no need to
 * explicitly keep track of these objects.
 */
class HQSessionController
    : public proxygen::HTTPSessionController
    , public proxygen::HTTPSessionBase::InfoCallback {
 public:
  using StreamData = std::pair<folly::IOBufQueue, bool>;

  explicit HQSessionController(
      const HQServerParams& /* params */,
      const HTTPTransactionHandlerProvider&,
      std::function<void(HQSession*)> onTransportReadyFn = nullptr);

  ~HQSessionController() override = default;

  void onDestroy(const proxygen::HTTPSessionBase& /* session*/) override;

  proxygen::HTTPTransactionHandler* getRequestHandler(
      proxygen::HTTPTransaction& /*txn*/,
      proxygen::HTTPMessage* /* msg */) override;

  proxygen::HTTPTransactionHandler* getParseErrorHandler(
      proxygen::HTTPTransaction* /*txn*/,
      const proxygen::HTTPException& /*error*/,
      const folly::SocketAddress& /*localAddress*/) override;

  proxygen::HTTPTransactionHandler* getTransactionTimeoutHandler(
      proxygen::HTTPTransaction* /*txn*/,
      const folly::SocketAddress& /*localAddress*/) override;

  void attachSession(proxygen::HTTPSessionBase* /*session*/) override;

  // The controller instance will be destroyed after this call.
  void detachSession(const proxygen::HTTPSessionBase* /*session*/) override;

  void onTransportReady(proxygen::HTTPSessionBase* /*session*/) override;
  void onTransportReady(const proxygen::HTTPSessionBase&) override {
  }

 private:
  // The owning session. NOTE: this must be a plain pointer to
  // avoid circular references
  proxygen::HQSession* session_{nullptr};
  // Provider of HTTPTransactionHandler, owned by HQServerTransportFactory
  const HTTPTransactionHandlerProvider& httpTransactionHandlerProvider_;
  std::function<void(HQSession*)> onTransportReadyFn_;
  uint64_t sessionCount_{0};
};

HQSessionController::HQSessionController(
    const HQServerParams& params,
    const HTTPTransactionHandlerProvider& httpTransactionHandlerProvider,
    std::function<void(HQSession*)> onTransportReadyFn)
    : httpTransactionHandlerProvider_(httpTransactionHandlerProvider),
      onTransportReadyFn_(std::move(onTransportReadyFn)) {
}

void HQSessionController::onTransportReady(HTTPSessionBase* /*session*/) {
  if (onTransportReadyFn_) {
    onTransportReadyFn_(session_);
  }
}

void HQSessionController::onDestroy(const HTTPSessionBase&) {
}

HTTPTransactionHandler* HQSessionController::getRequestHandler(
    HTTPTransaction& /*txn*/, HTTPMessage* msg) {
  return httpTransactionHandlerProvider_(msg);
}

HTTPTransactionHandler* FOLLY_NULLABLE
HQSessionController::getParseErrorHandler(
    HTTPTransaction* /*txn*/,
    const HTTPException& /*error*/,
    const folly::SocketAddress& /*localAddress*/) {
  return nullptr;
}

HTTPTransactionHandler* FOLLY_NULLABLE
HQSessionController::getTransactionTimeoutHandler(
    HTTPTransaction* /*txn*/, const folly::SocketAddress& /*localAddress*/) {
  return nullptr;
}

void HQSessionController::attachSession(HTTPSessionBase* /*session*/) {
  sessionCount_++;
}

void HQSessionController::detachSession(const HTTPSessionBase* /*session*/) {
  if (--sessionCount_ == 0) {
    delete this;
  }
}

} // namespace

namespace quic::samples {

HQServerTransportFactory::HQServerTransportFactory(
    const HQServerParams& params,
    HTTPTransactionHandlerProvider httpTransactionHandlerProvider,
    std::function<void(proxygen::HQSession*)> onTransportReadyFn)
    : params_(params),
      httpTransactionHandlerProvider_(
          std::move(httpTransactionHandlerProvider)),
      onTransportReadyFn_(std::move(onTransportReadyFn)) {
  alpnHandlers_[kHQ] = [this](std::shared_ptr<quic::QuicSocket> quicSocket,
                              wangle::ConnectionManager* connMgr) {
    quicSocket->setConnectionSetupCallback(nullptr);
    return new H1QDownstreamSession(
        std::move(quicSocket),
        new HQSessionController(
            params_, httpTransactionHandlerProvider_, onTransportReadyFn_),
        connMgr);
  };
}

QuicServerTransport::Ptr HQServerTransportFactory::make(
    folly::EventBase* evb,
    std::unique_ptr<quic::FollyAsyncUDPSocketAlias> socket,
    const folly::SocketAddress& /* peerAddr */,
    quic::QuicVersion,
    std::shared_ptr<const FizzServerContext> ctx) noexcept {
  auto transport = quic::QuicHandshakeSocketHolder::makeServerTransport(
      evb, std::move(socket), std::move(ctx), this);
  if (!params_.qLoggerPath.empty()) {
    transport->setQLogger(std::make_shared<HQLoggerHelper>(
        params_.qLoggerPath, params_.prettyJson, quic::VantagePoint::Server));
  }
  return transport;
}

void HQServerTransportFactory::onQuicTransportReady(
    std::shared_ptr<quic::QuicSocket> quicSocket) {
  auto alpn = quicSocket->getAppProtocol();
  auto it = alpnHandlers_.end();
  if (alpn) {
    it = alpnHandlers_.find(*alpn);
  }
  auto qevb = quicSocket->getEventBase();
  folly::EventBase* evb{nullptr};
  if (qevb) {
    evb = qevb->getTypedEventBase<quic::FollyQuicEventBase>()
              ->getBackingEventBase();
  }

  if (it == alpnHandlers_.end()) {
    // by default, it's H3
    handleHQAlpn(std::move(quicSocket), getConnectionManager(evb));
  } else {
    it->second(std::move(quicSocket), getConnectionManager(evb));
  }
}

void HQServerTransportFactory::onConnectionSetupError(
    std::shared_ptr<quic::QuicSocket>, quic::QuicError code) {
  LOG(ERROR) << "Failed to accept QUIC connection: " << code.message;
}

wangle::ConnectionManager* HQServerTransportFactory::getConnectionManager(
    folly::EventBase* evb) {
  auto connMgrPtrPtr = connMgr_.get(*evb);
  wangle::ConnectionManager* connMgr{nullptr};
  if (connMgrPtrPtr) {
    connMgr = (*connMgrPtrPtr).get();
  } else {
    auto& connMgrPtrRef = connMgr_.emplace(
        *evb, wangle::ConnectionManager::makeUnique(evb, params_.txnTimeout));
    connMgr = connMgrPtrRef.get();
  }
  return connMgr;
}

void HQServerTransportFactory::handleHQAlpn(
    std::shared_ptr<quic::QuicSocket> quicSocket,
    wangle::ConnectionManager* connMgr) {
  wangle::TransportInfo tinfo;
  auto controller = new HQSessionController(
      params_, httpTransactionHandlerProvider_, onTransportReadyFn_);
  auto session = new HQDownstreamSession(
      connMgr->getDefaultTimeout(), controller, tinfo, controller);
  quicSocket->setConnectionSetupCallback(session);
  quicSocket->setConnectionCallback(session);
  session->setSocket(std::move(quicSocket));
  session->setEgressSettings(
      {{proxygen::SettingsId::ENABLE_CONNECT_PROTOCOL, 1},
       {proxygen::SettingsId::_HQ_DATAGRAM_DRAFT_8, 1},
       {proxygen::SettingsId::_HQ_DATAGRAM, 1},
       {proxygen::SettingsId::_HQ_DATAGRAM_RFC, 1},
       {proxygen::SettingsId::ENABLE_WEBTRANSPORT, 1}});

  session->startNow();
  session->onTransportReady();
}

HQServer::HQServer(
    HQServerParams params,
    HTTPTransactionHandlerProvider httpTransactionHandlerProvider,
    std::function<void(proxygen::HQSession*)> onTransportReadyFn)
    : HQServer(std::move(params),
               std::make_unique<HQServerTransportFactory>(
                   params_,
                   std::move(httpTransactionHandlerProvider),
                   std::move(onTransportReadyFn))) {
}

HQServer::HQServer(HQServerParams params,
                   std::unique_ptr<quic::QuicServerTransportFactory> factory)
    : params_(std::move(params)) {
  params_.transportSettings.datagramConfig.enabled = true;
  server_ = quic::QuicServer::createQuicServer(params_.transportSettings);

  server_->setBindV6Only(false);
  server_->setCongestionControllerFactory(
      std::make_shared<ServerCongestionControllerFactory>());

  server_->setQuicServerTransportFactory(std::move(factory));
  server_->setQuicUDPSocketFactory(
      std::make_unique<QuicSharedUDPSocketFactory>());
  server_->setHealthCheckToken("health");
  server_->setSupportedVersion(params_.quicVersions);
  server_->setFizzContext(createFizzServerContext(params_));
  if (params_.rateLimitPerThread) {
    server_->setRateLimit(
        [rateLimitPerThread = params_.rateLimitPerThread.value()]() {
          return rateLimitPerThread;
        },
        1s);
  }
}

void HQServer::start() {
  folly::SocketAddress localAddress;
  if (params_.localAddress) {
    localAddress = *params_.localAddress;
  } else {
    localAddress.setFromLocalPort(params_.port);
  }
  server_->start(localAddress, params_.serverThreads);
}

const folly::SocketAddress HQServer::getAddress() const {
  server_->waitUntilInitialized();
  const auto& boundAddr = server_->getAddress();
  LOG(INFO) << "HQ server started at: " << boundAddr.describe();
  return boundAddr;
}

void HQServer::stop() {
  server_->shutdown();
}

void HQServer::rejectNewConnections(bool reject) {
  server_->rejectNewConnections([reject]() { return reject; });
}

} // namespace quic::samples
