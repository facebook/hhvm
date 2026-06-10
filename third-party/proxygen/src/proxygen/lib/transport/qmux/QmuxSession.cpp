/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/transport/qmux/QmuxSession.h>

#include <folly/io/coro/Transport.h>
#include <folly/logging/xlog.h>
#include <proxygen/lib/transport/qmux/QmuxCodec.h>

namespace {
using namespace proxygen::qmux;

constexpr uint64_t kStreamFrameOverhead = 17;
constexpr uint64_t kTargetTransportWriteBytes = 65'535;

void flushRecord(folly::IOBufQueue& recordBuf, folly::IOBufQueue& egressBuf) {
  if (!recordBuf.empty()) {
    writeRecord(egressBuf, recordBuf.move());
  }
}

bool appendFrameToRecord(folly::IOBufQueue& recordBuf,
                         folly::IOBufQueue& egressBuf,
                         folly::IOBufQueue& frameBuf,
                         uint64_t recordPayloadLimit) {
  if (frameBuf.empty()) {
    return true;
  }

  const auto frameSize = frameBuf.chainLength();
  if (frameSize > recordPayloadLimit) {
    XLOG(ERR) << "QMUX frame exceeds peer max_record_size frameSize="
              << frameSize << " peerMaxRecordSize=" << recordPayloadLimit;
    return false;
  }

  if (recordBuf.chainLength() + frameSize > recordPayloadLimit) {
    flushRecord(recordBuf, egressBuf);
  }
  recordBuf.append(frameBuf.move());
  return true;
}

//////// QmuxEventVisitor ////////

// Inherits shared event→frame serialization from WtEventVisitor, overrides
// only the two events that differ: DrainSession (no-op) and CloseSession
// (writes QxConnectionClose instead of WT capsule).
struct QmuxEventVisitor : proxygen::detail::WtEventVisitor {
  using WtEventVisitor::operator();

  void operator()(WtStreamManager::DrainSession) const noexcept {
    XLOG(DBG6) << "QmuxEventVisitor DrainSession (no-op)";
  }

  void operator()(WtStreamManager::CloseSession cs) noexcept {
    XLOG(DBG6) << "QmuxEventVisitor cs.err=" << cs.err << " cs.msg=" << cs.msg;
    sessionClosed = true;
    writeConnectionClose(egress,
                         QxConnectionClose{.errorCode = cs.err,
                                           .frameType = 0,
                                           .isAppError = true,
                                           .reasonPhrase = std::move(cs.msg)});
  }
};

} // namespace

namespace proxygen::qmux {

//////// QmuxCallback ////////

// Extends QmuxCodec::Callback (which inherits WtCapsuleCallback's shared
// frame→WtStreamManager bridging). Only adds QMUX-specific callbacks and
// overrides onConnectionError.
class QmuxCallback : public QmuxCodec::Callback {
 public:
  QmuxCallback(WtStreamManager& sm,
               proxygen::detail::WtSessionBase& wtSess,
               QmuxSession& session)
      : QmuxCodec::Callback(sm, wtSess), session_(session) {
  }

  // QMUX-specific callbacks
  void onConnectionClose(QxConnectionClose c) noexcept override {
    XLOG(DBG6) << __func__ << "; err=" << c.errorCode
               << "; reason=" << c.reasonPhrase;
    sm_.onCloseSession(
        WtStreamManager::CloseSession{.err = static_cast<uint64_t>(c.errorCode),
                                      .msg = std::move(c.reasonPhrase)});
  }

  // QmuxConnector consumes the initial transport parameters before QmuxSession
  // runs, so any TP frame reaching the steady-state codec is a peer protocol
  // violation.
  void onTransportParameters(QxTransportParams) noexcept override {
    XLOG(DFATAL) << "unexpected QX_TRANSPORT_PARAMETERS after handshake";
  }

  void onPing(QxPing p) noexcept override {
    XLOG(DBG6) << __func__ << "; seq=" << p.sequenceNumber;
    session_.pendingPongs_.emplace_back(p);
    session_.wtSmEgressCb.waitForEvent.signal();
  }

