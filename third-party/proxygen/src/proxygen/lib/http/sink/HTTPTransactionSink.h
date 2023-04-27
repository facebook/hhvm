/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/logging/xlog.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>

#include "proxygen/lib/http/sink/HTTPSink.h"

namespace proxygen {

/**
 * A HTTPTransactionSink forwards events to the client txn.
 */
class HTTPTransactionSink : public HTTPSink {
 public:
  explicit HTTPTransactionSink(HTTPTransaction* clientTxn)
      : httpTransaction_{clientTxn} {
    XCHECK(clientTxn)
        << "HTTPTransactionSink must be created with a valid clientTxn.";
  }
  ~HTTPTransactionSink() override = default;
  [[nodiscard]] HTTPTransaction* FOLLY_NULLABLE getHTTPTxn() const override {
    return httpTransaction_;
  }

  void detachAndAbortIfIncomplete(std::unique_ptr<HTTPSink> self) override {
    CHECK_EQ(self.get(), this);
    httpTransaction_->setTransportCallback(nullptr);
    httpTransaction_->setHandler(nullptr);
    if (!(httpTransaction_->isEgressComplete() ||
          httpTransaction_->isEgressEOMQueued()) ||
        !httpTransaction_->isIngressComplete()) {
      sendAbort();
    }
    self.reset();
  }

  // Sending data
  void sendHeaders(const HTTPMessage& headers) override {
    httpTransaction_->sendHeaders(headers);
  }
  bool sendHeadersWithDelegate(
      const HTTPMessage& headers,
      std::unique_ptr<DSRRequestSender> sender) override {
    return httpTransaction_->sendHeadersWithDelegate(headers,
                                                     std::move(sender));
  }
  void sendHeadersWithEOM(const HTTPMessage& headers) override {
    httpTransaction_->sendHeadersWithEOM(headers);
  }
  void sendHeadersWithOptionalEOM(const HTTPMessage& headers,
                                  bool eom) override {
    httpTransaction_->sendHeadersWithOptionalEOM(headers, eom);
  }
  void sendBody(std::unique_ptr<folly::IOBuf> body) override {
    httpTransaction_->sendBody(std::move(body));
  }
  void sendChunkHeader(size_t length) override {
    httpTransaction_->sendChunkHeader(length);
  }
  void sendChunkTerminator() override {
    httpTransaction_->sendChunkTerminator();
  }
  void sendTrailers(const HTTPHeaders& trailers) override {
    httpTransaction_->sendTrailers(trailers);
  }
  void sendEOM() override {
    httpTransaction_->sendEOM();
  }
  void sendAbort() override {
    httpTransaction_->sendAbort();
  }
  [[nodiscard]] bool canSendHeaders() const override {
    return httpTransaction_->canSendHeaders();
  }
  const wangle::TransportInfo& getSetupTransportInfo() const noexcept override {
    return httpTransaction_->getSetupTransportInfo();
  }
  void getCurrentTransportInfo(wangle::TransportInfo* tinfo) const override {
    httpTransaction_->getCurrentTransportInfo(tinfo);
  }
  // Flow control
  void pauseIngress() override {
    httpTransaction_->pauseIngress();
  }
  void resumeIngress() override {
    httpTransaction_->resumeIngress();
  }
  [[nodiscard]] bool isIngressPaused() const override {
    return httpTransaction_->isIngressPaused();
  }
  [[nodiscard]] bool isEgressPaused() const override {
    return httpTransaction_->isEgressPaused();
  }
  void setEgressRateLimit(uint64_t bitsPerSecond) override {
    httpTransaction_->setEgressRateLimit(bitsPerSecond);
  }
  // Client timeout
  void timeoutExpired() override {
    httpTransaction_->timeoutExpired();
  }
  void setIdleTimeout(std::chrono::milliseconds timeout) override {
    httpTransaction_->setIdleTimeout(timeout);
  }
  // Capabilities
  bool safeToUpgrade(HTTPMessage* req) const override;
  [[nodiscard]] bool supportsPush() const override {
    return true;
  }
  // Logging
  void describe(std::ostream& os) override {
    os << *httpTransaction_;
  }

 private:
  HTTPTransaction* httpTransaction_;
};

} // namespace proxygen
