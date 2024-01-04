/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/CppAttributes.h>
#include <proxygen/lib/http/HTTPMessage.h>
#include <proxygen/lib/http/sink/FlowControlInfo.h>

namespace folly {
class AsyncTransport;
}
namespace quic {
class QuicSocket;
}

namespace proxygen {

/**
 * A HTTPSink wraps a http transaction (ClientTxnSink), or an AsyncRequest
 * (AsyncRequestSink).
 *
 * Its purpose is to receive http txn events from HTTPRevProxyHandler.
 * This abstraction allows HTTPRevProxyHandler to continue the normal flow
 * in a similar way as it had http txn.
 *
 * Life-cycle
 * ==========
 * A HTTPSink is owned by a unique_ptr in HTTPRevProxyHandler. It's created
 * when a http txn (/AyncRequest) is set, and destructed when the http txn
 * /AsyncRequest finishes.
 */
class HTTPSink {
 public:
  virtual ~HTTPSink() = default;

  [[nodiscard]] virtual folly::Optional<HTTPCodec::StreamID> getStreamID()
      const = 0;
  [[nodiscard]] virtual CodecProtocol getCodecProtocol() const = 0;
  [[nodiscard]] virtual const folly::SocketAddress& getLocalAddress() const = 0;
  [[nodiscard]] virtual const folly::SocketAddress& getPeerAddress() const = 0;
  [[nodiscard]] virtual folly::Optional<HTTPPriority> getHTTPPriority()
      const = 0;
  [[nodiscard]] virtual const folly::AsyncTransport* getTCPTransport()
      const = 0;
  [[nodiscard]] virtual quic::QuicSocket* getQUICTransport() const = 0;
  [[nodiscard]] virtual std::chrono::seconds getSessionIdleDuration() const = 0;
  virtual void getCurrentFlowControlInfo(FlowControlInfo*) const = 0;
  [[nodiscard]] virtual CompressionInfo getHeaderCompressionInfo() const = 0;

  virtual void detachAndAbortIfIncomplete(std::unique_ptr<HTTPSink> self) = 0;
  // Sending data
  virtual void sendHeaders(const HTTPMessage& headers) = 0;
  virtual bool sendHeadersWithDelegate(
      const HTTPMessage& headers, std::unique_ptr<DSRRequestSender> sender) = 0;
  virtual void sendHeadersWithEOM(const HTTPMessage& headers) = 0;
  virtual void sendHeadersWithOptionalEOM(const HTTPMessage& headers,
                                          bool eom) = 0;
  virtual void sendBody(std::unique_ptr<folly::IOBuf> body) = 0;
  virtual void sendChunkHeader(size_t length) = 0;
  virtual void sendChunkTerminator() = 0;
  virtual void sendTrailers(const HTTPHeaders& trailers) = 0;
  virtual void sendEOM() = 0;
  virtual bool isEgressEOMSeen() = 0;
  virtual void sendAbort() = 0;
  virtual void updateAndSendPriority(HTTPPriority priority) = 0;
  enum class ByteEventFlags : uint8_t { ACK = 0x01, TX = 0x02 };
  virtual bool trackEgressBodyOffset(uint64_t bodyOffset,
                                     ByteEventFlags flags) = 0;

  // Check state
  [[nodiscard]] virtual bool canSendHeaders() const = 0;
  virtual const wangle::TransportInfo& getSetupTransportInfo()
      const noexcept = 0;
  virtual void getCurrentTransportInfo(wangle::TransportInfo* tinfo) const = 0;

  // Flow control
  virtual void pauseIngress() = 0;
  virtual void resumeIngress() = 0;
  [[nodiscard]] virtual bool isIngressPaused() const = 0;
  [[nodiscard]] virtual bool isEgressPaused() const = 0;
  virtual void setEgressRateLimit(uint64_t bitsPerSecond) = 0;
  // Client timeout
  virtual void timeoutExpired() = 0;
  virtual void setIdleTimeout(std::chrono::milliseconds timeout) = 0;
  // Capabilities
  virtual bool safeToUpgrade(HTTPMessage* req) const = 0;
  [[nodiscard]] virtual bool supportsPush() const = 0;
  // Logging
  virtual void describe(std::ostream& os) = 0;
};

} // namespace proxygen
