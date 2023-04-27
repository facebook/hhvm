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

/**
 * Factory for RequestHandlers.
 */
class RequestHandlerFactory {
 public:
  virtual ~RequestHandlerFactory() {
  }

  /**
   * Invoked in each thread server is going to handle requests
   * before we start handling requests. Can be used to setup
   * thread-local setup for each thread (stats and such).
   */
  virtual void onServerStart(folly::EventBase* evb) noexcept = 0;

  /**
   * Invoked in each handler thread after all the connections are
   * drained from that thread. Can be used to tear down thread-local setup.
   */
  virtual void onServerStop() noexcept = 0;

  /**
   * Invoked for each new request server handles. HTTPMessage is provided
   * so that user can potentially choose among several implementation of
   * handler based on URL or something. No need to save/copy this
   * HTTPMessage. RequestHandler will be given the HTTPMessage
   * in a separate callback.
   *
   * Some request handlers don't handle the request themselves (think filters).
   * They do take some actions based on request/response but otherwise they
   * just hand-off request to some other RequestHandler. This upstream
   * RequestHandler is given as first parameter. For the terminal RequestHandler
   * this will by nullptr.
   */
  virtual RequestHandler* onRequest(RequestHandler*, HTTPMessage*) noexcept = 0;
};

/**
 * Helper class to help beautify the way we make chains of these filters
 */
class RequestHandlerChain {
 public:
  std::vector<std::unique_ptr<RequestHandlerFactory>> build() {
    return std::move(chain_);
  }

  template <typename T, typename... Args>
  RequestHandlerChain& addThen(Args&&... args) {
    chain_.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    return *this;
  }

  RequestHandlerChain& addThen(std::unique_ptr<RequestHandlerFactory> h) {
    chain_.push_back(std::move(h));
    return *this;
  }

 private:
  std::vector<std::unique_ptr<RequestHandlerFactory>> chain_;
};

} // namespace proxygen
