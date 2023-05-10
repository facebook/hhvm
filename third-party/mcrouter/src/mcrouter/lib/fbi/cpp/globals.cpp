/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "globals.h"

#include <folly/IPAddress.h>

#include "mcrouter/lib/fbi/cpp/LogFailure.h"
#include "mcrouter/lib/fbi/network.h"

namespace facebook {
namespace memcache {
namespace globals {

namespace {

bool getAddrHash(const sockaddr* addr, void* ctx) {
  auto ipe = folly::IPAddress::tryFromSockAddr(addr);
  if (ipe.hasError()) {
    return true;
  }
  auto ip = ipe.value();
  if (!ip.isLoopback() && !ip.isLinkLocal()) {
    auto result = (uint32_t*)ctx;
    result[0] = 1;
    result[1] = ip.hash();
    return false;
  }

  return true;
}

// get hash from the first non-loopback/non-linklocal ip4/6 address
uint32_t getHash() {
  uint32_t result[2] = {0};

  if (!for_each_localaddr(getAddrHash, result)) {
    LOG_FAILURE(
        "mcrouter",
        failure::Category::kSystemError,
        "Can not enumerate local addresses: {}",
        strerror(errno));
    return 0;
  }

  if (result[0] == 0) {
    LOG_FAILURE(
        "mcrouter",
        failure::Category::kBadEnvironment,
        "Can not find a valid ip addresss");
    return 0;
  }

  return result[1];
}

uint32_t* getHostIdStorage() {
  static uint32_t h = getHash();
  return &h;
}

} // anonymous namespace

uint32_t hostid() {
  return *getHostIdStorage();
}

HostidMock::HostidMock(uint32_t value) {
  *getHostIdStorage() = value;
}

void HostidMock::reset() {
  *getHostIdStorage() = getHash();
}
} // namespace globals
} // namespace memcache
} // namespace facebook
