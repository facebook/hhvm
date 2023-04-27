/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/httpserver/ResponseHandler.h>
#include <proxygen/lib/http/session/HTTPTransaction.h>

namespace proxygen {

class RequestHandler;
class PushHandler;

/**
 * An adaptor that converts HTTPTransactionHandler to RequestHandler.
 * Apart from that it also -
 *
 * - Handles all the error cases itself as described below. It makes a terminal
 *   call onError(...) where you are expected to log something and stop
 *   processing the request if you have started so.
 *
 *   onError - Send a direct response back if no response has started and
 *             writing is still possible. Otherwise sends an abort.
 *
 * - Handles the 100-continue case for you (by sending Continue response),
 *   if RequestHandler returns false for canHandleExpect(). Otherwise,
 *   RequestHandler is responsible for handling it.
 */
class RequestHandlerAdaptor
    : public HTTPTransactionHandler
    , public ResponseHandler {
 public:
  explicit RequestHandlerAdaptor(RequestHandler* requestHandler);

 private:
  // HTTPTransactionHandler
  void setTransaction(HTTPTransaction* txn) noexcept override;
  void detachTransaction() noexcept override;
  void onHeadersComplete(std::unique_ptr<HTTPMessage> msg) noexcept override;
  void onBody(std::unique_ptr<folly::IOBuf> chain) noexcept override;
  void onChunkHeader(size_t length) noexcept override;
  void onChunkComplete() noexcept override;
  void onTrailers(std::unique_ptr<HTTPHeaders> trailers) noexcept override;
  void onEOM() noexcept override;
  void onUpgrade(UpgradeProtocol protocol) noexcept override;
  void onError(const HTTPException& error) noexcept override;
  void onGoaway(ErrorCode code) noexcept override;
  void onEgressPaused() noexcept override;
  void onEgressResumed() noexcept override;
  void onExTransaction(HTTPTransaction* txn) noexcept override;

  // ResponseHandler
  void sendHeaders(HTTPMessage& msg) noexcept override;
  void sendChunkHeader(size_t len) noexcept override;
  void sendBody(std::unique_ptr<folly::IOBuf> body) noexcept override;
  void sendChunkTerminator() noexcept override;
  void sendEOM() noexcept override;
  void sendAbort() noexcept override;
  void refreshTimeout() noexcept override;
  void pauseIngress() noexcept override;
  void resumeIngress() noexcept override;
  folly::Expected<ResponseHandler*, ProxygenError> newPushedResponse(
      PushHandler* pushHandler) noexcept override;
  ResponseHandler* newExMessage(ExMessageHandler* exHandler,
                                bool unidirectional) noexcept override;
  const wangle::TransportInfo& getSetupTransportInfo() const noexcept override;
  void getCurrentTransportInfo(wangle::TransportInfo* tinfo) const override;

  // Helper method
  void setError(ProxygenError err) noexcept;

  ProxygenError err_{kErrorNone};
};

} // namespace proxygen
