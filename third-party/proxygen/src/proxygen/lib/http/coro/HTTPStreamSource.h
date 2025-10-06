/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPError.h"
#include "proxygen/lib/http/coro/HTTPSource.h"
#include "proxygen/lib/http/coro/util/TimedBaton.h"
#include "proxygen/lib/http/coro/util/WindowContainer.h"
#include <proxygen/lib/http/codec/HTTPCodec.h>
#include <proxygen/lib/http/session/HTTPTransactionIngressSM.h>

#include <list>

namespace proxygen::coro {
/**
 * HTTPStreamSource is a source that is fed by a 'driver' that places events
 * into a queue.
 *
 * The proxygen coro session classes drive this class internally by reading and
 * parsing HTTP.
 *
 * Users of the library may use this class as an interface to non-coroutine
 * based asynchronous code that produces HTTP requests or responses.
 *
 * This class implements the consumer-only HTTPSource interface
 * (::readHeaderEvent, ::readBodyEvent), and also has a set of symmetric
 * producer methods to produce said events (::headers, ::body, etc.). The
 * consumer and producing threads may be different, however it only supports
 * single producer single consumer (SPSC).
 */
class HTTPStreamSource : public HTTPSource {
 public:
  /**
   * Callback for flow control and lifetime related events.  The callback must
   * outlive the source.
   *
   * Async producers that want flow control feedback can interpret the return
   * value from body to determine when to pause, and windowOpen to resume.
   */
  class Callback {
   public:
    virtual ~Callback() = default;
    Callback() = default;
    Callback(const Callback&) = delete;
    Callback& operator=(const Callback&) = delete;
    Callback(Callback&&) = delete;
    Callback& operator=(Callback&&) = delete;

    // The ID parameter to the callbacks can be HTTPCodec::kMaxStreamId if the
    // source is an async producer that did not provide a stream ID.

    // Called when amount of data is read out of this source. toAck is used by
    // sessions to know when to send stream flow control updates
    virtual void bytesProcessed(HTTPCodec::StreamID /*id*/,
                                size_t /*amount*/,
                                size_t /*toAck*/) {
    }

    // Called when the window changes from CLOSED -> OPEN
    virtual void windowOpen(HTTPCodec::StreamID /*id*/) {
    }

    // Called when the reader of this source has read EOM or error, or has
    // invoked stopReading
    virtual void sourceComplete(HTTPCodec::StreamID /*id*/,
                                folly::Optional<HTTPError> /*error*/) {
    }
  };

  // Constructor for sessions
  HTTPStreamSource(
      folly::EventBase* evb,
      HTTPCodec::StreamID id,
      Callback& callback,
      uint32_t recvStreamFlowControlWindow = 65535,
      std::chrono::milliseconds readTimeout = std::chrono::seconds(5));

  // Constructor for async producers
  // TODO: evb should not be required here (remove from TimedBaton/0)
  explicit HTTPStreamSource(
      folly::EventBase* evb,
      folly::Optional<HTTPCodec::StreamID> id = folly::none,
      Callback* callback = nullptr,
      uint32_t egressBufferSize = 65535);

  ~HTTPStreamSource() override {
    // must be destructed in evb thread
    auto* evb = event_.getEventBase();
    XCHECK(evb->isInEventBaseThread());
  }
  HTTPStreamSource(const HTTPStreamSource&) = delete;
  HTTPStreamSource& operator=(const HTTPStreamSource&) = delete;
  HTTPStreamSource(HTTPStreamSource&&) = delete;
  HTTPStreamSource& operator=(HTTPStreamSource&&) = delete;

  // Maybe collapse with getStreamID?
  HTTPCodec::StreamID getID() const {
    return id_;
  }

  // Calls from the 'driver' that fill the queue
  void headers(std::unique_ptr<HTTPMessage> msg, bool eom = false);
  enum class FlowControlState { OPEN, CLOSED, ERROR };
  FlowControlState body(BufQueue body, uint16_t padding, bool eom = false);
  void datagram(std::unique_ptr<folly::IOBuf> datagram);
  void pushPromise(std::unique_ptr<HTTPMessage> promise,
                   HTTPSource* pushSource,
                   bool eom = false);
  void trailers(std::unique_ptr<HTTPHeaders> trailers);
  void padding(uint16_t bytes);
  void eom();
  void abort(HTTPErrorCode error, std::string_view details = "");

  // HTTPSource methods
  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override;
  folly::coro::Task<HTTPBodyEvent> readBodyEvent(
      uint32_t max = std::numeric_limits<uint32_t>::max()) override;
  void stopReading(folly::Optional<const HTTPErrorCode> error) override;

  // This is only for H3 ingress push streams which can be created before a
  // transport stream ID is assigned
  void setStreamID(HTTPCodec::StreamID id) {
    XCHECK_EQ(id_, HTTPCodec::MaxStreamID);
    id_ = id;
  }

  folly::Optional<uint64_t> getStreamID() const override {
    return id_;
  }

  void setReadTimeout(std::chrono::milliseconds timeout) override {
    event_.setTimeout(timeout);
  }

  std::chrono::milliseconds getReadTimeout() const {
    return event_.getTimeout();
  }

  bool isUnprocessed() const {
    return state_ == HTTPTransactionIngressSM::State::Start;
  }

  bool headersAllowed() const {
    return isUnprocessed() ||
           state_ == HTTPTransactionIngressSM::State::NonFinalHeadersReceived;
  }

  // TODO: maybe remove this API
  void markEgressOnly() {
    state_ = HTTPTransactionIngressSM::State::ReceivingDone;
    enableSinkMode();
  }

  void skipContentLengthValidation() {
    shouldValidateContentLength_ = false;
  }

