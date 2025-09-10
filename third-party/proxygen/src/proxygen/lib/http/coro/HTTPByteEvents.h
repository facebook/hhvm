/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPError.h"
#include <folly/Optional.h>
#include <proxygen/lib/http/HTTPHeaderSize.h>
#include <proxygen/lib/utils/WeakRefCountedPtr.h>

namespace proxygen::coro {

/**
 * An HTTPByteEvent is an informational structure provided to an
 * HTTPByteEventCallback when the event fires, or is canceled.  Every byte event
 * has a Type:
 *
 *  TRANSPORT_WRITE: Event has been framed at the HTTP layer and is about to be
 *                   written to the transport
 *  KERNEL_WRITE:    Event has been successfully written to the kernel
 *  NIC_TX:          Event has been transmitted by the kernel to the network
 *  CUMULATIVE_ACK:  All bytes on this stream up to this Event have been
 *                   acknowledged by the peer.
 *
 * For HTTP/1x and HTTP/2, transportOffset is the offset of the event within the
 * HTTP session's egress byte stream.
 *
 * For HTTP/3, transportOffset is the offset within the QUIC stream carrying the
 * event.
 */
struct HTTPByteEvent {
  enum class Type : uint8_t {
    TRANSPORT_WRITE = 0x01,
    KERNEL_WRITE = 0x02,
    NIC_TX = 0x04,
    CUMULATIVE_ACK = 0x08,
  };
  static constexpr std::array<Type, 4> kByteEventTypes = {Type::TRANSPORT_WRITE,
                                                          Type::KERNEL_WRITE,
                                                          Type::NIC_TX,
                                                          Type::CUMULATIVE_ACK};

  // The stream ID may not be known in all cases
  folly::Optional<uint64_t> streamID;
  // fieldSectionInfo contains information about how the field section was
  // serialized and compressed, for HEADERS, TRAILERS or PUSH_PROMISE.  If the
  // event was cancelled before serialization, or for body events, this field is
  // not set.
  struct FieldSectionInfo {
    enum class Type { HEADERS, PUSH_PROMISE, TRAILERS };
    Type type;
    bool finalHeaders;
    HTTPHeaderSize size;
  };
  folly::Optional<FieldSectionInfo> fieldSectionInfo;
  uint64_t bodyOffset{0};      // body offset of the last byte of event
  uint64_t transportOffset{0}; // transport offset of the last byte of event
  uint64_t streamOffset{0};    // stream offset of the last byte of event

  bool eom{false};
  Type type;
};

/**
 * Callback class for receiving byte event notifications.
 */
class HTTPByteEventCallback
    : public EnableWeakRefCountedPtr<HTTPByteEventCallback> {
 public:
  virtual ~HTTPByteEventCallback() override = default;

  /**
   * Invoked when the byte event has occurred.
   */
  virtual void onByteEvent(HTTPByteEvent byteEvent) = 0;

  /**
   * Invoked if byte event is canceled due to reset, shutdown, or other error.
   *
   * The bodyOffset, transportOffset and eom flag of cancelledByteEvent may not
   * be known yet.
   */
  virtual void onByteEventCanceled(HTTPByteEvent cancelledByteEvent,
                                   HTTPError error) = 0;

 protected:
  // By default, the library uses HTTPByteEventCallback's like std::weak_ptr --
  // if the target has been deleted, the callback is ignored when the event
  // fires.  To guarantee callback delivery, delay destruction until
  // numWeakRefCountedPtrs() in onWeakRefCountedPtrDestroy() reaches 0.
  // See P583861615 for a helper class that can be used with std::shared_ptr
  void onWeakRefCountedPtrDestroy() override {
  }
  friend class WeakRefCountedPtr<HTTPByteEventCallback>;
};

using HTTPByteEventCallbackPtr = WeakRefCountedPtr<HTTPByteEventCallback>;

/**
 * This structure is supplied to an HTTPHeaderEvent or HTTPBodyEvent to register
 * for callbacks about its progress.  Once a registration has been created,
 * either onByteEvent or onByteEventCanceled will be invoked for each event
 * type.
 */
struct HTTPByteEventRegistration {
  folly::Optional<uint64_t> streamID; // if known
  uint8_t events{0};                  // Bitwise OR of HTTPByteEvent::Type
  HTTPByteEventCallbackPtr callback;

  HTTPByteEventRegistration() = default;
  HTTPByteEventRegistration(HTTPByteEventRegistration&& goner) = default;
  HTTPByteEventRegistration& operator=(HTTPByteEventRegistration&& goner) =
      default;
  ~HTTPByteEventRegistration() {
    cancel(
        HTTPError(HTTPErrorCode::CANCEL, "ByteEvent registration cancelled"));
  }
  void cancel(const HTTPError& error,
              folly::Optional<HTTPByteEvent> byteEvent = folly::none) {
    if (!byteEvent) {
      byteEvent.emplace();
      byteEvent->streamID = streamID;
    }
    for (auto t : HTTPByteEvent::kByteEventTypes) {
      if (events & uint8_t(t) && callback) {
        byteEvent->type = t;
        callback->onByteEventCanceled(*byteEvent, error);
      }
    }
    callback.reset();
  }
};

} // namespace proxygen::coro
