/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>
#include <string>

namespace proxygen {

// This is a wrapper around openssl base64 encoding.  It is a temporary wrapper
// around openssl, until a more optimized version lands in folly

class Base64 {
 public:
  static std::string decode(const std::string& b64message, int padding);
  static std::string urlDecode(const std::string& b64message);
  static std::string encode(folly::ByteRange buffer);
  static std::string urlEncode(folly::ByteRange buffer);
};

} // namespace proxygen
