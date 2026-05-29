/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/webtransport/QuicWtSession.h>

#include <proxygen/lib/http/codec/HQFramer.h>

using namespace proxygen;
using namespace proxygen::detail;
using FCState = WebTransport::FCState;

namespace {
static constexpr uint64_t kMaxWtIngressBuf = 65'535;

WtStreamManager::WtConfig createQuicConfig() {
  WtStreamManager::WtConfig config;
  config.selfMaxStreamsBidi = kMaxVarint;
  config.selfMaxStreamsUni = kMaxVarint;
  config.selfMaxConnData = kMaxVarint;
  config.selfMaxStreamDataBidi = kMaxWtIngressBuf;
  config.selfMaxStreamDataUni = kMaxWtIngressBuf;
  config.peerMaxStreamsBidi = kMaxVarint;
  config.peerMaxStreamsUni = kMaxVarint;
  config.peerMaxConnData = kMaxVarint;
  config.peerMaxStreamDataBidi = kMaxVarint;
  config.peerMaxStreamDataUni = kMaxVarint;
  return config;
}

struct QuicWtEventVisitor {
  quic::QuicSocket& quicSocket;
  H3ConnectStreamCallback* observer{nullptr};

  // operations map to QuicSocket invocations
  void operator()(WtStreamManager::ResetStream ev) const {
    quicSocket.resetStream(ev.streamId, ev.err);
  }

  void operator()(WtStreamManager::StopSending ev) const {
    quicSocket.setReadCallback(ev.streamId, nullptr, ev.err);
  }

  // operations need to be serialized on the backing http/3 connect stream (if
  // exists).
  void operator()(WtStreamManager::CloseSession ev) const {
    if (observer) {
      observer->onEvent(std::move(ev));
    }
  }

  void operator()(WtStreamManager::MaxConnData ev) const {
    if (observer) {
      observer->onEvent(std::move(ev));
    }
  }
  void operator()(WtStreamManager::MaxStreamsBidi ev) const {
    if (observer) {
      observer->onEvent(std::move(ev));
    }
  }
  void operator()(WtStreamManager::MaxStreamsUni ev) const {
    if (observer) {
      observer->onEvent(std::move(ev));
    }
  }
  void operator()(WtStreamManager::DrainSession ev) const {
    if (observer) {
      observer->onEvent(std::move(ev));
    }
  }

  // not applicable to http/3-wt or quic-wt
  void operator()(WtStreamManager::MaxStreamData /*ev*/) const {
  }
};

uint64_t getQuicAppErrCode(const QuicError& err) noexcept {
  auto* appEc = err.code.asApplicationErrorCode();
  return appEc ? *appEc : 0;
}

} // namespace

