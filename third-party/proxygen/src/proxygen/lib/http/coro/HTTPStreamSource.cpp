/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/HTTPStreamSource.h"
#include <folly/logging/xlog.h>

constexpr uint32_t kMaxBufferedDatagramSize = 65536;

namespace proxygen::coro {

HTTPStreamSource::HTTPStreamSource(folly::EventBase* evb,
                                   HTTPCodec::StreamID id,
                                   Callback* callback,
                                   uint32_t recvStreamFlowControlWindow,
                                   std::chrono::milliseconds readTimeout,
                                   bool strictFlowControl)
    : id_(id),
      callback_(callback),
      recvWindow_(recvStreamFlowControlWindow),
      event_(CHECK_NOTNULL(evb), readTimeout),
      sinkMode_(false),
      sourceComplete_(false),
      strictFlowControl_(strictFlowControl),
      shouldValidateContentLength_(true),
      canSuspend_(true) {
}

HTTPStreamSource::HTTPStreamSource(folly::EventBase* evb,
                                   HTTPCodec::StreamID id,
                                   Callback& callback,
                                   uint32_t recvStreamFlowControlWindow,
                                   std::chrono::milliseconds readTimeout)
    : HTTPStreamSource(
          evb, id, &callback, recvStreamFlowControlWindow, readTimeout, true) {
}

HTTPStreamSource::HTTPStreamSource(folly::EventBase* evb,
                                   folly::Optional<HTTPCodec::StreamID> id,
                                   Callback* callback,
                                   uint32_t egressBufferSize)
    : HTTPStreamSource(evb,
                       (id ? *id : HTTPCodec::MaxStreamID),
                       callback,
                       egressBufferSize,
                       std::chrono::milliseconds(0),
                       false) {
}

void HTTPStreamSource::headers(std::unique_ptr<HTTPMessage> msg, bool eom) {
  if (!validateHeaders(*msg, eom)) {
    return;
  }

  if (!sinkMode_) {
    headerQueue_.emplace_back(std::move(msg), eom);
    event_.signal();
  }
}

auto HTTPStreamSource::body(BufQueue body,
                            uint16_t padding,
                            bool eom) -> FlowControlState {
  auto length = body.chainLength();
  if (!validateStateTransition(HTTPTransactionIngressSM::Event::onBody, eom)) {
    return FlowControlState::ERROR;
  }

  validateContentLength(eom, length);

  if (!recvWindow_.reserve(length, padding, strictFlowControl_)) {
    setError(HTTPErrorCode::FLOW_CONTROL_ERROR, "Sender exceeded flow control");
    return FlowControlState::ERROR;
  }
  if (sinkMode_) {
    updateFlowControl(length);
  } else {
    // TODO: consider updateFlowControl(0) if padding is non-zero?
    if (!bodyQueue_.empty() &&
        bodyQueue_.back().eventType == HTTPBodyEvent::BODY) {
      // When adding ByteEventRegistrations here, update coalescing
      auto& back = bodyQueue_.back();
      back.event.body.append(body.move());
      back.eom = eom;
    } else {
      bodyQueue_.emplace_back(std::move(body), eom);
    }
    event_.signal();
  }

  return recvWindow_.getSize() > 0 ? FlowControlState::OPEN
                                   : FlowControlState::CLOSED;
}

void HTTPStreamSource::datagram(std::unique_ptr<folly::IOBuf> datagram) {
  if (!validateStateTransition(HTTPTransactionIngressSM::Event::onBody,
                               false) ||
      !datagram) {
    return;
  }
  auto length = datagram->computeChainDataLength();
  if (bufferedDatagramSize_ + length > kMaxBufferedDatagramSize) {
    XLOG(DBG2) << "Dropping datagram length=" << length
               << " bufferedDatagramSize_=" << bufferedDatagramSize_;
    return;
  }
  if (!sinkMode_) {
    bufferedDatagramSize_ += length;
    bodyQueue_.emplace_back(HTTPBodyEvent::Datagram(), std::move(datagram));
    event_.signal();
  }
}

void HTTPStreamSource::pushPromise(std::unique_ptr<HTTPMessage> promise,
                                   HTTPSource* pushSource,
                                   bool eom) {
  // Treat this like a body event for state machine purposes.  Legal
  // anytime after headers and before eom
  XCHECK(pushSource);
  if (!validateStateTransition(HTTPTransactionIngressSM::Event::onBody, eom)) {
    pushSource->stopReading();
    return;
  }
  if (!sinkMode_) {
    bodyQueue_.emplace_back(std::move(promise), pushSource, eom);
    event_.signal();
  } else {
    pushSource->stopReading();
  }
}

void HTTPStreamSource::trailers(std::unique_ptr<HTTPHeaders> trailers) {
  // Trailers are supposed to signify EOM, but there's no requirement the
  // FIN flag be present there, and there's likely an eom coming later, so
  // have to pass eom=false here.
  if (!validateStateTransition(HTTPTransactionIngressSM::Event::onTrailers,
                               false)) {
    return;
  }
  validateContentLength(/*eom=*/true);
  if (!sinkMode_) {
    bodyQueue_.emplace_back(std::move(trailers));
    event_.signal();
  }
}

void HTTPStreamSource::padding(uint16_t bytes) {
  if (!validateStateTransition(HTTPTransactionIngressSM::Event::onBody,
                               false)) {
    return;
  }
  if (!sinkMode_) {
    bodyQueue_.emplace_back(HTTPBodyEvent::Padding(), bytes);
    event_.signal();
  }
}

void HTTPStreamSource::eom() {
  if (!validateStateTransition(HTTPTransactionIngressSM::Event::onEOM)) {
    return;
  }
  validateContentLength(/*eom=*/true);
  if (!sinkMode_) {
    if (bodyQueue_.empty()) {
      if (headerQueue_.empty()) {
        // both queues are empty, put eom in body queue
        bodyQueue_.emplace_back(nullptr, true);
        event_.signal();
        XLOG(DBG6) << "placing eom in bodyQueue_ id=" << id_;
      } else {
        // body queue is empty but header queue is not empty, put eom in
        // header queue
        headerQueue_.back().eom = true;
        XLOG(DBG6) << "placing eom in headerQueue_ id=" << id_;
      }
    } else {
      // body queue is non-empty, put eom in body queue, except datagram
      if (bodyQueue_.back().eventType == HTTPBodyEvent::DATAGRAM) {
        bodyQueue_.emplace_back(nullptr, true);
      } else {
        bodyQueue_.back().eom = true;
      }
      XLOG(DBG6) << "placing eom in bodyQueue_ id=" << id_;
    }
  } else {
    XLOG(DBG4) << "discarding eom for sinkMode_ id=" << id_;
  }
}

void HTTPStreamSource::abort(HTTPErrorCode error, std::string_view details) {
  if (!isEOMSeen()) {
    state_ = HTTPTransactionIngressSM::State::ReceivingDone;
    setError(error,
             folly::to<std::string>("HTTP source aborted err=",
                                    getErrorString(error),
                                    details.empty() ? "" : ", details=",
                                    details),
             /*ingress=*/true);
  }
}

folly::coro::Task<HTTPHeaderEvent> HTTPStreamSource::readHeaderEvent() {
  XCHECK(event_.getEventBase()->isInEventBaseThread());

  bool eomReturn = false;
  auto guard =
      folly::makeGuard([this, &eomReturn] { checkForCompletion(eomReturn); });
  if (readState_ != ReadState::HeaderEvents) {
    setError(HTTPErrorCode::INVALID_STATE_TRANSITION,
             "readHeaderEvent called after final headers read");
  }
  if (!hasHeaderEvents()) {
    // TODO: handle timeouts here
    co_await waitForEvent();
  }
  if (error_) {
    co_yield folly::coro::co_error(std::move(error_->first));
  }
  // Calling body() before headers() will error out of waitForEvent
  XCHECK(!headerQueue_.empty());
  auto res = std::move(headerQueue_.front());
  headerQueue_.pop_front();
  eomReturn = res.eom;
  if (res.isFinal()) {
    readState_ = ReadState::BodyEvents;
  }

  co_return res;
}

folly::coro::Task<HTTPBodyEvent> HTTPStreamSource::readBodyEvent(uint32_t max) {
  XCHECK_GT(max, 0UL);
  XCHECK(event_.getEventBase()->isInEventBaseThread());
  bool eomReturn = false;
  auto guard =
      folly::makeGuard([this, &eomReturn] { checkForCompletion(eomReturn); });
  if (readState_ != ReadState::BodyEvents) {
    setError(HTTPErrorCode::INVALID_STATE_TRANSITION,
             "readBodyEvent called before final headers read");
  }
  if (!hasBodyEvents()) {
    event_.reset();
    if (canSuspend_) {
      canSuspend_ = false;
      auto t = folly::coro::co_invoke(
          [this]() -> folly::coro::Task<TimedBaton::Status> {
            return event_.wait();
          });
      co_return HTTPBodyEvent(std::move(t));
    }
    co_await waitForEvent();
    canSuspend_ = true;
  }
  if (error_) {
    // TODO: handle timeouts differently here?
    co_yield folly::coro::co_error(std::move(error_->first));
  }
  // If there's no error, there must be queued body event
  XCHECK(!bodyQueue_.empty());
  auto res = std::move(bodyQueue_.front());
  bodyQueue_.pop_front();
  if (res.eventType == HTTPBodyEvent::BODY && !res.event.body.empty()) {
    auto length = res.event.body.chainLength();
    if (length > max) {
      // If the first event is bigger than the caller wants,
      // split the remainder and push it back onto the front of
      // the queue. If this happens, EOM must be false.
      BufQueue bodyBuf(std::move(res.event.body));
      res.event.body = bodyBuf.splitAtMost(max);
      bodyQueue_.emplace_front(bodyBuf.move(), res.eom);
      res.eom = false;
      length = max;
    }
    updateFlowControl(length);
  } else if (res.eventType == HTTPBodyEvent::DATAGRAM) {
    auto length = res.event.datagram->computeChainDataLength();
    if (length > max) {
      VLOG(2) << "Returning DATAGRAM with length=" << length
              << " exceeding max=" << max;
    }
    DCHECK_GE(bufferedDatagramSize_, length);
    bufferedDatagramSize_ -= length;
  }
  eomReturn = res.eom;
  co_return res;
}

void HTTPStreamSource::stopReading(folly::Optional<const HTTPErrorCode> error) {
  XCHECK(!sourceComplete_) << "Cannot call stopReading twice, or after you "
                              "have finished reading";

  setErrorImpl(error.value_or(HTTPErrorCode::NO_ERROR),
               "Abandoned reading",
               /*ingress=*/false);
  checkForCompletion(false);
}

bool HTTPStreamSource::validateHeaders(const HTTPMessage& msg, bool eom) {
  bool finalHeaders = msg.isFinal();
  if (!validateStateTransition(
          finalHeaders ? HTTPTransactionIngressSM::Event::onFinalHeaders
                       : HTTPTransactionIngressSM::Event::onNonFinalHeaders,
          eom)) {
    return false;
  }
  if (msg.isResponse() && msg.getStatusCode() == 304) {
    // TODO: 101 and 200/CONNECT
    shouldValidateContentLength_ = false;
  }
  if (finalHeaders && shouldValidateContentLength_) {
    const auto& contentLen =
        msg.getHeaders().getSingleOrEmpty(HTTP_HEADER_CONTENT_LENGTH);
    auto parsedContentLen = folly::tryTo<uint64_t>(contentLen);
    if (parsedContentLen.hasError()) {
      shouldValidateContentLength_ = false;
      XLOG_IF(ERR, !contentLen.empty())
          << "Invalid content-length: " << contentLen;
    } else {
      expectedIngressContentLength_ = expectedIngressContentLengthRemaining_ =
          *parsedContentLen;
      validateContentLength(eom);
    }
  }
  return true;
}

void HTTPStreamSource::setError(HTTPErrorCode error,
                                std::string msg,
                                bool ingress) {
  // Errors can either come from the ingress - eg: RST_STREAM on this
  // stream, or they can be generated by processing logic within this class
  // (flow control, state machine, etc).  We track the source of the error,
  // and only return an "egress" error in ingressComplete.  This prevents
  // sending a RST_STREAM in response to a RST_STREAM.
  if (error_) {
    // there's already an error queued, no-op
    return;
  }
  XLOG(DBG4) << "Encountered error on stream=" << id_
             << " error=" << uint64_t(error) << " msg=" << msg;
  setErrorImpl(error, std::move(msg), ingress);
}

void HTTPStreamSource::setErrorImpl(HTTPErrorCode error,
                                    std::string msg,
                                    bool ingress) {
  error_.emplace(HTTPError(error, std::move(msg)), ingress);
  if (!headerQueue_.empty() && headerQueue_.back().headers->isFinal()) {
    error_->first.httpMessage = std::move(headerQueue_.back().headers);
  }
  // no need to queue any more events
  enableSinkMode();
  event_.signal();
}

void HTTPStreamSource::enableSinkMode() {
  XLOG(DBG4) << "clearing source events for id_=" << id_;
  headerQueue_.clear();
  while (!bodyQueue_.empty()) {
    auto res = std::move(bodyQueue_.front());
    if (res.eventType == HTTPBodyEvent::BODY && !res.event.body.empty()) {
      auto length = res.event.body.chainLength();
      updateFlowControl(length);
    }
    bodyQueue_.pop_front();
  }
  sinkMode_ = true;
}

folly::coro::Task<void> HTTPStreamSource::waitForEvent() noexcept {
  event_.reset();
  // Really not sure why it can be > 1
  XCHECK_LT(waiters_, std::numeric_limits<uint8_t>::max());
  waiters_++;
  auto status = co_await event_.wait();
  waiters_--;
  if (status == TimedBaton::Status::timedout) {
    setError(HTTPErrorCode::READ_TIMEOUT,
             folly::to<std::string>("Read timeout after ",
                                    event_.getTimeout().count()));
  } else if (status == TimedBaton::Status::cancelled) {
    setError(HTTPErrorCode::CORO_CANCELLED, "Read cancelled");
  }
}

} // namespace proxygen::coro
