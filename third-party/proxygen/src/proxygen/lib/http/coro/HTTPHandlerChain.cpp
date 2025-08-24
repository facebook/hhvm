/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/http/coro/HTTPHandlerChain.h"

namespace proxygen::coro {

void HTTPHandlerChain::insertFront(std::shared_ptr<HTTPChainHandler> handler) {
  if (!handler) {
    return;
  }

  if (front_) {
    handler->setNextHandler(std::move(front_));
  }
  front_ = std::move(handler);

  if (!back_) {
    back_ = front_;
  }
}

void HTTPHandlerChain::insertBack(std::shared_ptr<HTTPChainHandler> handler) {
  if (!handler) {
    return;
  }

  if (back_) {
    back_->setNextHandler(handler);
  }
  back_ = std::move(handler);

  if (!front_) {
    front_ = back_;
  }
}

} // namespace proxygen::coro
