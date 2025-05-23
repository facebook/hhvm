/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/logging/xlog.h>
#include <proxygen/lib/http/HTTPMessageFilters.h>
#include <proxygen/lib/http/codec/webtransport/WebTransportCapsuleCodec.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <proxygen/lib/http/webtransport/WebTransport.h>

namespace proxygen {

/**
 * WebTransportFilter class definition.
 *
 * HTTPSession/HQSession will install this filter on a WebTransport transaction.
 * The filter holds some session state like the flow controller. The onBody
 * callbacks will pass in the ingress to the
 * H2WebTransportCodec/H3WebTransportCodec, which then call the appropriate
 * capsule callbacks.
 */
class WebTransportFilter
    : public HTTPMessageFilter
    , public WebTransportImpl::TransportProvider
    , public HTTPTransactionTransportCallback
    , public WebTransportCapsuleCodec::Callback {
 public:
  explicit WebTransportFilter(HTTPTransaction* txn, CodecVersion version)
      : txn_(txn) {
    codec_ = std::make_unique<WebTransportCapsuleCodec>(this, version);
    txn_->setWTTransportProvider(
        static_cast<WebTransportImpl::TransportProvider*>(this));
    txn_->setTransportCallback(
        static_cast<HTTPTransactionTransportCallback*>(this));
    if (txn->isDownstream()) {
      nextNewWTBidiStream_++;
      nextNewWTUniStream_++;
    }
  }

  void clearTransaction() {
    txn_ = nullptr;
  }

  void onBody(std::unique_ptr<folly::IOBuf> chain) noexcept override {
    codec_->onIngress(std::move(chain), false);
  }

  void onEOM() noexcept override {
    codec_->onIngress(nullptr, true);
  }

  void trackedByteEventTX(const ByteEvent& event) noexcept override {
  }
  void firstHeaderByteFlushed() noexcept override {
  }
  void firstByteFlushed() noexcept override {
  }
  void lastByteFlushed() noexcept override {
  }
  void lastByteAcked(std::chrono::milliseconds latency) noexcept override {
  }
  void headerBytesGenerated(HTTPHeaderSize& size) noexcept override {
  }
  void headerBytesReceived(const HTTPHeaderSize& size) noexcept override {
  }
  void bodyBytesGenerated(size_t nbytes) noexcept override {
  }
  void bodyBytesReceived(size_t size) noexcept override {
  }

  folly::Expected<folly::Unit, WebTransport::ErrorCode> sendDatagram(
      std::unique_ptr<folly::IOBuf> datagram) override {
    return folly::unit;
  }

  folly::Expected<HTTPCodec::StreamID, WebTransport::ErrorCode>
  newWebTransportBidiStream() override {
    // TODO: Flow control
    auto res = nextNewWTBidiStream_;
    nextNewWTBidiStream_ += 4;
    return res;
  }

  folly::Expected<HTTPCodec::StreamID, WebTransport::ErrorCode>
  newWebTransportUniStream() override {
    // TODO: Flow control
    auto res = nextNewWTUniStream_;
    nextNewWTUniStream_ += 4;
    return res;
  }

  folly::Expected<WebTransport::FCState, WebTransport::ErrorCode>
  sendWebTransportStreamData(HTTPCodec::StreamID id,
                             std::unique_ptr<folly::IOBuf> data,
                             bool eof,
                             WebTransport::ByteEventCallback* wcb) override {
    return WebTransport::FCState::UNBLOCKED;
  }

  folly::Expected<folly::Unit, WebTransport::ErrorCode> resetWebTransportEgress(
      HTTPCodec::StreamID id, uint32_t errorCode) override {
    return folly::unit;
  }

  folly::Expected<folly::Unit, WebTransport::ErrorCode>
  setWebTransportStreamPriority(HTTPCodec::StreamID id,
                                HTTPPriority pri) override {
    return folly::unit;
  }

  folly::Expected<std::pair<std::unique_ptr<folly::IOBuf>, bool>,
                  WebTransport::ErrorCode>
  readWebTransportData(HTTPCodec::StreamID id, size_t max) override {
    return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
  }

  folly::Expected<folly::Unit, WebTransport::ErrorCode>
  initiateReadOnBidiStream(HTTPCodec::StreamID id,
                           quic::StreamReadCallback* readCallback) override {
    return folly::unit;
  }

  folly::Expected<folly::Unit, WebTransport::ErrorCode>
  pauseWebTransportIngress(HTTPCodec::StreamID id) override {
    return folly::unit;
  }

  folly::Expected<folly::Unit, WebTransport::ErrorCode>
  resumeWebTransportIngress(HTTPCodec::StreamID id) override {
    return folly::unit;
  }

  folly::Expected<folly::Unit, WebTransport::ErrorCode>
  stopReadingWebTransportIngress(HTTPCodec::StreamID id,
                                 folly::Optional<uint32_t> errorCode) override {
    return folly::unit;
  }

  void onWTResetStreamCapsule(WTResetStreamCapsule capsule) override {
    XLOG(DBG1) << __func__;
  }
  void onWTStopSendingCapsule(WTStopSendingCapsule capsule) override {
    XLOG(DBG1) << __func__;
  }
  void onWTStreamCapsule(WTStreamCapsule capsule) override {
    XLOG(DBG1) << __func__;
  }
  void onWTMaxDataCapsule(WTMaxDataCapsule capsule) override {
    XLOG(DBG1) << __func__;
  }
  void onWTMaxStreamDataCapsule(WTMaxStreamDataCapsule capsule) override {
    XLOG(DBG1) << __func__;
  }
  void onWTMaxStreamsCapsule(WTMaxStreamsCapsule capsule) override {
    XLOG(DBG1) << __func__;
  }
  void onWTDataBlockedCapsule(WTDataBlockedCapsule capsule) override {
    XLOG(DBG1) << __func__;
  }
  void onWTStreamDataBlockedCapsule(
      WTStreamDataBlockedCapsule capsule) override {
    XLOG(DBG1) << __func__;
  }
  void onWTStreamsBlockedCapsule(WTStreamsBlockedCapsule capsule) override {
    XLOG(DBG1) << __func__;
  }
  void onDatagramCapsule(DatagramCapsule capsule) override {
    XLOG(DBG1) << __func__;
  }
  void onCloseWebTransportSessionCapsule(
      CloseWebTransportSessionCapsule capsule) override {
    XLOG(DBG1) << __func__;
  }
  void onDrainWebTransportSessionCapsule(
      DrainWebTransportSessionCapsule capsule) override {
    XLOG(DBG1) << __func__;
  }

 private:
  HTTPTransaction* txn_;
  std::unique_ptr<CapsuleCodec> codec_;
  uint64_t nextNewWTBidiStream_{0};
  uint64_t nextNewWTUniStream_{2};
  uint64_t bodyOffset_{0};
  folly::F14FastMap<HTTPCodec::StreamID, WebTransportImpl::StreamReadHandle*>
      readCallbacks_;
  struct WriteCallback {
    HTTPCodec::StreamID id;
    uint64_t bodyOffset;
    quic::StreamWriteCallback* wcb;
  };
  std::list<WriteCallback> writeCallbacks_;
};

} // namespace proxygen
