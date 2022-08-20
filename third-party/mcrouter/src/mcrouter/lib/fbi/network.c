/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "network.h"

#include <ifaddrs.h>
#include <stddef.h>

static fbi_family_info_t family_info[] = {
    [AF_INET] =
        {
            .family = AF_INET,
            .addrlen = sizeof(struct in_addr),
            .sockaddrlen = sizeof(struct sockaddr_in),
            .addroff = offsetof(struct sockaddr_in, sin_addr),
            .strmaxlen = INET_ADDRSTRLEN,
        },
    [AF_INET6] =
        {
            .family = AF_INET6,
            .addrlen = sizeof(struct in6_addr),
            .sockaddrlen = sizeof(struct sockaddr_in6),
            .addroff = offsetof(struct sockaddr_in6, sin6_addr),
            .strmaxlen = INET6_ADDRSTRLEN,
        },
};

fbi_family_info_t* get_family_info(sa_family_t family) {
  fbi_family_info_t* info;

  if (family >= sizeof(family_info) / sizeof(family_info[0])) {
    return NULL;
  }

  info = family_info + family;

  return info->family ? info : NULL;
}

bool get_sa_address(
    const struct sockaddr* sa,
    const void** addr,
    uint16_t* addrlen) {
  fbi_family_info_t* info;

  if (!sa) {
    return false;
  }

  info = get_family_info(sa->sa_family);
  if (!info) {
    return false;
  }

  *addr = (const char*)sa + info->addroff;
  *addrlen = info->addrlen;

  return true;
}

bool for_each_localaddr(
    bool (*cb)(const struct sockaddr* addr, void* ctx),
    void* ctx) {
  struct ifaddrs* addrs;
  struct ifaddrs* it;

  if (getifaddrs(&addrs) == -1) {
    return false;
  }

  for (it = addrs; it; it = it->ifa_next) {
    if (it->ifa_addr && !cb(it->ifa_addr, ctx)) {
      break;
    }
  }

  freeifaddrs(addrs);

  return true;
}
