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

class HTTPDirectResponseHandler : public HTTPTransaction::Handler {
 public:
  HTTPDirectResponseHandler(unsigned statusCode,
                            const std::string& statusMsg,
                            const HTTPErrorPage* errorPage = nullptr);

  void forceConnectionClose(bool close) {
    forceConnectionClose_ = close;
  }
  // HTTPTransaction::Handler methods
  void setTransaction(HTTPTransaction* txn) noexcept override;
  void detachTransaction() noexcept override;
  void onHeadersComplete(std::unique_ptr<HTTPMessage> msg) noexcept override;
  void onBody(std::unique_ptr<folly::IOBuf> chain) noexcept override;
  void onTrailers(std::unique_ptr<HTTPHeaders> trailers) noexcept override;
  void onEOM() noexcept override;
  void onUpgrade(UpgradeProtocol protocol) noexcept override;
  void onError(const HTTPException& error) noexcept override;
  // These are no-ops since the direct response is already in memory
  void onEgressPaused() noexcept override {
  }
  void onEgressResumed() noexcept override {
  }

 private:
  ~HTTPDirectResponseHandler() override;

  HTTPTransaction* txn_;
  const HTTPErrorPage* errorPage_;
  std::string statusMessage_;
  unsigned statusCode_;
  bool headersSent_ : 1;
  bool eomSent_ : 1;
  bool forceConnectionClose_ : 1;
};

} // namespace proxygen
