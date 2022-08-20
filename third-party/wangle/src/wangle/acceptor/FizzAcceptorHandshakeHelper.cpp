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

#include <fizz/record/Types.h>
#include <wangle/acceptor/FizzAcceptorHandshakeHelper.h>
#include <wangle/acceptor/SSLAcceptorHandshakeHelper.h>
#include <wangle/ssl/SSLContextManager.h>

using namespace fizz::extensions;
using namespace fizz::server;

namespace wangle {
namespace detail {

template <typename T, std::enable_if_t<std::is_enum<T>::value, bool> = true>
std::string enumVectorToHexStr(const std::vector<T>& enumVector) {
  std::string hexStr;
  bool first = true;
  for (auto enumValue : enumVector) {
    if (first) {
      first = false;
    } else {
      hexStr += ":";
    }
    hexStr += fizz::enumToHex(enumValue);
  }
  return hexStr;
}
} // namespace detail

void FizzAcceptorHandshakeHelper::start(
    folly::AsyncSSLSocket::UniquePtr sock,
    AcceptorHandshakeHelper::Callback* callback) noexcept {
  callback_ = callback;
  sslContext_ = sock->getSSLContext();

  if (tokenBindingContext_) {
    tokenBindingExtension_ =
        std::make_shared<TokenBindingServerExtension>(tokenBindingContext_);
  }

  transport_ =
      createFizzServer(std::move(sock), context_, tokenBindingExtension_);
  transport_->accept(this);
}

AsyncFizzServer::UniquePtr FizzAcceptorHandshakeHelper::createFizzServer(
    folly::AsyncSSLSocket::UniquePtr sslSock,
    const std::shared_ptr<const FizzServerContext>& fizzContext,
    const std::shared_ptr<fizz::ServerExtensions>& extensions) {
  folly::AsyncSocket::UniquePtr asyncSock(
      new folly::AsyncSocket(std::move(sslSock)));
  asyncSock->cacheAddresses();
  AsyncFizzServer::UniquePtr fizzServer(
      new AsyncFizzServer(std::move(asyncSock), fizzContext, extensions));
  fizzServer->setHandshakeRecordAlignedReads(handshakeRecordAlignedReads_);

  return fizzServer;
}

void FizzAcceptorHandshakeHelper::fizzHandshakeSuccess(
    AsyncFizzServer* transport) noexcept {
  VLOG(3) << "Fizz handshake success";

  tinfo_.acceptTime = acceptTime_;
  tinfo_.secure = true;
  tinfo_.sslVersion = 0x0304;
  tinfo_.securityType = transport->getSecurityProtocol();
  tinfo_.sslSetupTime = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - acceptTime_);
  if (tokenBindingExtension_ &&
      tokenBindingExtension_->getNegotiatedKeyParam().has_value()) {
    tinfo_.negotiatedTokenBindingKeyParameters =
        static_cast<uint8_t>(*tokenBindingExtension_->getNegotiatedKeyParam());
  }

  auto* handshakeLogging = transport->getState().handshakeLogging();
  if (handshakeLogging) {
    if (handshakeLogging->clientSni) {
      tinfo_.sslServerName =
          std::make_shared<std::string>(*handshakeLogging->clientSni);
    }

    tinfo_.sslClientCiphersHex = std::make_shared<std::string>(
        detail::enumVectorToHexStr(handshakeLogging->clientCiphers));
    tinfo_.sslClientExts = std::make_shared<std::string>(
        folly::join(":", handshakeLogging->clientExtensions));
  }

  auto appProto = transport->getApplicationProtocol();

  if (loggingCallback_) {
    loggingCallback_->logFizzHandshakeSuccess(*transport, tinfo_);
  }

  callback_->connectionReady(
      std::move(transport_),
      std::move(appProto),
      SecureTransportType::TLS,
      SSLErrorEnum::NO_ERROR);
}

