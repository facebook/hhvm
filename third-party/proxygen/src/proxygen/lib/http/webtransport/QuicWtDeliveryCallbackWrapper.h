/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/logging/xlog.h>
#include <proxygen/lib/http/webtransport/WebTransport.h>
#include <quic/api/QuicSocketLite.h>

namespace proxygen {

class QuicWtDeliveryCallbackWrapper : public quic::ByteEventCallback {
 public:
  explicit QuicWtDeliveryCallbackWrapper(
      WebTransport::DeliveryCallback* deliveryCallback,
      uint32_t offsetToSubtract)
      : deliveryCallback_(deliveryCallback),
        offsetToSubtract_(offsetToSubtract) {
  }

  void onByteEvent(quic::ByteEvent byteEvent) override {
    XCHECK_EQ(byteEvent.type, quic::ByteEvent::Type::ACK);
    deliveryCallback_->onDelivery(byteEvent.id,
                                  byteEvent.offset - offsetToSubtract_);
    delete this;
  }

  void onByteEventCanceled(quic::ByteEventCancellation cancellation) override {
    XCHECK_EQ(cancellation.type, quic::ByteEvent::Type::ACK);
    deliveryCallback_->onDeliveryCancelled(
        cancellation.id, cancellation.offset - offsetToSubtract_);
    delete this;
  }

 private:
  WebTransport::DeliveryCallback* deliveryCallback_;
  // The reason we need an offset to subtract is because some WebTransport
  // implementations (e.g. WebTransport over HTTP/3) write an initial header
  // on the stream.
  uint32_t offsetToSubtract_;
};

} // namespace proxygen
