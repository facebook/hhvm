/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseHandler.h>

namespace proxygen {

/**
 * Filters are a way to add functionality to HTTPServer without complicating app
 * specific RequestHandler. The basic idea is
 *
 *   App-handler <=> Filter-1 <=> Filter-2 <=> Client
 *
 * The data flows through these filters between client and handler.  They can do
 * things like modify the data flowing through them, can update stats, create
 * traces and even send direct response if they feel like.
 *
 * The default implementation just lets everything pass through.
 */
class Filter
    : public RequestHandler
    , public ResponseHandler {
 public:
  explicit Filter(RequestHandler* upstream) : ResponseHandler(upstream) {
  }

  // Request handler
  void setResponseHandler(ResponseHandler* handler) noexcept override {
    // Save downstream handler and pass ourselves as downstream handler
    downstream_ = handler;
    txn_ = handler->getTransaction();
    upstream_->setResponseHandler(this);
  }

  void onRequest(std::unique_ptr<HTTPMessage> headers) noexcept override {
    upstream_->onRequest(std::move(headers));
  }

  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override {
    upstream_->onBody(std::move(body));
  }

  void onUpgrade(UpgradeProtocol protocol) noexcept override {
    upstream_->onUpgrade(protocol);
  }

  void onEOM() noexcept override {
    upstream_->onEOM();
  }

  void requestComplete() noexcept override {
    downstream_ = nullptr;
    upstream_->requestComplete();
    delete this;
  }

  void onError(ProxygenError err) noexcept override {
    downstream_ = nullptr;
    upstream_->onError(err);
    delete this;
  }

  void onGoaway(ErrorCode code) noexcept override {
    upstream_->onGoaway(code);
  }

  void onEgressPaused() noexcept override {
    upstream_->onEgressPaused();
  }

  void onEgressResumed() noexcept override {
    upstream_->onEgressResumed();
  }

  bool canHandleExpect() noexcept override {
    return upstream_->canHandleExpect();
  }

  ExMessageHandler* getExHandler() noexcept override {
    return upstream_->getExHandler();
  }

  // Response handler
  void sendHeaders(HTTPMessage& msg) noexcept override {
    downstream_->sendHeaders(msg);
  }

  void sendChunkHeader(size_t len) noexcept override {
    downstream_->sendChunkHeader(len);
  }

  void sendBody(std::unique_ptr<folly::IOBuf> body) noexcept override {
    downstream_->sendBody(std::move(body));
  }

  void sendChunkTerminator() noexcept override {
    downstream_->sendChunkTerminator();
  }

  void sendEOM() noexcept override {
    downstream_->sendEOM();
  }

  void sendAbort() noexcept override {
    downstream_->sendAbort();
  }

  void refreshTimeout() noexcept override {
    downstream_->refreshTimeout();
  }

  void pauseIngress() noexcept override {
    downstream_->pauseIngress();
  }

  void resumeIngress() noexcept override {
    downstream_->resumeIngress();
  }

  folly::Expected<ResponseHandler*, ProxygenError> newPushedResponse(
      PushHandler* handler) noexcept override {
    return downstream_->newPushedResponse(handler);
  }

  ResponseHandler* newExMessage(ExMessageHandler* exHandler,
                                bool unidirectional) noexcept override {
    return downstream_->newExMessage(exHandler, unidirectional);
  }

  const wangle::TransportInfo& getSetupTransportInfo() const noexcept override {
    return downstream_->getSetupTransportInfo();
  }

  void getCurrentTransportInfo(wangle::TransportInfo* tinfo) const override {
    downstream_->getCurrentTransportInfo(tinfo);
  }
};

} // namespace proxygen
