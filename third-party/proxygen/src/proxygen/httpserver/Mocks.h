/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/portability/GMock.h>
#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseHandler.h>

namespace proxygen {

/**
 * Mocks to help with testing
 */

class MockResponseHandler : public ResponseHandler {
 public:
  explicit MockResponseHandler(RequestHandler* h) : ResponseHandler(h) {
  }
#ifdef __clang__
#pragma clang diagnostic push
#if __clang_major__ > 3 || __clang_minor__ >= 6
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
#endif

  MOCK_METHOD((void), pauseIngress, (), (noexcept));
  MOCK_METHOD((void), refreshTimeout, (), (noexcept));
  MOCK_METHOD((void), resumeIngress, (), (noexcept));
  MOCK_METHOD((void), sendAbort, (), (noexcept));
  MOCK_METHOD((void), sendBody, (std::shared_ptr<folly::IOBuf>), (noexcept));
  MOCK_METHOD((void), sendChunkHeader, (size_t), (noexcept));
  MOCK_METHOD((void), sendChunkTerminator, (), (noexcept));
  MOCK_METHOD((void), sendEOM, (), (noexcept));
  MOCK_METHOD((void), sendHeaders, (HTTPMessage&), (noexcept));
  MOCK_METHOD((void), sendTrailers, (const HTTPHeaders&), (noexcept));
  MOCK_METHOD((folly::Expected<ResponseHandler*, ProxygenError>),
              newPushedResponse,
              (PushHandler*),
              (noexcept));
  MOCK_METHOD(void, getCurrentTransportInfo, (wangle::TransportInfo*), (const));

#ifdef __clang__
#pragma clang diagnostic pop
#endif

  const wangle::TransportInfo& getSetupTransportInfo() const noexcept override {
    return transportInfo;
  }

  void sendBody(std::unique_ptr<folly::IOBuf> body) noexcept override {
    if (body) {
      sendBody(std::shared_ptr<folly::IOBuf>(body.release()));
    } else {
      sendBody(std::shared_ptr<folly::IOBuf>(nullptr));
    }
  }

  wangle::TransportInfo transportInfo;
};

class MockRequestHandler : public RequestHandler {
 public:
#ifdef __clang__
#pragma clang diagnostic push
#if __clang_major__ > 3 || __clang_minor__ >= 6
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
#endif
  MOCK_METHOD((bool), canHandleExpect, (), (noexcept));
  MOCK_METHOD((void), onBody, (std::shared_ptr<folly::IOBuf>), (noexcept));
  MOCK_METHOD((void), onEOM, (), (noexcept));
  MOCK_METHOD((void), onEgressPaused, (), (noexcept));
  MOCK_METHOD((void), onEgressResumed, (), (noexcept));
  MOCK_METHOD((void), onError, (ProxygenError), (noexcept));
  MOCK_METHOD((void), onGoaway, (ErrorCode), (noexcept));
  MOCK_METHOD((void), onRequest, (std::shared_ptr<HTTPMessage>), (noexcept));
  MOCK_METHOD((void), onUpgrade, (UpgradeProtocol), (noexcept));
  MOCK_METHOD((void), requestComplete, (), (noexcept));
  MOCK_METHOD((void), setResponseHandler, (ResponseHandler*), (noexcept));

#ifdef __clang__
#pragma clang diagnostic pop
#endif

  void onRequest(std::unique_ptr<HTTPMessage> headers) noexcept override {
    onRequest(std::shared_ptr<HTTPMessage>(headers.release()));
  }

  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override {
    if (body) {
      onBody(std::shared_ptr<folly::IOBuf>(body.release()));
    } else {
      onBody(std::shared_ptr<folly::IOBuf>(nullptr));
    }
  }
};

} // namespace proxygen
