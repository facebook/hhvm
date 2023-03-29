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

namespace {
template <class T>
T getWithDefault(StructuredHeaders::Dictionary& dict,
                 const std::string& key,
                 T value,
                 bool& missing,
                 bool& malformed) {
  auto it = dict.find(key);
  if (it == dict.end()) {
    missing = true;
    return value;
  }

  try {
    return it->second.get<T>();
  } catch (const boost::bad_get&) {
    malformed = true;
    return value;
  }
}
} // namespace

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
  SCOPE_EXIT {
    if (logBadHeader) {
      LOG_EVERY_N(ERROR, 100)
          << "Received ill-formated priority header=" << priority;
    }
  };
  StructuredHeadersDecoder decoder(priority);
  StructuredHeaders::Dictionary dict;
  auto ret = decoder.decodeDictionary(dict);
  if (ret != StructuredHeaders::DecodeError::OK) {
    logBadHeader = true;
    return folly::none;
  }

  bool uMissing = false;
  bool iMissing = false;
  bool oMissing = false;
  bool malformed = false;
  int64_t urgency = getWithDefault<int64_t>(
      dict, "u", (int64_t)kDefaultHttpPriorityUrgency, uMissing, malformed);
  bool incremental = getWithDefault(dict, "i", false, iMissing, malformed);
  auto orderId = getWithDefault<int64_t>(dict, "o", 0, oMissing, malformed);
  if ((urgency > kMaxPriority || urgency < kMinPriority) || (orderId < 0) ||
      (uMissing && iMissing && oMissing) || malformed) {
    logBadHeader = true;
    return folly::none;
  }
  return HTTPPriority(static_cast<uint8_t>(urgency),
                      incremental,
                      static_cast<uint64_t>(orderId));
}

} // namespace proxygen
