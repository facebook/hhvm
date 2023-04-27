/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "RoutingPrefix.h"

#include <stdexcept>

#include <folly/String.h>

namespace facebook {
namespace memcache {
namespace mcrouter {

RoutingPrefix::RoutingPrefix(std::string prefix) : prefix_(std::move(prefix)) {
  initFromPrefix();
}

RoutingPrefix::RoutingPrefix(const char* prefix) : prefix_(prefix) {
  initFromPrefix();
}

RoutingPrefix::RoutingPrefix(folly::StringPiece prefix)
    : prefix_(prefix.str()) {
  initFromPrefix();
}

RoutingPrefix::RoutingPrefix(const RoutingPrefix& other) noexcept
    : prefix_(other.prefix_) {
  initFromPrefixUnsafe();
}

RoutingPrefix::RoutingPrefix(RoutingPrefix&& other) noexcept
    : prefix_(std::move(other.prefix_)) {
  initFromPrefixUnsafe();
}

RoutingPrefix& RoutingPrefix::operator=(const RoutingPrefix& other) noexcept {
  return *this = RoutingPrefix(other);
}

RoutingPrefix& RoutingPrefix::operator=(RoutingPrefix&& other) noexcept {
  swap(prefix_, other.prefix_);
  initFromPrefixUnsafe();
  return *this;
}

void RoutingPrefix::initFromPrefix() {
  if (prefix_.empty()) {
    throw std::invalid_argument("Routing prefix can not be empty");
  }

  // add trailing end leading slash, if missing
  if (prefix_[0] != '/') {
    prefix_ = "/" + prefix_;
  }

  if (prefix_.back() != '/') {
    prefix_ += "/";
  }

  std::vector<folly::StringPiece> parts;
  folly::split('/', prefix_, parts);
  // empty, region (non-empty), cluster (non-empty), empty
  if (parts.size() != 4 || !parts[0].empty() || parts[1].empty() ||
      parts[2].empty() || !parts[3].empty()) {
    throw std::invalid_argument(
        "Routing prefix (" + prefix_ +
        ") should be of the form /region/cluster/");
  }

  region_ = parts[1];
  cluster_ = parts[2];
}

void RoutingPrefix::initFromPrefixUnsafe() {
  auto splitPos = prefix_.find('/', 1);
  assert(splitPos != std::string::npos);

  region_.assign(prefix_.data() + 1, prefix_.data() + splitPos);
  cluster_.assign(
      prefix_.data() + splitPos + 1, prefix_.data() + prefix_.size() - 1);
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
