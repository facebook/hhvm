/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/network/AsyncTlsToPlaintextSocket.h"

#include <chrono>
#include <functional>
#include <utility>

#include <folly/GLog.h>
#include <folly/ScopeGuard.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/ssl/BasicTransportCertificate.h>
#include <folly/portability/OpenSSL.h>

#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp/async/TAsyncSSLSocket.h>

namespace facebook {
namespace memcache {

class AsyncTlsToPlaintextSocket::ConnectCallback
    : public folly::AsyncSocket::ConnectCallback {
 public:
  ConnectCallback(
      AsyncTlsToPlaintextSocket& me,
      folly::AsyncSocket::ConnectCallback* connectCallback)
      : me_(me), connectCallback_(connectCallback) {}

  void connectSuccess() noexcept override {
    DestructorGuard dg{&me_};
    SCOPE_EXIT {
      if (auto* cb = std::exchange(connectCallback_, nullptr)) {
        cb->connectSuccess();
      }
      delete this;
    };

    auto& impl = me_.impl_;
    auto activateSocket = [&] {
      impl->setSendTimeout(me_.writeTimeout_.count());
      impl->setReadCB(std::exchange(me_.readCallback_, nullptr));

      me_.state_ = State::CONNECTED;
      me_.flushWrites();
    };

    auto* tlsSocket =
        impl->getUnderlyingTransport<apache::thrift::async::TAsyncSSLSocket>();
    CHECK(tlsSocket);

    // Save state regarding session resumption
    if (tlsSocket->sessionResumptionAttempted()) {
      me_.resumptionStatus_ = tlsSocket->getSSLSessionReused()
          ? SessionResumptionStatus::RESUMPTION_ATTEMPTED_AND_SUCCEEDED
          : SessionResumptionStatus::RESUMPTION_ATTEMPTED_AND_FAILED;
    }

    if (tlsSocket->getApplicationProtocol() != kMcSecurityTlsToPlaintextProto) {
      FB_LOG_EVERY_MS(ERROR, 10)
          << "Failed to negotiate plaintext fallback. Falling back to full TLS.";
      // Even if the server fails the handshake, we must be sure to drain any
      // pending writes
      activateSocket();
      return;
    }

    // Save peer cert
    auto peerCert = folly::ssl::BasicTransportCertificate::create(
        tlsSocket->getPeerCertificate());

    // We need to mark the SSL as shutdown here, but need to do
    // it quietly so no alerts are sent over the wire.
    // This prevents SSL thinking we are shutting down in a bad state
    // when AsyncSSLSocket is cleaned up, which could remove the session
    // from the session cache
    auto* ssl = const_cast<SSL*>(tlsSocket->getSSL());
    SSL_set_quiet_shutdown(ssl, 1);
    SSL_shutdown(ssl);

    DCHECK_EQ(0, tlsSocket->getZeroCopyBufId());
    impl.reset(
        new folly::AsyncSocket(&me_.evb_, tlsSocket->detachNetworkSocket()));
    activateSocket();
    impl->getUnderlyingTransport<folly::AsyncSocket>()->setPeerCertificate(
        std::move(peerCert));
  }

  void connectErr(const folly::AsyncSocketException& ex) noexcept override {
    DestructorGuard dg{&me_};

    me_.failAllWrites(ex);
    if (auto* readCallback = std::exchange(me_.readCallback_, nullptr)) {
      readCallback->readErr(ex);
    }
    if (auto* cb = std::exchange(connectCallback_, nullptr)) {
      cb->connectErr(ex);
    }
    delete this;
  }

 private:
  AsyncTlsToPlaintextSocket& me_;
  folly::AsyncSocket::ConnectCallback* connectCallback_{nullptr};
};

void AsyncTlsToPlaintextSocket::connect(
    folly::AsyncSocket::ConnectCallback* connectCallback,
    const folly::SocketAddress& address,
    std::chrono::milliseconds connectTimeout,
    folly::SocketOptionMap socketOptions) {
  auto* const wrappedConnectCallback =
      new ConnectCallback(*this, connectCallback);
  impl_->getUnderlyingTransport<apache::thrift::async::TAsyncSSLSocket>()
      ->connect(
          wrappedConnectCallback,
          address,
          connectTimeout.count(),
          std::move(socketOptions));
}

void AsyncTlsToPlaintextSocket::flushWrites() {
  while (!bufferedWrites_.empty()) {
    auto& bufferedWrite = bufferedWrites_.front();
    auto* cb = bufferedWrite.callback;
    auto buf = std::move(bufferedWrite.buf);
    bufferedWrites_.pop_front();

    impl_->writeChain(cb, std::move(buf));
  }
}

void AsyncTlsToPlaintextSocket::failAllWrites(
    const folly::AsyncSocketException& ex) {
  while (!bufferedWrites_.empty()) {
    auto& bufferedWrite = bufferedWrites_.front();
    auto* cb = bufferedWrite.callback;
    bufferedWrites_.pop_front();

    cb->writeErr(0, ex);
  }
}

} // namespace memcache
} // namespace facebook
