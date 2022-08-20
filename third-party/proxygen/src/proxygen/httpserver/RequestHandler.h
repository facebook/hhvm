/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/session/HTTPTransaction.h>

namespace proxygen {

class ResponseHandler;
class ExMessageHandler;

/**
 * Interface to be implemented by objects that handle requests from
 * client. ResponseHandler acts as the client for these objects and
 * provides methods to send back the response
 */
class RequestHandler {
 public:
  /**
   * Saves the downstream handle with itself. Implementations of this
   * interface should use downstream_ to send back response.
   *
   * XXX: Only override this method if you are ABSOLUTELY sure what you
   *      are doing. If possible, just use downstream_ variable and dont
   *      mess with these things.
   */
  virtual void setResponseHandler(ResponseHandler* handler) noexcept {
    downstream_ = CHECK_NOTNULL(handler);
  }

  /**
   * Invoked when we have successfully fetched headers from client. This will
   * always be the first callback invoked on your handler.
   */
  virtual void onRequest(std::unique_ptr<HTTPMessage> headers) noexcept = 0;

  /**
   * Invoked when we get part of body for the request.
   */
  virtual void onBody(std::unique_ptr<folly::IOBuf> body) noexcept = 0;

  /**
   * Invoked when the session has been upgraded to a different protocol
   */
  virtual void onUpgrade(proxygen::UpgradeProtocol prot) noexcept = 0;

  /**
   * Invoked when we finish receiving the body.
   */
  virtual void onEOM() noexcept = 0;

  /**
   * Invoked when request processing has been completed and nothing more
   * needs to be done. This may be a good place to log some stats and
   * clean up resources. This is distinct from onEOM() because it is
   * invoked after the response is fully sent. Once this callback has been
   * received, `downstream_` should be considered invalid.
   */
  virtual void requestComplete() noexcept = 0;

  /**
   * Request failed. Maybe because of read/write error on socket or client
   * not being able to send request in time.
   *
   * NOTE: Can be invoked at any time (except for before onRequest).
   *
   * No more callbacks will be invoked after this. You should clean up after
   * yourself.
   */
  virtual void onError(ProxygenError err) noexcept = 0;

  /**
   * Signals from HTTP layer when we receive GOAWAY frame.
   */
  virtual void onGoaway(ErrorCode /*code*/) noexcept {
  }

  /**
   * Signals from HTTP layer when client queue is full or empty. If you are
   * sending a streaming response, consider implementing these and acting
   * accordingly. Saves your server from running out of memory.
   */
  virtual void onEgressPaused() noexcept {
  }

  virtual void onEgressResumed() noexcept {
  }

  /**
   * Returns true if the handler is responsible for responding to Expect
   * headers, false otherwise.
   */
  virtual bool canHandleExpect() noexcept {
    return false;
  }

  /**
   * Implement in control stream handler to support incoming child EX streams.
   */
  virtual ExMessageHandler* getExHandler() noexcept {
    LOG(FATAL) << "Not implemented";
    folly::assume_unreachable();
  }

  virtual ResponseHandler* getDownstream() noexcept {
    return downstream_;
  }

  virtual ~RequestHandler() {
  }

 protected:
  /**
   * A place designated for the response handler. You can use this to send back
   * the response in your RequestHandler.
   */
  ResponseHandler* downstream_{nullptr};
};

} // namespace proxygen
