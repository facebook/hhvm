/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/Range.h>
#include <folly/json/dynamic.h>

#include "mcrouter/lib/HashFunctionType.h"
#include "mcrouter/lib/RendezvousHashHelper.h"

namespace facebook {
namespace memcache {

/**
 * Weighted Rendezvous hashing based on RendezvousHashFunc.
 * Each server is assigned a weight between 0.0 and 1.0 inclusive.
 */
class WeightedRendezvousHashFunc {
 public:
  /**
   * @param endpoints   The strings to be hashed, one per backend server.
   *
   * @param json A list with one weight (double) per backend server, in the same
   * order as endpoints.
   *
   */
  WeightedRendezvousHashFunc(
      const std::vector<folly::StringPiece>& endpoints,
      const folly::dynamic& json);

  size_t operator()(folly::StringPiece key) const;

  class Iterator : public RendezvousIterator {
   public:
    Iterator(
        const std::vector<uint64_t>& hashes,
        const std::vector<double>& endpointWeights,
        const folly::StringPiece key);

    // An end / empty iterator.
    Iterator() {}
  };

  Iterator begin(folly::StringPiece key) const {
    return Iterator(endpointHashes_, endpointWeights_, key);
  }

  Iterator end() const {
    return Iterator();
  }

  static const char* type() {
    return "WeightedRendezvous";
  }

  static HashFunctionType typeId() {
    return HashFunctionType::WeightedRendezvous;
  }

 private:
  std::vector<uint64_t> endpointHashes_;
  std::vector<double> endpointWeights_;
};
} // namespace memcache
} // namespace facebook
