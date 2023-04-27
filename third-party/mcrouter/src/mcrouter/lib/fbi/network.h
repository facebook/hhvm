/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <netinet/in.h>
#include <stdbool.h>

__BEGIN_DECLS

typedef struct {
  sa_family_t family;
  uint16_t addrlen;
  uint16_t sockaddrlen;
  uint16_t addroff;
  uint16_t strmaxlen;
} fbi_family_info_t;

static inline bool ipv4_addr_loopback(const struct in_addr* a) {
  return a->s_addr == htonl(INADDR_LOOPBACK);
}

static inline bool ipv6_addr_loopback(const struct in6_addr* a) {
  return (a->s6_addr32[0] | a->s6_addr32[1] | a->s6_addr32[2] |
          (a->s6_addr32[3] ^ htonl(1))) == 0;
}

/**
 * This routine returns information about the given address family in constant
 * time.
 */
fbi_family_info_t* get_family_info(sa_family_t family);

/**
 * This routine returns the address component of the socket address structure
 * supplied.
 */
bool get_sa_address(
    const struct sockaddr* sa,
    const void** addr,
    uint16_t* addrlen);

/**
 * This routine calls the given callback routine for each local address
 * assigned to the network interfaces of the local computer. The callback
 * can return true to continue enumeration or false to stop early.
 *
 * @return true on success enumeration of interfaces (even if callback causes
 *         an early stop; false if an error occurred while getting the list of
 *         interfaces.
 */
bool for_each_localaddr(
    bool (*cb)(const struct sockaddr* addr, void* ctx),
    void* ctx);

__END_DECLS
