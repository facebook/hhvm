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
#include <proxygen/lib/http/session/HTTPTransaction.h>

namespace proxygen {

/**
 * If you have an HTTPTransactionHandler, you can use it as a RequestHandler
 * by wrapping it in a HTTPTransactionHandlerAdaptor.
 */
class HTTPTransactionHandlerAdaptor : public RequestHandler {
 public:
  explicit HTTPTransactionHandlerAdaptor(HTTPTransactionHandler* handler)
      : handler_(handler) {
  }

  void onRequest(std::unique_ptr<HTTPMessage> headers) noexcept override {
    // Note: HTTPTransactionHandlerAdaptor's will bypass any response filters.
    // They write directly to the transaction
    auto txn = downstream_->getTransaction();
    // I need the non-const original handler
    auto origHandler = const_cast<HTTPTransaction::Handler*>(txn->getHandler());
    handler_->setTransaction(txn);
    txn->setHandler(origHandler);

    handler_->onHeadersComplete(std::move(headers));
  }

  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override {
    handler_->onBody(std::move(body));
  }

  void onUpgrade(proxygen::UpgradeProtocol prot) noexcept override {
    handler_->onUpgrade(prot);
  }

  void onEOM() noexcept override {
    handler_->onEOM();
  }

  void requestComplete() noexcept override {
    handler_->detachTransaction();
    delete this;
  }

  void onError(ProxygenError err) noexcept override {
    HTTPException ex(HTTPException::Direction::INGRESS_AND_EGRESS,
                     "RequestHandler error");
    ex.setProxygenError(err);
    handler_->onError(ex);
    handler_->detachTransaction();
    delete this;
  }

  void onEgressPaused() noexcept override {
    handler_->onEgressPaused();
  }

  void onEgressResumed() noexcept override {
    handler_->onEgressResumed();
  }
  ~HTTPTransactionHandlerAdaptor() {
  }

 private:
  HTTPTransactionHandler* handler_;
};

} // namespace proxygen
