/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/RequestAclChecker.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

RequestAclChecker::RequestAclChecker(
    ExternalStatsHandler& statsHandler,
    bool requestAclCheckerEnable)
    : requestAclCheckerEnable_(requestAclCheckerEnable),
      requestAclCheckCb_(initRequestAclCheckCbIfEnabled(statsHandler)) {}

/* Determines if the command is of the form "refresh prefix-acl" */
bool RequestAclChecker::isRefreshCommand(
    const folly::StringPiece cmd) noexcept {
  std::vector<folly::StringPiece> parts;
  folly::split(' ', cmd, parts, true);
  return parts.size() == 2 && parts.at(0) == "refresh" &&
      parts.at(1) == "prefix-acl";
}

MemcacheRequestAclCheckerCallback
RequestAclChecker::initRequestAclCheckCbIfEnabled(
    ExternalStatsHandler& statsHandler) const noexcept {
  if (requestAclCheckerEnable_) {
    return getMemcacheServerRequestAclCheckCallback(statsHandler);
  }
  return {};
}

bool RequestAclChecker::isLocalRequest(
    const folly::Optional<struct sockaddr_storage>& address) noexcept {
  const auto addr = address.hasValue()
      ? reinterpret_cast<const struct sockaddr*>(address.get_pointer())
      : nullptr;
  if (!addr) {
    return false;
  }
  if (addr->sa_family == AF_INET) {
    struct in_addr* addr4 = &((struct sockaddr_in*)addr)->sin_addr;
    if (folly::IPAddressV4(*addr4).isLoopback()) {
      return true;
    }
  } else if (addr && addr->sa_family == AF_INET6) {
    struct in6_addr* addr6 = &((struct sockaddr_in6*)addr)->sin6_addr;
    if (folly::IPAddressV6(*addr6).isLoopback()) {
      return true;
    }
  }
  return false;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
