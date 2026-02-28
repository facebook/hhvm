// @nolint
/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "proxygen/lib/dns/Rfc6724.h"

#include <algorithm>
#include <cerrno>
#include <climits>
#ifdef _WIN32
#include <folly/portability/Windows.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#endif
#include <folly/portability/Unistd.h>
#include <string>
#include <system_error>

using folly::SocketAddress;
using std::string;
using std::vector;

namespace {

struct SortElement {
  SocketAddress* addr;
  bool hasSrcAddr;
  SocketAddress srcAddr;
  size_t originalOrder;
}; // struct SortElement

static bool find_src_addr(const SocketAddress* addr, SocketAddress& srcAddr);
static int get_common_prefix(const in6_addr* src, const in6_addr* dst);
static int get_label(const SocketAddress* addr);
static int get_scope(const SocketAddress* addr);
static int get_precedence(const SocketAddress* addr);
static int rfc6724_compare(const void* ptr1, const void* ptr2);

} // namespace

namespace proxygen {

void rfc6724_sort(vector<SocketAddress>& addrs,
                  const SocketAddress* srcAddr /* = nullptr */) {
  /** 
   * Some callers require strong exception safety guarentees. If `find_src_addr`
   * throws an exception, the output (e.g. vector<SocketAddress>& addrs) must 
   * be left in a valid state; this is a note to prevent future developers from 
   * breaking this guarantee.
   */
  vector<SortElement> sortVec;
  for (size_t i = 0; i < addrs.size(); ++i) {
    SortElement elem;
    elem.addr = &addrs[i];
    if (srcAddr == nullptr) {
      // throws if find_src_addr fails
      elem.hasSrcAddr = find_src_addr(elem.addr, elem.srcAddr);
    } else {
      elem.hasSrcAddr = true;
      elem.srcAddr = *srcAddr;
    }
    elem.originalOrder = i;
    sortVec.push_back(elem);
  }

  std::qsort(
      sortVec.data(), sortVec.size(), sizeof(SortElement), rfc6724_compare);
  for (size_t i = 0; i < addrs.size(); i++) {
    if (sortVec[i].originalOrder > i) {
      std::swap(addrs[i], addrs[sortVec[i].originalOrder]);
    }
  }
}

} // namespace proxygen

