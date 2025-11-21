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
#include <proxygen/lib/http/codec/HQUtils.h>
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
    if (version == CodecVersion::H3) {
      h3Tp_ = txn_->getWTTransportProvider();
    }
    txn_->setWTTransportProvider(
        static_cast<WebTransportImpl::TransportProvider*>(this));
    txn_->setTransportCallback(
        static_cast<HTTPTransactionTransportCallback*>(this));
    if (txn->isDownstream()) {
      nextNewWTBidiStream_++;
      nextNewWTUniStream_++;
    }
  }

  static std::unique_ptr<WebTransportFilter> make(HTTPTransaction* txn,
                                                  CodecVersion version) {
    auto filter = std::make_unique<WebTransportFilter>(txn, version);
    if (auto* nextTxnHandler = txn->getHandler()) {
      filter->setNextTransactionHandler(nextTxnHandler);
    }
    txn->setHandler(filter.get());
    return filter;
  }

  void clearTransaction() {
    txn_ = nullptr;
  }

  void setHandler(WebTransportHandler* handler) {
    handler_ = handler;
  }

  void setWebTransportImpl(WebTransportImpl* wtImpl) {
    wtImpl_ = wtImpl;
  }

  void onBody(std::unique_ptr<folly::IOBuf> chain) noexcept override {
    if (sessionClosed_) {
      XLOG(ERR) << "Received additional data after WT_CLOSE_SESSION capsule, "
                << "resetting CONNECT stream with H3_MESSAGE_ERROR";
      if (txn_) {
        auto errorCode =
            hq::hqToHttpErrorCode(HTTP3::ErrorCode::HTTP_MESSAGE_ERROR);
        txn_->sendAbort(errorCode);
      }
      return;
    }
    codec_->onIngress(std::move(chain), false);
  }

  void onEOM() noexcept override {
    codec_->onIngress(nullptr, true);

    if (nextTransactionHandler_) {
      nextTransactionHandler_->onEOM();
    }
  }

  void onDatagram(std::unique_ptr<folly::IOBuf> datagram) noexcept override {
    if (nextTransactionHandler_) {
      nextTransactionHandler_->onDatagram(std::move(datagram));
    }
  }

  void onWebTransportBidiStream(
      HTTPCodec::StreamID id,
      WebTransport::BidiStreamHandle stream) noexcept override {
    if (nextTransactionHandler_) {
      nextTransactionHandler_->onWebTransportBidiStream(id, std::move(stream));
    }
  }

  void onWebTransportUniStream(
      HTTPCodec::StreamID id,
      WebTransport::StreamReadHandle* stream) noexcept override {
    if (nextTransactionHandler_) {
      nextTransactionHandler_->onWebTransportUniStream(id, stream);
    }
  }

  std::unique_ptr<HTTPMessageFilter> clone() noexcept override {
    return nullptr;
  }

  void trackedByteEventTX(const ByteEvent& /*event*/) noexcept override {
  }
  void firstHeaderByteFlushed() noexcept override {
  }
  void firstByteFlushed() noexcept override {
  }
  void lastByteFlushed() noexcept override {
  }
  void lastByteAcked(std::chrono::milliseconds /*latency*/) noexcept override {
  }
  void headerBytesGenerated(HTTPHeaderSize& /*size*/) noexcept override {
  }
  void headerBytesReceived(const HTTPHeaderSize& /*size*/) noexcept override {
  }
  void bodyBytesGenerated(size_t /*nbytes*/) noexcept override {
  }
  void bodyBytesReceived(size_t /*size*/) noexcept override {
  }

  folly::Expected<folly::Unit, WebTransport::ErrorCode> sendDatagram(
      std::unique_ptr<folly::IOBuf> datagram) override {
    if (h3Tp_) {
      return h3Tp_->sendDatagram(std::move(datagram));
    }
    return folly::unit;
  }

  folly::Expected<HTTPCodec::StreamID, WebTransport::ErrorCode>
  newWebTransportBidiStream() override {
    // TODO: Flow control
    if (h3Tp_) {
      return h3Tp_->newWebTransportBidiStream();
    }
    auto res = nextNewWTBidiStream_;
    nextNewWTBidiStream_ += 4;
    return res;
  }

  folly::Expected<HTTPCodec::StreamID, WebTransport::ErrorCode>
  newWebTransportUniStream() override {
    // TODO: Flow control
    if (h3Tp_) {
      return h3Tp_->newWebTransportUniStream();
    }
    auto res = nextNewWTUniStream_;
    nextNewWTUniStream_ += 4;
    return res;
  }

  folly::Expected<WebTransport::FCState, WebTransport::ErrorCode>
  sendWebTransportStreamData(HTTPCodec::StreamID id,
                             std::unique_ptr<folly::IOBuf> data,
                             bool eof,
                             WebTransport::ByteEventCallback* wcb) override {
    if (h3Tp_) {
      return h3Tp_->sendWebTransportStreamData(id, std::move(data), eof, wcb);
    }
    return WebTransport::FCState::UNBLOCKED;
  }

  folly::Expected<folly::Unit, WebTransport::ErrorCode> sendWTMaxData(
      uint64_t maxData) override {
    WTMaxDataCapsule capsule{maxData};
    folly::IOBufQueue buf{folly::IOBufQueue::cacheChainLength()};
    if (auto res = writeWTMaxData(buf, capsule); !res.has_value()) {
      return folly::makeUnexpected(WebTransport::ErrorCode::SEND_ERROR);
    }
    if (txn_) {
      txn_->sendBody(buf.move());
    }
    return folly::unit;
  }

  folly::Expected<folly::Unit, WebTransport::ErrorCode> sendWTMaxStreams(
      uint64_t maxStreams, bool isBidi) override {
    WTMaxStreamsCapsule capsule{maxStreams};
    folly::IOBufQueue buf{folly::IOBufQueue::cacheChainLength()};
    if (auto res = writeWTMaxStreams(buf, capsule, isBidi); !res.has_value()) {
      return folly::makeUnexpected(WebTransport::ErrorCode::SEND_ERROR);
    }
    if (txn_) {
      txn_->sendBody(buf.move());
    }
    return folly::unit;
  }

  folly::Expected<folly::Unit, WebTransport::ErrorCode> resetWebTransportEgress(
      HTTPCodec::StreamID id, uint32_t errorCode) override {
    return h3Tp_ ? h3Tp_->resetWebTransportEgress(id, errorCode) : folly::unit;
  }

  folly::Expected<folly::Unit, WebTransport::ErrorCode>
  setWebTransportStreamPriority(HTTPCodec::StreamID id,
                                HTTPPriority pri) override {
    return h3Tp_ ? h3Tp_->setWebTransportStreamPriority(id, pri) : folly::unit;
  }

  folly::Expected<std::pair<std::unique_ptr<folly::IOBuf>, bool>,
                  WebTransport::ErrorCode>
  readWebTransportData(HTTPCodec::StreamID id, size_t max) override {
    return h3Tp_
               ? h3Tp_->readWebTransportData(id, max)
               : folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
  }

  folly::Expected<folly::Unit, WebTransport::ErrorCode>
  initiateReadOnBidiStream(HTTPCodec::StreamID id,
                           quic::StreamReadCallback* readCallback) override {
    return h3Tp_ ? h3Tp_->initiateReadOnBidiStream(id, readCallback)
                 : folly::unit;
  }

  folly::Expected<folly::Unit, WebTransport::ErrorCode>
  pauseWebTransportIngress(HTTPCodec::StreamID id) override {
    return h3Tp_ ? h3Tp_->pauseWebTransportIngress(id) : folly::unit;
  }

  folly::Expected<folly::Unit, WebTransport::ErrorCode>
  resumeWebTransportIngress(HTTPCodec::StreamID id) override {
    return h3Tp_ ? h3Tp_->resumeWebTransportIngress(id) : folly::unit;
  }

  folly::Expected<folly::Unit, WebTransport::ErrorCode>
  stopReadingWebTransportIngress(HTTPCodec::StreamID id,
                                 folly::Optional<uint32_t> errorCode) override {
    return h3Tp_ ? h3Tp_->stopReadingWebTransportIngress(id, errorCode)
                 : folly::unit;
  }

  folly::SemiFuture<folly::Unit> awaitUniStreamCredit() override {
    return folly::makeFuture(folly::unit);
  }

  folly::SemiFuture<folly::Unit> awaitBidiStreamCredit() override {
    return folly::makeSemiFuture(folly::unit);
  }

  bool canCreateUniStream() override {
    if ((nextNewWTUniStream_ >> 2) >= maxWTUniStreams_) {
      return false;
    }
    if (h3Tp_ && !h3Tp_->canCreateUniStream()) {
      return false;
    }
    return true;
  }

  bool canCreateBidiStream() override {
    if ((nextNewWTBidiStream_ >> 2) >= maxWTBidiStreams_) {
      return false;
    }
    if (h3Tp_ && !h3Tp_->canCreateBidiStream()) {
      return false;
    }
    return true;
  }

  folly::Expected<folly::Unit, WebTransport::ErrorCode>
  notifyPendingWriteOnStream(HTTPCodec::StreamID id,
                             quic::StreamWriteCallback* wcb) override {
    return h3Tp_ ? h3Tp_->notifyPendingWriteOnStream(id, wcb) : folly::unit;
  }

  bool usesEncodedApplicationErrorCodes() override {
    return h3Tp_ ? h3Tp_->usesEncodedApplicationErrorCodes()
                 : true; // For H2, we use encoded application error codes
  }

  uint64_t getWTInitialSendWindow() const override {
    return h3Tp_ ? h3Tp_->getWTInitialSendWindow() : quic::kMaxVarInt;
  }

  const folly::SocketAddress& getLocalAddress() const override {
    return txn_->getLocalAddress();
  }

  const folly::SocketAddress& getPeerAddress() const override {
    return txn_->getPeerAddress();
  }

  void onConnectionError(CapsuleCodec::ErrorCode error) noexcept override {
    XLOG(DBG1) << __func__ << " error=" << static_cast<int>(error);
  }
  void onPaddingCapsule(PaddingCapsule /*capsule*/) noexcept override {
    XLOG(DBG1) << __func__;
  }
  void onWTResetStreamCapsule(
      WTResetStreamCapsule /*capsule*/) noexcept override {
    XLOG(DBG1) << __func__;
  }
  void onWTStopSendingCapsule(
      WTStopSendingCapsule /*capsule*/) noexcept override {
    XLOG(DBG1) << __func__;
  }
  void onWTStreamCapsule(WTStreamCapsule /*capsule*/) noexcept override {
    XLOG(DBG1) << __func__;
  }
  void onWTMaxDataCapsule(WTMaxDataCapsule capsule) noexcept override {
    XLOG(DBG1) << __func__;
    if (wtImpl_) {
      wtImpl_->onMaxData(capsule.maximumData);
    }
  }
  void onWTMaxStreamDataCapsule(
      WTMaxStreamDataCapsule /*capsule*/) noexcept override {
    XLOG(DBG1) << __func__;
  }
  void onWTMaxStreamsBidiCapsule(
      WTMaxStreamsCapsule capsule) noexcept override {
    XLOG(DBG1) << __func__;
    maxWTBidiStreams_ = capsule.maximumStreams;
    if (wtImpl_) {
      wtImpl_->onMaxStreams(capsule.maximumStreams, true);
    }
  }
  void onWTMaxStreamsUniCapsule(WTMaxStreamsCapsule capsule) noexcept override {
    XLOG(DBG1) << __func__;
    maxWTUniStreams_ = capsule.maximumStreams;
    if (wtImpl_) {
      wtImpl_->onMaxStreams(capsule.maximumStreams, false);
    }
  }
  void onWTDataBlockedCapsule(
      WTDataBlockedCapsule /*capsule*/) noexcept override {
    XLOG(DBG1) << __func__;
  }
  void onWTStreamDataBlockedCapsule(
      WTStreamDataBlockedCapsule /*capsule*/) noexcept override {
    XLOG(DBG1) << __func__;
  }
  void onWTStreamsBlockedBidiCapsule(
      WTStreamsBlockedCapsule /*capsule*/) noexcept override {
    XLOG(DBG1) << __func__;
  }
  void onWTStreamsBlockedUniCapsule(
      WTStreamsBlockedCapsule /*capsule*/) noexcept override {
    XLOG(DBG1) << __func__;
  }
  void onDatagramCapsule(DatagramCapsule /*capsule*/) noexcept override {
    XLOG(DBG1) << __func__;
  }
  void onCloseWTSessionCapsule(
      CloseWebTransportSessionCapsule capsule) noexcept override {
    XLOG(DBG1) << __func__ << " errorCode=" << capsule.applicationErrorCode
               << " message=" << capsule.applicationErrorMessage;

    if (wtImpl_) {
      wtImpl_->terminateSession(capsule.applicationErrorCode);
      sessionClosed_ = true;
      closeErrorCode_ = capsule.applicationErrorCode;
      closeErrorMessage_ = capsule.applicationErrorMessage;
    }

    if (handler_) {
      handler_->onSessionEnd(capsule.applicationErrorCode);
    }

    txn_->sendEOM();
  }
  void onDrainWTSessionCapsule(
      DrainWebTransportSessionCapsule /*capsule*/) noexcept override {
    XLOG(DBG1) << __func__;
    if (handler_) {
      handler_->onSessionDrain();
    }
  }

 private:
  HTTPTransaction* txn_;
  std::unique_ptr<CapsuleCodec> codec_;
  uint64_t nextNewWTBidiStream_{0};
  uint64_t nextNewWTUniStream_{2};
  uint64_t maxWTBidiStreams_{0};
  uint64_t maxWTUniStreams_{0};
  folly::F14FastMap<HTTPCodec::StreamID, WebTransportImpl::StreamReadHandle*>
      readCallbacks_;
  struct WriteCallback {
    HTTPCodec::StreamID id;
    uint64_t bodyOffset;
    quic::StreamWriteCallback* wcb;
  };
  std::list<WriteCallback> writeCallbacks_;
  WebTransportHandler* handler_{nullptr};
  WebTransportImpl* wtImpl_{nullptr};
  [[maybe_unused]] TransportProvider* h3Tp_{nullptr};

  bool sessionClosed_{false};
  uint32_t closeErrorCode_{0};
  std::string closeErrorMessage_;
};

} // namespace proxygen
