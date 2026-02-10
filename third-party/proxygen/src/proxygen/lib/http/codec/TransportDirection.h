/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <iosfwd>
#include <stdint.h>
#include <string_view>

namespace proxygen {

enum class TransportDirection : uint8_t {
  DOWNSTREAM = 0, // toward the client
  UPSTREAM = 1    // toward the origin application or data
};

inline bool isUpstream(TransportDirection dir) noexcept {
  return dir == TransportDirection::UPSTREAM;
}

inline bool isDownstream(TransportDirection dir) noexcept {
  return dir == TransportDirection::DOWNSTREAM;
}

std::string_view getTransportDirectionString(TransportDirection dir);

TransportDirection operator!(TransportDirection dir);

std::ostream& operator<<(std::ostream& os, const TransportDirection dir);

} // namespace proxygen
