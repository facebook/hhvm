/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HQSession.h>
#include <proxygen/lib/http/sink/HTTPTransactionSink.h>

#include <proxygen/lib/http/RFC2616.h>

namespace proxygen {

int HTTPTransactionSink::getTCPTransportFD() const {
  auto transport = httpTransaction_->getTransport().getUnderlyingTransport();
  const folly::AsyncSocket* sock = nullptr;
  if (transport) {
    sock = transport->getUnderlyingTransport<folly::AsyncSocket>();
  }
  if (sock) {
    return sock->getNetworkSocket().toFd();
  }
  return -1;
}

const folly::AsyncTransportCertificate*
HTTPTransactionSink::getPeerCertificate() const {
  auto transport = httpTransaction_->getTransport().getUnderlyingTransport();
  if (transport) {
    return transport->getPeerCertificate();
  } else {
    auto quicSocket = getQUICTransport();
    if (quicSocket) {
      return quicSocket->getPeerCertificate().get();
    }
  }
  return nullptr;
}

quic::QuicSocket* HTTPTransactionSink::getQUICTransport() const {
  auto session = httpTransaction_->getTransport().getHTTPSessionBase();
  if (auto hqSession = dynamic_cast<HQSession*>(session)) {
    return hqSession->getQuicSocket();
  }
  return nullptr;
}

} // namespace proxygen
