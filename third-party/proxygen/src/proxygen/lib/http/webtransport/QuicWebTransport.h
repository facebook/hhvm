/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/logging/xlog.h>
#include <proxygen/lib/http/webtransport/WebTransportImpl.h>
#include <quic/api/QuicSocket.h>

namespace proxygen {

class QuicWebTransport
    : private WebTransportImpl::TransportProvider
    , private WebTransportImpl::SessionProvider
    , private quic::QuicSocket::ConnectionCallback
    , private quic::QuicSocket::DatagramCallback
    , public WebTransportImpl {

 public:
  explicit QuicWebTransport(std::shared_ptr<quic::QuicSocket> quicSocket)
      : WebTransportImpl(
            static_cast<WebTransportImpl::TransportProvider&>(*this),
            static_cast<WebTransportImpl::SessionProvider&>(*this)),
        quicSocket_(std::move(quicSocket)) {
    quicSocket_->setConnectionCallback(this);
    quicSocket_->setDatagramCallback(this);
  }

  ~QuicWebTransport() override = default;

  void setHandler(WebTransportHandler* handler) {
    handler_ = handler;
  }

 private:
  void onFlowControlUpdate(quic::StreamId /*id*/) noexcept override;

  void onNewBidirectionalStream(quic::StreamId id) noexcept override;

  void onNewUnidirectionalStream(quic::StreamId id) noexcept override;

  void onStopSending(quic::StreamId id,
                     quic::ApplicationErrorCode error) noexcept override;

  void onConnectionEnd() noexcept override;
  void onConnectionError(quic::QuicError code) noexcept override;
  void onConnectionEnd(quic::QuicError /* error */) noexcept override;
  void onBidirectionalStreamsAvailable(
      uint64_t /*numStreamsAvailable*/) noexcept override;

  void onUnidirectionalStreamsAvailable(
      uint64_t /*numStreamsAvailable*/) noexcept override;

  folly::Expected<HTTPCodec::StreamID, WebTransport::ErrorCode>
  newWebTransportBidiStream() override;

  folly::Expected<HTTPCodec::StreamID, WebTransport::ErrorCode>
  newWebTransportUniStream() override;

  folly::SemiFuture<folly::Unit> awaitUniStreamCredit() override;

  folly::SemiFuture<folly::Unit> awaitBidiStreamCredit() override;

  folly::Expected<WebTransport::FCState, WebTransport::ErrorCode>
  sendWebTransportStreamData(HTTPCodec::StreamID /*id*/,
                             std::unique_ptr<folly::IOBuf> /*data*/,
                             bool /*eof*/) override;

  folly::Expected<folly::Unit, WebTransport::ErrorCode>
  notifyPendingWriteOnStream(HTTPCodec::StreamID,
                             quic::StreamWriteCallback* wcb) override;

  folly::Expected<folly::Unit, WebTransport::ErrorCode> resetWebTransportEgress(
      HTTPCodec::StreamID /*id*/, uint32_t /*errorCode*/) override;

  folly::Expected<folly::Unit, WebTransport::ErrorCode>
      setWebTransportStreamPriority(HTTPCodec::StreamID /*id*/,
                                    HTTPPriority /*pri*/) override;

  folly::Expected<std::pair<std::unique_ptr<folly::IOBuf>, bool>,
                  WebTransport::ErrorCode>
  readWebTransportData(HTTPCodec::StreamID id, size_t max) override {
    auto res = quicSocket_->read(id, max);
    if (res) {
      return std::move(res.value());
    } else {
      return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
    }
  }

  folly::Expected<folly::Unit, WebTransport::ErrorCode>
  initiateReadOnBidiStream(HTTPCodec::StreamID id,
                           quic::StreamReadCallback* readCallback) override {
    auto res = quicSocket_->setReadCallback(id, readCallback);
    if (res) {
      return folly::unit;
    } else {
      return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
    }
  }

  folly::Expected<folly::Unit, WebTransport::ErrorCode>
      pauseWebTransportIngress(HTTPCodec::StreamID /*id*/) override;

  folly::Expected<folly::Unit, WebTransport::ErrorCode>
      resumeWebTransportIngress(HTTPCodec::StreamID /*id*/) override;

  folly::Expected<folly::Unit, WebTransport::ErrorCode>
      stopReadingWebTransportIngress(
          HTTPCodec::StreamID /*id*/,
          folly::Optional<uint32_t> /*errorCode*/) override;

  folly::Expected<folly::Unit, WebTransport::ErrorCode> sendDatagram(
      std::unique_ptr<folly::IOBuf> /*datagram*/) override;

  bool usesEncodedApplicationErrorCodes() override {
    return false;
  }

  folly::Expected<folly::Unit, WebTransport::ErrorCode> closeSession(
      folly::Optional<uint32_t> /*error*/) override;

  void onDatagramsAvailable() noexcept override;

  void onConnectionEndImpl(folly::Optional<quic::QuicError> error);

  std::shared_ptr<quic::QuicSocket> quicSocket_;
  WebTransportHandler* handler_{nullptr};
  folly::Optional<folly::Promise<folly::Unit>> waitingForUniStreams_;
  folly::Optional<folly::Promise<folly::Unit>> waitingForBidiStreams_;
};

} // namespace proxygen
