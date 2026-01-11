/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/logging/xlog.h>
#include <proxygen/lib/http/coro/util/CoroWtSession.h>

namespace {
static constexpr std::string_view kWtEventVisitor = "WtEventVisitor";
using StreamId = proxygen::HTTPCodec::StreamID;
using WtErrCode = proxygen::WebTransport::ErrorCode;
using WtStreamManager = proxygen::coro::detail::WtStreamManager;
using StreamWriteHandle = proxygen::WebTransport::StreamWriteHandle;
using StreamReadHandle = proxygen::WebTransport::StreamReadHandle;
using BidiStreamHandle = proxygen::WebTransport::BidiStreamHandle;
using StreamData = proxygen::WebTransport::StreamData;
using FCState = proxygen::WebTransport::FCState;

template <class T>
struct WtExpected {
  using Type = folly::Expected<T, proxygen::WebTransport::ErrorCode>;
};

folly::Promise<folly::Unit> makeEmptyPromise() {
  return folly::Promise<folly::Unit>::makeEmpty();
}

folly::Promise<folly::Unit> resetPromise(
    folly::Promise<folly::Unit>& p) noexcept {
  return std::exchange(p, makeEmptyPromise());
}

}; // namespace

namespace proxygen::coro::detail {

// sets default egress h2 wt http settings if ENABLE_CONNECT_PROTOCOL set
void setEgressWtHttpSettings(HTTPSettings* settings) {
  if (supportsWt({settings})) {
    // max data
    constexpr uint64_t kWtInitMaxData = std::numeric_limits<uint16_t>::max();
    static constexpr auto kMaxDataSettings = {
        SettingsId::WT_INITIAL_MAX_DATA,
        SettingsId::WT_INITIAL_MAX_STREAM_DATA_UNI,
        SettingsId::WT_INITIAL_MAX_STREAM_DATA_BIDI};
    for (auto maxDataSetting : kMaxDataSettings) {
      settings->setIfNotPresent(maxDataSetting, kWtInitMaxData);
    }

    // max streams
    constexpr uint64_t kWtInitMaxStreams = 10;
    static constexpr auto kMaxStreamsSettings = {
        SettingsId::WT_INITIAL_MAX_STREAMS_UNI,
        SettingsId::WT_INITIAL_MAX_STREAMS_BIDI,
    };
    for (auto maxStreams : kMaxStreamsSettings) {
      settings->setIfNotPresent(maxStreams, kWtInitMaxStreams);
    }
  }
}

WtStreamManager::WtConfig getWtConfig(const HTTPSettings* ingress,
                                      const HTTPSettings* egress) {
  WtStreamManager::WtConfig config;
  // either both ingress&egress are nullptr (e.g. http/1.1) or both non-nullptr
  if (egress || ingress) {
    XCHECK(egress && ingress);
    // set peer's WtConfig via ingress HTTPSettings
    config.peerMaxConnData =
        ingress->getSetting(SettingsId::WT_INITIAL_MAX_DATA, /*defaultVal=*/0);
    config.peerMaxStreamDataUni = ingress->getSetting(
        SettingsId::WT_INITIAL_MAX_STREAM_DATA_UNI, /*defaultVal=*/0);
    config.peerMaxStreamDataBidi = ingress->getSetting(
        SettingsId::WT_INITIAL_MAX_STREAM_DATA_BIDI, /*defaultVal=*/0);
    config.peerMaxStreamsUni =
        ingress->getSetting(SettingsId::WT_INITIAL_MAX_STREAMS_UNI,
                            /*defaultVal=*/0);
    config.peerMaxStreamsBidi =
        ingress->getSetting(SettingsId::WT_INITIAL_MAX_STREAMS_BIDI,
                            /*defaultVal=*/0);

    // set self's WtConfig via egress HTTPSettings
    config.selfMaxConnData =
        egress->getSetting(SettingsId::WT_INITIAL_MAX_DATA, /*defaultVal=*/0);
    config.selfMaxStreamDataUni = egress->getSetting(
        SettingsId::WT_INITIAL_MAX_STREAM_DATA_UNI, /*defaultVal=*/0);
    config.selfMaxStreamDataBidi = egress->getSetting(
        SettingsId::WT_INITIAL_MAX_STREAM_DATA_BIDI, /*defaultVal=*/0);
    config.selfMaxStreamsUni =
        egress->getSetting(SettingsId::WT_INITIAL_MAX_STREAMS_UNI,
                           /*defaultVal=*/0);
    config.selfMaxStreamsBidi =
        egress->getSetting(SettingsId::WT_INITIAL_MAX_STREAMS_BIDI,
                           /*defaultVal=*/0);
  }

  XLOG(DBG6) << config.selfMaxStreamsBidi << "; " << config.selfMaxStreamsUni
             << "; " << config.selfMaxConnData << "; "
             << config.selfMaxStreamDataBidi << "; "
             << config.selfMaxStreamDataUni << "; " << config.peerMaxStreamsBidi
             << "; " << config.peerMaxStreamsUni << "; "
             << config.peerMaxConnData << "; " << config.peerMaxStreamDataBidi
             << "; " << config.peerMaxStreamDataUni;

  return config;
}

bool supportsWt(std::initializer_list<const HTTPSettings*> settings) {
  constexpr auto kEnableConnectProto = SettingsId::ENABLE_CONNECT_PROTOCOL;
  constexpr auto kEnableWtMaxSess = SettingsId::WT_MAX_SESSIONS;
  return std::all_of(settings.begin(), settings.end(), [](auto* settings) {
    return settings &&
           settings->getSetting(kEnableConnectProto, /*defaultVal=*/0) &&
           settings->getSetting(kEnableWtMaxSess, /*defaultVal=*/0);
  });
}

void WtEventVisitor::operator()(
    WtStreamManager::ResetStream rst) const noexcept {
  XLOG(DBG6) << kWtEventVisitor << " rst.id=" << rst.streamId
             << "; rst.err=" << rst.err;
  writeWTResetStream(
      egress,
      WTResetStreamCapsule{.streamId = rst.streamId,
                           .appProtocolErrorCode = uint32_t(rst.err),
                           .reliableSize = rst.reliableSize});
}

void WtEventVisitor::operator()(
    WtStreamManager::StopSending ss) const noexcept {
  XLOG(DBG6) << kWtEventVisitor << " ss.id=" << ss.streamId
             << "; ss.err=" << ss.err;
  writeWTStopSending(
      egress,
      WTStopSendingCapsule{.streamId = ss.streamId,
                           .appProtocolErrorCode = uint32_t(ss.err)});
}

void WtEventVisitor::operator()(
    WtStreamManager::MaxConnData md) const noexcept {
  XLOG(DBG6) << kWtEventVisitor << " md.offset=" << md.maxData;
  writeWTMaxData(egress, WTMaxDataCapsule{md.maxData});
}

void WtEventVisitor::operator()(
    WtStreamManager::MaxStreamData msd) const noexcept {
  XLOG(DBG6) << kWtEventVisitor << " msd.id=" << msd.streamId
             << " msd.offset=" << msd.maxData;
  writeWTMaxStreamData(
      egress,
      WTMaxStreamDataCapsule{.streamId = msd.streamId,
                             .maximumStreamData = msd.maxData});
}

void WtEventVisitor::operator()(
    WtStreamManager::MaxStreamsBidi ms) const noexcept {
  XLOG(DBG6) << kWtEventVisitor << " msd.maxStreamsBidi=" << ms.maxStreams;
  writeWTMaxStreams(egress,
                    WTMaxStreamsCapsule{.maximumStreams = ms.maxStreams},
                    /*isBidi=*/true);
}

void WtEventVisitor::operator()(
    WtStreamManager::MaxStreamsUni ms) const noexcept {
  XLOG(DBG6) << kWtEventVisitor << " msd.maxStreamsUni=" << ms.maxStreams;
  writeWTMaxStreams(egress,
                    WTMaxStreamsCapsule{.maximumStreams = ms.maxStreams},
                    /*isBidi=*/false);
}

void WtEventVisitor::operator()(WtStreamManager::DrainSession) const noexcept {
  XLOG(DBG6) << kWtEventVisitor << " ds";
  writeDrainWebTransportSession(egress);
}

void WtEventVisitor::operator()(WtStreamManager::CloseSession cs) noexcept {
  XLOG(DBG6) << kWtEventVisitor << " cs.err=" << cs.err << " cs.msg=" << cs.msg;
  sessionClosed = true;
  writeCloseWebTransportSession(
      egress,
      CloseWebTransportSessionCapsule{.applicationErrorCode = uint32_t(cs.err),
                                      .applicationErrorMessage =
                                          std::move(cs.msg)});
}

// WtCapsuleCallback
void WtCapsuleCallback::onPaddingCapsule(PaddingCapsule) noexcept {
}

void WtCapsuleCallback::onWTResetStreamCapsule(
    WTResetStreamCapsule c) noexcept {
  XLOG(DBG6) << __func__ << "; id=" << c.streamId
             << "; err=" << c.appProtocolErrorCode;
  sm_.onResetStream(
      WtStreamManager::ResetStream{.streamId = c.streamId,
                                   .err = c.appProtocolErrorCode,
                                   .reliableSize = c.reliableSize});
}

void WtCapsuleCallback::onWTStopSendingCapsule(
    WTStopSendingCapsule c) noexcept {
  XLOG(DBG6) << __func__ << "; id=" << c.streamId
             << "; err=" << c.appProtocolErrorCode;
  sm_.onStopSending(WtStreamManager::StopSending{
      .streamId = c.streamId, .err = c.appProtocolErrorCode});
}

void WtCapsuleCallback::onWTStreamCapsule(WTStreamCapsule c) noexcept {
  XLOG(DBG6) << __func__ << "; id=" << c.streamId;
  if (auto* rh = sm_.getOrCreateIngressHandle(c.streamId)) {
    sm_.enqueue(*rh, {std::move(c.streamData), c.fin});
  }
}

void WtCapsuleCallback::onWTMaxDataCapsule(WTMaxDataCapsule c) noexcept {
  XLOG(DBG6) << __func__ << "; offset=" << c.maximumData;
  sm_.onMaxData(WtStreamManager::MaxConnData{.maxData = c.maximumData});
}

void WtCapsuleCallback::onWTMaxStreamDataCapsule(
    WTMaxStreamDataCapsule c) noexcept {
  XLOG(DBG6) << __func__ << "; id=" << c.streamId
             << "; offset=" << c.maximumStreamData;
  sm_.onMaxData(
      WtStreamManager::MaxStreamData{{c.maximumStreamData}, c.streamId});
}

void WtCapsuleCallback::onWTMaxStreamsBidiCapsule(
    WTMaxStreamsCapsule c) noexcept {
  XLOG(DBG6) << __func__ << "; max=" << c.maximumStreams;
  bool wasAvail = sm_.canCreateBidi();
  sm_.onMaxStreams(WtStreamManager::MaxStreamsBidi{c.maximumStreams});
  if (!wasAvail && sm_.canCreateBidi()) {
    wtSess_.onBidiStreamCreditAvail();
  }
}

void WtCapsuleCallback::onWTMaxStreamsUniCapsule(
    WTMaxStreamsCapsule c) noexcept {
  XLOG(DBG6) << __func__ << "; max=" << c.maximumStreams;
  bool wasAvail = sm_.canCreateUni();
  sm_.onMaxStreams(WtStreamManager::MaxStreamsUni{c.maximumStreams});
  if (!wasAvail && sm_.canCreateUni()) {
    wtSess_.onUniStreamCreditAvail();
  }
}

void WtCapsuleCallback::onWTDataBlockedCapsule(WTDataBlockedCapsule) noexcept {
  XLOG(DBG6) << __func__;
}

void WtCapsuleCallback::onWTStreamDataBlockedCapsule(
    WTStreamDataBlockedCapsule) noexcept {
  XLOG(DBG6) << __func__;
}

void WtCapsuleCallback::onWTStreamsBlockedBidiCapsule(
    WTStreamsBlockedCapsule) noexcept {
  XLOG(DBG6) << __func__;
}

void WtCapsuleCallback::onWTStreamsBlockedUniCapsule(
    WTStreamsBlockedCapsule) noexcept {
  XLOG(DBG6) << __func__;
}

void WtCapsuleCallback::onDatagramCapsule(DatagramCapsule) noexcept {
  XLOG(DBG6) << __func__;
}

void WtCapsuleCallback::onCloseWTSessionCapsule(
    CloseWebTransportSessionCapsule c) noexcept {
  XLOG(DBG6) << __func__ << "; err=" << c.applicationErrorCode
             << "; c.msg=" << c.applicationErrorMessage;
  sm_.onCloseSession(WtStreamManager::CloseSession{
      .err = c.applicationErrorCode, .msg = c.applicationErrorMessage});
}

void WtCapsuleCallback::onDrainWTSessionCapsule(
    DrainWebTransportSessionCapsule) noexcept {
  XLOG(DBG6) << __func__;
  sm_.onDrainSession(WtStreamManager::DrainSession{});
}

void WtCapsuleCallback::onCapsule(uint64_t capsuleType,
                                  uint64_t capsuleLength) noexcept {
  XLOG(DBG7) << __func__ << "; capsuleType=" << capsuleType
             << "; capsuleLength=" << capsuleLength;
}

void WtCapsuleCallback::onConnectionError(CapsuleCodec::ErrorCode) noexcept {
  XLOG(DBG6) << __func__;
  sm_.onCloseSession(
      WtStreamManager::CloseSession{.err = 0, .msg = "onConnectionError"});
}

CoroWtSession::CoroWtSession(
    folly::EventBase* evb,
    detail::WtDir dir,
    WtStreamManager::WtConfig wtConfig,
    std::unique_ptr<WebTransportHandler> handler) noexcept
    : evb_(evb),
      wtHandler_(std::move(handler)),
      sm_{dir, wtConfig, wtSmEgressCb_, wtSmIngressCb_, pq_},
      awaitUniCredit_(makeEmptyPromise()),
      awaitBidiCredit_(makeEmptyPromise()) {
}

CoroWtSession::~CoroWtSession() noexcept {
  cs_.requestCancellation();
}

WtExpected<StreamWriteHandle*>::Type CoroWtSession::createUniStream() noexcept {
  if (auto* res = sm_.createEgressHandle()) {
    return res;
  }
  return folly::makeUnexpected(WtErrCode::STREAM_CREATION_ERROR);
}

WtExpected<BidiStreamHandle>::Type CoroWtSession::createBidiStream() noexcept {
  auto res = sm_.createBidiHandle();
  if (res.readHandle || res.writeHandle) {
    XCHECK(res.readHandle && res.writeHandle);
    return BidiStreamHandle{.readHandle = res.readHandle,
                            .writeHandle = res.writeHandle};
  }
  return folly::makeUnexpected(WtErrCode::STREAM_CREATION_ERROR);
}

folly::SemiFuture<folly::Unit> CoroWtSession::awaitUniStreamCredit() noexcept {
  if (sm_.canCreateUni()) {
    return folly::makeSemiFuture();
  }
  auto [p, f] = folly::makePromiseContract<folly::Unit>();
  awaitUniCredit_ = std::move(p);
  return std::move(f);
}

folly::SemiFuture<folly::Unit> CoroWtSession::awaitBidiStreamCredit() noexcept {
  if (sm_.canCreateBidi()) {
    return folly::makeSemiFuture();
  }
  auto [p, f] = folly::makePromiseContract<folly::Unit>();
  awaitBidiCredit_ = std::move(p);
  return std::move(f);
}

WtExpected<folly::SemiFuture<StreamData>>::Type CoroWtSession::readStreamData(
    StreamId id) noexcept {
  if (auto* rh = sm_.getBidiHandle(id).readHandle) {
    return rh->readStreamData();
  }
  return folly::makeUnexpected(WtErrCode::INVALID_STREAM_ID);
}

WtExpected<FCState>::Type CoroWtSession::writeStreamData(
    StreamId id,
    std::unique_ptr<folly::IOBuf> data,
    bool fin,
    ByteEventCallback* byteEventCallback) noexcept {
  if (auto* wh = sm_.getBidiHandle(id).writeHandle) {
    return wh->writeStreamData(std::move(data), fin, byteEventCallback);
  }
  return folly::makeUnexpected(WtErrCode::INVALID_STREAM_ID);
}

WtExpected<folly::Unit>::Type CoroWtSession::resetStream(
    StreamId id, uint32_t error) noexcept {
  if (auto* wh = sm_.getBidiHandle(id).writeHandle) {
    return wh->resetStream(error);
  }
  return folly::makeUnexpected(WtErrCode::INVALID_STREAM_ID);
}

WtExpected<folly::Unit>::Type CoroWtSession::setPriority(
    uint64_t streamId, quic::PriorityQueue::Priority priority) noexcept {
  return folly::unit;
}

WtExpected<folly::Unit>::Type CoroWtSession::setPriorityQueue(
    std::unique_ptr<quic::PriorityQueue>) noexcept {
  return folly::unit;
}

WtExpected<folly::SemiFuture<StreamId>>::Type CoroWtSession::awaitWritable(
    StreamId id) noexcept {
  if (auto* wh = sm_.getBidiHandle(id).writeHandle) {
    return wh->awaitWritable();
  }
  return folly::makeUnexpected(WtErrCode::INVALID_STREAM_ID);
}

WtExpected<folly::Unit>::Type CoroWtSession::stopSending(
    StreamId id, uint32_t error) noexcept {
  if (auto* rh = sm_.getBidiHandle(id).readHandle) {
    return rh->stopSending(error);
  }
  return folly::makeUnexpected(WtErrCode::INVALID_STREAM_ID);
}

WtExpected<folly::Unit>::Type CoroWtSession::sendDatagram(
    std::unique_ptr<folly::IOBuf>) noexcept {
  XLOG(FATAL) << "not implemented";
}

quic::TransportInfo CoroWtSession::getTransportInfo() const noexcept {
  return quic::TransportInfo{};
}

WtExpected<folly::Unit>::Type CoroWtSession::closeSession(
    folly::Optional<uint32_t> error) noexcept {
  cs_.requestCancellation();
  sm_.shutdown(WtStreamManager::CloseSession{.err = error.value_or(0),
                                             .msg = "closeSession"});
  return folly::unit;
}

void CoroWtSession::onBidiStreamCreditAvail() noexcept {
  if (auto p = resetPromise(awaitBidiCredit_); p.valid()) {
    p.setValue();
  }
}

void CoroWtSession::onUniStreamCreditAvail() noexcept {
  if (auto p = resetPromise(awaitUniCredit_); p.valid()) {
    p.setValue();
  }
}

folly::coro::Task<void> CoroWtSession::readLoop(Ptr self,
                                                HTTPSourceHolder ingress) {
  WtCapsuleCallback wtCapsuleCallback{sm_, *self};
  WebTransportCapsuleCodec codec{&wtCapsuleCallback, CodecVersion::H2};

  while (!sm_.isClosed() && ingress.readable()) {
    auto maybeEv = co_await co_awaitTry(ingress.readBodyEvent());
    if (maybeEv.hasException() ||
        maybeEv->eventType != HTTPBodyEvent::BODY) { // skip non-body events
      XLOG_IF(DBG4, maybeEv.hasException())
          << "::readLoop ingress ex=" << maybeEv.exception().what();
      continue;
    }
    auto& event = maybeEv.value();
    codec.onIngress(event.event.body.move(), event.eom);
    // handle any new peer streams
    auto peerIds = std::move(wtSmIngressCb_.peerStreams);
    for (auto id : peerIds) {
      auto handle = sm_.getOrCreateBidiHandle(id);
      if (handle.writeHandle) { // write handle iff bidi stream
        wtHandler_->onNewBidiStream(handle);
      } else if (handle.readHandle) {
        wtHandler_->onNewUniStream(handle.readHandle);
      }
    }

    if (event.eom) {
      break;
    }
  }
  XLOG(DBG4) << "CoroWtSession::readLoop exiting";
  //  http/2 rst, eom or WtStreamManager closed – in either case invoke
  //  ::shutdown to exit write loop
  sm_.shutdown(WtStreamManager::CloseSession{
      .err = 0x00, .msg = "h2 stream ingress closed"});
  readLoopFinished();
}

folly::coro::Task<void> CoroWtSession::writeLoop(Ptr self,
                                                 EgressSourcePtr egress) {
  const auto timeout = egress->getReadTimeout();
  const folly::IOBuf empty;
  folly::IOBufQueue egressBuf{folly::IOBufQueue::cacheChainLength()};
  detail::WtEventVisitor eventVisitor{.egress = egressBuf};
  detail::EgressBackPressure streamSourceCallback;
  egress->setCallback(&streamSourceCallback);
  auto& waitForEventBaton = wtSmEgressCb_.waitForEvent;

  while (!eventVisitor.sessionClosed) {
    // wait for WtSession egress (i.e. underlying http/2 egress buffer space);
    // this is upperbounded by writeTimeout in HTTPBodyEventQueue
    XLOG(DBG6) << "waiting for http/2 egress fc";
    co_await streamSourceCallback.waitForEgress.wait();

    // wait for underlying wt ctrl events or writable streams
    XLOG(DBG6) << "waiting for WtStreamManager event";
    auto res = co_await waitForEventBaton.timedWait(evb_, timeout);
    if (res == TimedBaton::Status::timedout) {
      sm_.shutdown(WtStreamManager::CloseSession{.err = 0x00,
                                                 .msg = "wt write timed out"});
    } // fallthru to writing close_session below
    waitForEventBaton.reset();

    XLOG(DBG6) << "received WtStreamManager event";
    // always write control frames first (not subject to flow control)
    auto ctrl = sm_.moveEvents();
    for (auto& ev : ctrl) {
      std::visit(eventVisitor, ev);
    }
    egress->body(egressBuf.move(), /*padding=*/0, /*eom=*/false);

    // write stream data
    auto* wh = sm_.nextWritable();
    while (wh) {
      auto bytesAvailable = egress->window().getNonNegativeSize();
      XLOG(DBG4) << __func__ << "; id=" << wh->getID() << "; wh=" << wh
                 << "; bytesAvailable=" << bytesAvailable;
      if (bytesAvailable == 0) {
        XLOG(DBG5) << __func__ << " egress blocked";
        streamSourceCallback.waitForEgress.reset(); // block on next loop
        break;
      }
      auto id = wh->getID();
      auto dequeue = sm_.dequeue(*wh, /*atMost=*/bytesAvailable);
      writeWTStream(egressBuf,
                    WTStreamCapsule{.streamId = id,
                                    .streamData = std::move(dequeue.data),
                                    .fin = dequeue.fin});
      // ::body overflow checked in next iteration
      egress->body(egressBuf.move(), /*padding=*/0, /*eom=*/false);
      wh = sm_.nextWritable();
    }

    if (wh) { // if there's more pending data to be written, signal baton
      waitForEventBaton.signal();
    }
  }

  XLOG(DBG4) << "CoroWtSession::writeLoop exiting";
  egress->eom();
  writeLoopFinished();
  co_return;
}

void CoroWtSession::start(CoroWtSession::Ptr self,
                          HTTPSourceHolder&& ingress,
                          EgressSourcePtr&& egress) {
  wtHandler_->onWebTransportSession(self);
  auto ct = cs_.getToken();
  co_withExecutor(evb_,
                  co_withCancellation(ct, readLoop(self, std::move(ingress))))
      .start();
  co_withExecutor(evb_,
                  co_withCancellation(ct, writeLoop(self, std::move(egress))))
      .start();
}

void CoroWtSession::writeLoopFinished() noexcept {
  writeLoopDone_ = true;
  if (readLoopDone_) {
    std::exchange(wtHandler_, nullptr)->onSessionEnd(folly::none);
  }
}
void CoroWtSession::readLoopFinished() noexcept {
  readLoopDone_ = true;
  if (writeLoopDone_) {
    std::exchange(wtHandler_, nullptr)->onSessionEnd(folly::none);
  }
}

/**
 * when a producer wants to detach the ownership of the HTTPStreamSource, this
 * function transfers lifetime to consumer (i.e. self owned)
 */
void EgressSourcePtrDeleter::operator()(EgressSource* source) noexcept {
  if (!source->sourceComplete()) {
    source->setHeapAllocated();
    source->setCallback(nullptr);
    source->abort(HTTPErrorCode::CANCEL);
    return;
  }
  std::default_delete<HTTPStreamSource>{}(source);
}

} // namespace proxygen::coro::detail
