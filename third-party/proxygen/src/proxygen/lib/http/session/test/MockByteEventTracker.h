/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/GMock.h>
#include <proxygen/lib/http/session/ByteEventTracker.h>

namespace proxygen {

class MockByteEventTracker : public ByteEventTracker {
 public:
  explicit MockByteEventTracker(Callback* callback)
      : ByteEventTracker(callback) {
  }

  MOCK_METHOD(void,
              addPingByteEvent,
              (size_t, TimePoint, uint64_t, ByteEvent::Callback));
  MOCK_METHOD(void,
              addFirstBodyByteEvent,
              (uint64_t, HTTPTransaction*, ByteEvent::Callback));
  MOCK_METHOD(void,
              addFirstHeaderByteEvent,
              (uint64_t, HTTPTransaction*, ByteEvent::Callback));
  MOCK_METHOD(size_t, drainByteEvents, ());
  MOCK_METHOD(bool,
              processByteEvents,
              (std::shared_ptr<ByteEventTracker>, uint64_t));
  MOCK_METHOD(uint64_t, preSend, (bool*, bool*, bool*, uint64_t));

  MOCK_METHOD((void),
              addTrackedByteEvent,
              (HTTPTransaction*, uint64_t, ByteEvent::Callback),
              (noexcept));
  MOCK_METHOD((void),
              addLastByteEvent,
              (HTTPTransaction*, uint64_t, ByteEvent::Callback),
              (noexcept));
  MOCK_METHOD(
      (void),
      addTxByteEvent,
      (uint64_t, ByteEvent::EventType, HTTPTransaction*, ByteEvent::Callback),
      (noexcept));
  MOCK_METHOD(
      (void),
      addAckByteEvent,
      (uint64_t, ByteEvent::EventType, HTTPTransaction*, ByteEvent::Callback),
      (noexcept));

  // passthru to callback implementation functions
  void onTxnByteEventWrittenToBuf(const ByteEvent& event) {
    callback_->onTxnByteEventWrittenToBuf(event);
  }
};

} // namespace proxygen
