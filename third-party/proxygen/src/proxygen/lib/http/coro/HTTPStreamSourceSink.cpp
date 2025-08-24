/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/HTTPStreamSourceSink.h"

namespace {

using namespace proxygen;
using namespace proxygen::coro;
using folly::coro::co_withCancellation;

constexpr uint32_t kMaxBodyChunkSize = 65'536;

class NoopTxnHandler : public HTTPTransactionHandler {
 public:
  explicit NoopTxnHandler() = default;
  ~NoopTxnHandler() override = default;
  void setTransaction(HTTPTransaction*) noexcept override {
  }
  void detachTransaction() noexcept override {
  }
  void onHeadersComplete(std::unique_ptr<HTTPMessage>) noexcept override {
  }
  void onBody(std::unique_ptr<folly::IOBuf>) noexcept override {
  }
  void onChunkHeader(size_t) noexcept override {
  }
  void onChunkComplete() noexcept override {
  }
  void onTrailers(std::unique_ptr<HTTPHeaders>) noexcept override {
  }
  void onEOM() noexcept override {
  }
  void onUpgrade(UpgradeProtocol) noexcept override {
  }
  void onError(const HTTPException&) noexcept override {
  }
  void onInvariantViolation(const HTTPException&) noexcept override {
  }
  void onEgressPaused() noexcept override {
  }
  void onEgressResumed() noexcept override {
  }
  void onPushedTransaction(HTTPTransaction*) noexcept override {
  }
  void onExTransaction(HTTPTransaction*) noexcept override {
  }
  void onGoaway(ErrorCode) noexcept override {
  }
  void onDatagram(std::unique_ptr<folly::IOBuf>) noexcept override {
  }
  void onWebTransportBidiStream(
      HTTPCodec::StreamID, WebTransport::BidiStreamHandle) noexcept override {
  }
  void onWebTransportUniStream(
      HTTPCodec::StreamID, WebTransport::StreamReadHandle*) noexcept override {
  }
  void onWebTransportSessionClose(folly::Optional<uint32_t>) noexcept override {
  }
};

NoopTxnHandler kNoopTxnHandler{};

// helper source to defer eom; simply yields eom from ::readBodyEvent
class EomHttpSource : public HTTPSource {
  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override {
    folly::assume_unreachable();
  }
  folly::coro::Task<HTTPBodyEvent> readBodyEvent(
      uint32_t max = std::numeric_limits<uint32_t>::max()) override {
    co_return HTTPBodyEvent{/*body=*/nullptr, /*inEOM=*/true};
  }
  void stopReading(folly::Optional<const HTTPErrorCode>) override {
  }
};

inline void handleIngressException(
    HTTPTransactionHandler* handler,
    const folly::exception_wrapper& ex) noexcept {
  auto* httpError = CHECK_NOTNULL(ex.get_exception<HTTPError>());
  handler->onError(HTTPErrorToHTTPException(*httpError));
}

/**
 * Bespoke InlineExecutor necessary since folly::InlineExecutor trips an
 * "unsafe" XCHECK in folly::coro::Task
 *
 * Context:
 * This class is an adapter for push-based proxygen/lib <-> poll-based
 * proxygen::coro. The write path of proxygen/lib requests more data inline
 * (via HTTPSink::resumeIngress), but due to the default nature of coroutines
 * the continutation to send such data (via HTTPTxnHandler::onBody) is done at a
 * later time.
 *
 * To achieve parity in proxygen/lib downstream egress behaviour with
 * proxygen::coro upstream, we must schedule the continutation to produce data
 * inline.
 */
class InlineExecutor : public folly::Executor {
  void add(folly::Func f) override {
    f();
  }
};

struct InlineAwaitable {
  using WaitForIngressUnpaused =
      folly::coro::TaskWithExecutor<TimedBaton::Status>;

  friend folly::coro::ViaIfAsyncAwaitable<WaitForIngressUnpaused> co_viaIfAsync(
      folly::Executor::KeepAlive<>, InlineAwaitable&& me) noexcept {
    return folly::coro::co_viaIfAsync(me.task.executor(), std::move(me.task));
  }

  static InlineAwaitable make(folly::coro::Task<TimedBaton::Status>&& task,
                              InlineExecutor& exec) {
    return InlineAwaitable{co_withExecutor(&exec, std::move(task))};
  }

 private:
  explicit InlineAwaitable(WaitForIngressUnpaused&& taskIn) noexcept
      : task(std::move(taskIn)) {
  }

  WaitForIngressUnpaused task;
};

} // namespace

