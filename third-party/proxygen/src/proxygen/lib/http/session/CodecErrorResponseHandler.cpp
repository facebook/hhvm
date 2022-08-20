/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/CodecErrorResponseHandler.h>

#include <folly/Conv.h>

using folly::IOBuf;
using std::unique_ptr;

namespace proxygen {

CodecErrorResponseHandler::CodecErrorResponseHandler(ErrorCode /*statusCode*/)
    : txn_(nullptr) {
}

CodecErrorResponseHandler::~CodecErrorResponseHandler() {
}

void CodecErrorResponseHandler::setTransaction(HTTPTransaction* txn) noexcept {
  txn_ = txn;
}

void CodecErrorResponseHandler::detachTransaction() noexcept {
  delete this;
}

void CodecErrorResponseHandler::onHeadersComplete(
    std::unique_ptr<HTTPMessage> /*msg*/) noexcept {
  VLOG(4) << "discarding headers";
}

void CodecErrorResponseHandler::onBody(unique_ptr<IOBuf> /*chain*/) noexcept {
  VLOG(4) << "discarding request body";
}

void CodecErrorResponseHandler::onTrailers(
    unique_ptr<HTTPHeaders> /*trailers*/) noexcept {
  VLOG(4) << "discarding request trailers";
}

void CodecErrorResponseHandler::onEOM() noexcept {
}

void CodecErrorResponseHandler::onUpgrade(
    UpgradeProtocol /*protocol*/) noexcept {
}

void CodecErrorResponseHandler::onError(const HTTPException& error) noexcept {
  VLOG(4) << "processing error " << error;
  txn_->sendAbort();
}

} // namespace proxygen
