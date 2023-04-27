/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/RFC2616.h>

#include <stdlib.h>

#include <folly/String.h>
#include <folly/ThreadLocal.h>
#include <proxygen/lib/http/HTTPHeaders.h>

namespace {

/* Wapper around strtoul(3) */
bool strtoulWrapper(const char*& curs, const char* end, unsigned long& val) {
  char* endptr = nullptr;

  unsigned long v = strtoul(curs, &endptr, 10);
  if (endptr == curs) {
    return false;
  }

  if (endptr > end) {
    return false;
  }

  curs = endptr;
  val = v;

  return true;
}

bool equalsIgnoreCase(folly::StringPiece s1, folly::StringPiece s2) {
  if (s1.size() != s2.size()) {
    return false;
  }
  return std::equal(
      s1.begin(), s1.end(), s2.begin(), folly::AsciiCaseInsensitive());
}
} // namespace

namespace proxygen { namespace RFC2616 {

BodyAllowed isRequestBodyAllowed(folly::Optional<HTTPMethod> method) {
  if (method == HTTPMethod::TRACE) {
    return BodyAllowed::NOT_ALLOWED;
  }
  if (method == HTTPMethod::OPTIONS || method == HTTPMethod::POST ||
      method == HTTPMethod::PUT) {
    return BodyAllowed::DEFINED;
  }
  return BodyAllowed::NOT_DEFINED;
}

bool responseBodyMustBeEmpty(unsigned status) {
  return (status == 304 || status == 204 || (100 <= status && status < 200));
}

bool bodyImplied(const HTTPHeaders& headers) {
  return headers.exists(HTTP_HEADER_TRANSFER_ENCODING) ||
         headers.exists(HTTP_HEADER_CONTENT_LENGTH);
}

double parseQvalue(const EncodingParams& params) {
  double qvalue = 1.0;
  for (const auto& paramPair : params) {
    if (paramPair.first == "q") {
      qvalue = folly::to<double>(paramPair.second);
    }
  }
  return qvalue;
}

bool parseQvalues(folly::StringPiece value, TokenPairVec& output) {
  bool success = true;
  auto encodings = parseEncoding(value);
  if (encodings.hasException()) {
    return false;
  }

  for (const auto& pair : encodings.value()) {
    double qvalue = 1.0;
    try {
      qvalue = parseQvalue(pair.second);
    } catch (const std::range_error&) {
      // q=<some garbage>
      success = false;
    }
    output.emplace_back(pair.first, qvalue);
  }

  return success;
}

bool parseByteRangeSpec(folly::StringPiece value,
                        unsigned long& outFirstByte,
                        unsigned long& outLastByte,
                        unsigned long& outInstanceLength) {
  // We should start with "bytes "
  if (!value.startsWith("bytes ")) {
    return false;
  }

  const char* curs = value.begin() + 6 /* strlen("bytes ") */;
  const char* end = value.end();

  unsigned long firstByte = ULONG_MAX;
  unsigned long lastByte = ULONG_MAX;
  unsigned long instanceLength = ULONG_MAX;

  if (!strtoulWrapper(curs, end, firstByte)) {
    if (*curs != '*') {
      return false;
    }

    firstByte = 0;
    lastByte = ULONG_MAX;
    ++curs;
  } else {
    if (*curs != '-') {
      return false;
    }

    ++curs;

    if (!strtoulWrapper(curs, end, lastByte)) {
      return false;
    }
  }

  if (*curs != '/') {
    return false;
  }

  ++curs;
  if (*curs != '*') {
    if (!strtoulWrapper(curs, end, instanceLength)) {
      return false;
    }
  } else {
    ++curs;
  }

  if (curs < end && *curs != '\0') {
    return false;
  }

  if (lastByte < firstByte) {
    return false;
  }

  if ((lastByte - firstByte + 1) > instanceLength) {
    return false;
  }

  outFirstByte = firstByte;
  outLastByte = lastByte;
  outInstanceLength = instanceLength;
  return true;
}

folly::Try<EncodingList> parseEncoding(const folly::StringPiece header) {
  EncodingList result;
  std::vector<folly::StringPiece> topLevelTokens;
  folly::split(',', header, topLevelTokens, true /*ignore empty*/);

  if (topLevelTokens.empty()) {
    return folly::Try<EncodingList>(
        folly::make_exception_wrapper<std::runtime_error>(
            "Header value mustn't be empty."));
  }

  for (auto topLevelToken : topLevelTokens) {
    std::vector<folly::StringPiece> secondLevelTokens;
    folly::split(';', topLevelToken, secondLevelTokens, true /*ignore empty*/);

    if (secondLevelTokens.empty()) {
      return folly::Try<EncodingList>(
          folly::make_exception_wrapper<std::runtime_error>(
              "Encoding must have name."));
    }

    auto encoding = folly::trimWhitespace(secondLevelTokens.front());
    if (encoding.empty()) {
      return folly::Try<EncodingList>(
          folly::make_exception_wrapper<std::runtime_error>("Empty encoding!"));
    }
    EncodingParams params;
    params.reserve(secondLevelTokens.size() - 1);
    auto it = secondLevelTokens.begin();
    while (++it != secondLevelTokens.end()) {
      auto val = *it;
      auto key = val.split_step('=');

      key = folly::trimWhitespace(key);
      val = folly::trimWhitespace(val);

      if (key.empty()) {
        return folly::Try<EncodingList>(
            folly::make_exception_wrapper<std::runtime_error>(
                "Param key must not be empty!"));
      }

      params.emplace_back(key, val);
    }
    result.emplace_back(encoding, std::move(params));
  }
  return folly::Try<EncodingList>(result);
}

bool acceptsEncoding(const folly::StringPiece header,
                     const folly::StringPiece encoding) {
  auto encodings = parseEncoding(header);
  if (encodings.hasException()) {
    return false;
  }
  return acceptsEncoding(encodings.value(), encoding);
}

bool acceptsEncoding(const EncodingList& encodings,
                     const folly::StringPiece encoding) {
  for (const auto& pair : encodings) {
    if (equalsIgnoreCase(pair.first, encoding)) {
      try {
        auto qval = parseQvalue(pair.second);
        return qval > 0;
      } catch (const std::exception&) {
      }
      return true;
    }
  }
  return false;
}

}} // namespace proxygen::RFC2616
