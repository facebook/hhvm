/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/codec/TransportDirection.h>
#include <proxygen/lib/http/codec/webtransport/WebTransportCapsuleCodec.h>
#include <proxygen/lib/http/webtransport/WtStreamManager.h>

namespace proxygen {
class HTTPSettings;
}

namespace proxygen::detail {

// if ENABLE_CONNECT_PROTOCOL is set; applies some default h2 wt settings on the
// HttpSettings (e.g. MaxData, MaxStreamData, etc.)
void setEgressWtHttpSettings(HTTPSettings* settings) noexcept;

// if ENABLE_CONNECT_PROTOCOL is set; applies default h3 wt settings on the
// HttpSettings (e.g. MaxData, MaxStreams)
void setEgressWtH3Settings(HTTPSettings& settings) noexcept;

// derives the h2 WtConfig from the ingress & egress HttpSettings of HttpCodec
WtStreamManager::WtConfig getWtConfig(const HTTPSettings* ingress,
                                      const HTTPSettings* egress);

// derives the h3 WtConfig from the ingress & egress HttpSettings of HttpCodec
WtStreamManager::WtConfig getH3WtConfig(const HTTPSettings* ingress,
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
bool supportsH2Wt(std::initializer_list<const HTTPSettings*> settings) noexcept;

/**
 * http/3 wt draft:
 * Servers supporting WebTransport over HTTP/3 send:
 *   - A SETTINGS_WT_ENABLED setting with a value greater than "0"
 *   - A SETTINGS_ENABLE_CONNECT_PROTOCOL setting with a value of "1"
 *   - A SETTINGS_H3_DATAGRAM setting with a value of 1
 *
 * Clients supporting WebTransport over HTTP/3 send:
 *   - A SETTINGS_H3_DATAGRAM setting with a value of 1
 */
bool supportsH3Wt(TransportDirection dir,
                  const HTTPSettings* ingress,
                  const HTTPSettings* egress) noexcept;

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
  void onPadding(PaddingCapsule) noexcept override;
  void onResetStream(WTResetStreamCapsule) noexcept override;
  void onStopSending(WTStopSendingCapsule) noexcept override;
  void onStream(WTStreamCapsule) noexcept override;
  void onMaxData(WTMaxDataCapsule) noexcept override;
  void onMaxStreamData(WTMaxStreamDataCapsule) noexcept override;
  void onMaxStreamsBidi(WTMaxStreamsCapsule) noexcept override;
  void onMaxStreamsUni(WTMaxStreamsCapsule) noexcept override;
  void onDataBlocked(WTDataBlockedCapsule) noexcept override;
  void onStreamDataBlocked(WTStreamDataBlockedCapsule) noexcept override;
  void onStreamsBlockedBidi(WTStreamsBlockedCapsule) noexcept override;
  void onStreamsBlockedUni(WTStreamsBlockedCapsule) noexcept override;
  void onDatagram(DatagramCapsule) noexcept override;
  void onCloseSession(CloseWebTransportSessionCapsule) noexcept override;
  void onDrainSession(
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
  using IoBufPtr = std::unique_ptr<folly::IOBuf>;
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
      IoBufPtr data,
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
      IoBufPtr datagram) noexcept override;

  [[nodiscard]] quic::TransportInfo getTransportInfo() const noexcept override;

  WtExpected<folly::Unit>::Type closeSession(
      folly::Optional<uint32_t> error) noexcept override;

  // fns invoked from WebTransportCapsuleCallback to let CoroWtSession there's
  // available uni/bidi stream credit
  void onBidiStreamCreditAvail() noexcept;
  void onUniStreamCreditAvail() noexcept;
  // fn invoked from WebTransportCapsuleCallback
  void onDatagram(IoBufPtr datagram) noexcept {
    datagrams_.ingress.push_back(std::move(datagram));
  }

  folly::EventBase* evb() {
    return evb_;
  }

 protected:
  folly::Promise<folly::Unit>& uniCreditPromise() noexcept {
    return awaitUniCredit_;
  }
  folly::Promise<folly::Unit>& bidiCreditPromise() noexcept {
    return awaitBidiCredit_;
  }

  std::vector<IoBufPtr> moveIngressDatagrams() noexcept {
    return std::move(datagrams_.ingress);
  }
  std::vector<IoBufPtr> moveEgressDatagrams() noexcept {
    return std::move(datagrams_.egress);
  }

 private:
  folly::EventBase* evb_;
  WtStreamManager& sm_;
  folly::Promise<folly::Unit> awaitUniCredit_;
  folly::Promise<folly::Unit> awaitBidiCredit_;
  struct {
    std::vector<IoBufPtr> egress;
    std::vector<IoBufPtr> ingress;
  } datagrams_;
};

} // namespace proxygen::detail
