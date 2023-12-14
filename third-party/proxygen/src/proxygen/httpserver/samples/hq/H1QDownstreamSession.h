/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/HTTP1xCodec.h>
#include <proxygen/lib/http/session/HTTPDownstreamSession.h>
#include <quic/api/QuicSocket.h>
#include <quic/api/QuicStreamAsyncTransport.h>
#include <quic/common/events/FollyQuicEventBase.h>

namespace quic::samples {

class H1QDownstreamSession : public quic::QuicSocket::ConnectionCallback {
 public:
  H1QDownstreamSession(std::shared_ptr<quic::QuicSocket> sock,
                       proxygen::HTTPSessionController* controller,
                       wangle::ConnectionManager* connMgr)
      : sock_(std::move(sock)), controller_(controller), connMgr_(connMgr) {
    sock_->setConnectionCallback(this);
    // hold a place for this container session (HQSessionController doesn't
    // use the arg)
    controller_->attachSession(nullptr);
  }

  ~H1QDownstreamSession() override {
    controller_->detachSession(nullptr);
    if (sock_) {
      sock_->setConnectionCallback(nullptr);
    }
  }

  void onNewBidirectionalStream(quic::StreamId id) noexcept override {
    auto streamTransport =
        quic::QuicStreamAsyncTransport::createWithExistingStream(sock_, id);
    if (!streamTransport) {
      LOG(ERROR) << "Failed to create stream transport";
      sock_->stopSending(
          id,
          quic::ApplicationErrorCode(
              proxygen::HTTP3::ErrorCode::HTTP_REQUEST_REJECTED));
      sock_->resetStream(
          id,
          quic::ApplicationErrorCode(
              proxygen::HTTP3::ErrorCode::HTTP_REQUEST_REJECTED));
      return;
    }
    auto codec = std::make_unique<proxygen::HTTP1xCodec>(
        proxygen::TransportDirection::DOWNSTREAM,
        /*force1_1=*/false);
    wangle::TransportInfo tinfo;
    auto session = new proxygen::HTTPDownstreamSession(
        proxygen::WheelTimerInstance(
            std::chrono::seconds(5),
            sock_->getEventBase()
                ->getTypedEventBase<FollyQuicEventBase>()
                ->getBackingEventBase()),
        std::move(streamTransport),
        sock_->getLocalAddress(),
        sock_->getPeerAddress(),
        controller_,
        std::move(codec),
        tinfo,
        nullptr);
    connMgr_->addConnection(session);
    session->startNow();
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
  proxygen::HTTPSessionController* controller_{nullptr};
  wangle::ConnectionManager* connMgr_{nullptr};
};

} // namespace quic::samples
