/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/TransportDirection.h>

#include <ostream>

namespace proxygen {

const char* getTransportDirectionString(TransportDirection dir) {
  switch (dir) {
    case TransportDirection::UPSTREAM:
      return "upstream";
    case TransportDirection::DOWNSTREAM:
      return "downstream";
  }
  // unreachable
  return "";
}

TransportDirection operator!(TransportDirection dir) {
  return dir == TransportDirection::DOWNSTREAM ? TransportDirection::UPSTREAM
                                               : TransportDirection::DOWNSTREAM;
}

std::ostream& operator<<(std::ostream& os, const TransportDirection dir) {
  os << getTransportDirectionString(dir);
  return os;
}

} // namespace proxygen
