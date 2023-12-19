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

#include <fizz/extensions/tokenbinding/TokenBindingContext.h>
#include <fizz/extensions/tokenbinding/TokenBindingServerExtension.h>
#include <fizz/server/AsyncFizzServer.h>
#include <folly/experimental/io/AsyncIoUringSocket.h>
#include <wangle/acceptor/AcceptorHandshakeManager.h>
#include <wangle/acceptor/PeekingAcceptorHandshakeHelper.h>

namespace wangle {

class FizzHandshakeException : public wangle::SSLException {
 public:
  FizzHandshakeException(
      SSLErrorEnum error,
      const std::chrono::milliseconds& latency,
      uint64_t bytesRead,
      folly::exception_wrapper ex)
      : wangle::SSLException(error, latency, bytesRead),
        originalException_(std::move(ex)) {}

  const folly::exception_wrapper& getOriginalException() const {
    return originalException_;
  }

  using wangle::SSLException::SSLException;

 private:
  folly::exception_wrapper originalException_;
};

/**
 * FizzLoggingCallback is used as a logging hook for Fizz handshake events.
 */
class FizzLoggingCallback {
 public:
  virtual ~FizzLoggingCallback() = default;

  /**
   * logFizzHandshakeSuccess is invoked when the Fizz successfully accepted
   * the connection.
   *
   * @param server   A valid, non-owning reference to the Fizz server side
   *                 connection object that handled the connection.
   * @param tinfo    A filled out `wangle::TransportInfo` object summarizing
   *                 connection-oriented statistics and properties
   */
  virtual void logFizzHandshakeSuccess(
      const fizz::server::AsyncFizzServer& /*server*/,
      const wangle::TransportInfo& /*tinfo*/) noexcept {}

  /**
   * logFallbackHandshakeSuccess is invoked when Fizz was unable to handle the
   * connection (e.g. TLS 1.2 connection), but Fizz was able to successfully
   * handoff the handshake to a fallback implementation.
   *
   * @param server   A valid, non-owning reference to the AsyncSSLSocket
   *                 connection object that handled the connection.
   * @param tinfo    A filled out `wangle::TransportInfo` object summarizing
   *                 connection-oriented statistics and properties
   */
  virtual void logFallbackHandshakeSuccess(
      const folly::AsyncSSLSocket& /*server*/,
      const wangle::TransportInfo& /*tinfo*/) noexcept {}

  /**
   * logFizzHandshakeFallback is invoked when Fizz was unable to accept
   * the connection. The most common reason for this is that the client does
   * not support TLS 1.3.
   *
   * This is a non-connection-fatal error; while Fizz is unable to handle this
   * connection, the connection may still be accepted by a separate TLS
   * implementation (e.g. OpenSSL).
   *
   * This can only be invoked when
   * `FizzServerContext::setVersionFallbackEnabled` has been set.
   *
   * @param server   A valid, non-owning reference to the Fizz server side
   *                 connection object that handled the connection.
   * @param tinfo    A filled out `wangle::TransportInfo` object summarizing
   *                 connection-oriented statistics and properties
   */
  virtual void logFizzHandshakeFallback(
      const fizz::server::AsyncFizzServer& /*server*/,
      const wangle::TransportInfo& /*tinfo*/) noexcept {}

  /**
   * logFizzHandshakeError is invoked when Fizz encountered a connection-fatal
   * error while attempting to handshake with the client. This can be anything
   * from:
   *   * A client not sending a client certificate when Fizz was configured
   *     to require client certificates.
   *   * A client using a broken TLS 1.3 implementation.
   *   * Bitflips on the network causing TLS record integrity checks to fail.
   *   * ... and many more!
   *
   * This is a connection-fatal error; the connection is in an unrecoverable
   * and terminal state, and wangle will close the connection after this logging
   * hook call.
   *
   * @param server   A valid, non-owning reference to the Fizz server side
   *                 connection object that handled the connection.
   * @param ew       The exception object containing details on the cause of
   *                 the handshake failure.
   */
  virtual void logFizzHandshakeError(
      const fizz::server::AsyncFizzServer& /*server*/,
      const folly::exception_wrapper& /*ew*/) noexcept {}

  /**
   * logFallbackHandshakeError is invoked when the fallback OpenSSL
   * (AsyncSSLSocket) connections from Fizz encountered a connection-fatal
   * error while attempting to handshake with the client.
   *
   * This is a connection-fatal error; the connection is in an unrecoverable
   * and terminal state, and wangle will close the connection after this logging
   * hook call.
   *
   * @param server   A valid, non-owning reference to AsyncSSLSocket
   *                 connection object that handled the connection.
   * @param ew       The exception object containing details on the cause of
   *                 the handshake failure.
   */
  virtual void logFallbackHandshakeError(
      const folly::AsyncSSLSocket& /*server*/,
      const folly::AsyncSocketException& /*ex*/) noexcept {}
};

class FizzAcceptorHandshakeHelper;

/**
 * FizzHandshakeOptions supplies *optional* parameters to customize the behavior
 * of the `FizzAcceptorHandshakeHelper`.
 */
class FizzHandshakeOptions {
 public:
  /**
   * `setTokenBindingContext` is used to configure Token Binding (RFC 8472 +
   * tls1.3) for the accepted connection.
   */
  FizzHandshakeOptions& setTokenBindingContext(
      std::shared_ptr<fizz::extensions::TokenBindingContext> ctx) {
    tokenBindingCtx_ = std::move(ctx);
    return *this;
  }

