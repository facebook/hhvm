/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/webtransport/WebTransportSession.h>

namespace proxygen::detail {

using StreamWriteHandle = WebTransport::StreamWriteHandle;
using StreamReadHandle = WebTransport::StreamReadHandle;
using BidiStreamHandle = WebTransport::BidiStreamHandle;

/**
 * WebTransportWriteLoop contains all the context needed for egress. When egress
 * data is available to send (notified via WtStreamManager::EgressCallback), we
 * schedule WebTransportWriteLoop for the next EventBase loop (similar to how
 * proxygen::coro behaves) via ::schedule().
 */
struct WebTransportWriteLoop
    : public WtLooper
    , public WtStreamManager::EgressCallback {
  explicit WebTransportWriteLoop(folly::EventBase* evb,
                                 WtHttpSession& wtHttpSess,
                                 WtStreamManager& sm) noexcept;
  ~WebTransportWriteLoop() noexcept override = default;

  // WtStreamManager has data to egress, schedule write loop
  void eventsAvailable() noexcept final {
    schedule();
  }

 private:
  void runLoopCallback() noexcept final;
  WtHttpSession& wtHttpSess_;
  WtStreamManager& sm_;
  folly::IOBufQueue buf_{folly::IOBufQueue::cacheChainLength()};
  detail::WtEventVisitor eventVisitor_{buf_};
};

/*
 * WebTransportReadLoop contains all the context needed for ingress. When we
 * receive ingress data from the underlying HttpTxn, we schedule
 * WebTransportReadLoop for the next EventBase loop (similar to how
 * proxygen::coro behaves) via WebTransportReadLoop::schedule().
 */
struct WebTransportReadLoop
    : public WtLooper
    , public WtStreamManagerIngressCallback {
  explicit WebTransportReadLoop(folly::EventBase* evb,
                                WtHttpSession& wtHttpSess,
                                WtStreamManager& sm,
                                WebTransportHandler::Ptr& wtHandler) noexcept;
  ~WebTransportReadLoop() noexcept override = default;

  WtHttpSession& wtHttpSess_;
  WtStreamManager& sm_;
  WebTransportHandler::Ptr& wtHandler_;
  WtCapsuleCallback capsuleCb_;
  WebTransportCapsuleCodec wtCodec_{&capsuleCb_, CodecVersion::H2};

 private:
  void runLoopCallback() noexcept final;
};

WtHttpSession::WtHttpSession(WtLooper& readLoop, WtLooper& writeLoop) noexcept
    : txnHandler_(readLoop, writeLoop, *this) {
}

void WtHttpSession::writesDone() noexcept {
  bool wasWritesDone = std::exchange(writesDone_, true);
  VLOG(6) << "readsDone=" << readsDone_ << "; writesDone=" << writesDone_;
  if (!wasWritesDone && readsDone_) {
    onDone();
    self.reset();
  }
}

void WtHttpSession::readsDone() noexcept {
  bool wasReadsDone = std::exchange(readsDone_, true);
  VLOG(6) << "readsDone=" << readsDone_ << "; writesDone=" << writesDone_;
  if (!wasReadsDone && writesDone_) {
    onDone();
    self.reset();
  }
}

void WtHttpSession::onHttpError(const HTTPException&) noexcept {
  auto selfKeepAlive =
      this->self; // keep self alive until end of ::onError scope
  readsDone();
  writesDone();
}

struct H2WtSession::Context {
  Context(folly::EventBase* evb,
          WtDir dir,
          WtStreamManager::WtConfig wtConfig,
          WebTransportHandler::Ptr wtHandler,
          WtHttpSession& wtHttpSess) noexcept
      : wtHandler(std::move(wtHandler)),
        writeLoop{evb, wtHttpSess, sm},
        readLoop{evb, wtHttpSess, sm, this->wtHandler},
        sm(dir, wtConfig, writeLoop, readLoop, pq) {
  }
  quic::HTTPPriorityQueue pq;
  WebTransportHandler::Ptr wtHandler{nullptr};
  WebTransportWriteLoop writeLoop;
  WebTransportReadLoop readLoop;
  WtStreamManager sm;
};

H2WtSession::H2WtSession(folly::EventBase* evb,
                         WtLooper& readLoop,
                         WtLooper& writeLoop,
                         WtStreamManager& sm,
                         WebTransportHandler::Ptr& wtHandler) noexcept
    : WtHttpSession(readLoop, writeLoop),
      WtSessionBase(evb, sm),
      sm_(sm),
      wtHandler_(wtHandler) {
}

H2WtSession::~H2WtSession() noexcept {
  // abort txn and detach handler if applicable
  if (auto* txn = std::exchange(txnHandler_.txn_, nullptr)) {
    txn->sendAbort();
    txn->setHandler(nullptr);
  }
}

void H2WtSession::init(Ptr self, HttpWtClientCallbackPtr wtClientCb) noexcept {
  this->self = std::move(self);
  txnHandler_.wtClientCb_ = std::move(wtClientCb);
  wtHandler_->onWebTransportSession(this->self);
}

void H2WtSession::onHttpError(const HTTPException& err) noexcept {
  sm_.shutdown({kInternalError, err.describe()});
  WtHttpSession::onHttpError(err);
}
void H2WtSession::onDone() noexcept {
  std::exchange(wtHandler_, nullptr)->onSessionEnd(kInternalError);
}

const folly::SocketAddress& H2WtSession::getLocalAddress() const noexcept {
  return txnHandler_.selfAddr_;
}
const folly::SocketAddress& H2WtSession::getPeerAddress() const noexcept {
  return txnHandler_.peerAddr_;
}

/*static*/ auto H2WtSession::make(folly::EventBase* evb,
                                  WtDir dir,
                                  WtStreamManager::WtConfig wtConfig,
                                  WebTransportHandler::Ptr wtHandler) noexcept
    -> Ptr {

  // aggregate struct to alloc everything in a single chunk
  struct Agg {
    Agg(folly::EventBase* evb,
        WtDir dir,
        WtStreamManager::WtConfig wtConfig,
        WebTransportHandler::Ptr wtHandler)
        : ctx(evb, dir, wtConfig, std::move(wtHandler), wtHttpSess),
          wtHttpSess(evb, ctx.readLoop, ctx.writeLoop, ctx.sm, ctx.wtHandler) {
    }
    // NOTE: these two are cyclical dependencies, both accept refs to eachother
    // but this is ok since neither read/use members until after construction
    H2WtSession::Context ctx;
    H2WtSession wtHttpSess;
  };
  auto agg = std::make_shared<Agg>(evb, dir, wtConfig, std::move(wtHandler));
  auto& wtHttpSess = agg->wtHttpSess;
  return Ptr(agg, &wtHttpSess); // alias ptr
}

void WtLooper::schedule() noexcept {
  bool scheduled = isLoopCallbackScheduled();
  VLOG(6) << "type=" << type_ << "; scheduled=" << scheduled;
  if (!scheduled) {
    evb_->runInLoop(this);
  } // otherwise already scheduled
}

void WtLooper::cancel() noexcept {
  cancelLoopCallback();
}

WebTransportReadLoop::WebTransportReadLoop(
    folly::EventBase* evb,
    WtHttpSession& wtHttpSess,
    WtStreamManager& sm,
    WebTransportHandler::Ptr& wtHandler) noexcept
    : WtLooper(evb, Type::Read),
      wtHttpSess_(wtHttpSess),
      sm_(sm),
      wtHandler_(wtHandler),
      capsuleCb_(sm, static_cast<H2WtSession&>(wtHttpSess)) {
}

void WebTransportReadLoop::runLoopCallback() noexcept {
  auto& txnHandler = wtHttpSess_.txnHandler_;
  auto [data, eom] = txnHandler.moveBufferedIngress();
  CHECK(data || eom || txnHandler.ex_);
  VLOG(6) << __func__ << "; buf=" << data.get() << "; eom=" << eom;
  wtCodec_.onIngress(std::move(data), eom);
  // handle any new peer streams
  auto peerIds = std::move(peerStreams);
  for (auto id : peerIds) {
    VLOG(6) << "new peer wt stream id=" << id;
    auto handle = sm_.getOrCreateBidiHandle(id);
    if (handle.writeHandle) { // write handle iff bidi stream
      wtHandler_->onNewBidiStream(handle);
    } else if (handle.readHandle) {
      wtHandler_->onNewUniStream(handle.readHandle);
    }
  }

  if (eom || txnHandler.ex_) {
    VLOG(4) << "h2 wt ingress eom=" << eom << "; ex=" << txnHandler.ex_;
    sm_.shutdown(WtStreamManager::CloseSession{WebTransport::kInternalError,
                                               "h2 stream ingress closed"});
    wtHttpSess_.readsDone(); // might delete this
  }
}

WebTransportWriteLoop::WebTransportWriteLoop(folly::EventBase* evb,
                                             WtHttpSession& wtHttpSess,
                                             WtStreamManager& sm) noexcept
    : WtLooper(evb, Type::Write), wtHttpSess_(wtHttpSess), sm_(sm) {
}

void WebTransportWriteLoop::runLoopCallback() noexcept {
  /**
   * we can egress data if all the following is true:
   *  - wt_close_session has not been sent
   *  - txn is still attached
   *  - egress is unpaused
   */
  auto& txnHandler = wtHttpSess_.txnHandler_;
  auto& txn = txnHandler.txn_;

  auto canWrite = [&]() {
    const bool writesDone = wtHttpSess_.writesDone_;
    const bool egressPaused = txnHandler.egressPaused_;
    const bool loop = !writesDone && txn && !egressPaused;
    VLOG(6) << "writesDone=" << writesDone << "; txn=" << txn
            << "; egressPaused=" << egressPaused << "; loop=" << loop;
    return loop;
  };

  if (!canWrite()) {
    return;
  }

  // always write control frames first (not subject to flow control)
  auto ctrl = sm_.moveEvents();
  for (auto& ev : ctrl) {
    std::visit(eventVisitor_, ev);
  }
  txn->sendBody(buf_.move());
  // write stream data
  while (auto* wh = sm_.nextWritable()) {
    VLOG(4) << "id=" << wh->getID() << "; wh=" << wh
            << "; egressPaused=" << txnHandler.egressPaused_;
    if (txnHandler.egressPaused_) {
      return;
    }
    auto id = wh->getID();
    constexpr uint16_t kAtMost = std::numeric_limits<uint16_t>::max();
    auto dequeue = sm_.dequeue(*wh, /*atMost=*/kAtMost);
    writeWTStream(buf_,
                  WTStreamCapsule{id, std::move(dequeue.data), dequeue.fin});
    // ::body overflow checked in next iteration
    txn->sendBody(buf_.move());
  }

  if (eventVisitor_.sessionClosed) { // send eom if done
    txn->sendEOM();
    wtHttpSess_.writesDone();
  }
}

WebTransportTxnHandler::WebTransportTxnHandler(
    WtLooper& readLooper,
    WtLooper& writeLooper,
    WtHttpSession& wtHttpSession) noexcept
    : readLooper_(readLooper),
      writeLooper_(writeLooper),
      wtHttpSess_(wtHttpSession) {
}

void WebTransportTxnHandler::setTransaction(HTTPTransaction* txn) noexcept {
  txn_ = txn;
  txn_->getLocalAddress(selfAddr_);
  txn_->getPeerAddress(peerAddr_);
}

void WebTransportTxnHandler::onHeadersComplete(
    std::unique_ptr<HTTPMessage> msg) noexcept {
  // notify client of 2xx headers
  const bool isFinal = msg->isFinal();
  if (wtClientCb_) {
    wtClientCb_->onHeaders(std::move(msg));
  }
  if (isFinal) { // terminal event for WtClientCb
    wtClientCb_.reset();
  }
}

void WebTransportTxnHandler::onBody(
    std::unique_ptr<folly::IOBuf> chain) noexcept {
  ingress_.data.appendToChain(std::move(chain));
  readLooper_.schedule();
}

void WebTransportTxnHandler::onEOM() noexcept {
  ingress_.eom = true;
  readLooper_.schedule();
}

void WebTransportTxnHandler::onError(const HTTPException& error) noexcept {
  VLOG(6) << __func__ << "; ex=" << error.describe();
  if (wtClientCb_) { // terminal event for WtClientCb
    std::exchange(wtClientCb_, nullptr)->onErr(error);
  }
  ex_ = folly::make_exception_wrapper<HTTPException>(error);
  wtHttpSess_.onHttpError(error);
}

void WebTransportTxnHandler::onEgressPaused() noexcept {
  VLOG(6) << __func__;
  // cancel writeLoopCb if scheduled
  writeLooper_.cancel();
  egressPaused_ = true;
}

void WebTransportTxnHandler::onEgressResumed() noexcept {
  VLOG(6) << __func__;
  // schedule writeLoopCb
  writeLooper_.schedule();
  egressPaused_ = false;
}

auto WebTransportTxnHandler::moveBufferedIngress() noexcept -> BufferedIngress {
  return {ingress_.data.empty() ? nullptr : ingress_.data.pop(), ingress_.eom};
}

} // namespace proxygen::detail
