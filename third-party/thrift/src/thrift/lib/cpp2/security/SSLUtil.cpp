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
#if __has_include(<liburing.h>)
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
      plaintextTransport = std::move(io);
    }
  }
#endif
  if (!plaintextTransport) {
    DCHECK(sock);
    auto eb = sock->getEventBase();
    auto fd = sock->detachNetworkSocket();
    auto zcId = sock->getZeroCopyBufId();

    folly::SocketAddress addr;
    sock->getPeerAddress(&addr);

    // Create new socket from old, make sure not to throw
    auto populateStopTLSSocket = [&](auto stopTLSSock) -> folly::AsyncSocket* {
      stopTLSSock->setApplicationProtocol(fizzSock->getApplicationProtocol());
      if (cipher.hasValue()) {
        stopTLSSock->setCipher(cipher.value());
        stopTLSSock->setExportedMasterSecret(std::move(exportedMasterSecret));
      }
      return stopTLSSock;
    };
    if (addr.getFamily() == AF_UNIX) {
      DCHECK_EQ(0, zcId) << "Zero-copy not supported on AF_UNIX sockets";
      plaintextTransport.reset(populateStopTLSSocket(
          new StopTLSSocket<folly::AsyncFdSocket>(eb, fd, selfCert, peerCert)));
    } else {
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
