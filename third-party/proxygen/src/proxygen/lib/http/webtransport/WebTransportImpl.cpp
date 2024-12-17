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
}

namespace proxygen {

void WebTransportImpl::destroy() {
  // These loops are dicey for possible erasure from callbacks
  for (auto ingressStreamIt = wtIngressStreams_.begin();
       ingressStreamIt != wtIngressStreams_.end();) {
    auto id = ingressStreamIt->first;
    auto& stream = ingressStreamIt->second;
    ingressStreamIt++;
    // Deliver an error to the application if needed
    if (stream.open()) {
      VLOG(4) << "aborting WT ingress id=" << id;
      stream.deliverReadError(WebTransport::Exception(
          WebTransport::kInternalError, "shutting down"));
      stopReadingWebTransportIngress(id, WebTransport::kInternalError);
      // TODO: does the spec say how to handle this at the transport?  Eg: the
      // peer must RESET any open write streams.
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
    stream.onStopSending(WebTransport::kInternalError);
    // The handler may have run and reset this stream, removing it from
    // wtEgressStreams_, otherwise we have to reset it.
    if (wtEgressStreams_.find(id) != wtEgressStreams_.end()) {
      resetWebTransportEgress(id,
                              /*TODO: errorCode=*/WebTransport::kInternalError);
    }
  }
  wtEgressStreams_.clear();
}

folly::Expected<WebTransport::StreamWriteHandle*, WebTransport::ErrorCode>
WebTransportImpl::newWebTransportUniStream() {
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
  return WebTransport::BidiStreamHandle({readHandle, &egressRes.first->second});
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
      {&ingRes.first->second, &egRes.first->second});
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
WebTransportImpl::sendWebTransportStreamData(HTTPCodec::StreamID id,
                                             std::unique_ptr<folly::IOBuf> data,
                                             bool eof) {
  auto res = tp_.sendWebTransportStreamData(id, std::move(data), eof);
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
WebTransportImpl::stopReadingWebTransportIngress(HTTPCodec::StreamID id,
                                                 uint32_t errorCode) {
  auto res = tp_.stopReadingWebTransportIngress(id, errorCode);
  wtIngressStreams_.erase(id);
  sp_.refreshTimeout();
  return res;
}

folly::Expected<WebTransport::FCState, WebTransport::ErrorCode>
WebTransportImpl::StreamWriteHandle::writeStreamData(
    std::unique_ptr<folly::IOBuf> data, bool fin) {
  if (stopSendingErrorCode_) {
    return folly::makeUnexpected(WebTransport::ErrorCode::STOP_SENDING);
  }
  impl_.sp_.refreshTimeout();
  auto fcState = impl_.sendWebTransportStreamData(id_, std::move(data), fin);
  if (fcState.hasError()) {
    return folly::makeUnexpected(fcState.error());
  }
  return *fcState;
}

folly::Expected<folly::SemiFuture<folly::Unit>, WebTransport::ErrorCode>
WebTransportImpl::StreamWriteHandle::awaitWritable() {
  CHECK(!writePromise_) << "awaitWritable already called";
  auto contract = folly::makePromiseContract<folly::Unit>();
  writePromise_.emplace(std::move(contract.promise));
  writePromise_->setInterruptHandler(
      [this](const folly::exception_wrapper& ex) {
        VLOG(4) << "Exception from interrupt handler ex=" << ex.what();
        // if awaitWritable is cancelled, just reset it
        CHECK(ex.with_exception([this](const folly::FutureCancellation& ex) {
          VLOG(5) << "Setting exception ex=" << ex.what();
          writePromise_->setException(ex);
          writePromise_.reset();
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
  auto token = cancellationSource_.getToken();
  if (writePromise_) {
    writePromise_->setException(WebTransport::Exception(errorCode));
    writePromise_.reset();
  } else if (!stopSendingErrorCode_) {
    stopSendingErrorCode_ = errorCode;
  }

  cancellationSource_.requestCancellation();
}

void WebTransportImpl::StreamWriteHandle::onStreamWriteReady(
    quic::StreamId, uint64_t) noexcept {
  if (writePromise_) {
    writePromise_->setValue();
    writePromise_.reset();
  }
}

folly::SemiFuture<WebTransport::StreamData>
WebTransportImpl::StreamReadHandle::readStreamData() {
  VLOG(4) << __func__;
  CHECK(!readPromise_) << "One read at a time";
  if (error_) {
    auto ex = std::move(*error_);
    impl_.wtIngressStreams_.erase(getID());
    return folly::makeSemiFuture<WebTransport::StreamData>(std::move(ex));
  } else if (buf_.empty() && !eof_) {
    VLOG(4) << __func__ << " waiting for data";
    auto contract = folly::makePromiseContract<WebTransport::StreamData>();
    readPromise_.emplace(std::move(contract.promise));
    readPromise_->setInterruptHandler(
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
    WebTransport::StreamData streamData({buf_.move(), eof_});
    if (eof_) {
      impl_.wtIngressStreams_.erase(getID());
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
  quic::Buf data = std::move(readRes.value().first);
  bool eof = readRes.value().second;
  // deliver data, eof
  auto state = dataAvailable(std::move(data), eof);
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
  if (readPromise_) {
    readPromise_->setValue(WebTransport::StreamData({std::move(data), eof}));
    readPromise_.reset();
    if (eof) {
      impl_.wtIngressStreams_.erase(getID());
      return WebTransport::FCState::UNBLOCKED;
    }
  } else {
    buf_.append(std::move(data));
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
  cancellationSource_.requestCancellation();
  if (readPromise_) {
    readPromise_->setException(ex);
    readPromise_.reset();
    impl_.wtIngressStreams_.erase(getID());
  } else {
    error_ = ex;
  }
}

} // namespace proxygen
