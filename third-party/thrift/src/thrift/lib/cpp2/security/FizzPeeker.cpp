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

#include <fizz/server/AsyncFizzServer.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/net/NetworkSocket.h>
#include <thrift/lib/cpp/async/TAsyncSSLSocket.h>
#include <thrift/lib/cpp2/security/AsyncStopTLS.h>
#include <thrift/lib/cpp2/security/FizzPeeker.h>
#include <thrift/lib/cpp2/security/SSLUtil.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersContext.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersServerExtension.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>
#include <wangle/acceptor/FizzAcceptorHandshakeHelper.h>
#include <wangle/acceptor/SSLAcceptorHandshakeHelper.h>

namespace apache {
namespace thrift {

namespace {

class ThriftFizzAcceptorHandshakeHelper;

/**
 * ThriftFizzAcceptorHandshakeHelper represents a single asynchronous Fizz
 * handshake. It has Thrift specific functionality such as including
 * a Thrift extension in the handshake and managing StopTLS negotiations.
 *
 * IMPLEMENTATION NOTES:
 * To fulfill the AcceptorHandshakeHelper contract as documented in wangle,
 * we must ensure that we always send either a `connectionReady()` or
 * `connectionError()` during the lifetime of this helper object.
 *
 * `dropConnection()` is inherited from the parent, which will close the
 * underlying socket. To fulfill our promises to the Handshake Manager, we
 * just need to ensure that at any time while this object lives, if we close
 * the underlying socket, this will result in some error being propagated.
 *
 * If the socket is closed:
 *    * During the initial TLS handshake, this results in a fizzHandshakeErr
 *      firing, which will trigger a connectionError().
 *    * If we are performing StopTLS, and we receive a `dropConnection()` after
 *      the initial TLS handshake but before the peer close_notify arrives, then
 *      we rely on `AsyncStopTLS` to receive a `readErr()` which will fire
 *      `stopTLSError()` which will fire `connectionError()`
 */
class ThriftFizzAcceptorHandshakeHelper
    : public wangle::FizzAcceptorHandshakeHelper,
      private AsyncStopTLS::Callback {
 public:
  ThriftFizzAcceptorHandshakeHelper(
      std::shared_ptr<const fizz::server::FizzServerContext> context,
      const folly::SocketAddress& clientAddr,
      std::chrono::steady_clock::time_point acceptTime,
      wangle::TransportInfo& tinfo,
      wangle::FizzHandshakeOptions&& options,
      const std::shared_ptr<apache::thrift::ThriftParametersContext>&
          thriftParametersContext)
      : wangle::FizzAcceptorHandshakeHelper::FizzAcceptorHandshakeHelper(
            context, clientAddr, acceptTime, tinfo, std::move(options)),
        thriftParametersContext_(thriftParametersContext) {}

  void start(
      folly::AsyncSSLSocket::UniquePtr sock,
      wangle::AcceptorHandshakeHelper::Callback* callback) noexcept override {
    callback_ = callback;
    sslContext_ = sock->getSSLContext();

    if (thriftParametersContext_) {
      thriftExtension_ =
          std::make_shared<apache::thrift::ThriftParametersServerExtension>(
              thriftParametersContext_);
    }
    transport_ = createFizzServer(std::move(sock), context_, thriftExtension_);
    transport_->accept(this);
  }

 private:
  // AsyncFizzServer::HandshakeCallback API
  void fizzHandshakeSuccess(
      fizz::server::AsyncFizzServer* transport) noexcept override {
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

  // Invoked by AsyncStopTLS when StopTLS downgrade completes successfully
  void stopTLSSuccess(std::unique_ptr<folly::IOBuf> endOfData) override {
    auto appProto = transport_->getApplicationProtocol();
    auto plaintextTransport = moveToPlaintext(transport_.get());
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

  // Invoked by AsyncStopTLS when StopTLS downgrade was interrupted or did
  // not finish successfully.
  void stopTLSError(const folly::exception_wrapper& ew) override {
    callback_->connectionError(transport_.get(), ew, sslError_);
  }

  std::shared_ptr<apache::thrift::ThriftParametersContext>
      thriftParametersContext_;
  std::shared_ptr<apache::thrift::ThriftParametersServerExtension>
      thriftExtension_;
  AsyncStopTLS::UniquePtr stopTLSAsyncFrame_;
};
} // namespace

wangle::AcceptorHandshakeHelper::UniquePtr FizzPeeker::getHelper(
    const std::vector<uint8_t>& /* bytes */,
    const folly::SocketAddress& clientAddr,
    std::chrono::steady_clock::time_point acceptTime,
    wangle::TransportInfo& tinfo) {
  if (!context_) {
    return nullptr;
  }
  auto optionsCopy = options_;
  return wangle::AcceptorHandshakeHelper::UniquePtr(
      new ThriftFizzAcceptorHandshakeHelper(
          context_,
          clientAddr,
          acceptTime,
          tinfo,
          std::move(optionsCopy),
          thriftParametersContext_));
}

} // namespace thrift
} // namespace apache
