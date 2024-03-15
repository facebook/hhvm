/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/session/ByteEvents.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>

namespace proxygen {

class TransactionByteEvent : public ByteEvent {
 public:
  TransactionByteEvent(uint64_t byteNo,
                       EventType eventType,
                       HTTPTransaction* txn,
                       ByteEvent::Callback callback = nullptr)
      : ByteEvent(byteNo, eventType, callback), txn_(txn) {
    txn_->incrementPendingByteEvents();
  }

  ~TransactionByteEvent() override {
    txn_->decrementPendingByteEvents();
  }

  HTTPTransaction* getTransaction() const override {
    return txn_;
  }

  HTTPTransaction* txn_;
};

/**
 * TimestampByteEvents are used to wait for TX and ACK timestamps.
 *
 * Contain a timeout that determines when the timestamp event expires (e.g., we
 * stop waiting to receive the timestamp from the system).
 */
class TimestampByteEvent
    : public TransactionByteEvent
    , public folly::HHWheelTimer::Callback {
 public:
  enum TimestampType {
    TX,
    ACK,
  };
  /**
   * The instances of TimestampByteEvent::Callback *MUST* outlive the ByteEvent
   * it is registered on.
   */
  class Callback {
   public:
    virtual ~Callback() {
    }
    virtual void timeoutExpired(TimestampByteEvent* event) noexcept = 0;
  };

  TimestampByteEvent(TimestampByteEvent::Callback* asyncTimeoutCallback,
                     TimestampType timestampType,
                     uint64_t byteNo,
                     EventType eventType,
                     HTTPTransaction* txn,
                     ByteEvent::Callback callback = nullptr)
      : TransactionByteEvent(byteNo, eventType, txn, callback),
        timestampType_(timestampType),
        asyncTimeoutCallback_(asyncTimeoutCallback) {
  }

  void timeoutExpired() noexcept override {
    asyncTimeoutCallback_->timeoutExpired(this);
  }

  const TimestampType timestampType_;

 private:
  Callback* asyncTimeoutCallback_;
};

} // namespace proxygen
