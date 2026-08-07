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
#include <proxygen/lib/transport/qmux/QmuxTransport.h>
#include <quic/common/events/QuicExecutor.h>

namespace proxygen::qmux {

class QmuxConnector {
 public:
  static folly::coro::Task<QmuxSession::Ptr> connect(
      std::shared_ptr<quic::QuicExecutor> executor,
      WtDir dir,
      QxTransportParams selfParams,
      std::unique_ptr<QmuxTransport> transport,
      std::chrono::milliseconds timeout,
      QmuxSession::Config sessionConfig = {});
};

// Builds the WtConfig that QmuxSession needs from the QUIC transport
// parameters each side advertised. Exposed for testing.
WtStreamManager::WtConfig makeWtConfig(
    const QxTransportParams& selfParams,
    const QxTransportParams& peerParams) noexcept;

} // namespace proxygen::qmux