namespace {

#define IPV6_ADDR_SCOPE_NODELOCAL 0x01
#define IPV6_ADDR_SCOPE_INTFACELOCAL 0x01
#define IPV6_ADDR_SCOPE_LINKLOCAL 0x02
#define IPV6_ADDR_SCOPE_SITELOCAL 0x05
#define IPV6_ADDR_SCOPE_ORGLOCAL 0x08
#define IPV6_ADDR_SCOPE_GLOBAL 0x0e

#define IPV6_ADDR_MC_SCOPE(a) ((a)->s6_addr[1] & 0x0f)

/* RFC 4193. */
#define IN6_IS_ADDR_ULA(a) (((a)->s6_addr[0] & 0xfe) == 0xfc)

#ifndef IN_LOOPBACK // this macro is defined in mac headers: netinet/in.h
#define IN_LOOPBACK(a) ((((long int)(a)) & 0xff000000) == 0x7f000000)
#endif

/* These macros are modelled after the ones in <netinet/in6.h>. */

/* RFC 4380, section 2.6 */
#define IN6_IS_ADDR_TEREDO(a) (((const uint32_t*)(a))[0] == ntohl(0x20010000))

/* RFC 3056, section 2. */
#ifdef IN6_IS_ADDR_6TO4 // this macro is defined in mac sys headers:
                        // netinet/in.h
#undef IN6_IS_ADDR_6TO4
#endif
#define IN6_IS_ADDR_6TO4(a) \
  (((a)->s6_addr[0] == 0x20) && ((a)->s6_addr[1] == 0x02))

/* 6bone testing address area (3ffe::/16), deprecated in RFC 3701. */
#define IN6_IS_ADDR_6BONE(a) \
  (((a)->s6_addr[0] == 0x3f) && ((a)->s6_addr[1] == 0xfe))

// throws std::system_error on socket() fail
static bool find_src_addr(const SocketAddress* addr, SocketAddress& srcAddr) {
  int r = 0, sock = 0;
  socklen_t len = 0;
  sockaddr_storage sa;

  switch (addr->getFamily()) {
    case AF_INET:
      len = sizeof(sockaddr_in);
      break;
    case AF_INET6:
      len = sizeof(sockaddr_in6);
      break;
    default:
      /* No known usable source address for non-INET families. */
      return false;
  }

  sock = ::socket(addr->getFamily(), SOCK_DGRAM, IPPROTO_UDP);
  if (sock == -1) {
    auto err = errno;
    if (err == EAFNOSUPPORT) {
      return false;
    } else {
      throw std::system_error(err, std::system_category());
    }
  }

  do {
    sockaddr_storage addrStorage;
    addr->getAddress(&addrStorage);
    sockaddr* saddr = reinterpret_cast<sockaddr*>(&addrStorage);
    r = ::connect(sock, saddr, len);
  } while (r == -1 && errno == EINTR);

  if (r == -1) {
#ifdef _WIN32
    ::closesocket(sock);
#else
    close(sock);
#endif
    return false;
  }

  r = ::getsockname(sock, reinterpret_cast<sockaddr*>(&sa), &len);
#ifdef _WIN32
  ::closesocket(sock);
#else
  close(sock);
#endif

  if (r == -1) {
    throw std::system_error(errno, std::system_category());
  }

  try {
    srcAddr.setFromSockaddr(reinterpret_cast<sockaddr*>(&sa), len);
  } catch (...) {
    throw std::system_error(EPROTONOSUPPORT, std::system_category());
  }

  return true;
}

static int get_scope(const SocketAddress* addr) {
  if (addr->getFamily() == AF_INET6) {
    const in6_addr i6a = addr->getIPAddress().asV6().toAddr();
    const in6_addr* i6ap = &i6a;

    if (IN6_IS_ADDR_MULTICAST(i6ap)) {
      return IPV6_ADDR_MC_SCOPE(i6ap);
    } else if (IN6_IS_ADDR_LOOPBACK(i6ap) || IN6_IS_ADDR_LINKLOCAL(i6ap)) {
      /*
       * RFC 4291 section 2.5.3 says loopback is to be treated as having
       * link-local scope.
       */
      return IPV6_ADDR_SCOPE_LINKLOCAL;
    } else if (IN6_IS_ADDR_SITELOCAL(i6ap)) {
      return IPV6_ADDR_SCOPE_SITELOCAL;
    } else {
      return IPV6_ADDR_SCOPE_GLOBAL;
    }
  } else if (addr->getFamily() == AF_INET) {
    const in_addr ia = addr->getIPAddress().asV4().toAddr();

    unsigned long int na = ntohl(ia.s_addr);

    if (IN_LOOPBACK(na) ||                 /* 127.0.0.0/8 */
        (na & 0xffff0000) == 0xa9fe0000) { /* 169.254.0.0/16 */
      return IPV6_ADDR_SCOPE_LINKLOCAL;
    } else {
      /*
       * RFC 6724 section 3.2. Other IPv4 addresses, including private addresses
       * and shared addresses (100.64.0.0/10), are assigned global scope.
       */
      return IPV6_ADDR_SCOPE_GLOBAL;
    }
  }
  /*
   * This should never happen.
   * Return a scope with low priority as a last resort.
   */
  return IPV6_ADDR_SCOPE_NODELOCAL;
}

/*
 * Get the label for a given IPv4/IPv6 address.
 * RFC 6724, section 2.1.
 */
static int get_label(const SocketAddress* addr) {
  if (addr->getFamily() == AF_INET) {
    return 4;
  } else if (addr->getFamily() == AF_INET6) {
    const in6_addr i6a = addr->getIPAddress().asV6().toAddr();
    const in6_addr* i6ap = &i6a;

    if (IN6_IS_ADDR_LOOPBACK(i6ap)) {
      return 0;
    } else if (IN6_IS_ADDR_V4MAPPED(i6ap)) {
      return 4;
    } else if (IN6_IS_ADDR_6TO4(i6ap)) {
      return 2;
    } else if (IN6_IS_ADDR_TEREDO(i6ap)) {
      return 5;
    } else if (IN6_IS_ADDR_ULA(i6ap)) {
      return 13;
    } else if (IN6_IS_ADDR_V4COMPAT(i6ap)) {
      return 3;
    } else if (IN6_IS_ADDR_SITELOCAL(i6ap)) {
      return 11;
    } else if (IN6_IS_ADDR_6BONE(i6ap)) {
      return 12;
    } else {
      /* All other IPv6 addresses, including global unicast addresses. */
      return 1;
    }
  } else {
    /*
     * This should never happen.
     * Return a semi-random label as a last resort.
     */
    return 1;
  }
}

/*
 * Get the precedence for a given IPv4/IPv6 address.
 * RFC 6724, section 2.1.
 */
static int get_precedence(const SocketAddress* addr) {
  if (addr->getFamily() == AF_INET) {
    return 35;
  } else if (addr->getFamily() == AF_INET6) {
    const in6_addr i6a = addr->getIPAddress().asV6().toAddr();
    const in6_addr* i6ap = &i6a;

    if (IN6_IS_ADDR_LOOPBACK(i6ap)) {
      return 50;
    } else if (IN6_IS_ADDR_V4MAPPED(i6ap)) {
      return 35;
    } else if (IN6_IS_ADDR_6TO4(i6ap)) {
      return 30;
    } else if (IN6_IS_ADDR_TEREDO(i6ap)) {
      return 5;
    } else if (IN6_IS_ADDR_ULA(i6ap)) {
      return 3;
    } else if (IN6_IS_ADDR_V4COMPAT(i6ap) || IN6_IS_ADDR_SITELOCAL(i6ap) ||
               IN6_IS_ADDR_6BONE(i6ap)) {
      return 1;
    } else {
      /* All other IPv6 addresses, including global unicast addresses. */
      return 40;
    }
  } else {
    return 1;
  }
}

/*
 * Find number of matching initial bits between the two addresses a1 and a2.
 */
static int get_common_prefix(const in6_addr* src, const in6_addr* dst) {
  const char* p1 = reinterpret_cast<const char*>(src);
  const char* p2 = reinterpret_cast<const char*>(dst);

  for (unsigned i = 0; i < sizeof(*src); ++i) {
    int x, j;

    if (p1[i] == p2[i]) {
      continue;
    }
    x = p1[i] ^ p2[i];
    for (j = 0; j < CHAR_BIT; ++j) {
      if (x & (1 << (CHAR_BIT - 1))) {
        return i * CHAR_BIT + j;
      }
      x <<= 1;
    }
  }
  return sizeof(*src) * CHAR_BIT;
}

/*
 * Compare two source/destination address pairs.
 * RFC 6724, section 6.
 */
static int rfc6724_compare(const void* ptr1, const void* ptr2) {
  const SortElement* l = reinterpret_cast<const SortElement*>(ptr1);
  const SortElement* r = reinterpret_cast<const SortElement*>(ptr2);
  int scopeSrcL, scopeDstL, scopeMatchL;
  int scopeSrcR, scopeDstR, scopeMatchR;
  int labelSrcL, labelDstL, labelMatchL;
  int labelSrcR, labelDstR, labelMatchR;
  int precedenceL, precedenceR;
  int prefixLenL, prefixLenR;

  /* Rule 1: Avoid unusable destinations. */
  if (l->hasSrcAddr != r->hasSrcAddr) {
    return int(r->hasSrcAddr) - int(l->hasSrcAddr);
  }

  /* Rule 2: Prefer matching scope. */
  scopeSrcL = get_scope(&l->srcAddr);
  scopeDstL = get_scope(l->addr);
  scopeMatchL = (scopeSrcL == scopeDstL);

  scopeSrcR = get_scope(&r->srcAddr);
  scopeDstR = get_scope(r->addr);
  scopeMatchR = (scopeSrcR == scopeDstR);

  if (scopeMatchL != scopeMatchR) {
    return int(scopeMatchR) - int(scopeMatchL);
  }

  /*
   * Rule 3: Avoid deprecated addresses.
   * XXX: We don't currently have a good way of finding this.
   */

  /*
   * Rule 4: Prefer home addresses.
   * XXX: We don't currently have a good way of finding this.
   */

  /* Rule 5: Prefer matching label. */
  labelSrcL = get_label(&l->srcAddr);
  labelDstL = get_label(l->addr);
  labelMatchL = (labelSrcL == labelDstL);

  labelSrcR = get_label(&r->srcAddr);
  labelDstR = get_label(r->addr);
  labelMatchR = (labelSrcR == labelDstR);

  if (labelMatchL != labelMatchR) {
    return int(labelMatchR) - int(labelMatchL);
  }

  /* Rule 6: Prefer higher precedence. */
  precedenceL = get_precedence(l->addr);
  precedenceR = get_precedence(r->addr);
  if (precedenceL != precedenceR) {
    return precedenceR - precedenceL;
  }

  /*
   * Rule 7: Prefer native transport.
   * XXX: We don't currently have a good way of finding this.
   */

  /* Rule 8: Prefer smaller scope. */
  if (scopeDstL != scopeDstR) {
    return scopeDstL - scopeDstR;
  }

  /*
   * Rule 9: Use longest matching prefix.
   * We implement this for IPv6 only, as the rules in RFC 6724 don't seem
   * to work very well directly applied to IPv4. (glibc uses information from
   * the routing table for a custom IPv4 implementation here.)
   */
  if (l->hasSrcAddr && l->addr->getFamily() == AF_INET6 && r->hasSrcAddr &&
      r->addr->getFamily() == AF_INET6) {

    const in6_addr addrSrcL = l->addr->getIPAddress().asV6().toAddr();
    const in6_addr addrDstL = l->srcAddr.getIPAddress().asV6().toAddr();
    const in6_addr addrSrcR = r->addr->getIPAddress().asV6().toAddr();
    const in6_addr addrDstR = r->srcAddr.getIPAddress().asV6().toAddr();

    prefixLenL = get_common_prefix(&addrSrcL, &addrDstL);
    prefixLenR = get_common_prefix(&addrSrcR, &addrDstR);
    if (prefixLenL != prefixLenR) {
      return prefixLenR - prefixLenL;
    }
  }

  /*
   * Rule 10: Leave the order unchanged.
   * We need this since qsort() is not necessarily stable.
   */
  return int(l->originalOrder) - int(r->originalOrder);
}

} // namespace
