/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <iostream>

#include <folly/io/async/EventBase.h>
#include <folly/json/json.h>

#include "mcrouter/lib/Reply.h"
#include "mcrouter/lib/carbon/CarbonMessageConversionUtils.h"
#include "mcrouter/lib/network/AsyncMcClient.h"

namespace carbon {

template <class Request>
bool JsonClient::sendRequest(
    const folly::dynamic& requestJson,
    folly::dynamic& replyJson) {
  bool hasErrors = false;
  auto onParsingError = [this, &hasErrors](
                            folly::StringPiece field, folly::StringPiece msg) {
    hasErrors = true;
    onError(folly::to<std::string>(field, ": ", msg));
  };
  Request request;
  convertFromFollyDynamic(requestJson, request, std::move(onParsingError));

  if (!options().ignoreParsingErrors && hasErrors) {
    onError("Found errors parsing json. Aborting - requests will not be sent.");
    return false;
  }

  facebook::memcache::ReplyT<Request> reply;
  bool replyReceived = false;
  fiberManager_.addTask(
      [this, request = std::move(request), &replyReceived, &reply]() {
        reply = client_.sendSync(
            std::move(request),
            std::chrono::milliseconds(options().serverTimeoutMs));
        replyReceived = true;
      });

  while (!replyReceived) {
    evb_.loopOnce();
  }

  replyJson = convertToFollyDynamic(reply);
  return true;
}

} // namespace carbon
