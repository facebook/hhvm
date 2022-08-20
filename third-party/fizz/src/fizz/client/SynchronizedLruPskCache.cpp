/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/client/SynchronizedLruPskCache.h>

namespace fizz {
namespace client {

SynchronizedLruPskCache::SynchronizedLruPskCache(uint64_t mapMax)
    : cache_(EvictingPskMap(mapMax)) {}

folly::Optional<CachedPsk> SynchronizedLruPskCache::getPsk(
    const std::string& identity) {
  auto cacheMap = cache_.wlock();
  auto result = cacheMap->find(identity);
  if (result != cacheMap->end()) {
    if (std::chrono::system_clock::now() >
        result->second.ticketExpirationTime) {
      VLOG(1) << "PSK expired: " << identity << ", id: "
              << (result->second.serverCert
                      ? result->second.serverCert->getIdentity()
                      : "none");
      cacheMap->erase(result);
      return folly::none;
    }
    return result->second;
  } else {
    return folly::none;
  }
}

void SynchronizedLruPskCache::putPsk(
    const std::string& identity,
    CachedPsk psk) {
  auto cacheMap = cache_.wlock();
  cacheMap->set(identity, std::move(psk));
}

void SynchronizedLruPskCache::removePsk(const std::string& identity) {
  auto cacheMap = cache_.wlock();
  cacheMap->erase(identity);
}

} // namespace client
} // namespace fizz
