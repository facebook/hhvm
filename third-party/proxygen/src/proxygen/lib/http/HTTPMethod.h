/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Optional.h>
#include <folly/Range.h>
#include <string>

namespace proxygen {

/**
 * Defined in winnt.h
 * Several proxygen files include folly/portability/OpenSSL.h
 *   -> folly/portability/Windows.h -> Windows.h -> winnt.h
 */
#if defined(_WIN32) && defined(DELETE)
#undef DELETE
#endif

/**
 * See the definitions in RFC2616 5.1.1 for the source of this
 * list. Today, proxygen only understands the methods defined in 5.1.1 and
 * is not aware of any extension methods. If you wish to support extension
 * methods, you must handle those separately from this enum.
 */
enum class HTTPMethod {
  GET,
  POST,
  OPTIONS,
  DELETE,
  HEAD,
  CONNECT,
  CONNECT_UDP,
  PUT,
  TRACE,
  PATCH,
  SUB,
  PUB,
  UNSUB
};

/**
 * Returns the HTTPMethod that matches the method. Although RFC2616 5.1.1
 * says methods are case-sensitive, we ignore case here since most
 * programmers probably really meant "GET" not "get". If the method is not
 * recognized, the return value will be None
 */
extern folly::Optional<HTTPMethod> stringToMethod(folly::StringPiece method);

/**
 * Returns a string representation of the method. If EXTENSION_METHOD is
 * passed, then an empty string is returned
 */
extern const std::string& methodToString(HTTPMethod method);

std::ostream& operator<<(std::ostream& os, HTTPMethod method);

} // namespace proxygen
