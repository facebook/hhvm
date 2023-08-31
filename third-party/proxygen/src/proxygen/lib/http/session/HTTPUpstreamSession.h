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
   * @param maxVirtualPri  Number of virtual priority nodes to represent fixed
   *                         priority levels.
   */
  HTTPUpstreamSession(
      const WheelTimerInstance& wheelTimer,
      folly::AsyncTransport::UniquePtr&& sock,
      const folly::SocketAddress& localAddr,
      const folly::SocketAddress& peerAddr,
      std::unique_ptr<HTTPCodec> codec,
      const wangle::TransportInfo& tinfo,
      InfoCallback* infoCallback,
      uint8_t maxVirtualPri = 0,
      std::shared_ptr<const PriorityMapFactory> priorityMapFactory =
          std::shared_ptr<const PriorityMapFactory>())
      : HTTPSession(wheelTimer,
                    std::move(sock),
                    localAddr,
                    peerAddr,
                    nullptr,
                    std::move(codec),
                    tinfo,
                    infoCallback),
        maxVirtualPriorityLevel_(priorityMapFactory ? 0 : maxVirtualPri),
        priorityMapFactory_(priorityMapFactory) {
    if (sock_) {
      auto asyncSocket = sock_->getUnderlyingTransport<folly::AsyncSocket>();
      if (asyncSocket) {
        asyncSocket->setBufferCallback(this);
      }
    }
    CHECK_EQ(codec_->getTransportDirection(), TransportDirection::UPSTREAM);
  }

  // uses folly::HHWheelTimer instance which is used on client side & thrift
  HTTPUpstreamSession(
      folly::HHWheelTimer* wheelTimer,
      folly::AsyncTransport::UniquePtr&& sock,
      const folly::SocketAddress& localAddr,
      const folly::SocketAddress& peerAddr,
      std::unique_ptr<HTTPCodec> codec,
      const wangle::TransportInfo& tinfo,
      InfoCallback* infoCallback,
      uint8_t maxVirtualPri = 0,
      std::shared_ptr<const PriorityMapFactory> priorityMapFactory =
          std::shared_ptr<const PriorityMapFactory>())
      : HTTPUpstreamSession(WheelTimerInstance(wheelTimer),
                            std::move(sock),
                            localAddr,
                            peerAddr,
                            std::move(codec),
                            tinfo,
                            infoCallback,
                            maxVirtualPri,
                            priorityMapFactory) {
  }

  using FilterIteratorFn = std::function<void(HTTPCodecFilter*)>;

  void attachThreadLocals(folly::EventBase* eventBase,
                          folly::SSLContextPtr sslContext,
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

  virtual folly::Optional<const HTTPMessage::HTTP2Priority> getHTTPPriority(
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

  bool onNativeProtocolUpgrade(HTTPCodec::StreamID streamID,
                               CodecProtocol protocol,
                               const std::string& protocolString,
                               HTTPMessage& msg) override;

  void maybeAttachSSLContext(folly::SSLContextPtr sslContext) const;
  void maybeDetachSSLContext() const;

  uint8_t maxVirtualPriorityLevel_{0};

  std::shared_ptr<const PriorityMapFactory> priorityMapFactory_;
  std::unique_ptr<PriorityAdapter> priorityAdapter_;
};

} // namespace proxygen
