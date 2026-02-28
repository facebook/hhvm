/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/HTTPTransactionAdaptorSource.h"
#include "proxygen/lib/http/coro/HTTPError.h"
#include "proxygen/lib/http/coro/HTTPSourceReader.h"

namespace {
constexpr std::chrono::milliseconds kDefaultEgressTimeout =
    std::chrono::milliseconds(0);
}

namespace proxygen::coro {
HTTPTransactionAdaptorSource::HTTPTransactionAdaptorSource(
    folly::EventBase* evb)
    : evb_(evb),
      ingressSource_(evb, folly::none, this /*callback */),
      egressResumed_(evb, kDefaultEgressTimeout) {

  gate_.then([this] { delete this; });

  // egress starts unpaused.
  egressResumed_.signal();
}

HTTPTransactionAdaptorSource* HTTPTransactionAdaptorSource::create(
    folly::EventBase* evb) {
  return new HTTPTransactionAdaptorSource(evb);
}

HTTPTransactionAdaptorSource::~HTTPTransactionAdaptorSource() {
  XCHECK(!txn_);
  XCHECK(ingressSource_.inputFinished());
}

folly::CancellationToken HTTPTransactionAdaptorSource::getCancelToken() {
  return cancelSource_.getToken();
}

HTTPSource* HTTPTransactionAdaptorSource::getIngressSource() {
  return &ingressSource_;
}

void HTTPTransactionAdaptorSource::setEgressSource(
    HTTPSourceHolder egressSource) {
  egressSource_ = std::move(egressSource);
  startEgressLoop();
}

void HTTPTransactionAdaptorSource::setTransaction(
    HTTPTransaction* txn) noexcept {
  txn_ = txn;
}

void HTTPTransactionAdaptorSource::detachTransaction() noexcept {
  requestCancellation();

  if (egressSource_) {
    egressSource_.stopReading();
    egressSource_ = nullptr;
  }

  txn_ = nullptr;
  gate_.set(Event::EgressComplete);
}

void HTTPTransactionAdaptorSource::onHeadersComplete(
    std::unique_ptr<HTTPMessage> msg) noexcept {
  ingressSource_.headers(std::move(msg), false /*eom*/);
}

void HTTPTransactionAdaptorSource::onBody(
    std::unique_ptr<folly::IOBuf> chain) noexcept {
  windowState_ = ingressSource_.body(std::move(chain), false /*eom*/);
  if (windowState_ == HTTPStreamSource::FlowControlState::CLOSED &&
      hasTransaction()) {
    txn_->pauseIngress();
  }
}

void HTTPTransactionAdaptorSource::onTrailers(
    std::unique_ptr<HTTPHeaders> trailers) noexcept {
  ingressSource_.trailers(std::move(trailers));
}

void HTTPTransactionAdaptorSource::onEOM() noexcept {
  ingressSource_.eom();
}

void HTTPTransactionAdaptorSource::onUpgrade(
    UpgradeProtocol /* protocol */) noexcept {
  // no-op
}

void HTTPTransactionAdaptorSource::onError(
    const HTTPException& error) noexcept {
  ingressSource_.abort(HTTPException2HTTPErrorCode(error));
  txn_->sendAbort();
  requestCancellation();
}

void HTTPTransactionAdaptorSource::onEgressPaused() noexcept {
  egressResumed_.reset();
}

void HTTPTransactionAdaptorSource::onEgressResumed() noexcept {
  egressResumed_.signal();
}

void HTTPTransactionAdaptorSource::windowOpen(HTTPCodec::StreamID /*id*/) {
  windowState_ = HTTPStreamSource::FlowControlState::OPEN;
  if (hasTransaction()) {
    txn_->resumeIngress();
  }
}

void HTTPTransactionAdaptorSource::sourceComplete(
    HTTPCodec::StreamID /*id*/, folly::Optional<HTTPError> /*error*/) {
  gate_.set(Event::IngressComplete);
}

bool HTTPTransactionAdaptorSource::hasTransaction() const {
  return txn_;
}

void HTTPTransactionAdaptorSource::startEgressLoop() {
  co_withExecutor(
      evb_,
      folly::coro::co_withCancellation(cancelSource_.getToken(), egressLoop()))
      .start();
}

folly::coro::Task<void> HTTPTransactionAdaptorSource::egressLoop() {

  // Ensure egress source is still available before we start reading.
  co_await folly::coro::co_safe_point;

  HTTPSourceReader reader(std::move(egressSource_));
  reader
      .preReadAsync([&]() -> folly::coro::Task<bool> {
        co_await egressResumed_.wait();
        co_return HTTPSourceReader::Continue;
      })
      .onHeaders([&, token = cancelSource_.getToken()](
                     std::unique_ptr<HTTPMessage> msg,
                     auto /* isFinal */,
                     bool eom) -> bool {
        if (token.isCancellationRequested()) {
          return HTTPSourceReader::Cancel;
        }
        txn_->sendHeadersWithOptionalEOM(*msg, eom);
        return HTTPSourceReader::Continue;
      })
      .onBody([&, token = cancelSource_.getToken()](BufQueue body,
                                                    bool eom) -> bool {
        if (token.isCancellationRequested()) {
          return HTTPSourceReader::Cancel;
        }
        if (!body.empty()) {
          txn_->sendBody(body.move());
        }
        if (eom) {
          txn_->sendEOM();
        }
        return HTTPSourceReader::Continue;
      })
      .onTrailers([&](std::unique_ptr<HTTPHeaders> trailers) {
        txn_->sendTrailers(*trailers);
      })
      .onError([&, token = cancelSource_.getToken()](
                   HTTPSourceReader::ErrorContext /* ctx */,
                   const HTTPError& error) {
        if (token.isCancellationRequested()) {
          return;
        }

        XLOG_EVERY_MS(ERR, 10000)
            << "Error while handling transaction" << error.describe();
        txn_->sendAbort();
      });

  co_await reader.read();
}

void HTTPTransactionAdaptorSource::requestCancellation() {
  cancelSource_.requestCancellation();
}

} // namespace proxygen::coro
