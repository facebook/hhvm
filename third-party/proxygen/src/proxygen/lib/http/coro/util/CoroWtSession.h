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
#include <quic/priority/HTTPPriorityQueue.h>

namespace proxygen::coro::detail {

// if ENABLE_CONNECT_PROTOCOL is set; applies some default h2 wt settings on the
// HttpSettings (e.g. MaxData, MaxStreamData, etc.)
void setEgressWtHttpSettings(HTTPSettings* settings);

// derives the WtConfig from the ingress & egress HttpSettings of HttpCodec
WtStreamManager::WtConfig getWtConfig(const HTTPSettings* ingress,
                                      const HTTPSettings* egress);

/**
 * http/2 wt draft:
 * > In order to indicate support for WebTransport, both the client and the
 * > server MUST send a SETTINGS_WEBTRANSPORT_MAX_SESSIONS value greater than
 * > "0" in their SETTINGS frame
 *
 * > An endpoint needs to send both SETTINGS_ENABLE_CONNECT_PROTOCOL and
 * > SETTINGS_WEBTRANSPORT_MAX_SESSIONS for WebTransport to be enabled.
 */
bool supportsWt(std::initializer_list<const HTTPSettings*> settings);

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
  void sourceComplete(StreamId, folly::Optional<HTTPError>) override {
    waitForEgress.signal();
  }
  CancellableBaton waitForEgress;
};

/**
 * This is a helper utility (applicable to both http/2 and http/3) to visit
 * every Event yielded by WtStreamManager. The visitor simply serializes the
 * capsules in the IOBufQueue in order. This IOBufQueue should be subsequently
 * sent on the CONNECT stream (the stream originally carrying the CONNECT
 * stream).
 */
struct WtEventVisitor {
  folly::IOBufQueue& egress;
  bool sessionClosed{false};

  void operator()(WtStreamManager::ResetStream rst) const noexcept;
  void operator()(WtStreamManager::StopSending ss) const noexcept;
  void operator()(WtStreamManager::MaxConnData md) const noexcept;
  void operator()(WtStreamManager::MaxStreamData msd) const noexcept;
  void operator()(WtStreamManager::MaxStreamsBidi ms) const noexcept;
  void operator()(WtStreamManager::MaxStreamsUni ms) const noexcept;
  void operator()(WtStreamManager::DrainSession ds) const noexcept;
  void operator()(WtStreamManager::CloseSession cs) noexcept;
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

/**
 * This is a helper utility (applicable to both http/2 and http/3) that simply
 * aggregates all newly created peer stream ids into a std::vector for
 * convenience. The library should use these ids to query WtStreamManager and
 * pass the resulting handle into WebTransportHandler.
 */
struct WtStreamManagerIngressCallback : WtStreamManager::IngressCallback {
  ~WtStreamManagerIngressCallback() override = default;
  void onNewPeerStream(uint64_t streamId) noexcept override {
    peerStreams.push_back(streamId);
  }
  std::vector<uint64_t> peerStreams;
};

template <class T>
struct WtExpected {
  using Type = folly::Expected<T, WebTransport::ErrorCode>;
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

class CoroWtSession : public WebTransport {
 public:
  using Ptr = std::shared_ptr<CoroWtSession>;
  CoroWtSession(folly::EventBase* evb,
                detail::WtDir dir,
                WtStreamManager::WtConfig wtConfig,
                std::unique_ptr<WebTransportHandler> handler) noexcept;

  ~CoroWtSession() noexcept override;

  WtExpected<StreamWriteHandle*>::Type createUniStream() noexcept override;

  WtExpected<BidiStreamHandle>::Type createBidiStream() noexcept override;

  folly::SemiFuture<folly::Unit> awaitUniStreamCredit() noexcept override;

  folly::SemiFuture<folly::Unit> awaitBidiStreamCredit() noexcept override;

  WtExpected<folly::SemiFuture<StreamData>>::Type readStreamData(
      StreamId id) noexcept override;

  WtExpected<FCState>::Type writeStreamData(
      StreamId id,
      std::unique_ptr<folly::IOBuf> data,
      bool fin,
      ByteEventCallback* byteEventCallback) noexcept override;

  WtExpected<folly::Unit>::Type resetStream(StreamId id,
                                            uint32_t error) noexcept override;

