/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

#include <folly/Range.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

/**
 * Stores mcrouter routing prefix ("/region/cluster/" string).
 * Invariant: stored routing prefix is always of form
 * "/region/cluster/", where 'region' and 'cluster' are non-empty strings.
 */
class RoutingPrefix {
 public:
  RoutingPrefix() = default;
  /**
   * Constructor from string
   *
   * @throws  std::invalid_argument if prefix is not of form
   *          "[/]region/cluster[/]"
   */
  explicit RoutingPrefix(std::string prefix);
  explicit RoutingPrefix(const char* prefix);
  explicit RoutingPrefix(folly::StringPiece prefix);

  // Move and copy constructors
  RoutingPrefix(const RoutingPrefix& other) noexcept;
  RoutingPrefix(RoutingPrefix&& other) noexcept;
  RoutingPrefix& operator=(const RoutingPrefix& other) noexcept;
  RoutingPrefix& operator=(RoutingPrefix&& other) noexcept;

  // Conversion to string and StringPiece
  /* implicit */ operator const std::string&() const {
    return prefix_;
  }
  /* implicit */ operator folly::StringPiece() const {
    return prefix_;
  }
  const std::string& str() const {
    return prefix_;
  }

  // 'region' and 'cluster' parts accessors
  folly::StringPiece getRegion() const {
    return region_;
  }
  folly::StringPiece getCluster() const {
    return cluster_;
  }

 private:
  std::string prefix_;
  folly::StringPiece region_;
  folly::StringPiece cluster_;

  void initFromPrefix();
  void initFromPrefixUnsafe();
};

template <class String>
void toAppend(const RoutingPrefix& prefix, String* out) {
  out->append(prefix.str());
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