  void onPong(QxPing p) noexcept override {
    XLOG(DBG6) << __func__ << "; seq=" << p.sequenceNumber;
  }

  void onConnectionError(QmuxErrorCode err) noexcept override {
    XLOG(DBG6) << __func__ << "; err=" << static_cast<uint32_t>(err);
    sm_.onCloseSession(
        WtStreamManager::CloseSession{.err = 0, .msg = "onConnectionError"});
  }

  // Override the inherited buffering implementation: deliver inbound datagrams
  // straight to the application handler. Per QMUX spec, datagrams MAY be
  // dropped when they cannot be promptly delivered, so dropping when no
  // handler is attached is acceptable.
  void onDatagram(DatagramCapsule dgram) noexcept override {
    XLOG(DBG6) << __func__;
    if (auto* handler = session_.wtHandler_) {
      handler->onDatagram(std::move(dgram.httpDatagramPayload));
    }
  }

 private:
  QmuxSession& session_;
};

//////// QmuxSession ////////

QmuxSession::QmuxSession(folly::EventBase* evb,
                         WtDir dir,
                         QxTransportParams selfParams,
                         std::unique_ptr<folly::coro::TransportIf> transport,
                         WtStreamManager::WtConfig wtConfig,
                         uint64_t peerMaxRecordSize,
                         uint64_t effectiveMaxIdleTimeoutMs,
                         std::unique_ptr<folly::IOBuf> initialIngress,
                         Config config)
    : CoroWtSessionBase(dir, wtConfig),
      WtSessionBase(evb, sm),
      localAddr_(transport->getLocalAddress()),
      peerAddr_(transport->getPeerAddress()),
      transport_(std::move(transport)),
      selfParams_(std::move(selfParams)),
      effectiveMaxIdleTimeoutMs_(effectiveMaxIdleTimeoutMs),
      initialIngress_(std::move(initialIngress)),
      peerMaxRecordSize_(peerMaxRecordSize),
      config_(config) {
}

QmuxSession::~QmuxSession() {
  cs_.requestCancellation();
}

proxygen::detail::WtExpected<folly::Unit>::Type QmuxSession::closeSession(
    folly::Optional<uint32_t> error) noexcept {
  cs_.requestCancellation();
  sm.shutdown(WtStreamManager::CloseSession{.err = error.value_or(0),
                                            .msg = "closeSession"});
  return folly::unit;
}

proxygen::detail::WtExpected<folly::Unit>::Type QmuxSession::sendDatagram(
    std::unique_ptr<folly::IOBuf> datagram) noexcept {
  pendingDatagrams_.emplace_back(std::move(datagram));
  wtSmEgressCb.waitForEvent.signal();
  return folly::unit;
}

void QmuxSession::start(Ptr self) {
  XLOG(DBG4) << "QmuxSession::start dir=" << (peerAddr_.describe());
  if (wtHandler_) {
    wtHandler_->onWebTransportSession(self);
  }
  auto ct = cs_.getToken();
  auto* eventBase = evb();
  co_withExecutor(eventBase, co_withCancellation(ct, readLoop(self))).start();
  co_withExecutor(eventBase, co_withCancellation(ct, writeLoop(self))).start();
}

folly::coro::Task<void> QmuxSession::readLoop(Ptr self) {
  XLOG(DBG4) << "QmuxSession::readLoop started dir=" << peerAddr_.describe();
  QmuxCallback qmuxCallback{sm, *self, *self};
  QmuxCodec codec{&qmuxCallback,
                  [&sm = this->sm](uint64_t streamId, uint64_t offset) {
                    auto* rh = sm.getIngressHandle(streamId);
                    return rh && offset == sm.streamBytesReceived(*rh);
                  }};
  codec.setMaxRecordSize(self->selfParams_.maxRecordSize);
  folly::IOBufQueue ingressBuf{folly::IOBufQueue::cacheChainLength()};

  resetIdleTimeout();

  while (!sm.isClosed()) {
    bool eom = false;
    if (initialIngress_) {
      // First iteration: drain bytes the connector read past the TP frame
      // (any in-record trailers it rewrapped, plus subsequent records).
      codec.onIngress(std::move(initialIngress_));
    } else {
      // minReadSize and newAllocationSize copied from CoroWtSession::readLoop.
      // timeout=0 = wait indefinitely; idleTimeout_ enforces the deadline.
      auto readRes = co_await co_awaitTry(
          transport_->read(ingressBuf,
                           /*minReadSize=*/1460,
                           /*newAllocationSize=*/4000,
                           /*timeout=*/std::chrono::milliseconds(0)));
      if (readRes.hasException()) {
        XLOG(DBG4) << __func__ << "; ex=" << readRes.exception();
        break;
      }
      eom = (*readRes == 0);
      codec.onIngress(ingressBuf.move());
      resetIdleTimeout();
    }

    if (wtHandler_) {
      detail::NotifyPeerStreamsGuard notify{wtSmIngressCb, sm, *wtHandler_};
    }

    if (eom) {
      break;
    }
  }
  idleTimeout_.cancelTimeout();
  XLOG(DBG4) << "QmuxSession::readLoop exiting";
  sm.shutdown(WtStreamManager::CloseSession{.err = 0x00,
                                            .msg = "stream ingress closed"});
  readLoopFinished();
}

void QmuxSession::resetIdleTimeout() {
  if (effectiveMaxIdleTimeoutMs_ == 0) {
    // Neither endpoint advertised a non-zero max_idle_timeout, so the
    // connection has no idle deadline. Nothing to do.
    return;
  }
  idleTimeout_.cancelTimeout();
  evb()->timer().scheduleTimeout(
      &idleTimeout_, std::chrono::milliseconds(effectiveMaxIdleTimeoutMs_));
}

void QmuxSession::onIdleTimeout() {
  XLOG(DBG4) << "QmuxSession::onIdleTimeout sess=" << this
             << " effectiveMaxIdleTimeoutMs=" << effectiveMaxIdleTimeoutMs_;
  cs_.requestCancellation();
  sm.shutdown(WtStreamManager::CloseSession{.err = 0, .msg = "idle timeout"});
}

folly::coro::Task<void> QmuxSession::writeLoop(Ptr self) {
  XLOG(DBG4) << "QmuxSession::writeLoop started dir=" << peerAddr_.describe();
  auto& waitForEventBaton = wtSmEgressCb.waitForEvent;
  bool sessionClosed{false};

  // sessionClosed flips to true once the visitor has serialized a
  // QxConnectionClose for sm's CloseSession event.
  while (!sessionClosed) {
    XLOG(DBG6) << "waiting for WtStreamManager event";
    co_await waitForEventBaton.wait();
    waitForEventBaton.reset();

    // Cap each record's payload so it fits within the peer's
    // max_record_size.
    const auto recordPayloadLimit = self->peerMaxRecordSize_;
    folly::IOBufQueue recordBuf{folly::IOBufQueue::cacheChainLength()};
    folly::IOBufQueue egressBuf{folly::IOBufQueue::cacheChainLength()};

    XLOG(DBG6) << "received WtStreamManager event";
    // Always write control frames first
    auto ctrl = sm.moveEvents();
    bool writeFrameError{false};
    for (auto& ev : ctrl) {
      folly::IOBufQueue frameBuf{folly::IOBufQueue::cacheChainLength()};
      QmuxEventVisitor eventVisitor{
          {.egress = frameBuf, .protocol = FrameProtocol::QMUX}};
      std::visit(eventVisitor, ev);
      sessionClosed |= eventVisitor.sessionClosed;
      if (!appendFrameToRecord(
              recordBuf, egressBuf, frameBuf, recordPayloadLimit)) {
        writeFrameError = true;
        break;
      }
    }

    if (!writeFrameError) {
      for (const auto& pong : pendingPongs_) {
        folly::IOBufQueue frameBuf{folly::IOBufQueue::cacheChainLength()};
        writePong(frameBuf, pong);
        if (!appendFrameToRecord(
                recordBuf, egressBuf, frameBuf, recordPayloadLimit)) {
          writeFrameError = true;
          break;
        }
      }
      pendingPongs_.clear();
    }

    if (writeFrameError) {
      sm.onCloseSession(WtStreamManager::CloseSession{
          .err = static_cast<uint64_t>(QmuxErrorCode::FRAME_ENCODING_ERROR),
          .msg = "frame exceeds peer max_record_size"});
      continue;
    }

    // Cap data per STREAM frame so the whole frame fits within peer's
    // max_record_size, leaving room for STREAM frame header overhead.
    const auto maxStreamData = recordPayloadLimit > kStreamFrameOverhead
                                   ? recordPayloadLimit - kStreamFrameOverhead
                                   : 0;
    auto* wh = sm.nextWritable();
    XCHECK(!(wh && maxStreamData == 0))
        << "peer max_record_size too small. The framer rejects undersized "
           "max_record_size at TP parse time";

    while (wh) {
      if (recordBuf.empty() && !egressBuf.empty() &&
          egressBuf.chainLength() >= kTargetTransportWriteBytes) {
        break;
      }
      if (recordBuf.chainLength() + kStreamFrameOverhead >=
          recordPayloadLimit) {
        flushRecord(recordBuf, egressBuf);
        continue;
      }

      auto id = wh->getID();
      const auto recordBytes = recordBuf.chainLength();
      const auto atMost =
          std::min(recordPayloadLimit - recordBytes - kStreamFrameOverhead,
                   maxStreamData);
      auto dequeue = sm.dequeue(*wh, /*atMost=*/atMost);
      writeWTStream(recordBuf,
                    WTStreamCapsule{.streamId = id,
                                    .streamData = std::move(dequeue.data),
                                    .fin = dequeue.fin},
                    FrameProtocol::QMUX);
      wh = sm.nextWritable();
    }
    flushRecord(recordBuf, egressBuf);
    if (wh) {
      // Loop hit the transport-write byte target with writable streams still
      // pending.
      // The stream manager won't ring the bell for state it already signalled,
      // so re-arm the baton ourselves to drain the remainder next iteration
      // instead of parking with sendable bytes queued.
      waitForEventBaton.signal();
    }

    while (!pendingDatagrams_.empty()) {
      writeDatagram(recordBuf,
                    DatagramCapsule{.httpDatagramPayload =
                                        std::move(pendingDatagrams_.front())},
                    FrameProtocol::QMUX);
      pendingDatagrams_.pop_front();
    }

    // Flush all accumulated QMux records in one transport write.
    flushRecord(recordBuf, egressBuf);
    if (!egressBuf.empty()) {
      auto writeRes = co_await co_awaitTry(
          transport_->write(egressBuf, config_.writeTimeout));
      egressBuf.move();
      if (writeRes.hasException()) {
        sm.onCloseSession(
            WtStreamManager::CloseSession{.err = 0, .msg = "write error"});
        XLOG(DBG4) << __func__ << "; ex=" << writeRes.exception();
        break;
      }
      // QMux draft "Closing the Connection": endpoints reset the idle
      // timer when sending or receiving QMux frames. The frames just
      // flushed satisfy the egress half of that rule.
      self->resetIdleTimeout();
    }
  }

  XLOG(DBG4) << "QmuxSession::writeLoop exiting";
  transport_->shutdownWrite();
  writeLoopFinished();
  co_return;
}

void QmuxSession::readLoopFinished() noexcept {
  readLoopDone_ = true;
  if (writeLoopDone_) {
    if (auto* handler = std::exchange(wtHandler_, nullptr)) {
      handler->onSessionEnd(folly::none);
    }
  }
}

void QmuxSession::writeLoopFinished() noexcept {
  writeLoopDone_ = true;
  if (readLoopDone_) {
    if (auto* handler = std::exchange(wtHandler_, nullptr)) {
      handler->onSessionEnd(folly::none);
    }
  }
}

} // namespace proxygen::qmux
