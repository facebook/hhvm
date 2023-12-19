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

#include <folly/io/async/AsyncSocket.h>
#include <folly/net/NetworkSocket.h>
#include <thrift/lib/cpp/async/TAsyncSSLSocket.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/security/FizzPeeker.h>
#include <thrift/lib/cpp2/security/SSLUtil.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>
#include <wangle/acceptor/FizzAcceptorHandshakeHelper.h>
#include <wangle/acceptor/SSLAcceptorHandshakeHelper.h>

THRIFT_FLAG_DEFINE_int64(thrift_key_update_threshold, 0);

namespace apache {
namespace thrift {

namespace detail {
THRIFT_PLUGGABLE_FUNC_REGISTER(
    void, setSockOptStopTLS, folly::AsyncSocketTransport&) {
  return;
}
} // namespace detail

void ThriftFizzAcceptorHandshakeHelper::start(
    folly::AsyncSSLSocket::UniquePtr sock,
    wangle::AcceptorHandshakeHelper::Callback* callback) noexcept {
  callback_ = callback;

  if (thriftParametersContext_) {
    thriftExtension_ =
        std::make_shared<apache::thrift::ThriftParametersServerExtension>(
            thriftParametersContext_);
  }
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

  auto appProto = transport->getApplicationProtocol();

  if (loggingCallback_) {
    loggingCallback_->logFizzHandshakeSuccess(*transport, tinfo_);
  }

  if (thriftExtension_ && thriftExtension_->getNegotiatedStopTLS()) {
    VLOG(5) << "Beginning StopTLS negotiation";
    stopTLSAsyncFrame_.reset(new AsyncStopTLS(*this));

    // We are running as part of a wangle::ManagedConnection. The timeout
    // is managed by wangle; wangle will close() the underlying transport
    // (which will trigger an error) when its own timer elapses.
    stopTLSAsyncFrame_->start(
        transport, AsyncStopTLS::Role::Server, std::chrono::milliseconds(0));
  } else {
    callback_->connectionReady(
        std::move(transport_),
        std::move(appProto),
        SecureTransportType::TLS,
        wangle::SSLErrorEnum::NO_ERROR);
  }
}

void ThriftFizzAcceptorHandshakeHelper::stopTLSSuccess(
    std::unique_ptr<folly::IOBuf> endOfData) {
  auto appProto = transport_->getApplicationProtocol();
  auto plaintextTransport = moveToPlaintext(transport_.get());
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

wangle::AcceptorHandshakeHelper::UniquePtr FizzPeeker::getHelper(
    const std::vector<uint8_t>& bytes,
    const folly::SocketAddress& clientAddr,
    std::chrono::steady_clock::time_point acceptTime,
    wangle::TransportInfo& tinfo) {
  return getThriftHelper(bytes, clientAddr, acceptTime, tinfo);
}

folly::DelayedDestructionUniquePtr<ThriftFizzAcceptorHandshakeHelper>
FizzPeeker::getThriftHelper(
    const std::vector<uint8_t>& /* bytes */,
    const folly::SocketAddress& clientAddr,
    std::chrono::steady_clock::time_point acceptTime,
    wangle::TransportInfo& tinfo) {
  if (!(context_ && sslContextManager_)) {
    return nullptr;
  }
  auto optionsCopy = options_;
  optionsCopy.setkeyUpdateThreshold(THRIFT_FLAG(thrift_key_update_threshold));
  return folly::DelayedDestructionUniquePtr<ThriftFizzAcceptorHandshakeHelper>(
      new ThriftFizzAcceptorHandshakeHelper(
          context_,
          sslContextManager_,
          clientAddr,
          acceptTime,
          tinfo,
          std::move(optionsCopy),
          transportOptions_));
}
} // namespace thrift
} // namespace apache