  /**
   * `setLoggingCallback` defines the callback that will be invoked upon
   * fizz connection acceptance or failure.
   *
   * FizzHandshakeOptions does not take ownership over the supplied callback. If
   * define, the caller must guarantee that it outlives `wangle::Acceptor`.
   */
  FizzHandshakeOptions& setLoggingCallback(FizzLoggingCallback* callback) {
    loggingCallback_ = callback;
    return *this;
  }

  /**
   * `setHandshakeRecordAlignedReads` configures the Fizz acceptor to use
   * record aligned reads, which is a prerequisite if attempting to use kTLS.
   * This is required for kTLS support.
   *
   * For more information, refer to
   * `AsyncFizzBase::setHandshakeRecordAlignedReads`
   */
  FizzHandshakeOptions& setHandshakeRecordAlignedReads(bool flag) {
    handshakeRecordAlignedReads_ = flag;
    return *this;
  }

  /**
   * setkeyUpdateThreshold_ configures the server to initiate a key update
   * after encrypting a certain number of bytes using the same key.
   */
  FizzHandshakeOptions& setkeyUpdateThreshold(size_t flag) {
    keyUpdateThreshold_ = flag;
    return *this;
  }

  /**
   * `setPreferIoUringSocket` controls whether the accepted client connection
   * should be handled with an io_uring based transport.
   *
   * io_uring based transports are more efficient than traditional libevent
   * based transports, where kernel support exists.
   *
   * This flag is a hint -- if the host does not support io_uring, a normal
   * libevent based transport will be created (the default behavior, if this
   * option was not specified).
   */
  FizzHandshakeOptions& setPreferIoUringSocket(bool flag) {
    preferIoUringSocket_ = flag;
    return *this;
  }

