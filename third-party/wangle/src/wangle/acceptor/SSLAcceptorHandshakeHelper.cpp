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

#include <wangle/acceptor/SSLAcceptorHandshakeHelper.h>

#include <wangle/acceptor/Acceptor.h>
#include <wangle/acceptor/SecureTransportType.h>
#include <string>

namespace wangle {

static const std::string empty_string;

using namespace folly;

void SSLAcceptorHandshakeHelper::start(
    folly::AsyncSSLSocket::UniquePtr sock,
    AcceptorHandshakeHelper::Callback* callback) noexcept {
  socket_ = std::move(sock);
  callback_ = callback;

  socket_->enableClientHelloParsing();
  socket_->forceCacheAddrOnFailure(true);
  socket_->sslAccept(this);
}

void SSLAcceptorHandshakeHelper::fillSSLTransportInfoFields(
    AsyncSSLSocket* sock,
    TransportInfo& tinfo) {
  tinfo.secure = true;
  tinfo.securityType = sock->getSecurityProtocol();
  tinfo.sslSetupBytesRead = folly::to_narrow(sock->getRawBytesReceived());
  tinfo.sslSetupBytesWritten = folly::to_narrow(sock->getRawBytesWritten());
  tinfo.sslServerName = sock->getSSLServerName()
      ? std::make_shared<std::string>(sock->getSSLServerName())
      : nullptr;
  tinfo.sslCipher = sock->getNegotiatedCipherName()
      ? std::make_shared<std::string>(sock->getNegotiatedCipherName())
      : nullptr;
  tinfo.sslVersion = sock->getSSLVersion();
  const char* sigAlgName = sock->getSSLCertSigAlgName();
  tinfo.sslCertSigAlgName =
      std::make_shared<std::string>(sigAlgName ? sigAlgName : "");
  tinfo.sslCertSize = sock->getSSLCertSize();
  tinfo.sslResume = SSLUtil::getResumeState(sock);
  tinfo.sslClientCiphers = std::make_shared<std::string>();
  sock->getSSLClientCiphers(*tinfo.sslClientCiphers);
  tinfo.sslClientCiphersHex = std::make_shared<std::string>();
  sock->getSSLClientCiphers(
      *tinfo.sslClientCiphersHex, /* convertToString = */ false);
  tinfo.sslServerCiphers = std::make_shared<std::string>();
  sock->getSSLServerCiphers(*tinfo.sslServerCiphers);
  tinfo.sslClientComprMethods =
      std::make_shared<std::string>(sock->getSSLClientComprMethods());
  tinfo.sslClientExts = std::make_shared<std::string>(sock->getSSLClientExts());
  tinfo.sslClientSigAlgs =
      std::make_shared<std::string>(sock->getSSLClientSigAlgs());
  tinfo.sslClientSupportedVersions =
      std::make_shared<std::string>(sock->getSSLClientSupportedVersions());
  tinfo.clientAlpns = folly::copy_to_shared_ptr(sock->getClientAlpns());
}

void SSLAcceptorHandshakeHelper::handshakeSuc(AsyncSSLSocket* sock) noexcept {
  const unsigned char* nextProto = nullptr;
  unsigned nextProtoLength = 0;
  sock->getSelectedNextProtocolNoThrow(&nextProto, &nextProtoLength);
  if (VLOG_IS_ON(3)) {
    if (nextProto) {
      VLOG(3) << "Client selected next protocol "
              << std::string((const char*)nextProto, nextProtoLength);
    } else {
      VLOG(3) << "Client did not select a next protocol";
    }
  }

  // fill in SSL-related fields from TransportInfo
  // the other fields like RTT are filled in the Acceptor
  tinfo_.acceptTime = acceptTime_;
  tinfo_.sslSetupTime = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - acceptTime_);
  fillSSLTransportInfoFields(sock, tinfo_);

  auto nextProtocol = nextProto
      ? std::string((const char*)nextProto, nextProtoLength)
      : empty_string;

  // The callback will delete this.
  callback_->connectionReady(
      std::move(socket_),
      std::move(nextProtocol),
      SecureTransportType::TLS,
      SSLErrorEnum::NO_ERROR);
}

void SSLAcceptorHandshakeHelper::handshakeErr(
    AsyncSSLSocket* sock,
    const AsyncSocketException& ex) noexcept {
  auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - acceptTime_);
  VLOG(3) << "SSL handshake error with " << describeAddresses(sock) << " after "
          << elapsedTime.count() << " ms; " << sock->getRawBytesReceived()
          << " bytes received & " << sock->getRawBytesWritten()
          << " bytes sent: " << ex.what();

  auto sslEx = folly::make_exception_wrapper<SSLException>(
      sslError_, elapsedTime, sock->getRawBytesReceived());

  // The callback will delete this.
  callback_->connectionError(socket_.get(), sslEx, sslError_);
}

} // namespace wangle
