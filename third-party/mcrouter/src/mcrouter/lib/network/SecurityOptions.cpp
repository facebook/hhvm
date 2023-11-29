/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "SecurityOptions.h"

#include <exception>

#include <folly/lang/Assume.h>

namespace facebook {
namespace memcache {

const char* securityMechToString(SecurityMech mech) {
  switch (mech) {
    case SecurityMech::NONE:
      return "plain";
    case SecurityMech::TLS:
      return "ssl";
    case SecurityMech::TLS_TO_PLAINTEXT:
      return "tls_to_plain";
    case SecurityMech::TLS13_FIZZ:
      return "fizz";
    case SecurityMech::KTLS12:
      return "ktls12";
  }
  folly::assume_unreachable();
}

SecurityMech parseSecurityMech(folly::StringPiece s) {
  if (s == "ssl") {
    return SecurityMech::TLS;
  } else if (s == "plain") {
    return SecurityMech::NONE;
  } else if (s == "tls_to_plain") {
    return SecurityMech::TLS_TO_PLAINTEXT;
  } else if (s == "fizz") {
    return SecurityMech::TLS13_FIZZ;
  } else if (s == "ktls12") {
    return SecurityMech::KTLS12;
  }
  throw std::invalid_argument("Invalid security mech");
}

} // namespace memcache
} // namespace facebook
