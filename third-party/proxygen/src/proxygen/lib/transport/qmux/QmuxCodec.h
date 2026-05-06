/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/webtransport/WtUtils.h>
#include <proxygen/lib/transport/qmux/QmuxFramer.h>

#include <folly/io/IOBufQueue.h>

namespace proxygen::qmux {

class QmuxCodec {
 public:
  class Callback : public proxygen::detail::WtCapsuleCallback {
   public:
    using WtCapsuleCallback::WtCapsuleCallback;
    ~Callback() noexcept override = default;

    // QMUX-specific callbacks
    virtual void onConnectionClose(QxConnectionClose capsule) noexcept = 0;
    virtual void onTransportParameters(QxTransportParams params) noexcept = 0;
    virtual void onPing(QxPing ping) noexcept = 0;
    virtual void onPong(QxPing pong) noexcept = 0;
  };

  explicit QmuxCodec(Callback* cb, OffsetValidator offsetValidator = nullptr);
  void onIngress(std::unique_ptr<folly::IOBuf> data);
  void setMaxRecordSize(uint64_t maxRecordSize);

 private:
  Callback* callback_;
  uint64_t maxRecordSize_{kDefaultMaxRecordSize};
  bool receivedTransportParams_{false};
  folly::IOBufQueue ingress_{folly::IOBufQueue::cacheChainLength()};
  folly::Optional<QmuxErrorCode> connError_;
  OffsetValidator offsetValidator_;
};

} // namespace proxygen::qmux
