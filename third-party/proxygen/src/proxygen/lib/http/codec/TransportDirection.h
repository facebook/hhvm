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

namespace proxygen {

enum class TransportDirection : uint8_t {
  DOWNSTREAM, // toward the client
  UPSTREAM    // toward the origin application or data
};

const char* getTransportDirectionString(TransportDirection dir);

TransportDirection operator!(TransportDirection dir);

std::ostream& operator<<(std::ostream& os, const TransportDirection dir);

} // namespace proxygen
