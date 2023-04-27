/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/compress/HPACKHeader.h>

namespace proxygen {

std::ostream& operator<<(std::ostream& os, const HPACKHeader& h) {
  os << h.name << ": " << h.value;
  return os;
}

} // namespace proxygen
