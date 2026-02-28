/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPError.h"
#include "proxygen/lib/http/coro/HTTPSourceHolder.h"
#include "proxygen/lib/http/coro/util/CancellableBaton.h"
#include <folly/coro/Task.h>
#include <folly/logging/xlog.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>

namespace proxygen::coro {

/**
 * This class supports egress buffering and prioritization for HTTPCoroSession.
 * It will read and queue body events from a source, as long as there is space
 * available in its buffer.  Errors are returned direclty, but for non-error,
 * only the EOM flag is returned from readBodyEvent.
 *
 * The session will later call dequeueBodyEvent to retrieve the events for
 * serialization.
 */
class HTTPBodyEventQueue {
 public:
  class Callback {
   public:
    virtual void onEgressBytesBuffered(int64_t bytes) noexcept = 0;
    virtual ~Callback() = default;
  };

  HTTPBodyEventQueue(
      folly::EventBase* evb,
      HTTPCodec::StreamID id,
      Callback& callback,
      size_t limit = 65535,
      std::chrono::milliseconds writeTimeout = std::chrono::seconds(5))
      : id_(id),
        callback_(callback),
        evb_(evb),
        writeTimeout_(writeTimeout),
        limit_(limit) {
  }

  virtual ~HTTPBodyEventQueue() = default;

  void setSource(HTTPSource* source) {
    XCHECK(!source_);
    source_.setSource(source);
  }

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent();

  // Like HTTPSource::readBodyEvent, but returns only eom signal and buffers
  // the event.  Errors go right through.
  struct ReadBodyResult {
    folly::Optional<folly::coro::Task<TimedBaton::Status>> resume{folly::none};
    bool eom{false};
  };
  folly::coro::Task<ReadBodyResult> readBodyEvent(
      uint32_t max = std::numeric_limits<uint32_t>::max());

  uint64_t observedBodyLength() const {
    return observedBodyLength_;
  }

  virtual void contentLengthMismatch() {
    XLOG(ERR) << folly::to<std::string>(
        "Content-Length/body mismatch on egress: expected= ",
        expectedContentLength_,
        ", actual= ",
        observedBodyLength_);
    shouldValidateContentLength_ = false;
  }

  void validateContentLength(bool eom = false) {
    if (shouldValidateContentLength_ &&
        (observedBodyLength_ > expectedContentLength_ ||
         (eom && observedBodyLength_ < expectedContentLength_))) {
      contentLengthMismatch();
    }
  }

  void setExpectedEgressContentLength(const std::string& contentLen, bool eom);

  bool empty() const {
    return bodyQueue_.empty();
  }

  void clear(const HTTPError& err);

  HTTPBodyEvent dequeueBodyEvent(
      uint32_t max = std::numeric_limits<uint32_t>::max());

  void stopReading(HTTPErrorCode error) {
    if (source_) {
      source_.stopReading(error);
    }
    // TODO: if this is called while a coroutine is blocked on events, the
    // reading coroutine would be stuck.  The intent of the API not to do that.
  }

  void skipContentLengthValidation() {
    shouldValidateContentLength_ = false;
  }

 private:
  /**
   * return the amount of space available in the underlying buffer, comapred to
   * limit_.  If the buffer is over the limit, wait for buffer space to become
   * free.
   *
   * There's nothing "HTTP" specific here except we're using the HTTPError
   * for convenience
   */
  folly::coro::Task<size_t> waitAvailableBuffer();

  bool processBodyEvent(HTTPBodyEvent&& bodyEvent);

  size_t availableBuffer() {
    return limit_ >= bufferedBodyBytes_ ? limit_ - bufferedBodyBytes_ : 0;
  }

  HTTPCodec::StreamID id_;
  Callback& callback_;
  folly::EventBase* evb_;
  std::chrono::milliseconds writeTimeout_;
  detail::CancellableBaton event_;
  HTTPSourceHolder source_;
  std::list<HTTPBodyEvent> bodyQueue_;
  size_t bufferedBodyBytes_{0};
  size_t limit_;
  uint64_t expectedContentLength_{0};
  uint64_t observedBodyLength_{0};
  bool shouldValidateContentLength_{true};
};

} // namespace proxygen::coro
