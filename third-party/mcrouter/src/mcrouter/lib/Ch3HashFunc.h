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

namespace facebook::memcache {

/* CH3 consistent hashing function object */
class Ch3HashFunc {
 public:
  explicit Ch3HashFunc(size_t n) : n_(n) {
    if (!n_ || n_ > furc_maximum_pool_size()) {
      throw std::logic_error("Pool size out of range for Ch3");
    }
  }

  size_t operator()(folly::StringPiece hashable) const {
    return furc_hash(hashable.data(), hashable.size(), n_);
  }

  static const char* type() {
    return "Ch3";
  }

  static HashFunctionType typeId() {
    return HashFunctionType::CH3;
  }

 private:
  size_t n_;
};
} // namespace facebook::memcache
