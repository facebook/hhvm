/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "proxygen/lib/dns/DNSModule.h"

#include <folly/Singleton.h>
#include <folly/SocketAddress.h>
#include <folly/portability/GFlags.h>

using folly::SocketAddress;

FOLLY_GFLAGS_DEFINE_int32(dns_cache_size, 4096, "DNS cache size");
FOLLY_GFLAGS_DEFINE_int32(dns_cache_clear_size, 256, "DNS cache clear size");
FOLLY_GFLAGS_DEFINE_int32(stale_dns_cache_size_multiplier,
                          4,
                          "Size multiplier for stale dns cache");
FOLLY_GFLAGS_DEFINE_int32(stale_cache_ttl_min,
                          86400,
                          "Stale dns cache min TTL in secs");
FOLLY_GFLAGS_DEFINE_int32(stale_cache_ttl_scale,
                          3,
                          "Stale dns cache TTL multiplier");

namespace proxygen {

static folly::Singleton<DNSModule> gDNSModule;

std::shared_ptr<DNSModule> DNSModule::get() {
  return gDNSModule.try_get();
}

DNSModule::DNSModule() {
  cacheMaxSize_ = FLAGS_dns_cache_size;
  cacheClearSize_ = FLAGS_dns_cache_clear_size;
  staleCacheSizeMultiplier_ = FLAGS_stale_dns_cache_size_multiplier;
  staleCacheTTLMin_ = FLAGS_stale_cache_ttl_min;
  staleCacheTTLScale_ = FLAGS_stale_cache_ttl_scale;
}

} // namespace proxygen
