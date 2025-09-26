/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/webtransport/WebTransportImpl.h>

namespace {

constexpr uint64_t kMaxWTIngressBuf = 65535;

using StreamData = proxygen::WebTransport::StreamData;
using ReadPromiseT = folly::Promise<StreamData>;
ReadPromiseT emptyReadPromise() {
  return ReadPromiseT::makeEmpty();
}

using WritePromiseT = folly::Promise<uint64_t>;
WritePromiseT emptyWritePromise() {
  return WritePromiseT::makeEmpty();
}

} // namespace

namespace proxygen {

void WebTransportImpl::destroy() {
  terminateSessionStreams(WebTransport::kInternalError, "");
}

void WebTransportImpl::terminateSessionStreams(uint32_t errorCode,
                                               const std::string& reason) {
  // These loops are dicey for possible erasure from callbacks
  for (auto ingressStreamIt = wtIngressStreams_.begin();
       ingressStreamIt != wtIngressStreams_.end();) {
    auto id = ingressStreamIt->first;
    auto& stream = ingressStreamIt->second;
    ingressStreamIt++;
    // Deliver an error to the application if needed
    if (stream.open()) {
      VLOG(4) << "aborting WT ingress id=" << id << " with error=" << errorCode;
      stream.deliverReadError(WebTransport::Exception(errorCode, reason));
      stopReadingWebTransportIngress(id, errorCode);
    } else {
      VLOG(4) << "WT ingress already complete for id=" << id;
    }
  }
  wtIngressStreams_.clear();
  for (auto egressStreamIt = wtEgressStreams_.begin();
       egressStreamIt != wtEgressStreams_.end();) {
    auto id = egressStreamIt->first;
    auto& stream = egressStreamIt->second;
    egressStreamIt++;
    // Deliver an error to the application
    stream.onStopSending(errorCode);
    // The handler may have run and reset this stream, removing it from
    // wtEgressStreams_, otherwise we have to reset it.
    if (wtEgressStreams_.find(id) != wtEgressStreams_.end()) {
      resetWebTransportEgress(id, errorCode);
    }
  }
  wtEgressStreams_.clear();
}

void WebTransportImpl::onMaxData(uint64_t maxData) noexcept {
  if (sendFlowController_.grant(maxData) &&
      sendFlowController_.getAvailable() > 0) {
    for (auto it = wtEgressStreams_.begin();
         it != wtEgressStreams_.end() &&
         sendFlowController_.getAvailable() > 0;) {
      // Increment here in case we delete the stream in
      // flushBufferedWrites -> sendWebTransportStreamData
      auto currIt = it++;
      currIt->second.flushBufferedWrites();
    }
  } else {
    VLOG(4) << __func__ << " failed to grant maxData=" << maxData;
  }
}

folly::Expected<WebTransport::StreamWriteHandle*, WebTransport::ErrorCode>
WebTransportImpl::newWebTransportUniStream() {
  if (sessionCloseError_.has_value()) {
    return folly::makeUnexpected(WebTransport::ErrorCode::SESSION_TERMINATED);
  }
  auto id = tp_.newWebTransportUniStream();
  if (!id) {
    return folly::makeUnexpected(
        WebTransport::ErrorCode::STREAM_CREATION_ERROR);
  }
  auto res = wtEgressStreams_.emplace(std::piecewise_construct,
                                      std::forward_as_tuple(*id),
                                      std::forward_as_tuple(*this, *id));
  sp_.refreshTimeout();
  return &res.first->second;
}

folly::Expected<WebTransport::BidiStreamHandle, WebTransport::ErrorCode>
WebTransportImpl::newWebTransportBidiStream() {
  if (sessionCloseError_.has_value()) {
    return folly::makeUnexpected(WebTransport::ErrorCode::SESSION_TERMINATED);
  }
  auto id = tp_.newWebTransportBidiStream();
  if (!id) {
    return folly::makeUnexpected(
        WebTransport::ErrorCode::STREAM_CREATION_ERROR);
  }
  auto ingressRes =
      wtIngressStreams_.emplace(std::piecewise_construct,
                                std::forward_as_tuple(*id),
                                std::forward_as_tuple(*this, *id));
  auto readHandle = &ingressRes.first->second;
  tp_.initiateReadOnBidiStream(*id, readHandle);
  sp_.refreshTimeout();
  auto egressRes = wtEgressStreams_.emplace(std::piecewise_construct,
                                            std::forward_as_tuple(*id),
                                            std::forward_as_tuple(*this, *id));
  return WebTransport::BidiStreamHandle(
      {.readHandle = readHandle, .writeHandle = &egressRes.first->second});
}

WebTransportImpl::BidiStreamHandle WebTransportImpl::onWebTransportBidiStream(
    HTTPCodec::StreamID id) {
  auto ingRes = wtIngressStreams_.emplace(std::piecewise_construct,
                                          std::forward_as_tuple(id),
                                          std::forward_as_tuple(*this, id));

  auto egRes = wtEgressStreams_.emplace(std::piecewise_construct,
                                        std::forward_as_tuple(id),
                                        std::forward_as_tuple(*this, id));
  return WebTransportImpl::BidiStreamHandle(
      {.readHandle = &ingRes.first->second,
       .writeHandle = &egRes.first->second});
}

WebTransportImpl::StreamReadHandle* WebTransportImpl::onWebTransportUniStream(
    HTTPCodec::StreamID id) {
  auto ingRes = wtIngressStreams_.emplace(std::piecewise_construct,
                                          std::forward_as_tuple(id),
                                          std::forward_as_tuple(*this, id));

  return &ingRes.first->second;
}

folly::Expected<WebTransportImpl::WebTransport::FCState,
                WebTransport::ErrorCode>
WebTransportImpl::sendWebTransportStreamData(
    HTTPCodec::StreamID id,
    std::unique_ptr<folly::IOBuf> data,
    bool eof,
    ByteEventCallback* deliveryCallback) {
  auto dataLen = data ? data->computeChainDataLength() : 0;
  // WebTransportImpl::sendWebTransportStreamData will only be called when
  // dataLen <= available window
  bool blocked = !sendFlowController_.reserve(dataLen);
  auto res = tp_.sendWebTransportStreamData(
      id, std::move(data), eof, deliveryCallback);
  if (blocked) {
    res = WebTransport::FCState::BLOCKED;
  }
  if (eof || res.hasError()) {
    wtEgressStreams_.erase(id);
  }
  sp_.refreshTimeout();
  return res;
}

folly::Expected<folly::Unit, WebTransport::ErrorCode>
WebTransportImpl::resetWebTransportEgress(HTTPCodec::StreamID id,
                                          uint32_t errorCode) {
  auto res = tp_.resetWebTransportEgress(id, errorCode);
  wtEgressStreams_.erase(id);
  sp_.refreshTimeout();
  return res;
}

folly::Expected<folly::Unit, WebTransport::ErrorCode>
WebTransportImpl::stopReadingWebTransportIngress(
    HTTPCodec::StreamID id, folly::Optional<uint32_t> errorCode) {
  auto res = tp_.stopReadingWebTransportIngress(id, errorCode);
  wtIngressStreams_.erase(id);
  sp_.refreshTimeout();
  return res;
}

// -- StreamWriteHandle & StreamReadHandle functions below --

WebTransportImpl::StreamWriteHandle::StreamWriteHandle(WebTransportImpl& tp,
                                                       HTTPCodec::StreamID id)
    : WebTransport::StreamWriteHandle(id),
      impl_(tp),
      writePromise_(emptyWritePromise()) {
}

folly::Expected<WebTransport::FCState, WebTransport::ErrorCode>
WebTransportImpl::StreamWriteHandle::writeStreamData(
    std::unique_ptr<folly::IOBuf> data,
    bool fin,
    ByteEventCallback* deliveryCallback) {
  if (stopSendingErrorCode_) {
    return folly::makeUnexpected(WebTransport::ErrorCode::STOP_SENDING);
  }
  if (!data && !fin) {
    return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
  }

  impl_.sp_.refreshTimeout();

  if (!bufferedWrites_.empty() && bufferedWrites_.back().fin) {
    return folly::makeUnexpected(WebTransport::ErrorCode::GENERIC_ERROR);
  }
  if (!bufferedWrites_.empty() &&
      bufferedWrites_.back().deliveryCallback == nullptr) {
    if (data) {
      bufferedWrites_.back().buf.append(std::move(data));
    }
    bufferedWrites_.back().deliveryCallback = deliveryCallback;
    bufferedWrites_.back().fin = fin;
  } else {
    bufferedWrites_.emplace_back(std::move(data), deliveryCallback, fin);
  }

  return flushBufferedWrites();
}

folly::Expected<folly::SemiFuture<uint64_t>, WebTransport::ErrorCode>
WebTransportImpl::StreamWriteHandle::awaitWritable() {
  CHECK(!writePromise_.valid()) << "awaitWritable already called";
  auto contract = folly::makePromiseContract<uint64_t>();
  writePromise_ = std::move(contract.promise);
  writePromise_.setInterruptHandler([this](const folly::exception_wrapper& ex) {
    VLOG(4) << "Exception from interrupt handler ex=" << ex.what();
    // if awaitWritable is cancelled, just reset it
    CHECK(ex.with_exception([this](const folly::FutureCancellation& ex) {
      VLOG(5) << "Setting exception ex=" << ex.what();
      writePromise_.setException(ex);
      writePromise_ = emptyWritePromise();
    })) << "Unexpected exception type";
  });
  impl_.tp_.notifyPendingWriteOnStream(id_, this);
  return std::move(contract.future);
}

void WebTransportImpl::onWebTransportStopSending(HTTPCodec::StreamID id,
                                                 uint32_t errorCode) {
  // The caller already decodes errorCode, if necessary
  auto it = wtEgressStreams_.find(id);
  if (it != wtEgressStreams_.end()) {
    it->second.onStopSending(errorCode);
  }
}

void WebTransportImpl::StreamWriteHandle::onStopSending(uint32_t errorCode) {
  // The caller already decodes errorCode, if necessary
  if (writePromise_.valid()) {
    writePromise_.setException(WebTransport::Exception(errorCode));
    writePromise_ = emptyWritePromise();
  } else if (!stopSendingErrorCode_) {
    stopSendingErrorCode_ = errorCode;
  }

  cs_.requestCancellation();
}

void WebTransportImpl::StreamWriteHandle::onStreamWriteReady(
    quic::StreamId, uint64_t maxToSend) noexcept {
  streamWriteReady_ = true;
  flushBufferedWrites();
  maybeResolveWritePromise(maxToSend);
}

folly::Expected<WebTransport::FCState, WebTransport::ErrorCode>
WebTransportImpl::StreamWriteHandle::flushBufferedWrites() {
  auto availableSpace = impl_.sendFlowController_.getAvailable();

  while (availableSpace > 0 && !bufferedWrites_.empty()) {
    auto& frontEntry = bufferedWrites_.front();
    auto bufToSend = frontEntry.buf.splitAtMost(availableSpace);

    availableSpace -= bufToSend->computeChainDataLength();
    ByteEventCallback* sendDeliveryCallback = nullptr;
    bool sendFin = false;

    if (frontEntry.buf.empty()) {
      sendDeliveryCallback = frontEntry.deliveryCallback;
      sendFin = frontEntry.fin;
      bufferedWrites_.pop_front();
    }

    auto res = impl_.sendWebTransportStreamData(
        id_, std::move(bufToSend), sendFin, sendDeliveryCallback);

    if (res.hasError()) {
      return folly::makeUnexpected(res.error());
    }

    if (sendFin || *res == WebTransport::FCState::BLOCKED) {
      return *res;
    }
  }

  return bufferedWrites_.empty() ? WebTransport::FCState::UNBLOCKED
                                 : WebTransport::FCState::BLOCKED;
}

/**
 * A write can be blocked for two separate reasons:
 * 1. The QUIC transport has no room
 * 2. There's no room as per the WT_MAX_DATA sent to us by the peer
 *
 * This sets writePromise_ only if there's room in the QUIC transport
 * and there's room as per the WT_MAX_DATA sent to us by the peer, as well as no
 * pending writes.
 */
void WebTransportImpl::StreamWriteHandle::maybeResolveWritePromise(
    uint64_t maxToSend) {
  if (!writePromise_.valid()) {
    return;
  }

  if (impl_.sendFlowController_.getAvailable() > 0 && streamWriteReady_ &&
      bufferedWrites_.empty()) {
    writePromise_.setValue(maxToSend);
    writePromise_ = emptyWritePromise();
    streamWriteReady_ = false;
  }
}

WebTransportImpl::StreamReadHandle::StreamReadHandle(WebTransportImpl& impl,
                                                     HTTPCodec::StreamID id)
    : WebTransport::StreamReadHandle(id),
      impl_(impl),
      readPromise_(emptyReadPromise()) {
}

folly::SemiFuture<StreamData>
WebTransportImpl::StreamReadHandle::readStreamData() {
  VLOG(4) << __func__;
  CHECK(!readPromise_.valid()) << "One read at a time";
  if (error_) {
    auto ex = std::move(error_);
    impl_.wtIngressStreams_.erase(getID());
    return folly::makeSemiFuture<StreamData>(std::move(ex));
  } else if (buf_.empty() && !eof_) {
    VLOG(4) << __func__ << " waiting for data";
    auto contract = folly::makePromiseContract<StreamData>();
    readPromise_ = std::move(contract.promise);
    readPromise_.setInterruptHandler(
        [this](const folly::exception_wrapper& ex) {
          VLOG(4) << "Exception from interrupt handler ex=" << ex.what();
          CHECK(ex.with_exception([this](const folly::FutureCancellation& ex) {
            // TODO: allow app to configure the reset code on cancellation?
            impl_.tp_.stopReadingWebTransportIngress(
                id_, WebTransport::kInternalError);
            deliverReadError(ex);
          })) << "Unexpected exception type";
        });
    return std::move(contract.future);
  } else {
    VLOG(4) << __func__ << " returning data len=" << buf_.chainLength();
    auto bufLen = buf_.chainLength();
    StreamData streamData({.data = buf_.move(), .fin = eof_});
    impl_.bytesRead_ += bufLen;
    impl_.maybeGrantFlowControl();

    if (eof_) {
      // unregister the read callback, but don't send STOP_SENDING
      impl_.stopReadingWebTransportIngress(id_, folly::none);
    } else if (bufLen >= kMaxWTIngressBuf) {
      VLOG(4) << __func__ << " resuming reads";
      impl_.tp_.resumeWebTransportIngress(getID());
    }
    return folly::makeFuture(std::move(streamData));
  }
}

void WebTransportImpl::StreamReadHandle::readAvailable(
    quic::StreamId id) noexcept {
  impl_.sp_.refreshTimeout();
  auto readRes = impl_.tp_.readWebTransportData(id, 65535);
  if (readRes.hasError()) {
    LOG(ERROR) << "Got synchronous read error=" << uint32_t(readRes.error());
    readError(id,
              quic::QuicError(quic::LocalErrorCode::INTERNAL_ERROR,
                              "sync read error"));
    impl_.wtIngressStreams_.erase(getID());
    return;
  }
  quic::BufPtr data = std::move(readRes.value().first);
  bool eof = readRes.value().second;
  // deliver data, eof
  auto state = dataAvailable(std::move(data), eof);
  if (state == WebTransport::FCState::SESSION_CLOSED) {
    impl_.terminateSession(WebTransport::kInternalError);
    return;
  }
  if (state == WebTransport::FCState::BLOCKED && !eof) {
    VLOG(4) << __func__ << " pausing reads";
    impl_.tp_.pauseWebTransportIngress(id);
  }
}

WebTransport::FCState WebTransportImpl::StreamReadHandle::dataAvailable(
    std::unique_ptr<folly::IOBuf> data, bool eof) {
  VLOG(4)
      << "dataAvailable buflen=" << (data ? data->computeChainDataLength() : 0)
      << " eof=" << uint64_t(eof);

  auto len = data ? data->computeChainDataLength() : 0;
  if (!impl_.recvFlowController_.reserve(len)) {
    return WebTransport::FCState::SESSION_CLOSED;
  }

  if (readPromise_.valid()) {
    impl_.bytesRead_ += len;
    impl_.maybeGrantFlowControl();
    readPromise_.setValue(StreamData({.data = std::move(data), .fin = eof}));
    readPromise_ = emptyReadPromise();
    if (eof) {
      // unregister the read callback, but don't send STOP_SENDING
      impl_.stopReadingWebTransportIngress(getID(), folly::none);
      return WebTransport::FCState::UNBLOCKED;
    }
  } else {
    buf_.append(std::move(data)); // ok if nullptr
    eof_ = eof;
  }
  VLOG(4) << "dataAvailable buflen=" << buf_.chainLength();
  return (eof || buf_.chainLength() < kMaxWTIngressBuf)
             ? WebTransport::FCState::UNBLOCKED
             : WebTransport::FCState::BLOCKED;
}

void WebTransportImpl::StreamReadHandle::readError(
    quic::StreamId id, quic::QuicError error) noexcept {
  // Do I need to setReadCallback(id, nullptr);
  impl_.sp_.refreshTimeout();
  auto quicAppErrorCode = error.code.asApplicationErrorCode();
  if (quicAppErrorCode) {
    folly::Expected<uint32_t, WebTransport::ErrorCode> appErrorCode{
        *quicAppErrorCode};
    if (impl_.tp_.usesEncodedApplicationErrorCodes()) {
      appErrorCode =
          proxygen::WebTransport::toApplicationErrorCode(*quicAppErrorCode);
      if (!appErrorCode) {
        deliverReadError(WebTransport::Exception(
            *quicAppErrorCode, "received invalid reset_stream"));
        return;
      }
    }
    deliverReadError(
        WebTransport::Exception(*appErrorCode, "received reset_stream"));
    return;
  } else {
    VLOG(4) << error;
  }
  // any other error
  deliverReadError(WebTransport::Exception(
      proxygen::WebTransport::kInternalError, "quic error"));
}

void WebTransportImpl::StreamReadHandle::deliverReadError(
    const folly::exception_wrapper& ex) {
  cs_.requestCancellation();
  if (readPromise_.valid()) {
    readPromise_.setException(ex);
    readPromise_ = emptyReadPromise();
    impl_.wtIngressStreams_.erase(getID());
  } else {
    error_ = ex;
  }
}

void WebTransportImpl::maybeGrantFlowControl() {
  if (shouldGrantFlowControl()) {
    auto newMaxData = bytesRead_ + kDefaultWTReceiveWindow;
    recvFlowController_.grant(newMaxData);
    tp_.sendWTMaxData(newMaxData);
  }
}

bool WebTransportImpl::shouldGrantFlowControl() const {
  auto bufferedBytes = recvFlowController_.getCurrentOffset() - bytesRead_;
  return bufferedBytes < kDefaultWTReceiveWindow / 2;
}

} // namespace proxygen