 private:
  std::shared_ptr<fizz::extensions::TokenBindingContext> tokenBindingCtx_{
      nullptr};
  FizzLoggingCallback* loggingCallback_{nullptr};
  bool handshakeRecordAlignedReads_{false};
  size_t keyUpdateThreshold_{0};
  bool preferIoUringSocket_{false};
  friend class FizzAcceptorHandshakeHelper;
};

class SSLContextManager;

class FizzAcceptorHandshakeHelper
    : public wangle::AcceptorHandshakeHelper,
      public fizz::server::AsyncFizzServer::HandshakeCallback,
      public folly::AsyncSSLSocket::HandshakeCB,
      public folly::AsyncDetachFdCallback {
 public:
  FizzAcceptorHandshakeHelper(
      std::shared_ptr<const fizz::server::FizzServerContext> context,
      std::shared_ptr<const SSLContextManager> sslContextManager,
      const folly::SocketAddress& clientAddr,
      std::chrono::steady_clock::time_point acceptTime,
      wangle::TransportInfo& tinfo,
      FizzHandshakeOptions&& options,
      fizz::AsyncFizzBase::TransportOptions transportOptions)
      : context_(std::move(context)),
        sslContextManager_(std::move(sslContextManager)),
        tokenBindingContext_(std::move(options.tokenBindingCtx_)),
        clientAddr_(clientAddr),
        acceptTime_(acceptTime),
        tinfo_(tinfo),
        loggingCallback_(options.loggingCallback_),
        handshakeRecordAlignedReads_(options.handshakeRecordAlignedReads_),
        keyUpdateThreshold_(options.keyUpdateThreshold_),
        preferIoUringSocket_(options.preferIoUringSocket_),
        transportOptions_(transportOptions) {
    DCHECK(context_);
    DCHECK(sslContextManager_);
  }

  void start(
      folly::AsyncSSLSocket::UniquePtr sock,
      wangle::AcceptorHandshakeHelper::Callback* callback) noexcept override;

  void dropConnection(
      wangle::SSLErrorEnum reason = wangle::SSLErrorEnum::NO_ERROR) override {
    sslError_ = reason;
    if (transport_) {
      transport_->closeNow();
      return;
    }
    if (sslSocket_) {
      sslSocket_->closeNow();
      return;
    }
  }

 protected:
  // These are *explicitly* non virtual. Subclasses should not customize the
  // behavior of how Fizz servers and AsyncSSLSockets are created. Any
  // customization of these objects should be done through wangle managed
  // settings. This is to ensure that any settings on
  // `FizzAcceptorHandshakeHelper` are properly reflected when these objects are
  // created.
  fizz::server::AsyncFizzServer::UniquePtr createFizzServer(
      folly::AsyncSSLSocket::UniquePtr sslSock,
      const std::shared_ptr<const fizz::server::FizzServerContext>& fizzContext,
      const std::shared_ptr<fizz::ServerExtensions>& extensions,
      fizz::AsyncFizzBase::TransportOptions options);

  // AsyncFizzServer::HandshakeCallback API
  void fizzHandshakeSuccess(
      fizz::server::AsyncFizzServer* transport) noexcept override;
  void fizzHandshakeError(
      fizz::server::AsyncFizzServer* transport,
      folly::exception_wrapper ex) noexcept override;
  void fizzHandshakeAttemptFallback(
      fizz::server::AttemptVersionFallback fallback) override;

  // AsyncSSLSocket::HandshakeCallback API
  void handshakeSuc(folly::AsyncSSLSocket* sock) noexcept override;
  void handshakeErr(
      folly::AsyncSSLSocket* sock,
      const folly::AsyncSocketException& ex) noexcept override;

  // AsyncIoUringSocket::AsyncDetachFdCallback
  void fdDetached(
      folly::NetworkSocket ns,
      std::unique_ptr<folly::IOBuf> unread) noexcept override;
  void fdDetachFail(const folly::AsyncSocketException& ex) noexcept override;

  /**
   * Handles SSLContext selection for TLS handshake fallback logic.
   * Returns the default ssl context if no sni or no context match for sni.
   */
  std::shared_ptr<folly::SSLContext> selectSSLCtx(
      const folly::Optional<std::string>& sni) const;

  std::shared_ptr<const fizz::server::FizzServerContext> context_;
  std::shared_ptr<const SSLContextManager> sslContextManager_;
  std::shared_ptr<fizz::extensions::TokenBindingContext> tokenBindingContext_;
  std::shared_ptr<fizz::extensions::TokenBindingServerExtension>
      tokenBindingExtension_;
  fizz::server::AsyncFizzServer::UniquePtr transport_;
  folly::AsyncSSLSocket::UniquePtr sslSocket_;
  wangle::AcceptorHandshakeHelper::Callback* callback_;
  const folly::SocketAddress& clientAddr_;
  std::chrono::steady_clock::time_point acceptTime_;
  wangle::TransportInfo& tinfo_;
  wangle::SSLErrorEnum sslError_{wangle::SSLErrorEnum::NO_ERROR};
  FizzLoggingCallback* loggingCallback_;
  bool handshakeRecordAlignedReads_{false};
  size_t keyUpdateThreshold_{0};

  fizz::server::AttemptVersionFallback fallback_;
  bool preferIoUringSocket_{false};
  fizz::AsyncFizzBase::TransportOptions transportOptions_;
};

class DefaultToFizzPeekingCallback
    : public wangle::PeekingAcceptorHandshakeHelper::PeekCallback {
 public:
  DefaultToFizzPeekingCallback()
      : wangle::PeekingAcceptorHandshakeHelper::PeekCallback(0) {}

  std::shared_ptr<const fizz::server::FizzServerContext> getContext() const {
    return context_;
  }

  void setContext(
      std::shared_ptr<const fizz::server::FizzServerContext> context) {
    context_ = std::move(context);
  }

  void setSSLContextManager(
      std::shared_ptr<const SSLContextManager> sslContextManager) {
    sslContextManager_ = std::move(sslContextManager);
  }

  void setTransportOptions(
      fizz::AsyncFizzBase::TransportOptions transportOptions) {
    transportOptions_ = transportOptions;
  }

  /**
   * Return a reference to the `FizzHandshakeOptions` class to customize
   * handshake behavior.
   *
   * CALLER BEWARE: These options are initially set by the base
   * `wangle::Acceptor`'s translation from wangle::ServerSocketConfig. If you
   * subclass this, ensure to not clobber any of the variables that are directly
   * managed by the base wangle::Acceptor.
   */
  FizzHandshakeOptions& options() {
    return options_;
  }

  wangle::AcceptorHandshakeHelper::UniquePtr getHelper(
      const std::vector<uint8_t>& /* bytes */,
      const folly::SocketAddress& clientAddr,
      std::chrono::steady_clock::time_point acceptTime,
      wangle::TransportInfo& tinfo) override {
    auto optionsCopy = options_;
    if (!(context_ && sslContextManager_)) {
      return nullptr;
    }
    return wangle::AcceptorHandshakeHelper::UniquePtr(
        new FizzAcceptorHandshakeHelper(
            context_,
            sslContextManager_,
            clientAddr,
            acceptTime,
            tinfo,
            std::move(optionsCopy),
            transportOptions_));
  }

 protected:
  std::shared_ptr<const fizz::server::FizzServerContext> context_;
  std::shared_ptr<const SSLContextManager> sslContextManager_;
  FizzHandshakeOptions options_;
  fizz::AsyncFizzBase::TransportOptions transportOptions_;
};
} // namespace wangle
