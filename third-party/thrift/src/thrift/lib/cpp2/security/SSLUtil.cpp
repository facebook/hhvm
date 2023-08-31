/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/security/SSLUtil.h>

#include <folly/experimental/io/AsyncIoUringSocketFactory.h>
#include <folly/io/async/fdsock/AsyncFdSocket.h>
#include <folly/io/async/ssl/BasicTransportCertificate.h>

#include <fizz/client/AsyncFizzClient.h>
#include <fizz/experimental/util/CertExtraction.h>
#include <fizz/protocol/Exporter.h>
#include <fizz/server/AsyncFizzServer.h>

namespace apache {
namespace thrift {

// private class meant to encapsulate all the information that needs to be
// preserved across sockets for the tls downgrade scenario
namespace {

template <class Parent>
class StopTLSSocket : public Parent {
 public:
  StopTLSSocket(
      folly::EventBase* evb,
      folly::NetworkSocket fd,
      uint32_t zeroCopyBufId,
      std::shared_ptr<const fizz::Cert> selfCert,
      std::shared_ptr<const fizz::Cert> peerCert)
      : Parent(evb, fd, zeroCopyBufId),
        selfCert_(std::move(selfCert)),
        peerCert_(std::move(peerCert)) {}

  StopTLSSocket(
      folly::EventBase* evb,
      folly::NetworkSocket fd,
      std::shared_ptr<const fizz::Cert> selfCert,
      std::shared_ptr<const fizz::Cert> peerCert)
      : Parent(evb, fd),
        selfCert_(std::move(selfCert)),
        peerCert_(std::move(peerCert)) {}

  StopTLSSocket(
      folly::EventBase* evb,
      std::shared_ptr<const fizz::Cert> selfCert,
      std::shared_ptr<const fizz::Cert> peerCert)
      : Parent(evb),
        selfCert_(std::move(selfCert)),
        peerCert_(std::move(peerCert)) {}

  std::string getSecurityProtocol() const override { return "stopTLS"; }

  std::string getApplicationProtocol() const noexcept override { return alpn_; }

  void setApplicationProtocol(std::string alpn) noexcept {
    alpn_ = std::move(alpn);
  }

  const folly::AsyncTransportCertificate* getPeerCertificate() const override {
    return peerCert_.get();
  }

  const folly::AsyncTransportCertificate* getSelfCertificate() const override {
    return selfCert_.get();
  }

  void dropPeerCertificate() noexcept override { peerCert_.reset(); }

  void dropSelfCertificate() noexcept override { selfCert_.reset(); }

  void setCipher(fizz::CipherSuite cipher) { origCipherSuite_ = cipher; }

  void setExportedMasterSecret(std::unique_ptr<folly::IOBuf> buf) {
    exportedMasterSecret_ = std::move(buf);
  }

  std::unique_ptr<folly::IOBuf> getExportedKeyingMaterial(
      folly::StringPiece label,
      std::unique_ptr<folly::IOBuf> context,
      uint16_t length) const override {
    if (exportedMasterSecret_ == nullptr) {
      return nullptr;
    }
    auto factory = fizz::OpenSSLFactory();
    return fizz::Exporter::getExportedKeyingMaterial(
        factory,
        origCipherSuite_,
        exportedMasterSecret_->coalesce(),
        label,
        std::move(context),
        length);
  }

