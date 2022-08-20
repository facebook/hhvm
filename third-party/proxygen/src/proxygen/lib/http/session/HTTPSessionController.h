/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <glog/logging.h>
#include <proxygen/lib/http/codec/compress/HeaderIndexingStrategy.h>

namespace folly {
class SocketAddress;
}

namespace proxygen {

class HTTPException;
class HTTPMessage;
class HTTPSessionBase;
class HTTPTransaction;
class HTTPTransactionHandler;

class HTTPSessionController {
 public:
  virtual ~HTTPSessionController() {
  }

  /**
   * Will be invoked whenever HTTPSession successfully parses a
   * request
   *
   * The controller creates a Handler for a new transaction.  The
   * transaction and HTTP message (request) are passed so the
   * implementation can construct different handlers based on these.
   * The transaction will be explicitly set on the handler later via
   * setTransaction.  The request message will be passed in
   * onHeadersComplete.
   */
  virtual HTTPTransactionHandler* getRequestHandler(HTTPTransaction& txn,
                                                    HTTPMessage* msg) = 0;

  /**
   * Will be invoked when HTTPSession is unable to parse a new request
   * on the connection because of bad input.
   *
   * error contains specific information about what went wrong
   */
  virtual HTTPTransactionHandler* getParseErrorHandler(
      HTTPTransaction* txn,
      const HTTPException& error,
      const folly::SocketAddress& localAddress) = 0;

  /**
   * Will be invoked when HTTPSession times out parsing a new request.
   */
  virtual HTTPTransactionHandler* getTransactionTimeoutHandler(
      HTTPTransaction* txn, const folly::SocketAddress& localAddress) = 0;

  /**
   * Inform the controller it is associated with this particular session.
   */
  virtual void attachSession(HTTPSessionBase* session) = 0;

  /**
   * Informed at the end when the given HTTPSession is going away.
   */
  virtual void detachSession(const HTTPSessionBase* session) = 0;

  /**
   * Inform the controller that the session's codec changed.
   */
  virtual void onSessionCodecChange(HTTPSessionBase* /*session*/) {
  }

  /**
   * Invoked when the underlying transport is ready.
   *
   * On invocation, the controller can perform operations that depend on access
   * to the transport (socket), such as setting up instrumentation or looking up
   * configuration that depends on the peer's address.

   * For HQ/QUIC, attachSession() is called before the underlying transport is
   * initialized, so transport related operations must be performed here.
   */
  virtual void onTransportReady(HTTPSessionBase* /*session*/) {
  }

  /**
   * Optionally allow the session to query custom graceful shutdown timeout.
   */
  virtual std::chrono::milliseconds getGracefulShutdownTimeout() const {
    return std::chrono::milliseconds(0);
  }

  /**
   * Optionally allow the session to query custom flow control timeout.
   */
  virtual std::chrono::milliseconds getSessionFlowControlTimeout() const {
    return std::chrono::milliseconds(0);
  }

  /**
   * Returns the H2 header indexing strategy to be employed by the session
   */
  virtual const HeaderIndexingStrategy* getHeaderIndexingStrategy() const {
    return HeaderIndexingStrategy::getDefaultInstance();
  }
};

class HTTPUpstreamSessionController : public HTTPSessionController {
  HTTPTransactionHandler* getRequestHandler(HTTPTransaction& /*txn*/,
                                            HTTPMessage* /*msg*/) final {
    LOG(FATAL) << "Unreachable";
    return nullptr;
  }

  /**
   * Will be invoked when HTTPSession is unable to parse a new request
   * on the connection because of bad input.
   *
   * error contains specific information about what went wrong
   */
  HTTPTransactionHandler* getParseErrorHandler(
      HTTPTransaction* /*txn*/,
      const HTTPException& /*error*/,
      const folly::SocketAddress& /*localAddress*/) final {
    LOG(FATAL) << "Unreachable";
    return nullptr;
  }

  /**
   * Will be invoked when HTTPSession times out parsing a new request.
   */
  HTTPTransactionHandler* getTransactionTimeoutHandler(
      HTTPTransaction* /*txn*/,
      const folly::SocketAddress& /*localAddress*/) final {
    LOG(FATAL) << "Unreachable";
    return nullptr;
  }
};

} // namespace proxygen
