/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "PushRequestHandler.h"

#include "proxygen/httpserver/samples/push/PushStats.h"
#include <folly/FileUtil.h>
#include <proxygen/httpserver/PushHandler.h>
#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>

using namespace proxygen;

namespace PushService {

const std::string kPushFileName("proxygen/httpserver/samples/push/pusheen.txt");
std::string gPushBody;

// Create a large body so we can give time for the push request to be made
std::string createLargeBody() {
  std::string data = gPushBody;
  while (data.size() < 1000 * 1000) {
    data += gPushBody;
  }
  return data;
}

PushRequestHandler::PushRequestHandler(PushStats* stats) : stats_(stats) {
  if (gPushBody.empty()) {
    CHECK(folly::readFile(kPushFileName.c_str(), gPushBody))
        << "Failed to read push file=" << kPushFileName;
  }
}

void PushRequestHandler::onRequest(
    std::unique_ptr<HTTPMessage> headers) noexcept {
  stats_->recordRequest();
  if (!headers->getHeaders().getSingleOrEmpty("X-PushIt").empty()) {
    const auto downstreamPush = downstream_->newPushedResponse(new PushHandler);
    if (downstreamPush.hasError()) {
      LOG(ERROR) << "can't push: " << getErrorString(downstreamPush.error());
      return;
    }
    downstreamPush_ = downstreamPush.value();

    if (headers->getPathAsStringPiece() == "/requestLargePush") {
      LOG(INFO) << "sending large push ";

      ResponseBuilder(downstreamPush_)
          .promise("/largePush",
                   headers->getHeaders().getSingleOrEmpty(HTTP_HEADER_HOST),
                   HTTPMethod::GET)
          .send();

      ResponseBuilder(downstreamPush_)
          .status(200, "OK")
          .body(createLargeBody())
          .sendWithEOM();
    } else {
      LOG(INFO) << "sending small push ";

      ResponseBuilder(downstreamPush_)
          .promise("/pusheen",
                   headers->getHeaders().getSingleOrEmpty(HTTP_HEADER_HOST),
                   HTTPMethod::GET)
          .send();

      ResponseBuilder(downstreamPush_)
          .status(200, "OK")
          .body(gPushBody)
          .sendWithEOM();
    }
  }
}

void PushRequestHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  if (body_) {
    body_->prependChain(std::move(body));
  } else {
    body_ = std::move(body);
  }
}

void PushRequestHandler::onEOM() noexcept {
  ResponseBuilder(downstream_)
      .status(200, "OK")
      .header("Request-Number",
              folly::to<std::string>(stats_->getRequestCount()))
      .body(std::move(body_))
      .sendWithEOM();
}

void PushRequestHandler::onUpgrade(UpgradeProtocol /*protocol*/) noexcept {
  // handler doesn't support upgrades
}

void PushRequestHandler::requestComplete() noexcept {
  delete this;
}

void PushRequestHandler::onError(ProxygenError /*err*/) noexcept {
  delete this;
}

} // namespace PushService
