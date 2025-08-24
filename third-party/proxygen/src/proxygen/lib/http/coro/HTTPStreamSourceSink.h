/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPCoroSession.h"
#include "proxygen/lib/http/coro/HTTPError.h"
#include "proxygen/lib/http/coro/HTTPSourceReader.h"
#include "proxygen/lib/http/coro/HTTPStreamSource.h"
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <proxygen/lib/http/sink/HTTPSink.h>
#include <proxygen/lib/utils/ConditionalGate.h>

#include <utility>

namespace proxygen::coro {

/**
 * This class is a bridge between the HTTPTransaction and HTTPSource APIs
 *
 * Upstream Usage
 *
 * HTTPCoroSession* upstreamSession
 * HTTPCoroSession::RequestReservation reservation;
 *
 * auto sink = std::make_unique<HTTPStreamSourceSink>(
 *    evb, upstreamSession->acquireKeepAlive(), handler);
 * sink->transact(upstreamSession, std::move(reservation))
 *   .scheduleOn(evb).start();
 */
class HTTPStreamSourceUpstreamSink
    : public HTTPSink
    , public HTTPStreamSource::Callback {
 public:
  explicit HTTPStreamSourceUpstreamSink(folly::EventBase* handlerEvb,
                                        HTTPSessionContextPtr sessionCtx,
                                        HTTPTransactionHandler* handler);
  ~HTTPStreamSourceUpstreamSink() override;

  HTTPSource* getEgressSource() {
    return &egressSource_;
  }
  [[nodiscard]] virtual folly::Optional<HTTPCodec::StreamID> getStreamID()
      const override {
    return id_;
  }
  [[nodiscard]] virtual CodecProtocol getCodecProtocol() const override {
    return sessionCtx_->getCodecProtocol();
  }
  [[nodiscard]] folly::Optional<HTTPPriority> getHTTPPriority() const override {
    // TODO: !
    return folly::none;
  }
  [[nodiscard]] const folly::SocketAddress& getLocalAddress() const override {
    return sessionCtx_->getLocalAddress();
  }
  [[nodiscard]] const folly::SocketAddress& getPeerAddress() const override {
    return sessionCtx_->getPeerAddress();
  }
  [[nodiscard]] const folly::AsyncTransportCertificate* getPeerCertificate()
      const override {
    return sessionCtx_->getPeerCertificate();
  }
  [[nodiscard]] int getTCPTransportFD() const override {
    return sessionCtx_->getAsyncTransportFD();
  }
  [[nodiscard]] quic::QuicSocket* getQUICTransport() const override {
    return sessionCtx_->getQUICTransport();
  }
  [[nodiscard]] std::chrono::seconds getSessionIdleDuration() const override {
    return std::chrono::seconds(0);
  }
  void getCurrentFlowControlInfo(FlowControlInfo*) const override {
  }
  [[nodiscard]] CompressionInfo getHeaderCompressionInfo() const override {
    return CompressionInfo();
  }
  void detachAndAbortIfIncomplete(std::unique_ptr<HTTPSink> self) override;

  // Sending data
  void sendHeaders(const HTTPMessage& headers) override {
    sendHeadersWithOptionalEOM(headers, false);
  }
  bool sendHeadersWithDelegate(
      const HTTPMessage& headers,
      std::unique_ptr<DSRRequestSender> sender) override {
    LOG(FATAL) << "No DSR yet in proxygen::coro";
    return false;
  }
  void sendHeadersWithEOM(const HTTPMessage& headers) override {
    sendHeadersWithOptionalEOM(headers, true);
  }
  void sendHeadersWithOptionalEOM(const HTTPMessage& headers,
                                  bool eom) override;
  void sendBody(std::unique_ptr<folly::IOBuf> body) override;
  void sendChunkHeader(size_t /*length*/) override {
  }
  void sendChunkTerminator() override {
  }
  void sendTrailers(const HTTPHeaders& trailers) override {
    egressSource_.trailers(std::make_unique<HTTPHeaders>(trailers));
  }
  void sendPadding(uint16_t bytes) override {
    egressSource_.padding(bytes);
  }
  void sendEOM() override {
    egressSource_.eom();
  }
  bool isEgressEOMSeen() override {
    return egressSource_.isEOMSeen();
  }
  void sendAbort() override;
  void updateAndSendPriority(HTTPPriority /*priority*/) override {
    // TODO: Implement changing priority
  }
  bool trackEgressBodyOffset(uint64_t, ByteEventFlags) override {
    // TODO:
    return false;
  }
  bool canSendHeaders() const override {
    return egressSource_.headersAllowed();
  }
  const wangle::TransportInfo& getSetupTransportInfo() const noexcept override {
    return sessionCtx_->getSetupTransportInfo();
  }
  void getCurrentTransportInfo(wangle::TransportInfo* tinfo) const override {
    sessionCtx_->getCurrentTransportInfo(tinfo,
                                         /*includeSetupFields=*/true);
  }
  // Flow control
  void pauseIngress() override {
    XLOG(DBG8) << __func__;
    ingressResumed_.reset();
  }
  void resumeIngress() override {
    XLOG(DBG8) << __func__;
    ingressResumed_.signal();
    /**
     * due to InlineExecutor being used, this will resume read loop inline and
     * may cause the HTTPTxn to detach; it's important nothing comes after this
     * line or a potential uaf
     */
  }
  bool isIngressPaused() const override {
    return !ingressResumed_.ready();
  }
  bool isEgressPaused() const override {
    return windowState_ == HTTPStreamSource::FlowControlState::CLOSED;
  }
  void setEgressRateLimit(uint64_t /*bitsPerSecond*/) override {
    // We can implement this by declaring a RateLimitFilter here and returning
    // it in a chain with the egressSource.  Unfortunately, we pay a cost for
    // that extra filter and never use it (upstream, for now).  So just no-op.
  }
  // Timeout
  void setIdleTimeout(std::chrono::milliseconds timeout) override {
    // TODO(@damlaj): implement
  }
  void timeoutExpired() override {
    cancellationSource_.requestCancellation();
  }
  // Capabilities
  bool supportsPush() const override {
    // Technically HTTPCoroSession supports server push, but we'd need to
    // abstract out more of the push API.
    return false;
  }

  // Logging
  void describe(std::ostream& os) override {
    auto& peerAddr = sessionCtx_->getPeerAddress();
    auto& localAddr = sessionCtx_->getLocalAddress();
    if (sessionCtx_->isDownstream()) {
      os << "downstream=" << peerAddr << ", " << localAddr << "=local";
    } else {
      os << ", local=" << localAddr << ", " << peerAddr << "=upstream";
    }
    os << ", streamID=" << (id_.value_or(HTTPCodec::MaxStreamID));
  }

  virtual folly::coro::Task<void> transact(
      HTTPCoroSession* upstreamSession,
      HTTPCoroSession::RequestReservation reservation);

 private:
  folly::coro::Task<void> read(HTTPSourceHolder ingressSource);

  void windowOpen(HTTPCodec::StreamID) override {
    windowState_ = HTTPStreamSource::FlowControlState::OPEN;
    handler_->onEgressResumed();
  }

  void sourceComplete(HTTPCodec::StreamID /*id*/,
                      folly::Optional<HTTPError> /*error*/) override {
    gate_.set(Event::EgressComplete);
  }

  HTTPSessionContextPtr sessionCtx_;
  struct EgressHeaders {
    const HTTPMessage* msg{nullptr};
    bool eom{false};
  } egressHeaders_;

  class EgressSource : public HTTPStreamSource {
   public:
    using HTTPStreamSource::HTTPStreamSource;
    using HTTPStreamSource::validateHeadersAndSkip;
  } egressSource_;
  HTTPSourceHolder ingressSource_;
  HTTPTransactionHandler* handler_{nullptr};
  folly::CancellationSource cancellationSource_;
  detail::CancellableBaton ingressResumed_;
  HTTPStreamSource::FlowControlState windowState_{
      HTTPStreamSource::FlowControlState::OPEN};
  folly::Optional<HTTPCodec::StreamID> id_;
  enum class Event { IngressComplete, EgressComplete };
  ConditionalGate<Event, 2> gate_;
  // after detachAndAbortIfIncomplete
  std::unique_ptr<HTTPSink> self_;
};

} // namespace proxygen::coro
