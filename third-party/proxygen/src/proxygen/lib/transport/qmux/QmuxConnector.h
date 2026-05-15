/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <folly/coro/Task.h>
#include <proxygen/lib/http/webtransport/WtStreamManager.h>
#include <proxygen/lib/transport/qmux/QmuxFramer.h>
#include <proxygen/lib/transport/qmux/QmuxSession.h>

namespace folly {
class EventBase;
} // namespace folly

namespace folly::coro {
class TransportIf;
} // namespace folly::coro

namespace proxygen::qmux {

class QmuxConnector {
 public:
  static folly::coro::Task<QmuxSession::Ptr> connect(
      folly::EventBase* evb,
      WtDir dir,
      QxTransportParams selfParams,
      std::unique_ptr<folly::coro::TransportIf> transport,
      std::chrono::milliseconds timeout,
      QmuxSession::Config sessionConfig = {});
};

// Builds the WtConfig that QmuxSession needs from the QUIC transport
// parameters each side advertised. Exposed for testing.
WtStreamManager::WtConfig makeWtConfig(
    const QxTransportParams& selfParams,
    const QxTransportParams& peerParams) noexcept;

} // namespace proxygen::qmux
