/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/IntrusiveList.h>
#include <folly/Portability.h>
#include <functional>
#include <proxygen/lib/utils/Time.h>

namespace proxygen {

class HTTPTransaction;
class ByteEvent {
 public:
  enum class EventFlags : uint8_t { ACK = 0x01, TX = 0x02 };

  enum EventType {
    FIRST_BYTE,
    LAST_BYTE,
    PING_REPLY_SENT,
    FIRST_HEADER_BYTE,
    TRACKED_BYTE,
    SECOND_TO_LAST_PACKET,
  };
  typedef std::function<void(ByteEvent&)> Callback;
  FOLLY_PUSH_WARNING
  FOLLY_CLANG_DISABLE_WARNING("-Wsigned-enum-bitfield")
  ByteEvent(uint64_t byteOffset,
            EventType eventType,
            Callback callback = nullptr)
      : eventType_(eventType),
        timestampTx_(false),
        timestampAck_(false),
        byteOffset_(byteOffset),
        callback_(callback) {
  }
  FOLLY_POP_WARNING
  virtual ~ByteEvent() {
  }
  EventType getType() const {
    return eventType_;
  }
  uint64_t getByteOffset() const {
    return byteOffset_;
  }
  virtual HTTPTransaction* getTransaction() const {
    return nullptr;
  }
  virtual int64_t getLatency() {
    return -1;
  }

  folly::SafeIntrusiveListHook listHook;
  EventType eventType_ : 3; // packed w/ byteOffset_
  // (tx|ack)Tracked_ is used by TX and ACK-tracking ByteEventTrackers to
  // mark if TX and/or ACK timestamping has been requested for this ByteEvent.
  // The ByteEventTracker's configuration determines which events TX and ACK
  // timestamping is requested for.
  //
  // For ByteEvents with timestamps requested, TX and ACK timestamps can be
  // captured by the ByteEventTracker::Callback handler by requesting them
  // via calls to addTxByteEvent and addAckByteEvent respectively when the
  // handler is processing the callback for onByteEvent. If the handler does
  // not add these events, the timestamps will still be generated but will not
  // be delivered to the handler.
  bool timestampTx_ : 1;  // packed w/ byteOffset_
  bool timestampAck_ : 1; // packed w/ byteOffset_
  uint64_t byteOffset_ : (8 * sizeof(uint64_t) - 5);
  Callback callback_{nullptr};
};

constexpr ByteEvent::EventFlags operator|(const ByteEvent::EventFlags& lhs,
                                          const ByteEvent::EventFlags& rhs) {
  return static_cast<ByteEvent::EventFlags>(
      std::underlying_type<ByteEvent::EventFlags>::type(lhs) |
      std::underlying_type<ByteEvent::EventFlags>::type(rhs));
}

constexpr bool operator&(const ByteEvent::EventFlags& lhs,
                         const ByteEvent::EventFlags& rhs) {
  return (std::underlying_type<ByteEvent::EventFlags>::type(lhs) &
          std::underlying_type<ByteEvent::EventFlags>::type(rhs));
}

std::ostream& operator<<(std::ostream& os, const ByteEvent& txn);

class PingByteEvent : public ByteEvent {
 public:
  PingByteEvent(uint64_t byteOffset,
                TimePoint pingRequestReceivedTime,
                ByteEvent::Callback callback)
      : ByteEvent(byteOffset, PING_REPLY_SENT, callback),
        pingRequestReceivedTime_(pingRequestReceivedTime) {
  }

  int64_t getLatency() override;

  TimePoint pingRequestReceivedTime_;
};

} // namespace proxygen
