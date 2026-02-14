/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/io/coro/Transport.h>
#include <folly/logging/xlog.h>
#include <proxygen/lib/http/coro/util/CoroWtSession.h>

namespace {
using namespace proxygen::coro;
using folly::coro::co_error;
using folly::coro::co_nothrow;

BufQueue* asBodyEv(HTTPBodyEvent& event) {
  return event.eventType == HTTPBodyEvent::BODY ? &event.event.body : nullptr;
}

}; // namespace

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
 * Generally speaking, heap allocated HTTPSources are consumer-owned (i.e. when
 * a consumer reads a terminal event, e.g. exc or eom, it will deallocate/free
 * itself). To simplify lifetime here, we utilize some trickery to create a
 * producer-owned HTTPSource.
 *
 * When a producer is done using the source (i.e. when this Deleter is invoked),
 * we check to see if the consuming side is also done (i.e. ::sourceComplete):
 *
 *     - If consumer is done, both producer and consumer are finished and we can
 *       free the object.
 *
 *     - If the consumer is not done however, we invoke ::setHeapAllocated
 *       (misnomer/confusing here) to transfer ownership to the consumer.
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

namespace {
using namespace proxygen::coro::detail;
using folly::AsyncSocketException;
using AsyncSocketExceptionType = AsyncSocketException::AsyncSocketExceptionType;

folly::exception_wrapper makeSocketEx(AsyncSocketExceptionType type,
                                      std::string_view err = "") {
  return folly::make_exception_wrapper<AsyncSocketException>(type,
                                                             std::string(err));
}

/**
 * Wraps an egress HTTPStreamSource and an ingress HTTPSource (also of concrete
 * type HTTPStreamSource, but irrelevant for the most part). Exposes reading
 * from an ingress HTTPSource & writing to an egress HTTPSource over a
 * folly::coro::TransportIf interface.
 */
class HttpSourceTransport : public folly::coro::TransportIf {
 public:
  HttpSourceTransport(folly::EventBase* evb,
                      EgressSourcePtr&& egressSource,
                      HTTPSourceHolder&& ingress) noexcept
      : evb_(evb),
        egressSource_(std::move(egressSource)),
        ingressSource_(std::move(ingress)) {
    egressSource_->setCallback(&callback_);
  }

  ~HttpSourceTransport() noexcept override = default;

  folly::EventBase* getEventBase() noexcept override {
    return evb_;
  }

  folly::AsyncTransport* getTransport() const noexcept override {
    return nullptr;
  }

  const folly::AsyncTransportCertificate* getPeerCertificate()
      const noexcept override {
    return nullptr;
  }

  folly::coro::Task<size_t> read(
      folly::IOBufQueue& buf,
      size_t /*minReadSize*/,
      size_t /*newAllocationSize*/,
      std::chrono::milliseconds timeout) noexcept override {
    if (std::exchange(deferredEof_, false)) {
      co_return 0;
    }

    // read after ingress terminal event is read (i.e. eom or exc) is an error
    if (!ingressSource_.readable()) {
      co_yield co_error(makeSocketEx(AsyncSocketExceptionType::INTERNAL_ERROR));
    }

    ingressSource_.setReadTimeout(timeout);
    // loop until the first BodyEvent or exc is yielded
    folly::Try<HTTPBodyEvent> ev{
        HTTPBodyEvent{/*body=*/nullptr, /*inEOM=*/false}};
    bool done = false;
    while (!done) {
      ev = co_await co_awaitTry(ingressSource_.readBodyEvent());
      if (ev.hasException()) {
        co_yield co_error(makeSocketEx(AsyncSocketExceptionType::END_OF_FILE,
                                       ev.exception().what()));
      }
      auto* body = asBodyEv(*ev);
      done = bool(body) || ev->eom; // loop again if not body event
    }

    uint64_t len = 0;
    if (auto* body = asBodyEv(*ev)) {
      len = body->chainLength();
      buf.append(body->move()); // ok if nullptr
    }
    deferredEof_ = len > 0 && ev->eom;
    co_return len;
  }

  folly::coro::Task<folly::Unit> write(folly::IOBufQueue& ioBufQueue,
                                       std::chrono::milliseconds timeout,
                                       folly::WriteFlags,
                                       WriteInfo* info) noexcept override {
    if (callback_.ex) {
      XLOG(DBG4) << "id=" << getId() << "; ex=" << callback_.ex.what();
      co_yield co_error(makeSocketEx(AsyncSocketExceptionType::NOT_OPEN,
                                     callback_.ex.what()));
    }
    auto bytesAvailable = egressSource_->window().getNonNegativeSize();
    XLOG(DBG4) << "id=" << getId() << "; bytesAvailable=" << bytesAvailable;
    if (bytesAvailable == 0) {
      XLOG(DBG5) << __func__ << " egress blocked";
      callback_.waitForEgress.reset(); // suspend until egress drained
    }
    auto res = co_await callback_.waitForEgress.timedWait(evb_, timeout);
    if (res == TimedBaton::Status::timedout) {
      XLOG(DBG6) << "id=" << getId() << "; ingress timeout";
      co_yield co_error(makeSocketEx(AsyncSocketExceptionType::TIMED_OUT));
    }
    if (res == TimedBaton::Status::cancelled) {
      XLOG(DBG6) << "id=" << getId() << "; cancelled";
      co_yield co_error(makeSocketEx(AsyncSocketExceptionType::CANCELED));
    }
    auto len = ioBufQueue.chainLength();
    egressSource_->body(ioBufQueue.move(), /*padding=*/0, /*eom=*/false);
    if (info) {
      info->bytesWritten = len;
    }
    co_return folly::unit;
  }

  void close() noexcept override {
    egressSource_->eom();
    ingressSource_.setSource(nullptr);
  }

  void shutdownWrite() noexcept override {
    egressSource_->eom();
  }

  void closeWithReset() noexcept override {
    egressSource_->abort(HTTPErrorCode::CANCEL);
    ingressSource_.setSource(nullptr);
  }

  // unimplemented fns
  folly::coro::Task<folly::Unit> write(folly::ByteRange,
                                       std::chrono::milliseconds,
                                       folly::WriteFlags,
                                       WriteInfo*) noexcept override {
    XLOG(FATAL) << "not implemented";
  }
  folly::coro::Task<size_t> read(folly::MutableByteRange,
                                 std::chrono::milliseconds) noexcept override {
    XLOG(FATAL) << "not implemented";
  }
  folly::SocketAddress getLocalAddress() const noexcept override {
    XLOG(FATAL) << "not implemented";
  }
  folly::SocketAddress getPeerAddress() const noexcept override {
    XLOG(FATAL) << "not implemented";
  }

 private:
  uint64_t getId() const {
    return egressSource_->getID();
  }
  folly::EventBase* evb_;
  EgressSourcePtr egressSource_;
  HTTPSourceHolder ingressSource_;
  bool deferredEof_{false};
  EgressBackPressure callback_; // initially signalled
};

} // namespace

namespace proxygen::coro::detail {

std::unique_ptr<folly::coro::TransportIf> makeHttpSourceTransport(
    folly::EventBase* evb,
    EgressSourcePtr&& source,
    HTTPSourceHolder&& ingress) {
  return std::make_unique<HttpSourceTransport>(
      evb, std::move(source), std::move(ingress));
}

} // namespace proxygen::coro::detail
