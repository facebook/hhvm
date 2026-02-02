/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/webtransport/WebTransportCapsuleCodec.h>
#include <proxygen/lib/http/webtransport/WtStreamManager.h>

namespace proxygen {
class HTTPSettings;
}

namespace proxygen::detail {

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

/**
 * This is a helper utility (applicable to both http/2 and http/3) to capsules
 * received on the CONNECT stream to the WtStreamManager. This is pretty much a
 * 1:1 mapping to the WtStreamManager api.
 */
class WtSessionBase; // fwd-decl
struct WtCapsuleCallback : WebTransportCapsuleCodec::Callback {
  WtStreamManager& sm_;
  WtSessionBase& wtSess_;

  explicit WtCapsuleCallback(WtStreamManager& sm,
                             WtSessionBase& wtSess) noexcept
      : sm_(sm), wtSess_(wtSess) {
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

template <class T>
struct WtExpected {
  using Type = folly::Expected<T, WebTransport::ErrorCode>;
};
using StreamId = uint64_t;
class WtSessionBase : public WebTransport {
 public:
  using Ptr = std::shared_ptr<WtSessionBase>;
  WtSessionBase(folly::EventBase* evb, WtStreamManager& sm) noexcept;
  ~WtSessionBase() noexcept override = default;

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

  folly::Expected<folly::Unit, ErrorCode> setPriorityQueue(
      std::unique_ptr<quic::PriorityQueue> queue) noexcept override;

  WtExpected<folly::SemiFuture<StreamId>>::Type awaitWritable(
      StreamId id) noexcept override;

  WtExpected<folly::Unit>::Type stopSending(StreamId id,
                                            uint32_t error) noexcept override;

  WtExpected<folly::Unit>::Type sendDatagram(
      std::unique_ptr<folly::IOBuf> datagram) noexcept override;

  [[nodiscard]] quic::TransportInfo getTransportInfo() const noexcept override;

  WtExpected<folly::Unit>::Type closeSession(
      folly::Optional<uint32_t> error) noexcept override;

  // fns invoked from WebTransportCapsuleCallback to let CoroWtSession there's
  // available uni/bidi stream credit
  void onBidiStreamCreditAvail() noexcept;
  void onUniStreamCreditAvail() noexcept;

  folly::EventBase* evb() {
    return evb_;
  }

 private:
  folly::EventBase* evb_;
  WtStreamManager& sm_;
  folly::Promise<folly::Unit> awaitUniCredit_;
  folly::Promise<folly::Unit> awaitBidiCredit_;
};

} // namespace proxygen::detail
