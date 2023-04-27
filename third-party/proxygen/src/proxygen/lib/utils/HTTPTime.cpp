/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/HTTPTime.h>

#include <folly/portability/Time.h>

namespace proxygen {

folly::Optional<int64_t> parseHTTPDateTime(const std::string& s) {
  struct tm tm = {};

  if (s.empty()) {
    return folly::none;
  }

  // Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
  // Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
  // Sun Nov 6 08:49:37 1994        ; ANSI C's asctime() format
  //    Assume GMT as per rfc2616 (see HTTP-date):
  //       - https://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html
  if (strptime(s.c_str(), "%a, %d %b %Y %H:%M:%S GMT", &tm) != nullptr ||
      strptime(s.c_str(), "%a, %d-%b-%y %H:%M:%S GMT", &tm) != nullptr ||
      strptime(s.c_str(), "%a %b %d %H:%M:%S %Y", &tm) != nullptr) {
    return folly::Optional<int64_t>(timegm(&tm));
  }

  return folly::none;
}

} // namespace proxygen
