/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/session/HTTPDirectResponseHandler.h>

#include <folly/Conv.h>
#include <proxygen/lib/http/session/HTTPErrorPage.h>

using folly::IOBuf;
using std::string;
using std::unique_ptr;

namespace proxygen {

HTTPDirectResponseHandler::HTTPDirectResponseHandler(
    unsigned statusCode,
    const std::string& statusMsg,
    const HTTPErrorPage* errorPage)
    : txn_(nullptr),
      errorPage_(errorPage),
      statusMessage_(statusMsg),
      statusCode_(statusCode),
      headersSent_(false),
      eomSent_(false),
      forceConnectionClose_(true) {
}

HTTPDirectResponseHandler::~HTTPDirectResponseHandler() {
}

void HTTPDirectResponseHandler::setTransaction(HTTPTransaction* txn) noexcept {
  txn_ = txn;
}

void HTTPDirectResponseHandler::detachTransaction() noexcept {
  delete this;
}

void HTTPDirectResponseHandler::onHeadersComplete(
    std::unique_ptr<HTTPMessage> /*msg*/) noexcept {
  VLOG(4) << "processing request";
  headersSent_ = true;
  HTTPMessage response;
  std::unique_ptr<folly::IOBuf> responseBody;
  response.setHTTPVersion(1, 1);
  response.setStatusCode(statusCode_);
  if (!statusMessage_.empty()) {
    response.setStatusMessage(statusMessage_);
  } else {
    response.setStatusMessage(HTTPMessage::getDefaultReason(statusCode_));
  }
  if (forceConnectionClose_) {
    response.getHeaders().add(HTTP_HEADER_CONNECTION, "close");
  }
  if (errorPage_) {
    HTTPErrorPage::Page page = errorPage_->generate(
        0, statusCode_, statusMessage_, nullptr, empty_string);
    VLOG(4) << "sending error page with type " << page.contentType;
    response.getHeaders().add(HTTP_HEADER_CONTENT_TYPE, page.contentType);
    responseBody = std::move(page.content);
  }
  response.getHeaders().add(
      HTTP_HEADER_CONTENT_LENGTH,
      folly::to<string>(responseBody ? responseBody->computeChainDataLength()
                                     : 0));
  txn_->sendHeaders(response);
  if (responseBody) {
    txn_->sendBody(std::move(responseBody));
  }
}

void HTTPDirectResponseHandler::onBody(unique_ptr<IOBuf> /*chain*/) noexcept {
  VLOG(4) << "discarding request body";
}

void HTTPDirectResponseHandler::onTrailers(
    unique_ptr<HTTPHeaders> /*trailers*/) noexcept {
  VLOG(4) << "discarding request trailers";
}

void HTTPDirectResponseHandler::onEOM() noexcept {
  eomSent_ = true;
  txn_->sendEOM();
}

void HTTPDirectResponseHandler::onUpgrade(
    UpgradeProtocol /*protocol*/) noexcept {
}

void HTTPDirectResponseHandler::onError(const HTTPException& error) noexcept {
  if (error.getDirection() == HTTPException::Direction::INGRESS) {
    if (error.getProxygenError() == kErrorTimeout) {
      VLOG(4) << "processing ingress timeout";
      if (!headersSent_) {
        onHeadersComplete(nullptr);
      }
      if (!eomSent_) {
        onEOM();
      }
    } else {
      VLOG(4) << "processing ingress error";
      if (!headersSent_) {
        onHeadersComplete(nullptr);
      }
      if (!eomSent_) {
        onEOM();
      }
    }
  }
}

} // namespace proxygen
