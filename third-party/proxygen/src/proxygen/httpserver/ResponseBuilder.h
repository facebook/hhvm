/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/ScopeGuard.h>
#include <proxygen/httpserver/ResponseHandler.h>

namespace proxygen {

/**
 * Helps you make responses and send them on demand.
 *
 * NOTE: We don't do any correctness checks here, we depend on
 *       state machine in HTTPTransaction to tell us when an
 *       error occurs
 *
 * Three expected use cases are
 *
 * 1. Send all response at once. If this is an error
 *    response, most probably you also want 'closeConnection'.
 *
 * ResponseBuilder(handler)
 *    .status(200, "OK")
 *    .body(...)
 *    .sendWithEOM();
 *
 * 2. Sending back response in chunks.
 *
 * ResponseBuilder(handler)
 *    .status(200, "OK")
 *    .body(...)
 *    .send();  // Without `WithEOM` we make it chunked
 *
 * 1 or more time
 *
 * ResponseBuilder(handler)
 *    .body(...)
 *    .send();
 *
 * At last
 *
 * ResponseBuilder(handler)
 *    .body(...)
 *    .sendWithEOM();
 *
 * 3. Accept or reject Upgrade Requests
 *
 * ResponseBuilder(handler)
 *    .acceptUpgradeRequest() // send '200 OK' without EOM
 *
 * or
 *
 * ResponseBuilder(handler)
 *    .rejectUpgradeRequest() // send '400 Bad Request'
 *
 */
class ResponseBuilder {
 public:
  explicit ResponseBuilder(ResponseHandler* txn) : txn_(txn) {
  }

  ResponseBuilder& promise(const std::string& url, const std::string& host) {
    headers_ = std::make_unique<HTTPMessage>();
    headers_->setHTTPVersion(1, 1);
    headers_->setURL(url);
    headers_->getHeaders().set(HTTP_HEADER_HOST, host);
    return *this;
  }

  ResponseBuilder& promise(const std::string& url,
                           const std::string& host,
                           HTTPMethod method) {
    promise(url, host);
    headers_->setMethod(method);
    return *this;
  }

  ResponseBuilder& status(uint16_t code, const std::string& message) {
    headers_ = std::make_unique<HTTPMessage>();
    headers_->setHTTPVersion(1, 1);
    headers_->setStatusCode(code);
    headers_->setStatusMessage(message);
    return *this;
  }

  template <typename T>
  ResponseBuilder& header(const std::string& headerIn, const T& value) {
    CHECK(headers_) << "You need to call `status` before adding headers";
    headers_->getHeaders().add(headerIn, value);
    return *this;
  }

  template <typename T>
  ResponseBuilder& header(HTTPHeaderCode code, const T& value) {
    CHECK(headers_) << "You need to call `status` before adding headers";
    headers_->getHeaders().add(code, value);
    return *this;
  }

  ResponseBuilder& body(std::unique_ptr<folly::IOBuf> bodyIn) {
    if (bodyIn) {
      if (body_) {
        body_->prependChain(std::move(bodyIn));
      } else {
        body_ = std::move(bodyIn);
      }
    }

    return *this;
  }

  template <typename T>
  ResponseBuilder& body(T&& t) {
    return body(folly::IOBuf::maybeCopyBuffer(
        folly::to<std::string>(std::forward<T>(t))));
  }

  ResponseBuilder& closeConnection() {
    return header(HTTP_HEADER_CONNECTION, "close");
  }

  ResponseBuilder& trailers(const HTTPHeaders& trailers) {
    trailers_.reset(new HTTPHeaders(trailers));
    return *this;
  }

  void sendWithEOM() {
    sendEOM_ = true;
    send();
  }

  void send() {
    // Once we send them, we don't want to send them again
    SCOPE_EXIT {
      headers_.reset();
    };

    // By default, chunked
    bool chunked = true;

    // If we have complete response, we can use Content-Length and get done
    if (headers_ && sendEOM_) {
      chunked = false;
    }

    if (headers_) {
      // We don't need to add Content-Length or Encoding for 1xx responses
      if (headers_->isResponse() && headers_->getStatusCode() >= 200) {
        if (chunked) {
          headers_->setIsChunked(true);
        } else {
          const auto len = body_ ? body_->computeChainDataLength() : 0;
          headers_->getHeaders().add(HTTP_HEADER_CONTENT_LENGTH,
                                     folly::to<std::string>(len));
        }
      }

      txn_->sendHeaders(*headers_);
    }

    if (body_) {
      if (chunked) {
        auto bodyLength = body_->computeChainDataLength();
        txn_->sendChunkHeader(bodyLength);
        txn_->sendBody(std::move(body_));
        txn_->sendChunkTerminator();
      } else {
        txn_->sendBody(std::move(body_));
      }
    }

    if (sendEOM_) {
      if (trailers_) {
        auto txn = txn_->getTransaction();
        if (txn) {
          txn->sendTrailers(*trailers_);
        }
        trailers_.reset();
      }
      txn_->sendEOM();
    }
  }

  enum class UpgradeType {
    CONNECT_REQUEST = 0,
    HTTP_UPGRADE,
  };

  void acceptUpgradeRequest(UpgradeType upgradeType,
                            const std::string upgradeProtocol = "") {
    headers_ = std::make_unique<HTTPMessage>();
    if (upgradeType == UpgradeType::CONNECT_REQUEST) {
      headers_->constructDirectResponse({1, 1}, 200, "OK");
    } else {
      CHECK(!upgradeProtocol.empty());
      headers_->constructDirectResponse({1, 1}, 101, "Switching Protocols");
      headers_->getHeaders().add(HTTP_HEADER_UPGRADE, upgradeProtocol);
      headers_->getHeaders().add(HTTP_HEADER_CONNECTION, "Upgrade");
    }
    txn_->sendHeaders(*headers_);
  }

  void rejectUpgradeRequest() {
    headers_ = std::make_unique<HTTPMessage>();
    headers_->constructDirectResponse({1, 1}, 400, "Bad Request");
    txn_->sendHeaders(*headers_);
    txn_->sendEOM();
  }

  ResponseBuilder& setEgressWebsocketHeaders() {
    headers_->setEgressWebsocketUpgrade();
    return *this;
  }

  const HTTPMessage* getHeaders() const {
    return headers_.get();
  }

 private:
  ResponseHandler* const txn_{nullptr};

  std::unique_ptr<HTTPMessage> headers_;
  std::unique_ptr<folly::IOBuf> body_;
  std::unique_ptr<HTTPHeaders> trailers_;

  // If true, sends EOM.
  bool sendEOM_{false};
};

} // namespace proxygen
