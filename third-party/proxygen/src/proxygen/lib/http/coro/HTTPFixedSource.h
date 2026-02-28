/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/logging/xlog.h>
#include <proxygen/lib/http/coro/HTTPSource.h>
#include <proxygen/lib/utils/URL.h>

namespace proxygen::coro {

/**
 * An HTTP source from an in memory source.
 *
 * Useful to be the head of a chain of sources
 *
 * h2upstreamSession.newRequest(HTTPRateLimitSource(HTTPFixedSource(msg,
 * body)));
 */

class HTTPFixedSource : public HTTPSource {
 public:
  explicit HTTPFixedSource(std::unique_ptr<HTTPMessage> msg,
                           std::unique_ptr<folly::IOBuf> body = nullptr)
      : msg_(std::move(msg)) {
    if (body) {
      body_.append(std::move(body));
      msg_->getHeaders().set(HTTP_HEADER_CONTENT_LENGTH,
                             folly::to<std::string>(body_.chainLength()));
    }
  }

  explicit HTTPFixedSource(std::unique_ptr<folly::IOBuf> body) {
    body_.append(std::move(body));
  }

  // Copy constructor that does a deep copy of the provided Source
  HTTPFixedSource(const HTTPFixedSource& source)
      : HTTPFixedSource(std::make_unique<HTTPMessage>(*source.msg_),
                        source.body_.clone()) {
  }

  HTTPFixedSource(HTTPFixedSource&& source) noexcept
      : HTTPFixedSource(std::move(source.msg_), source.body_.move()) {
  }

  static HTTPFixedSource* makeFixedSource(
      std::unique_ptr<HTTPMessage> msg,
      std::unique_ptr<folly::IOBuf> body = nullptr) {
    auto source = new HTTPFixedSource(std::move(msg), std::move(body));
    source->setHeapAllocated();
    return source;
  }

  static HTTPFixedSource* makeFixedBody(std::unique_ptr<folly::IOBuf> body) {
    auto resp = new HTTPFixedSource(std::move(body));
    resp->setHeapAllocated();
    return resp;
  }

  static HTTPFixedSource* makeFixedRequest(
      URL url,
      HTTPMethod method = HTTPMethod::GET,
      std::unique_ptr<folly::IOBuf> body = nullptr) {
    auto req = makeFixedRequest(url.makeRelativeURL(), method, std::move(body));
    req->msg_->setSecure(url.isSecure());
    if (url.hasHost()) {
      req->msg_->getHeaders().set(HTTP_HEADER_HOST,
                                  url.getHostAndPortOmitDefault());
    }
    return req;
  }
  static HTTPFixedSource* makeFixedRequest(
      std::string path,
      HTTPMethod method = HTTPMethod::GET,
      std::unique_ptr<folly::IOBuf> body = nullptr) {
    auto msg = std::make_unique<HTTPMessage>();
    msg->setURL(std::move(path));
    msg->setMethod(method);
    msg->setHTTPVersion(1, 1);
    auto resp = new HTTPFixedSource(std::move(msg), std::move(body));
    resp->setHeapAllocated();
    return resp;
  }

  static HTTPFixedSource* makeFixedResponse(uint16_t statusCode,
                                            std::string body) {
    return makeFixedResponse(statusCode,
                             folly::IOBuf::fromString(std::move(body)));
  }

  static HTTPFixedSource* makeFixedResponse(
      uint16_t statusCode, std::unique_ptr<folly::IOBuf> body = nullptr) {
    auto msg = std::make_unique<HTTPMessage>();
    msg->setStatusCode(statusCode);
    msg->setStatusMessage(HTTPMessage::getDefaultReason(statusCode));
    msg->setHTTPVersion(1, 1);
    auto resp = new HTTPFixedSource(std::move(msg), std::move(body));
    resp->setHeapAllocated();
    return resp;
  }

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override {
    HTTPHeaderEvent event(std::move(msg_), body_.empty());
    auto guard = folly::makeGuard(lifetime(event));
    co_return event;
  }

  folly::coro::Task<HTTPBodyEvent> readBodyEvent(
      uint32_t max = std::numeric_limits<uint32_t>::max()) override {
    if (!pushes_.empty()) {
      auto res = std::move(pushes_.front());
      pushes_.pop_front();
      co_return res;
    }
    if (body_.empty() && trailers_) {
      HTTPBodyEvent event(std::move(trailers_));
      auto guard = folly::makeGuard(lifetime(event));
      co_return event;
    } else {
      auto body = body_.splitAtMost(max);
      HTTPBodyEvent event(std::move(body), body_.empty() && !trailers_);
      auto guard = folly::makeGuard(lifetime(event));
      co_return event;
    }
  }

  void stopReading(folly::Optional<const HTTPErrorCode>) override {
    if (heapAllocated_) {
      delete this;
    }
  }

  void setReadTimeout(std::chrono::milliseconds) override {
  }

  std::unique_ptr<HTTPMessage> msg_;
  std::list<HTTPBodyEvent> pushes_;
  std::unique_ptr<HTTPHeaders> trailers_;
  BufQueue body_;
};

class HTTPErrorSource : public HTTPSource {
 public:
  explicit HTTPErrorSource(HTTPError error, bool heapAllocated = true)
      : error_(std::move(error)) {
    if (heapAllocated) {
      setHeapAllocated();
    }
  }

  folly::coro::Task<HTTPHeaderEvent> readHeaderEvent() override {
    auto error = std::move(error_);
    if (heapAllocated_) {
      delete this;
    }
    co_yield folly::coro::co_error(error);
  }

  folly::coro::Task<HTTPBodyEvent> readBodyEvent(
      uint32_t /*max = std::numeric_limits<uint32_t>::max()*/) override {
    XLOG(FATAL) << "Unreachable";
  }

  void stopReading(
      folly::Optional<const HTTPErrorCode> = folly::none) override {
    if (heapAllocated_) {
      delete this;
    }
  }

  void setReadTimeout(std::chrono::milliseconds) override {
  }

  HTTPError error_;
};

} // namespace proxygen::coro