namespace proxygen::coro {

HTTPStreamSourceUpstreamSink::HTTPStreamSourceUpstreamSink(
    folly::EventBase* handlerEvb,
    HTTPSessionContextPtr sessionCtx,
    HTTPTransactionHandler* handler)
    : sessionCtx_(std::move(sessionCtx)),
      egressSource_(handlerEvb, folly::none, this),
      handler_(handler) {
  ingressResumed_.signal();
  gate_.then([this] {
    auto self = std::move(self_);
    handler_->detachTransaction();
  });
}

HTTPStreamSourceUpstreamSink::~HTTPStreamSourceUpstreamSink() {
  egressSource_.setCallback(nullptr);
  cancellationSource_.requestCancellation();
}

void HTTPStreamSourceUpstreamSink::detachAndAbortIfIncomplete(
    std::unique_ptr<HTTPSink> self) {
  CHECK_EQ(self.get(), this);
  self_ = std::move(self);
  handler_ = &kNoopTxnHandler;
  sendAbort();
}

void HTTPStreamSourceUpstreamSink::sendAbort() {
  XLOG(DBG6) << __func__;
  egressSource_.abort(HTTPErrorCode::CANCEL);
  cancellationSource_.requestCancellation();
}

void HTTPStreamSourceUpstreamSink::sendHeadersWithOptionalEOM(
    const HTTPMessage& headers, bool eom) {
  egressHeaders_ = {&headers, eom};
  // the session will only invoke ::readBodyEvent on egressSource_
  egressSource_.validateHeadersAndSkip(*egressHeaders_.msg, egressHeaders_.eom);
}

void HTTPStreamSourceUpstreamSink::sendBody(
    std::unique_ptr<folly::IOBuf> body) {
  using FcState = HTTPStreamSource::FlowControlState;
  windowState_ = egressSource_.body(std::move(body), /*padding=*/0, false);
  if (windowState_ == FcState::CLOSED) {
    handler_->onEgressPaused();
  }
}

folly::coro::Task<void> HTTPStreamSourceUpstreamSink::transact(
    HTTPCoroSession* upstreamSession,
    HTTPCoroSession::RequestReservation reservation) {
  /**
   * I hate this... proxy bailed req after ::transact() by sending direct resp
   * TODO(@damlaj): make ::transact private and invoke interally from
   * ::sendHeaders
   */
  if (egressHeaders_.msg == nullptr) {
    gate_.set(Event::IngressComplete);
    sourceComplete(/*id=*/HTTPCodec::MaxStreamID, folly::none);
    co_return;
  }
  auto responseSource = upstreamSession->sendRequest(
      std::move(reservation),
      *egressHeaders_.msg,
      egressHeaders_.eom ? nullptr : &egressSource_);
  if (egressHeaders_.eom) {
    sourceComplete(/*id=*/HTTPCodec::MaxStreamID, folly::none);
  }

  if (responseSource.hasError()) {
    std::string err{responseSource.error().describe()};
    XLOG(DBG6) << "failed ::sendRequest; err=" << err;
    handler_->onError(
        HTTPException(HTTPException::Direction::INGRESS_AND_EGRESS, err));
    gate_.set(Event::IngressComplete);
    co_return;
  }

  // respSource must have streamID
  id_ = responseSource->getStreamID();
  XCHECK(id_);
  egressSource_.setStreamID(id_.value());

  co_await folly::coro::co_withCancellation(cancellationSource_.getToken(),
                                            read(std::move(*responseSource)));
}

folly::coro::Task<void> HTTPStreamSourceUpstreamSink::read(
    HTTPSourceHolder ingressSource) {
  auto g = folly::makeGuard([this] { gate_.set(Event::IngressComplete); });
  InlineExecutor inlineExec{};
  auto token = cancellationSource_.getToken();
  auto shouldContinue = [&]() -> bool {
    return !token.isCancellationRequested() && bool(ingressSource);
  };

  EomHttpSource eomSource;
  // read header events
  while (shouldContinue()) {
    co_await ingressResumed_.wait();
    auto headerEvent = co_await co_awaitTry(ingressSource.readHeaderEvent());
    if (headerEvent.hasException()) {
      XLOG(DBG6) << "headerEvent err=" << headerEvent.exception().what();
      handleIngressException(handler_, headerEvent.exception());
      break;
    }
    if (token.isCancellationRequested()) {
      break;
    }

    bool final = headerEvent->isFinal();
    handler_->onHeadersComplete(std::move(headerEvent->headers));
    // revproxy will *always* pause ingress after ::onHeaderComplete, switch to
    // eomSource for simplicity
    if (headerEvent->eom) {
      ingressSource.setSource(&eomSource);
      break;
    } else if (final) {
      break;
    }
  }

  // read body events
  while (shouldContinue()) {
    co_await InlineAwaitable::make(
        co_withCancellation(token, ingressResumed_.wait()), inlineExec);
    auto bodyEvent =
        co_await co_awaitTry(ingressSource.readBodyEvent(kMaxBodyChunkSize));
    if (bodyEvent.hasException()) {
      XLOG(DBG6) << "bodyEvent err=" << bodyEvent.exception().what();
      handleIngressException(handler_, bodyEvent.exception());
      break;
    }
    if (token.isCancellationRequested()) {
      break;
    }

    switch (bodyEvent->eventType) {
      case HTTPBodyEvent::BODY: {
        auto& body = bodyEvent->event.body;
        if (!body.empty()) {
          handler_->onBody(body.move());
        }
        break;
      }
      case HTTPBodyEvent::SUSPEND: {
        auto& resume = bodyEvent->event.resume;
        co_await co_awaitTry(std::move(resume));
        break;
      }
      case HTTPBodyEvent::TRAILERS: {
        handler_->onTrailers(std::move(bodyEvent->event.trailers));
        break;
      }
      // ignored
      case HTTPBodyEvent::PADDING:
      case HTTPBodyEvent::DATAGRAM:
      case HTTPBodyEvent::PUSH_PROMISE:
      case HTTPBodyEvent::UPGRADE:
        break;
    }

    if (bodyEvent->eom) {
      handler_->onEOM();
    }
  }
  XLOG(DBG6) << __func__ << "; done";
}

} // namespace proxygen::coro
