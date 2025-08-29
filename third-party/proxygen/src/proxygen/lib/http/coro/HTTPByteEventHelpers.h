/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPEvents.h"
#include "proxygen/lib/http/coro/util/Refcount.h"

#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/HHWheelTimer.h>
#include <quic/api/QuicSocket.h>

#include <folly/Utility.h>

namespace proxygen::coro {

class PendingByteEvent : public folly::HHWheelTimer::Callback {
 public:
  PendingByteEvent(uint64_t inSessionOffset,
                   HTTPByteEvent inByteEvent,
                   HTTPByteEventCallbackPtr inCallback)
      : sessionOffset(inSessionOffset),
        byteEvent(std::move(inByteEvent)),
        callback(std::move(inCallback)) {
  }

  ~PendingByteEvent() override {
    if (callback) {
      XLOG_IF(DFATAL, "Cancelled PendingByteEvent with active callback");
      HTTPError err(HTTPErrorCode::CANCEL, "ByteEvent Cancelled");
      callback->onByteEventCanceled(byteEvent, std::move(err));
    }
  }

  void timeoutExpired() noexcept override {
    XLOG(DBG4) << "Timed out waiting for ByteEvent";
    if (auto cb = std::move(callback)) {
      cb->onByteEventCanceled(byteEvent,
                              HTTPError(HTTPErrorCode::READ_TIMEOUT,
                                        "Timed out waiting for ByteEvent"));
    }
    XCHECK(refcount);
    // incRef called when timeout scheduled in scheduleOrFireTxAckEvent
    refcount->decRef();
    refcount = nullptr;
  }

  void callbackCanceled() noexcept override {
    XCHECK(refcount);
    refcount->decRef();
    refcount = nullptr;
  }

  static size_t fireEvents(std::list<PendingByteEvent>& events,
                           uint64_t offset);

  static void cancelEvents(std::list<PendingByteEvent>& events,
                           const HTTPError& error);

  uint64_t sessionOffset;
  HTTPByteEvent byteEvent;
  HTTPByteEventCallbackPtr callback;
  // Initialized in scheduleOrFireTxAckEvent
  Refcount* refcount{nullptr};
};

// Holds byte events discovered during an iteration of writeLoop, and
// registered after QuicSocket::writeChain
struct QuicWriteLoopByteEvent {
  QuicWriteLoopByteEvent(
      std::vector<HTTPByteEventRegistration> inReg,
      folly::Optional<HTTPByteEvent::FieldSectionInfo> inFsInfo,
      uint64_t inBodyOffset,
      uint64_t inEventOffset)
      : byteEventRegistrations(std::move(inReg)),
        fieldSectionInfo(std::move(inFsInfo)),
        bodyOffset(inBodyOffset),
        eventOffset(inEventOffset) {
  }
  std::vector<HTTPByteEventRegistration> byteEventRegistrations;
  folly::Optional<HTTPByteEvent::FieldSectionInfo> fieldSectionInfo;
  uint64_t bodyOffset;
  uint64_t eventOffset;
};

class QuicByteEventCallback : public quic::ByteEventCallback {
 public:
  QuicByteEventCallback(Refcount& refcount,
                        HTTPByteEvent httpByteEvent,
                        HTTPByteEventCallbackPtr callback)
      : refcount_(refcount),
        httpByteEvent_(std::move(httpByteEvent)),
        callback_(std::move(callback)) {
  }

  [[nodiscard]] quic::ByteEvent::Type quicByteEventType() const {
    switch (httpByteEvent_.type) {
      case HTTPByteEvent::Type::NIC_TX:
      case HTTPByteEvent::Type::KERNEL_WRITE:
        return quic::ByteEvent::Type::TX;
      case HTTPByteEvent::Type::CUMULATIVE_ACK:
        return quic::ByteEvent::Type::ACK;
      case HTTPByteEvent::Type::TRANSPORT_WRITE:
      default:
        XLOG(FATAL) << "Invalid event type "
                    << folly::to_underlying(httpByteEvent_.type);
    }
  }

  void cancel() {
    callback_->onByteEventCanceled(
        httpByteEvent_, HTTPError(HTTPErrorCode::TRANSPORT_WRITE_ERROR));
    delete this;
  }

 private:
  void onByteEventRegistered(quic::ByteEvent byteEvent) override {
    XLOG(DBG4) << "onByteEventRegistered for id=" << byteEvent.id;
    refcount_.incRef();
  }

  void onByteEvent(quic::ByteEvent byteEvent) override {
    XLOG(DBG4) << "onByteEvent for id=" << byteEvent.id;
    callback_->onByteEvent(httpByteEvent_);
    refcount_.decRef();
    delete this;
  }

  void onByteEventCanceled(quic::ByteEvent byteEvent) override {
    XLOG(DBG4) << "onByteEventCanceled for id=" << byteEvent.id;
    auto& refcount = refcount_;
    cancel();
    refcount.decRef();
  }

