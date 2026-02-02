/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/logging/xlog.h>
#include <proxygen/lib/http/coro/util/CoroWtSession.h>

namespace proxygen::coro::detail {

CoroWtSession::CoroWtSession(
    folly::EventBase* evb,
    WtDir dir,
    WtStreamManager::WtConfig wtConfig,
    std::unique_ptr<WebTransportHandler> handler) noexcept
    : CoroWtSessionBase(dir, wtConfig),
      WtSessionBase(evb, sm),
      wtHandler_(std::move(handler)) {
}

CoroWtSession::~CoroWtSession() noexcept {
  cs_.requestCancellation();
}

WtExpected<folly::Unit>::Type CoroWtSession::closeSession(
    folly::Optional<uint32_t> error) noexcept {
  cs_.requestCancellation();
  sm.shutdown(WtStreamManager::CloseSession{.err = error.value_or(0),
                                            .msg = "closeSession"});
  return folly::unit;
}

using WtCapsuleCallback = proxygen::detail::WtCapsuleCallback;
folly::coro::Task<void> CoroWtSession::readLoop(Ptr self,
                                                HTTPSourceHolder ingress) {
  WtCapsuleCallback wtCapsuleCallback{sm, *this};
  WebTransportCapsuleCodec codec{&wtCapsuleCallback, CodecVersion::H2};

  while (!sm.isClosed() && ingress.readable()) {
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
    auto peerIds = std::move(wtSmIngressCb.peerStreams);
    for (auto id : peerIds) {
      auto handle = sm.getOrCreateBidiHandle(id);
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
  sm.shutdown(WtStreamManager::CloseSession{.err = 0x00,
                                            .msg = "h2 stream ingress closed"});
  readLoopFinished();
}

folly::coro::Task<void> CoroWtSession::writeLoop(Ptr self,
                                                 EgressSourcePtr egress) {
  const auto timeout = egress->getReadTimeout();
  const folly::IOBuf empty;
  folly::IOBufQueue egressBuf{folly::IOBufQueue::cacheChainLength()};
  proxygen::detail::WtEventVisitor eventVisitor{.egress = egressBuf};
  detail::EgressBackPressure streamSourceCallback;
  egress->setCallback(&streamSourceCallback);
  auto& waitForEventBaton = wtSmEgressCb.waitForEvent;

  while (!eventVisitor.sessionClosed) {
    // wait for WtSession egress (i.e. underlying http/2 egress buffer space);
    // this is upperbounded by writeTimeout in HTTPBodyEventQueue
    XLOG(DBG6) << "waiting for http/2 egress fc";
    co_await streamSourceCallback.waitForEgress.wait();

    // wait for underlying wt ctrl events or writable streams
    XLOG(DBG6) << "waiting for WtStreamManager event";
    auto res = co_await waitForEventBaton.timedWait(evb(), timeout);
    if (res == TimedBaton::Status::timedout) {
      sm.shutdown(WtStreamManager::CloseSession{.err = 0x00,
                                                .msg = "wt write timed out"});
    } // fallthru to writing close_session below
    waitForEventBaton.reset();

    XLOG(DBG6) << "received WtStreamManager event";
    // always write control frames first (not subject to flow control)
    auto ctrl = sm.moveEvents();
    for (auto& ev : ctrl) {
      std::visit(eventVisitor, ev);
    }
    egress->body(egressBuf.move(), /*padding=*/0, /*eom=*/false);

    // write stream data
    auto* wh = sm.nextWritable();
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
      auto dequeue = sm.dequeue(*wh, /*atMost=*/bytesAvailable);
      writeWTStream(egressBuf,
                    WTStreamCapsule{.streamId = id,
                                    .streamData = std::move(dequeue.data),
                                    .fin = dequeue.fin});
      // ::body overflow checked in next iteration
      egress->body(egressBuf.move(), /*padding=*/0, /*eom=*/false);
      wh = sm.nextWritable();
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
  auto* eventBase = evb();
  co_withExecutor(eventBase,
                  co_withCancellation(ct, readLoop(self, std::move(ingress))))
      .start();
  co_withExecutor(eventBase,
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
