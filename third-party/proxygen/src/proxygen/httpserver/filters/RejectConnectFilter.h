/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/httpserver/Filters.h>
#include <proxygen/httpserver/RequestHandlerFactory.h>
#include <proxygen/httpserver/ResponseBuilder.h>

namespace proxygen {

/**
 * A filter that rejects CONNECT/UPGRADE requests.
 */
class RejectConnectFilter : public Filter {
 public:
  explicit RejectConnectFilter(RequestHandler* upstream) : Filter(upstream) {
  }

  void onRequest(std::unique_ptr<HTTPMessage> /*msg*/) noexcept override {
    upstream_->onError(kErrorMethodNotSupported);
    upstream_ = nullptr;

    ResponseBuilder(downstream_).rejectUpgradeRequest();
  }

  void onBody(std::unique_ptr<folly::IOBuf> /*body*/) noexcept override {
  }

  void onUpgrade(UpgradeProtocol /*protocol*/) noexcept override {
  }

  void onEOM() noexcept override {
  }

  void requestComplete() noexcept override {
    CHECK(!upstream_);
    delete this;
  }

  void onError(ProxygenError err) noexcept override {
    // If onError is invoked before we forward the error
    if (upstream_) {
      upstream_->onError(err);
      upstream_ = nullptr;
    }

    delete this;
  }

  void onEgressPaused() noexcept override {
  }

  void onEgressResumed() noexcept override {
  }

  // Response handler
  void sendHeaders(HTTPMessage& /*msg*/) noexcept override {
  }

  void sendChunkHeader(size_t /*len*/) noexcept override {
  }

  void sendBody(std::unique_ptr<folly::IOBuf> /*body*/) noexcept override {
  }

  void sendChunkTerminator() noexcept override {
  }

  void sendEOM() noexcept override {
  }

  void sendAbort() noexcept override {
  }

  void refreshTimeout() noexcept override {
  }
};

class RejectConnectFilterFactory : public RequestHandlerFactory {
 public:
  void onServerStart(folly::EventBase* /*evb*/) noexcept override {
  }

  void onServerStop() noexcept override {
  }

  RequestHandler* onRequest(RequestHandler* h,
                            HTTPMessage* msg) noexcept override {

    if (msg->getMethod() == HTTPMethod::CONNECT) {
      return new RejectConnectFilter(h);
    }

    // No need to insert this filter
    return h;
  }
};

} // namespace proxygen
