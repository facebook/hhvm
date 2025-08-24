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

#include <quic/api/QuicSocket.h>

#include <folly/Utility.h>

namespace proxygen::coro {

struct PendingByteEvent {
  PendingByteEvent(uint64_t inSessionOffset,
                   HTTPByteEvent inByteEvent,
                   HTTPByteEventCallbackPtr inCallback)
      : sessionOffset(inSessionOffset),
        byteEvent(std::move(inByteEvent)),
        callback(std::move(inCallback)) {
  }

  ~PendingByteEvent() {
    if (callback) {
      XLOG_IF(DFATAL, "Cancelled PendingByteEvent with active callback");
      HTTPError err(HTTPErrorCode::CANCEL, "ByteEvent Cancelled");
      callback->onByteEventCanceled(byteEvent, std::move(err));
    }
  }

  static size_t fireEvents(std::list<PendingByteEvent>& events,
                           uint64_t offset);

  static void cancelEvents(std::list<PendingByteEvent>& events,
                           const HTTPError& error);

  uint64_t sessionOffset;
  HTTPByteEvent byteEvent;
  HTTPByteEventCallbackPtr callback;
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

} // namespace proxygen::coro
