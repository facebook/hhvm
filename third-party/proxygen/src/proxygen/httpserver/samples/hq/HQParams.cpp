/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/httpserver/samples/hq/HQParams.h>

namespace quic::samples {

bool HTTPVersion::parse(const std::string& verString) {
  // version, major and minor are fields of struct HTTPVersion
  version = verString;
  if (version.length() == 1) {
    major = folly::to<uint16_t>(version);
    minor = 0;
    canonical = folly::to<std::string>(major, ".", minor);
    return true;
  }
  std::string delimiter = ".";
  std::size_t pos = version.find(delimiter);
  if (pos == std::string::npos) {
    LOG(ERROR) << "Invalid http-version string: " << version
               << ", defaulting to HTTP/1.1";
    major = 1;
    minor = 1;
    canonical = folly::to<std::string>(major, ".", minor);
    return false;
  }

  try {
    std::string majorVer = version.substr(0, pos);
    std::string minorVer = version.substr(pos + delimiter.length());
    major = folly::to<uint16_t>(majorVer);
    minor = folly::to<uint16_t>(minorVer);
    canonical = folly::to<std::string>(major, ".", minor);
    return true;
  } catch (const folly::ConversionError&) {
    LOG(ERROR) << "Invalid http-version string: " << version
               << ", defaulting to HTTP/1.1";
    major = 1;
    minor = 1;
    canonical = folly::to<std::string>(major, ".", minor);
    return false;
  }
}

std::ostream& operator<<(std::ostream& o, const HTTPVersion& v) {
  o << "http-version=" << v.major << "/" << v.minor << " (orig=" << v.version
    << ", canonical=" << v.canonical << ")";
  return o;
}

} // namespace quic::samples
