/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "EchoHandler.h"

#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>

#include "EchoStats.h"

using namespace proxygen;

DEFINE_bool(request_number,
            true,
            "Include request sequence number in response");

namespace EchoService {

EchoHandler::EchoHandler(EchoStats* stats) : stats_(stats) {
}

void EchoHandler::onRequest(std::unique_ptr<HTTPMessage> req) noexcept {
  stats_->recordRequest();
  ResponseBuilder builder(downstream_);
  builder.status(200, "OK");
  if (FLAGS_request_number) {
    builder.header("Request-Number",
                   folly::to<std::string>(stats_->getRequestCount()));
  }
  req->getHeaders().forEach([&](std::string& name, std::string& value) {
    builder.header(folly::to<std::string>("x-echo-", name), value);
  });
  builder.send();
}

void EchoHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  ResponseBuilder(downstream_).body(std::move(body)).send();
}

void EchoHandler::onEOM() noexcept {
  ResponseBuilder(downstream_).sendWithEOM();
}

void EchoHandler::onUpgrade(UpgradeProtocol /*protocol*/) noexcept {
  // handler doesn't support upgrades
}

void EchoHandler::requestComplete() noexcept {
  delete this;
}

void EchoHandler::onError(ProxygenError /*err*/) noexcept {
  delete this;
}
} // namespace EchoService