 private:
  // alpn of original socket, must save
  std::string alpn_;
  std::shared_ptr<const fizz::Cert> selfCert_;
  std::shared_ptr<const fizz::Cert> peerCert_;
  fizz::CipherSuite origCipherSuite_;
  std::unique_ptr<folly::IOBuf> exportedMasterSecret_{nullptr};
};
} // namespace

template <class FizzSocket>
folly::AsyncSocketTransport::UniquePtr moveToPlaintext(FizzSocket* fizzSock) {
  if (fizzSock == nullptr) {
    return nullptr;
  }
  auto [selfCert, peerCert] =
      fizz::detail::getSelfPeerCertificateShared(*fizzSock);

  auto cipher = fizzSock->getCipher();
  std::unique_ptr<folly::IOBuf> exportedMasterSecret{nullptr};
  if (fizzSock->getState().exporterMasterSecret().has_value() &&
      fizzSock->getState().exporterMasterSecret().value() != nullptr) {
    exportedMasterSecret =
        fizzSock->getState().exporterMasterSecret().value()->clone();
  }

  auto sock = fizzSock->template getUnderlyingTransport<folly::AsyncSocket>();
  folly::AsyncSocketTransport::UniquePtr plaintextTransport;
#if defined(__linux__) && __has_include(<liburing.h>)
  if (!sock &&
      fizzSock->template getUnderlyingTransport<folly::AsyncIoUringSocket>()) {
    // `AsyncFdSocket` currently lacks uring support, so hardcode `AsyncSocket`
    auto stopTLSSocket = new StopTLSSocket<folly::AsyncSocket>(
        fizzSock->getEventBase(), selfCert, peerCert);
    if (cipher.hasValue()) {
      stopTLSSocket->setCipher(cipher.value());
      stopTLSSocket->setExportedMasterSecret(std::move(exportedMasterSecret));
    }
    auto newSocket = folly::AsyncTransport::UniquePtr(stopTLSSocket);
    folly::AsyncIoUringSocket::UniquePtr io =
        fizzSock->template tryExchangeUnderlyingTransport<
            folly::AsyncIoUringSocket>(newSocket);
    if (io) {
      io->setReadCB(nullptr);
      io->setApplicationProtocol(fizzSock->getApplicationProtocol());
      io->setSelfCertificate(selfCert);
      io->setPeerCertificate(peerCert);
      plaintextTransport = std::move(io);
    }
  }
#endif
  if (!plaintextTransport) {
    DCHECK(sock);
    auto eb = sock->getEventBase();
    auto fd = sock->detachNetworkSocket();
    auto zcId = sock->getZeroCopyBufId();

    // Create new socket from old, make sure not to throw
    auto populateStopTLSSocket = [&](auto stopTLSSock) -> folly::AsyncSocket* {
      stopTLSSock->setApplicationProtocol(fizzSock->getApplicationProtocol());
      if (cipher.hasValue()) {
        stopTLSSock->setCipher(cipher.value());
        stopTLSSock->setExportedMasterSecret(std::move(exportedMasterSecret));
      }
      return stopTLSSock;
    };
#if !defined(_WIN32) // No FD-passing on Windows, don't try to make it build.
    folly::SocketAddress addr;
    sock->getPeerAddress(&addr);
    if (addr.getFamily() == AF_UNIX) {
      DCHECK_EQ(0, zcId) << "Zero-copy not supported on AF_UNIX sockets";
      auto newFdSock =
          new StopTLSSocket<folly::AsyncFdSocket>(eb, fd, selfCert, peerCert);
      if (auto oldFdSock =
              fizzSock
                  ->template getUnderlyingTransport<folly::AsyncFdSocket>()) {
        newFdSock->swapFdReadStateWith(oldFdSock);
      } else if (
          dynamic_cast<fizz::server::AsyncFizzServer*>(fizzSock) != nullptr) {
        // If the handshake was NOT negotiated over an `AsyncFdSocket`, then
        // the following race condition could happen:
        //  - Server closes TLS.
        //  - Client succeeds at `moveToPlaintext`, sends an FD-bearing request
        //  - Server receives the data of the FD-bearing request on the
        //    `FizzSocket` because `moveToPlaintext` has not yet succeeded.
        //    The FDs are lost (or leaked), because the `recvmsg` in
        //    `AsyncSocket` does not know to read them from the received
        //    ancillary data.
        //  - In `ThriftFizzAcceptorHandshakeHelper::stopTLSSuccess` (more docs
        //    there), server finished `moveToPlaintext` and moves the
        //    previously received request data to the new `AsyncFdSocket`.
        //  - Rocket parses a request that expects FDs, but fails to pop
        //    them from the `AsyncFdSocket` because it never got FDs.
        LOG(DFATAL) << "For AF_UNIX, AsyncFizzServer must always be backed by "
                    << "an underlying AsyncFdSocket";
      }
      plaintextTransport.reset(populateStopTLSSocket(newFdSock));
    } else
#endif
    {
      plaintextTransport.reset(
          populateStopTLSSocket(new StopTLSSocket<folly::AsyncSocket>(
              eb, fd, zcId, selfCert, peerCert)));
    }
  }
  return plaintextTransport;
}
template folly::AsyncSocketTransport::UniquePtr moveToPlaintext(
    fizz::client::AsyncFizzClient* socket);
template folly::AsyncSocketTransport::UniquePtr moveToPlaintext(
    fizz::server::AsyncFizzServer* socket);
} // namespace thrift
} // namespace apache