  bool isEOMSeen() const {
    return state_ == HTTPTransactionIngressSM::State::EOMQueued ||
           state_ == HTTPTransactionIngressSM::State::ReceivingDone;
  }

  bool inputFinished() const {
    return error_ || isEOMSeen();
  }

  void setCallback(Callback* callback) {
    callback_ = callback;
  }

  void describe(std::ostream& os) const {
    os << ", streamID=" << id_;
  }

  uint32_t bodyBytesBuffered() const {
    return recvWindow_.getWindow().getOutstanding();
  }

 private:
  bool hasHeaderEvents() const {
    return error_ || !headerQueue_.empty();
  }

  bool hasBodyEvents() const {
    return error_ || !bodyQueue_.empty();
  }

  void setError(HTTPErrorCode error, std::string msg, bool ingress = false);
  void setErrorImpl(HTTPErrorCode error, std::string msg, bool ingress);

  void enableSinkMode();

  folly::coro::Task<void> waitForEvent() noexcept;

  void contentLengthMismatchOnIngress(uint64_t observedBodySize) {
    auto errorMsg = folly::to<std::string>(
        "Content-Length/body mismatch on ingress: expected= ",
        expectedIngressContentLength_,
        ", actual= ",
        observedBodySize);
    XLOG(ERR) << errorMsg << " " << this;
    expectedIngressContentLengthRemaining_ = 0;
    shouldValidateContentLength_ = false;
    setError(HTTPErrorCode::CONTENT_LENGTH_MISMATCH, errorMsg);
  }

  // Because the driver can send events in any order, we must validate they
  // occur in a valid order per the HTTP state machine.
  bool validateStateTransition(HTTPTransactionIngressSM::Event event,
                               bool eomFlag = false) {
    if (!HTTPTransactionIngressSM::transit(state_, event)) {
      auto msg =
          folly::to<std::string>("Invalid ingress state transition, state=",
                                 state_,
                                 ", event=",
                                 event,
                                 ", streamID=",
                                 id_);
      setError(HTTPErrorCode::INVALID_STATE_TRANSITION, std::move(msg));
      return false;
    }
    if (eomFlag) {
      return validateStateTransition(HTTPTransactionIngressSM::Event::onEOM,
                                     false);
    }
    return true;
  }

  void validateContentLength(bool eom = false, size_t addedBodyLength = 0) {
    if (!shouldValidateContentLength_) {
      return;
    }
    if (addedBodyLength > expectedIngressContentLengthRemaining_) {
      contentLengthMismatchOnIngress(
          (addedBodyLength - expectedIngressContentLengthRemaining_) +
          expectedIngressContentLength_);
    } else {
      expectedIngressContentLengthRemaining_ -= addedBodyLength;
    }
    if (eom) {
      // Empty body ok with Content-Length set for HEAD responses and some 304
      // responses
      if (expectedIngressContentLengthRemaining_ > 0) {
        contentLengthMismatchOnIngress(expectedIngressContentLength_ -
                                       expectedIngressContentLengthRemaining_);
      }
    }
  }

  void updateFlowControl(size_t length) {
    bool wasBlocked = !sinkMode_ && recvWindow_.getSize() <= 0;
    auto toAck = recvWindow_.processed(length);
    if (callback_) {
      callback_->bytesProcessed(id_, length, inputFinished() ? 0 : toAck);
      if (wasBlocked && recvWindow_.getSize() > 0) {
        callback_->windowOpen(id_);
      }
    }
  }

  folly::Optional<HTTPError> getEgressError() {
    if (!error_ || error_->second) {
      return folly::none;
    }
    return error_->first;
  }

  void checkForCompletion(bool eomReturn) {
    if (!sourceComplete_ && (error_ || eomReturn) && waiters_ == 0) {
      sourceComplete_ = true;
      auto heapAllocated = heapAllocated_;
      if (callback_) {
        callback_->sourceComplete(id_, getEgressError());
      }
      if (heapAllocated) {
        delete this;
      }
    }
  }

  HTTPStreamSource(folly::EventBase* evb,
                   HTTPCodec::StreamID id,
                   Callback* callback,
                   uint32_t recvStreamFlowControlWindow,
                   std::chrono::milliseconds readTimeout,
                   bool strictFlowControl);

 protected:
  // consumer will be expected to only invoke ::readBodyEvent
  void validateHeadersAndSkip(const HTTPMessage& msg, bool eom = false) {
    XCHECK_EQ(readState_, ReadState::HeaderEvents);
    XCHECK(headerQueue_.empty());
    validateHeaders(msg, eom);
    readState_ = ReadState::BodyEvents;
  }

 private:
  bool validateHeaders(const HTTPMessage& msg, bool eom = false);

  HTTPCodec::StreamID id_;
  Callback* callback_;
  folly::Optional<std::pair<HTTPError, bool>> error_;
  std::list<HTTPHeaderEvent> headerQueue_;
  std::list<HTTPBodyEvent> bodyQueue_;
  WindowContainer recvWindow_;
  TimedBaton event_;
  uint64_t expectedIngressContentLength_{0};
  uint64_t expectedIngressContentLengthRemaining_{0};
  uint32_t bufferedDatagramSize_{0};
  uint8_t waiters_{0};
  HTTPTransactionIngressSM::State state_{
      HTTPTransactionIngressSM::getNewInstance()};
  enum class ReadState : uint8_t {
    HeaderEvents,
    BodyEvents
  } readState_{ReadState::HeaderEvents};
  bool sinkMode_ : 1;
  bool sourceComplete_ : 1;
  bool strictFlowControl_ : 1;
  bool shouldValidateContentLength_ : 1;
  bool canSuspend_ : 1;
};
} // namespace proxygen::coro
