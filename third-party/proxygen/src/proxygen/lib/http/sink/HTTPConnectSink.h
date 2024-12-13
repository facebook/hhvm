/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTransport.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <proxygen/lib/http/sink/HTTPSink.h>
#include <proxygen/lib/utils/ConditionalGate.h>

namespace proxygen {

/**
 * A HTTPConnectSink writes data straight to an AsyncTransport.
 */
class HTTPConnectSink
    : public HTTPSink
    , public folly::DestructorCheck
    , public folly::AsyncTransport::ReadCallback
    , private folly::AsyncTransport::WriteCallback
    , private folly::HHWheelTimer::Callback {

 public:
  explicit HTTPConnectSink(folly::AsyncTransport::UniquePtr socket,
                           HTTPTransactionHandler* handler)
      : sock_(std::move(socket)), handler_(handler) {
    CHECK(sock_);
    sock_->getLocalAddress(&localAddress_);
    sock_->getPeerAddress(&peerAddress_);
  }

  ~HTTPConnectSink() override = default;

  [[nodiscard]] folly::Optional<HTTPCodec::StreamID> getStreamID()
      const override {
    return folly::none;
  }

  [[nodiscard]] CodecProtocol getCodecProtocol() const override {
    // This is meaningless :(
    return CodecProtocol::HTTP_1_1;
  }

  [[nodiscard]] const folly::SocketAddress& getLocalAddress() const override {
    return localAddress_;
  }

  [[nodiscard]] const folly::SocketAddress& getPeerAddress() const override {
    return peerAddress_;
  }

  [[nodiscard]] const folly::AsyncTransportCertificate* getPeerCertificate()
      const override {
    return sock_->getPeerCertificate();
  }

  [[nodiscard]] folly::Optional<HTTPPriority> getHTTPPriority() const override {
    return folly::none;
  }

  [[nodiscard]] int getTCPTransportFD() const override {
    return CHECK_NOTNULL(sock_->getUnderlyingTransport<folly::AsyncSocket>())
        ->getNetworkSocket()
        .toFd();
  }

  [[nodiscard]] quic::QuicSocket* getQUICTransport() const override {
    return nullptr;
  }

  [[nodiscard]] std::chrono::seconds getSessionIdleDuration() const override {
    return std::chrono::seconds(0);
  }

  void getCurrentFlowControlInfo(FlowControlInfo*) const override {
  }

  [[nodiscard]] CompressionInfo getHeaderCompressionInfo() const override {
    return {};
  }

  void detachAndAbortIfIncomplete(std::unique_ptr<HTTPSink> self) override;
  // Sending data
  void sendHeaders(const HTTPMessage&) override {
  }

  bool sendHeadersWithDelegate(const HTTPMessage&,
                               std::unique_ptr<DSRRequestSender>) override {
    return true;
  }

  void sendHeadersWithEOM(const HTTPMessage&) override {
    sendEOM();
  }

  void sendHeadersWithOptionalEOM(const HTTPMessage&, bool eom) override {
    if (eom) {
      sendEOM();
    }
  }

  void sendBody(std::unique_ptr<folly::IOBuf> body) override;

  void sendChunkHeader(size_t) override {
  }

  void sendChunkTerminator() override {
  }

  void sendTrailers(const HTTPHeaders&) override {
  }

  void sendPadding(uint16_t) override {
  }

  void sendEOM() override;

  bool isEgressEOMSeen() override;

  void sendAbort() override;

  void updateAndSendPriority(HTTPPriority) override {
  }

  bool trackEgressBodyOffset(uint64_t, ByteEventFlags) override {
    return false;
  }

  // Check state
  [[nodiscard]] bool canSendHeaders() const override {
    return sock_->good();
  }

  const wangle::TransportInfo& getSetupTransportInfo() const noexcept override {
    return transportInfo_;
  }

  void getCurrentTransportInfo(wangle::TransportInfo* tinfo) const override;

  // Flow control
  void pauseIngress() override;

  void resumeIngress() override;

  [[nodiscard]] bool isIngressPaused() const override;

  [[nodiscard]] bool isEgressPaused() const override;

  void setEgressRateLimit(uint64_t) override {
  }

  void timeoutExpired() noexcept override;

  void setIdleTimeout(std::chrono::milliseconds timeout) override;

  // Capabilities
  bool safeToUpgrade(HTTPMessage*) const override {
    return false;
  }

  [[nodiscard]] bool supportsPush() const override {
    return false;
  }

  // Logging
  void describe(std::ostream& os) override {
    os << "HTTPConnectSink on " << localAddress_ << " to " << peerAddress_;
  }

  // Callback methods
  void getReadBuffer(void** buf, size_t* bufSize) override;

  void readDataAvailable(size_t readSize) noexcept override;

  void readEOF() noexcept override;

  void readErr(const folly::AsyncSocketException& err) noexcept override;

  void writeSuccess() noexcept override;

  void writeErr(size_t,
                const folly::AsyncSocketException& err) noexcept override;

 private:
  void resetIdleTimeout() {
    cancelTimeout();
    if (idleTimeout_.count()) {
      sock_->getEventBase()->timer().scheduleTimeout(this, idleTimeout_);
    }
  }

  // Returns true if this sink was destroyed
  bool writeComplete();

  folly::AsyncTransport::UniquePtr sock_;
  folly::SocketAddress peerAddress_;
  folly::SocketAddress localAddress_;
  wangle::TransportInfo transportInfo_;

  /** Chain of ingress IOBufs */
  folly::IOBufQueue readBuf_{folly::IOBufQueue::cacheChainLength()};
  HTTPTransactionHandler* handler_{nullptr};

  std::chrono::milliseconds idleTimeout_{0};
  bool destroyOnWriteComplete_{false};
  uint8_t outstandingWrites_{0};

  bool ingressEOMRead_{false};
  bool egressEOMSeen_{false};
  bool handlerEgressPaused_{false};
};

} // namespace proxygen
