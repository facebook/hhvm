/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/HTTPPriorityFunctions.h>

#include <proxygen/lib/http/structuredheaders/StructuredHeadersDecoder.h>

namespace proxygen {

folly::Optional<HTTPPriority> httpPriorityFromHTTPMessage(
    const HTTPMessage& message) {
  return httpPriorityFromString(
      message.getHeaders().getSingleOrEmpty(HTTP_HEADER_PRIORITY));
}

folly::Optional<HTTPPriority> httpPriorityFromString(
    folly::StringPiece priority) {
  if (priority.empty()) {
    return folly::none;
  }
  bool logBadHeader = false;
  folly::Optional<HTTPPriority> httpPriority;
  SCOPE_EXIT {
    if (logBadHeader) {
      LOG(ERROR) << "Received ill-formated priority header=" << priority;
    }
  };
  StructuredHeadersDecoder decoder(priority);
  StructuredHeaders::Dictionary dict;
  auto ret = decoder.decodeDictionary(dict);
  if (ret != StructuredHeaders::DecodeError::OK) {
    logBadHeader = true;
    return folly::none;
  }
  if (dict.size() > 2) {
    logBadHeader = true;
    return folly::none;
  }
  bool hasUrgency = dict.find("u") != dict.end();
  bool hasIncremental = dict.find("i") != dict.end();
  if (dict.size() == 2 && !(hasUrgency && hasIncremental)) {
    logBadHeader = true;
    return folly::none;
  }
  if (dict.size() == 1 && !hasUrgency) {
    logBadHeader = true;
    return folly::none;
  }
  if (!hasUrgency || dict["u"].tag != StructuredHeaderItem::Type::INT64) {
    logBadHeader = true;
    return folly::none;
  }
  folly::tryTo<uint8_t>(dict["u"].get<int64_t>()).then([&](uint8_t urgency) {
    if (urgency > kMaxPriority) {
      logBadHeader = true;
    }
    bool incremental = false;
    if (hasIncremental) {
      if (dict["i"].tag != StructuredHeaderItem::Type::BOOLEAN) {
        logBadHeader = true;
      }
      incremental = true;
    }
    httpPriority.emplace(urgency, incremental);
  });
  return httpPriority;
}

} // namespace proxygen
