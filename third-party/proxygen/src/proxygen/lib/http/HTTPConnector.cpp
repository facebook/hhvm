/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/HTTPConnector.h>

#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/AsyncSSLSocket.h>
#include <proxygen/lib/http/codec/DefaultHTTPCodecFactory.h>
#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/codec/HTTP2Codec.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <proxygen/lib/http/session/HTTPUpstreamSession.h>
#include <wangle/ssl/SSLUtil.h>

using namespace folly;
using namespace std;

namespace proxygen {

HTTPConnector::HTTPConnector(Callback* callback,
                             folly::HHWheelTimer* timeoutSet)
    : HTTPConnector(callback, WheelTimerInstance(timeoutSet)) {
}

HTTPConnector::HTTPConnector(Callback* callback,
                             const WheelTimerInstance& timeout)
    : cb_(CHECK_NOTNULL(callback)),
      timeout_(timeout),
      httpCodecFactory_(std::make_unique<DefaultHTTPCodecFactory>(false)) {
}

HTTPConnector::~HTTPConnector() {
  reset();
}

void HTTPConnector::reset() {
  if (socket_) {
    auto cb = cb_;
    cb_ = nullptr;
    socket_.reset(); // This invokes connectError() but will be ignored
    cb_ = cb;
  }
}

void HTTPConnector::setPlaintextProtocol(const std::string& plaintextProto) {
  plaintextProtocol_ = plaintextProto;
}

void HTTPConnector::setHTTPVersionOverride(bool enabled) {
  httpCodecFactory_->setForceHTTP1xCodecTo1_1(enabled);
}

void HTTPConnector::connect(EventBase* eventBase,
                            const folly::SocketAddress& connectAddr,
                            std::chrono::milliseconds timeoutMs,
                            const SocketOptionMap& socketOptions,
                            const folly::SocketAddress& bindAddr) {

  DCHECK(!isBusy());
  transportInfo_ = wangle::TransportInfo();
  transportInfo_.secure = false;
  auto sock = new AsyncSocket(eventBase);
  socket_.reset(sock);
  connectStart_ = getCurrentTime();
  cb_->preConnect(sock);
  sock->connect(this, connectAddr, timeoutMs.count(), socketOptions, bindAddr);
}

void HTTPConnector::connectSSL(EventBase* eventBase,
                               const folly::SocketAddress& connectAddr,
                               const shared_ptr<const SSLContext>& context,
                               std::shared_ptr<folly::ssl::SSLSession> session,
                               std::chrono::milliseconds timeoutMs,
                               const SocketOptionMap& socketOptions,
                               const folly::SocketAddress& bindAddr,
                               const std::string& serverName) {

  DCHECK(!isBusy());
  transportInfo_ = wangle::TransportInfo();
  transportInfo_.secure = true;
  auto sslSock = new AsyncSSLSocket(context, eventBase);
  if (session) {
    sslSock->setSSLSession(session);
  }
  sslSock->setServerName(serverName);
  sslSock->forceCacheAddrOnFailure(true);
  socket_.reset(sslSock);
  connectStart_ = getCurrentTime();
  cb_->preConnect(sslSock);
  sslSock->connect(
      this, connectAddr, timeoutMs.count(), socketOptions, bindAddr);
}

std::chrono::milliseconds HTTPConnector::timeElapsed() {
  if (timePointInitialized(connectStart_)) {
    return millisecondsSince(connectStart_);
  }
  return std::chrono::milliseconds(0);
}

// Callback interface

void HTTPConnector::connectSuccess() noexcept {
  if (!cb_) {
    return;
  }

  folly::SocketAddress localAddress;
  folly::SocketAddress peerAddress;
  socket_->getLocalAddress(&localAddress);
  socket_->getPeerAddress(&peerAddress);

  std::unique_ptr<HTTPCodec> codec;
  std::string protoCopy;
  std::string* proto{&protoCopy};
  transportInfo_.acceptTime = getCurrentTime();
  if (transportInfo_.secure) {
    AsyncSSLSocket* sslSocket =
        socket_->getUnderlyingTransport<AsyncSSLSocket>();

    if (sslSocket) {
      transportInfo_.appProtocol =
          std::make_shared<std::string>(socket_->getApplicationProtocol());
      transportInfo_.sslSetupTime = millisecondsSince(connectStart_);
      transportInfo_.sslCipher = sslSocket->getNegotiatedCipherName()
                                     ? std::make_shared<std::string>(
                                           sslSocket->getNegotiatedCipherName())
                                     : nullptr;
      transportInfo_.sslVersion = sslSocket->getSSLVersion();
      transportInfo_.sslResume = wangle::SSLUtil::getResumeState(sslSocket);
    }

    protoCopy = socket_->getApplicationProtocol();
  } else {
    proto = &plaintextProtocol_;
  }

  CHECK(proto);
  codec = httpCodecFactory_->getCodec(
      *proto, TransportDirection::UPSTREAM, transportInfo_.secure);

  if (!codec) {
    AsyncSocketException ex(
        AsyncSocketException::INTERNAL_ERROR,
        folly::to<string>("HTTPCodecFactory failed to create codec for proto=",
                          *proto));
    connectErr(ex);
    return;
  }

  HTTPUpstreamSession* session = new HTTPUpstreamSession(timeout_,
                                                         std::move(socket_),
                                                         localAddress,
                                                         peerAddress,
                                                         std::move(codec),
                                                         transportInfo_,
                                                         nullptr);

  cb_->connectSuccess(session);
}

void HTTPConnector::connectErr(const AsyncSocketException& ex) noexcept {
  socket_.reset();
  if (cb_) {
    cb_->connectError(ex);
  }
}

} // namespace proxygen
