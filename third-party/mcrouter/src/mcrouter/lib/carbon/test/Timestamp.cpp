/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/lib/carbon/test/Timestamp.h"
#include <limits>

namespace carbon {
namespace test {

uint64_t Timestamp::toUint64() const {
  return v_;
}

Timestamp Timestamp::fromUint64(uint64_t value) {
  return Timestamp(value);
}

int64_t Timestamp::toInt64() const {
  return static_cast<int64_t>(toUint64());
}

Timestamp Timestamp::fromInt64(int64_t value) {
  return fromUint64(static_cast<uint64_t>(value));
}

} // namespace test
} // namespace carbon
