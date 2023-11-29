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
#include <fizz/server/State.h>
#include <folly/experimental/io/AsyncIoUringSocketFactory.h>
#if !defined(_WIN32) // No FD-passing on Windows, don't try to make it build.
#include <folly/io/async/fdsock/AsyncFdSocket.h>
#endif
#include <wangle/acceptor/FizzAcceptorHandshakeHelper.h>
#include <wangle/acceptor/SSLAcceptorHandshakeHelper.h>
#include <wangle/ssl/ClientHelloExtStats.h>
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

  if (tokenBindingContext_) {
    tokenBindingExtension_ =
        std::make_shared<TokenBindingServerExtension>(tokenBindingContext_);
  }

  transport_ = createFizzServer(
      std::move(sock), context_, tokenBindingExtension_, transportOptions_);
  transport_->accept(this);
}

AsyncFizzServer::UniquePtr FizzAcceptorHandshakeHelper::createFizzServer(
    folly::AsyncSSLSocket::UniquePtr sslSock,
    const std::shared_ptr<const FizzServerContext>& fizzContext,
    const std::shared_ptr<fizz::ServerExtensions>& extensions,
    fizz::AsyncFizzBase::TransportOptions options) {
  folly::AsyncTransport::UniquePtr asyncTransport;
  if (preferIoUringSocket_ &&
      folly::AsyncIoUringSocketFactory::supports(sslSock->getEventBase())) {
    asyncTransport = folly::AsyncIoUringSocketFactory::create<
        folly::AsyncTransport::UniquePtr>(std::move(sslSock));
  } else {
#if !defined(_WIN32)
    folly::SocketAddress addr;
    sslSock->getPeerAddress(&addr);
#endif
    folly::AsyncSocket::UniquePtr asyncSock(
#if !defined(_WIN32)
        addr.getFamily() == AF_UNIX
            ? new folly::AsyncFdSocket(
                  folly::AsyncFdSocket::DoesNotMoveFdSocketState{},
                  std::move(sslSock))
            :
#endif
            new folly::AsyncSocket(std::move(sslSock)));
    asyncSock->cacheAddresses();
    asyncTransport = folly::AsyncTransport::UniquePtr(std::move(asyncSock));
  }
  AsyncFizzServer::UniquePtr fizzServer(new AsyncFizzServer(
      std::move(asyncTransport), fizzContext, extensions, options));
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
  tinfo_.echStatus =
      fizz::server::toString(transport->getState().echStatus()).str();

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
    tinfo_.clientAlpns = std::make_shared<std::vector<std::string>>(
        handshakeLogging->clientAlpns);
    tinfo_.sslClientSigAlgs =
        std::make_shared<std::string>(detail::enumVectorToHexStr(
            handshakeLogging->clientSignatureAlgorithms));
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

// AsyncIoUringSocket::AsyncDetachFdCallback
void FizzAcceptorHandshakeHelper::fdDetached(
    folly::NetworkSocket ns,
    std::unique_ptr<folly::IOBuf> unread) noexcept {
  if (!fallback_.clientHello) {
    fallback_.clientHello = std::move(unread);
  } else if (unread) {
    fallback_.clientHello->appendToChain(std::move(unread));
  }

  auto context = selectSSLCtx(fallback_.sni);
  sslSocket_ = folly::AsyncSSLSocket::UniquePtr(
      new folly::AsyncSSLSocket(context, transport_->getEventBase(), ns));
  transport_.reset();

  sslSocket_->setPreReceivedData(std::move(fallback_.clientHello));
  sslSocket_->enableClientHelloParsing();
  sslSocket_->forceCacheAddrOnFailure(true);
  sslSocket_->sslAccept(this);
}

void FizzAcceptorHandshakeHelper::fdDetachFail(
    const folly::AsyncSocketException& ex) noexcept {
  fizzHandshakeError(
      transport_.get(),
      folly::make_exception_wrapper<folly::AsyncSocketException>(ex));
}

std::shared_ptr<folly::SSLContext> FizzAcceptorHandshakeHelper::selectSSLCtx(
    const folly::Optional<std::string>& sni) const {
  if (sni) {
    if (auto context = sslContextManager_->getSSLCtx(sni.value())) {
      return context;
    }
  } else {
    if (auto context = sslContextManager_->getNoSNICtx()) {
      return context;
    }
    if (auto stats = sslContextManager_->getClientHelloExtStats()) {
      stats->recordAbsentHostname();
    }
  }
  return sslContextManager_->getDefaultSSLCtx();
}

void FizzAcceptorHandshakeHelper::fizzHandshakeAttemptFallback(
    AttemptVersionFallback fallback) {
  VLOG(3) << "Fallback to OpenSSL";
  if (loggingCallback_) {
    loggingCallback_->logFizzHandshakeFallback(*transport_, tinfo_);
  }

  folly::AsyncSocket* socket =
      transport_->getUnderlyingTransport<folly::AsyncSocket>();
  if (!socket &&
      folly::AsyncIoUringSocketFactory::asyncDetachFd(*transport_, this)) {
    fallback_ = std::move(fallback);
    return;
  }

  auto context = selectSSLCtx(fallback.sni);
  sslSocket_ = folly::AsyncSSLSocket::UniquePtr(
      new folly::AsyncSSLSocket(context, CHECK_NOTNULL(socket)));
  transport_.reset();

  sslSocket_->setPreReceivedData(std::move(fallback.clientHello));
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
