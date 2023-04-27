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

class HTTPErrorPage;

class CodecErrorResponseHandler : public HTTPTransaction::Handler {
 public:
  explicit CodecErrorResponseHandler(ErrorCode statusCode);

  // HTTPTransaction::Handler methods
  void setTransaction(HTTPTransaction* txn) noexcept override;
  void detachTransaction() noexcept override;
  void onHeadersComplete(std::unique_ptr<HTTPMessage> msg) noexcept override;
  void onBody(std::unique_ptr<folly::IOBuf> chain) noexcept override;
  void onTrailers(std::unique_ptr<HTTPHeaders> trailers) noexcept override;
  void onEOM() noexcept override;
  void onUpgrade(UpgradeProtocol protocol) noexcept override;
  void onError(const HTTPException& error) noexcept override;
  // These are no-ops since the error response is already in memory
  void onEgressPaused() noexcept override {
  }
  void onEgressResumed() noexcept override {
  }

 private:
  ~CodecErrorResponseHandler() override;

  HTTPTransaction* txn_;
};

} // namespace proxygen
