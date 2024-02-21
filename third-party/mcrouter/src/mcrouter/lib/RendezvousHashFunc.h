/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <queue>
#include <vector>

#include <glog/logging.h>

#include <folly/Range.h>
#include <folly/json/dynamic.h>

#include "mcrouter/lib/HashFunctionType.h"
#include "mcrouter/lib/RendezvousHashHelper.h"

namespace facebook {
namespace memcache {

/**
 * An implementation of Rendezvous hashing. For efficiency reasons, we don't
 * hash mixed keys directly. Instead we mix the endpoint hash and key hash.
 *
 * See https://en.wikipedia.org/wiki/Rendezvous_hashing for a detailed
 * description of the algorithm.
 */
class RendezvousHashFunc {
 public:
  /**
   * @param endpoints  A list of backend servers
   */
  RendezvousHashFunc(
      const std::vector<folly::StringPiece>& endpoints,
      const folly::dynamic& json);

  size_t operator()(folly::StringPiece key) const {
    return *begin(key);
  }

  RendezvousIterator begin(folly::StringPiece key) const;

  RendezvousIterator end() const {
    return RendezvousIterator();
  }

  static const char* type() {
    return "Rendezvous";
  }

  static HashFunctionType typeId() {
    return HashFunctionType::Rendezvous;
  }

 private:
  std::vector<uint64_t> endpointHashes_;
};
} // namespace memcache
} // namespace facebook