  Refcount& refcount_;
  HTTPByteEvent httpByteEvent_;
  HTTPByteEventCallbackPtr callback_;
};

class AsyncSocketByteEventObserver
    : public folly::AsyncSocket::LegacyLifecycleObserver {
 public:
  struct RegAndEvent {
    HTTPByteEventRegistration registration;
    HTTPByteEvent byteEvent;
  };
  struct TxAckEvent {
    explicit TxAckEvent(uint64_t offset) : sessionByteOffset(offset) {
    }
    const uint64_t sessionByteOffset;
    std::vector<RegAndEvent> regAndEvents;
    folly::WriteFlags writeFlags();

    void cancel(const HTTPError& error) {
      auto vec = std::move(regAndEvents);
      for (auto& regAndEvent : vec) {
        regAndEvent.registration.cancel(error,
                                        std::move(regAndEvent.byteEvent));
      }
    }
  };

  AsyncSocketByteEventObserver()
      : LegacyLifecycleObserver(getConfigWithByteEventsEnabled()) {
  }

  void setByteEventTimeout(std::chrono::milliseconds timeout) {
    byteEventTimeout_ = timeout;
  }

  void cancelEvents(const HTTPError& error) {
    PendingByteEvent::cancelEvents(transportWriteEvents_, error);
    PendingByteEvent::cancelEvents(kernelWriteEvents_, error);
    for (auto& txAckEvent : txAckEvents_) {
      txAckEvent.cancel(error);
    }
    txAckEvents_.clear();
    numPendingTxAckEvents_ = 0;
    PendingByteEvent::cancelEvents(txEvents_, error);
    PendingByteEvent::cancelEvents(ackEvents_, error);
  }

  void registerByteEvents(
      uint64_t streamID,
      uint64_t sessionByteOffset,
      const folly::Optional<HTTPByteEvent::FieldSectionInfo>& fsInfo,
      uint64_t bodyOffset,
      std::vector<HTTPByteEventRegistration>&& registrations,
      bool eom);

  void transportWrite(uint64_t sessionWrittenOffset) {
    PendingByteEvent::fireEvents(transportWriteEvents_, sessionWrittenOffset);
  }

  void transportWriteComplete(uint64_t sessionWrittenOffset,
                              folly::Optional<TxAckEvent> txAckEvent) {
    PendingByteEvent::fireEvents(kernelWriteEvents_, sessionWrittenOffset);
    if (txAckEvent) {
      for (auto& regAndEvent : txAckEvent->regAndEvents) {
        scheduleOrFireTxAckEvent(std::move(regAndEvent));
      }
    }
  }

  folly::coro::Task<TimedBaton::Status> zeroRefs() {
    return refCount_.zeroRefs();
  }

  folly::Optional<TxAckEvent> nextTxAckEvent() {
    if (txAckEvents_.empty()) {
      return folly::none;
    } else {
      TxAckEvent txAckEvent(std::move(txAckEvents_.front()));
      txAckEvents_.pop_front();
      for (auto& regAndEvent : txAckEvent.regAndEvents) {
        numPendingTxAckEvents_ -=
            numTxAckEventFlags(regAndEvent.registration.events);
      }
      return txAckEvent;
    }
  }

  [[nodiscard]] bool isRegistered() const {
    return timer_ != nullptr;
  }

  // AsyncSocket::LegacyLifecycleObserver overrides
  void observerAttach(folly::AsyncSocket* sock) noexcept override {
    timer_ = &sock->getEventBase()->timer();
  }
  void observerDetach(folly::AsyncSocket*) noexcept override {
    timer_ = nullptr;
  }
  void destroy(folly::AsyncSocket* /* socket */) noexcept override {
  }
  void byteEvent(folly::AsyncSocket*,
                 const folly::AsyncSocketObserverInterface::ByteEvent&
                     event) noexcept override;

  void byteEventsEnabled(folly::AsyncSocket*) noexcept override {
    txAckByteEventsEnabled_ = true;
  }

  void byteEventsUnavailable(
      folly::AsyncSocket*,
      const folly::AsyncSocketException&) noexcept override {
    txAckByteEventsEnabled_ = false;
  }

 private:
  static Config getConfigWithByteEventsEnabled() {
    Config config;
    config.byteEvents = true;
    return config;
  }

  uint8_t numTxAckEventFlags(uint8_t events) {
    bool txEvent = events & uint8_t(HTTPByteEvent::Type::NIC_TX);
    bool ackEvent = events & uint8_t(HTTPByteEvent::Type::CUMULATIVE_ACK);
    return (txEvent ? 1 : 0) + (ackEvent ? 1 : 0);
  }

  // There's a maximum number of outstanding events, returns true if we can
  // register nEvents more without exceeding the limit
  bool canRegister(uint8_t nEvents) const;

  void scheduleOrFireTxAckEvent(RegAndEvent regAndEvent);

  void decRef(size_t nEvents) {
    while (nEvents-- > 0) {
      refCount_.decRef();
    }
  }

  folly::HHWheelTimer* timer_{nullptr};
  size_t maxTransportWriteOffset_{0};
  size_t maxTransportTxOffset_{0};
  size_t maxTransportAckOffset_{0};
  std::chrono::milliseconds byteEventTimeout_{std::chrono::seconds(10)};
  Refcount refCount_;
  std::list<PendingByteEvent> transportWriteEvents_;
  std::list<PendingByteEvent> kernelWriteEvents_;
  std::list<TxAckEvent> txAckEvents_;
  std::list<PendingByteEvent> txEvents_;
  std::list<PendingByteEvent> ackEvents_;
  uint8_t numPendingTxAckEvents_{0};
  bool txAckByteEventsEnabled_{false};
};

} // namespace proxygen::coro
