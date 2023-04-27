/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "mcrouter/ProxyDestinationKey.h"

#include "folly/Format.h"
#include "mcrouter/ProxyDestinationBase.h"
#include "mcrouter/lib/network/AccessPoint.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

ProxyDestinationKey::ProxyDestinationKey(const ProxyDestinationBase& dst)
    : ProxyDestinationKey(
          *dst.accessPoint(),
          dst.shortestWriteTimeout(),
          dst.idx()) {}
ProxyDestinationKey::ProxyDestinationKey(
    const AccessPoint& ap,
    std::chrono::milliseconds timeout_,
    uint32_t id)
    : accessPoint(ap), timeout(timeout_), idx(id) {}

std::string ProxyDestinationKey::str() const {
  std::string additionalConnectionIdx;
  if (idx > 0) {
    additionalConnectionIdx = folly::sformat("-{}", idx);
  }
  if (accessPoint.getProtocol() == mc_ascii_protocol) {
    // we cannot send requests with different timeouts for ASCII, since
    // it will break in-order nature of the protocol
    return folly::sformat(
        "{}-{}{}",
        accessPoint.toString(),
        timeout.count(),
        additionalConnectionIdx);
  } else {
    return folly::sformat(
        "{}{}", accessPoint.toString(), additionalConnectionIdx);
  }
}

size_t ProxyDestinationKey::hash() const {
  auto result = folly::hash::hash_combine(
      accessPoint.getHost(),
      accessPoint.getPort(),
      static_cast<std::underlying_type_t<mc_protocol_e>>(
          accessPoint.getProtocol()),
      static_cast<std::underlying_type_t<SecurityMech>>(
          accessPoint.getSecurityMech()),
      accessPoint.compressed());
  if (idx > 0) {
    result = folly::hash::hash_combine(result, idx);
  }
  if (accessPoint.getProtocol() == mc_ascii_protocol) {
    result = folly::hash::hash_combine(result, timeout.count());
  }
  return result;
}

bool ProxyDestinationKey::operator==(const ProxyDestinationKey& other) const {
  if (accessPoint.getHost() != other.accessPoint.getHost() ||
      accessPoint.getPort() != other.accessPoint.getPort() ||
      accessPoint.getProtocol() != other.accessPoint.getProtocol() ||
      accessPoint.getSecurityMech() != other.accessPoint.getSecurityMech() ||
      accessPoint.compressed() != other.accessPoint.compressed() ||
      idx != other.idx) {
    return false;
  }
  return timeout == other.timeout ||
      accessPoint.getProtocol() != mc_ascii_protocol;
}

} // namespace mcrouter
} // namespace memcache
} // namespace facebook
