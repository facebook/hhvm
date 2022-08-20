/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/session/ByteEventTracker.h>
#include <quic/api/QuicSocket.h>

namespace proxygen {

/**
 * ByteEventTracker specialized for HQSession.
 */
class HQByteEventTracker : public ByteEventTracker {
 public:
  HQByteEventTracker(Callback* callback,
                     quic::QuicSocket* socket,
                     quic::StreamId streamId);

  /**
   * Called when a ByteEvent offset has been written to the socket.
   *
   * Triggered by processByteEvents.
   *
   * HQByteEventTracker implementation registers callbacks for TX and ACK events
   * with the underlying QuicSocket when certain ByteEvents are written.
   */
  void onByteEventWrittenToSocket(const ByteEvent& event) override;

 private:
  quic::QuicSocket* const socket_;
  const quic::StreamId streamId_;
};

} // namespace proxygen
