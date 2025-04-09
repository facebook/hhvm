/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/webtransport/QuicWebTransport.h>
#include <quic/priority/HTTPPriorityQueue.h>

using FCState = proxygen::WebTransport::FCState;

namespace proxygen {

void QuicWebTransport::onFlowControlUpdate(quic::StreamId /*id*/) noexcept {
}

void QuicWebTransport::onNewBidirectionalStream(quic::StreamId id) noexcept {
  XCHECK(quicSocket_);
  if (!handler_) {
    resetWebTransportEgress(id, WebTransport::kInternalError);
    stopReadingWebTransportIngress(id, WebTransport::kInternalError);
    return;
  }
  auto handle = WebTransportImpl::onWebTransportBidiStream(id);
  handler_->onNewBidiStream(
      WebTransport::BidiStreamHandle({handle.readHandle, handle.writeHandle}));
  quicSocket_->setReadCallback(id, handle.readHandle);
}

void QuicWebTransport::onNewUnidirectionalStream(quic::StreamId id) noexcept {
  XCHECK(quicSocket_);
  if (!handler_) {
    LOG(ERROR) << "Handler not set";
    stopReadingWebTransportIngress(id, WebTransport::kInternalError);
    return;
  }
  auto readHandle = WebTransportImpl::onWebTransportUniStream(id);
  handler_->onNewUniStream(readHandle);
  quicSocket_->setReadCallback(id, readHandle);
}

void QuicWebTransport::onStopSending(
    quic::StreamId id, quic::ApplicationErrorCode errorCode) noexcept {
  onWebTransportStopSending(id, static_cast<uint32_t>(errorCode));
}

void QuicWebTransport::onConnectionEnd() noexcept {
  onConnectionEndImpl(folly::none);
}

void QuicWebTransport::onConnectionError(quic::QuicError error) noexcept {
  onConnectionEndImpl(error);
}
void QuicWebTransport::onConnectionEnd(quic::QuicError error) noexcept {
  onConnectionEndImpl(error);
}

void QuicWebTransport::onConnectionEndImpl(
    folly::Optional<quic::QuicError> error) {
  destroy();
  folly::Optional<uint32_t> wtError;
  if (error) {
    if (error->code.type() == quic::QuicErrorCode::Type::ApplicationErrorCode) {
      wtError = static_cast<uint32_t>(*error->code.asApplicationErrorCode());
    } else {
      XLOG(ERR) << "QUIC Connection Error: " << *error;
      wtError = std::numeric_limits<uint32_t>::max();
    }
  }
  quicSocket_.reset();
  handler_->onSessionEnd(wtError);
}

folly::Expected<HTTPCodec::StreamID, WebTransport::ErrorCode>
QuicWebTransport::newWebTransportBidiStream() {
  XCHECK(quicSocket_);
  auto id = quicSocket_->createBidirectionalStream();
  if (id.hasError()) {
    return folly::makeUnexpected(ErrorCode::GENERIC_ERROR);
  }
  return id.value();
}

folly::Expected<HTTPCodec::StreamID, WebTransport::ErrorCode>
QuicWebTransport::newWebTransportUniStream() {
  XCHECK(quicSocket_);
  auto id = quicSocket_->createUnidirectionalStream();
  if (id.hasError()) {
    return folly::makeUnexpected(ErrorCode::GENERIC_ERROR);
  }
  return id.value();
}

folly::Expected<WebTransport::FCState, WebTransport::ErrorCode>
QuicWebTransport::sendWebTransportStreamData(
    HTTPCodec::StreamID id,
    std::unique_ptr<folly::IOBuf> data,
    bool eof,
    ByteEventCallback* deliveryCallback) {
  XCHECK(quicSocket_);
  auto res =
      quicSocket_->writeChain(id, std::move(data), eof, deliveryCallback);
  if (!res) {
    return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
  }
  auto flowControl = quicSocket_->getStreamFlowControl(id);
  if (!flowControl) {
    LOG(ERROR) << "Failed to get flow control";
    return folly::makeUnexpected(WebTransport::ErrorCode::SEND_ERROR);
  }
  if (!eof && flowControl->sendWindowAvailable == 0) {
    VLOG(4) << "fc window closed";
    return FCState::BLOCKED;
  } else {
    return FCState::UNBLOCKED;
  }
}

folly::Expected<folly::Unit, WebTransport::ErrorCode>
QuicWebTransport::notifyPendingWriteOnStream(HTTPCodec::StreamID id,
                                             quic::StreamWriteCallback* wcb) {
  XCHECK(quicSocket_);
  quicSocket_->notifyPendingWriteOnStream(id, wcb);
  return folly::unit;
}

folly::Expected<folly::Unit, WebTransport::ErrorCode>
QuicWebTransport::resetWebTransportEgress(HTTPCodec::StreamID id,
                                          uint32_t errorCode) {
  XCHECK(quicSocket_);
  auto res = quicSocket_->resetStream(id, errorCode);
  if (!res) {
    return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
  }
  return folly::unit;
}

folly::Expected<folly::Unit, WebTransport::ErrorCode>
QuicWebTransport::setWebTransportStreamPriority(HTTPCodec::StreamID id,
                                                HTTPPriority pri) {

  XCHECK(quicSocket_);
  auto res = quicSocket_->setStreamPriority(
      id,
      quic::HTTPPriorityQueue::Priority(
          pri.urgency, pri.incremental, pri.orderId));
  if (res.hasError()) {
    return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
  }

  return folly::unit;
}

folly::Expected<folly::Unit, WebTransport::ErrorCode>
QuicWebTransport::pauseWebTransportIngress(HTTPCodec::StreamID id) {
  XCHECK(quicSocket_);
  auto res = quicSocket_->pauseRead(id);
  if (res.hasError()) {
    return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
  }
  return folly::unit;
}

folly::Expected<folly::Unit, WebTransport::ErrorCode>
QuicWebTransport::resumeWebTransportIngress(HTTPCodec::StreamID id) {
  XCHECK(quicSocket_);
  auto res = quicSocket_->resumeRead(id);
  if (res.hasError()) {
    return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
  }
  return folly::unit;
}

folly::Expected<folly::Unit, WebTransport::ErrorCode>
QuicWebTransport::stopReadingWebTransportIngress(
    HTTPCodec::StreamID id, folly::Optional<uint32_t> errorCode) {
  XCHECK(quicSocket_);
  quic::Optional<quic::ApplicationErrorCode> quicErrorCode;
  if (errorCode) {
    quicErrorCode = quic::ApplicationErrorCode(*errorCode);
  }
  auto res = quicSocket_->setReadCallback(id, nullptr, quicErrorCode);
  if (res.hasError()) {
    return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
  }
  return folly::unit;
}

folly::Expected<folly::Unit, WebTransport::ErrorCode>
QuicWebTransport::sendDatagram(std::unique_ptr<folly::IOBuf> datagram) {
  XCHECK(quicSocket_);
  auto writeRes = quicSocket_->writeDatagram(std::move(datagram));
  if (writeRes.hasError()) {
    LOG(ERROR) << "Failed to send datagram";
    return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
  }
  return folly::unit;
}

folly::Expected<folly::Unit, WebTransport::ErrorCode>
QuicWebTransport::closeSession(folly::Optional<uint32_t> error) {
  if (quicSocket_) {
    if (error) {
      quicSocket_->close(quic::QuicError(quic::ApplicationErrorCode(*error)));
    } else {
      quicSocket_->close(quic::QuicError(quic::ApplicationErrorCode(0)));
    }
    quicSocket_.reset();
  } // else we came from connectionEnd/Error and quicSocket_ is reset
  return folly::unit;
}

void QuicWebTransport::onUnidirectionalStreamsAvailable(
    uint64_t numStreamsAvailable) noexcept {
  if (numStreamsAvailable > 0 && waitingForUniStreams_) {
    waitingForUniStreams_->setValue(folly::unit);
    waitingForUniStreams_.reset();
  }
}

folly::SemiFuture<folly::Unit> QuicWebTransport::awaitUniStreamCredit() {
  XCHECK(quicSocket_);
  auto numOpenable = quicSocket_->getNumOpenableUnidirectionalStreams();
  if (numOpenable > 0) {
    return folly::makeFuture(folly::unit);
  }
  CHECK(!waitingForUniStreams_);
  auto [promise, future] = folly::makePromiseContract<folly::Unit>();
  waitingForUniStreams_ = std::move(promise);
  return std::move(future);
}

void QuicWebTransport::onBidirectionalStreamsAvailable(
    uint64_t numStreamsAvailable) noexcept {
  if (numStreamsAvailable > 0 && waitingForBidiStreams_) {
    waitingForBidiStreams_->setValue(folly::unit);
    waitingForBidiStreams_.reset();
  }
}

folly::SemiFuture<folly::Unit> QuicWebTransport::awaitBidiStreamCredit() {
  XCHECK(quicSocket_);
  auto numOpenable = quicSocket_->getNumOpenableBidirectionalStreams();
  if (numOpenable > 0) {
    return folly::makeFuture(folly::unit);
  }
  CHECK(!waitingForBidiStreams_);
  auto [promise, future] = folly::makePromiseContract<folly::Unit>();
  waitingForBidiStreams_ = std::move(promise);
  return std::move(future);
}

void QuicWebTransport::onDatagramsAvailable() noexcept {
  XCHECK(quicSocket_);
  auto result = quicSocket_->readDatagramBufs();
  if (result.hasError()) {
    LOG(ERROR) << "Got error while reading datagrams: error="
               << toString(result.error());
    closeSession(0);
    return;
  }
  VLOG(4) << "Received " << result.value().size() << " datagrams";
  for (auto& datagram : result.value()) {
    handler_->onDatagram(std::move(datagram));
  }
}

} // namespace proxygen
