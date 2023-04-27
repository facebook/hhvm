/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>

namespace proxygen {

/**
 * Handler that sends a fixed response back.
 */
class DirectResponseHandler : public RequestHandler {
 public:
  DirectResponseHandler(int code, std::string message, std::string body)
      : code_(code),
        message_(std::move(message)),
        body_(folly::IOBuf::copyBuffer(body)) {
  }

  void onRequest(std::unique_ptr<HTTPMessage> /*headers*/) noexcept override {
    ResponseBuilder(downstream_)
        .status(code_, std::move(message_))
        .body(std::move(body_))
        .send();
  }

  void onBody(std::unique_ptr<folly::IOBuf> /*body*/) noexcept override {
  }

  void onUpgrade(proxygen::UpgradeProtocol /*prot*/) noexcept override {
  }

  void onEOM() noexcept override {
    ResponseBuilder(downstream_).sendWithEOM();
  }

  void requestComplete() noexcept override {
    delete this;
  }

  void onError(ProxygenError /*err*/) noexcept override {
    delete this;
  }

 private:
  const int code_;
  std::string message_;
  std::unique_ptr<folly::IOBuf> body_;
};

} // namespace proxygen
