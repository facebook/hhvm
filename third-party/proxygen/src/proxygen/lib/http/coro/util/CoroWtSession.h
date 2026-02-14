/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/webtransport/WebTransportCapsuleCodec.h>
#include <proxygen/lib/http/coro/HTTPSourceHolder.h>
#include <proxygen/lib/http/coro/HTTPStreamSource.h>
#include <proxygen/lib/http/coro/util/CancellableBaton.h>
#include <proxygen/lib/http/webtransport/WtStreamManager.h>
#include <proxygen/lib/http/webtransport/WtUtils.h>
#include <quic/priority/HTTPPriorityQueue.h>

namespace folly::coro {
class TransportIf;
}

namespace proxygen::coro::detail {

using WtStreamManager = proxygen::detail::WtStreamManager;
using WtDir = proxygen::detail::WtDir;

using StreamId = HTTPCodec::StreamID;

/**
 * This is a helper utility (applicable to both http/2 and http/3) to apply
 * backpressure when we're blocked on writing data on the upgraded stream (i.e.
 * the stream originally carrying the CONNECT request, referred to as the
 * CONNECT stream by the wt http/3 rfc)
 */
struct EgressBackPressure : public HTTPStreamSource::Callback {
  EgressBackPressure() {
    waitForEgress.signal(); // initially signalled
  }
  void windowOpen(StreamId) override {
    waitForEgress.signal();
  }
  void sourceComplete(StreamId, folly::Optional<HTTPError> err) override {
    waitForEgress.signal();
    ex = err.value_or(HTTPError{HTTPErrorCode::NO_ERROR});
  }
  CancellableBaton waitForEgress;
  folly::exception_wrapper ex;
};

/**
 * This is a helper utility (applicable to both http/2 and http/3) that simply
 * posts a baton once an event is available to be dequeued from WtStreamManager.
 */
struct WtStreamManagerEgressCallback : WtStreamManager::EgressCallback {
  ~WtStreamManagerEgressCallback() override = default;
  void eventsAvailable() noexcept override {
    waitForEvent.signal();
  }
  CancellableBaton waitForEvent;
};

struct EgressSource : HTTPStreamSource {
 public:
  using HTTPStreamSource::HTTPStreamSource;
  using HTTPStreamSource::validateHeadersAndSkip;
};
struct EgressSourcePtrDeleter {
  void operator()(EgressSource*) noexcept;
};
using EgressSourcePtr = std::unique_ptr<EgressSource, EgressSourcePtrDeleter>;

std::unique_ptr<folly::coro::TransportIf> makeHttpSourceTransport(
    folly::EventBase* evb,
    EgressSourcePtr&& egress,
    HTTPSourceHolder&& ingress);

template <class T>
struct WtExpected {
  using Type = folly::Expected<T, WebTransport::ErrorCode>;
};

// WtStreamManager must be constructed prior to WtSessionBase; shim class
struct CoroWtSessionBase {
  CoroWtSessionBase(WtDir dir, WtStreamManager::WtConfig wtConfig) noexcept
      : sm(dir, wtConfig, wtSmEgressCb, wtSmIngressCb, pq) {
  }
  quic::HTTPPriorityQueue pq;
  WtStreamManagerEgressCallback wtSmEgressCb;
  proxygen::detail::WtStreamManagerIngressCallback wtSmIngressCb;

  WtStreamManager sm;
};

class CoroWtSession
    : public CoroWtSessionBase
    , public proxygen::detail::WtSessionBase {
 public:
  using Ptr = std::shared_ptr<CoroWtSession>;
  CoroWtSession(folly::EventBase* evb,
                WtDir dir,
                WtStreamManager::WtConfig wtConfig,
                std::unique_ptr<WebTransportHandler> handler) noexcept;

  ~CoroWtSession() noexcept override;

  WtExpected<folly::Unit>::Type closeSession(
      folly::Optional<uint32_t> error = folly::none) noexcept override;

  // launches read & write loops
  void start(Ptr self, HTTPSourceHolder&& ingress, EgressSourcePtr&& egress);

 private:
  folly::coro::Task<void> readLoop(Ptr self, HTTPSourceHolder ingress);
  folly::coro::Task<void> writeLoop(Ptr self, EgressSourcePtr egress);

  void writeLoopFinished() noexcept;
  void readLoopFinished() noexcept;

  std::unique_ptr<WebTransportHandler> wtHandler_;
  folly::CancellationSource cs_;
  bool readLoopDone_ : 1 {false};
  bool writeLoopDone_ : 1 {false};
};

} // namespace proxygen::coro::detail
