/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/httpserver/RequestHandler.h>

namespace proxygen {

class PushHandler : public RequestHandler {
 public:
  PushHandler() : innerHandler_(*this) {
  }

  // Caller may implement these callbacks if desired
  void requestComplete() noexcept override {
    delete this;
  }
  void onError(ProxygenError /*err*/) noexcept override {
    delete this;
  }

  HTTPPushTransactionHandler* getHandler() {
    return &innerHandler_;
  }

 private:
  class InnerPushHandler : public HTTPPushTransactionHandler {
   public:
    explicit InnerPushHandler(PushHandler& handler) : handler_(handler) {
    }

    void setTransaction(HTTPTransaction* /*txn*/) noexcept override {
    }
    void detachTransaction() noexcept override {
      handler_.requestComplete();
    }
    void onError(const HTTPException& error) noexcept override {
      handler_.onError(error.getProxygenError());
    }
    void onEgressPaused() noexcept override {
      handler_.onEgressPaused();
    }
    void onEgressResumed() noexcept override {
      handler_.onEgressResumed();
    }

   private:
    PushHandler& handler_;
  };

  void onRequest(std::unique_ptr<HTTPMessage> /*headers*/) noexcept override {
    LOG(FATAL) << "Unreachable";
  }

  void onBody(std::unique_ptr<folly::IOBuf> /*body*/) noexcept override {
    LOG(FATAL) << "Unreachable";
  }

  void onUpgrade(proxygen::UpgradeProtocol /*prot*/) noexcept override {
    LOG(FATAL) << "Unreachable";
  }

  void onEOM() noexcept override {
    LOG(FATAL) << "Unreachable";
  }

  InnerPushHandler innerHandler_;
};

} // namespace proxygen
