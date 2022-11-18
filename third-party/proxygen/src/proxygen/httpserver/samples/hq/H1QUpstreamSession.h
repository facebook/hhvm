/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <proxygen/lib/http/session/HTTPUpstreamSession.h>
#include <quic/api/QuicSocket.h>
#include <quic/api/QuicStreamAsyncTransport.h>

namespace quic::samples {

class H1QUpstreamSession
    : public quic::QuicSocket::ConnectionCallback
    , public proxygen::HTTPSessionBase::InfoCallback {
 public:
  explicit H1QUpstreamSession(std::shared_ptr<quic::QuicSocket> sock)
      : sock_(std::move(sock)) {
    sock_->setConnectionCallback(this);
  }

  ~H1QUpstreamSession() override {
    if (sock_) {
      sock_->close(folly::none);
      sock_->setConnectionCallback(nullptr);
    }
  }

  proxygen::HTTPTransaction* newTransaction(
      proxygen::HTTPTransactionHandler* handler) {
    auto streamTransport =
        quic::QuicStreamAsyncTransport::createWithNewStream(sock_);
    if (!streamTransport) {
      LOG(ERROR) << "Failed to create stream transport";
      return nullptr;
    }
    auto codec = std::make_unique<proxygen::HTTP1xCodec>(
        proxygen::TransportDirection::UPSTREAM,
        /*force1_1=*/false);
    wangle::TransportInfo tinfo;
    auto session = new proxygen::HTTPUpstreamSession(
        proxygen::WheelTimerInstance(std::chrono::seconds(5),
                                     sock_->getEventBase()),
        std::move(streamTransport),
        sock_->getLocalAddress(),
        sock_->getPeerAddress(),
        std::move(codec),
        tinfo,
        this);
    session->startNow();
    return session->newTransaction(handler);
  }
  void onCreate(const proxygen::HTTPSessionBase&) override {
    txns_++;
  }

  void onDestroy(const proxygen::HTTPSessionBase&) override {
    if (--txns_ == 0 && draining_) {
      delete this;
    }
  }

  void drain() {
    draining_ = true;
    if (txns_ == 0) {
      delete this;
    }
  }

  void onNewBidirectionalStream(quic::StreamId id) noexcept override {
    sock_->resetStream(id,
                       quic::ApplicationErrorCode(
                           proxygen::HTTP3::ErrorCode::HTTP_REQUEST_REJECTED));
    sock_->stopSending(id,
                       quic::ApplicationErrorCode(
                           proxygen::HTTP3::ErrorCode::HTTP_REQUEST_REJECTED));
  }
  void onNewUnidirectionalStream(quic::StreamId id) noexcept override {
    sock_->stopSending(id,
                       quic::ApplicationErrorCode(
                           proxygen::HTTP3::ErrorCode::HTTP_REQUEST_REJECTED));
  }

  // ignore
  void onStopSending(quic::StreamId,
                     quic::ApplicationErrorCode) noexcept override {
  }
  void onConnectionEnd() noexcept override {
    LOG(INFO) << __func__;
    delete this;
  }
  using proxygen::HTTPSessionBase::InfoCallback::onConnectionError;
  void onConnectionError(quic::QuicError) noexcept override {
    LOG(INFO) << __func__;
    delete this;
  }
  void onConnectionEnd(quic::QuicError /* error */) noexcept override {
    LOG(INFO) << __func__;
    delete this;
  }

 private:
  std::shared_ptr<quic::QuicSocket> sock_;
  uint64_t txns_{0};
  bool draining_ = false;
};

} // namespace quic::samples
