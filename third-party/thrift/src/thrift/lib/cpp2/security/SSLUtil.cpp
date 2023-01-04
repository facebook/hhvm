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
#include <folly/io/async/ssl/BasicTransportCertificate.h>

namespace apache {
namespace thrift {
// private class meant to encapsulate all the information that needs to be
// preserved across sockets for the tls downgrade scenario
namespace {
class StopTLSSocket : public folly::AsyncSocket {
 public:
  using AsyncSocket::AsyncSocket;

  std::string getSecurityProtocol() const override { return "stopTLS"; }

  std::string getApplicationProtocol() const noexcept override { return alpn_; }

  void setApplicationProtocol(std::string alpn) noexcept { alpn_ = alpn; }

 private:
  // alpn of original socket, must save
  std::string alpn_;
};
} // namespace

folly::AsyncSocketTransport::UniquePtr moveToPlaintext(
    folly::AsyncTransportWrapper* transport) {
  // Grab certs from transport
  auto selfCert = folly::ssl::BasicTransportCertificate::create(
      transport->getSelfCertificate());
  auto peerCert = folly::ssl::BasicTransportCertificate::create(
      transport->getPeerCertificate());
  auto sock = transport->getUnderlyingTransport<folly::AsyncSocket>();

  folly::AsyncSocketTransport::UniquePtr plaintextTransport;
#if __has_include(<liburing.h>)
  if (!sock && transport->getUnderlyingTransport<folly::AsyncIoUringSocket>()) {
    folly::AsyncTransport::UniquePtr newSocket{
        folly::AsyncSocket::newSocket(transport->getEventBase())};
    folly::AsyncIoUringSocket::UniquePtr io =
        transport->tryExchangeUnderlyingTransport<folly::AsyncIoUringSocket>(
            newSocket);
    if (io) {
      io->setReadCB(nullptr);
      io->setApplicationProtocol(transport->getApplicationProtocol());
      plaintextTransport = std::move(io);
    }
  }
#endif
  if (!plaintextTransport) {
    DCHECK(sock);
    auto eb = sock->getEventBase();
    auto fd = sock->detachNetworkSocket();
    auto zcId = sock->getZeroCopyBufId();

    // create new socket make sure not to throw
    auto stopTLSSocket = new StopTLSSocket(eb, fd, zcId);
    stopTLSSocket->setApplicationProtocol(transport->getApplicationProtocol());
    plaintextTransport = folly::AsyncSocketTransport::UniquePtr(stopTLSSocket);
  }

  plaintextTransport->setSelfCertificate(std::move(selfCert));
  plaintextTransport->setPeerCertificate(std::move(peerCert));
  return plaintextTransport;
}

} // namespace thrift
} // namespace apache
