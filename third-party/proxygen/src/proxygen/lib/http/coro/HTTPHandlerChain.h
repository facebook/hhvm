/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPCoroSession.h"

namespace proxygen::coro {

class HTTPChainHandler : public HTTPHandler {
 public:
  HTTPChainHandler() = default;
  ~HTTPChainHandler() override = default;

  std::shared_ptr<HTTPHandler> getNextHandler() {
    return nextHandler_;
  }

  void setNextHandler(std::shared_ptr<HTTPHandler> nextHandler) {
    nextHandler_ = std::move(nextHandler);
  }

 protected:
  std::shared_ptr<HTTPHandler> nextHandler_{nullptr};
};

class HTTPHandlerChain {
 public:
  HTTPHandlerChain() = default;
  ~HTTPHandlerChain() = default;

  std::shared_ptr<HTTPChainHandler> getFront() {
    return front_;
  }
  std::shared_ptr<HTTPChainHandler> getBack() {
    return back_;
  }

  void insertFront(std::shared_ptr<HTTPChainHandler> handler);
  void insertBack(std::shared_ptr<HTTPChainHandler> handler);

 private:
  std::shared_ptr<HTTPChainHandler> front_{nullptr};
  std::shared_ptr<HTTPChainHandler> back_{nullptr};
};

} // namespace proxygen::coro