  WtExpected<folly::Unit>::Type setPriority(
      uint64_t streamId,
      quic::PriorityQueue::Priority priority) noexcept override;

  folly::Expected<folly::Unit, WebTransport::ErrorCode> setPriorityQueue(
      std::unique_ptr<quic::PriorityQueue>) noexcept override;

  WtExpected<folly::SemiFuture<StreamId>>::Type awaitWritable(
      StreamId id) noexcept override;

  WtExpected<folly::Unit>::Type stopSending(StreamId id,
                                            uint32_t error) noexcept override;

  WtExpected<folly::Unit>::Type sendDatagram(
      std::unique_ptr<folly::IOBuf> datagram) noexcept override;

  [[nodiscard]] quic::TransportInfo getTransportInfo() const noexcept override;

  WtExpected<folly::Unit>::Type closeSession(
      folly::Optional<uint32_t> error = folly::none) noexcept override;

  // launches read & write loops
  void start(Ptr self, HTTPSourceHolder&& ingress, EgressSourcePtr&& egress);

  // fns invoked from WebTransportCapsuleCallback to let CoroWtSession there's
  // available uni/bidi stream credit
  void onBidiStreamCreditAvail() noexcept;
  void onUniStreamCreditAvail() noexcept;

 private:
  folly::coro::Task<void> readLoop(Ptr self, HTTPSourceHolder ingress);
  folly::coro::Task<void> writeLoop(Ptr self, EgressSourcePtr egress);

  void writeLoopFinished() noexcept;
  void readLoopFinished() noexcept;

  folly::EventBase* evb_;
  std::unique_ptr<WebTransportHandler> wtHandler_;
  folly::CancellationSource cs_;

  quic::HTTPPriorityQueue pq_;
  detail::WtStreamManagerEgressCallback wtSmEgressCb_;
  detail::WtStreamManagerIngressCallback wtSmIngressCb_;
  WtStreamManager sm_;
  folly::Promise<folly::Unit> awaitUniCredit_;
  folly::Promise<folly::Unit> awaitBidiCredit_;
  bool readLoopDone_ : 1 {false};
  bool writeLoopDone_ : 1 {false};
};

/**
 * This is a helper utility (applicable to both http/2 and http/3) to capsules
 * received on the CONNECT stream to the WtStreamManager. This is pretty much a
 * 1:1 mapping to the WtStreamManager api.
 */
struct WtCapsuleCallback : WebTransportCapsuleCodec::Callback {
  WtStreamManager& sm_;
  CoroWtSession& wtSess_;
  explicit WtCapsuleCallback(WtStreamManager& sm, CoroWtSession& sess) noexcept
      : sm_(sm), wtSess_(sess) {
  }
  ~WtCapsuleCallback() noexcept override = default;
  void onPaddingCapsule(PaddingCapsule) noexcept override;
  void onWTResetStreamCapsule(WTResetStreamCapsule) noexcept override;
  void onWTStopSendingCapsule(WTStopSendingCapsule) noexcept override;
  void onWTStreamCapsule(WTStreamCapsule) noexcept override;
  void onWTMaxDataCapsule(WTMaxDataCapsule) noexcept override;
  void onWTMaxStreamDataCapsule(WTMaxStreamDataCapsule) noexcept override;
  void onWTMaxStreamsBidiCapsule(WTMaxStreamsCapsule) noexcept override;
  void onWTMaxStreamsUniCapsule(WTMaxStreamsCapsule) noexcept override;
  void onWTDataBlockedCapsule(WTDataBlockedCapsule) noexcept override;
  void onWTStreamDataBlockedCapsule(
      WTStreamDataBlockedCapsule) noexcept override;
  void onWTStreamsBlockedBidiCapsule(WTStreamsBlockedCapsule) noexcept override;
  void onWTStreamsBlockedUniCapsule(WTStreamsBlockedCapsule) noexcept override;
  void onDatagramCapsule(DatagramCapsule) noexcept override;
  void onCloseWTSessionCapsule(
      CloseWebTransportSessionCapsule) noexcept override;
  void onDrainWTSessionCapsule(
      DrainWebTransportSessionCapsule capsule) noexcept override;
  void onCapsule(uint64_t capsuleType,
                 uint64_t capsuleLength) noexcept override;
  void onConnectionError(CapsuleCodec::ErrorCode) noexcept override;
};

} // namespace proxygen::coro::detail
