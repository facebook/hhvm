/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <stdint.h>

namespace carbon {
namespace util {

inline uint16_t zigzag(const int16_t i) {
  return (static_cast<uint16_t>(i) << 1) ^ static_cast<uint16_t>(i >> 15);
}

inline uint32_t zigzag(const int32_t i) {
  return (static_cast<uint32_t>(i) << 1) ^ static_cast<uint32_t>(i >> 31);
}

inline uint64_t zigzag(const int64_t i) {
  return (static_cast<uint64_t>(i) << 1) ^ static_cast<uint64_t>(i >> 63);
}

inline int16_t unzigzag(const uint16_t i) {
  return (i >> 1) ^ -(i & 1);
}

inline int32_t unzigzag(const uint32_t i) {
  return (i >> 1) ^ -(i & 1);
}

inline int64_t unzigzag(const uint64_t i) {
  return (i >> 1) ^ -(i & 1);
}

} // namespace util
} // namespace carbon