void FizzAcceptorHandshakeHelper::fizzHandshakeError(
    AsyncFizzServer* transport,
    folly::exception_wrapper ex) noexcept {
  if (loggingCallback_) {
    loggingCallback_->logFizzHandshakeError(*transport, ex);
  }

  auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - acceptTime_);
  VLOG(3) << "Fizz handshake error with " << describeAddresses(transport)
          << " after " << elapsedTime.count() << " ms; "
          << transport->getRawBytesReceived() << " bytes received & "
          << transport->getRawBytesWritten() << " bytes sent: " << ex.what();

  auto handshakeException =
      folly::make_exception_wrapper<FizzHandshakeException>(
          sslError_,
          elapsedTime,
          transport->getRawBytesReceived(),
          std::move(ex));

  callback_->connectionError(
      transport_.get(), std::move(handshakeException), sslError_);
}

folly::AsyncSSLSocket::UniquePtr FizzAcceptorHandshakeHelper::createSSLSocket(
    const std::shared_ptr<folly::SSLContext>& context,
    folly::AsyncTransport::UniquePtr transport) {
  auto socket = transport->getUnderlyingTransport<folly::AsyncSocket>();
  auto sslSocket = folly::AsyncSSLSocket::UniquePtr(
      new folly::AsyncSSLSocket(context, CHECK_NOTNULL(socket)));
  transport.reset();
  return sslSocket;
}

void FizzAcceptorHandshakeHelper::fizzHandshakeAttemptFallback(
    std::unique_ptr<folly::IOBuf> clientHello) {
  VLOG(3) << "Fallback to OpenSSL";
  if (loggingCallback_) {
    loggingCallback_->logFizzHandshakeFallback(*transport_, tinfo_);
  }
  sslSocket_ = createSSLSocket(sslContext_, std::move(transport_));

  sslSocket_->setPreReceivedData(std::move(clientHello));
  sslSocket_->enableClientHelloParsing();
  sslSocket_->forceCacheAddrOnFailure(true);
  sslSocket_->sslAccept(this);
}

void FizzAcceptorHandshakeHelper::handshakeSuc(
    folly::AsyncSSLSocket* sock) noexcept {
  auto appProto = sock->getApplicationProtocol();
  if (!appProto.empty()) {
    VLOG(3) << "Client selected next protocol " << appProto;
  } else {
    VLOG(3) << "Client did not select a next protocol";
  }

  // fill in SSL-related fields from TransportInfo
  // the other fields like RTT are filled in the Acceptor
  tinfo_.acceptTime = acceptTime_;
  tinfo_.sslSetupTime = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - acceptTime_);
  wangle::SSLAcceptorHandshakeHelper::fillSSLTransportInfoFields(sock, tinfo_);

  if (loggingCallback_) {
    loggingCallback_->logFallbackHandshakeSuccess(*sock, tinfo_);
  }

  // The callback will delete this.
  callback_->connectionReady(
      std::move(sslSocket_),
      std::move(appProto),
      SecureTransportType::TLS,
      SSLErrorEnum::NO_ERROR);
}

void FizzAcceptorHandshakeHelper::handshakeErr(
    folly::AsyncSSLSocket* sock,
    const folly::AsyncSocketException& ex) noexcept {
  if (loggingCallback_) {
    loggingCallback_->logFallbackHandshakeError(*sock, ex);
  }

  auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - acceptTime_);
  VLOG(3) << "SSL handshake error with " << describeAddresses(sock) << " after "
          << elapsedTime.count() << " ms; " << sock->getRawBytesReceived()
          << " bytes received & " << sock->getRawBytesWritten()
          << " bytes sent: " << ex.what();

  auto sslEx = folly::make_exception_wrapper<SSLException>(
      sslError_, elapsedTime, sock->getRawBytesReceived());

  // The callback will delete this.
  callback_->connectionError(sslSocket_.get(), sslEx, sslError_);
}
} // namespace wangle
