/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <gmock/gmock.h>

#include <proxygen/lib/http/HTTPMessageFilters.h>

namespace proxygen {

static const std::string kMockFilterName = "MockFilter";

class MockHTTPMessageFilter : public HTTPMessageFilter {
 public:
  MOCK_METHOD((void),
              onHeadersComplete,
              (std::shared_ptr<HTTPMessage>),
              (noexcept));
  MOCK_METHOD((void), onBody, (std::shared_ptr<folly::IOBuf>), (noexcept));
  MOCK_METHOD((void), pause, (), (noexcept));
  MOCK_METHOD((void), onChunkHeader, (size_t), (noexcept));
  MOCK_METHOD((void), resume, (uint64_t), (noexcept));
  MOCK_METHOD((void), onChunkComplete, (), (noexcept));
  MOCK_METHOD((void), onTrailers, (std::shared_ptr<HTTPHeaders>), (noexcept));
  MOCK_METHOD((void), onEOM, (), (noexcept));
  MOCK_METHOD((void), onUpgrade, (UpgradeProtocol), (noexcept));
  MOCK_METHOD((void), onError, (const HTTPException&), (noexcept));

  void onHeadersComplete(std::unique_ptr<HTTPMessage> msg) noexcept override {
    if (trackHeadersPassedThrough_) {
      requestHeadersCopy_ =
          std::make_shared<const HTTPHeaders>(msg->getHeaders());
    }
    onHeadersComplete(std::shared_ptr<HTTPMessage>(msg.release()));
  }

  void onBody(std::unique_ptr<folly::IOBuf> chain) noexcept override {
    if (trackDataPassedThrough_) {
      bodyDataReceived_.append(chain->clone());
    }
    onBody(std::shared_ptr<folly::IOBuf>(chain.release()));
  }

  void onTrailers(std::unique_ptr<HTTPHeaders> trailers) noexcept override {
    if (trackTrailersPassedThrough_) {
      requestTrailersCopy_ =
          trailers ? std::make_shared<const HTTPHeaders>(*trailers.get())
                   : nullptr;
    }
    onTrailers(std::shared_ptr<HTTPHeaders>(trailers.release()));
  }

  void nextOnHeadersCompletePublic(std::shared_ptr<HTTPMessage> msg) {
    std::unique_ptr<HTTPMessage> msgU(new HTTPMessage(*msg));
    nextOnHeadersComplete(std::move(msgU));
  }

  const std::string& getFilterName() const noexcept override {
    return kMockFilterName;
  }

  boost::variant<HTTPMessageFilter*, HTTPSink*> getPrevElement() {
    return prev_;
  }

  [[noreturn]] std::unique_ptr<HTTPMessageFilter> clone() noexcept override {
    LOG(FATAL) << "clone() not implemented for filter: "
               << this->getFilterName();
  };

  void setAllowDSR(bool allow) {
    allowDSR_ = allow;
  }

  bool allowDSR() const noexcept override {
    return allowDSR_;
  }

  void nextOnEOMPublic() {
    nextOnEOM();
  }

  std::unique_ptr<folly::IOBuf> bodyDataSinceLastCheck() {
    return bodyDataReceived_.move();
  }

  void setTrackDataPassedThrough(bool track) {
    trackDataPassedThrough_ = track;
  }

  void setTrackHeadersPassedThrough(bool track = true) {
    trackHeadersPassedThrough_ = track;
  }

  void setTrackTrailersPassedThrough(bool track = true) {
    trackTrailersPassedThrough_ = track;
  }

  std::shared_ptr<const HTTPHeaders> getRequestHeadersCopy() {
    return requestHeadersCopy_;
  }

  std::shared_ptr<const HTTPHeaders> getRequestTrailersCopy() {
    return requestTrailersCopy_;
  }

 private:
  folly::IOBufQueue bodyDataReceived_{folly::IOBufQueue::cacheChainLength()};
  bool trackDataPassedThrough_{false};
  bool trackHeadersPassedThrough_{false};
  bool trackTrailersPassedThrough_{false};
  std::shared_ptr<const HTTPHeaders> requestHeadersCopy_;
  std::shared_ptr<const HTTPHeaders> requestTrailersCopy_;
  bool allowDSR_{true};
};

} // namespace proxygen
