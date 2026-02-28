/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>

#include "mcrouter/lib/HashFunctionType.h"
#include "mcrouter/lib/fbi/hash.h"

namespace folly {
struct dynamic;
} // namespace folly

namespace facebook {
namespace memcache {

/* Crc32 : crc32 hashing function object */
class Crc32HashFunc {
 public:
  explicit Crc32HashFunc(size_t n) : n_(n) {}

  size_t operator()(folly::StringPiece hashable) const {
    auto res = crc32_hash(hashable.data(), hashable.size());
    return (res & 0x7fffffff) % n_;
  }

  static const char* type() {
    return "Crc32";
  }
  static HashFunctionType typeId() {
    return HashFunctionType::CRC32;
  }

 private:
  size_t n_;
};
} // namespace memcache
} // namespace facebook
