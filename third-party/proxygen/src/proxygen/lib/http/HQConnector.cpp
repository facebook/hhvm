/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/HQConnector.h>

#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <proxygen/lib/http/session/HQSession.h>
#include <quic/api/QuicSocket.h>
#include <quic/congestion_control/CongestionControllerFactory.h>
#include <quic/fizz/client/handshake/FizzClientQuicHandshakeContext.h>

using namespace folly;
using namespace std;
using namespace fizz::client;

namespace proxygen {

std::chrono::milliseconds HQConnector::timeElapsed() {
  if (timePointInitialized(connectStart_)) {
    return millisecondsSince(connectStart_);
  }
  return std::chrono::milliseconds(0);
}

void HQConnector::reset() {
  if (session_) {
    // This destroys the session
    session_->dropConnection();
    session_ = nullptr;
  }
}

void HQConnector::setTransportSettings(
    quic::TransportSettings transportSettings) {
  transportSettings_ = transportSettings;
}

void HQConnector::setQuicPskCache(
    std::shared_ptr<quic::QuicPskCache> quicPskCache) {
  quicPskCache_ = std::move(quicPskCache);
}

void HQConnector::connect(
    EventBase* eventBase,
    folly::Optional<folly::SocketAddress> localAddr,
    const folly::SocketAddress& connectAddr,
    std::shared_ptr<const FizzClientContext> fizzContext,
    std::shared_ptr<const fizz::CertificateVerifier> verifier,
    std::chrono::milliseconds connectTimeout,
    const SocketOptionMap& socketOptions,
    folly::Optional<std::string> sni,
    std::shared_ptr<quic::QLogger> qLogger,
    std::shared_ptr<quic::LoopDetectorCallback> quicLoopDetectorCallback,
    std::shared_ptr<quic::QuicTransportStatsCallback>
        quicTransportStatsCallback) {

  DCHECK(!isBusy());
  auto sock = std::make_unique<quic::QuicAsyncUDPSocketWrapperImpl>(eventBase);
  auto quicClient = quic::QuicClientTransport::newClient(
      eventBase,
      std::move(sock),
      quic::FizzClientQuicHandshakeContext::Builder()
          .setFizzClientContext(fizzContext)
          .setCertificateVerifier(std::move(verifier))
          .setPskCache(quicPskCache_)
          .build(),
      useConnectionEndWithErrorCallback_);
  quicClient->setHostname(sni.value_or(connectAddr.getAddressStr()));
  quicClient->addNewPeerAddress(connectAddr);
  if (localAddr.hasValue()) {
    quicClient->setLocalAddress(*localAddr);
  }
  quicClient->setCongestionControllerFactory(
      std::make_shared<quic::DefaultCongestionControllerFactory>());
  quicClient->setTransportStatsCallback(std::move(quicTransportStatsCallback));

  // Always use connected UDP sockets
  transportSettings_.connectUDP = true;
  quicClient->setTransportSettings(transportSettings_);
  quicClient->setQLogger(std::move(qLogger));
  quicClient->setLoopDetectorCallback(std::move(quicLoopDetectorCallback));
  quicClient->setSocketOptions(socketOptions);
  session_ = new proxygen::HQUpstreamSession(transactionTimeout_,
                                             connectTimeout,
                                             nullptr, // controller
                                             wangle::TransportInfo(),
                                             nullptr); // InfoCallback

  session_->setSocket(quicClient);
  session_->setConnectCallback(this);
  if (h3Settings_) {
    session_->setEgressSettings(*h3Settings_);
  }
  session_->startNow();

  VLOG(4) << "connecting to " << connectAddr.describe();
  connectStart_ = getCurrentTime();
  quicClient->start(session_, session_);
}

void HQConnector::onReplaySafe() noexcept {
  CHECK(session_);
  if (cb_) {
    auto session = session_;
    session_ = nullptr;
    cb_->connectSuccess(session);
  }
}

void HQConnector::connectError(quic::QuicError error) noexcept {
  CHECK(session_);
  reset();
  if (cb_) {
    cb_->connectError(error.code);
  }
}

} // namespace proxygen
