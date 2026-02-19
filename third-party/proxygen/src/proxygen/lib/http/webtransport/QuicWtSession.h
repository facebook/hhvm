/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/logging/xlog.h>
#include <proxygen/lib/http/webtransport/WebTransport.h>
#include <proxygen/lib/http/webtransport/WtStreamManager.h>
#include <proxygen/lib/http/webtransport/WtUtils.h>
#include <quic/api/QuicSocket.h>
#include <quic/priority/HTTPPriorityQueue.h>

namespace proxygen {

class QuicWtSession
    : public detail::WtSessionBase
    , private quic::QuicSocket::ReadCallback
    , private quic::QuicSocket::ConnectionCallback
    , private quic::QuicSocket::DatagramCallback
    , private quic::StreamWriteCallback
    , private detail::WtStreamManager::ReadCallback
    , public detail::WtStreamManager::EgressCallback
    , public detail::WtStreamManager::IngressCallback {

 public:
  explicit QuicWtSession(std::shared_ptr<quic::QuicSocket> quicSocket,
                         std::unique_ptr<WebTransportHandler> wtHandler);

  ~QuicWtSession() override;

  [[nodiscard]] quic::TransportInfo getTransportInfo() const noexcept override {
    XCHECK(quicSocket_);
    return quicSocket_->getTransportInfo();
  }

  [[nodiscard]] quic::Expected<quic::QuicSocketLite::FlowControlState,
                               quic::LocalErrorCode>
  getConnectionFlowControl() const {
    XCHECK(quicSocket_);
    return quicSocket_->getConnectionFlowControl();
  }

  folly::Expected<StreamWriteHandle*, ErrorCode> createUniStream() noexcept
      override;
  folly::Expected<BidiStreamHandle, ErrorCode> createBidiStream() noexcept
      override;
  folly::SemiFuture<folly::Unit> awaitUniStreamCredit() noexcept override;
  folly::SemiFuture<folly::Unit> awaitBidiStreamCredit() noexcept override;

  folly::Expected<folly::Unit, ErrorCode> sendDatagram(
      std::unique_ptr<folly::IOBuf> datagram) noexcept override;

  [[nodiscard]] const folly::SocketAddress& getLocalAddress() const override {
    return quicSocket_->getLocalAddress();
  }

  [[nodiscard]] const folly::SocketAddress& getPeerAddress() const override {
    return quicSocket_->getPeerAddress();
  }

  folly::Expected<folly::Unit, ErrorCode> closeSession(
      folly::Optional<uint32_t> error) noexcept override;

 private:
  // -- quic::QuicSocket::ReadCallback overrides --
  void readAvailable(quic::StreamId streamId) noexcept override;
  void readError(quic::StreamId streamId,
                 quic::QuicError error) noexcept override;

  // -- QuicSocket::ConnectionCallback overrides --
  void onNewBidirectionalStream(quic::StreamId id) noexcept override;
  void onNewUnidirectionalStream(quic::StreamId id) noexcept override;
  void onStopSending(quic::StreamId id,
                     quic::ApplicationErrorCode error) noexcept override;
  void onConnectionEnd() noexcept override;
  void onConnectionEnd(quic::QuicError /* error */) noexcept override;
  void onConnectionError(quic::QuicError code) noexcept override;
  void onBidirectionalStreamsAvailable(
      uint64_t /*numStreamsAvailable*/) noexcept override;
  void onUnidirectionalStreamsAvailable(
      uint64_t /*numStreamsAvailable*/) noexcept override;

  // -- QuicSocket::DatagramCallback overrides --
  void onDatagramsAvailable() noexcept override;

  // -- StreamWriteCallback overrides --
  void onStreamWriteReady(quic::StreamId id,
                          uint64_t maxToSend) noexcept override;
  void onStreamWriteError(quic::StreamId id,
                          quic::QuicError error) noexcept override;

  // -- WtStreamManager::ReadCallback overrides --
  void readReady(detail::WtStreamManager::WtReadHandle& rh) noexcept override;

  // -- WtStreamManager::EgressCallback overrides --
  void eventsAvailable() noexcept override;

  // -- WtStreamManager::IngressCallback overrides --
  void onNewPeerStream(uint64_t streamId) noexcept override;

  folly::Expected<folly::Unit, ErrorCode> closeSessionImpl(
      folly::Optional<uint32_t> error);
  void onConnectionEndImpl(const folly::Optional<quic::QuicError>& error);
  void maybePauseIngress(uint64_t id) noexcept;
  void maybeResumeIngress(uint64_t id) noexcept;

  std::shared_ptr<quic::QuicSocket> quicSocket_{nullptr};
  std::unique_ptr<WebTransportHandler> wtHandler_;
  std::unique_ptr<quic::HTTPPriorityQueue> priorityQueue_;
  detail::WtStreamManager sm_;
};

} // namespace proxygen
