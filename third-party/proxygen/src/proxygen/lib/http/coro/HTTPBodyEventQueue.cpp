/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/HTTPBodyEventQueue.h"

#include <folly/Conv.h>

using folly::coro::co_error;
using folly::coro::co_nothrow;
using folly::coro::co_safe_point;

namespace proxygen::coro {

folly::coro::Task<HTTPHeaderEvent> HTTPBodyEventQueue::readHeaderEvent() {
  auto res = co_await co_nothrow(source_.readHeaderEvent());
  const auto& ct = co_await folly::coro::co_current_cancellation_token;
  if (ct.isCancellationRequested()) {
    co_yield co_error(HTTPError{HTTPErrorCode::CORO_CANCELLED});
  }

  if (res.headers->isResponse() && res.headers->getStatusCode() == 304) {
    skipContentLengthValidation();
  } else if (shouldValidateContentLength_ && res.isFinal()) {
    setExpectedEgressContentLength(
        res.headers->getHeaders().getSingleOrEmpty(HTTP_HEADER_CONTENT_LENGTH),
        res.eom);
  }

  co_return res;
}

folly::coro::Task<HTTPBodyEventQueue::ReadBodyResult>
HTTPBodyEventQueue::readBodyEvent(uint32_t max) {
  XCHECK(source_);
  XLOG(DBG4) << "Waiting for buffer space";
  auto bufferSpace = availableBuffer();
  if (!bufferSpace) {
    bufferSpace = co_await co_nothrow(waitAvailableBuffer());
  }
  XLOG(DBG4) << "Got buffer space len=" << bufferSpace
             << " waiting for body event";
  max = std::min(max, uint32_t(bufferSpace));
  auto bodyEvent = co_await co_nothrow(source_.readBodyEvent(max));
  const auto& ct = co_await folly::coro::co_current_cancellation_token;
  if (ct.isCancellationRequested()) {
    co_yield co_error(HTTPError{HTTPErrorCode::CORO_CANCELLED});
  }

  if (bodyEvent.eventType == HTTPBodyEvent::SUSPEND) {
    co_return ReadBodyResult({.resume = std::move(bodyEvent.event.resume)});
  }
  if (bodyEvent.eventType == HTTPBodyEvent::BODY &&
      !bodyEvent.event.body.empty()) {
    callback_.onEgressBytesBuffered(
        static_cast<int64_t>(bodyEvent.event.body.chainLength()));
  }
  co_return ReadBodyResult({
      .resume = folly::none,
      .eom = processBodyEvent(std::move(bodyEvent)),
  });
}

void HTTPBodyEventQueue::setExpectedEgressContentLength(
    const std::string& contentLen, bool eom) {
  if (contentLen.empty()) {
    shouldValidateContentLength_ = false;
    return;
  }

  auto convResult = folly::tryTo<uint64_t>(contentLen);
  if (convResult.hasError()) {
    XLOG(ERR)
        << "Invalid content-length: " << contentLen << ", ex="
        << folly::makeConversionError(convResult.error(), contentLen).what();
    shouldValidateContentLength_ = false;
    return;
  }

  expectedContentLength_ = convResult.value();
  validateContentLength(eom);
}

void HTTPBodyEventQueue::clear(const HTTPError& err) {
  size_t bytesBuffered{0};
  for (auto& ev : bodyQueue_) {
    for (auto& reg : ev.byteEventRegistrations) {
      reg.cancel(err, folly::none);
    }
    if (ev.eventType == HTTPBodyEvent::EventType::BODY) {
      bytesBuffered += ev.event.body.chainLength();
    }
  }
  bodyQueue_.clear();
  callback_.onEgressBytesBuffered(-static_cast<int64_t>(bytesBuffered));
}

HTTPBodyEvent HTTPBodyEventQueue::dequeueBodyEvent(uint32_t max) {
  XCHECK(!bodyQueue_.empty());
  auto bodyEvent = std::move(bodyQueue_.front());
  bodyQueue_.pop_front();
  if (bodyEvent.eventType == HTTPBodyEvent::BODY) {
    auto length = bodyEvent.event.body.chainLength();
    auto toReturn = std::min(max, uint32_t(length));
    XCHECK_GE(bufferedBodyBytes_, toReturn);
    bool wasOverLimit = bufferedBodyBytes_ >= limit_;
    bufferedBodyBytes_ -= toReturn;
    if (wasOverLimit && bufferedBodyBytes_ < limit_) {
      event_.signal();
    }
    if (toReturn < length) {
      // only return part of it, split the buffer and re-push it to the front
      auto resultBuf = bodyEvent.event.body.splitAtMost(toReturn);
      bodyQueue_.emplace_front(std::move(bodyEvent));
      return HTTPBodyEvent(std::move(resultBuf), false);
    }
  }
  return bodyEvent;
}

folly::coro::Task<size_t> HTTPBodyEventQueue::waitAvailableBuffer() {
  while (bufferedBodyBytes_ >= limit_) {
    event_.reset();
    auto status = co_await event_.timedWait(evb_, writeTimeout_);
    if (status == TimedBaton::Status::timedout) {
      co_yield folly::coro::co_error(HTTPError(
          HTTPErrorCode::READ_TIMEOUT, "timed out waiting for buffer space"));
    } else if (status == TimedBaton::Status::cancelled) {
      co_yield folly::coro::co_error(HTTPError(
          HTTPErrorCode::CORO_CANCELLED, "cancelled waiting for buffer space"));
    }
  }
  co_return limit_ - bufferedBodyBytes_;
}

bool HTTPBodyEventQueue::processBodyEvent(HTTPBodyEvent&& bodyEvent) {
  XLOG(DBG4) << "Queuing body event";
  for (auto& reg : bodyEvent.byteEventRegistrations) {
    if (!reg.streamID) {
      reg.streamID = id_;
    }
  }
  bool eom = bodyEvent.eom;
  size_t addedBodyLength = 0;
  if (bodyEvent.eventType == HTTPBodyEvent::BODY) {
    addedBodyLength = bodyEvent.event.body.chainLength();
    bufferedBodyBytes_ += addedBodyLength;
    observedBodyLength_ += addedBodyLength;
    if (!bodyQueue_.empty() &&
        bodyQueue_.back().eventType == HTTPBodyEvent::BODY &&
        bodyQueue_.back().byteEventRegistrations.empty()) {
      // Coalesce this bodyEvent into the last one in queue
      auto& back = bodyQueue_.back();
      back.event.body.append(bodyEvent.event.body.move());
      back.eom = eom;
      back.byteEventRegistrations = std::move(bodyEvent.byteEventRegistrations);
    } else {
      bodyQueue_.emplace_back(std::move(bodyEvent));
    }
  } else {
    bodyQueue_.emplace_back(std::move(bodyEvent));
  }
  validateContentLength(eom);
  return eom;
}

} // namespace proxygen::coro
