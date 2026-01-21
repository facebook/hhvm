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

#pragma once

#include <fizz/server/AsyncFizzServer.h>
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/security/AsyncStopTLS.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersContext.h>
#include <thrift/lib/cpp2/security/extensions/ThriftParametersServerExtension.h>
#include <wangle/acceptor/FizzAcceptorHandshakeHelper.h>

namespace apache::thrift {

namespace detail {
THRIFT_PLUGGABLE_FUNC_DECLARE(
    void, setSockOptStopTLS, folly::AsyncSocketTransport&);

THRIFT_PLUGGABLE_FUNC_DECLARE(
    void, setSockOptTLSInfo, fizz::server::AsyncFizzServer*);
} // namespace detail

class ThriftParametersContext;

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
      std::shared_ptr<apache::thrift::ThriftParametersServerExtension>
          thriftExtension,
      std::shared_ptr<const fizz::server::FizzServerContext> context,
      std::shared_ptr<const wangle::SSLContextManager> sslContextManager,
      const folly::SocketAddress& clientAddr,
      std::chrono::steady_clock::time_point acceptTime,
      wangle::TransportInfo& tinfo,
      wangle::FizzHandshakeOptions&& options,
      fizz::AsyncFizzBase::TransportOptions transportOptions)
      : wangle::FizzAcceptorHandshakeHelper::FizzAcceptorHandshakeHelper(
            std::move(context),
            std::move(sslContextManager),
            clientAddr,
            acceptTime,
            tinfo,
            std::move(options),
            transportOptions,
            {}),
        thriftExtension_(std::move(thriftExtension)) {}

  void start(
      folly::AsyncSSLSocket::UniquePtr sock,
      wangle::AcceptorHandshakeHelper::Callback* callback) noexcept override;

 private:
  // AsyncFizzServer::HandshakeCallback API
  void fizzHandshakeSuccess(
      fizz::server::AsyncFizzServer* transport) noexcept override;

  // Invoked by AsyncStopTLS when StopTLS downgrade completes successfully
  void stopTLSSuccess(std::unique_ptr<folly::IOBuf> endOfData) override;

  // Invoked by AsyncStopTLS when StopTLS downgrade was interrupted or did
  // not finish successfully.
  void stopTLSError(const folly::exception_wrapper& ew) override {
    callback_->connectionError(transport_.get(), ew, sslError_);
  }

  std::shared_ptr<apache::thrift::ThriftParametersServerExtension>
      thriftExtension_;
  AsyncStopTLS::UniquePtr stopTLSAsyncFrame_;
};
} // namespace apache::thrift
