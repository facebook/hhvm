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

#include <fizz/experimental/psp/PSP.h>
#include <folly/io/async/AsyncSocket.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/security/FizzPeeker.h>
#include <thrift/lib/cpp2/security/SSLUtil.h>
#include <wangle/acceptor/FizzAcceptorHandshakeHelper.h>
#include <wangle/acceptor/SSLAcceptorHandshakeHelper.h>

namespace apache::thrift {

namespace detail {
THRIFT_PLUGGABLE_FUNC_REGISTER(
    void, setSockOptStopTLS, folly::AsyncSocketTransport&) {
  return;
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    void, setSockOptTLSInfo, fizz::server::AsyncFizzServer*) {
  return;
}
} // namespace detail

void ThriftFizzAcceptorHandshakeHelper::start(
    folly::AsyncSSLSocket::UniquePtr sock,
    wangle::AcceptorHandshakeHelper::Callback* callback) noexcept {
  callback_ = callback;

  transport_ = createFizzServer(
      std::move(sock), context_, thriftExtension_, transportOptions_);
  transport_->accept(this);
}

void ThriftFizzAcceptorHandshakeHelper::fizzHandshakeSuccess(
    fizz::server::AsyncFizzServer* transport) noexcept {
  VLOG(3) << "Fizz handshake success";

  tinfo_.acceptTime = acceptTime_;
  tinfo_.secure = true;
  tinfo_.sslVersion = 0x0304;
  tinfo_.securityType = transport->getSecurityProtocol();
  tinfo_.sslSetupTime = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - acceptTime_);

  auto* handshakeLogging = transport->getState().handshakeLogging();
  if (handshakeLogging && handshakeLogging->clientSni) {
    tinfo_.sslServerName =
        std::make_shared<std::string>(*handshakeLogging->clientSni);
  }

  if (transport != nullptr) {
    detail::setSockOptTLSInfo(transport);
  }

  // TODO(T173985721) We intend on enabling automatic rekeying for all Fizz
  // connections regardless of ciphersuite
  auto negotiatedCipher = transport->getCipher();
  if (negotiatedCipher == fizz::CipherSuite::TLS_AEGIS_128L_SHA256 ||
      negotiatedCipher == fizz::CipherSuite::TLS_AEGIS_256_SHA512) {
    transport->setRekeyAfterWriting(keyUpdateThreshold_);
  }

  auto appProto = transport->getApplicationProtocol();

  if (loggingCallback_) {
    loggingCallback_->logFizzHandshakeSuccess(*transport, tinfo_);
  }

  if (thriftExtension_) {
    // The client and server may negotiate some "post handshake" transport
    // operation to be performed prior to returning the transport to
    // the Thrift server. This negotiation occurs through the usage of the
    // Thrift extension sent in TLS.

    // PSP is mutually exclusive with StopTLS.
    uint64_t pspHandshakeVersion = thriftExtension_->getNegotiatedPSPUpgrade();
    if (pspHandshakeVersion == PSPNegotiationVersion::THRIFT_PSP_V0) {
      // TODO: PSPVersion currently hardcoded to AES-128-GCM
      pspUpgradeFrame_ = fizz::psp::pspUpgradeV0(
          transport,
          fizz::psp::PSPVersion::VER0,
          fizz::psp::KernelPSP::make(folly::getGlobalCPUExecutor()));
      return pspUpgradeFrame_->start(this);
    }

    if (thriftExtension_->getNegotiatedStopTLS()) {
      VLOG(5) << "Beginning StopTLS negotiation";
      stopTLSAsyncFrame_.reset(new AsyncStopTLS(*this));

      // We are running as part of a wangle::ManagedConnection. The timeout
      // is managed by wangle; wangle will close() the underlying transport
      // (which will trigger an error) when its own timer elapses.
      return stopTLSAsyncFrame_->start(
          transport, AsyncStopTLS::Role::Server, std::chrono::milliseconds(0));
    }
  }
  callback_->connectionReady(
      std::move(transport_),
      std::move(appProto),
      SecureTransportType::TLS,
      wangle::SSLErrorEnum::NO_ERROR);
}

void ThriftFizzAcceptorHandshakeHelper::stopTLSSuccess(
    std::unique_ptr<folly::IOBuf> endOfData) {
  auto appProto = transport_->getApplicationProtocol();
  auto plaintextTransport =
      toFDSocket(transport_.get(), kSecurityProtocolStopTLS);
  tinfo_.securityType = plaintextTransport->getSecurityProtocol();

  detail::setSockOptStopTLS(*plaintextTransport);
  // The server initiates the close, which means the client will be the first
  // to successfully terminate tls and return the socket back to the caller.
  // What this means for us is we clearly don't know if our fizz transport
  // will only read the close notify and not additionally read any data the
  // application decided to send when it got back the socket. Fizz already
  // exposes any post close notify data and we shove it back into the socket
  // here.
  plaintextTransport->setPreReceivedData(std::move(endOfData));
  plaintextTransport->cacheAddresses();
  // kill the fizz socket unique ptr
  transport_.reset();
  callback_->connectionReady(
      std::move(plaintextTransport),
      std::move(appProto),
      SecureTransportType::TLS,
      wangle::SSLErrorEnum::NO_ERROR);
}

void ThriftFizzAcceptorHandshakeHelper::pspSuccess(
    folly::NetworkSocket) noexcept {
  auto applicationProtocol = transport_->getApplicationProtocol();

  auto plaintextTransport =
      toFDSocket(transport_.get(), kSecurityProtocolPSPV0);
  plaintextTransport->cacheAddresses();
  transport_.reset();

  tinfo_.securityType = kSecurityProtocolPSPV0;

  return callback_->connectionReady(
      std::move(plaintextTransport),
      std::move(applicationProtocol),
      SecureTransportType::TLS,
      wangle::SSLErrorEnum::NO_ERROR);
}

void ThriftFizzAcceptorHandshakeHelper::pspError(
    const folly::exception_wrapper& ew) noexcept {
  return callback_->connectionError(transport_.get(), ew, sslError_);
}

} // namespace apache::thrift
