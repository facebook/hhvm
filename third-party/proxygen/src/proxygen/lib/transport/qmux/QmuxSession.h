/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <deque>
#include <folly/io/async/HHWheelTimer.h>
#include <proxygen/lib/http/coro/util/CoroWtSession.h>
#include <proxygen/lib/http/webtransport/WtStreamManager.h>
#include <proxygen/lib/http/webtransport/WtUtils.h>
#include <proxygen/lib/transport/qmux/QmuxFramer.h>

namespace folly::coro {
class TransportIf;
}

namespace proxygen::qmux {

using WtStreamManager = proxygen::detail::WtStreamManager;
using WtDir = proxygen::detail::WtDir;

// QmuxSession owns the steady-state I/O loops over a transport whose
// QX_TRANSPORT_PARAMETERS exchange has already completed. Construction
// requires a fully-formed WtConfig (typically built by QmuxConnector) and
// the peer's advertised max_record_size so the writer can size records
// correctly. Any bytes the connector read past the TP frame are passed in
// as `initialIngress` and drained by the readLoop before its first read.
class QmuxSession
    : public proxygen::coro::detail::CoroWtSessionBase
    , public proxygen::detail::WtSessionBase {
 public:
  using Ptr = std::shared_ptr<QmuxSession>;

  QmuxSession(folly::EventBase* evb,
              WtDir dir,
              QxTransportParams selfParams,
              std::unique_ptr<folly::coro::TransportIf> transport,
              WtStreamManager::WtConfig wtConfig,
              uint64_t peerMaxRecordSize,
              uint64_t effectiveMaxIdleTimeoutMs,
              std::unique_ptr<folly::IOBuf> initialIngress,
              Config config);
  ~QmuxSession() override;

  void setHandler(WebTransportHandler* handler) {
    wtHandler_ = handler;
  }

  void start(Ptr self);

  proxygen::detail::WtExpected<folly::Unit>::Type closeSession(
      folly::Optional<uint32_t> error = folly::none) noexcept override;

  proxygen::detail::WtExpected<folly::Unit>::Type sendDatagram(
      std::unique_ptr<folly::IOBuf> datagram) noexcept override;

  [[nodiscard]] const folly::SocketAddress& getLocalAddress()
      const noexcept override {
    return localAddr_;
  }

  [[nodiscard]] const folly::SocketAddress& getPeerAddress()
      const noexcept override {
    return peerAddr_;
  }

  [[nodiscard]] bool isIdleTimeoutScheduled() const noexcept {
    return idleTimeout_.isScheduled();
  }

 private:
  friend class QmuxCallback;
  folly::coro::Task<void> readLoop(Ptr self);
  folly::coro::Task<void> writeLoop(Ptr self);
  void readLoopFinished() noexcept;
  void writeLoopFinished() noexcept;

  void resetIdleTimeout();
  void onIdleTimeout();

  class IdleTimeoutCallback : public folly::HHWheelTimer::Callback {
   public:
    explicit IdleTimeoutCallback(QmuxSession& session) noexcept
        : session_(session) {
    }
    void timeoutExpired() noexcept override {
      session_.onIdleTimeout();
    }

   private:
    QmuxSession& session_;
  };

  WebTransportHandler* wtHandler_{nullptr};
  folly::SocketAddress localAddr_;
  folly::SocketAddress peerAddr_;
  folly::CancellationSource cs_;
  std::unique_ptr<folly::coro::TransportIf> transport_;
  QxTransportParams selfParams_;
  uint64_t effectiveMaxIdleTimeoutMs_;
  std::unique_ptr<folly::IOBuf> initialIngress_;
  std::deque<std::unique_ptr<folly::IOBuf>> pendingDatagrams_;
  std::vector<QxPing> pendingPongs_;
  uint64_t peerMaxRecordSize_;
  Config config_;
  IdleTimeoutCallback idleTimeout_{*this};
  bool readLoopDone_ : 1 {false};
  bool writeLoopDone_ : 1 {false};
};

} // namespace proxygen::qmux
