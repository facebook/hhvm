/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/async/SSLContext.h>
#include <proxygen/lib/http/codec/compress/HeaderCodec.h>
#include <proxygen/lib/http/session/HTTPSession.h>
#include <proxygen/lib/http/session/HTTPSessionStats.h>

namespace proxygen {

class HTTPSessionStats;

class HTTPUpstreamSession : public HTTPSession {
  using NewTransactionError = std::string;

 public:
  /**
   * @param sock           An open socket on which any applicable TLS
   *                         handshaking has been completed already.
   * @param localAddr      Address and port of the local end of the socket.
   * @param peerAddr       Address and port of the remote end of the socket.
   * @param codec          A codec with which to parse/generate messages in
   *                         whatever HTTP-like wire format this session needs.
   */
  HTTPUpstreamSession(const WheelTimerInstance& wheelTimer,
                      folly::AsyncTransport::UniquePtr&& sock,
                      const folly::SocketAddress& localAddr,
                      const folly::SocketAddress& peerAddr,
                      std::unique_ptr<HTTPCodec> codec,
                      const wangle::TransportInfo& tinfo,
                      InfoCallback* infoCallback);

  // uses folly::HHWheelTimer instance which is used on client side & thrift
  HTTPUpstreamSession(folly::HHWheelTimer* wheelTimer,
                      folly::AsyncTransport::UniquePtr&& sock,
                      const folly::SocketAddress& localAddr,
                      const folly::SocketAddress& peerAddr,
                      std::unique_ptr<HTTPCodec> codec,
                      const wangle::TransportInfo& tinfo,
                      InfoCallback* infoCallback);

  using FilterIteratorFn = std::function<void(HTTPCodecFilter*)>;

  void attachThreadLocals(folly::EventBase* eventBase,
                          std::shared_ptr<const folly::SSLContext> sslContext,
                          const WheelTimerInstance& wheelTimer,
                          HTTPSessionStats* stats,
                          FilterIteratorFn fn,
                          HeaderCodec::Stats* headerCodecStats,
                          HTTPSessionController* controller) override;

  void detachThreadLocals(bool detachSSLContext = false) override;

  void startNow() override;

  /**
   * Creates a new transaction on this upstream session. Invoking this function
   * also has the side-affect of starting reads after this event loop completes.
   *
   * @param handler The request handler to attach to this transaction. It must
   *                not be null.
   */
  HTTPTransaction* newTransaction(HTTPTransaction::Handler* handler) override;

  /**
   * Sends a WebTransport request to the peer. If this is sent on a session that
   * does not support WebTransport (e.g. ENABLE_CONNECT setting is false, or
   * underlying transport is http/1.1), the future will yield an error
   * synchronously.
   *
   * The promise/future is resolved when we either receive the http headers from
   * the peer or an error occurs, whichever occurs first. The client can
   * optimistically begin sending WebTransport data prior to the peer responding
   * with a 2xx via the WebTransportHandler::onWebTransportSession callback
   */
  folly::SemiFuture<std::unique_ptr<HTTPMessage>> sendWebTransportRequest(
      const HTTPMessage& req, WebTransportHandler::Ptr wtHandler) noexcept;

  /**
   * Returns true if the underlying transport has completed full handshake.
   */
  bool isReplaySafe() const override;

  /**
   * Returns true if this session has no open transactions and the underlying
   * transport can be used again in a new request.
   */
  bool isReusable() const override;

  /**
   * Returns true if the session is shutting down
   */
  bool isClosing() const override;

  /**
   * Drains the current transactions and prevents new transactions from being
   * created on this session. When the number of transactions reaches zero, this
   * session will shutdown the transport and delete itself.
   */
  void drain() override {
    HTTPSession::drain();
  }

  folly::Optional<const HTTPMessage::HTTP2Priority> getHTTPPriority(
      uint8_t level) override {
    if (!priorityAdapter_) {
      return HTTPSession::getHTTPPriority(level);
    }
    return priorityAdapter_->getHTTPPriority(level);
  }

  folly::Expected<HTTPTransaction*, NewTransactionError>
  newTransactionWithError(HTTPTransaction::Handler* handler);

 protected:
  ~HTTPUpstreamSession() override;

 private:
  /**
   * Called by onHeadersComplete(). Currently a no-op for upstream.
   */
  void setupOnHeadersComplete(HTTPTransaction* /* txn */,
                              HTTPMessage* /* msg */) override {
  }

  /**
   * Called by transactionTimeout() if the transaction has no handler.
   */
  HTTPTransaction::Handler* getTransactionTimeoutHandler(
      HTTPTransaction* txn) override;

  bool allTransactionsStarted() const override;

  void maybeAttachSSLContext(
      std::shared_ptr<const folly::SSLContext> sslContext) const;
  void maybeDetachSSLContext() const;

  std::unique_ptr<PriorityAdapter> priorityAdapter_;
};

} // namespace proxygen