namespace proxygen {

QuicWtSessionBase::QuicWtSessionBase(
    std::shared_ptr<quic::QuicSocket> quicSocket,
    WtHandlerPtr wtHandler,
    WtStreamManager::WtConfig wtConfig,
    H3ConnectStreamCallback* observer)
    : WtSessionBase(nullptr, sm_),
      quicSocket_(std::move(quicSocket)),
      wtHandler_(std::move(wtHandler)),
      priorityQueue_(std::make_unique<quic::HTTPPriorityQueue>()),
      sm_{quicSocket_->getNodeType() == quic::QuicNodeType::Server
              ? detail::WtDir::Server
              : detail::WtDir::Client,
          wtConfig,
          smCb_,
          smCb_,
          *priorityQueue_},
      observer_(observer) {
}

QuicWtSessionBase::~QuicWtSessionBase() {
  QuicWtSessionBase::closeSession(folly::none);
}

auto QuicWtSessionBase::createWtEgressHandle(StreamId id) noexcept
    -> BidiStreamHandle {
  // bidi or uni type deduced by WtStreamManager from id
  auto res = sm_.getOrCreateBidiHandle(id);
  const bool success = res.writeHandle;
  const bool bidi = success && res.readHandle;
  // canCreate(Uni|Bidi) checked in ::create(Uni|Bidi)Stream
  XCHECK(success);
  quicSocket_->setStopSendingCallback(id, &stopSendingCb_);
  if (bidi) {
    sm_.setReadCb(*res.readHandle, &smCb_);
    quicSocket_->setReadCallback(id, &readCb_);
  }
  return res;
}

bool QuicWtSessionBase::hasEgressUniCredit() const noexcept {
  return quicSocket_->getNumOpenableUnidirectionalStreams() &&
         sm_.canCreateUni();
}
bool QuicWtSessionBase::hasEgressBidiCredit() const noexcept {
  return quicSocket_->getNumOpenableBidirectionalStreams() &&
         sm_.canCreateBidi();
}

folly::Expected<WebTransport::StreamWriteHandle*, WebTransport::ErrorCode>
QuicWtSessionBase::createUniStream() noexcept {
  XCHECK(quicSocket_);
  if (!hasEgressUniCredit()) {
    return folly::makeUnexpected(ErrorCode::STREAM_CREATION_ERROR);
  }
  auto id = quicSocket_->createUnidirectionalStream();
  XCHECK(id);
  return CHECK_NOTNULL(createWtEgressHandle(*id).writeHandle);
}

folly::Expected<WebTransport::BidiStreamHandle, WebTransport::ErrorCode>
QuicWtSessionBase::createBidiStream() noexcept {
  XCHECK(quicSocket_);
  if (!hasEgressBidiCredit()) {
    return folly::makeUnexpected(ErrorCode::STREAM_CREATION_ERROR);
  }
  auto id = quicSocket_->createBidirectionalStream();
  XCHECK(id);
  auto res = createWtEgressHandle(*id);
  XCHECK(res.readHandle && res.writeHandle);
  return res;
}

folly::SemiFuture<folly::Unit>
QuicWtSessionBase::awaitUniStreamCredit() noexcept {
  XCHECK(quicSocket_);
  if (hasEgressUniCredit()) {
    return folly::makeFuture(folly::unit);
  }
  auto [promise, future] = folly::makePromiseContract<folly::Unit>();
  uniCreditPromise() = std::move(promise);
  return std::move(future);
}

folly::SemiFuture<folly::Unit>
QuicWtSessionBase::awaitBidiStreamCredit() noexcept {
  XCHECK(quicSocket_);
  if (hasEgressBidiCredit()) {
    return folly::makeFuture(folly::unit);
  }
  auto [promise, future] = folly::makePromiseContract<folly::Unit>();
  bidiCreditPromise() = std::move(promise);
  return std::move(future);
}

folly::Expected<folly::Unit, WebTransport::ErrorCode>
QuicWtSessionBase::sendDatagram(IoBufPtr datagram) noexcept {
  XCHECK(quicSocket_);
  auto writeRes = quicSocket_->writeDatagram(std::move(datagram));
  if (writeRes.hasError()) {
    XLOG(ERR) << __func__ << "; err= " << writeRes.error();
    return folly::makeUnexpected(ErrorCode::GENERIC_ERROR);
  }
  return folly::unit;
}

folly::Expected<folly::Unit, WebTransport::ErrorCode>
QuicWtSessionBase::closeSession(folly::Optional<uint32_t> error) noexcept {
  sm_.shutdown({.err = error.value_or(0), .msg = "closeSession"});
  if (auto wtHandler = std::exchange(wtHandler_, nullptr)) {
    wtHandler->onSessionEnd(error);
  }
  return folly::unit;
}

void QuicWtSessionBase::onDatagram(IoBufPtr dgram) noexcept {
  if (wtHandler_) {
    wtHandler_->onDatagram(std::move(dgram));
  }
}

// -- QuicReadCallback overrides --
void QuicWtSessionBase::QuicReadCallback::readAvailable(StreamId id) noexcept {
  XCHECK(sess.quicSocket_);
  auto& sm = sess.sm_;
  auto& quicSocket = sess.quicSocket_;

  auto* rh = sm.getBidiHandle(id).readHandle;
  if (!rh) {
    XLOG(ERR) << "nullptr rh id=" << id;
    return;
  }
  const auto canRead = sm.recvBytesAvail(*rh);
  if (canRead == 0) {
    sess.maybePauseIngress(*rh);
    return;
  }
  auto readRes = quicSocket->read(id, canRead);
  if (readRes.hasError()) {
    XLOG(ERR) << "::read err id=" << id;
    return;
  }
  auto& [data, eof] = readRes.value();
  auto res = sm.enqueue(
      *rh, WebTransport::StreamData{.data = std::move(data), .fin = eof});
  XCHECK_NE(res, detail::WtStreamManager::Result::Fail);
  if (!eof) { // ::enqueue w/ eof=true may deallocate rh
    sess.maybePauseIngress(*rh);
  }
}

void QuicWtSessionBase::QuicReadCallback::readError(StreamId id,
                                                    QuicError error) noexcept {
  XLOG(ERR) << __func__ << "; id=" << id << "; err=" << error;
  sess.sm_.onResetStream(detail::WtStreamManager::ResetStream{
      id, *error.code.asApplicationErrorCode()});
}

void QuicWtSessionBase::QuicStopSendingCallback::onStopSending(
    StreamId id, quic::ApplicationErrorCode ec) noexcept {
  sess.sm_.onStopSending(detail::WtStreamManager::StopSending{id, ec});
}

// -- StreamManagerCallback overrides --
void QuicWtSessionBase::StreamManagerCallback::readReady(
    detail::WtStreamManager::WtReadHandle& rh) noexcept {
  sess.maybeResumeIngress(rh);
}

void QuicWtSessionBase::StreamManagerCallback::eventsAvailable() noexcept {
  XCHECK(sess.quicSocket_);
  // process control events first
  QuicWtEventVisitor visitor{*sess.quicSocket_, sess.observer_};
  auto events = sess.sm_.moveEvents();
  for (auto& event : events) {
    std::visit(visitor, event);
  }
  // then process writable streams
  while (!sess.priorityQueue_->empty()) {
    auto id = sess.priorityQueue_->getNextScheduledID(std::nullopt);
    if (!id.isStreamID()) { // skip datagrams
      break;
    }
    auto streamId = id.asStreamID();
    auto maxData = sess.quicSocket_->getMaxWritableOnStream(streamId);
    auto* wh = sess.sm_.getBidiHandle(streamId).writeHandle;
    if (!wh || !maxData) {
      XLOG(DBG4) << "nullptr wh id=" << streamId;
      sess.priorityQueue_->erase(id);
      continue;
    }
    if (*maxData == 0) {
      XLOG(DBG4) << "egress conn-fc blocked id=" << streamId;
      sess.priorityQueue_->erase(id);
      sess.quicSocket_->notifyPendingWriteOnStream(streamId, &sess);
      continue;
    }
    auto streamData = sess.sm_.dequeue(*wh, *maxData);
    if (streamData.data || streamData.fin) {
      auto res = sess.quicSocket_->writeChain(streamId,
                                              std::move(streamData.data),
                                              streamData.fin,
                                              streamData.deliveryCallback);
      if (res.hasError()) {
        XLOG(ERR) << "QuicSocket::writeChain err id=" << streamId;
        wh->resetStream(WebTransport::kInternalError);
      }
    }
  }
}

void QuicWtSessionBase::StreamManagerCallback::onNewPeerStream(
    uint64_t /*streamId*/) noexcept {
}

// -- StreamWriteCallback overrides --
void QuicWtSessionBase::onStreamWriteReady(quic::StreamId streamId,
                                           uint64_t /*maxToSend*/) noexcept {
  auto* wh = sm_.getBidiHandle(streamId).writeHandle;
  if (wh) {
    priorityQueue_->insertOrUpdate(
        quic::PriorityQueue::Identifier::fromStreamID(wh->getID()),
        wh->getPriority());
    smCb_.eventsAvailable();
  }
}

void QuicWtSessionBase::onStreamWriteError(quic::StreamId id,
                                           QuicError error) noexcept {
  XLOG(ERR) << __func__ << "; id=" << id << "; err=" << error;
  if (auto* wh = sm_.getBidiHandle(id).writeHandle) {
    wh->resetStream(WebTransport::kInternalError);
  }
}

void QuicWtSessionBase::maybePauseIngress(
    detail::WtStreamManager::WtReadHandle& handle) noexcept {
  XCHECK(quicSocket_);
  const auto id = handle.getID();
  if (sm_.recvBytesAvail(handle) == 0) {
    XLOG(DBG4) << __func__ << "; id=" << id;
    auto res = quicSocket_->pauseRead(id);
    XLOG_IF(ERR, res.hasError()) << __func__ << "; err id=" << id;
  }
}

void QuicWtSessionBase::maybeResumeIngress(
    detail::WtStreamManager::WtReadHandle& handle) noexcept {
  XCHECK(quicSocket_);
  const auto id = handle.getID();
  if (sm_.recvBytesAvail(handle) > 0) {
    XLOG(DBG4) << __func__ << "; id=" << id;
    auto res = quicSocket_->resumeRead(id);
    XLOG_IF(ERR, res.hasError()) << __func__ << "; err id=" << id;
  }
}

bool QuicWtSessionBase::acquireIngressStream(uint64_t id) noexcept {
  XCHECK(quicSocket_ && quic::isRemoteStream(quicSocket_->getNodeType(), id));
  XCHECK(wtHandler_);
  // WtStreamManager deduces type from id (i.e. works whether uni or bidi)
  auto handle = sm_.getOrCreateBidiHandle(id);
  const bool success = handle.readHandle;
  const bool bidi = success && handle.writeHandle;
  if (success) {
    sm_.setReadCb(*handle.readHandle, &smCb_);
    quicSocket_->setReadCallback(id, &readCb_);
    if (bidi) {
      quicSocket_->setStopSendingCallback(id, &stopSendingCb_);
      wtHandler_->onNewBidiStream(handle);
    } else {
      wtHandler_->onNewUniStream(handle.readHandle);
    }
  }
  return success;
}

/**
 * QuicWtSession implementation below. Most of the functionality is shared with
 * QuicWtSessionBase - however this derived class assumes ownership of all of
 * QuicSocket's streams, so it owns and installs a
 * QuicConnectionCallback. Similarly the destructor/::closeSession closes all
 * streams on the underlying QuicSocket via QuicSocket::close.
 */
QuicWtSession::QuicWtSession(std::shared_ptr<quic::QuicSocket> quicSocket,
                             std::unique_ptr<WebTransportHandler> wtHandler)
    : QuicWtSession(
          std::move(quicSocket),
          WtHandlerPtr{wtHandler.release(), WtHandlerDeleter{.owning = true}}) {
}

QuicWtSession::QuicWtSession(std::shared_ptr<quic::QuicSocket> quicSocket,
                             WebTransportHandler* wtHandler)
    : QuicWtSession(
          std::move(quicSocket),
          WtHandlerPtr{wtHandler, WtHandlerDeleter{.owning = false}}) {
}

QuicWtSession::QuicWtSession(std::shared_ptr<quic::QuicSocket> quicSocket,
                             WtHandlerPtr wtHandler)
    : QuicWtSessionBase(
          std::move(quicSocket), std::move(wtHandler), createQuicConfig()) {
  quicSocket_->setConnectionCallback(&connCb_);
  quicSocket_->setDatagramCallback(&dgramCb_);
}

QuicWtSession::~QuicWtSession() {
  QuicWtSession::closeSession(folly::none);
}

folly::Expected<folly::Unit, WebTransport::ErrorCode>
QuicWtSession::closeSession(folly::Optional<uint32_t> error) noexcept {
  QuicWtSessionBase::closeSession(error);
  quicSocket_->setConnectionCallback(nullptr);
  quicSocket_->setDatagramCallback(nullptr);
  quicSocket_->close(QuicError(quic::ApplicationErrorCode(error.value_or(0))));
  return folly::unit;
}

// -- QuicConnectionCallback overrides --
void QuicWtSession::QuicConnectionCallback::onNewBidirectionalStream(
    StreamId id) noexcept {
  // always expected to succeed as config.peerMaxStreamsBidi = inf
  XCHECK(sess.acquireIngressStream(id));
}

void QuicWtSession::QuicConnectionCallback::onNewUnidirectionalStream(
    StreamId id) noexcept {
  // always expected to succeed as config.peerMaxStreamsUni = inf
  XCHECK(sess.acquireIngressStream(id));
}

void QuicWtSession::QuicConnectionCallback::onConnectionEnd() noexcept {
  sess.closeSession(folly::none);
}

void QuicWtSession::QuicConnectionCallback::onConnectionEnd(
    QuicError error) noexcept {
  sess.closeSession(getQuicAppErrCode(error));
}

void QuicWtSession::QuicConnectionCallback::onConnectionError(
    QuicError error) noexcept {
  sess.closeSession(getQuicAppErrCode(error));
}

void QuicWtSession::QuicConnectionCallback::onBidirectionalStreamsAvailable(
    uint64_t numStreamsAvailable) noexcept {
  if (numStreamsAvailable > 0) {
    sess.onBidiStreamCreditAvail();
  }
}

void QuicWtSession::QuicConnectionCallback::onUnidirectionalStreamsAvailable(
    uint64_t numStreamsAvailable) noexcept {
  if (numStreamsAvailable > 0) {
    sess.onUniStreamCreditAvail();
  }
}

void QuicWtSession::QuicDgramCallback::onDatagramsAvailable() noexcept {
  XCHECK(sess.quicSocket_);
  auto& quicSocket = sess.quicSocket_;

  auto result = quicSocket->readDatagramBufs();
  if (result.hasError()) {
    XLOG(ERR) << __func__ << "; err=" << toString(result.error());
    sess.closeSession(WebTransport::kInternalError);
    return;
  }

  XLOG(DBG4) << "rx nDatagrams=" << result->size();
  for (auto& dgram : result.value()) {
    sess.onDatagram(std::move(dgram));
  }
}

H3WtSession::H3WtSession(std::shared_ptr<quic::QuicSocket> quicSocket,
                         std::unique_ptr<WebTransportHandler> wtHandler,
                         WtStreamManager::WtConfig wtConfig,
                         uint64_t connectStreamId,
                         H3ConnectStreamCallback& observer) noexcept
    : QuicWtSessionBase(
          std::move(quicSocket),
          WtHandlerPtr{wtHandler.release(), WtHandlerDeleter{.owning = true}},
          std::move(wtConfig),
          &observer),
      connectStreamId_(connectStreamId) {
  XCHECK_LE(connectStreamId, detail::kMaxVarint);
}

H3WtSession::~H3WtSession() noexcept {
  H3WtSession::closeSession(folly::none);
}

folly::Expected<folly::Unit, WebTransport::ErrorCode> H3WtSession::closeSession(
    folly::Optional<uint32_t> error) noexcept {
  // we need to bidi reset all assoc quic streams (ss+rst_stream)
  auto streamIds = sm_.streamIds();
  auto ec = error.value_or(0);
  // bidirectionally reset all assoc quic streams
  for (uint64_t id : streamIds) {
    quicSocket_->setReadCallback(id, nullptr, ec);
    quicSocket_->resetStream(id, ec);
  }
  QuicWtSessionBase::closeSession(error);
  return folly::unit;
}

folly::Expected<WebTransport::StreamWriteHandle*, WebTransport::ErrorCode>
H3WtSession::createUniStream() noexcept {
  auto result = QuicWtSessionBase::createUniStream();
  if (result) {
    writeWtFramePrefix(result.value()->getID());
  }
  return result;
}

folly::Expected<WebTransport::BidiStreamHandle, WebTransport::ErrorCode>
H3WtSession::createBidiStream() noexcept {
  auto result = QuicWtSessionBase::createBidiStream();
  if (result) {
    writeWtFramePrefix(result->writeHandle->getID());
  }
  return result;
}

void H3WtSession::writeWtFramePrefix(uint64_t id) noexcept {
  auto streamType = quic::isBidirectionalStream(id)
                        ? hq::WebTransportStreamType::BIDI
                        : hq::WebTransportStreamType::UNI;
  folly::IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
  XCHECK(hq::writeWTStreamPreface(writeBuf, streamType, connectStreamId_));
  XCHECK(quicSocket_->writeChain(id, writeBuf.move(), false));
}

auto H3WtSession::onConnMaxData(WtStreamManager::MaxConnData mcd) noexcept
    -> WtSmResult {
  return sm_.onMaxData(mcd);
}
auto H3WtSession::onMaxStreams(WtStreamManager::MaxStreamsUni ms) noexcept
    -> WtSmResult {
  return sm_.onMaxStreams(ms);
}
auto H3WtSession::onMaxStreams(WtStreamManager::MaxStreamsBidi ms) noexcept
    -> WtSmResult {
  return sm_.onMaxStreams(ms);
}
void H3WtSession::onDrainSession(WtStreamManager::DrainSession ds) noexcept {
  return sm_.onDrainSession(ds);
}
void H3WtSession::onCloseSession(WtStreamManager::CloseSession&& cs) noexcept {
  return sm_.onCloseSession(std::move(cs));
}

} // namespace proxygen
