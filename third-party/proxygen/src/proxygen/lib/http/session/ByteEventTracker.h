/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/session/AckLatencyEvent.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <proxygen/lib/http/session/TransactionByteEvents.h>
#include <proxygen/lib/utils/Time.h>

namespace proxygen {

class TTLBAStats;

/**
 * ByteEventTracker can be used to fire application callbacks when a given
 * byte of a transport stream has been processed.  The primary usage is to
 * fire the callbacks when the byte is accepted by the transport, not when
 * the byte has been written on the wire, or acknowledged.
 *
 * Subclasses may implement handling of acknowledgement timing.
 */
class ByteEventTracker {
 public:
  class Callback {
   public:
    virtual ~Callback() {
    }
    virtual void onPingReplyLatency(int64_t latency) noexcept = 0;
    virtual void onTxnByteEventWrittenToBuf(
        const ByteEvent& event) noexcept = 0;
    virtual void onDeleteTxnByteEvent() noexcept = 0;
  };

  virtual ~ByteEventTracker();
  explicit ByteEventTracker(Callback* callback) : callback_(callback) {
  }

  /**
   * Assumes the byte events of another ByteEventTracker that this object
   * is replacing.
   */
  virtual void absorb(ByteEventTracker&& other);
  void setCallback(Callback* callback) {
    callback_ = callback;
  }

  /**
   * drainByteEvents should be called to clear out any pending events holding
   * transactions when processByteEvents will no longer be called
   */
  virtual size_t drainByteEvents();

  /**
   * processByteEvents is called whenever the transport has accepted more bytes.
   * bytesWritten is the number of bytes written to the transport over its
   * lifetime.
   */
  virtual bool processByteEvents(std::shared_ptr<ByteEventTracker> self,
                                 uint64_t bytesWritten);

  /**
   * Called when a ByteEvent offset has been written to the socket.
   *
   * Triggered by processByteEvents. Can be overridden by subclasses to trigger
   * adding timestamps on socket writes.
   */
  virtual void onByteEventWrittenToSocket(const ByteEvent& /* event */) {
  }

  /**
   * The following methods add byte events for tracking
   */
  void addPingByteEvent(size_t pingSize,
                        TimePoint timestamp,
                        uint64_t bytesScheduledBeforePing,
                        ByteEvent::Callback callback = nullptr);

  virtual void addFirstBodyByteEvent(uint64_t offset,
                                     HTTPTransaction* txn,
                                     ByteEvent::Callback callback = nullptr);

  virtual void addFirstHeaderByteEvent(uint64_t offset,
                                       HTTPTransaction* txn,
                                       ByteEvent::Callback callback = nullptr);

  virtual void addLastByteEvent(
      HTTPTransaction* txn,
      uint64_t byteNo,
      ByteEvent::Callback callback = nullptr) noexcept;
  virtual void addTrackedByteEvent(
      HTTPTransaction* txn,
      uint64_t byteNo,
      ByteEvent::Callback callback = nullptr) noexcept;

  /**
   * Disables socket timestamp tracking and drains any related events.
   *
   * Returns the number of socket timestamp events drained, if any.
   * Only implemented for trackers with socket timestamp capabilities.
   */
  virtual size_t disableSocketTimestampEvents() {
    // not implemented for base ByteEventTracker
    return 0;
  }

  /** The base ByteEventTracker cannot track NIC TX. */
  virtual void addTxByteEvent(uint64_t /*offset*/,
                              ByteEvent::EventType /*eventType*/,
                              HTTPTransaction* /*txn*/,
                              ByteEvent::Callback = nullptr) {
  }

  /** The base ByteEventTracker cannot track ACKs. */
  virtual void addAckByteEvent(uint64_t /*offset*/,
                               ByteEvent::EventType /*eventType*/,
                               HTTPTransaction* /*txn*/,
                               ByteEvent::Callback = nullptr) {
  }

  /**
   * HTTPSession uses preSend to truncate writes when timestamping is required.
   *
   * In TX and ACK-tracking ByteEventTrackers, preSend should examine pending
   * byte events and return the number of bytes until the next byte requiring
   * timestamping or 0 if none are pending. In addition, when returning non-zero
   * value preSend should indicate the timestamping required (TX and/or ACK).
   */
  virtual uint64_t preSend(bool* /*cork*/,
                           bool* /*timestampTx*/,
                           bool* /*timestampAck*/,
                           uint64_t /*bytesWritten*/) {
    return 0;
  }

  virtual void setTTLBAStats(TTLBAStats* /* stats */) {
  }

 protected:
  // the last value of byteWritten passed to processByteEvents
  // should always increase
  uint64_t bytesWritten_ = 0;

  // byteEvents_ is in the ascending order of ByteEvent::byteOffset_
  folly::CountedIntrusiveList<ByteEvent, &ByteEvent::listHook> byteEvents_;

  Callback* callback_;
};

} // namespace proxygen
