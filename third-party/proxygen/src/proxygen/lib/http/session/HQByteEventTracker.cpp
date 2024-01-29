/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HQByteEventTracker.h>

namespace {
class HQTransportByteEvent
    : public proxygen::TransactionByteEvent
    , public quic::QuicSocket::ByteEventCallback {

 public:
  using TransactionByteEvent::TransactionByteEvent;

  void onByteEvent(quic::QuicSocket::ByteEvent byteEvent) override {
    if (txn_) {
      switch (byteEvent.type) {
        case quic::QuicSocket::ByteEvent::Type::TX:
          txn_->onEgressTrackedByteEventTX(*this);
          break;
        case quic::QuicSocket::ByteEvent::Type::ACK:
          txn_->onEgressTrackedByteEventAck(*this);
          break;
      }
    }
    delete this;
  }

  void onByteEventCanceled(
      quic::QuicSocket::ByteEventCancellation /* cancellation */) override {
    delete this;
  }
};
} // namespace

namespace proxygen {

HQByteEventTracker::HQByteEventTracker(Callback* callback,
                                       quic::QuicSocket* socket,
                                       quic::StreamId streamId)
    : ByteEventTracker(callback), socket_(socket), streamId_(streamId) {
}

void HQByteEventTracker::onByteEventWrittenToSocket(const ByteEvent& event) {
  // create a ByteEvent
  const auto& txn = event.getTransaction();
  const auto& streamOffset = event.getByteOffset();
  switch (event.eventType_) {
    case ByteEvent::FIRST_BYTE:
      [[fallthrough]];
    case ByteEvent::LAST_BYTE: {
      // install TX callback
      {
        auto cb = new HQTransportByteEvent(
            streamOffset, event.eventType_, txn, nullptr);
        auto ret = socket_->registerTxCallback(streamId_, streamOffset, cb);
        if (ret.hasError()) {
          // failed to install callback; destroy
          delete cb;
        }
      }
      // install ACK callback
      {
        auto cb = new HQTransportByteEvent(
            streamOffset, event.eventType_, txn, nullptr);
        auto ret =
            socket_->registerDeliveryCallback(streamId_, streamOffset, cb);
        if (ret.hasError()) {
          // failed to install callback; destroy
          delete cb;
        }
      }
      break;
    }
    default:
      break;
  }
}

} // namespace proxygen
