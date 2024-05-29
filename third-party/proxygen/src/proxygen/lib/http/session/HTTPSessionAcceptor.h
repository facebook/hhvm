/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/AsyncSSLSocket.h>
#include <proxygen/lib/http/codec/HTTPCodecFactory.h>
#include <proxygen/lib/http/session/HTTPDownstreamSession.h>
#include <proxygen/lib/http/session/HTTPErrorPage.h>
#include <proxygen/lib/http/session/SimpleController.h>
#include <proxygen/lib/services/HTTPAcceptor.h>

namespace proxygen {

class HTTPSessionStats;

/**
 * Specialization of Acceptor that serves as an abstract base for
 * acceptors that support HTTP and related protocols.
 */
class HTTPSessionAcceptor
    : public HTTPAcceptor
    , private HTTPSessionBase::InfoCallback {
 public:
  explicit HTTPSessionAcceptor(const AcceptorConfiguration& accConfig);
  explicit HTTPSessionAcceptor(const AcceptorConfiguration& accConfig,
                               std::shared_ptr<HTTPCodecFactory> codecFactory);
  ~HTTPSessionAcceptor() override;

  /**
   * Set the default error page generator.
   */
  void setDefaultErrorPage(std::unique_ptr<HTTPErrorPage> generator) {
    defaultErrorPage_ = std::move(generator);
  }

  /**
   * Access the default error page generator.
   */
  const HTTPErrorPage* getDefaultErrorPage() const {
    return defaultErrorPage_.get();
  }

  /**
   * Access the right error page generator for a connection.
   * @param   localAddr  Address of the local end of the connection.
   * @return  The diagnostic error page generator if one has been
   *          set AND the connection is to an internal VIP, or
   *          else the default error page generator if one has
   *          been set, or else nullptr.
   */
  virtual const HTTPErrorPage* getErrorPage(
      const folly::SocketAddress& addr) const;

  /**
   * Set the codec factory for this session
   */
  void setCodecFactory(std::shared_ptr<HTTPCodecFactory> codecFactory) {
    codecFactory_ = codecFactory;
  }

  /**
   * return the codec factory for this session
   */
  std::shared_ptr<HTTPCodecFactory> getCodecFactory() {
    return codecFactory_;
  }

  /**
   * Create a Handler for a new transaction.  The transaction and HTTP message
   * (request) are passed so the implementation can construct different
   * handlers based on these.  The transaction will be explicitly set on the
   * handler later via setTransaction.  The request message will be passed
   * in onHeadersComplete.
   */
  virtual HTTPTransaction::Handler* newHandler(HTTPTransaction& txn,
                                               HTTPMessage* msg) noexcept = 0;

  /**
   * Set an HTTPSession::InfoCallback to use for each session instead of the
   * acceptor object.
   */
  void setSessionInfoCallback(HTTPSession::InfoCallback* cb) {
    sessionInfoCb_ = cb;
  }

  virtual bool getHttp2PrioritiesEnabled() {
    return accConfig_.HTTP2PrioritiesEnabled;
  }

 protected:
  /**
   * This function is invoked when a new session is created to get the
   * controller to associate with the new session. Child classes may
   * override this function to provide their own more sophisticated
   * controller here.
   */
  virtual std::shared_ptr<HTTPSessionController> getController() {
    return simpleController_;
  }

  HTTPSessionStats* downstreamSessionStats_{nullptr};

  bool setEnableConnectProtocol_{false};

  HTTPSession::InfoCallback* getSessionInfoCallback() {
    return sessionInfoCb_ ? sessionInfoCb_ : this;
  }

  // Acceptor methods
  void onNewConnection(folly::AsyncTransport::UniquePtr sock,
                       const folly::SocketAddress* address,
                       const std::string& nextProtocol,
                       wangle::SecureTransportType secureTransportType,
                       const wangle::TransportInfo& tinfo) override;

  virtual void startSession(HTTPSessionBase& session) {
    session.startNow();
  }
  virtual size_t dropIdleConnections(size_t num);

  virtual void onSessionCreationError(ProxygenError /*error*/) {
  }

 private:
  HTTPSessionAcceptor(const HTTPSessionAcceptor&) = delete;
  HTTPSessionAcceptor& operator=(const HTTPSessionAcceptor&) = delete;

  /** General-case error page generator */
  std::unique_ptr<HTTPErrorPage> defaultErrorPage_;

  std::shared_ptr<HTTPCodecFactory> codecFactory_{};

  std::shared_ptr<SimpleController> simpleController_;

  HTTPSession::InfoCallback* sessionInfoCb_{nullptr};

  /**
   * 0.0.0.0:0, a valid address to use if getsockname() or getpeername() fails
   */
  static const folly::SocketAddress unknownSocketAddress_;
};

} // namespace proxygen
