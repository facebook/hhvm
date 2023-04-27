/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/session/HTTPSessionController.h>
#include <string>

namespace proxygen {

class HTTPErrorPage;
class HTTPSessionAcceptor;

/**
 * This simple controller provides some basic default behaviors. When
 * errors occur, it will install an appropriate handler. Otherwise, it
 * will install the acceptor's default handler.
 */
class SimpleController : public HTTPSessionController {
 public:
  explicit SimpleController(HTTPSessionAcceptor* acceptor);

  /**
   * Will be invoked whenever HTTPSession successfully parses a
   * request
   */
  HTTPTransactionHandler* getRequestHandler(HTTPTransaction& txn,
                                            HTTPMessage* msg) override;

  /**
   * Will be invoked when HTTPSession is unable to parse a new request
   * on the connection because of bad input.
   *
   * error contains specific information about what went wrong
   */
  HTTPTransactionHandler* getParseErrorHandler(
      HTTPTransaction* txn,
      const HTTPException& error,
      const folly::SocketAddress& localAddress) override;

  /**
   * Will be invoked when HTTPSession times out parsing a new request.
   */
  HTTPTransactionHandler* getTransactionTimeoutHandler(
      HTTPTransaction* txn, const folly::SocketAddress& localAddress) override;

  void attachSession(HTTPSessionBase*) override;
  void detachSession(const HTTPSessionBase*) override;

  std::chrono::milliseconds getGracefulShutdownTimeout() const override;

 protected:
  HTTPTransactionHandler* createErrorHandler(uint32_t statusCode,
                                             const std::string& statusMessage,
                                             const HTTPErrorPage* errorPage);

  HTTPSessionAcceptor* const acceptor_{nullptr};
};

} // namespace proxygen
